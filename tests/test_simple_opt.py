import pyoptinterface as poi
from pytest import approx
import pytest


def test_simple_opt(model_interface):
    model = model_interface

    x = model.add_variable(lb=0.0, ub=20.0)
    y = model.add_variable()
    model.set_variable_bounds(y, 8.0, 20.0)

    model.set_variable_attribute(x, poi.VariableAttribute.Name, "x")
    model.set_variable_attribute(y, poi.VariableAttribute.Name, "y")

    obj = x * x + y * y
    model.set_objective(obj, poi.ObjectiveSense.Minimize)

    conexpr = x + y
    con1 = model.add_linear_constraint(
        conexpr - 10.0, poi.ConstraintSense.GreaterEqual, 0.0, name="con1"
    )

    assert model.number_of_variables() == 2
    assert model.number_of_constraints(poi.ConstraintType.Linear) == 1

    model.optimize()
    status = model.get_model_attribute(poi.ModelAttribute.TerminationStatus)
    assert status == poi.TerminationStatusCode.OPTIMAL
    x_val = model.get_variable_attribute(x, poi.VariableAttribute.Value)
    y_val = model.get_variable_attribute(y, poi.VariableAttribute.Value)
    assert x_val == approx(2.0)
    assert y_val == approx(8.0)
    obj_val = model.get_value(obj)
    assert obj_val == approx(68.0)
    obj_val_attr = model.get_model_attribute(poi.ModelAttribute.ObjectiveValue)
    assert obj_val_attr == approx(obj_val)
    conexpr_val = model.get_value(conexpr)
    assert conexpr_val == approx(10.0)

    assert model.pprint(x) == "x"
    assert model.pprint(y) == "y"
    assert model.pprint(obj) == "1*x*x+1*y*y"
    assert model.pprint(conexpr) == "1*x+1*y"

    model.delete_constraint(con1)
    assert model.number_of_constraints(poi.ConstraintType.Linear) == 0
    con2 = model.add_linear_constraint(conexpr, poi.ConstraintSense.GreaterEqual, 20.0)
    assert model.number_of_constraints(poi.ConstraintType.Linear) == 1
    model.optimize()
    status = model.get_model_attribute(poi.ModelAttribute.TerminationStatus)
    assert status == poi.TerminationStatusCode.OPTIMAL
    x_val = model.get_variable_attribute(x, poi.VariableAttribute.Value)
    y_val = model.get_variable_attribute(y, poi.VariableAttribute.Value)
    assert x_val == approx(10.0, abs=1e-3)
    assert y_val == approx(10.0, abs=1e-3)

    model.delete_constraint(con2)
    con3 = model.add_linear_constraint(conexpr, poi.ConstraintSense.GreaterEqual, 20.05)
    model.set_variable_attribute(
        x, poi.VariableAttribute.Domain, poi.VariableDomain.Integer
    )
    model.set_objective(x + 2 * y, poi.ObjectiveSense.Minimize)
    model.optimize()
    status = model.get_model_attribute(poi.ModelAttribute.TerminationStatus)
    assert status == poi.TerminationStatusCode.OPTIMAL
    x_val = model.get_variable_attribute(x, poi.VariableAttribute.Value)
    y_val = model.get_variable_attribute(y, poi.VariableAttribute.Value)
    assert x_val == approx(12.0)
    assert y_val == approx(8.05)

    model.delete_constraint(con3)
    con4 = model.add_linear_constraint(conexpr, poi.ConstraintSense.GreaterEqual, 10.0)
    model.set_variable_attribute(
        x, poi.VariableAttribute.Domain, poi.VariableDomain.Continuous
    )
    model.set_variable_attribute(y, poi.VariableAttribute.LowerBound, 0.0)
    model.set_objective(obj, poi.ObjectiveSense.Minimize)
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


def test_constant_objective(model_interface_oneshot):
    model = model_interface_oneshot

    x = model.add_variable(lb=0.0, ub=1.0)
    obj = 1.0
    model.set_objective(obj, poi.ObjectiveSense.Minimize)
    model.optimize()
    obj_val = model.get_model_attribute(poi.ModelAttribute.ObjectiveValue)
    assert obj_val == approx(1.0)
    model.set_objective(obj, poi.ObjectiveSense.Maximize)
    model.optimize()
    status = model.get_model_attribute(poi.ModelAttribute.TerminationStatus)
    assert (
        status == poi.TerminationStatusCode.OPTIMAL
        or status == poi.TerminationStatusCode.LOCALLY_SOLVED
    )
    obj_val = model.get_model_attribute(poi.ModelAttribute.ObjectiveValue)
    assert obj_val == approx(1.0)


def test_constraint_primal_dual(model_interface_oneshot):
    model = model_interface_oneshot

    x = model.add_variable(lb=0.0, ub=1.0)
    y = model.add_variable(lb=0.0, ub=1.0)

    model.set_variable_attribute(x, poi.VariableAttribute.Name, "x")
    model.set_variable_attribute(y, poi.VariableAttribute.Name, "y")

    obj = x + y
    model.set_objective(obj, poi.ObjectiveSense.Minimize)

    conexpr = x + 2 * y
    con1 = model.add_linear_constraint(conexpr, poi.Geq, 1.0, name="con1")

    model.optimize()
    status = model.get_model_attribute(poi.ModelAttribute.TerminationStatus)
    assert (
        status == poi.TerminationStatusCode.OPTIMAL
        or status == poi.TerminationStatusCode.LOCALLY_SOLVED
    )

    primal_val = model.get_constraint_attribute(con1, poi.ConstraintAttribute.Primal)
    assert primal_val == approx(1.0)

    dual_val = model.get_constraint_attribute(con1, poi.ConstraintAttribute.Dual)
    assert dual_val == approx(0.5)


def test_add_quadratic_expr_as_linear_throws_error(model_interface_oneshot):
    model = model_interface_oneshot

    xs = model.add_m_variables(10)
    x2_sum = poi.quicksum(x * x for x in xs.flat)

    with pytest.raises(RuntimeError, match="add_linear_constraint"):
        model.add_linear_constraint(x2_sum <= 1.0)


def test_exprbuilder_self_operation(model_interface_oneshot):
    model = model_interface_oneshot

    x = model.add_m_variables(2, lb=1.0, ub=4.0)

    expr = poi.ExprBuilder(x[0] + 2.0 * x[1] + 3.0)
    expr += expr
    model.set_objective(expr)
    model.optimize()
    obj_value = model.get_value(expr)
    assert obj_value == approx(12.0)

    expr = poi.ExprBuilder(x[0] + 2.0 * x[1] + 3.0)
    expr -= expr
    model.set_objective(expr)
    model.optimize()
    obj_value = model.get_value(expr)
    assert obj_value == approx(0.0)

    expr = poi.ExprBuilder(x[0] + 2.0 * x[1] + 3.0)
    expr *= expr
    model.set_objective(expr)
    model.optimize()
    obj_value = model.get_value(expr)
    assert obj_value == approx(36.0)
