import logging
import os
import platform
import types
from pathlib import Path

from .highs_model_ext import (
    RawModel,
    HighsSolutionStatus,
    Enum,
    load_library,
)
from .attributes import (
    VariableAttribute,
    ConstraintAttribute,
    ModelAttribute,
    ResultStatusCode,
    TerminationStatusCode,
)
from .core_ext import ConstraintType, VariableIndex
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
        "Linux": "libhighs.so",
        "Darwin": "libhighs.dylib",
        "Windows": "highs.dll",
    }[platform.system()]

    # Environment
    home = os.environ.get("HiGHS_HOME", None)
    if home and os.path.exists(home):
        lib = Path(home) / subdir / libname
        if lib.exists():
            libs.append(str(lib))

    # HiGHS pypi installation
    try:
        import highsbox

        home = highsbox.highs_dist_dir()

        if os.path.exists(home):
            lib = Path(home) / subdir / libname
            if lib.exists():
                libs.append(str(lib))
    except Exception:
        pass

    # default names
    default_libname = libname
    libs.append(default_libname)

    return libs


def autoload_library():
    libs = detected_libraries()
    for lib in libs:
        ret = load_library(lib)
        if ret:
            logging.info(f"Loaded HiGHS library: {lib}")
            return True
    return False


autoload_library()


variable_attribute_get_func_map = {
    VariableAttribute.Value: lambda model, v: model.get_value(v),
    VariableAttribute.LowerBound: lambda model, v: model.get_variable_lower_bound(v),
    VariableAttribute.UpperBound: lambda model, v: model.get_variable_upper_bound(v),
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


def get_dualstatus(model):
    sol = model.solution
    if sol.dual_solution_status == Enum.kHighsSolutionStatusFeasible:
        return ResultStatusCode.FEASIBLE_POINT
    elif sol.dual_solution_status == Enum.kHighsSolutionStatusInfeasible:
        return ResultStatusCode.INFEASIBLE_POINT
    elif model.solution.has_dual_ray:
        return ResultStatusCode.INFEASIBILITY_CERTIFICATE
    else:
        return ResultStatusCode.NO_SOLUTION


def get_primalstatus(model):
    sol = model.solution
    if sol.primal_solution_status == Enum.kHighsSolutionStatusFeasible:
        return ResultStatusCode.FEASIBLE_POINT
    elif sol.primal_solution_status == Enum.kHighsSolutionStatusInfeasible:
        return ResultStatusCode.INFEASIBLE_POINT
    elif sol.has_primal_ray:
        return ResultStatusCode.INFEASIBILITY_CERTIFICATE
    return ResultStatusCode.NO_SOLUTION


def get_rawstatusstring(model):
    status = model.solution.status
    model_status = model.solution.model_status
    if status == HighsSolutionStatus.OPTIMIZE_NOT_CALLED:
        return "OPTIMIZE_NOT_CALLED"
    elif status == HighsSolutionStatus.OPTIMIZE_ERROR:
        return "There was an error calling optimize!"
    elif model_status == Enum.kHighsModelStatusNotset:
        return "kHighsModelStatusNotset"
    elif model_status == Enum.kHighsModelStatusLoadError:
        return "kHighsModelStatusLoadError"
    elif model_status == Enum.kHighsModelStatusModelError:
        return "kHighsModelStatusModelError"
    elif model_status == Enum.kHighsModelStatusPresolveError:
        return "kHighsModelStatusPresolveError"
    elif model_status == Enum.kHighsModelStatusSolveError:
        return "kHighsModelStatusSolveError"
    elif model_status == Enum.kHighsModelStatusPostsolveError:
        return "kHighsModelStatusPostsolveError"
    elif model_status == Enum.kHighsModelStatusModelEmpty:
        return "kHighsModelStatusModelEmpty"
    elif model_status == Enum.kHighsModelStatusOptimal:
        return "kHighsModelStatusOptimal"
    elif model_status == Enum.kHighsModelStatusInfeasible:
        return "kHighsModelStatusInfeasible"
    elif model_status == Enum.kHighsModelStatusUnboundedOrInfeasible:
        return "kHighsModelStatusUnboundedOrInfeasible"
    elif model_status == Enum.kHighsModelStatusUnbounded:
        return "kHighsModelStatusUnbounded"
    elif model_status == Enum.kHighsModelStatusObjectiveBound:
        return "kHighsModelStatusObjectiveBound"
    elif model_status == Enum.kHighsModelStatusObjectiveTarget:
        return "kHighsModelStatusObjectiveTarget"
    elif model_status == Enum.kHighsModelStatusTimeLimit:
        return "kHighsModelStatusTimeLimit"
    elif model_status == Enum.kHighsModelStatusIterationLimit:
        return "kHighsModelStatusIterationLimit"
    else:
        assert model_status == Enum.kHighsModelStatusUnknown
        return "kHighsModelStatusUnknown"


def get_terminationstatus(model):
    status = model.solution.status
    model_status = model.solution.model_status
    if status == HighsSolutionStatus.OPTIMIZE_NOT_CALLED:
        return TerminationStatusCode.OPTIMIZE_NOT_CALLED
    elif status == HighsSolutionStatus.OPTIMIZE_ERROR:
        return TerminationStatusCode.OTHER_ERROR
    elif model_status == Enum.kHighsModelStatusNotset:
        return TerminationStatusCode.OTHER_ERROR
    elif model_status == Enum.kHighsModelStatusLoadError:
        return TerminationStatusCode.OTHER_ERROR
    elif model_status == Enum.kHighsModelStatusModelError:
        return TerminationStatusCode.INVALID_MODEL
    elif model_status == Enum.kHighsModelStatusPresolveError:
        return TerminationStatusCode.OTHER_ERROR
    elif model_status == Enum.kHighsModelStatusSolveError:
        return TerminationStatusCode.OTHER_ERROR
    elif model_status == Enum.kHighsModelStatusPostsolveError:
        return TerminationStatusCode.OTHER_ERROR
    elif model_status == Enum.kHighsModelStatusModelEmpty:
        return TerminationStatusCode.INVALID_MODEL
    elif model_status == Enum.kHighsModelStatusOptimal:
        return TerminationStatusCode.OPTIMAL
    elif model_status == Enum.kHighsModelStatusInfeasible:
        return TerminationStatusCode.Infeasible
    elif model_status == Enum.kHighsModelStatusUnboundedOrInfeasible:
        return TerminationStatusCode.INFEASIBLE_OR_UNBOUNDED
    elif model_status == Enum.kHighsModelStatusUnbounded:
        return TerminationStatusCode.DUAL_INFEASIBLE
    elif model_status == Enum.kHighsModelStatusObjectiveBound:
        return TerminationStatusCode.OBJECTIVE_LIMIT
    elif model_status == Enum.kHighsModelStatusObjectiveTarget:
        return TerminationStatusCode.OBJECTIVE_LIMIT
    elif model_status == Enum.kHighsModelStatusTimeLimit:
        return TerminationStatusCode.TIME_LIMIT
    elif model_status == Enum.kHighsModelStatusIterationLimit:
        return TerminationStatusCode.IterationLimit
    elif model_status == Enum.kHighsModelStatusUnknown:
        return TerminationStatusCode.OTHER_ERROR
    else:
        assert model_status == Enum.kHighsModelStatusSolutionLimit
        return TerminationStatusCode.SOLUTION_LIMIT


model_attribute_get_func_map = {
    ModelAttribute.ObjectiveSense: lambda model: model.get_obj_sense(),
    # ModelAttribute.DualObjectiveValue: lambda model: model.getdualobj(),
    ModelAttribute.ObjectiveValue: lambda model: model.get_obj_value(),
    ModelAttribute.SolveTimeSec: lambda model: model.getruntime(),
    ModelAttribute.NumberOfThreads: lambda model: model.get_raw_option_int("threads"),
    ModelAttribute.RelativeGap: lambda model: model.get_raw_option_double("mip_gap"),
    ModelAttribute.TimeLimitSec: lambda model: model.get_raw_option_double(
        "time_limit"
    ),
    ModelAttribute.DualStatus: get_dualstatus,
    ModelAttribute.PrimalStatus: get_primalstatus,
    ModelAttribute.RawStatusString: get_rawstatusstring,
    ModelAttribute.TerminationStatus: get_terminationstatus,
    ModelAttribute.Silent: lambda model: not model.get_raw_option_bool("output_flag"),
    ModelAttribute.SolverName: lambda _: "HiGHS",
    ModelAttribute.SolverVersion: lambda model: model.version_string(),
}

model_attribute_set_func_map = {
    ModelAttribute.ObjectiveSense: lambda model, v: model.set_obj_sense(v),
    ModelAttribute.NumberOfThreads: lambda model, v: model.set_raw_option_int(
        "threads", v
    ),
    ModelAttribute.RelativeGap: lambda model, v: model.set_raw_option_double(
        "mip_gap", v
    ),
    ModelAttribute.TimeLimitSec: lambda model, v: model.set_raw_option_double(
        "time_limit", v
    ),
    ModelAttribute.Silent: lambda model, v: model.set_raw_option_bool(
        "output_flag", not v
    ),
}

constraint_attribute_get_func_map = {
    ConstraintAttribute.Name: lambda model, constraint: model.get_constraint_name(
        constraint
    ),
    ConstraintAttribute.Primal: lambda model, constraint: model.get_constraint_primal(
        constraint
    ),
    ConstraintAttribute.Dual: lambda model, constraint: model.get_constraint_dual(
        constraint
    ),
}

constraint_attribute_set_func_map = {
    ConstraintAttribute.Name: lambda model, constraint, value: model.set_constraint_name(
        constraint, value
    ),
}

highs_option_raw_type_map = {
    Enum.kHighsOptionTypeBool: bool,
    Enum.kHighsOptionTypeInt: int,
    Enum.kHighsOptionTypeDouble: float,
    Enum.kHighsOptionTypeString: str,
}

highs_info_raw_type_map = {
    Enum.kHighsInfoTypeInt: int,
    Enum.kHighsInfoTypeInt64: "int64",
    Enum.kHighsInfoTypeDouble: float,
}


class Model(RawModel):
    def __init__(self):
        super().__init__()

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
        if type not in {ConstraintType.Linear}:
            raise ValueError(f"Unknown constraint type: {type}")
        return self.m_n_constraints

    def number_of_variables(self):
        return self.m_n_variables

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
        param_type = highs_option_raw_type_map[self.raw_option_type(param_name)]
        get_function_map = {
            bool: self.get_raw_option_bool,
            int: self.get_raw_option_int,
            float: self.get_raw_option_double,
            str: self.get_raw_option_string,
        }
        get_function = get_function_map[param_type]
        return get_function(param_name)

    def set_raw_parameter(self, param_name: str, value):
        param_type = highs_option_raw_type_map[self.raw_option_type(param_name)]
        set_function_map = {
            bool: self.set_raw_option_bool,
            int: self.set_raw_option_int,
            float: self.set_raw_option_double,
            str: self.set_raw_option_string,
        }
        set_function = set_function_map[param_type]
        set_function(param_name, value)

    def get_raw_information(self, param_name: str):
        param_type = highs_info_raw_type_map[self.raw_info_type(param_name)]
        get_function_map = {
            int: self.get_raw_attribute_int,
            "int64": self.get_raw_attribute_int64,
            float: self.get_raw_attribute_double,
        }
        get_function = get_function_map[param_type]
        return get_function(param_name)

    def optimize(self):
        mip_start = self.mip_start_values
        if len(mip_start) != 0:
            variables = list(mip_start.keys())
            values = list(mip_start.values())
            self.set_primal_start(variables, values)
            mip_start.clear()
        super().optimize()
