from io import StringIO
import types
import logging
import platform
from typing import Optional

from llvmlite import ir

from .ipopt_model_ext import RawModel, ApplicationReturnStatus, load_library
from .codegen_c import generate_csrc_prelude, generate_csrc_from_graph
from .jit_c import TCCJITCompiler
from .codegen_llvm import create_llvmir_basic_functions, generate_llvmir_from_graph
from .jit_llvm import LLJITCompiler
from .function_tracing import trace_function, FunctionTracingResult, Vars, Params

from .core_ext import ConstraintIndex, ConstraintSense
from .nleval_ext import (
    NLConstraintIndex,
    FunctionIndex,
    AutodiffSymbolicStructure,
    AutodiffEvaluator,
)
from .cppad_interface_ext import (
    cppad_trace_function,
    CppADAutodiffGraph,
    cppad_autodiff,
)

from .attributes import (
    VariableAttribute,
    ConstraintAttribute,
    ModelAttribute,
    ResultStatusCode,
    TerminationStatusCode,
)
from .solver_common import (
    _direct_get_model_attribute,
    _direct_set_model_attribute,
    _direct_get_entity_attribute,
    _direct_set_entity_attribute,
)
from .aml import make_nd_variable


def detected_libraries():
    libs = []

    # default names
    default_libnames = {
        "Linux": ["libipopt.so"],
        "Darwin": ["libipopt.dylib"],
        "Windows": ["ipopt-3.dll", "ipopt.dll", "libipopt-3.dll", "libipopt.dll"],
    }[platform.system()]
    libs.extend(default_libnames)

    return libs


def autoload_library():
    libs = detected_libraries()
    for lib in libs:
        ret = load_library(lib)
        if ret:
            logging.info(f"Loaded IPOPT library: {lib}")
            return True
    return False


autoload_library()


def compile_functions_c(model: "Model", jit_compiler: TCCJITCompiler):
    io = StringIO()

    generate_csrc_prelude(io)

    for (
        function_index,
        cppad_autodiff_graph,
    ) in model.function_cppad_autodiff_graphs.items():
        name = model.function_names[function_index]
        autodiff_structure = model.function_autodiff_structures[function_index]

        np = autodiff_structure.np
        ny = autodiff_structure.ny

        f_name = name
        generate_csrc_from_graph(
            io,
            cppad_autodiff_graph.f,
            f_name,
            np=np,
            indirect_x=True,
            indirect_p=True,
        )
        if autodiff_structure.has_jacobian:
            jacobian_name = name + "_jacobian"
            generate_csrc_from_graph(
                io,
                cppad_autodiff_graph.jacobian,
                jacobian_name,
                np=np,
                indirect_x=True,
                indirect_p=True,
            )
            gradient_name = name + "_gradient"
            generate_csrc_from_graph(
                io,
                cppad_autodiff_graph.jacobian,
                gradient_name,
                np=np,
                indirect_x=True,
                indirect_p=True,
                indirect_y=True,
                add_y=True,
            )
        if autodiff_structure.has_hessian:
            hessian_name = name + "_hessian"
            generate_csrc_from_graph(
                io,
                cppad_autodiff_graph.hessian,
                hessian_name,
                np=np,
                hessian_lagrange=True,
                nw=ny,
                indirect_x=True,
                indirect_p=True,
                indirect_y=True,
                add_y=True,
            )

    csrc = io.getvalue()

    jit_compiler.source_code = csrc

    jit_compiler.compile_string(csrc.encode())

    for (
        function_index,
        cppad_autodiff_graph,
    ) in model.function_cppad_autodiff_graphs.items():
        name = model.function_names[function_index]
        autodiff_structure = model.function_autodiff_structures[function_index]

        f_name = name
        jacobian_name = name + "_jacobian"
        gradient_name = name + "_gradient"
        hessian_name = name + "_hessian"

        f_ptr = jit_compiler.get_symbol(f_name.encode())
        jacobian_ptr = gradient_ptr = hessian_ptr = 0
        if autodiff_structure.has_jacobian:
            jacobian_ptr = jit_compiler.get_symbol(jacobian_name.encode())
            gradient_ptr = jit_compiler.get_symbol(gradient_name.encode())
        if autodiff_structure.has_hessian:
            hessian_ptr = jit_compiler.get_symbol(hessian_name.encode())

        evaluator = AutodiffEvaluator(
            autodiff_structure, f_ptr, jacobian_ptr, gradient_ptr, hessian_ptr
        )
        model._set_function_evaluator(function_index, evaluator)


def compile_functions_llvm(model: "Model", jit_compiler: LLJITCompiler):
    module = ir.Module(name="my_module")
    create_llvmir_basic_functions(module)

    export_functions = []

    for (
        function_index,
        cppad_autodiff_graph,
    ) in model.function_cppad_autodiff_graphs.items():
        name = model.function_names[function_index]
        autodiff_structure = model.function_autodiff_structures[function_index]

        np = autodiff_structure.np
        ny = autodiff_structure.ny

        f_name = name
        generate_llvmir_from_graph(
            module,
            cppad_autodiff_graph.f,
            f_name,
            np=np,
            indirect_x=True,
            indirect_p=True,
        )
        if autodiff_structure.has_jacobian:
            jacobian_name = name + "_jacobian"
            generate_llvmir_from_graph(
                module,
                cppad_autodiff_graph.jacobian,
                jacobian_name,
                np=np,
                indirect_x=True,
                indirect_p=True,
            )
            gradient_name = name + "_gradient"
            generate_llvmir_from_graph(
                module,
                cppad_autodiff_graph.jacobian,
                gradient_name,
                np=np,
                indirect_x=True,
                indirect_p=True,
                indirect_y=True,
                add_y=True,
            )
        if autodiff_structure.has_hessian:
            hessian_name = name + "_hessian"
            generate_llvmir_from_graph(
                module,
                cppad_autodiff_graph.hessian,
                hessian_name,
                np=np,
                hessian_lagrange=True,
                nw=ny,
                indirect_x=True,
                indirect_p=True,
                indirect_y=True,
                add_y=True,
            )

        export_functions.extend([f_name, jacobian_name, gradient_name, hessian_name])

    jit_compiler.compile_module(module, export_functions)

    for (
        function_index,
        cppad_autodiff_graph,
    ) in model.function_cppad_autodiff_graphs.items():
        name = model.function_names[function_index]
        autodiff_structure = model.function_autodiff_structures[function_index]

        f_name = name
        jacobian_name = name + "_jacobian"
        gradient_name = name + "_gradient"
        hessian_name = name + "_hessian"

        f_ptr = jit_compiler.get_symbol(f_name)
        jacobian_ptr = gradient_ptr = hessian_ptr = 0
        if autodiff_structure.has_jacobian:
            jacobian_ptr = jit_compiler.get_symbol(jacobian_name)
            gradient_ptr = jit_compiler.get_symbol(gradient_name)
        if autodiff_structure.has_hessian:
            hessian_ptr = jit_compiler.get_symbol(hessian_name)

        evaluator = AutodiffEvaluator(
            autodiff_structure, f_ptr, jacobian_ptr, gradient_ptr, hessian_ptr
        )
        model._set_function_evaluator(function_index, evaluator)


variable_attribute_get_func_map = {
    VariableAttribute.Value: lambda model, v: model.get_value(v),
    VariableAttribute.LowerBound: lambda model, v: model.get_variable_lb(v),
    VariableAttribute.UpperBound: lambda model, v: model.get_variable_ub(v),
    VariableAttribute.PrimalStart: lambda model, v: model.get_variable_start(v),
    VariableAttribute.Name: lambda model, v: model.get_variable_name(v),
}

variable_attribute_set_func_map = {
    VariableAttribute.LowerBound: lambda model, v, val: model.set_variable_lb(v, val),
    VariableAttribute.UpperBound: lambda model, v, val: model.set_variable_ub(v, val),
    VariableAttribute.PrimalStart: lambda model, v, val: model.set_variable_start(
        v, val
    ),
    VariableAttribute.Name: lambda model, v, val: model.set_variable_name(v, val),
}


def get_dualstatus(model):
    status = model.m_status
    if status == ApplicationReturnStatus.Solve_Succeeded:
        return ResultStatusCode.FEASIBLE_POINT
    elif status == ApplicationReturnStatus.Feasible_Point_Found:
        return ResultStatusCode.FEASIBLE_POINT
    elif status == ApplicationReturnStatus.Solved_To_Acceptable_Level:
        return ResultStatusCode.NEARLY_FEASIBLE_POINT
    else:
        return ResultStatusCode.UNKNOWN_RESULT_STATUS


def get_primalstatus(model):
    status = model.m_status
    if status == ApplicationReturnStatus.Solve_Succeeded:
        return ResultStatusCode.FEASIBLE_POINT
    elif status == ApplicationReturnStatus.Feasible_Point_Found:
        return ResultStatusCode.FEASIBLE_POINT
    elif status == ApplicationReturnStatus.Solved_To_Acceptable_Level:
        return ResultStatusCode.NEARLY_FEASIBLE_POINT
    elif status == ApplicationReturnStatus.Infeasible_Problem_Detected:
        return ResultStatusCode.INFEASIBLE_POINT
    else:
        return ResultStatusCode.UNKNOWN_RESULT_STATUS


def get_rawstatusstring(model):
    status = model.m_status
    return status.name


def get_terminationstatus(model):
    status = model.m_status
    if (
        status == ApplicationReturnStatus.Solve_Succeeded
        or status == ApplicationReturnStatus.Feasible_Point_Found
    ):
        return TerminationStatusCode.LOCALLY_SOLVED
    elif status == ApplicationReturnStatus.Infeasible_Problem_Detected:
        return TerminationStatusCode.LOCALLY_INFEASIBLE
    elif status == ApplicationReturnStatus.Solved_To_Acceptable_Level:
        return TerminationStatusCode.ALMOST_LOCALLY_SOLVED
    elif status == ApplicationReturnStatus.Search_Direction_Becomes_Too_Small:
        return TerminationStatusCode.NUMERICAL_ERROR
    elif status == ApplicationReturnStatus.Diverging_Iterates:
        return TerminationStatusCode.NORM_LIMIT
    elif status == ApplicationReturnStatus.User_Requested_Stop:
        return TerminationStatusCode.INTERRUPTED
    elif status == ApplicationReturnStatus.Maximum_Iterations_Exceeded:
        return TerminationStatusCode.ITERATION_LIMIT
    elif status == ApplicationReturnStatus.Maximum_CpuTime_Exceeded:
        return TerminationStatusCode.TIME_LIMIT
    elif status == ApplicationReturnStatus.Maximum_WallTime_Exceeded:
        return TerminationStatusCode.TIME_LIMIT
    elif status == ApplicationReturnStatus.Restoration_Failed:
        return TerminationStatusCode.NUMERICAL_ERROR
    elif status == ApplicationReturnStatus.Error_In_Step_Computation:
        return TerminationStatusCode.NUMERICAL_ERROR
    elif status == ApplicationReturnStatus.Invalid_Option:
        return TerminationStatusCode.INVALID_OPTION
    elif status == ApplicationReturnStatus.Not_Enough_Degrees_Of_Freedom:
        return TerminationStatusCode.INVALID_MODEL
    elif status == ApplicationReturnStatus.Invalid_Problem_Definition:
        return TerminationStatusCode.INVALID_MODEL
    elif status == ApplicationReturnStatus.Invalid_Number_Detected:
        return TerminationStatusCode.INVALID_MODEL
    elif status == ApplicationReturnStatus.Unrecoverable_Exception:
        return TerminationStatusCode.OTHER_ERROR
    elif status == ApplicationReturnStatus.NonIpopt_Exception_Thrown:
        return TerminationStatusCode.OTHER_ERROR
    else:
        assert status == ApplicationReturnStatus.Insufficient_Memory
        return TerminationStatusCode.MEMORY_LIMIT


model_attribute_get_func_map = {
    ModelAttribute.ObjectiveValue: lambda model: model.get_obj_value(),
    ModelAttribute.DualStatus: get_dualstatus,
    ModelAttribute.PrimalStatus: get_primalstatus,
    ModelAttribute.RawStatusString: get_rawstatusstring,
    ModelAttribute.TerminationStatus: get_terminationstatus,
    ModelAttribute.SolverName: lambda _: "IPOPT",
}

model_attribute_set_func_map = {
    ModelAttribute.TimeLimitSec: lambda model, v: model.set_raw_parameter(
        "max_wall_time", v
    ),
    ModelAttribute.Silent: lambda model, v: model.set_raw_option_bool(
        "print_level", 0 if v else 5
    ),
}


def get_constraint_primal(model, constraint):
    if isinstance(constraint, ConstraintIndex):
        return model.get_constraint_primal(constraint.index)
    elif isinstance(constraint, NLConstraintIndex):
        index = constraint.index
        dim = constraint.dim
        values = [model.get_constraint_primal(index + i) for i in range(dim)]
        return values

    raise ValueError(f"Unknown constraint type: {type(constraint)}")


def get_constraint_dual(model, constraint):
    if isinstance(constraint, ConstraintIndex):
        return model.get_constraint_dual(constraint.index)
    elif isinstance(constraint, NLConstraintIndex):
        index = constraint.index
        dim = constraint.dim
        values = [model.get_constraint_dual(index + i) for i in range(dim)]
        return values

    raise ValueError(f"Unknown constraint type: {type(constraint)}")


constraint_attribute_get_func_map = {
    ConstraintAttribute.Primal: get_constraint_primal,
    ConstraintAttribute.Dual: get_constraint_dual,
}

constraint_attribute_set_func_map = {}


def match_variable_values(
    tracing_result: FunctionTracingResult, vars_dict: dict[str, any]
):
    nx = tracing_result.n_variables()
    variable_indices_map = tracing_result.variable_indices_map
    var_values = [None for _ in range(nx)]
    for v_name, value in vars_dict.items():
        index = variable_indices_map.get(v_name, None)
        if index is None:
            raise ValueError(f"Unknown variable name: {v_name}")
        var_values[index] = value
    for i, value in enumerate(var_values):
        if value is None:
            missing_name = tracing_result.variable_names[i]
            raise ValueError(f"Missing initial value for variable {missing_name}")
    return var_values


def match_parameter_values(
    tracing_result: FunctionTracingResult, params_dict: dict[str, any]
):
    np = tracing_result.n_parameters()
    parameter_indices_map = tracing_result.parameter_indices_map
    param_values = [None for _ in range(np)]
    for p_name, value in params_dict.items():
        index = parameter_indices_map.get(p_name, None)
        if index is None:
            raise ValueError(f"Unknown parameter name: {p_name}")
        param_values[index] = value
    for i, value in enumerate(param_values):
        if value is None:
            missing_name = tracing_result.parameter_names[i]
            raise ValueError(f"Missing initial value for parameter {missing_name}")
    return param_values


class Model(RawModel):
    def __init__(self, jit: str = "LLVM"):
        super().__init__()

        if jit == "C":
            self.jit_compiler = TCCJITCompiler()
        elif jit == "LLVM":
            self.jit_compiler = LLJITCompiler()
        else:
            raise ValueError(f"JIT engine can only be 'C' or 'LLVM', got {jit}")
        self.jit = jit
        self.add_variables = types.MethodType(make_nd_variable, self)

        self.function_cppad_autodiff_graphs: dict[FunctionIndex, CppADAutodiffGraph] = (
            {}
        )
        self.function_autodiff_structures: dict[
            FunctionIndex, AutodiffSymbolicStructure
        ] = {}
        self.function_evaluators: dict[FunctionIndex, AutodiffEvaluator] = {}
        self.function_names: dict[FunctionIndex, str] = {}
        self.function_tracing_results: dict[FunctionIndex, FunctionTracingResult] = {}

    @staticmethod
    def supports_variable_attribute(attribute: VariableAttribute, settable=False):
        if settable:
            return attribute in variable_attribute_set_func_map
        else:
            return attribute in variable_attribute_get_func_map

    @staticmethod
    def supports_model_attribute(attribute: ModelAttribute, settable=False):
        if settable:
            return attribute in model_attribute_set_func_map
        else:
            return attribute in model_attribute_get_func_map

    @staticmethod
    def supports_constraint_attribute(attribute: ConstraintAttribute, settable=False):
        if settable:
            return attribute in constraint_attribute_set_func_map
        else:
            return attribute in constraint_attribute_get_func_map

    def optimize(self):
        if self.jit == "C":
            compile_functions_c(self, self.jit_compiler)
        elif self.jit == "LLVM":
            compile_functions_llvm(self, self.jit_compiler)
        super()._optimize()

    def register_function(
        self,
        f,
        vars: Optional[Vars] = None,
        params: Optional[Vars] = None,
        name: str = None,
    ):
        tracing_result = trace_function(f)
        cppad_function = cppad_trace_function(
            tracing_result.graph, tracing_result.results
        )

        autodiff_structure = AutodiffSymbolicStructure()
        cppad_graph = CppADAutodiffGraph()

        nx = cppad_function.nx
        if vars is not None:
            var_values = match_variable_values(tracing_result, vars.__dict__)
        else:
            var_values = [(i + 1) / (nx + 1) for i in range(nx)]
        np = cppad_function.np
        if params is not None:
            param_values = match_parameter_values(tracing_result, params.__dict__)
        else:
            param_values = [(i + 1) / (np + 1) for i in range(np)]

        cppad_autodiff(
            cppad_function, autodiff_structure, cppad_graph, var_values, param_values
        )

        function_index = super()._register_function(autodiff_structure)
        self.function_cppad_autodiff_graphs[function_index] = cppad_graph
        self.function_autodiff_structures[function_index] = autodiff_structure
        self.function_tracing_results[function_index] = tracing_result

        if name == None:
            name = f.__name__

        # if it is not a valid identifier, we generate our own name based on numbering
        if not name.isidentifier():
            name = f"nlfunc_{function_index.index}"
        else:
            name = f"nlfunc_{name}_{function_index.index}"

        self.function_names[function_index] = name

        return function_index

    def add_nl_constraint(
        self,
        function_index,
        vars: Vars,
        params: Optional[Params] = None,
        eq=None,
        lb=None,
        ub=None,
        name=None,
    ):
        tracing_result = self.function_tracing_results.get(function_index, None)
        if tracing_result is None:
            raise ValueError("Unregistered nonlinear function!")

        var_values = match_variable_values(tracing_result, vars.__dict__)
        if params is not None:
            param_values = match_parameter_values(tracing_result, params.__dict__)
            # if param is a double, we need to convert it to a ParameterIndex
            for i, param in enumerate(param_values):
                if isinstance(param, (int, float)):
                    param_values[i] = self.add_parameter(param)
        else:
            param_values = []
            np = tracing_result.n_parameters()
            if np > 0:
                raise ValueError(
                    "Missing parameters for parameterized nonlinear function"
                )

        ny = tracing_result.n_outputs()
        bounds_constraint = False
        eq_constraint = False
        if eq is not None:
            if lb is not None or ub is not None:
                raise ValueError("Cannot specify both equality and inequality bounds")
            eq_constraint = True

            if isinstance(eq, float):
                eq = [eq] * ny

            if len(eq) != ny:
                raise ValueError(
                    "Equality bounds must have the same length as the number of outputs, expects {ny}"
                )
        else:
            bounds_constraint = True
            if lb is None and ub is None:
                raise ValueError("Must specify either equality or inequality bounds")
            elif lb is None:
                lb = float("-inf")
            elif ub is None:
                ub = float("inf")

            if isinstance(lb, float):
                lb = [lb] * ny
            if isinstance(ub, float):
                ub = [ub] * ny

            if len(lb) != ny or len(ub) != ny:
                raise ValueError(
                    "Bounds must have the same length as the number of outputs, expects {ny}"
                )

        if bounds_constraint:
            constraint_index = super()._add_nl_constraint_bounds(
                function_index, var_values, param_values, lb, ub
            )
        else:
            constraint_index = super()._add_nl_constraint_eq(
                function_index, var_values, param_values, eq
            )

        return constraint_index

    def add_nl_objective(
        self, function_index, vars: Vars, params: Optional[Params] = None
    ):
        tracing_result = self.function_tracing_results.get(function_index, None)
        if tracing_result is None:
            raise ValueError("Unregistered nonlinear function!")

        var_values = match_variable_values(tracing_result, vars.__dict__)
        if params is not None:
            param_values = match_parameter_values(tracing_result, params.__dict__)
            # if param is a double, we need to convert it to a ParameterIndex
            for i, param in enumerate(param_values):
                if isinstance(param, (int, float)):
                    param_values[i] = self.add_parameter(param)
        else:
            param_values = []
            np = tracing_result.n_parameters()
            if np > 0:
                raise ValueError(
                    "Missing parameters for parameterized nonlinear function"
                )

        super()._add_nl_objective(function_index, var_values, param_values)

    def get_variable_attribute(self, variable, attribute: VariableAttribute):
        def e(attribute):
            raise ValueError(f"Unknown variable attribute to get: {attribute}")

        value = _direct_get_entity_attribute(
            self,
            variable,
            attribute,
            variable_attribute_get_func_map,
            e,
        )
        return value

    def set_variable_attribute(self, variable, attribute: VariableAttribute, value):
        def e(attribute):
            raise ValueError(f"Unknown variable attribute to set: {attribute}")

        _direct_set_entity_attribute(
            self,
            variable,
            attribute,
            value,
            variable_attribute_set_func_map,
            e,
        )

    def get_model_attribute(self, attribute: ModelAttribute):
        def e(attribute):
            raise ValueError(f"Unknown model attribute to get: {attribute}")

        value = _direct_get_model_attribute(
            self,
            attribute,
            model_attribute_get_func_map,
            e,
        )
        return value

    def set_model_attribute(self, attribute: ModelAttribute, value):
        def e(attribute):
            raise ValueError(f"Unknown model attribute to set: {attribute}")

        _direct_set_model_attribute(
            self,
            attribute,
            value,
            model_attribute_set_func_map,
            e,
        )

    def get_constraint_attribute(self, constraint, attribute: ConstraintAttribute):
        def e(attribute):
            raise ValueError(f"Unknown constraint attribute to get: {attribute}")

        value = _direct_get_entity_attribute(
            self,
            constraint,
            attribute,
            constraint_attribute_get_func_map,
            e,
        )
        return value

    def set_constraint_attribute(
        self, constraint, attribute: ConstraintAttribute, value
    ):
        def e(attribute):
            raise ValueError(f"Unknown constraint attribute to set: {attribute}")

        _direct_set_entity_attribute(
            self,
            constraint,
            attribute,
            value,
            constraint_attribute_set_func_map,
            e,
        )

    def set_raw_parameter(self, param_name: str, value):
        ty = type(value)
        if ty == int:
            self.set_raw_option_int(param_name, value)
        elif ty == float:
            self.set_raw_option_double(param_name, value)
        elif ty == str:
            self.set_raw_option_string(param_name, value)
        else:
            raise ValueError(f"Unsupported parameter type: {ty}")
