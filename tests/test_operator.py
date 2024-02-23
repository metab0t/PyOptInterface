from operator import add, sub, mul, truediv

from pyoptinterface import (
    VariableIndex,
    ExprBuilder,
    ScalarAffineFunction,
    ScalarQuadraticFunction,
)
from pytest import approx


def evaluate(expr, var_value_map=None):
    if isinstance(expr, VariableIndex):
        return var_value_map[expr.index]
    elif isinstance(expr, ScalarAffineFunction):
        s = 0.0
        if expr.constant is not None:
            s = expr.constant
        for index, coef in zip(expr.variables, expr.coefficients):
            val = var_value_map[index]
            s += coef * val
        return s
    elif isinstance(expr, ScalarQuadraticFunction):
        s = 0.0
        if expr.affine_part is not None:
            s = evaluate(expr.affine_part, var_value_map)
        for index1, index2, coef in zip(
            expr.variable_1s, expr.variable_2s, expr.coefficients
        ):
            val1 = var_value_map[index1]
            val2 = var_value_map[index2]
            s += coef * val1 * val2
        return s
    elif isinstance(expr, ExprBuilder):
        degree = expr.degree()
        if degree < 2:
            return evaluate(ScalarAffineFunction(expr), var_value_map)
        elif degree == 2:
            return evaluate(ScalarQuadraticFunction(expr), var_value_map)
        else:
            raise ValueError("Unsupported degree")
    elif isinstance(expr, (int, float)):
        return expr
    else:
        raise ValueError("Unsupported type")


def degree(expr):
    if isinstance(expr, VariableIndex):
        return 1
    elif isinstance(expr, ScalarAffineFunction):
        return 1
    elif isinstance(expr, ScalarQuadraticFunction):
        return 2
    elif isinstance(expr, ExprBuilder):
        return expr.degree()
    elif isinstance(expr, (int, float)):
        return 0
    else:
        raise ValueError("Unsupported type")


def test_evaluate():
    assert evaluate(1.0) == approx(1.0)
    assert evaluate(2) == 2

    N = 6
    vars = [VariableIndex(i) for i in range(N)]
    var_value_map = {v.index: float(v.index) for v in vars}

    for i in range(N):
        assert evaluate(vars[i], var_value_map) == approx(i)

    expr = vars[0] + 1 * vars[1] + 2 * vars[2]
    assert evaluate(expr, var_value_map) == approx(5.0)

    expr = ExprBuilder(expr)
    assert evaluate(expr, var_value_map) == approx(5.0)

    expr = vars[0] + 2 * vars[1] + 5 * vars[3] * vars[2] + 6 * vars[4] * vars[4]
    answer = 2 + 5 * 3 * 2 + 6 * 4 * 4
    assert evaluate(expr, var_value_map) == approx(answer)

    expr = ExprBuilder(expr)
    assert evaluate(expr, var_value_map) == approx(answer)


def test_operator():
    N = 6
    vars = [VariableIndex(i) for i in range(N)]
    var_value_map = {v.index: float(v.index) for v in vars}

    exprs = [
        1,
        2.0,
        vars[4],
        3 * vars[2],
        2 * vars[1] + vars[3],
        2 * vars[2] + vars[3] + 5.0,
        13 * vars[4] * vars[4],
        17 * vars[3] * vars[3] + 1.0,
        17 * vars[3] * vars[5] + 3 * vars[1],
        11 * vars[5] * vars[5] + 7 * vars[1] + 3.0,
    ]
    exprs += [ExprBuilder(e) for e in exprs]

    expr_values = [evaluate(e, var_value_map) for e in exprs]

    for op in [add, sub]:
        for i, ei in enumerate(exprs):
            for j, ej in enumerate(exprs):
                expr = op(ei, ej)
                value = evaluate(expr, var_value_map)
                assert value == approx(op(expr_values[i], expr_values[j]))

    op = mul
    for i, ei in enumerate(exprs):
        for j, ej in enumerate(exprs):
            total_degree = degree(ei) + degree(ej)
            if total_degree > 2:
                continue
            expr = op(ei, ej)
            value = evaluate(expr, var_value_map)
            # flag = value == approx(op(expr_values[i], expr_values[j]))
            # if not flag:
            #     k = 1
            assert value == approx(op(expr_values[i], expr_values[j]))

    op = truediv
    for i, ei in enumerate(exprs):
        for j, ej in enumerate(exprs):
            if not isinstance(ej, (int, float)):
                continue
            expr = op(ei, ej)
            value = evaluate(expr, var_value_map)
            assert value == approx(op(expr_values[i], expr_values[j]))
