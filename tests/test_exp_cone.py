import pyoptinterface as poi
from pytest import approx
import pytest
import math


def test_exp_cone(model_interface):
    model = model_interface

    if not hasattr(model, "add_exp_cone_constraint"):
        pytest.skip("Model interface does not support exponential cone constraints")

    N = 4
    x = model.add_variables(range(N), lb=1.0)
    t = model.add_variables(range(N), lb=0.0)
    one = model.add_variable(lb=1.0, ub=1.0, name="one")

    obj = poi.quicksum(t)
    model.set_objective(obj, poi.ObjectiveSense.Minimize)

    con = poi.make_tupledict(
        range(N), rule=lambda i: model.add_exp_cone_constraint([t[i], one, x[i]])
    )

    model.optimize()
    status = model.get_model_attribute(poi.ModelAttribute.TerminationStatus)
    assert status == poi.TerminationStatusCode.OPTIMAL

    x_val = x.map(model.get_value)
    t_val = t.map(model.get_value)
    for i in range(N):
        assert x_val[i] == approx(1.0)
        assert t_val[i] == approx(math.e)

    obj_val = model.get_value(obj)
    assert obj_val == approx(N * math.e)

    model.delete_constraint(con[0])
    model.optimize()
    status = model.get_model_attribute(poi.ModelAttribute.TerminationStatus)
    assert status == poi.TerminationStatusCode.OPTIMAL

    x_val = x.map(model.get_value)
    t_val = t.map(model.get_value)
    for i in range(1, N):
        assert x_val[i] == approx(1.0)
        assert t_val[i] == approx(math.e)

    obj_val = model.get_value(obj)
    assert obj_val == approx((N - 1) * math.e)
