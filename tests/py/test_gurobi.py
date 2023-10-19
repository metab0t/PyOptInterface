import pyoptinterface_nb as core
from pyoptinterface_nb import gurobi
from pytest import approx

Model = gurobi.Model


def test_gurobi():
    model = Model()

    x = model.add_variable()
    y = model.add_variable()

    model.set_variable_attribute(x, core.VariableAttribute.LowerBound, 0.0)
    model.set_variable_attribute(x, core.VariableAttribute.UpperBound, 20.0)

    model.set_variable_attribute(y, core.VariableAttribute.LowerBound, 8.0)
    model.set_variable_attribute(y, core.VariableAttribute.UpperBound, 20.0)

    model.set_objective(x * x + y * y, core.ObjectiveSense.Minimize)

    con1 = model.add_linear_constraint(x + y, core.ConstraintSense.GreaterEqual, 10.0)

    model.optimize()

    status = model.get_model_raw_attribute_int("Status")
    assert status == gurobi.GRB.OPTIMAL

    x_val = model.get_variable_attribute(x, core.VariableAttribute.Value)
    y_val = model.get_variable_attribute(y, core.VariableAttribute.Value)
    assert x_val == approx(2.0)
    assert y_val == approx(8.0)

    model.delete_constraint(con1)
    con2 = model.add_linear_constraint(x + y, core.ConstraintSense.GreaterEqual, 20.0)
    model.optimize()

    status = model.get_model_raw_attribute_int("Status")
    assert status == gurobi.GRB.OPTIMAL

    x_val = model.get_variable_attribute(x, core.VariableAttribute.Value)
    y_val = model.get_variable_attribute(y, core.VariableAttribute.Value)
    assert x_val == approx(10.0)
    assert y_val == approx(10.0)

    model.delete_constraint(con2)
    con3 = model.add_linear_constraint(x + y, core.ConstraintSense.GreaterEqual, 20.1)
    model.set_variable_attribute(
        x, core.VariableAttribute.Domain, core.VariableDomain.Integer
    )
    model.optimize()

    status = model.get_model_raw_attribute_int("Status")
    assert status == gurobi.GRB.OPTIMAL

    x_val = model.get_variable_attribute(x, core.VariableAttribute.Value)
    y_val = model.get_variable_attribute(y, core.VariableAttribute.Value)
    assert x_val == approx(10.0)
    assert y_val == approx(10.1)
