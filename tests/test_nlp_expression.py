import pyoptinterface as poi
from pyoptinterface import nl
import math
import pytest
from pytest import approx


def test_nlp_expressiontree(model_interface):
    model = model_interface

    if not hasattr(model, "add_nl_constraint"):
        pytest.skip("Model interface does not support nonlinear constraint")

    x = model.add_variable(lb=0.0, ub=2.0)
    y = model.add_variable(lb=0.0, ub=2.0)

    with nl.graph():
        z = nl.exp(x) + nl.exp(2 * y)
        model.add_nl_constraint(z <= 2 * math.exp(1.0))

        z = x * x * x
        model.add_nl_constraint(z >= 0.8**3)

    model.set_objective(x + 2 * y, poi.ObjectiveSense.Maximize)
    model.optimize()

    x_value = model.get_value(x)
    y_value = model.get_value(y)

    # Note: with a feasibility tolerance defaulted to 1e-6 + the
    # effect of the internal solver scaling, x and y can assume
    # values relatively far away from the expected ones.
    # E.g.: x = 1.0005, y = 0.49975
    assert x_value == approx(1.0, rel=1e-2)
    assert y_value == approx(0.5, rel=1e-2)


def test_nlp_expressiontree_obj(model_interface):
    model = model_interface

    if not hasattr(model, "add_nl_constraint"):
        pytest.skip("Model interface does not support nonlinear constraint")
    if not hasattr(model, "add_nl_objective"):
        pytest.skip("Model interface does not support nonlinear objective")

    x = model.add_variable(lb=-2.0, ub=2.0)
    y = model.add_variable(lb=-2.0, ub=2.0)

    with nl.graph():
        z = x**2 + y**2
        model.add_nl_constraint(z, (-1.0, 1.0))

    with nl.graph():
        z = nl.exp(x**2) + nl.exp(y**2)
        model.add_nl_objective(z)

    model.optimize()

    x_value = model.get_value(x)
    y_value = model.get_value(y)

    assert x_value == approx(0.0, abs=1e-4)
    assert y_value == approx(0.0, abs=1e-4)
