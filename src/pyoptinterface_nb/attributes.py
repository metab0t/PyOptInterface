from enum import Enum, auto
from .core_ext import VariableDomain, ObjectiveSense


class VariableAttribute(Enum):
    Value = auto()
    LowerBound = auto()
    UpperBound = auto()
    Domain = auto()
    PrimalStart = auto()
    Name = auto()


var_attr_type_map = {
    VariableAttribute.Value: float,
    VariableAttribute.LowerBound: float,
    VariableAttribute.UpperBound: float,
    VariableAttribute.PrimalStart: float,
    VariableAttribute.Domain: VariableDomain,
    VariableAttribute.Name: str,
}


def default_variable_attribute_type(attribute):
    T = var_attr_type_map.get(attribute, None)
    if not T:
        raise ValueError(f"Unknown variable attribute: {attribute}")
    return T


class ModelAttribute(Enum):
    # ModelLike API
    NumberOfConstraints = auto()
    NumberOfVariables = auto()
    Name = auto()
    ObjectiveSense = auto()

    # AbstractOptimizer API
    DualStatus = auto()
    PrimalStatus = auto()
    RawStatusString = auto()
    TerminationStatus = auto()
    BarrierIterations = auto()
    DualObjectiveValue = auto()
    NodeCount = auto()
    NumberOfThreads = auto()
    ObjectiveBound = auto()
    ObjectiveValue = auto()
    RelativeGap = auto()
    Silent = auto()
    SimplexIterations = auto()
    SolverName = auto()
    SolverVersion = auto()
    SolverTimeSec = auto()
    TimeLimitSec = auto()
    ObjectiveLimit = auto()


class ResultStatusCode(Enum):
    NO_SOLUTION = auto()
    FEASIBLE_POINT = auto()
    NEARLY_FEASIBLE_POINT = auto()
    INFEASIBLE_POINT = auto()
    INFEASIBILITY_CERTIFICATE = auto()
    NEARLY_INFEASIBILITY_CERTIFICATE = auto()
    REDUCTION_CERTIFICATE = auto()
    NEARLY_REDUCTION_CERTIFICATE = auto()
    UNKNOWN_RESULT_STATUS = auto()
    OTHER_RESULT_STATUS = auto()


class TerminationStatusCode(Enum):
    OPTIMIZE_NOT_CALLED = auto()
    OPTIMAL = auto()
    INFEASIBLE = auto()
    DUAL_INFEASIBLE = auto()
    LOCALLY_SOLVED = auto()
    LOCALLY_INFEASIBLE = auto()
    INFEASIBLE_OR_UNBOUNDED = auto()
    ALMOST_OPTIMAL = auto()
    ALMOST_INFEASIBLE = auto()
    ALMOST_DUAL_INFEASIBLE = auto()
    ALMOST_LOCALLY_SOLVED = auto()
    ITERATION_LIMIT = auto()
    TIME_LIMIT = auto()
    NODE_LIMIT = auto()
    SOLUTION_LIMIT = auto()
    MEMORY_LIMIT = auto()
    OBJECTIVE_LIMIT = auto()
    NORM_LIMIT = auto()
    OTHER_LIMIT = auto()
    SLOW_PROGRESS = auto()
    NUMERICAL_ERROR = auto()
    INVALID_MODEL = auto()
    INVALID_OPTION = auto()
    INTERRUPTED = auto()
    OTHER_ERROR = auto()


model_attr_type_map = {
    ModelAttribute.NumberOfConstraints: int,
    ModelAttribute.NumberOfVariables: int,
    ModelAttribute.Name: str,
    ModelAttribute.ObjectiveSense: ObjectiveSense,
    ModelAttribute.DualStatus: ResultStatusCode,
    ModelAttribute.PrimalStatus: ResultStatusCode,
    ModelAttribute.RawStatusString: str,
    ModelAttribute.TerminationStatus: TerminationStatusCode,
    ModelAttribute.BarrierIterations: int,
    ModelAttribute.DualObjectiveValue: float,
    ModelAttribute.NodeCount: int,
    ModelAttribute.NumberOfThreads: int,
    ModelAttribute.ObjectiveBound: float,
    ModelAttribute.ObjectiveValue: float,
    ModelAttribute.RelativeGap: float,
    ModelAttribute.Silent: bool,
    ModelAttribute.SimplexIterations: int,
    ModelAttribute.SolverName: str,
    ModelAttribute.SolverVersion: str,
    ModelAttribute.SolverTimeSec: float,
    ModelAttribute.TimeLimitSec: float,
    ModelAttribute.ObjectiveLimit: float,
}


def default_model_attribute_type(attribute):
    T = model_attr_type_map.get(attribute, None)
    if not T:
        raise ValueError(f"Unknown model attribute: {attribute}")
    return T
