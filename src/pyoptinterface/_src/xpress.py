import os
import platform
from pathlib import Path
import logging
from typing import Dict, Tuple, Union, overload

from .xpress_model_ext import RawModel, Env, load_library, XPRS
from .attributes import (
    VariableAttribute,
    ConstraintAttribute,
    ModelAttribute,
    ResultStatusCode,
    TerminationStatusCode,
)
from .core_ext import (
    VariableIndex,
    ScalarAffineFunction,
    ScalarQuadraticFunction,
    ExprBuilder,
    VariableDomain,
    ConstraintType,
    ConstraintSense,
    ObjectiveSense,
)
from .nlexpr_ext import ExpressionHandle
from .nlfunc import ExpressionGraphContext, convert_to_expressionhandle
from .comparison_constraint import ComparisonConstraint
from .solver_common import (
    _get_model_attribute,
    _set_model_attribute,
    _get_entity_attribute,
    _direct_get_entity_attribute,
    _set_entity_attribute,
    _direct_set_entity_attribute,
)

from .aml import make_variable_tupledict, make_variable_ndarray
from .matrix import add_matrix_constraints


def detected_libraries():
    libs = []

    subdir = {
        "Linux": "lib",
        "Darwin": "lib",
        "Windows": "bin",
    }[platform.system()]
    libname = {
        "Linux": "libxprs.so",
        "Darwin": "libxprs.dylib",
        "Windows": "xprs.dll",
    }[platform.system()]

    # Environment
    home = os.environ.get("XPRESSDIR", None)
    if home and os.path.exists(home):
        lib = Path(home) / subdir / libname
        if lib.exists():
            libs.append(str(lib))

    # default names
    default_libname = libname
    libs.append(default_libname)

    return libs


def autoload_library():
    libs = detected_libraries()
    for lib in libs:
        ret = load_library(lib)
        if ret:
            logging.info(f"Loaded Xpress library: {lib}")
            return True
    return False


autoload_library()


# LP status codes (TerminationStatus, RawStatusString)
_RAW_LPSTATUS_STRINGS = {
    XPRS.LPSTATUS.UNSTARTED: (
        TerminationStatusCode.OPTIMIZE_NOT_CALLED,
        "LP problem optimization not started",
    ),
    XPRS.LPSTATUS.OPTIMAL: (
        TerminationStatusCode.OPTIMAL,
        "Optimal LP solution found",
    ),
    XPRS.LPSTATUS.INFEAS: (
        TerminationStatusCode.INFEASIBLE,
        "Infeasible LP problem",
    ),
    XPRS.LPSTATUS.CUTOFF: (
        TerminationStatusCode.OBJECTIVE_LIMIT,
        "LP problem objective worse than cutoff value",
    ),
    XPRS.LPSTATUS.UNFINISHED: (
        TerminationStatusCode.ITERATION_LIMIT,
        "LP problem optimization unfinished",
    ),
    XPRS.LPSTATUS.UNBOUNDED: (
        TerminationStatusCode.DUAL_INFEASIBLE,
        "LP problem is unbounded",
    ),
    XPRS.LPSTATUS.CUTOFF_IN_DUAL: (
        TerminationStatusCode.OBJECTIVE_LIMIT,
        "LP dual bound is worse than dual cutoff value",
    ),
    XPRS.LPSTATUS.UNSOLVED: (
        TerminationStatusCode.NUMERICAL_ERROR,
        "LP problem could not be solved due to numerical issues",
    ),
    XPRS.LPSTATUS.NONCONVEX: (
        TerminationStatusCode.INVALID_MODEL,
        "LP problem contains quadratic data which is not convex, consider using FICO Xpress Global",
    ),
}

# MIP status codes (TerminationStatus, RawStatusString)
_RAW_MIPSTATUS_STRINGS = {
    XPRS.MIPSTATUS.NOT_LOADED: (
        TerminationStatusCode.OPTIMIZE_NOT_CALLED,
        "MIP problem has not been loaded",
    ),
    XPRS.MIPSTATUS.LP_NOT_OPTIMAL: (
        TerminationStatusCode.ITERATION_LIMIT,
        "MIP search incomplete, the initial continuous relaxation has not been solved and no integer solution has been found",
    ),
    XPRS.MIPSTATUS.LP_OPTIMAL: (
        TerminationStatusCode.ITERATION_LIMIT,
        "MIP search incomplete, the initial continuous relaxation has been solved and no integer solution has been found",
    ),
    XPRS.MIPSTATUS.NO_SOL_FOUND: (
        TerminationStatusCode.ITERATION_LIMIT,
        "MIP search incomplete, no integer solution found",
    ),
    XPRS.MIPSTATUS.SOLUTION: (
        TerminationStatusCode.ITERATION_LIMIT,
        "MIP search incomplete, an integer solution has been found",
    ),
    XPRS.MIPSTATUS.INFEAS: (
        TerminationStatusCode.INFEASIBLE,
        "MIP search complete, MIP is infeasible, no integer solution found",
    ),
    XPRS.MIPSTATUS.OPTIMAL: (
        TerminationStatusCode.OPTIMAL,
        "MIP search complete, optimal integer solution found",
    ),
    XPRS.MIPSTATUS.UNBOUNDED: (
        TerminationStatusCode.DUAL_INFEASIBLE,
        "MIP search incomplete, the initial continuous relaxation was found to be unbounded. A solution may have been found",
    ),
}

# NLP status codes (TerminationStatus, RawStatusString)
_RAW_NLPSTATUS_STRINGS = {
    XPRS.NLPSTATUS.UNSTARTED: (
        TerminationStatusCode.OPTIMIZE_NOT_CALLED,
        "Optimization unstarted",
    ),
    XPRS.NLPSTATUS.SOLUTION: (
        TerminationStatusCode.LOCALLY_SOLVED,
        "Solution found",
    ),
    XPRS.NLPSTATUS.OPTIMAL: (
        TerminationStatusCode.OPTIMAL,
        "Globally optimal",
    ),
    XPRS.NLPSTATUS.NOSOLUTION: (
        TerminationStatusCode.ITERATION_LIMIT,
        "No solution found",
    ),
    XPRS.NLPSTATUS.INFEASIBLE: (
        TerminationStatusCode.INFEASIBLE,
        "Proven infeasible",
    ),
    XPRS.NLPSTATUS.UNBOUNDED: (
        TerminationStatusCode.DUAL_INFEASIBLE,
        "Locally unbounded",
    ),
    XPRS.NLPSTATUS.UNFINISHED: (
        TerminationStatusCode.ITERATION_LIMIT,
        "Not yet solved to completion",
    ),
    XPRS.NLPSTATUS.UNSOLVED: (
        TerminationStatusCode.NUMERICAL_ERROR,
        "Could not be solved due to numerical issues",
    ),
}


def get_terminationstatus(model):
    opt_type = model.get_optimize_type()

    if opt_type == XPRS.OPTIMIZETYPE.LP:
        raw_status = model.get_lp_status()
        status_string_pair = _RAW_LPSTATUS_STRINGS.get(raw_status, None)
    elif opt_type == XPRS.OPTIMIZETYPE.MIP:
        raw_status = model.get_mip_status()
        status_string_pair = _RAW_MIPSTATUS_STRINGS.get(raw_status, None)
    else:  # NLP or LOCAL
        raw_status = model.get_nlp_status()
        status_string_pair = _RAW_NLPSTATUS_STRINGS.get(raw_status, None)

    if not status_string_pair:
        raise ValueError(f"Unknown termination status: {raw_status}")
    return status_string_pair[0]


def get_primalstatus(model):
    opt_type = model.get_optimize_type()

    if opt_type == XPRS.OPTIMIZETYPE.LP:
        status = model.get_lp_status()
        if status == XPRS.LPSTATUS.OPTIMAL:
            return ResultStatusCode.FEASIBLE_POINT
        return ResultStatusCode.NO_SOLUTION

    elif opt_type == XPRS.OPTIMIZETYPE.MIP:
        status = model.get_mip_status()
        if model.get_raw_attribute_int_by_id(XPRS.MIPSOLS) > 0:
            return ResultStatusCode.FEASIBLE_POINT
        return ResultStatusCode.NO_SOLUTION

    else:  # NLP or LOCAL
        status = model.get_nlp_status()
        if status in (
            XPRS.NLPSTATUS.OPTIMAL,
            XPRS.NLPSTATUS.SOLUTION,
            XPRS.NLPSTATUS.UNBOUNDED,
        ):
            return ResultStatusCode.FEASIBLE_POINT
        return ResultStatusCode.NO_SOLUTION


def get_dualstatus(model):
    opt_type = model.get_optimize_type()
    if opt_type != XPRS.OPTIMIZETYPE.LP:
        return ResultStatusCode.NO_SOLUTION

    status = model.get_lp_status()
    if status == XPRS.LPSTATUS.OPTIMAL:
        return ResultStatusCode.FEASIBLE_POINT
    return ResultStatusCode.NO_SOLUTION


def get_rawstatusstring(model):
    opt_type = model.get_optimize_type()

    if opt_type == XPRS.OPTIMIZETYPE.LP:
        raw_status = model.get_lp_status()
        status_string_pair = _RAW_LPSTATUS_STRINGS.get(raw_status, None)
    elif opt_type == XPRS.OPTIMIZETYPE.MIP:
        raw_status = model.get_mip_status()
        status_string_pair = _RAW_MIPSTATUS_STRINGS.get(raw_status, None)
    else:  # NLP or LOCAL
        raw_status = model.get_nlp_status()
        status_string_pair = _RAW_NLPSTATUS_STRINGS.get(raw_status, None)

    if not status_string_pair:
        raise ValueError(f"Unknown termination status: {raw_status}")
    return status_string_pair[1]


# Variable maps
variable_attribute_get_func_map = {  # UB, LB, Name, etc
    VariableAttribute.Value: lambda model, v: model.get_variable_value(v),
    VariableAttribute.LowerBound: lambda model, v: model.get_variable_lowerbound(v),
    VariableAttribute.UpperBound: lambda model, v: model.get_variable_upperbound(v),
    VariableAttribute.PrimalStart: lambda model, v: model.get_variable_mip_start(v),
    VariableAttribute.Domain: lambda model, v: model.get_variable_type(v),
    VariableAttribute.Name: lambda model, v: model.get_variable_name(v),
    VariableAttribute.IISLowerBound: lambda model, v: model.get_variable_lowerbound_IIS(
        v
    ),
    VariableAttribute.IISUpperBound: lambda model, v: model.get_variable_upperbound_IIS(
        v
    ),
    VariableAttribute.ReducedCost: lambda model, v: model.get_variable_rc(v),
}

variable_attribute_set_func_map = (
    {  # Subset of the previous one about stuff that can be set
        VariableAttribute.LowerBound: lambda model, v, x: model.set_variable_lowerbound(
            v, x
        ),
        VariableAttribute.UpperBound: lambda model, v, x: model.set_variable_upperbound(
            v, x
        ),
        VariableAttribute.PrimalStart: lambda model, v, x: model.set_variable_mip_start(
            v, x
        ),
        VariableAttribute.Domain: lambda model, v, x: model.set_variable_type(v, x),
        VariableAttribute.Name: lambda model, v, x: model.set_variable_name(v, x),
    }
)

constraint_attribute_get_func_map = {
    ConstraintAttribute.Name: lambda model, c: model.get_constraint_name(c),
    ConstraintAttribute.Primal: lambda model, c: model.get_normalized_rhs(c)
    - model.get_constraint_slack(c),
    ConstraintAttribute.Dual: lambda model, c: model.get_constraint_dual(c),
    ConstraintAttribute.IIS: lambda model, c: model.is_constraint_in_IIS(c),
}

constraint_attribute_set_func_map = {
    ConstraintAttribute.Name: lambda model, constraint, value: model.set_constraint_name(
        constraint, value
    ),
}

model_attribute_get_func_map = {
    ModelAttribute.Name: lambda model: model.get_problem_name(),
    ModelAttribute.ObjectiveSense: lambda model: model.get_raw_attribute_dbl_by_id(
        XPRS.OBJSENSE
    ),
    ModelAttribute.BarrierIterations: lambda model: model.get_raw_attribute_int_by_id(
        XPRS.BARITER
    ),
    ModelAttribute.DualObjectiveValue: lambda model: model.get_raw_attribute_dbl_by_id(
        XPRS.LPOBJVAL
    ),
    ModelAttribute.NodeCount: lambda model: model.get_raw_attribute_int_by_id(
        XPRS.NODES
    ),
    ModelAttribute.ObjectiveBound: lambda model: model.get_raw_attribute_dbl_by_id(
        XPRS.LPOBJVAL
    ),
    ModelAttribute.ObjectiveValue: lambda model: model.get_raw_attribute_dbl_by_id(
        XPRS.OBJVAL
    ),
    ModelAttribute.SimplexIterations: lambda model: model.get_raw_attribute_int_by_id(
        XPRS.SIMPLEXITER
    ),
    ModelAttribute.SolveTimeSec: lambda model: model.get_raw_attribute_dbl_by_id(
        XPRS.TIME
    ),
    ModelAttribute.NumberOfThreads: lambda model: model.get_raw_control_int(
        XPRS.THREADS
    ),
    ModelAttribute.RelativeGap: lambda model: model.get_raw_control_dbl_by_id(
        XPRS.MIPRELSTOP
    ),
    ModelAttribute.TimeLimitSec: lambda model: model.get_raw_contorl_dbl_by_id(
        XPRS.TIMELIMIT
    ),
    ModelAttribute.DualStatus: get_dualstatus,
    ModelAttribute.PrimalStatus: get_primalstatus,
    ModelAttribute.RawStatusString: get_rawstatusstring,
    ModelAttribute.TerminationStatus: get_terminationstatus,
    ModelAttribute.Silent: lambda model: model.get_raw_control_int_by_id(XPRS.OUTPUTLOG)
    == 0,
    ModelAttribute.SolverName: lambda _: "FICO Xpress",
    ModelAttribute.SolverVersion: lambda model: model.version_string(),
}

model_control_set_func_map = {
    ModelAttribute.Name: lambda model, value: model.set_problem_name(value),
    ModelAttribute.ObjectiveSense: lambda model, value: model.set_raw_control_dbl_by_id(
        XPRS.OBJSENSE, value
    ),
    ModelAttribute.NumberOfThreads: lambda model, value: model.set_raw_control_int_by_id(
        XPRS.THREADS, value
    ),
    ModelAttribute.TimeLimitSec: lambda model, value: model.set_raw_control_dbl_by_id(
        XPRS.TIMELIMIT, value
    ),
    ModelAttribute.Silent: lambda model, value: model.set_raw_control_int_by_id(
        XPRS.OUTPUTLOG, 0 if value else 1
    ),
}

model_attribute_get_translate_func_map = {
    ModelAttribute.ObjectiveSense: lambda v: {
        XPRS.OBJ_MINIMIZE: ObjectiveSense.Minimize,
        XPRS.OBJ_MAXIMIZE: ObjectiveSense.Maximize,
    }[v],
}

model_attribute_set_translate_func_map = {
    ModelAttribute.ObjectiveSense: lambda v: {
        ObjectiveSense.Minimize: XPRS.OBJ_MINIMIZE,
        ObjectiveSense.Maximize: XPRS.OBJ_MAXIMIZE,
    }[v],
}

DEFAULT_ENV = None


def init_default_env():
    global DEFAULT_ENV
    if DEFAULT_ENV is None:
        DEFAULT_ENV = Env()


class Model(RawModel):
    def __init__(self, env=None):
        # Initializing with raw model object
        if isinstance(env, RawModel):
            super().__init__(env)
            return

        if env is None:
            init_default_env()
            env = DEFAULT_ENV
        super().__init__(env)
        self.mip_start_values: Dict[VariableIndex, float] = dict()

    def optimize(self):
        if self._is_mip():
            mip_start = self.mip_start_values
            if len(mip_start) != 0:
                variables = list(mip_start.keys())
                values = list(mip_start.values())
                self.add_mip_start(variables, values)
                mip_start.clear()
        super().optimize()

    @staticmethod
    def supports_variable_attribute(attribute: VariableAttribute, settable=False):
        if settable:
            return attribute in variable_attribute_set_func_map
        else:
            return attribute in variable_attribute_get_func_map

    @staticmethod
    def supports_model_attribute(attribute: ModelAttribute, settable=False):
        if settable:
            return attribute in model_control_set_func_map
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

    def number_of_constraints(self, type: ConstraintType):
        if type in {ConstraintType.Linear, ConstraintType.Quadratic}:
            return self.get_raw_attribute_int_by_id(XPRS.ROWS)
        if type == ConstraintType.SOS:
            return self.get_raw_attribute_int_by_id(XPRS.SETS)
        raise ValueError(f"Unknown constraint type: {type}")

    def number_of_variables(self):
        return self.get_raw_attribute_int_by_id(XPRS.INPUTCOLS)

    def get_model_attribute(self, attribute: ModelAttribute):
        def e(attribute):
            raise ValueError(f"Unknown model attribute to get: {attribute}")

        value = _get_model_attribute(
            self,
            attribute,
            model_attribute_get_func_map,
            model_attribute_get_translate_func_map,
            e,
        )
        return value

    def set_model_attribute(self, attribute: ModelAttribute, value):
        def e(attribute):
            raise ValueError(f"Unknown model attribute to set: {attribute}")

        _set_model_attribute(
            self,
            attribute,
            value,
            model_control_set_func_map,
            model_attribute_set_translate_func_map,
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
            raise ValueError("Expression should be convertible to ExpressionHandle")

        con = self._add_single_nl_constraint(graph, expr, *args, **kwargs)
        return con

    def add_nl_objective(self, expr):
        graph = ExpressionGraphContext.current_graph()
        expr = convert_to_expressionhandle(graph, expr)
        if not isinstance(expr, ExpressionHandle):
            raise ValueError("Expression should be convertible to ExpressionHandle")
        self._add_single_nl_objective(graph, expr)

    def set_callback(self, cb, where):
        def cb_wrapper(raw_model, ctx):
            # Warning: This is super hacky. We need to provide a complete Model
            # object to the callback (a RawModel is not enough). Xpress invokes
            # callbacks with thread-local problem pointers, so we reuse the
            # original object by swapping the pointers temporarily. This is
            # okay because we've serialized access to the model anyway (GIL
            # limitation). But all of this happens at the C++ level, here we
            # only need to provide the user callback with the original complete
            # Model object. So it looks like we're giving the original model to
            # the callbacks, but in reality we pull a switcheroo behind the
            # curtains.
            cb(self, ctx)

        super().set_callback(cb_wrapper, where)

    add_variables = make_variable_tupledict
    add_m_variables = make_variable_ndarray
    add_m_linear_constraints = add_matrix_constraints
