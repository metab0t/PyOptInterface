import pyoptinterface as poi
import numpy as np
from scipy.sparse import coo_array
from pytest import approx


def test_matrix_api(model_interface_oneshot):
    model = model_interface_oneshot

    N = 10
    x = model.add_m_variables(N, lb=0.0)
    A = np.eye(N)
    ub = 3.0
    lb = 1.0
    A_sparse = coo_array(np.eye(N))
    model.add_m_linear_constraints(A, x, poi.Leq, ub)
    model.add_m_linear_constraints(A_sparse, x, poi.Geq, lb)

    obj = poi.quicksum(x)

    model.set_objective(obj)
    model.optimize()
    obj_value = model.get_model_attribute(poi.ModelAttribute.ObjectiveValue)
    assert obj_value == approx(N * lb)

    model.set_objective(-obj)
    model.optimize()
    obj_value = model.get_model_attribute(poi.ModelAttribute.ObjectiveValue)
    assert obj_value == approx(-N * ub)


def test_quicksum_ndarray(model_interface_oneshot):
    model = model_interface_oneshot

    N = 10
    x = model.add_m_variables((N, 2 * N), lb=1.0, ub=3.0)
    obj = poi.quicksum(x)
    model.set_objective(obj)
    model.optimize()
    obj_value = model.get_model_attribute(poi.ModelAttribute.ObjectiveValue)
    assert obj_value == approx(2 * N**2)
