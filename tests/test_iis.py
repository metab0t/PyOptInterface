import pyoptinterface as poi
import pytest


def test_constraint_iis(model_interface):
    model = model_interface

    if not hasattr(model, "computeIIS"):
        pytest.skip("Model interface does not support IIS computation")

    x = model.add_variable(lb=0.0, name="x")
    y = model.add_variable(lb=0.0, name="y")

    con1 = model.add_linear_constraint(x + y, poi.Geq, 5.0)
    con2 = model.add_linear_constraint(x + 2 * y, poi.Leq, 1.0)

    model.set_objective(x)

    model.computeIIS()

    con1_iis = model.get_constraint_attribute(con1, poi.ConstraintAttribute.IIS)
    con2_iis = model.get_constraint_attribute(con2, poi.ConstraintAttribute.IIS)

    assert con1_iis
    assert con2_iis
