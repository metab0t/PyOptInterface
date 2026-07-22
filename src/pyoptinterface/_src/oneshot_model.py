from typing import Optional, Tuple, Union, overload

from .core_ext import RawOneShotModel, VariableDomain, VariableIndex
from .comparison_constraint import ComparisonConstraint
from .aml import make_variable_tupledict, make_variable_ndarray


class OneShotModel(RawOneShotModel):
    def __init__(self):
        super().__init__()

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

    add_variables = make_variable_tupledict
    add_m_variables = make_variable_ndarray

    def add_m_variables(
        self,
        shape: Union[Tuple[int, ...], int],
        domain: Optional[VariableDomain] = None,
        lb: Optional[float] = None,
        ub: Optional[float] = None,
        name: Optional[str] = None,
    ):
        import numpy as np

        kw_args = dict()
        if domain is not None:
            kw_args["domain"] = domain
        if lb is not None:
            kw_args["lb"] = lb
        if ub is not None:
            kw_args["ub"] = ub
        if name is not None:
            kw_args["name"] = name

        if isinstance(shape, int):
            shape = (shape,)

        N = int(np.prod(shape))
        index_start = self._batch_add_variables(shape, **kw_args)
        variables = np.empty(N, dtype=object)
        for i in range(N):
            variables[i] = VariableIndex(index_start + i)
        variables = variables.reshape(shape, order="C")

        return variables
