from pyoptinterface_nb import core
from pyoptinterface_nb import gurobi
from pytest import approx

Model = gurobi.Model


def test_gurobi():
    env = gurobi.Env()
    model = Model(env)

    x = model.add_variable()
    y = model.add_variable()

    model.set_variable_raw_attribute_double(x, "LB", 0.0)
    model.set_variable_raw_attribute_double(x, "UB", 20.0)

    model.set_variable_raw_attribute_double(y, "LB", 8.0)
    model.set_variable_raw_attribute_double(y, "UB", 20.0)

    model.set_objective(x * x + y * y, core.ObjectiveSense.Minimize)

    con1 = model.add_linear_constraint(x + y, core.ConstraintSense.GreaterEqual, 10.0)

    model.optimize()

    status = model.get_model_raw_attribute_int("Status")
    assert status == gurobi.GRB.OPTIMAL

    x_val = model.get_variable_raw_attribute_double(x, "X")
    y_val = model.get_variable_raw_attribute_double(y, "X")
    assert x_val == approx(2.0)
    assert y_val == approx(8.0)

    model.delete_constraint(con1)
    con2 = model.add_linear_constraint(x + y, core.ConstraintSense.GreaterEqual, 20.0)
    model.optimize()

    status = model.get_model_raw_attribute_int("Status")
    assert status == gurobi.GRB.OPTIMAL

    x_val = model.get_variable_raw_attribute_double(x, "X")
    y_val = model.get_variable_raw_attribute_double(y, "X")
    assert x_val == approx(10.0)
    assert y_val == approx(10.0)
