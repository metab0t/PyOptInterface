import pyoptinterface as poi

import pytest


def test_compare_constraint(model_interface):
    model = model_interface

    x = model.add_variable(lb=0.0)
    y = model.add_variable(lb=0.0)

    def t_expr(expr):
        con = model.add_linear_constraint(expr >= 1.0)
        model.set_objective(expr)
        model.optimize()
        expr_value = model.get_value(expr)
        assert expr_value == pytest.approx(1.0, abs=1e-6)
        model.delete_constraint(con)

        con = model.add_linear_constraint(expr <= 2.0)
        model.set_objective(expr, poi.ObjectiveSense.Maximize)
        model.optimize()
        expr_value = model.get_value(expr)
        assert expr_value == pytest.approx(2.0, abs=1e-6)
        model.delete_constraint(con)

        con = model.add_linear_constraint(expr == 3.0)
        model.set_objective(expr, poi.ObjectiveSense.Maximize)
        model.optimize()
        expr_value = model.get_value(expr)
        assert expr_value == pytest.approx(3.0, abs=1e-6)
        model.delete_constraint(con)

        con = model.add_linear_constraint(1.0 <= expr)
        model.set_objective(expr)
        model.optimize()
        expr_value = model.get_value(expr)
        assert expr_value == pytest.approx(1.0, abs=1e-6)
        model.delete_constraint(con)

        con = model.add_linear_constraint(2.0 >= expr)
        model.set_objective(expr, poi.ObjectiveSense.Maximize)
        model.optimize()
        expr_value = model.get_value(expr)
        assert expr_value == pytest.approx(2.0, abs=1e-6)
        model.delete_constraint(con)

        con = model.add_linear_constraint(3.0 == expr)
        model.set_objective(expr, poi.ObjectiveSense.Maximize)
        model.optimize()
        expr_value = model.get_value(expr)
        assert expr_value == pytest.approx(3.0, abs=1e-6)
        model.delete_constraint(con)

    expr = x + y
    t_expr(expr)

    expr = poi.ExprBuilder(x + y)
    t_expr(expr)

    if hasattr(model, "add_quadratic_constraint"):
        con_expr = x * x + y * y
        expr = x + y

        con = model.add_quadratic_constraint(con_expr <= 18.0)
        model.set_objective(expr, poi.ObjectiveSense.Maximize)
        model.optimize()
        expr_value = model.get_value(expr)
        assert expr_value == pytest.approx(6.0, abs=1e-5)
        model.delete_constraint(con)

        con = model.add_quadratic_constraint(32.0 >= con_expr)
        model.set_objective(expr, poi.ObjectiveSense.Maximize)
        model.optimize()
        expr_value = model.get_value(expr)
        assert expr_value == pytest.approx(8.0, abs=1e-5)
        model.delete_constraint(con)
