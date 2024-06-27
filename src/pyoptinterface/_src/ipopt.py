from io import StringIO
import types
import logging
import platform

from llvmlite import ir

from .ipopt_model_ext import RawModel, ApplicationReturnStatus, load_library
from .codegen_c import generate_csrc_prelude, generate_csrc_from_graph
from .jit_c import TCCJITCompiler
from .codegen_llvm import create_llvmir_basic_functions, generate_llvmir_from_graph
from .jit_llvm import LLJITCompiler
from .tracefun import trace_adfun

from .core_ext import ConstraintIndex
from .nlcore_ext import NLConstraintIndex, initialize_cpp_graph_operator_info

initialize_cpp_graph_operator_info()

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
        "Darwin": ["libpopt.dylib"],
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


def compile_functions_c(backend: RawModel, jit_compiler: TCCJITCompiler):
    io = StringIO()

    generate_csrc_prelude(io)

    functions = backend.m_function_model.nl_functions

    for function in functions:
        name = function.name

        f_name = name
        generate_csrc_from_graph(
            io,
            function.f_graph,
            f_name,
            np=function.np,
            indirect_x=True,
            indirect_p=True,
        )
        if function.has_jacobian:
            jacobian_name = name + "_jacobian"
            generate_csrc_from_graph(
                io,
                function.jacobian_graph,
                jacobian_name,
                np=function.np,
                indirect_x=True,
                indirect_p=True,
            )
            gradient_name = name + "_gradient"
            generate_csrc_from_graph(
                io,
                function.jacobian_graph,
                gradient_name,
                np=function.np,
                indirect_x=True,
                indirect_p=True,
                indirect_y=True,
                add_y=True,
            )
        if function.has_hessian:
            hessian_name = name + "_hessian"
            generate_csrc_from_graph(
                io,
                function.hessian_graph,
                hessian_name,
                np=function.np,
                hessian_lagrange=True,
                nw=function.ny,
                indirect_x=True,
                indirect_p=True,
                indirect_y=True,
                add_y=True,
            )

    csrc = io.getvalue()

    jit_compiler.source_code = csrc

    jit_compiler.compile_string(csrc.encode())

    for function in functions:
        name = function.name
        f_name = name
        jacobian_name = name + "_jacobian"
        gradient_name = name + "_gradient"
        hessian_name = name + "_hessian"

        f_ptr = jit_compiler.get_symbol(f_name.encode())
        jacobian_ptr = gradient_ptr = hessian_ptr = 0
        if function.has_jacobian:
            jacobian_ptr = jit_compiler.get_symbol(jacobian_name.encode())
            gradient_ptr = jit_compiler.get_symbol(gradient_name.encode())
        if function.has_hessian:
            hessian_ptr = jit_compiler.get_symbol(hessian_name.encode())

        function.assign_evaluators(f_ptr, jacobian_ptr, gradient_ptr, hessian_ptr)


def compile_functions_llvm(backend: RawModel, jit_compiler: LLJITCompiler):
    module = ir.Module(name="my_module")
    create_llvmir_basic_functions(module)

    functions = backend.m_function_model.nl_functions

    export_functions = []

    for function in functions:
        name = function.name

        f_name = name
        generate_llvmir_from_graph(
            module,
            function.f_graph,
            f_name,
            np=function.np,
            indirect_x=True,
            indirect_p=True,
        )
        if function.has_jacobian:
            jacobian_name = name + "_jacobian"
            generate_llvmir_from_graph(
                module,
                function.jacobian_graph,
                jacobian_name,
                np=function.np,
                indirect_x=True,
                indirect_p=True,
            )
            gradient_name = name + "_gradient"
            generate_llvmir_from_graph(
                module,
                function.jacobian_graph,
                gradient_name,
                np=function.np,
                indirect_x=True,
                indirect_p=True,
                indirect_y=True,
                add_y=True,
            )
        if function.has_hessian:
            hessian_name = name + "_hessian"
            generate_llvmir_from_graph(
                module,
                function.hessian_graph,
                hessian_name,
                np=function.np,
                hessian_lagrange=True,
                nw=function.ny,
                indirect_x=True,
                indirect_p=True,
                indirect_y=True,
                add_y=True,
            )

        export_functions.extend([f_name, jacobian_name, gradient_name, hessian_name])

    jit_compiler.compile_module(module, export_functions)

    for function in functions:
        name = function.name
        f_name = name
        jacobian_name = name + "_jacobian"
        gradient_name = name + "_gradient"
        hessian_name = name + "_hessian"

        f_ptr = jit_compiler.get_symbol(f_name)
        jacobian_ptr = gradient_ptr = hessian_ptr = 0
        if function.has_jacobian:
            jacobian_ptr = jit_compiler.get_symbol(jacobian_name)
            gradient_ptr = jit_compiler.get_symbol(gradient_name)
        if function.has_hessian:
            hessian_ptr = jit_compiler.get_symbol(hessian_name)

        function.assign_evaluators(f_ptr, jacobian_ptr, gradient_ptr, hessian_ptr)


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


class Model(RawModel):
    def __init__(self):
        super().__init__()

        self.n_nl_functions = 0
        self.jit_compiler = None
        self.add_variables = types.MethodType(make_nd_variable, self)

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

    def optimize(self, jit_engine="LLVM"):
        if jit_engine == "C":
            self.jit_compiler = TCCJITCompiler()
            compile_functions_c(self, self.jit_compiler)
        elif jit_engine == "LLVM":
            self.jit_compiler = LLJITCompiler()
            compile_functions_llvm(self, self.jit_compiler)
        super()._optimize()

    def register_function(
        self, f, /, var, param=(), var_values=None, param_values=None, name=None
    ):
        adfun = trace_adfun(f, var, param)
        nx = adfun.nx
        if var_values is not None:
            assert len(var_values) == nx
        else:
            var_values = [(i + 1) / (nx + 1) for i in range(nx)]
        np = adfun.np
        if param_values is not None:
            assert len(param_values) == np
        else:
            param_values = [(i + 1) / (np + 1) for i in range(np)]
        if name is None:
            name = f"nlfunction_{self.n_nl_functions}"
        self.n_nl_functions += 1
        return super()._register_function(adfun, name, var_values, param_values)

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
