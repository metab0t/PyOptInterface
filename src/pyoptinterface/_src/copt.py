import os
import platform
import types
from pathlib import Path
import logging

from .copt_model_ext import RawModel, Env, COPT, load_library
from .attributes import (
    VariableAttribute,
    ConstraintAttribute,
    ModelAttribute,
    ResultStatusCode,
    TerminationStatusCode,
)
from .core_ext import ConstraintType, VariableIndex, ObjectiveSense
from .solver_common import (
    _direct_get_model_attribute,
    _direct_set_model_attribute,
    _direct_get_entity_attribute,
    _direct_set_entity_attribute,
)
from .aml import make_nd_variable


def detected_libraries():
    libs = []

    subdir = {
        "Linux": "lib",
        "Darwin": "lib",
        "Windows": "bin",
    }[platform.system()]
    libname = {
        "Linux": "libcopt.so",
        "Darwin": "libcopt.dylib",
        "Windows": "copt.dll",
    }[platform.system()]

    # Environment
    home = os.environ.get("COPT_HOME", None)
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
            logging.info(f"Loaded COPT library: {lib}")
            return True
    return False


autoload_library()

DEFAULT_ENV = None


def init_default_env():
    global DEFAULT_ENV
    if DEFAULT_ENV is None:
        DEFAULT_ENV = Env()


variable_attribute_get_func_map = {
    VariableAttribute.Value: lambda model, v: model.get_variable_info(v, "Value"),
    VariableAttribute.LowerBound: lambda model, v: model.get_variable_info(v, "LB"),
    VariableAttribute.UpperBound: lambda model, v: model.get_variable_info(v, "UB"),
    VariableAttribute.PrimalStart: lambda model, v: model.mip_start_values.get(v, None),
    VariableAttribute.Domain: lambda model, v: model.get_variable_type(v),
    VariableAttribute.Name: lambda model, v: model.get_variable_name(v),
}

variable_attribute_set_func_map = {
    VariableAttribute.LowerBound: lambda model, v, val: model.set_variable_lower_bound(
        v, val
    ),
    VariableAttribute.UpperBound: lambda model, v, val: model.set_variable_upper_bound(
        v, val
    ),
    VariableAttribute.PrimalStart: lambda model, v, val: model.mip_start_values.__setitem__(
        v, val
    ),
    VariableAttribute.Domain: lambda model, v, val: model.set_variable_type(v, val),
    VariableAttribute.Name: lambda model, v, val: model.set_variable_name(v, val),
}

constraint_type_attribute_name_map = {
    ConstraintType.Linear: "Rows",
    ConstraintType.Quadratic: "QConstrs",
}

copt_parameter_raw_type_map = {
    0: float,
    1: int,
}

copt_attribute_raw_type_map = {
    2: float,
    3: int,
}


def get_objsense(model):
    raw_sense = model.get_raw_attribute_int("ObjSense")
    if raw_sense == COPT.MINIMIZE:
        return ObjectiveSense.Minimize
    elif raw_sense == COPT.MAXIMIZE:
        return ObjectiveSense.Maximize
    else:
        raise ValueError(f"Unknown objective sense: {raw_sense}")


def get_objval(model):
    if model._is_mip():
        attr_name = "BestBnd"
    else:
        attr_name = "LpObjval"
    obj = model.get_raw_attribute_double(attr_name)
    return obj


def get_primalstatus(model):
    if model._is_mip():
        attr_name = "HasMipSol"
    else:
        attr_name = "HasLpSol"
    has_sol = model.get_raw_attribute_int(attr_name)
    if has_sol != 0:
        return ResultStatusCode.FEASIBLE_POINT
    else:
        return ResultStatusCode.NO_SOLUTION


def get_dualstatus(model):
    if not model._is_mip():
        has_sol = model.get_raw_attribute_int("HasLpSol")
        if has_sol != 0:
            return ResultStatusCode.FEASIBLE_POINT
    return ResultStatusCode.NO_SOLUTION


# LP status codes. Descriptions taken from COPT user guide.
# Code : (TerminationStatus, RawStatusString)
_RAW_LPSTATUS_STRINGS = {
    COPT.UNSTARTED: (
        TerminationStatusCode.OPTIMIZE_NOT_CALLED,
        "The LP optimization is not started yet.",
    ),
    COPT.OPTIMAL: (
        TerminationStatusCode.OPTIMAL,
        "The LP problem is solved to optimality.",
    ),
    COPT.INFEASIBLE: (
        TerminationStatusCode.INFEASIBLE,
        "The LP problem is infeasible.",
    ),
    COPT.UNBOUNDED: (
        TerminationStatusCode.DUAL_INFEASIBLE,
        "The LP problem is unbounded.",
    ),
    COPT.NUMERICAL: (
        TerminationStatusCode.NUMERICAL_ERROR,
        "Numerical trouble encountered.",
    ),
    COPT.IMPRECISE: (
        TerminationStatusCode.ALMOST_OPTIMAL,
        "The LP problem is solved to optimality with relaxed tolerances.",
    ),
    COPT.TIMEOUT: (
        TerminationStatusCode.TIME_LIMIT,
        "The LP optimization is stopped because of time limit.",
    ),
    COPT.UNFINISHED: (
        TerminationStatusCode.NUMERICAL_ERROR,
        "The LP optimization is stopped but the solver cannot provide a solution because of numerical difficulties.",
    ),
    COPT.INTERRUPTED: (
        TerminationStatusCode.INTERRUPTED,
        "The LP optimization is stopped by user interrupt.",
    ),
}

# MIP status codes. Descriptions taken from COPT user guide.
# Code : (TerminationStatus, RawStatusString)
_RAW_MIPSTATUS_STRINGS = {
    COPT.UNSTARTED: (
        TerminationStatusCode.OPTIMIZE_NOT_CALLED,
        "The MIP optimization is not started yet.",
    ),
    COPT.OPTIMAL: (
        TerminationStatusCode.OPTIMAL,
        "The MIP problem is solved to optimality.",
    ),
    COPT.INFEASIBLE: (
        TerminationStatusCode.INFEASIBLE,
        "The MIP problem is infeasible.",
    ),
    COPT.UNBOUNDED: (
        TerminationStatusCode.DUAL_INFEASIBLE,
        "The MIP problem is unbounded.",
    ),
    COPT.INF_OR_UNB: (
        TerminationStatusCode.INFEASIBLE_OR_UNBOUNDED,
        "The MIP problem is infeasible or unbounded.",
    ),
    COPT.NODELIMIT: (
        TerminationStatusCode.NODE_LIMIT,
        "The MIP optimization is stopped because of node limit.",
    ),
    COPT.TIMEOUT: (
        TerminationStatusCode.TIME_LIMIT,
        "The MIP optimization is stopped because of time limit.",
    ),
    COPT.UNFINISHED: (
        TerminationStatusCode.NUMERICAL_ERROR,
        "The MIP optimization is stopped but the solver cannot provide a solution because of numerical difficulties.",
    ),
    COPT.INTERRUPTED: (
        TerminationStatusCode.INTERRUPTED,
        "The MIP optimization is stopped by user interrupt.",
    ),
}


def get_terminationstatus(model):
    if model._is_mip():
        raw_status = model.get_raw_attribute_int("MipStatus")
        status_string_pair = _RAW_MIPSTATUS_STRINGS.get(raw_status, None)
    else:
        raw_status = model.get_raw_attribute_int("LpStatus")
        status_string_pair = _RAW_LPSTATUS_STRINGS.get(raw_status, None)
    if not status_string_pair:
        raise ValueError(f"Unknown termination status: {raw_status}")
    return status_string_pair[0]


def get_rawstatusstring(model):
    if model._is_mip():
        raw_status = model.get_raw_attribute_int("MipStatus")
        status_string_pair = _RAW_MIPSTATUS_STRINGS.get(raw_status, None)
    else:
        raw_status = model.get_raw_attribute_int("LpStatus")
        status_string_pair = _RAW_LPSTATUS_STRINGS.get(raw_status, None)
    if not status_string_pair:
        raise ValueError(f"Unknown termination status: {raw_status}")
    return status_string_pair[1]


def get_silent(model):
    return model.get_raw_parameter_int("LogToConsole") == 0


model_attribute_get_func_map = {
    ModelAttribute.ObjectiveSense: get_objsense,
    ModelAttribute.BarrierIterations: lambda model: model.get_model_raw_attribute_int(
        "BarrierIter"
    ),
    ModelAttribute.DualObjectiveValue: get_objval,
    ModelAttribute.NodeCount: lambda model: model.get_model_raw_attribute_int(
        "NodeCnt"
    ),
    ModelAttribute.ObjectiveBound: get_objval,
    ModelAttribute.ObjectiveValue: get_objval,
    ModelAttribute.SimplexIterations: lambda model: model.get_model_raw_attribute_int(
        "SimplexIter"
    ),
    ModelAttribute.SolveTimeSec: lambda model: model.get_raw_parameter_double(
        "SolvingTime"
    ),
    ModelAttribute.NumberOfThreads: lambda model: model.get_raw_parameter_int(
        "Threads"
    ),
    ModelAttribute.RelativeGap: lambda model: model.get_raw_parameter_double("RelGap"),
    ModelAttribute.TimeLimitSec: lambda model: model.get_raw_parameter_double(
        "TimeLimit"
    ),
    ModelAttribute.DualStatus: get_dualstatus,
    ModelAttribute.PrimalStatus: get_primalstatus,
    ModelAttribute.RawStatusString: get_rawstatusstring,
    ModelAttribute.TerminationStatus: get_terminationstatus,
    ModelAttribute.Silent: get_silent,
    ModelAttribute.SolverName: lambda _: "COPT",
    ModelAttribute.SolverVersion: lambda model: model.version_string(),
}


def set_silent(model, value: bool):
    if value:
        model.set_raw_parameter_int("LogToConsole", 0)
    else:
        model.set_raw_parameter_int("LogToConsole", 1)


model_attribute_set_func_map = {
    ModelAttribute.ObjectiveSense: lambda model, v: model.set_obj_sense(v),
    ModelAttribute.NumberOfThreads: lambda model, v: model.set_raw_parameter_int(
        "Threads", v
    ),
    ModelAttribute.RelativeGap: lambda model, v: model.set_raw_parameter_double(
        "RelGap", v
    ),
    ModelAttribute.TimeLimitSec: lambda model, v: model.set_raw_parameter_double(
        "TimeLimit", v
    ),
    ModelAttribute.Silent: set_silent,
}

constraint_attribute_get_func_map = {
    ConstraintAttribute.Name: lambda model, constraint: model.get_constraint_name(
        constraint
    ),
    ConstraintAttribute.Primal: lambda model, constraint: model.get_constraint_info(
        constraint, "Slack"
    ),
    ConstraintAttribute.Dual: lambda model, constraint: model.get_constraint_info(
        constraint, "Dual"
    ),
}

constraint_attribute_set_func_map = {
    ConstraintAttribute.Name: lambda model, constraint, value: model.set_constraint_name(
        constraint, value
    ),
}

callback_info_typemap = {
    "BestObj": float,
    "BestBnd": float,
    "HasIncumbent": int,
    "MipCandObj": float,
    "RelaxSolObj": float,
    "NodeStatus": int,
}


class Model(RawModel):
    def __init__(self, env=None):
        if env is None:
            init_default_env()
            env = DEFAULT_ENV
        super().__init__(env)

        # We must keep a reference to the environment to prevent it from being garbage collected
        self._env = env
        self.mip_start_values: dict[VariableIndex, float] = dict()

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
        attr_name = constraint_type_attribute_name_map.get(type, None)
        if not attr_name:
            raise ValueError(f"Unknown constraint type: {type}")
        return self.get_raw_attribute_int(attr_name)

    def number_of_variables(self):
        return self.get_raw_attribute_int("Cols")

    def _is_mip(self):
        ismip = self.get_raw_attribute_int("IsMIP")
        return ismip > 0

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

    def get_raw_parameter(self, param_name: str):
        param_type = copt_parameter_raw_type_map[
            self.raw_parameter_attribute_type(param_name)
        ]
        get_function_map = {
            int: self.get_raw_parameter_int,
            float: self.get_raw_parameter_double,
        }
        get_function = get_function_map[param_type]
        return get_function(param_name)

    def set_raw_parameter(self, param_name: str, value):
        param_type = copt_parameter_raw_type_map[
            self.raw_parameter_attribute_type(param_name)
        ]
        set_function_map = {
            int: self.set_raw_parameter_int,
            float: self.set_raw_parameter_double,
        }
        set_function = set_function_map[param_type]
        set_function(param_name, value)

    def get_raw_attribute(self, param_name: str):
        param_type = copt_attribute_raw_type_map[
            self.raw_parameter_attribute_type(param_name)
        ]
        get_function_map = {
            int: self.get_raw_attribute_int,
            float: self.get_raw_attribute_double,
        }
        get_function = get_function_map[param_type]
        return get_function(param_name)

    def optimize(self):
        if self._is_mip():
            mip_start = self.mip_start_values
            if len(mip_start) != 0:
                variables = list(mip_start.keys())
                values = list(mip_start.values())
                self.add_mip_start(variables, values)
                mip_start.clear()
        super().optimize()

    def cb_get_info(self, what):
        cb_info_type = callback_info_typemap.get(what, None)
        if cb_info_type is None:
            raise ValueError(f"Unknown callback info type: {what}")
        if cb_info_type == int:
            return self.cb_get_info_int(what)
        elif cb_info_type == float:
            return self.cb_get_info_double(what)
        else:
            raise ValueError(f"Unknown callback info type: {what}")
