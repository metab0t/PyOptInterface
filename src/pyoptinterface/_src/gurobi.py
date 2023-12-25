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
    ResultStatusCode,
    TerminationStatusCode,
)
from .core_ext import VariableDomain, ConstraintType, ObjectiveSense
from .solver_common import (
    _get_model_attribute,
    _set_model_attribute,
    _get_entity_attribute,
    _set_entity_attribute,
)

DEFAULT_ENV = None


def init_default_env():
    global DEFAULT_ENV
    if DEFAULT_ENV is None:
        DEFAULT_ENV = Env()


# Variable Attribute
variable_attribute_get_func_map = {
    VariableAttribute.Value: lambda model, v: model.get_variable_raw_attribute_double(
        v, "X"
    ),
    VariableAttribute.LowerBound: lambda model, v: model.get_variable_raw_attribute_double(
        v, "LB"
    ),
    VariableAttribute.UpperBound: lambda model, v: model.get_variable_raw_attribute_double(
        v, "UB"
    ),
    VariableAttribute.PrimalStart: lambda model, v: model.get_variable_raw_attribute_double(
        v, "Start"
    ),
    VariableAttribute.Domain: lambda model, v: model.get_variable_raw_attribute_char(
        v, "VType"
    ),
    VariableAttribute.Name: lambda model, v: model.get_variable_raw_attribute_string(
        v, "VarName"
    ),
}

variable_attribute_get_translate_func_map = {
    VariableAttribute.Domain: lambda v: {
        GRB.BINARY: VariableDomain.Binary,
        GRB.INTEGER: VariableDomain.Integer,
        GRB.CONTINUOUS: VariableDomain.Continuous,
        GRB.SEMICONT: VariableDomain.SemiContinuous,
    }[v],
}

variable_attribute_set_translate_func_map = {
    VariableAttribute.Domain: lambda v: {
        VariableDomain.Binary: GRB.BINARY,
        VariableDomain.Integer: GRB.INTEGER,
        VariableDomain.Continuous: GRB.CONTINUOUS,
        VariableDomain.SemiContinuous: GRB.SEMICONT,
    }[v],
}

variable_attribute_set_func_map = {
    VariableAttribute.Value: lambda model, v, x: model.set_variable_raw_attribute_double(
        v, "X", x
    ),
    VariableAttribute.LowerBound: lambda model, v, x: model.set_variable_raw_attribute_double(
        v, "LB", x
    ),
    VariableAttribute.UpperBound: lambda model, v, x: model.set_variable_raw_attribute_double(
        v, "UB", x
    ),
    VariableAttribute.PrimalStart: lambda model, v, x: model.set_variable_raw_attribute_double(
        v, "Start", x
    ),
    VariableAttribute.Domain: lambda model, v, x: model.set_variable_raw_attribute_char(
        v, "VType", x
    ),
    VariableAttribute.Name: lambda model, v, x: model.set_variable_raw_attribute_string(
        v, "VarName", x
    ),
}

# Model Attribute

constraint_type_attribute_name_map = {
    ConstraintType.Linear: "NumConstrs",
    ConstraintType.Quadratic: "NumQConstrs",
}

_RAW_STATUS_STRINGS = [  # TerminationStatus, RawStatusString
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
    ModelAttribute.Name: lambda model: model.get_model_raw_attribute_string(
        "ModelName"
    ),
    ModelAttribute.ObjectiveSense: lambda model: model.get_model_raw_attribute_int(
        "ModelSense"
    ),
    ModelAttribute.BarrierIterations: lambda model: model.get_model_raw_attribute_int(
        "BarIterCount"
    ),
    ModelAttribute.DualObjectiveValue: lambda model: model.get_model_raw_attribute_double(
        "ObjBound"
    ),
    ModelAttribute.NodeCount: lambda model: model.get_model_raw_attribute_int(
        "NodeCount"
    ),
    ModelAttribute.ObjectiveBound: lambda model: model.get_model_raw_attribute_double(
        "ObjBound"
    ),
    ModelAttribute.ObjectiveValue: lambda model: model.get_model_raw_attribute_double(
        "ObjVal"
    ),
    ModelAttribute.SimplexIterations: lambda model: model.get_model_raw_attribute_int(
        "IterCount"
    ),
    ModelAttribute.SolveTimeSec: lambda model: model.get_model_raw_attribute_double(
        "RunTime"
    ),
    ModelAttribute.NumberOfThreads: lambda model: model.get_model_raw_parameter_int(
        "Threads"
    ),
    ModelAttribute.RelativeGap: lambda model: model.get_model_raw_parameter_double(
        "MIPGap"
    ),
    ModelAttribute.TimeLimitSec: lambda model: model.get_model_raw_parameter_double(
        "TimeLimit"
    ),
    ModelAttribute.DualStatus: get_dualstatus,
    ModelAttribute.PrimalStatus: get_primalstatus,
    ModelAttribute.RawStatusString: get_rawstatusstring,
    ModelAttribute.TerminationStatus: get_terminationstatus,
    ModelAttribute.Silent: get_silent,
    ModelAttribute.SolverName: lambda _: "Gurobi",
    ModelAttribute.SolverVersion: lambda model: model.version_string(),
}

model_attribute_get_translate_func_map = {
    ModelAttribute.ObjectiveSense: lambda v: {
        GRB.MINIMIZE: ObjectiveSense.Minimize,
        GRB.MAXIMIZE: ObjectiveSense.Maximize,
    }[v],
}

model_attribute_set_translate_func_map = {
    ModelAttribute.ObjectiveSense: lambda v: {
        ObjectiveSense.Minimize: GRB.MINIMIZE,
        ObjectiveSense.Maximize: GRB.MAXIMIZE,
    }[v],
}


def set_silent(model, value: bool):
    if value:
        model.set_parameter_int("OutputFlag", 0)
    else:
        model.set_parameter_int("OutputFlag", 1)


model_attribute_set_func_map = {
    ModelAttribute.Name: lambda model, v: model.set_model_raw_attribute_string(
        "ModelName", v
    ),
    ModelAttribute.ObjectiveSense: lambda model, v: model.set_model_raw_attribute_int(
        "ModelSense", v
    ),
    ModelAttribute.NumberOfThreads: lambda model, v: model.set_model_raw_parameter_int(
        "Threads", v
    ),
    ModelAttribute.RelativeGap: lambda model, v: model.set_model_raw_parameter_double(
        "MIPGap", v
    ),
    ModelAttribute.TimeLimitSec: lambda model, v: model.set_model_raw_parameter_double(
        "TimeLimit", v
    ),
    ModelAttribute.Silent: set_silent,
}

# Constraint Attribute


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

    def supports_variable_attribute(self, attribute: VariableAttribute):
        return True

    def get_variable_attribute(self, variable, attribute: VariableAttribute):
        def e(attribute):
            raise ValueError(f"Unknown variable attribute to get: {attribute}")

        value = _get_entity_attribute(
            self,
            variable,
            attribute,
            variable_attribute_get_func_map,
            variable_attribute_get_translate_func_map,
            e,
        )
        return value

    def set_variable_attribute(self, variable, attribute: VariableAttribute, value):
        def e(attribute):
            raise ValueError(f"Unknown variable attribute to set: {attribute}")

        _set_entity_attribute(
            self,
            variable,
            attribute,
            value,
            variable_attribute_set_func_map,
            variable_attribute_set_translate_func_map,
            e,
        )

    def number_of_constraints(self, type: ConstraintType):
        attr_name = constraint_type_attribute_name_map.get(type, None)
        if not attr_name:
            raise ValueError(f"Unknown constraint type: {type}")
        return self.get_model_raw_attribute_int(attr_name)

    def number_of_variables(self):
        return self.get_model_raw_attribute_int("NumVars")

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
            model_attribute_set_func_map,
            model_attribute_set_translate_func_map,
            e,
        )

    def get_constraint_attribute(self, constraint, attribute: ConstraintAttribute):
        def e(attribute):
            raise ValueError(f"Unknown constraint attribute to get: {attribute}")

        value = _get_entity_attribute(
            self,
            constraint,
            attribute,
            constraint_attribute_get_func_map,
            {},
            e,
        )
        return value

    def set_constraint_attribute(
        self, constraint, attribute: ConstraintAttribute, value
    ):
        def e(attribute):
            raise ValueError(f"Unknown constraint attribute to set: {attribute}")

        _set_entity_attribute(
            self,
            constraint,
            attribute,
            value,
            constraint_attribute_set_func_map,
            {},
            e,
        )

    def get_raw_parameter(self, param_name: str):
        param_type = gurobi_raw_type_map[self.raw_parameter_type(param_name)]
        get_function_map = {
            int: self.get_raw_parameter_int,
            float: self.get_raw_parameter_double,
            str: self.get_raw_parameter_string,
        }
        get_function = get_function_map[param_type]
        return get_function(param_name)

    def set_raw_parameter(self, param_name: str, value):
        param_type = gurobi_raw_type_map[self.raw_parameter_type(param_name)]
        set_function_map = {
            int: self.set_raw_parameter_int,
            float: self.set_raw_parameter_double,
            str: self.set_raw_parameter_string,
        }
        set_function = set_function_map[param_type]
        set_function(param_name, value)
