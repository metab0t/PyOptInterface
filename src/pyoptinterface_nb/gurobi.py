# try to load DLL of gurobi in${GUUROBI_HOME}/bin
# only on windows
import os, platform

if platform.system() == "Windows":
    gurobi_home = os.environ.get("GUROBI_HOME", None)
    if not gurobi_home:
        raise ValueError("GUROBI_HOME is not set in environment")
    if not os.path.exists(gurobi_home):
        raise ValueError(f"GUROBI_HOME does not exist: {gurobi_home}")
    os.add_dll_directory(os.path.join(gurobi_home, "bin"))

try:
    from .gurobi_model_ext import RawModel, Env, GRB
except Exception as e:
    raise ImportError(
        f"Failed to import gurobi_model_ext. Please check if GUROBI_HOME is set correctly. Error: {e}"
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
from .core_ext import VariableDomain, ConstraintType, ObjectiveSense
from .ctypes_helper import pycapsule_to_cvoidp

DEFAULT_ENV = None


def init_default_env():
    global DEFAULT_ENV
    if DEFAULT_ENV is None:
        DEFAULT_ENV = Env()


# Variable Attribute

variable_attribute_name_map = {
    VariableAttribute.Value: "X",
    VariableAttribute.LowerBound: "LB",
    VariableAttribute.UpperBound: "UB",
    VariableAttribute.PrimalStart: "Start",
    VariableAttribute.Domain: "VType",
    VariableAttribute.Name: "VarName",
}


def translate_variable_attribute_name(attribute: VariableAttribute) -> str:
    name = variable_attribute_name_map.get(attribute, None)
    if not name:
        raise ValueError(f"Unknown variable attribute: {attribute}")
    return name


variable_attribute_value_need_translate_type = {VariableDomain}

variable_attribute_value_to_gurobi_map = {
    VariableDomain.Binary: GRB.BINARY,
    VariableDomain.Integer: GRB.INTEGER,
    VariableDomain.Continuous: GRB.CONTINUOUS,
    VariableDomain.SemiContinuous: GRB.SEMICONT,
}

variable_attribute_gurobi_to_value_map = {
    v: k for k, v in variable_attribute_value_to_gurobi_map.items()
}

CHAR_TYPE = "char"
variable_attribute_query_type_map = var_attr_type_map | {
    VariableAttribute.Domain: CHAR_TYPE
}

# Model Attribute

constraint_type_attribute_name_map = {
    ConstraintType.Linear: "NumConstrs",
    ConstraintType.Quadratic: "NumQConstrs",
}

_RAW_STATUS_STRINGS = [
    # TerminationStatus, RawStatusString
    (
        TerminationStatusCode.OPTIMIZE_NOT_CALLED,
        "Model is loaded, but no solution information is available.",
    ),
    (
        TerminationStatusCode.OPTIMAL,
        "Model was solved to optimality (subject to tolerances), and an optimal solution is available.",
    ),
    (TerminationStatusCode.INFEASIBLE, "Model was proven to be infeasible."),
    (
        TerminationStatusCode.INFEASIBLE_OR_UNBOUNDED,
        "Model was proven to be either infeasible or unbounded. To obtain a more definitive conclusion, set the DualReductions parameter to 0 and reoptimize.",
    ),
    (
        TerminationStatusCode.DUAL_INFEASIBLE,
        "Model was proven to be unbounded. Important note: an unbounded status indicates the presence of an unbounded ray that allows the objective to improve without limit. It says nothing about whether the model has a feasible solution. If you require information on feasibility, you should set the objective to zero and reoptimize.",
    ),
    (
        TerminationStatusCode.OBJECTIVE_LIMIT,
        "Optimal objective for model was proven to be worse than the value specified in the Cutoff parameter. No solution information is available.",
    ),
    (
        TerminationStatusCode.ITERATION_LIMIT,
        "Optimization terminated because the total number of simplex iterations performed exceeded the value specified in the IterationLimit parameter, or because the total number of barrier iterations exceeded the value specified in the BarIterLimit parameter.",
    ),
    (
        TerminationStatusCode.NODE_LIMIT,
        "Optimization terminated because the total number of branch-and-cut nodes explored exceeded the value specified in the NodeLimit parameter.",
    ),
    (
        TerminationStatusCode.TIME_LIMIT,
        "Optimization terminated because the time expended exceeded the value specified in the TimeLimit parameter.",
    ),
    (
        TerminationStatusCode.SOLUTION_LIMIT,
        "Optimization terminated because the number of solutions found reached the value specified in the SolutionLimit parameter.",
    ),
    (TerminationStatusCode.INTERRUPTED, "Optimization was terminated by the user."),
    (
        TerminationStatusCode.NUMERICAL_ERROR,
        "Optimization was terminated due to unrecoverable numerical difficulties.",
    ),
    (
        TerminationStatusCode.LOCALLY_SOLVED,
        "Unable to satisfy optimality tolerances; a sub-optimal solution is available.",
    ),
    (
        TerminationStatusCode.OTHER_ERROR,
        "An asynchronous optimization call was made, but the associated optimization run is not yet complete.",
    ),
    (
        TerminationStatusCode.OBJECTIVE_LIMIT,
        "User specified an objective limit (a bound on either the best objective or the best bound), and that limit has been reached.",
    ),
    (
        TerminationStatusCode.OTHER_LIMIT,
        "Optimization terminated because the work expended exceeded the value specified in the WorkLimit parameter.",
    ),
    (
        TerminationStatusCode.MEMORY_LIMIT,
        "Optimization terminated because the total amount of allocated memory exceeded the value specified in the SoftMemLimit parameter.",
    ),
]

model_attribute_name_map = {
    # ModelLike API
    ModelAttribute.Name: "ModelName",
    ModelAttribute.ObjectiveSense: "ModelSense",
    # AbstractOptimizer API
    # DualStatus
    # PrimalStatus
    # RawStatusString
    # TerminationStatus
    ModelAttribute.BarrierIterations: "BarIterCount",
    ModelAttribute.DualObjectiveValue: "ObjBound",
    ModelAttribute.NodeCount: "NodeCount",
    # NumberOfThreads
    ModelAttribute.ObjectiveBound: "ObjBound",
    ModelAttribute.ObjectiveValue: "ObjVal",
    # RelativeGap
    # Silent
    ModelAttribute.SimplexIterations: "IterCount",
    # SolverName
    # SolverVersion
    ModelAttribute.SolveTimeSec: "RunTime",
    # TimeLimitSec
}

model_attribute_parameter_name_map = {
    ModelAttribute.NumberOfThreads: "Threads",
    ModelAttribute.RelativeGap: "MIPGap",
    ModelAttribute.TimeLimitSec: "TimeLimit",
}

gurobi_raw_type_map = {
    1: int,
    2: float,
    3: str,
}


def get_terminationstatus(model):
    status = model.get_model_raw_attribute_int("Status")
    assert status >= 1 and status <= 15, f"Unknown status code: {status}"
    return _RAW_STATUS_STRINGS[status - 1][0]


def get_primalstatus(model):
    termination_status = get_terminationstatus(model)
    if termination_status in (
        TerminationStatusCode.DUAL_INFEASIBLE,
        TerminationStatusCode.INFEASIBLE_OR_UNBOUNDED,
    ):
        try:
            model.get_model_raw_attribute_list_double("UnbdRay", [0])
            return ResultStatusCode.INFEASIBILITY_CERTIFICATE
        except:
            return ResultStatusCode.NO_SOLUTION
    solcount = model.get_model_raw_attribute_int("SolCount")
    if solcount == 0:
        return ResultStatusCode.NO_SOLUTION
    else:
        return ResultStatusCode.FEASIBLE_POINT


def get_dualstatus(model):
    if model.get_model_raw_attribute_int("IsMIP") != 0:
        return ResultStatusCode.NO_SOLUTION
    elif (
        model.get_model_raw_attribute_int("IsQCP") != 0
        and model.get_model_raw_attribute("QCPDual") != 1
    ):
        return ResultStatusCode.NO_SOLUTION
    termination_status = get_terminationstatus(model)
    if termination_status in (
        TerminationStatusCode.DUAL_INFEASIBLE,
        TerminationStatusCode.INFEASIBLE_OR_UNBOUNDED,
    ):
        try:
            model.get_model_raw_attribute_list_double("FarkasDual", [0])
            return ResultStatusCode.INFEASIBILITY_CERTIFICATE
        except:
            return ResultStatusCode.NO_SOLUTION
    solcount = model.get_model_raw_attribute_int("SolCount")
    if solcount == 0:
        return ResultStatusCode.NO_SOLUTION
    try:
        model.get_model_raw_attribute_list_double("RC", [0])
        return ResultStatusCode.FEASIBLE_POINT
    except:
        return ResultStatusCode.NO_SOLUTION


def get_rawstatusstring(model):
    status = model.get_model_raw_attribute_int("Status")
    assert status >= 1 and status <= 15, f"Unknown status code: {status}"
    return _RAW_STATUS_STRINGS[status - 1][1]


def get_silent(model):
    return model.get_model_raw_parameter_int("OutputFlag") == 0


model_attribute_get_func_map = {
    ModelAttribute.DualStatus: get_dualstatus,
    ModelAttribute.PrimalStatus: get_primalstatus,
    ModelAttribute.RawStatusString: get_rawstatusstring,
    ModelAttribute.TerminationStatus: get_terminationstatus,
    ModelAttribute.Silent: get_silent,
    ModelAttribute.SolverName: lambda _: "Gurobi",
    ModelAttribute.SolverVersion: lambda model: model.version_string(),
}


def set_silent(model, value: bool):
    if value:
        model.set_parameter_int("OutputFlag", 0)
    else:
        model.set_parameter_int("OutputFlag", 1)


model_attribute_set_func_map = {
    ModelAttribute.Silent: set_silent,
}


def get_constraint_name(model, constraint):
    type = constraint.type
    attr_name_dict = {
        ConstraintType.Linear: "ConstrName",
        ConstraintType.Quadratric: "QConstrName",
    }
    attr_name = attr_name_dict.get(type, None)
    if not attr_name:
        raise ValueError(f"Unknown constraint type: {type}")
    return model.get_constraint_raw_attribute_string(constraint, attr_name)


def get_constraint_primal(model, constraint):
    # Linear : RHS - Slack
    # Quadratic : QCRHS - QCSlack
    type = constraint.type
    attr_name_dict = {
        ConstraintType.Linear: ("RHS", "Slack"),
        ConstraintType.Quadratric: ("QCRHS", "QCSlack"),
    }
    attr_name = attr_name_dict.get(type, None)
    if not attr_name:
        raise ValueError(f"Unknown constraint type: {type}")
    rhs = model.get_constraint_raw_attribute_double(constraint, attr_name[0])
    slack = model.get_constraint_raw_attribute_double(constraint, attr_name[1])
    return rhs - slack


def get_constraint_dual(model, constraint):
    type = constraint.type
    attr_name_dict = {
        ConstraintType.Linear: "Pi",
        ConstraintType.Quadratric: "QCPi",
    }
    attr_name = attr_name_dict.get(type, None)
    if not attr_name:
        raise ValueError(f"Unknown constraint type: {type}")
    return model.get_constraint_raw_attribute_string(constraint, attr_name)


constraint_attribute_get_func_map = {
    ConstraintAttribute.Name: get_constraint_name,
    ConstraintAttribute.Primal: get_constraint_primal,
    ConstraintAttribute.Dual: get_constraint_dual,
}


def set_constraint_name(model, constraint, value):
    type = constraint.type
    attr_name_dict = {
        ConstraintType.Linear: "ConstrName",
        ConstraintType.Quadratric: "QConstrName",
    }
    attr_name = attr_name_dict.get(type, None)
    if not attr_name:
        raise ValueError(f"Unknown constraint type: {type}")
    model.set_constraint_raw_attribute_string(constraint, attr_name, value)


constraint_attribute_set_func_map = {
    ConstraintAttribute.Name: set_constraint_name,
}


class Model(RawModel):
    def __init__(self, env=None):
        if env is None:
            init_default_env()
            env = DEFAULT_ENV
        super().__init__(env)

    @property
    def c_pointer(self):
        return pycapsule_to_cvoidp(self.get_raw_model())

    def supports_variable_attribute(self, attribute: VariableAttribute):
        return True

    def get_variable_attribute(self, variable, attribute: VariableAttribute):
        value_type = default_variable_attribute_type(attribute)
        query_type = variable_attribute_query_type_map[attribute]
        assert query_type in (int, float, str, CHAR_TYPE)

        query_name = translate_variable_attribute_name(attribute)
        get_function_map = {
            int: self.get_variable_raw_attribute_int,
            float: self.get_variable_raw_attribute_double,
            str: self.get_variable_raw_attribute_string,
            CHAR_TYPE: self.get_variable_raw_attribute_char,
        }
        get_function = get_function_map[query_type]
        value = get_function(variable, query_name)

        if value_type in variable_attribute_value_need_translate_type:
            value = variable_attribute_gurobi_to_value_map[value]
        return value

    def set_variable_attribute(self, variable, attribute: VariableAttribute, value):
        value_type = default_variable_attribute_type(attribute)
        query_type = variable_attribute_query_type_map[attribute]
        assert query_type in (int, float, str, CHAR_TYPE)

        query_name = translate_variable_attribute_name(attribute)
        set_function_map = {
            int: self.set_variable_raw_attribute_int,
            float: self.set_variable_raw_attribute_double,
            str: self.set_variable_raw_attribute_string,
            CHAR_TYPE: self.set_variable_raw_attribute_char,
        }
        set_function = set_function_map[query_type]

        if value_type in variable_attribute_value_need_translate_type:
            value = variable_attribute_value_to_gurobi_map[value]
        set_function(variable, query_name, value)

    def number_of_constraints(self, type: ConstraintType):
        attr_name = constraint_type_attribute_name_map.get(type, None)
        if not attr_name:
            raise ValueError(f"Unknown constraint type: {type}")
        return self.get_model_raw_attribute_int(attr_name)

    def number_of_variables(self):
        return self.get_model_raw_attribute_int("NumVars")

    def get_model_attribute(self, attribute: ModelAttribute):
        attr_name = model_attribute_name_map.get(attribute, None)
        if attr_name:
            param_type = gurobi_raw_type_map[self.raw_attribute_type(attr_name)]
            get_function_map = {
                int: self.get_model_raw_attribute_int,
                float: self.get_model_raw_attribute_double,
                str: self.get_model_raw_attribute_string,
            }
            get_function = get_function_map[param_type]
            attr = get_function(attr_name)
            if attribute == ModelAttribute.ObjectiveSense:
                attr = {
                    GRB.MINIMIZE: ObjectiveSense.Minimize,
                    GRB.MAXIMIZE: ObjectiveSense.Maximize,
                }[attr]
            return attr

        param_name = model_attribute_parameter_name_map.get(attribute, None)
        if param_name:
            param_type = gurobi_raw_type_map[self.raw_parameter_type(param_name)]
            get_function_map = {
                int: self.get_model_raw_parameter_int,
                float: self.get_model_raw_parameter_double,
                str: self.get_model_raw_parameter_string,
            }
            get_function = get_function_map[param_type]
            return get_function(param_name)

        func = model_attribute_get_func_map.get(attribute, None)
        if func:
            return func(self)

        raise ValueError(f"Unknown model attribute: {attribute}")

    def set_model_attribute(self, attribute: ModelAttribute, value):
        attr_name = model_attribute_name_map.get(attribute, None)
        if attr_name:
            param_type = gurobi_raw_type_map[self.raw_attribute_type(attr_name)]
            set_function_map = {
                int: self.set_model_raw_attribute_int,
                float: self.set_model_raw_attribute_double,
                str: self.set_model_raw_attribute_string,
            }
            set_function = set_function_map[param_type]
            if attribute == ModelAttribute.ObjectiveSense:
                value = {
                    ObjectiveSense.Minimize: GRB.MINIMIZE,
                    ObjectiveSense.Maximize: GRB.MAXIMIZE,
                }[value]
            set_function(attr_name, value)
            return

        param_name = model_attribute_parameter_name_map.get(attribute, None)
        if param_name:
            param_type = gurobi_raw_type_map[self.raw_parameter_type(param_name)]
            set_function_map = {
                int: self.set_model_raw_parameter_int,
                float: self.set_model_raw_parameter_double,
                str: self.set_model_raw_parameter_string,
            }
            set_function = set_function_map[param_type]
            set_function(param_name, value)
            return

        func = model_attribute_set_func_map.get(attribute, None)
        if func:
            func(self, value)
            return

        raise ValueError(f"Unknown model attribute: {attribute}")

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

    def get_model_raw_parameter(model, param_name: str):
        param_type = gurobi_raw_type_map[model.raw_parameter_type(param_name)]
        get_function_map = {
            int: model.get_model_raw_parameter_int,
            float: model.get_model_raw_parameter_double,
            str: model.get_model_raw_parameter_string,
        }
        get_function = get_function_map[param_type]
        return get_function(param_name)

    def set_model_raw_parameter(model, param_name: str, value):
        param_type = gurobi_raw_type_map[model.raw_parameter_type(param_name)]
        set_function_map = {
            int: model.set_model_raw_parameter_int,
            float: model.set_model_raw_parameter_double,
            str: model.set_model_raw_parameter_string,
        }
        set_function = set_function_map[param_type]
        set_function(param_name, value)
