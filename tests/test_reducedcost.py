import pyoptinterface as poi
import pytest
from pytest import approx


def test_simple_redcost(model_interface):
    model = model_interface

    if not model.supports_variable_attribute(poi.VariableAttribute.ReducedCost):
        pytest.skip("Model interface does not support ReducedCost attribute")

    x = model.add_variable(lb=0.0, ub=2.0)
    y = model.add_variable(lb=0.0, ub=2.5)
    model.add_linear_constraint(x + 2 * y >= 6.0)

    model.set_objective(x + y)

    model.optimize()
    status = model.get_model_attribute(poi.ModelAttribute.TerminationStatus)
    assert status == poi.TerminationStatusCode.OPTIMAL

    x_redcost = model.get_variable_attribute(x, poi.VariableAttribute.ReducedCost)
    y_redcost = model.get_variable_attribute(y, poi.VariableAttribute.ReducedCost)

    assert x_redcost == approx(0.0, abs=1e-5)
    assert y_redcost == approx(-1.0, abs=1e-5)
