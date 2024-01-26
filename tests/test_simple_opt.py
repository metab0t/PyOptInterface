import pyoptinterface as poi
from pytest import approx


def test(model_interface):
    model = model_interface

    x = model.add_variable(lb=0.0, ub=20.0)
    y = model.add_variable(lb=8.0, ub=20.0)

    model.set_variable_attribute(x, poi.VariableAttribute.Name, "x")
    model.set_variable_attribute(y, poi.VariableAttribute.Name, "y")

    obj = x * x + y * y
    model.set_objective(obj, poi.ObjectiveSense.Minimize)

    conexpr = x + y
    con1 = model.add_linear_constraint(
        conexpr, poi.ConstraintSense.GreaterEqual, 10.0, name="con1"
    )

    model.optimize()
    status = model.get_model_attribute(poi.ModelAttribute.TerminationStatus)
    assert status == poi.TerminationStatusCode.OPTIMAL
    x_val = model.get_variable_attribute(x, poi.VariableAttribute.Value)
    y_val = model.get_variable_attribute(y, poi.VariableAttribute.Value)
    assert x_val == approx(2.0)
    assert y_val == approx(8.0)
    obj_val = model.get_value(obj)
    assert obj_val == approx(68.0)
    conexpr_val = model.get_value(conexpr)
    assert conexpr_val == approx(10.0)

    assert model.pprint(x) == "x"
    assert model.pprint(y) == "y"
    assert model.pprint(obj) == "1*x*x+1*y*y"
    assert model.pprint(conexpr) == "1*x+1*y"

    model.delete_constraint(con1)
    con2 = model.add_linear_constraint(conexpr, poi.ConstraintSense.GreaterEqual, 20.0)
    model.optimize()
    status = model.get_model_attribute(poi.ModelAttribute.TerminationStatus)
    assert status == poi.TerminationStatusCode.OPTIMAL
    x_val = model.get_variable_attribute(x, poi.VariableAttribute.Value)
    y_val = model.get_variable_attribute(y, poi.VariableAttribute.Value)
    assert x_val == approx(10.0, abs=1e-3)
    assert y_val == approx(10.0, abs=1e-3)

    model.delete_constraint(con2)
    con3 = model.add_linear_constraint(conexpr, poi.ConstraintSense.GreaterEqual, 20.1)
    model.set_variable_attribute(
        x, poi.VariableAttribute.Domain, poi.VariableDomain.Integer
    )
    model.optimize()
    status = model.get_model_attribute(poi.ModelAttribute.TerminationStatus)
    assert status == poi.TerminationStatusCode.OPTIMAL
    x_val = model.get_variable_attribute(x, poi.VariableAttribute.Value)
    y_val = model.get_variable_attribute(y, poi.VariableAttribute.Value)
    assert x_val == approx(10.0)
    assert y_val == approx(10.1)

    model.delete_constraint(con3)
    con4 = model.add_linear_constraint(conexpr, poi.ConstraintSense.GreaterEqual, 10.0)
    model.set_variable_attribute(
        x, poi.VariableAttribute.Domain, poi.VariableDomain.Continuous
    )
    model.set_variable_attribute(y, poi.VariableAttribute.LowerBound, 0.0)
    model.optimize()
    status = model.get_model_attribute(poi.ModelAttribute.TerminationStatus)
    assert status == poi.TerminationStatusCode.OPTIMAL
    x_val = model.get_variable_attribute(x, poi.VariableAttribute.Value)
    y_val = model.get_variable_attribute(y, poi.VariableAttribute.Value)
    assert x_val == approx(5.0)
    assert y_val == approx(5.0)

    model.set_normalized_rhs(con4, 16.0)
    model.optimize()
    status = model.get_model_attribute(poi.ModelAttribute.TerminationStatus)
    assert status == poi.TerminationStatusCode.OPTIMAL
    x_val = model.get_variable_attribute(x, poi.VariableAttribute.Value)
    y_val = model.get_variable_attribute(y, poi.VariableAttribute.Value)
    assert x_val == approx(8.0)
    assert y_val == approx(8.0)
