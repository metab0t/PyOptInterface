import math
import pytest

import pyoptinterface as poi
from pyoptinterface import ipopt, nlfunc


def test_ipopt(ipopt_model_ctor):
    model = ipopt_model_ctor()

    x = model.add_variable(lb=0.1, ub=10.0, start=0.65)
    y = model.add_variable(lb=0.1, ub=10.0, start=0.35)

    model.add_linear_constraint(x + y, poi.Eq, 1.0)

    def obj(vars):
        return nlfunc.exp(vars.x) + nlfunc.exp(vars.y)

    obj_f = model.register_function(obj)
    model.add_nl_objective(obj_f, vars=nlfunc.Vars(x=x, y=y))

    def con(vars):
        x = vars.x
        y = vars.y

        z = x * x
        s = y * y

        return [z, s]

    con_f = model.register_function(con)
    model.add_nl_constraint(
        con_f, vars=nlfunc.Vars(x=x, y=y), lb=[0.36, 0.04], ub=[4.0, 4.0]
    )

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

    def all_nlfuncs(vars):
        x = vars.x

        values = []
        for f in nl_funcs:
            if f == nlfunc.pow:
                v = f(x, 2)
            else:
                v = f(x)
            values.append(v)

        return values

    all_nlfuncs_f = model.register_function(all_nlfuncs)
    N = len(nl_funcs)
    B = 1e10
    all_nlfuncs_con = model.add_nl_constraint(
        all_nlfuncs_f, vars=nlfunc.Vars(x=x), lb=[-B] * N, ub=[B] * N
    )

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

    con_values = model.get_constraint_attribute(
        all_nlfuncs_con, poi.ConstraintAttribute.Primal
    )

    vars = nlfunc.Vars(x=x_value)
    correct_con_values = all_nlfuncs(vars)

    assert con_values == pytest.approx(correct_con_values)


def test_nlp_param(ipopt_model_ctor):
    model = ipopt_model_ctor()

    N = 10
    xs = []
    for i in range(N):
        x = model.add_variable(lb=0.0, ub=10.0, start=1.0)
        xs.append(x)

    def obj(vars):
        return nlfunc.exp(vars.x)

    obj_f = model.register_function(obj)

    for i in range(N):
        model.add_nl_objective(obj_f, vars=nlfunc.Vars(x=xs[i]))

    def con(vars, params):
        x = vars.x
        p = params.p
        return x * (p + 1) * (p + 1)

    con_f = model.register_function(con)

    for i in range(N):
        model.add_nl_constraint(
            con_f, vars=nlfunc.Vars(x=xs[i]), params=nlfunc.Params(p=i), lb=[1.0]
        )

    model.optimize()

    assert (
        model.get_model_attribute(poi.ModelAttribute.TerminationStatus)
        == poi.TerminationStatusCode.LOCALLY_SOLVED
    )

    x_values = [model.get_value(x) for x in xs]
    correct_x_values = [1.0 / (i + 1) / (i + 1) for i in range(N)]

    assert x_values == pytest.approx(correct_x_values)


def test_nlfunc_ifelse(ipopt_model_ctor):
    for x_, fx in zip([0.2, 0.5, 1.0, 2.0, 3.0], [0.2, 0.5, 1.0, 4.0, 9.0]):
        model = ipopt_model_ctor()

        x = model.add_variable(lb=0.0, ub=10.0, start=1.0)

        def con(vars):
            x = vars.x
            return nlfunc.ifelse(x > 1.0, x**2, x)

        con_f = model.register_function(con)
        model.add_nl_constraint(con_f, vars=nlfunc.Vars(x=x), lb=[fx])

        model.set_objective(x)

        model.optimize()

        x_value = model.get_value(x)
        assert x_value == pytest.approx(x_)


if __name__ == "__main__":

    def c():
        return ipopt.Model(jit="C")

    test_ipopt(c)
    test_nlp_param(c)
    test_nlfunc_ifelse(c)

    def llvm():
        return ipopt.Model(jit="LLVM")

    test_ipopt(llvm)
    test_nlp_param(llvm)
    test_nlfunc_ifelse(llvm)
