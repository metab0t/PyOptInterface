from pyoptinterface import ipopt, nlfunc
import pytest
import math


def test_nlp_reopt(ipopt_model_ctor):
    model = ipopt_model_ctor()

    x = model.add_variable(lb=0.1)
    y = model.add_variable(lb=0.1)

    def obj(vars):
        return nlfunc.exp(vars.x) + nlfunc.exp(vars.y)

    obj_f = model.register_function(obj)
    model.add_nl_objective(obj_f, vars=nlfunc.Vars(x=x, y=y))

    def con(vars):
        x = vars.x
        return x**4

    con_f = model.register_function(con)
    model.add_nl_constraint(con_f, vars=nlfunc.Vars(x=x), lb=1.0)
    model.add_nl_constraint(con_f, vars=nlfunc.Vars(x=y), lb=1.0)

    model.optimize()

    assert model.get_value(x) == pytest.approx(1.0)
    assert model.get_value(y) == pytest.approx(1.0)

    z = model.add_variable(lb=0.2)
    model.add_nl_objective(con_f, vars=nlfunc.Vars(x=z))

    model.optimize()

    assert model.get_value(z) == pytest.approx(0.2)

    def con2(vars):
        x = vars.x
        return nlfunc.log(x)

    con2_f = model.register_function(con2)
    model.add_nl_constraint(con2_f, vars=nlfunc.Vars(x=z), lb=math.log(4.0))

    model.optimize()

    assert model.get_value(z) == pytest.approx(4.0)


if __name__ == "__main__":

    def c():
        return ipopt.Model(jit="C")

    test_nlp_reopt(c)

    def llvm():
        return ipopt.Model(jit="LLVM")

    test_nlp_reopt(llvm)
