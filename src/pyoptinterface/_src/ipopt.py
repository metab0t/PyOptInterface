from io import StringIO
import logging
import platform
from typing import Optional, List, Dict, Set, Union, Tuple, overload

from llvmlite import ir

from .ipopt_model_ext import RawModel, ApplicationReturnStatus, load_library
from .codegen_c import generate_csrc_prelude, generate_csrc_from_graph
from .jit_c import TCCJITCompiler
from .codegen_llvm import create_llvmir_basic_functions, generate_llvmir_from_graph
from .jit_llvm import LLJITCompiler
from .nlexpr_ext import ExpressionHandle, ExpressionGraph, unpack_comparison_expression
from .nlfunc import (
    ExpressionGraphContext,
    convert_to_expressionhandle,
)

from .core_ext import (
    VariableIndex,
    ScalarAffineFunction,
    ScalarQuadraticFunction,
    ExprBuilder,
    ConstraintSense,
)
from .comparison_constraint import ComparisonConstraint
from .nleval_ext import (
    AutodiffSymbolicStructure,
    ConstraintAutodiffEvaluator,
    ObjectiveAutodiffEvaluator,
)
from .cppad_interface_ext import (
    CppADAutodiffGraph,
    cppad_trace_graph_constraints,
    cppad_trace_graph_objective,
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
from .constraint_bridge import bridge_soc_quadratic_constraint
from .aml import make_variable_tupledict, make_variable_ndarray
from .matrix import add_matrix_constraints


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
    ModelAttribute.Silent: lambda model, v: model.set_raw_parameter(
        "print_level", 0 if v else 5
    ),
}


def get_constraint_primal(model, constraint):
    return model.get_constraint_primal(constraint)


def get_constraint_dual(model, constraint):
    return model.get_constraint_dual(constraint)


constraint_attribute_get_func_map = {
    ConstraintAttribute.Primal: get_constraint_primal,
    ConstraintAttribute.Dual: get_constraint_dual,
}

constraint_attribute_set_func_map = {}


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

        # store graph_instance to graph_index
        self.graph_instance_to_index: Dict[ExpressionGraph, int] = {}
        self.graph_instances: List[ExpressionGraph] = []

        self.nl_constraint_group_num = 0
        self.nl_constraint_group_representatives: List[int] = []
        self.nl_constraint_cppad_autodiff_graphs: List[CppADAutodiffGraph] = []
        self.nl_constraint_autodiff_structures: List[AutodiffSymbolicStructure] = []
        self.nl_constraint_evaluators: List[ConstraintAutodiffEvaluator] = []

        self.nl_objective_group_num = 0
        self.nl_objective_group_representatives: List[int] = []
        self.nl_objective_cppad_autodiff_graphs: List[CppADAutodiffGraph] = []
        self.nl_objective_autodiff_structures: List[AutodiffSymbolicStructure] = []
        self.nl_objective_evaluators: List[ObjectiveAutodiffEvaluator] = []

        # record the analyzed part of the problem
        self.n_graph_instances_since_last_optimize = 0
        self.nl_constraint_group_num_since_last_optimize = 0
        self.nl_objective_group_num_since_last_optimize = 0

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
        if ty is int:
            self.set_raw_option_int(param_name, value)
        elif ty is float:
            self.set_raw_option_double(param_name, value)
        elif ty is str:
            self.set_raw_option_string(param_name, value)
        else:
            raise ValueError(f"Unsupported parameter type: {ty}")

    @overload
    def add_linear_constraint(
        self,
        expr: Union[VariableIndex, ScalarAffineFunction, ExprBuilder],
        sense: ConstraintSense,
        rhs: float,
        name: str = "",
    ): ...

    @overload
    def add_linear_constraint(
        self,
        expr: Union[VariableIndex, ScalarAffineFunction, ExprBuilder],
        interval: Tuple[float, float],
        name: str = "",
    ): ...

    @overload
    def add_linear_constraint(
        self,
        con: ComparisonConstraint,
        name: str = "",
    ): ...

    def add_linear_constraint(self, arg, *args, **kwargs):
        if isinstance(arg, ComparisonConstraint):
            return self._add_linear_constraint(
                arg.lhs, arg.sense, arg.rhs, *args, **kwargs
            )
        else:
            return self._add_linear_constraint(arg, *args, **kwargs)

    @overload
    def add_quadratic_constraint(
        self,
        expr: Union[ScalarQuadraticFunction, ExprBuilder],
        sense: ConstraintSense,
        rhs: float,
        name: str = "",
    ): ...

    @overload
    def add_quadratic_constraint(
        self,
        expr: Union[ScalarQuadraticFunction, ExprBuilder],
        interval: Tuple[float, float],
        name: str = "",
    ): ...

    @overload
    def add_quadratic_constraint(
        self,
        con: ComparisonConstraint,
        name: str = "",
    ): ...

    def add_quadratic_constraint(self, arg, *args, **kwargs):
        if isinstance(arg, ComparisonConstraint):
            return self._add_quadratic_constraint(
                arg.lhs, arg.sense, arg.rhs, *args, **kwargs
            )
        else:
            return self._add_quadratic_constraint(arg, *args, **kwargs)

    @overload
    def add_nl_constraint(
        self,
        expr,
        sense: ConstraintSense,
        rhs: float,
        /,
        name: str = "",
    ): ...

    @overload
    def add_nl_constraint(
        self,
        expr,
        interval: Tuple[float, float],
        /,
        name: str = "",
    ): ...

    @overload
    def add_nl_constraint(
        self,
        con,
        /,
        name: str = "",
    ): ...

    def add_nl_constraint(self, expr, *args, **kwargs):
        graph = ExpressionGraphContext.current_graph()
        expr = convert_to_expressionhandle(graph, expr)
        if not isinstance(expr, ExpressionHandle):
            raise ValueError(
                "Expression should be able to be converted to ExpressionHandle"
            )

        n_args = len(args)

        if n_args == 0:
            is_comparison = graph.is_compare_expression(expr)
            if is_comparison:
                expr, lb, ub = unpack_comparison_expression(graph, expr, float("inf"))
            else:
                raise ValueError("Must specify either equality or inequality bounds")
        elif n_args == 1:
            arg = args[0]
            if isinstance(arg, tuple):
                lb, ub = arg
            else:
                raise ValueError("Must specify either equality or inequality bounds")
        elif n_args == 2:
            sense = args[0]
            rhs = args[1]
            if isinstance(sense, ConstraintSense) and isinstance(rhs, float):
                if sense == ConstraintSense.Equal:
                    lb = ub = rhs
                elif sense == ConstraintSense.LessEqual:
                    lb = -float("inf")
                    ub = rhs
                elif sense == ConstraintSense.GreaterEqual:
                    lb = rhs
                    ub = float("inf")
                else:
                    raise ValueError(f"Unknown constraint sense: {sense}")
            else:
                raise ValueError("Must specify either equality or inequality bounds")
        else:
            raise ValueError("Must specify either equality or inequality bounds")

        graph.add_constraint_output(expr)

        graph_index = self.graph_instance_to_index.get(graph, None)
        if graph_index is None:
            graph_index = self._add_graph_index()
            self.graph_instance_to_index[graph] = graph_index
            self.graph_instances.append(graph)

        con = self._add_single_nl_constraint(graph_index, graph, lb, ub)

        return con

    def add_nl_objective(self, expr):
        graph = ExpressionGraphContext.current_graph()
        expr = convert_to_expressionhandle(graph, expr)
        if not isinstance(expr, ExpressionHandle):
            raise ValueError(
                "Expression should be able to be converted to ExpressionHandle"
            )

        graph.add_objective_output(expr)

        graph_index = self.graph_instance_to_index.get(graph, None)
        if graph_index is None:
            graph_index = self._add_graph_index()
            self.graph_instance_to_index[graph] = graph_index
            self.graph_instances.append(graph)

    def optimize(self):
        self._find_similar_graphs()
        self._compile_evaluators()
        # print("Compiling evaluators successfully")
        # print(self.jit_compiler.source_codes[0])

        self.n_graph_instances_since_last_optimize = len(self.graph_instances)
        self.nl_constraint_group_num_since_last_optimize = self.nl_constraint_group_num
        self.nl_objective_group_num_since_last_optimize = self.nl_objective_group_num

        super()._optimize()

    def _find_similar_graphs(self):
        for i in range(
            self.n_graph_instances_since_last_optimize, len(self.graph_instances)
        ):
            graph = self.graph_instances[i]
            self._finalize_graph_instance(i, graph)

        # constraint part

        n_groups = self._aggregate_nl_constraint_groups()
        # print(f"Found {n_groups} nonlinear constraint groups of similar graphs")
        self.nl_constraint_group_num = n_groups

        rep_instances = self.nl_constraint_group_representatives

        for i in range(self.nl_constraint_group_num_since_last_optimize, n_groups):
            graph_index = self._get_nl_constraint_group_representative(i)
            rep_instances.append(graph_index)

        # objective part
        n_groups = self._aggregate_nl_objective_groups()
        # print(f"Found {n_groups} nonlinear objective groups of similar graphs")
        self.nl_objective_group_num = n_groups

        rep_instances = self.nl_objective_group_representatives

        for i in range(self.nl_objective_group_num_since_last_optimize, n_groups):
            graph_index = self._get_nl_objective_group_representative(i)
            rep_instances.append(graph_index)

    def _compile_evaluators(self):
        # for each group of nonlinear constraint and objective, we construct a cppad_autodiff graph
        # and then compile them to get the function pointers

        # constraint
        # self.nl_constraint_cppad_autodiff_graphs.clear()
        # self.nl_constraint_autodiff_structures.clear()
        for i in range(
            self.nl_constraint_group_num_since_last_optimize,
            self.nl_constraint_group_num,
        ):
            graph_index = self.nl_constraint_group_representatives[i]
            graph = self.graph_instances[graph_index]

            # print(graph)

            cppad_function = cppad_trace_graph_constraints(graph)

            nx = cppad_function.nx
            var_values = [(i + 1) / (nx + 1) for i in range(nx)]
            np = cppad_function.np
            param_values = [(i + 1) / (np + 1) for i in range(np)]

            # print(f"nx = {nx}, np = {np}")
            # print(f"var_values = {var_values}")
            # print(f"param_values = {param_values}")

            autodiff_structure = AutodiffSymbolicStructure()
            cppad_graph = CppADAutodiffGraph()

            cppad_autodiff(
                cppad_function,
                autodiff_structure,
                cppad_graph,
                var_values,
                param_values,
            )

            # print(cppad_graph.f)

            self._assign_nl_constraint_group_autodiff_structure(i, autodiff_structure)

            self.nl_constraint_cppad_autodiff_graphs.append(cppad_graph)
            self.nl_constraint_autodiff_structures.append(autodiff_structure)

        # objective
        # self.nl_objective_cppad_autodiff_graphs.clear()
        # self.nl_objective_autodiff_structures.clear()
        for i in range(
            self.nl_objective_group_num_since_last_optimize, self.nl_objective_group_num
        ):
            graph_index = self.nl_objective_group_representatives[i]
            graph = self.graph_instances[graph_index]

            cppad_function = cppad_trace_graph_objective(graph)

            nx = cppad_function.nx
            var_values = [(i + 1) / (nx + 1) for i in range(nx)]
            np = cppad_function.np
            param_values = [(i + 1) / (np + 1) for i in range(np)]

            autodiff_structure = AutodiffSymbolicStructure()
            cppad_graph = CppADAutodiffGraph()

            cppad_autodiff(
                cppad_function,
                autodiff_structure,
                cppad_graph,
                var_values,
                param_values,
            )

            self._assign_nl_objective_group_autodiff_structure(i, autodiff_structure)

            self.nl_objective_cppad_autodiff_graphs.append(cppad_graph)
            self.nl_objective_autodiff_structures.append(autodiff_structure)

        # compile the evaluators
        jit_compiler = self.jit_compiler
        if isinstance(jit_compiler, TCCJITCompiler):
            self._codegen_c()
        elif isinstance(jit_compiler, LLJITCompiler):
            self._codegen_llvm()

    def _codegen_c(self):
        jit_compiler: TCCJITCompiler = self.jit_compiler
        io = StringIO()

        generate_csrc_prelude(io)

        for group_index in range(
            self.nl_constraint_group_num_since_last_optimize,
            self.nl_constraint_group_num,
        ):
            cppad_autodiff_graph = self.nl_constraint_cppad_autodiff_graphs[group_index]
            autodiff_structure = self.nl_constraint_autodiff_structures[group_index]

            np = autodiff_structure.np
            ny = autodiff_structure.ny

            name = f"nlconstraint_{group_index}"

            f_name = name
            generate_csrc_from_graph(
                io,
                cppad_autodiff_graph.f,
                f_name,
                np=np,
                indirect_x=True,
            )
            if autodiff_structure.has_jacobian:
                jacobian_name = name + "_jacobian"
                generate_csrc_from_graph(
                    io,
                    cppad_autodiff_graph.jacobian,
                    jacobian_name,
                    np=np,
                    indirect_x=True,
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
                    indirect_y=True,
                    add_y=True,
                )

        for group_index in range(
            self.nl_objective_group_num_since_last_optimize, self.nl_objective_group_num
        ):
            cppad_autodiff_graph = self.nl_objective_cppad_autodiff_graphs[group_index]
            autodiff_structure = self.nl_objective_autodiff_structures[group_index]

            np = autodiff_structure.np
            ny = autodiff_structure.ny

            name = f"nlobjective_{group_index}"

            f_name = name
            generate_csrc_from_graph(
                io, cppad_autodiff_graph.f, f_name, np=np, indirect_x=True, add_y=True
            )
            if autodiff_structure.has_jacobian:
                jacobian_name = name + "_jacobian"
                generate_csrc_from_graph(
                    io,
                    cppad_autodiff_graph.jacobian,
                    jacobian_name,
                    np=np,
                    indirect_x=True,
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
                    indirect_y=True,
                    add_y=True,
                )

        csrc = io.getvalue()

        inst = jit_compiler.create_instance()
        jit_compiler.compile_string(inst, csrc)

        for group_index in range(
            self.nl_constraint_group_num_since_last_optimize,
            self.nl_constraint_group_num,
        ):
            name = f"nlconstraint_{group_index}"
            autodiff_structure = self.nl_constraint_autodiff_structures[group_index]
            has_parameter = autodiff_structure.has_parameter

            f_name = name
            jacobian_name = name + "_jacobian"
            hessian_name = name + "_hessian"

            f_ptr = inst.get_symbol(f_name)
            jacobian_ptr = hessian_ptr = 0
            if autodiff_structure.has_jacobian:
                jacobian_ptr = inst.get_symbol(jacobian_name)
            if autodiff_structure.has_hessian:
                hessian_ptr = inst.get_symbol(hessian_name)

            evaluator = ConstraintAutodiffEvaluator(
                has_parameter, f_ptr, jacobian_ptr, hessian_ptr
            )
            self._assign_nl_constraint_group_autodiff_evaluator(group_index, evaluator)

        for group_index in range(
            self.nl_objective_group_num_since_last_optimize, self.nl_objective_group_num
        ):
            name = f"nlobjective_{group_index}"
            autodiff_structure = self.nl_objective_autodiff_structures[group_index]
            has_parameter = autodiff_structure.has_parameter

            f_name = name
            jacobian_name = name + "_jacobian"
            hessian_name = name + "_hessian"

            f_ptr = inst.get_symbol(f_name)
            jacobian_ptr = hessian_ptr = 0
            if autodiff_structure.has_jacobian:
                jacobian_ptr = inst.get_symbol(jacobian_name)
            if autodiff_structure.has_hessian:
                hessian_ptr = inst.get_symbol(hessian_name)

            evaluator = ObjectiveAutodiffEvaluator(
                has_parameter, f_ptr, jacobian_ptr, hessian_ptr
            )
            self._assign_nl_objective_group_autodiff_evaluator(group_index, evaluator)

    def _codegen_llvm(self):
        jit_compiler: LLJITCompiler = self.jit_compiler
        module = ir.Module(name="my_module")
        create_llvmir_basic_functions(module)

        export_functions = []

        for group_index in range(
            self.nl_constraint_group_num_since_last_optimize,
            self.nl_constraint_group_num,
        ):
            cppad_autodiff_graph = self.nl_constraint_cppad_autodiff_graphs[group_index]
            autodiff_structure = self.nl_constraint_autodiff_structures[group_index]

            np = autodiff_structure.np
            ny = autodiff_structure.ny

            name = f"nlconstraint_{group_index}"

            f_name = name
            generate_llvmir_from_graph(
                module,
                cppad_autodiff_graph.f,
                f_name,
                np=np,
                indirect_x=True,
            )
            export_functions.append(f_name)
            if autodiff_structure.has_jacobian:
                jacobian_name = name + "_jacobian"
                generate_llvmir_from_graph(
                    module,
                    cppad_autodiff_graph.jacobian,
                    jacobian_name,
                    np=np,
                    indirect_x=True,
                )
                export_functions.append(jacobian_name)
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
                    indirect_y=True,
                    add_y=True,
                )
                export_functions.append(hessian_name)

        for group_index in range(
            self.nl_objective_group_num_since_last_optimize, self.nl_objective_group_num
        ):
            cppad_autodiff_graph = self.nl_objective_cppad_autodiff_graphs[group_index]
            autodiff_structure = self.nl_objective_autodiff_structures[group_index]

            np = autodiff_structure.np
            ny = autodiff_structure.ny

            name = f"nlobjective_{group_index}"

            f_name = name
            generate_llvmir_from_graph(
                module,
                cppad_autodiff_graph.f,
                f_name,
                np=np,
                indirect_x=True,
                add_y=True,
            )
            export_functions.append(f_name)
            if autodiff_structure.has_jacobian:
                jacobian_name = name + "_jacobian"
                generate_llvmir_from_graph(
                    module,
                    cppad_autodiff_graph.jacobian,
                    jacobian_name,
                    np=np,
                    indirect_x=True,
                    indirect_y=True,
                    add_y=True,
                )
                export_functions.append(jacobian_name)
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
                    indirect_y=True,
                    add_y=True,
                )
                export_functions.append(hessian_name)

        rt = jit_compiler.compile_module(module, export_functions)

        for group_index in range(
            self.nl_constraint_group_num_since_last_optimize,
            self.nl_constraint_group_num,
        ):
            name = f"nlconstraint_{group_index}"
            autodiff_structure = self.nl_constraint_autodiff_structures[group_index]
            has_parameter = autodiff_structure.has_parameter

            f_name = name
            jacobian_name = name + "_jacobian"
            hessian_name = name + "_hessian"

            f_ptr = rt[f_name]
            jacobian_ptr = hessian_ptr = 0
            if autodiff_structure.has_jacobian:
                jacobian_ptr = rt[jacobian_name]
            if autodiff_structure.has_hessian:
                hessian_ptr = rt[hessian_name]

            evaluator = ConstraintAutodiffEvaluator(
                has_parameter, f_ptr, jacobian_ptr, hessian_ptr
            )
            self._assign_nl_constraint_group_autodiff_evaluator(group_index, evaluator)

        for group_index in range(
            self.nl_objective_group_num_since_last_optimize, self.nl_objective_group_num
        ):
            name = f"nlobjective_{group_index}"
            autodiff_structure = self.nl_objective_autodiff_structures[group_index]
            has_parameter = autodiff_structure.has_parameter

            f_name = name
            jacobian_name = name + "_jacobian"
            hessian_name = name + "_hessian"

            f_ptr = rt[f_name]
            jacobian_ptr = hessian_ptr = 0
            if autodiff_structure.has_jacobian:
                jacobian_ptr = rt[jacobian_name]
            if autodiff_structure.has_hessian:
                hessian_ptr = rt[hessian_name]

            evaluator = ObjectiveAutodiffEvaluator(
                has_parameter, f_ptr, jacobian_ptr, hessian_ptr
            )
            self._assign_nl_objective_group_autodiff_evaluator(group_index, evaluator)


Model.add_variables = make_variable_tupledict
Model.add_m_variables = make_variable_ndarray
Model.add_m_linear_constraints = add_matrix_constraints
Model.add_second_order_cone_constraint = bridge_soc_quadratic_constraint
