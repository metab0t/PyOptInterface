import pytest

import pyoptinterface as poi
from pyoptinterface import ipopt

IPOPT_PATH = r"D:\Ipopt\bin\ipopt-3.dll"

if not ipopt.load_library(IPOPT_PATH):
    exit(1)


def test_ipopt():
    model = ipopt.Model()

    x = model.add_variable(lb=0.0, ub=10.0, start=2.0)
    y = model.add_variable(lb=0.0, ub=10.0, start=2.0)

    model.add_linear_constraint(x + y, poi.Eq, 2.0)

    def obj(vars):
        x = vars["x"]
        y = vars["y"]
        return poi.exp(x) + poi.exp(y)

    obj_f = model.register_function(obj, x=["x", "y"], name="obj")
    model.add_nl_objective(obj_f, [x, y])

    def con(vars):
        x = vars["x"]
        y = vars["y"]

        z = x * x
        s = y * y

        return [z, s]

    con_f = model.register_function(con, ["x", "y"], name="con")
    model.add_nl_constraint(con_f, [x, y], poi.In, lb=[1.21, 0.5], ub=[4.0, 4.0])

    model.optimize()

    assert (
        model.get_model_attribute(poi.ModelAttribute.TerminationStatus)
        == poi.TerminationStatusCode.LOCALLY_SOLVED
    )

    assert model.get_value(x) == pytest.approx(1.1)
    assert model.get_value(y) == pytest.approx(0.9)


if __name__ == "__main__":
    test_ipopt()
