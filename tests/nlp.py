import math
import pytest

import pyoptinterface as poi
from pyoptinterface import ipopt

IPOPT_PATH = r"D:\Ipopt\bin\ipopt-3.dll"

if not ipopt.load_library(IPOPT_PATH):
    exit(1)


def test_ipopt():
    model = ipopt.Model()

    x = model.add_variable(lb=0.0, ub=10.0, start=0.8)
    y = model.add_variable(lb=0.0, ub=10.0, start=0.5)

    model.add_linear_constraint(x + y, poi.Eq, 1.0)

    def obj(vars):
        x = vars["x"]
        y = vars["y"]
        return poi.exp(x) + poi.exp(y)

    obj_f = model.register_function(obj, var=["x", "y"], name="obj")
    model.add_nl_objective(obj_f, [x, y])

    def con(vars):
        x = vars["x"]
        y = vars["y"]

        z = x * x
        s = y * y

        return [z, s]

    con_f = model.register_function(con, ["x", "y"], name="con")
    model.add_nl_constraint(con_f, [x, y], poi.In, lb=[0.36, 0.04], ub=[4.0, 4.0])

    nl_funcs = [
        poi.abs,
        poi.acos,
        poi.acosh,
        poi.asin,
        poi.asinh,
        poi.atan,
        poi.atanh,
        poi.cos,
        poi.cosh,
        poi.erf,
        poi.erfc,
        poi.exp,
        poi.expm1,
        poi.log1p,
        poi.log,
        poi.pow,
        poi.sin,
        poi.sinh,
        poi.sqrt,
        poi.tan,
        poi.tanh,
    ]
    py_funcs = [
        abs,
        math.acos,
        math.acosh,
        math.asin,
        math.asinh,
        math.atan,
        math.atanh,
        math.cos,
        math.cosh,
        math.erf,
        math.erfc,
        math.exp,
        math.expm1,
        math.log1p,
        math.log,
        math.pow,
        math.sin,
        math.sinh,
        math.sqrt,
        math.tan,
        math.tanh,
    ]

    def all_nlfuncs(vars):
        x = vars["x"]

        values = []
        for f in nl_funcs:
            if f == poi.acosh:
                v = f(x + 1)
            elif f == poi.pow:
                v = f(x, x)
            else:
                v = f(x)
            values.append(v)

        return values

    all_nlfuncs_f = model.register_function(all_nlfuncs, var=["x"], name="all_nlfuncs")
    N = len(nl_funcs)
    B = 1e10
    all_nlfuncs_con = model.add_nl_constraint(all_nlfuncs_f, [x], poi.In, lb=[-B] * N, ub=[B] * N)

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

    con_values = model.get_constraint_attribute(all_nlfuncs_con, poi.ConstraintAttribute.Primal)
    
    correct_con_values = []
    for f in py_funcs:
        if f == math.acosh:
            v = f(x_value + 1)
        elif f == math.pow:
            v = f(x_value, x_value)
        else:
            v = f(x_value)
        correct_con_values.append(v)
    assert con_values == pytest.approx(correct_con_values)


if __name__ == "__main__":
    test_ipopt()
