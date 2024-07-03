import os
import platform
from typing import Optional
import types
from pathlib import Path
import re
import logging

from .mosek_model_ext import RawModel, Env, Enum, load_library
from .attributes import (
    VariableAttribute,
    ConstraintAttribute,
    ModelAttribute,
    ResultStatusCode,
    TerminationStatusCode,
)
from .core_ext import ConstraintType
from .solver_common import (
    _direct_get_model_attribute,
    _direct_set_model_attribute,
    _direct_get_entity_attribute,
    _direct_set_entity_attribute,
)
from .constraint_bridge import bridge_soc_quadratic_constraint
from .aml import make_nd_variable


def detected_libraries():
    libs = []

    libname_pattern = {
        "Linux": r"libmosek64\.so",
        "Darwin": r"libmosek64\.dylib",
        "Windows": r"mosek64_(\d+)_(\d+)\.dll",
    }[platform.system()]
    suffix_pattern = {
        "Linux": "*.so",
        "Darwin": "*.dylib",
        "Windows": "*.dll",
    }[platform.system()]

    # Environment
    home = os.environ.get("MOSEK_10_2_BINDIR", None)
    if home is None:
        home = os.environ.get("MOSEK_10_1_BINDIR", None)
    if home and os.path.exists(home):
        dir = Path(home)
        for path in dir.glob(suffix_pattern):
            match = re.match(libname_pattern, path.name)
            if match:
                libs.append(str(path))

    # mosekpy installation
    try:
        import mosek

        dir = Path(mosek.__path__[0])

        libname_pattern = {
            "Linux": r"libmosek64\.so\.*",
            "Darwin": r"libmosek64\.*\.dylib",
            "Windows": r"mosek64_(\d+)_(\d+)\.dll",
        }[platform.system()]
        suffix_pattern = {
            "Linux": "*.so.*",
            "Darwin": "*.dylib",
            "Windows": "*.dll",
        }[platform.system()]

        for path in dir.glob(suffix_pattern):
            match = re.match(libname_pattern, path.name)
            if match:
                libs.append(str(path))
    except Exception:
        pass

    # default names
    default_libname = {
        "Linux": "libmosek64.so",
        "Darwin": "libmosek64.dylib",
        "Windows": "mosek64_10_2.dll",
    }[platform.system()]
    libs.append(default_libname)

    return libs


def autoload_library():
    libs = detected_libraries()
    for lib in libs:
        ret = load_library(lib)
        if ret:
            logging.info(f"Loaded Mosek library: {lib}")
            return True
    return False


autoload_library()

DEFAULT_ENV = None


def init_default_env():
    global DEFAULT_ENV
    if DEFAULT_ENV is None:
        DEFAULT_ENV = Env()


variable_attribute_get_func_map = {
    VariableAttribute.Value: lambda model, v: model.get_value(v),
    VariableAttribute.LowerBound: lambda model, v: model.get_variable_lower_bound(v),
    VariableAttribute.UpperBound: lambda model, v: model.get_variable_upper_bound(v),
    # VariableAttribute.PrimalStart: lambda model, v: model.mip_start_values.get(v, None),
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
    VariableAttribute.PrimalStart: lambda model, v, val: model.set_variable_primal(
        v, val
    ),
    VariableAttribute.Domain: lambda model, v, val: model.set_variable_type(v, val),
    VariableAttribute.Name: lambda model, v, val: model.set_variable_name(v, val),
}


def get_primalstatus(model):
    solsta = model.getsolsta()
    if solsta == Enum.MSK_SOL_STA_UNKNOWN:
        return ResultStatusCode.UNKNOWN_RESULT_STATUS
    elif solsta == Enum.MSK_SOL_STA_OPTIMAL:
        return ResultStatusCode.FEASIBLE_POINT
    elif solsta == Enum.MSK_SOL_STA_PRIM_FEAS:
        return ResultStatusCode.FEASIBLE_POINT
    elif solsta == Enum.MSK_SOL_STA_DUAL_FEAS:
        return ResultStatusCode.UNKNOWN_RESULT_STATUS
    elif solsta == Enum.MSK_SOL_STA_PRIM_AND_DUAL_FEAS:
        return ResultStatusCode.FEASIBLE_POINT
    elif solsta == Enum.MSK_SOL_STA_PRIM_INFEAS_CER:
        return ResultStatusCode.NO_SOLUTION
    elif solsta == Enum.MSK_SOL_STA_DUAL_INFEAS_CER:
        return ResultStatusCode.INFEASIBILITY_CERTIFICATE
    elif solsta == Enum.MSK_SOL_STA_PRIM_ILLPOSED_CER:
        return ResultStatusCode.NO_SOLUTION
    elif solsta == Enum.MSK_SOL_STA_DUAL_ILLPOSED_CER:
        return ResultStatusCode.REDUCTION_CERTIFICATE
    elif solsta == Enum.MSK_SOL_STA_INTEGER_OPTIMAL:
        return ResultStatusCode.FEASIBLE_POINT
    else:
        return ResultStatusCode.UNKNOWN_RESULT_STATUS


def get_dualstatus(model):
    solsta = model.getsolsta()
    if solsta == Enum.MSK_SOL_STA_UNKNOWN:
        return ResultStatusCode.UNKNOWN_RESULT_STATUS
    elif solsta == Enum.MSK_SOL_STA_OPTIMAL:
        return ResultStatusCode.FEASIBLE_POINT
    elif solsta == Enum.MSK_SOL_STA_PRIM_FEAS:
        return ResultStatusCode.UNKNOWN_RESULT_STATUS
    elif solsta == Enum.MSK_SOL_STA_DUAL_FEAS:
        return ResultStatusCode.FEASIBLE_POINT
    elif solsta == Enum.MSK_SOL_STA_PRIM_AND_DUAL_FEAS:
        return ResultStatusCode.FEASIBLE_POINT
    elif solsta == Enum.MSK_SOL_STA_PRIM_INFEAS_CER:
        return ResultStatusCode.INFEASIBILITY_CERTIFICATE
    elif solsta == Enum.MSK_SOL_STA_DUAL_INFEAS_CER:
        return ResultStatusCode.NO_SOLUTION
    elif solsta == Enum.MSK_SOL_STA_PRIM_ILLPOSED_CER:
        return ResultStatusCode.REDUCTION_CERTIFICATE
    elif solsta == Enum.MSK_SOL_STA_DUAL_ILLPOSED_CER:
        return ResultStatusCode.NO_SOLUTION
    elif solsta == Enum.MSK_SOL_STA_INTEGER_OPTIMAL:
        return ResultStatusCode.NO_SOLUTION
    else:
        return ResultStatusCode.UNKNOWN_RESULT_STATUS


solsta_names = {
    Enum.MSK_SOL_STA_UNKNOWN: "MSK_SOL_STA_UNKNOWN",
    Enum.MSK_SOL_STA_OPTIMAL: "MSK_SOL_STA_OPTIMAL",
    Enum.MSK_SOL_STA_PRIM_FEAS: "MSK_SOL_STA_PRIM_FEAS",
    Enum.MSK_SOL_STA_DUAL_FEAS: "MSK_SOL_STA_DUAL_FEAS",
    Enum.MSK_SOL_STA_PRIM_AND_DUAL_FEAS: "MSK_SOL_STA_PRIM_AND_DUAL_FEAS",
    Enum.MSK_SOL_STA_PRIM_INFEAS_CER: "MSK_SOL_STA_PRIM_INFEAS_CER",
    Enum.MSK_SOL_STA_DUAL_INFEAS_CER: "MSK_SOL_STA_DUAL_INFEAS_CER",
    Enum.MSK_SOL_STA_PRIM_ILLPOSED_CER: "MSK_SOL_STA_PRIM_ILLPOSED_CER",
    Enum.MSK_SOL_STA_DUAL_ILLPOSED_CER: "MSK_SOL_STA_DUAL_ILLPOSED_CER",
    Enum.MSK_SOL_STA_INTEGER_OPTIMAL: "MSK_SOL_STA_INTEGER_OPTIMAL",
}

termination_names = {
    Enum.MSK_RES_OK: "MSK_RES_OK",
    Enum.MSK_RES_TRM_MAX_ITERATIONS: "MSK_RES_TRM_MAX_ITERATIONS",
    Enum.MSK_RES_TRM_MAX_TIME: "MSK_RES_TRM_MAX_TIME",
    Enum.MSK_RES_TRM_OBJECTIVE_RANGE: "MSK_RES_TRM_OBJECTIVE_RANGE",
    Enum.MSK_RES_TRM_STALL: "MSK_RES_TRM_STALL",
    Enum.MSK_RES_TRM_USER_CALLBACK: "MSK_RES_TRM_USER_CALLBACK",
    Enum.MSK_RES_TRM_MIO_NUM_RELAXS: "MSK_RES_TRM_MIO_NUM_RELAXS",
    Enum.MSK_RES_TRM_MIO_NUM_BRANCHES: "MSK_RES_TRM_MIO_NUM_BRANCHES",
    Enum.MSK_RES_TRM_NUM_MAX_NUM_INT_SOLUTIONS: "MSK_RES_TRM_NUM_MAX_NUM_INT_SOLUTIONS",
    Enum.MSK_RES_TRM_MAX_NUM_SETBACKS: "MSK_RES_TRM_MAX_NUM_SETBACKS",
    Enum.MSK_RES_TRM_NUMERICAL_PROBLEM: "MSK_RES_TRM_NUMERICAL_PROBLEM",
    Enum.MSK_RES_TRM_LOST_RACE: "MSK_RES_TRM_LOST_RACE",
    Enum.MSK_RES_TRM_INTERNAL: "MSK_RES_TRM_INTERNAL",
    Enum.MSK_RES_TRM_INTERNAL_STOP: "MSK_RES_TRM_INTERNAL_STOP",
}


def get_rawstatusstring(model):
    trm = model.last_solve_return_code
    if trm is None:
        return "OPTIMIZE_NOT_CALLED"
    elif trm == Enum.MSK_RES_OK:
        solsta = model.getsolsta()
        return solsta_names[solsta]
    else:
        return termination_names[trm]


def get_terminationstatus(model):
    trm = model.last_solve_return_code
    prosta = model.getprosta()
    solsta = model.getsolsta()
    if trm is None:
        return TerminationStatusCode.OPTIMIZE_NOT_CALLED
    elif trm == Enum.MSK_RES_OK:
        if prosta in (Enum.MSK_PRO_STA_PRIM_INFEAS, Enum.MSK_PRO_STA_ILL_POSED):
            return TerminationStatusCode.INFEASIBLE
        elif prosta == Enum.MSK_PRO_STA_DUAL_INFEAS:
            return TerminationStatusCode.DUAL_INFEASIBLE
        elif prosta == Enum.MSK_PRO_STA_PRIM_INFEAS_OR_UNBOUNDED:
            return TerminationStatusCode.INFEASIBLE_OR_UNBOUNDED
        elif solsta in (Enum.MSK_SOL_STA_OPTIMAL, Enum.MSK_SOL_STA_INTEGER_OPTIMAL):
            return TerminationStatusCode.OPTIMAL
        else:
            return TerminationStatusCode.OTHER_ERROR  # ??
    elif trm == Enum.MSK_RES_TRM_MAX_ITERATIONS:
        return TerminationStatusCode.ITERATION_LIMIT
    elif trm == Enum.MSK_RES_TRM_MAX_TIME:
        return TerminationStatusCode.TIME_LIMIT
    elif trm == Enum.MSK_RES_TRM_OBJECTIVE_RANGE:
        return TerminationStatusCode.OBJECTIVE_LIMIT
    elif trm == Enum.MSK_RES_TRM_MIO_NUM_RELAXS:
        return TerminationStatusCode.OTHER_LIMIT
    elif trm == Enum.MSK_RES_TRM_MIO_NUM_BRANCHES:
        return TerminationStatusCode.NODE_LIMIT
    elif trm == Enum.MSK_RES_TRM_NUM_MAX_NUM_INT_SOLUTIONS:
        return TerminationStatusCode.SOLUTION_LIMIT
    elif trm == Enum.MSK_RES_TRM_STALL:
        return TerminationStatusCode.SLOW_PROGRESS
    elif trm == Enum.MSK_RES_TRM_USER_CALLBACK:
        return TerminationStatusCode.INTERRUPTED
    elif trm == Enum.MSK_RES_TRM_MAX_NUM_SETBACKS:
        return TerminationStatusCode.OTHER_LIMIT
    elif trm == Enum.MSK_RES_TRM_NUMERICAL_PROBLEM:
        return TerminationStatusCode.SLOW_PROGRESS
    elif trm == Enum.MSK_RES_TRM_INTERNAL:
        return TerminationStatusCode.OTHER_ERROR
    elif trm == Enum.MSK_RES_TRM_INTERNAL_STOP:
        return TerminationStatusCode.OTHER_ERROR
    else:
        TerminationStatusCode.OTHER_ERROR


model_attribute_get_func_map = {
    ModelAttribute.ObjectiveSense: lambda model: model.get_obj_sense(),
    ModelAttribute.DualObjectiveValue: lambda model: model.getdualobj(),
    ModelAttribute.ObjectiveValue: lambda model: model.getprimalobj(),
    ModelAttribute.SolveTimeSec: lambda model: model.get_raw_information_double(
        "MSK_DINF_OPTIMIZER_TIME"
    ),
    ModelAttribute.NumberOfThreads: lambda model: model.get_raw_parameter_int(
        "MSK_IPAR_NUM_THREADS"
    ),
    ModelAttribute.RelativeGap: lambda model: model.get_raw_parameter_double(
        "MSK_DPAR_MIO_REL_GAP_CONST"
    ),
    ModelAttribute.TimeLimitSec: lambda model: model.get_raw_parameter_double(
        "MSK_DPAR_OPTIMIZER_MAX_TIME"
    ),
    ModelAttribute.DualStatus: get_dualstatus,
    ModelAttribute.PrimalStatus: get_primalstatus,
    ModelAttribute.RawStatusString: get_rawstatusstring,
    ModelAttribute.TerminationStatus: get_terminationstatus,
    ModelAttribute.Silent: lambda model: model.silent,
    ModelAttribute.SolverName: lambda _: "MOSEK",
    ModelAttribute.SolverVersion: lambda model: model.version_string(),
}


def set_silent(model, value: bool):
    model.silent = value
    if value:
        model.disable_log()
    else:
        model.enable_log()


model_attribute_set_func_map = {
    ModelAttribute.ObjectiveSense: lambda model, v: model.set_obj_sense(v),
    ModelAttribute.NumberOfThreads: lambda model, v: model.set_raw_parameter_int(
        "MSK_IPAR_NUM_THREADS", v
    ),
    ModelAttribute.RelativeGap: lambda model, v: model.set_raw_parameter_double(
        "MSK_DPAR_MIO_REL_GAP_CONST", v
    ),
    ModelAttribute.TimeLimitSec: lambda model, v: model.set_raw_parameter_double(
        "MSK_DPAR_OPTIMIZER_MAX_TIME", v
    ),
    ModelAttribute.Silent: set_silent,
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


def tell_enum_type(name: str):
    if name.startswith("MSK_D"):
        return float
    elif name.startswith("MSK_I"):
        return int
    elif name.startswith("MSK_S"):
        return str
    else:
        raise ValueError(f"Unknown parameter name: {name}")


class Model(RawModel):
    def __init__(self, env=None):
        if env is None:
            init_default_env()
            env = DEFAULT_ENV
        super().__init__(env)
        # We must keep a reference to the environment to prevent it from being garbage collected
        self._env = env
        self.last_solve_return_code: Optional[int] = None
        self.silent = True

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
        if type not in {ConstraintType.Linear, ConstraintType.Quadratic}:
            raise ValueError(f"Unknown constraint type: {type}")
        return self.getnumcon()

    def number_of_variables(self):
        return self.getnumvar()

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
        param_type = tell_enum_type(param_name)
        get_function_map = {
            int: self.get_raw_parameter_int,
            float: self.get_raw_parameter_double,
            str: self.get_raw_parameter_string,
        }
        get_function = get_function_map[param_type]
        return get_function(param_name)

    def set_raw_parameter(self, param_name: str, value):
        param_type = tell_enum_type(param_name)
        set_function_map = {
            int: self.set_raw_parameter_int,
            float: self.set_raw_parameter_double,
            str: self.set_raw_parameter_string,
        }
        set_function = set_function_map[param_type]
        set_function(param_name, value)

    def get_raw_information(self, param_name: str):
        param_type = tell_enum_type(param_name)
        get_function_map = {
            int: self.get_raw_information_int,
            float: self.get_raw_information_double,
        }
        get_function = get_function_map[param_type]
        return get_function(param_name)

    def optimize(self):
        ret = super().optimize()
        self.last_solve_return_code = ret
