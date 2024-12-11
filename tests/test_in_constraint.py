import pyoptinterface as poi

import pytest


def test_in_constraint_sense(ipopt_model_ctor):
    model = ipopt_model_ctor()

    x = model.add_variable(lb=0.0)
    y = model.add_variable(lb=0.0)

    expr = x + y
    model.add_linear_constraint(expr, poi.In, 0.0, 1.0)
    model.set_objective(expr)
    model.optimize()

    expr_value = model.get_value(expr)
    assert expr_value == pytest.approx(0.0, abs=1e-6)

    model.set_objective(-expr)
    model.optimize()

    expr_value = model.get_value(expr)
    assert expr_value == pytest.approx(1.0, rel=1e-6)
