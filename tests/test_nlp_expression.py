import pyoptinterface as poi
from pyoptinterface import nlfunc
import math
import pytest
from pytest import approx


def test_nlp_expressiontree(model_interface):
    model = model_interface

    if not hasattr(model, "add_nl_constraint"):
        pytest.skip("Model interface does not support nonlinear constraint")

    x = model.add_variable(lb=0.0, ub=2.0)
    y = model.add_variable(lb=0.0, ub=2.0)

    with nlfunc.graph():
        z = nlfunc.exp(x) + nlfunc.exp(2 * y)
        model.add_nl_constraint(z <= 2 * math.exp(1.0))

    model.set_objective(x + 2 * y, poi.ObjectiveSense.Maximize)
    model.optimize()

    x_value = model.get_value(x)
    y_value = model.get_value(y)

    assert x_value == approx(1.0, rel=1e-6)
    assert y_value == approx(0.5, rel=1e-6)
