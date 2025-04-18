import math
import pytest

import pyoptinterface as poi
from pyoptinterface import ipopt, nlfunc


def test_ipopt(nlp_model_ctor):
    model = nlp_model_ctor()

    x = model.add_variable(lb=0.1, ub=10.0, start=0.65)
    y = model.add_variable(lb=0.1, ub=10.0, start=0.35)

    model.add_linear_constraint(x + y, poi.Eq, 1.0)

    with nlfunc.graph():
        model.add_nl_objective(nlfunc.exp(x) + nlfunc.exp(y))

    with nlfunc.graph():
        z = x * x
        s = y * y
        model.add_nl_constraint(z, lb=0.36, ub=4.0)
        model.add_nl_constraint(s, lb=0.04, ub=4.0)

    nl_funcs = [
        nlfunc.abs,
        nlfunc.acos,
        nlfunc.asin,
        nlfunc.atan,
        nlfunc.cos,
        nlfunc.exp,
        nlfunc.log,
        nlfunc.log10,
        nlfunc.pow,
        nlfunc.sin,
        nlfunc.sqrt,
        nlfunc.tan,
    ]

    B = 1e10
    all_nlfuncs_con = []
    with nlfunc.graph():
        for f in nl_funcs:
            if f == nlfunc.pow:
                v = f(x, 2)
            else:
                v = f(x)
            con = model.add_nl_constraint(v, lb=-B, ub=B)
            all_nlfuncs_con.append(con)

    model.optimize()

    assert (
        model.get_model_attribute(poi.ModelAttribute.TerminationStatus)
        == poi.TerminationStatusCode.LOCALLY_SOLVED
    )

    x_value = model.get_value(x)
    y_value = model.get_value(y)
    assert x_value == pytest.approx(0.6)
    assert y_value == pytest.approx(0.4)

    obj_value = model.get_model_attribute(poi.ModelAttribute.ObjectiveValue)
    assert obj_value == pytest.approx(math.exp(x_value) + math.exp(y_value))

    con_values = [
        model.get_constraint_attribute(con, poi.ConstraintAttribute.Primal)
        for con in all_nlfuncs_con
    ]

    correct_con_values = []
    for f in nl_funcs:
        if f == nlfunc.pow:
            correct_con_values.append(f(x_value, 2))
        else:
            correct_con_values.append(f(x_value))

    assert con_values == pytest.approx(correct_con_values)


def test_nlfunc_ifelse(nlp_model_ctor):
    for x_, fx in zip([0.2, 0.5, 1.0, 2.0, 3.0], [0.2, 0.5, 1.0, 4.0, 9.0]):
        model = nlp_model_ctor()

        x = model.add_variable(lb=0.0, ub=10.0, start=1.0)

        with nlfunc.graph():
            y = nlfunc.ifelse(x > 1.0, x**2, x)
            model.add_nl_constraint(y, lb=fx)

        model.set_objective(x)

        model.optimize()

        x_value = model.get_value(x)
        assert x_value == pytest.approx(x_)


if __name__ == "__main__":

    def c():
        return ipopt.Model(jit="C")

    test_ipopt(c)
    test_nlfunc_ifelse(c)

    def llvm():
        return ipopt.Model(jit="LLVM")

    test_ipopt(llvm)
    test_nlfunc_ifelse(llvm)
