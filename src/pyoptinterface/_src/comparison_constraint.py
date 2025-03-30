from dataclasses import dataclass
from typing import Any

from .core_ext import ConstraintSense


@dataclass
class ComparisonConstraint:
    sense: ConstraintSense
    lhs: Any
    rhs: Any
