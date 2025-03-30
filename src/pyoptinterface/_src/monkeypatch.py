from .core_ext import (
    VariableIndex,
    ScalarAffineFunction,
    ScalarQuadraticFunction,
    ExprBuilder,
    ConstraintSense,
)
from .comparison_constraint import ComparisonConstraint

from .nlexpr_ext import (
    ExpressionHandle,
    BinaryOperator,
    NaryOperator,
    UnaryOperator,
    ArrayType,
)
from .nlfunc import ExpressionGraphContext, convert_to_expressionhandle


def patch_core_compararison_operator(cls):
    def _compare(self, other, op: ConstraintSense):
        if isinstance(
            other,
            (VariableIndex, ScalarAffineFunction, ScalarQuadraticFunction, ExprBuilder),
        ):
            lhs = self - other
            rhs = 0.0
        elif isinstance(other, (int, float)):
            lhs = self
            rhs = other
        else:
            return NotImplemented

        constraint = ComparisonConstraint(op, lhs, rhs)
        return constraint

    def __eq__(self, other):
        return self._compare(other, ConstraintSense.Equal)

    def __le__(self, other):
        return self._compare(other, ConstraintSense.LessEqual)

    def __ge__(self, other):
        return self._compare(other, ConstraintSense.GreaterEqual)

    cls._compare = _compare
    cls.__eq__ = __eq__
    cls.__le__ = __le__
    cls.__ge__ = __ge__


def pow_int(graph, expr, N):
    if N == 0:
        return 1
    if N < 0:
        return graph.add_binary(BinaryOperator.Div, 1, pow_int(graph, expr, -N))
    if N == 1:
        return expr

    M, r = divmod(N, 2)

    pow_2 = graph.add_nary(NaryOperator.Mul, [expr, expr])
    pow_2M = pow_int(graph, pow_2, M)

    if r == 0:
        return pow_2M
    else:
        return graph.add_nary(NaryOperator.Mul, [pow_2M, expr])


def patch_expressionhandle(cls):
    def _binary_operator(self, other, op: BinaryOperator, swap=False):
        graph = ExpressionGraphContext.current_graph()
        other = convert_to_expressionhandle(graph, other)
        if not isinstance(other, ExpressionHandle):
            return NotImplemented

        if swap:
            new_expression = graph.add_binary(op, other, self)
        else:
            new_expression = graph.add_binary(op, self, other)
        return new_expression

    def _nary_operator(self, other, op: NaryOperator):
        graph = ExpressionGraphContext.current_graph()
        other = convert_to_expressionhandle(graph, other)
        if not isinstance(other, ExpressionHandle):
            return NotImplemented

        if self.array == ArrayType.Nary and graph.get_nary_operator(self) == op:
            graph.append_nary(self, other)
            return self
        else:
            new_expression = graph.add_nary(op, [self, other])
            return new_expression

    def __add__(self, other):
        return self._nary_operator(other, NaryOperator.Add)

    def __radd__(self, other):
        return self._nary_operator(other, NaryOperator.Add)

    def __sub__(self, other):
        return self._binary_operator(other, BinaryOperator.Sub)

    def __rsub__(self, other):
        return self._binary_operator(other, BinaryOperator.Sub, swap=True)

    def __mul__(self, other):
        return self._nary_operator(other, NaryOperator.Mul)

    def __rmul__(self, other):
        return self._nary_operator(other, NaryOperator.Mul)

    def __truediv__(self, other):
        return self._binary_operator(other, BinaryOperator.Div)

    def __rtruediv__(self, other):
        return self._binary_operator(other, BinaryOperator.Div, swap=True)

    def __neg__(self):
        graph = ExpressionGraphContext.current_graph()
        new_expression = graph.add_unary(UnaryOperator.Neg, self)
        return new_expression

    def __pow__(self, other):
        # special case for integer powers, which can be implemented as a repeated multiplication
        if isinstance(other, int):
            graph = ExpressionGraphContext.current_graph()
            new_expression = pow_int(graph, self, other)
            return new_expression
        else:
            return self._binary_operator(other, BinaryOperator.Pow)

    def __rpow__(self, other):
        return self._binary_operator(other, BinaryOperator.Pow, swap=True)

    # compare operators
    def __eq__(self, other):
        return self._binary_operator(other, BinaryOperator.Equal)

    def __ne__(self, other):
        return self._binary_operator(other, BinaryOperator.NotEqual)

    def __lt__(self, other):
        return self._binary_operator(other, BinaryOperator.LessThan)

    def __le__(self, other):
        return self._binary_operator(other, BinaryOperator.LessEqual)

    def __gt__(self, other):
        return self._binary_operator(other, BinaryOperator.GreaterThan)

    def __ge__(self, other):
        return self._binary_operator(other, BinaryOperator.GreaterEqual)

    cls._binary_operator = _binary_operator
    cls._nary_operator = _nary_operator
    cls.__add__ = __add__
    cls.__radd__ = __radd__
    cls.__sub__ = __sub__
    cls.__rsub__ = __rsub__
    cls.__mul__ = __mul__
    cls.__rmul__ = __rmul__
    cls.__truediv__ = __truediv__
    cls.__rtruediv__ = __rtruediv__
    cls.__neg__ = __neg__
    cls.__pow__ = __pow__
    cls.__rpow__ = __rpow__
    cls.__eq__ = __eq__
    cls.__ne__ = __ne__
    cls.__lt__ = __lt__
    cls.__le__ = __le__
    cls.__gt__ = __gt__
    cls.__ge__ = __ge__


def _monkeypatch_all():
    patch_core_compararison_operator(VariableIndex)
    patch_core_compararison_operator(ScalarAffineFunction)
    patch_core_compararison_operator(ScalarQuadraticFunction)
    patch_core_compararison_operator(ExprBuilder)

    patch_expressionhandle(ExpressionHandle)
