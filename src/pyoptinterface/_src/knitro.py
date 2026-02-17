import logging
import os
import platform
import re
from pathlib import Path
from typing import Tuple, Union, overload

from .aml import make_variable_ndarray, make_variable_tupledict
from .attributes import (
    ConstraintAttribute,
    ModelAttribute,
    ResultStatusCode,
    TerminationStatusCode,
    VariableAttribute,
)
from .comparison_constraint import ComparisonConstraint
from .core_ext import (
    ConstraintIndex,
    ConstraintSense,
    ExprBuilder,
    ScalarAffineFunction,
    ScalarQuadraticFunction,
    VariableIndex,
)
from .knitro_model_ext import KN, RawEnv, RawModel, load_library
from .matrix import add_matrix_constraints
from .nlexpr_ext import ExpressionGraph, ExpressionHandle
from .nlfunc import ExpressionGraphContext, convert_to_expressionhandle
from .solver_common import (
    _get_entity_attribute,
    _get_model_attribute,
    _set_entity_attribute,
    _set_model_attribute,
)


def detected_libraries():
    libs = []

    subdir = {
        "Linux": "lib",
        "Darwin": "lib",
        "Windows": "bin",
    }[platform.system()]
    libname_pattern = {
        "Linux": r"libknitro\.so.*",
        "Darwin": r"libknitro\.dylib.*",
        "Windows": r"knitro\d*\.dll",
    }[platform.system()]
    suffix_pattern = {
        "Linux": "*.so*",
        "Darwin": "*.dylib*",
        "Windows": "*.dll",
    }[platform.system()]

    # Environment variable
    knitro_dir = os.environ.get("KNITRODIR", None)
    if knitro_dir and os.path.exists(knitro_dir):
        dir = Path(knitro_dir) / subdir
        if dir.exists():
            for path in dir.glob(suffix_pattern):
                match = re.match(libname_pattern, path.name)
                if match:
                    libs.append(str(path))

    try:
        import knitro

        dir = Path(knitro.__path__[0])
        dir = dir / "lib"
        for path in dir.glob(suffix_pattern):
            match = re.match(libname_pattern, path.name)
            if match:
                libs.append(str(path))
    except ImportError:
        pass

    # Default library names
    default_libname = {
        "Linux": ["libknitro.so"],
        "Darwin": ["libknitro.dylib"],
        "Windows": ["knitro.dll"],
    }[platform.system()]
    libs.extend(default_libname)

    return libs


def autoload_library():
    libs = detected_libraries()
    for lib in libs:
        ret = load_library(lib)
        if ret:
            logging.info(f"Loaded KNITRO library: {lib}")
            return True
    return False


autoload_library()


# Variable Attribute
variable_attribute_get_func_map = {
    VariableAttribute.Value: lambda model, v: model.get_value(v),
    VariableAttribute.LowerBound: lambda model, v: model.get_variable_lb(v),
    VariableAttribute.UpperBound: lambda model, v: model.get_variable_ub(v),
    VariableAttribute.Name: lambda model, v: model.get_variable_name(v),
    VariableAttribute.ReducedCost: lambda model, v: model.get_variable_rc(v),
}

variable_attribute_set_func_map = {
    VariableAttribute.LowerBound: lambda model, v, x: model.set_variable_lb(v, x),
    VariableAttribute.UpperBound: lambda model, v, x: model.set_variable_ub(v, x),
    VariableAttribute.PrimalStart: lambda model, v, x: model.set_variable_start(v, x),
    VariableAttribute.Name: lambda model, v, x: model.set_variable_name(v, x),
    VariableAttribute.Domain: lambda model, v, x: model.set_variable_domain(v, x),
}

# Constraint Attribute
constraint_attribute_get_func_map = {
    ConstraintAttribute.Primal: lambda model, c: model.get_constraint_primal(c),
    ConstraintAttribute.Dual: lambda model, c: model.get_constraint_dual(c),
    ConstraintAttribute.Name: lambda model, c: model.get_constraint_name(c),
}

constraint_attribute_set_func_map = {
    ConstraintAttribute.Name: lambda model, c, x: model.set_constraint_name(c, x),
}

_RAW_STATUS_STRINGS = [
    (TerminationStatusCode.OPTIMAL, KN.RC_OPTIMAL),
    (TerminationStatusCode.OPTIMAL, KN.RC_OPTIMAL_OR_SATISFACTORY),
    (TerminationStatusCode.ALMOST_OPTIMAL, KN.RC_NEAR_OPT),
    (TerminationStatusCode.ALMOST_OPTIMAL, KN.RC_FEAS_XTOL),
    (TerminationStatusCode.ALMOST_OPTIMAL, KN.RC_FEAS_NO_IMPROVE),
    (TerminationStatusCode.ALMOST_OPTIMAL, KN.RC_FEAS_FTOL),
    (TerminationStatusCode.LOCALLY_SOLVED, KN.RC_FEAS_BEST),
    (TerminationStatusCode.LOCALLY_SOLVED, KN.RC_FEAS_MULTISTART),
    (TerminationStatusCode.INFEASIBLE, KN.RC_INFEASIBLE),
    (TerminationStatusCode.LOCALLY_INFEASIBLE, KN.RC_INFEAS_XTOL),
    (TerminationStatusCode.LOCALLY_INFEASIBLE, KN.RC_INFEAS_NO_IMPROVE),
    (TerminationStatusCode.LOCALLY_INFEASIBLE, KN.RC_INFEAS_MULTISTART),
    (TerminationStatusCode.INFEASIBLE, KN.RC_INFEAS_CON_BOUNDS),
    (TerminationStatusCode.INFEASIBLE, KN.RC_INFEAS_VAR_BOUNDS),
    (TerminationStatusCode.DUAL_INFEASIBLE, KN.RC_UNBOUNDED),
    (TerminationStatusCode.INFEASIBLE_OR_UNBOUNDED, KN.RC_UNBOUNDED_OR_INFEAS),
    (TerminationStatusCode.ITERATION_LIMIT, KN.RC_ITER_LIMIT_FEAS),
    (TerminationStatusCode.ITERATION_LIMIT, KN.RC_ITER_LIMIT_INFEAS),
    (TerminationStatusCode.TIME_LIMIT, KN.RC_TIME_LIMIT_FEAS),
    (TerminationStatusCode.TIME_LIMIT, KN.RC_TIME_LIMIT_INFEAS),
    (TerminationStatusCode.OTHER_LIMIT, KN.RC_FEVAL_LIMIT_FEAS),
    (TerminationStatusCode.OTHER_LIMIT, KN.RC_FEVAL_LIMIT_INFEAS),
    (TerminationStatusCode.OTHER_LIMIT, KN.RC_MIP_EXH_FEAS),
    (TerminationStatusCode.OTHER_LIMIT, KN.RC_MIP_EXH_INFEAS),
    (TerminationStatusCode.NODE_LIMIT, KN.RC_MIP_NODE_LIMIT_FEAS),
    (TerminationStatusCode.NODE_LIMIT, KN.RC_MIP_NODE_LIMIT_INFEAS),
    (TerminationStatusCode.INTERRUPTED, KN.RC_USER_TERMINATION),
    (TerminationStatusCode.NUMERICAL_ERROR, KN.RC_EVAL_ERR),
    (TerminationStatusCode.MEMORY_LIMIT, KN.RC_OUT_OF_MEMORY),
    (TerminationStatusCode.OTHER_ERROR, KN.RC_CALLBACK_ERR),
    (TerminationStatusCode.OTHER_ERROR, KN.RC_LP_SOLVER_ERR),
    (TerminationStatusCode.OTHER_ERROR, KN.RC_LINEAR_SOLVER_ERR),
    (TerminationStatusCode.OTHER_ERROR, KN.RC_INTERNAL_ERROR),
]


def _termination_status_knitro(model: "Model"):
    if model.is_dirty:
        return TerminationStatusCode.OPTIMIZE_NOT_CALLED

    code = model.solve_status
    for ts, rs in _RAW_STATUS_STRINGS:
        if code == rs:
            return ts
    return TerminationStatusCode.OTHER_ERROR


def _result_status_knitro(model: "Model"):
    if model.is_dirty:
        return ResultStatusCode.NO_SOLUTION

    code = model.solve_status

    feasible = {
        KN.RC_OPTIMAL,
        KN.RC_OPTIMAL_OR_SATISFACTORY,
        KN.RC_NEAR_OPT,
        KN.RC_FEAS_XTOL,
        KN.RC_FEAS_NO_IMPROVE,
        KN.RC_FEAS_FTOL,
        KN.RC_FEAS_BEST,
        KN.RC_FEAS_MULTISTART,
        KN.RC_ITER_LIMIT_FEAS,
        KN.RC_TIME_LIMIT_FEAS,
        KN.RC_FEVAL_LIMIT_FEAS,
        KN.RC_MIP_EXH_FEAS,
        KN.RC_MIP_TERM_FEAS,
        KN.RC_MIP_SOLVE_LIMIT_FEAS,
        KN.RC_MIP_NODE_LIMIT_FEAS,
    }

    infeasible = {
        KN.RC_INFEASIBLE,
        KN.RC_INFEAS_XTOL,
        KN.RC_INFEAS_NO_IMPROVE,
        KN.RC_INFEAS_MULTISTART,
        KN.RC_INFEAS_CON_BOUNDS,
        KN.RC_INFEAS_VAR_BOUNDS,
        KN.RC_ITER_LIMIT_INFEAS,
        KN.RC_TIME_LIMIT_INFEAS,
        KN.RC_FEVAL_LIMIT_INFEAS,
        KN.RC_MIP_EXH_INFEAS,
        KN.RC_MIP_SOLVE_LIMIT_INFEAS,
        KN.RC_MIP_NODE_LIMIT_INFEAS,
    }

    if code in feasible:
        return ResultStatusCode.FEASIBLE_POINT
    if code in infeasible:
        return ResultStatusCode.INFEASIBLE_POINT
    return ResultStatusCode.NO_SOLUTION


# Model Attribute
model_attribute_get_func_map = {
    ModelAttribute.ObjectiveValue: lambda model: model.get_obj_value(),
    ModelAttribute.ObjectiveSense: lambda model: model.get_obj_sense(),
    ModelAttribute.TerminationStatus: _termination_status_knitro,
    ModelAttribute.RawStatusString: lambda model: (
        f"KNITRO status code: {model.solve_status}"
    ),
    ModelAttribute.PrimalStatus: _result_status_knitro,
    ModelAttribute.NumberOfThreads: lambda model: model.get_raw_parameter(
        KN.PARAM_NUMTHREADS
    ),
    ModelAttribute.TimeLimitSec: lambda model: model.get_raw_parameter(
        KN.PARAM_MAXTIME
    ),
    ModelAttribute.BarrierIterations: lambda model: model.get_number_iterations(),
    ModelAttribute.NodeCount: lambda model: model.get_mip_node_count(),
    ModelAttribute.ObjectiveBound: lambda model: model.get_obj_bound(),
    ModelAttribute.RelativeGap: lambda model: model.get_mip_relative_gap(),
    ModelAttribute.SolverName: lambda model: model.get_solver_name(),
    ModelAttribute.SolverVersion: lambda model: model.get_release(),
    ModelAttribute.SolveTimeSec: lambda model: model.get_solve_time(),
}

model_attribute_set_func_map = {
    ModelAttribute.ObjectiveSense: lambda model, x: model.set_obj_sense(x),
    ModelAttribute.NumberOfThreads: lambda model, x: model.set_raw_parameter(
        KN.PARAM_NUMTHREADS, x
    ),
    ModelAttribute.Silent: lambda model, x: model.set_raw_parameter(
        KN.PARAM_OUTLEV, KN.OUTLEV_NONE if x else KN.OUTLEV_ITER_10
    ),
    ModelAttribute.TimeLimitSec: lambda model, x: model.set_raw_parameter(
        KN.PARAM_MAXTIME, x
    ),
}


class Env(RawEnv):
    """
    KNITRO license manager environment.
    """

    @property
    def is_empty(self):
        return self.empty()


class Model(RawModel):
    """
    KNITRO model class for PyOptInterface.
    """

    def __init__(self, env: Env = None) -> None:
        if env is not None:
            super().__init__(env)
        else:
            super().__init__()
        self.graph_map: dict[ExpressionGraph, int] = {}

    def _reset_graph_map(self) -> None:
        self.graph_map.clear()

    def _add_graph_expr(
        self, expr: ExpressionHandle
    ) -> tuple[ExpressionGraph, ExpressionHandle]:
        graph = ExpressionGraphContext.current_graph()
        expr = convert_to_expressionhandle(graph, expr)
        if not isinstance(expr, ExpressionHandle):
            raise ValueError("Expression should be convertible to ExpressionHandle")
        if graph not in self.graph_map:
            self.graph_map[graph] = len(self.graph_map)
        return graph, expr

    def init(self, env: Env = None) -> None:
        if env is not None:
            super().init(env)
        else:
            super().init()
        self._reset_graph_map()

    def close(self) -> None:
        super().close()
        self._reset_graph_map()

    @staticmethod
    def supports_variable_attribute(
        attribute: VariableAttribute, setable: bool = False
    ) -> bool:
        if setable:
            return attribute in variable_attribute_set_func_map
        else:
            return attribute in variable_attribute_get_func_map

    @staticmethod
    def supports_constraint_attribute(
        attribute: ConstraintAttribute, setable: bool = False
    ) -> bool:
        if setable:
            return attribute in constraint_attribute_set_func_map
        else:
            return attribute in constraint_attribute_get_func_map

    @staticmethod
    def supports_model_attribute(
        attribute: ModelAttribute, setable: bool = False
    ) -> bool:
        if setable:
            return attribute in model_attribute_set_func_map
        else:
            return attribute in model_attribute_get_func_map

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
    ) -> ConstraintIndex: ...

    @overload
    def add_linear_constraint(
        self,
        con: ComparisonConstraint,
        name: str = "",
    ) -> ConstraintIndex: ...

    def add_linear_constraint(self, arg, *args, **kwargs) -> ConstraintIndex:
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
    ) -> ConstraintIndex: ...

    @overload
    def add_quadratic_constraint(
        self,
        con: ComparisonConstraint,
        name: str = "",
    ) -> ConstraintIndex: ...

    def add_quadratic_constraint(self, arg, *args, **kwargs) -> ConstraintIndex:
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
    ) -> ConstraintIndex: ...

    @overload
    def add_nl_constraint(
        self,
        expr,
        interval: Tuple[float, float],
        /,
        name: str = "",
    ) -> ConstraintIndex: ...

    @overload
    def add_nl_constraint(
        self,
        con,
        /,
        name: str = "",
    ) -> ConstraintIndex: ...

    def add_nl_constraint(self, expr, *args, **kwargs) -> ConstraintIndex:
        graph, expr = self._add_graph_expr(expr)
        return self._add_single_nl_constraint(graph, expr, *args, **kwargs)

    def add_nl_objective(self, expr) -> None:
        graph, expr = self._add_graph_expr(expr)
        self._add_single_nl_objective(graph, expr)

    def get_model_attribute(self, attr: ModelAttribute):
        def e(attribute):
            raise ValueError(f"Unknown model attribute to get: {attribute}")

        return _get_model_attribute(self, attr, model_attribute_get_func_map, {}, e)

    def set_model_attribute(self, attr: ModelAttribute, value):
        def e(attribute):
            raise ValueError(f"Unknown model attribute to set: {attribute}")

        _set_model_attribute(self, attr, value, model_attribute_set_func_map, {}, e)

    def get_variable_attribute(self, variable: VariableIndex, attr: VariableAttribute):
        def e(attribute):
            raise ValueError(f"Unknown variable attribute to get: {attribute}")

        return _get_entity_attribute(
            self,
            variable,
            attr,
            variable_attribute_get_func_map,
            {},
            e,
        )

    def set_variable_attribute(
        self, variable: VariableIndex, attr: VariableAttribute, value
    ):
        def e(attribute):
            raise ValueError(f"Unknown variable attribute to set: {attribute}")

        _set_entity_attribute(
            self,
            variable,
            attr,
            value,
            variable_attribute_set_func_map,
            {},
            e,
        )

    def get_constraint_attribute(
        self, constraint: ConstraintIndex, attr: ConstraintAttribute
    ):
        def e(attribute):
            raise ValueError(f"Unknown constraint attribute to get: {attribute}")

        return _get_entity_attribute(
            self,
            constraint,
            attr,
            constraint_attribute_get_func_map,
            {},
            e,
        )

    def set_constraint_attribute(
        self, constraint: ConstraintIndex, attr: ConstraintAttribute, value
    ):
        def e(attribute):
            raise ValueError(f"Unknown constraint attribute to set: {attribute}")

        _set_entity_attribute(
            self,
            constraint,
            attr,
            value,
            constraint_attribute_set_func_map,
            {},
            e,
        )

    @property
    def is_dirty(self) -> bool:
        return self.dirty()

    @property
    def is_empty(self) -> bool:
        return self.empty() and not self.graph_map

    @property
    def solve_status(self) -> int:
        return self.get_solve_status()


Model.add_variables = make_variable_tupledict
Model.add_m_variables = make_variable_ndarray
Model.add_m_linear_constraints = add_matrix_constraints
