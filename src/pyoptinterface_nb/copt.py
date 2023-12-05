import os, platform

if platform.system() == "Windows":
    copt_home = os.environ.get("COPT_HOME", None)
    if not copt_home:
        raise ValueError("COPT_HOME is not set in environment")
    if not os.path.exists(copt_home):
        raise ValueError(f"COPT_HOME does not exist: {copt_home}")
    os.add_dll_directory(os.path.join(copt_home, "bin"))

try:
    from .copt_model_ext import RawModel, Env, Constants
except Exception as e:
    raise ImportError(
        f"Failed to import copt_model_ext. Please check if COPT_HOME is set correctly. Error: {e}"
    )

from .attributes import (
    VariableAttribute,
    ConstraintAttribute,
    ModelAttribute,
    var_attr_type_map,
    default_variable_attribute_type,
    ResultStatusCode,
    TerminationStatusCode,
)
from .core_ext import VariableDomain, ConstraintType, VariableIndex
from .ctypes_helper import pycapsule_to_cvoidp

DEFAULT_ENV = None


def init_default_env():
    global DEFAULT_ENV
    if DEFAULT_ENV is None:
        DEFAULT_ENV = Env()


constraint_type_attribute_name_map = {
    ConstraintType.Linear: "Rows",
    ConstraintType.Quadratic: "QConstrs",
}


def get_objsense(model):
    raw_sense = model.get_raw_attribute_int("ObjSense")
    if raw_sense == Constants.COPT_MINIMIZE:
        return ObjectiveSense.Minimize
    elif raw_sense == Constants.COPT_MAXIMIZE:
        return ObjectiveSense.Maximize
    else:
        raise ValueError(f"Unknown objective sense: {raw_sense}")


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
    Constants.COPT_LPSTATUS_UNSTARTED: (
        TerminationStatusCode.OPTIMIZE_NOT_CALLED,
        "The LP optimization is not started yet.",
    ),
    Constants.COPT_LPSTATUS_OPTIMAL: (
        TerminationStatusCode.OPTIMAL,
        "The LP problem is solved to optimality.",
    ),
    Constants.COPT_LPSTATUS_INFEASIBLE: (
        TerminationStatusCode.INFEASIBLE,
        "The LP problem is infeasible.",
    ),
    Constants.COPT_LPSTATUS_UNBOUNDED: (
        TerminationStatusCode.DUAL_INFEASIBLE,
        "The LP problem is unbounded.",
    ),
    Constants.COPT_LPSTATUS_NUMERICAL: (
        TerminationStatusCode.NUMERICAL_ERROR,
        "Numerical trouble encountered.",
    ),
    Constants.COPT_LPSTATUS_IMPRECISE: (
        TerminationStatusCode.ALMOST_OPTIMAL,
        "The LP problem is solved to optimality with relaxed tolerances.",
    ),
    Constants.COPT_LPSTATUS_TIMEOUT: (
        TerminationStatusCode.TIME_LIMIT,
        "The LP optimization is stopped because of time limit.",
    ),
    Constants.COPT_LPSTATUS_UNFINISHED: (
        TerminationStatusCode.NUMERICAL_ERROR,
        "The LP optimization is stopped but the solver cannot provide a solution because of numerical difficulties.",
    ),
    Constants.COPT_LPSTATUS_INTERRUPTED: (
        TerminationStatusCode.INTERRUPTED,
        "The LP optimization is stopped by user interrupt.",
    ),
}

# MIP status codes. Descriptions taken from COPT user guide.
# Code : (TerminationStatus, RawStatusString)
_RAW_MIPSTATUS_STRINGS = {
    Constants.COPT_MIPSTATUS_UNSTARTED: (
        TerminationStatusCode.OPTIMIZE_NOT_CALLED,
        "The MIP optimization is not started yet.",
    ),
    Constants.COPT_MIPSTATUS_OPTIMAL: (
        TerminationStatusCode.OPTIMAL,
        "The MIP problem is solved to optimality.",
    ),
    Constants.COPT_MIPSTATUS_INFEASIBLE: (
        TerminationStatusCode.INFEASIBLE,
        "The MIP problem is infeasible.",
    ),
    Constants.COPT_MIPSTATUS_UNBOUNDED: (
        TerminationStatusCode.DUAL_INFEASIBLE,
        "The MIP problem is unbounded.",
    ),
    Constants.COPT_MIPSTATUS_INF_OR_UNB: (
        TerminationStatusCode.INFEASIBLE_OR_UNBOUNDED,
        "The MIP problem is infeasible or unbounded.",
    ),
    Constants.COPT_MIPSTATUS_NODELIMIT: (
        TerminationStatusCode.NODE_LIMIT,
        "The MIP optimization is stopped because of node limit.",
    ),
    Constants.COPT_MIPSTATUS_TIMEOUT: (
        TerminationStatusCode.TIME_LIMIT,
        "The MIP optimization is stopped because of time limit.",
    ),
    Constants.COPT_MIPSTATUS_UNFINISHED: (
        TerminationStatusCode.NUMERICAL_ERROR,
        "The MIP optimization is stopped but the solver cannot provide a solution because of numerical difficulties.",
    ),
    Constants.COPT_MIPSTATUS_INTERRUPTED: (
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


model_attribute_get_func_map = {
    ModelAttribute.ObjectiveSense: get_objsense,
    ModelAttribute.DualStatus: get_dualstatus,
    ModelAttribute.PrimalStatus: get_primalstatus,
    ModelAttribute.RawStatusString: get_rawstatusstring,
    ModelAttribute.TerminationStatus: get_terminationstatus,
    ModelAttribute.Silent: lambda model: model.get_raw_attribute_int("LogToConsole")
    == 0,
    ModelAttribute.SolverName: lambda _: "COPT",
    ModelAttribute.SolverVersion: lambda model: model.version_string(),
}


def set_silent(model, value: bool):
    if value:
        model.set_raw_attribute_int("LogToConsole", 0)
    else:
        model.set_raw_attribute_int("LogToConsole", 1)


model_attribute_set_func_map = {
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


class Model(RawModel):
    def __init__(self, env=None):
        if env is None:
            init_default_env()
            env = DEFAULT_ENV
        super().__init__(env)

        self.mip_start_values: dict[VariableIndex, float] = dict()

    @property
    def c_pointer(self):
        return pycapsule_to_cvoidp(self.get_raw_model())

    def supports_variable_attribute(self, attribute: VariableAttribute):
        return True

    def get_variable_attribute(self, variable, attribute: VariableAttribute):
        get_function_map = {
            VariableAttribute.Value: lambda v: self.get_variable_info(v, "Value"),
            VariableAttribute.LowerBound: lambda v: self.get_variable_info(v, "LB"),
            VariableAttribute.UpperBound: lambda v: self.get_variable_info(v, "UB"),
            VariableAttribute.PrimalStart: lambda v: self.mip_start_values.get(v, None),
            VariableAttribute.Domain: lambda v: self.get_variable_type(v),
            VariableAttribute.Name: lambda v: self.get_variable_name(v),
        }
        get_function = get_function_map.get(attribute, None)
        if not get_function:
            raise ValueError(f"Unknown variable attribute: {attribute}")
        return get_function(variable)

    def set_variable_attribute(self, variable, attribute: VariableAttribute, value):
        set_function_map = {
            VariableAttribute.LowerBound: lambda v, val: self.set_variable_lower_bound(
                v, val
            ),
            VariableAttribute.UpperBound: lambda v, val: self.set_variable_upper_bound(
                v, val
            ),
            VariableAttribute.PrimalStart: lambda v, val: self.mip_start_values.__setitem__(
                v, val
            ),
            VariableAttribute.Domain: lambda v, val: self.set_variable_type(v, val),
            VariableAttribute.Name: lambda v, val: self.set_variable_name(v, val),
        }
        set_function = set_function_map.get(attribute, None)
        if not set_function:
            raise ValueError(f"Unknown variable attribute: {attribute}")
        return set_function(variable, value)

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
        get_function = model_attribute_get_func_map.get(attribute, None)
        if not get_function:
            raise ValueError(f"Unknown model attribute: {attribute}")
        return get_function(self)

    def set_model_attribute(self, attribute: ModelAttribute, value):
        set_function = model_attribute_set_func_map.get(attribute, None)
        if not set_function:
            raise ValueError(f"Unknown model attribute: {attribute}")
        return set_function(self, value)

    def get_constraint_attribute(self, constraint, attribute: ConstraintAttribute):
        func = constraint_attribute_get_func_map.get(attribute, None)
        if func:
            return func(self, constraint)

        raise ValueError(f"Unknown constraint attribute: {attribute}")

    def set_constraint_attribute(
        self, constraint, attribute: ConstraintAttribute, value
    ):
        func = constraint_attribute_set_func_map.get(attribute, None)
        if func:
            return func(self, constraint, value)
        raise ValueError(f"Unknown constraint attribute: {attribute}")
