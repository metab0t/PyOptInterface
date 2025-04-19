import math
import pytest

import pyoptinterface as poi
from pyoptinterface import ipopt, nl


def test_easy_nlp(nlp_model_ctor):
    model = nlp_model_ctor()

    x = model.add_variable(lb=0.1, ub=10.0)
    y = model.add_variable(lb=0.1, ub=10.0)

    model.add_linear_constraint(x + y, poi.Eq, 1.0)

    with nl.graph():
        model.add_nl_objective(nl.exp(x) + nl.exp(y))

    with nl.graph():
        z = x * x
        s = y * y
        model.add_nl_constraint(z, (0.36, 4.0))
        model.add_nl_constraint(s, (0.04, 4.0))

    nl_funcs = [
        nl.abs,
        nl.acos,
        nl.asin,
        nl.atan,
        nl.cos,
        nl.exp,
        nl.log,
        nl.log10,
        nl.pow,
        nl.sin,
        nl.sqrt,
        nl.tan,
    ]

    B = 1e10
    all_nlfuncs_con = []
    with nl.graph():
        for f in nl_funcs:
            if f == nl.pow:
                v = f(x, 2)
            else:
                v = f(x)
            con = model.add_nl_constraint(v, (-B, B))
            all_nlfuncs_con.append(con)

    model.optimize()

    termination_status = model.get_model_attribute(poi.ModelAttribute.TerminationStatus)
    assert (
        termination_status == poi.TerminationStatusCode.LOCALLY_SOLVED
        or termination_status == poi.TerminationStatusCode.OPTIMAL
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
        if f == nl.pow:
            correct_con_values.append(f(x_value, 2))
        else:
            correct_con_values.append(f(x_value))

    assert con_values == pytest.approx(correct_con_values)


def test_nlfunc_ifelse(nlp_model_ctor):
    if nlp_model_ctor is not ipopt.Model:
        pytest.skip("ifelse is only supported in IPOPT")

    for x_, fx in zip([0.2, 0.5, 1.0, 2.0, 3.0], [0.2, 0.5, 1.0, 4.0, 9.0]):
        model = nlp_model_ctor()

        x = model.add_variable(lb=0.0, ub=10.0)

        with nl.graph():
            y = nl.ifelse(x > 1.0, x**2, x)
            model.add_nl_constraint(y, poi.Geq, fx)

        model.set_objective(x)

        model.optimize()

        x_value = model.get_value(x)
        assert x_value == pytest.approx(x_)


if __name__ == "__main__":

    def c():
        return ipopt.Model(jit="C")

    test_easy_nlp(c)
    test_nlfunc_ifelse(c)

    def llvm():
        return ipopt.Model(jit="LLVM")

    test_easy_nlp(llvm)
    test_nlfunc_ifelse(llvm)
