from pyoptinterface_nb import core
from pyoptinterface_nb import gurobi
from pytest import approx

Model = gurobi.Model


def test_gurobi():
    env = gurobi.Env()
    model = Model(env)

    x = model.add_variable()
    y = model.add_variable()

    model.set_variable_attribute_double(x, core.VariableAttribute.LowerBound, 0.0)
    model.set_variable_attribute_double(x, core.VariableAttribute.UpperBound, 20.0)

    model.set_variable_attribute_double(y, core.VariableAttribute.LowerBound, 8.0)
    model.set_variable_attribute_double(y, core.VariableAttribute.UpperBound, 20.0)

    model.set_objective(x * x + y * y, core.ObjectiveSense.Minimize)

    con1 = model.add_linear_constraint(x + y, core.ConstraintSense.GreaterEqual, 10.0)

    model.optimize()

    status = model.get_model_raw_attribute_int("Status")
    assert status == gurobi.GRB.OPTIMAL

    x_val = model.get_variable_attribute_double(x, core.VariableAttribute.Value)
    y_val = model.get_variable_attribute_double(y, core.VariableAttribute.Value)
    assert x_val == approx(2.0)
    assert y_val == approx(8.0)

    model.delete_constraint(con1)
    con2 = model.add_linear_constraint(x + y, core.ConstraintSense.GreaterEqual, 20.0)
    model.optimize()

    status = model.get_model_raw_attribute_int("Status")
    assert status == gurobi.GRB.OPTIMAL

    x_val = model.get_variable_attribute_double(x, core.VariableAttribute.Value)
    y_val = model.get_variable_attribute_double(y, core.VariableAttribute.Value)
    assert x_val == approx(10.0)
    assert y_val == approx(10.0)
