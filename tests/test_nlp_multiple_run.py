from pyoptinterface import copt, ipopt, nl
import pytest
import math


def test_nlp_reopt(nlp_model_ctor):
    model = nlp_model_ctor()

    x = model.add_variable(lb=0.1)
    y = model.add_variable(lb=0.1)

    with nl.graph():
        model.add_nl_objective(x**2 + y**2)

    with nl.graph():
        model.add_nl_constraint(x**2 <= 1.0)
        model.add_nl_constraint(y**2 <= 1.0)

    model.optimize()

    assert model.get_value(x) == pytest.approx(0.1)
    assert model.get_value(y) == pytest.approx(0.1)

    z = model.add_variable(lb=0.2)
    with nl.graph():
        model.add_nl_objective(z**4)

    model.optimize()

    assert model.get_value(z) == pytest.approx(0.2)

    with nl.graph():
        model.add_nl_constraint(nl.log(z) >= math.log(4.0))

    model.optimize()

    print(model.get_value(z))

    assert model.get_value(z) == pytest.approx(4.0, rel=1e-5)


if __name__ == "__main__":

    def c():
        return ipopt.Model(jit="C")

    test_nlp_reopt(c)

    def llvm():
        return ipopt.Model(jit="LLVM")

    test_nlp_reopt(llvm)

    test_nlp_reopt(copt.Model)
