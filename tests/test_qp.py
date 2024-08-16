import pyoptinterface as poi
from pytest import approx


def test_simple_qp(model_interface):
    model = model_interface

    N = 6

    x = model.add_variables(range(N), lb=0.0)

    model.add_linear_constraint(poi.quicksum(x), poi.Geq, N)

    s = poi.quicksum(x)
    s *= s
    model.set_objective(s, poi.ObjectiveSense.Minimize)

    model.optimize()
    status = model.get_model_attribute(poi.ModelAttribute.TerminationStatus)
    assert status == poi.TerminationStatusCode.OPTIMAL

    obj_val = model.get_model_attribute(poi.ModelAttribute.ObjectiveValue)
    assert obj_val == approx(N**2)
