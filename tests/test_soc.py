import pyoptinterface as poi
from pytest import approx
import pytest

def test_soc(model_interface):
    model = model_interface

    if not hasattr(model, "add_second_order_cone_constraint"):
        pytest.skip("Model interface does not support second order cone constraints")

    x = model.add_variable(lb=0.0, name="x")
    y = model.add_variable(lb=3.0, name="y")
    z = model.add_variable(lb=4.0, name="z")

    obj = x + y + z
    model.set_objective(obj, poi.ObjectiveSense.Minimize)

    con1 = model.add_second_order_cone_constraint([x, y, z])
    model.optimize()
    status = model.get_model_attribute(poi.ModelAttribute.TerminationStatus)
    assert status == poi.TerminationStatusCode.OPTIMAL
    x_val = model.get_value(x)
    y_val = model.get_value(y)
    z_val = model.get_value(z)
    assert x_val == approx(5.0)
    assert y_val == approx(3.0)
    assert z_val == approx(4.0)
    obj_val = model.get_value(obj)
    assert obj_val == approx(12.0)
