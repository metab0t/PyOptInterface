import pyoptinterface as poi
from pyoptinterface import nl, gurobi

import pytest


def test_linear_in_constraint(model_interface_oneshot):
    model = model_interface_oneshot

    if isinstance(model, gurobi.Model):
        pytest.skip("Gurobi does not support range linear constraints")

    x = model.add_variable(lb=0.0)
    y = model.add_variable(lb=0.0)

    expr = x + y
    model.add_linear_constraint(expr, (0, 1.0))
    model.set_objective(expr)
    model.optimize()

    expr_value = model.get_value(expr)
    assert expr_value == pytest.approx(0.0, abs=1e-6)

    model.set_objective(-expr)
    model.optimize()

    expr_value = model.get_value(expr)
    assert expr_value == pytest.approx(1.0, rel=1e-6)


def test_nl_in_constraint(nlp_model_ctor):
    model = nlp_model_ctor()

    x = model.add_variable(lb=0.0)
    y = model.add_variable(lb=0.0)

    with nl.graph():
        expr = nl.exp(x) + nl.exp(y)
        model.add_nl_constraint(expr, (5.0, 10.0))
        model.add_nl_objective(expr)

    model.optimize()
    objective_value = model.get_model_attribute(poi.ModelAttribute.ObjectiveValue)
    assert objective_value == pytest.approx(5.0, abs=1e-6)
