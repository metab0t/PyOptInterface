import os
import platform
from pathlib import Path
import re
import logging

from .mindopt_model_ext import RawModel, RawEnv, load_library
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
    _direct_get_entity_attribute,
    _set_entity_attribute,
    _direct_set_entity_attribute,
)
from .constraint_bridge import bridge_soc_quadratic_constraint
from .aml import make_variable_tupledict, make_variable_ndarray
from .matrix import add_matrix_constraints
