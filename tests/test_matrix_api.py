import pyoptinterface as poi
import numpy as np
from scipy.sparse import eye_array
from pytest import approx


def test_matrix_api(model_interface):
    model = model_interface

    N = 10
    x = model.add_m_variables(N, lb=0.0)
    A = np.eye(N)
    ub = 3.0
    lb = 1.0
    A_sparse = eye_array(N)
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
