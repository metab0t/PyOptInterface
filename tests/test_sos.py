import pyoptinterface as poi
from pytest import approx
import pytest


# write a test to test if SOS1 and SOS2 constraint are functional
def test_sos(model_interface):
    model = model_interface

    if not hasattr(model, "add_sos_constraint"):
        pytest.skip("Model interface does not support SOS constraints")

    N = 10
    vars = [model.add_variable(lb=0.0, ub=1.0) for _ in range(N)]

    obj = poi.quicksum(vars)
    model.set_objective(obj, poi.ObjectiveSense.Maximize)

    con1 = model.add_sos_constraint(vars, poi.SOSType.SOS1)
    model.optimize()
    status = model.get_model_attribute(poi.ModelAttribute.TerminationStatus)
    assert status == poi.TerminationStatusCode.OPTIMAL
    obj_val = model.get_value(obj)
    assert obj_val == approx(1.0)

    nz_indices = [i for i in range(N) if model.get_value(vars[i]) > 1e-3]
    assert len(nz_indices) == 1

    model.delete_constraint(con1)
    con2 = model.add_sos_constraint(vars, poi.SOSType.SOS2)
    model.optimize()
    status = model.get_model_attribute(poi.ModelAttribute.TerminationStatus)
    assert status == poi.TerminationStatusCode.OPTIMAL
    obj_val = model.get_value(obj)
    assert obj_val == approx(2.0)

    nz_indices = [i for i in range(N) if model.get_value(vars[i]) > 1e-3]
    assert len(nz_indices) == 2
    v1, v2 = nz_indices
    assert v1 + 1 == v2
