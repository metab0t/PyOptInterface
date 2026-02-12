import pyoptinterface as poi
from pytest import approx


def test_update(model_interface):
    model = model_interface

    x = model.add_variables(range(3), lb=1.0, ub=2.0)

    model.delete_variable(x[1])

    expr = x[0] - x[2]
    con = model.add_linear_constraint(expr, poi.Eq, 0.0)

    obj = x[0] - x[2]
    model.set_objective(obj)

    model.optimize()

    assert model.get_value(obj) == approx(0.0)

    model.delete_constraint(con)
    model.set_variable_attribute(x[0], poi.VariableAttribute.LowerBound, 2.0)
    assert model.get_variable_attribute(
        x[0], poi.VariableAttribute.LowerBound
    ) == approx(2.0)

    model.optimize()

    assert model.get_value(x[0]) == approx(2.0)
    assert model.get_value(x[2]) == approx(2.0)

    con = model.add_linear_constraint(expr, poi.Eq, 0.0)
    model.set_normalized_coefficient(con, x[2], -2.0)

    model.optimize()

    assert model.get_value(x[0]) == approx(2.0)
    assert model.get_value(x[2]) == approx(1.0)

    model.set_variable_attribute(x[0], poi.VariableAttribute.LowerBound, 1.5)
    model.set_variable_attribute(x[2], poi.VariableAttribute.LowerBound, 0.5)
    model.set_objective_coefficient(x[0], 2.0)
    model.set_objective_coefficient(x[2], 1.0)
    model.optimize()

    assert model.get_value(x[0]) == approx(1.5)
    assert model.get_value(x[2]) == approx(0.75)
