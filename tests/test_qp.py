import pyoptinterface as poi
from pytest import approx

import numpy as np


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


# reported by https://github.com/metab0t/PyOptInterface/issues/59
def test_shuffle_qp_objective(model_interface):
    model = model_interface

    N = 3
    weights = model.add_m_variables(N, lb=0)

    expected_returns = [0.05, 0.07, 0.12]
    min_return = 0.06
    cov = [
        (0, 0, 0.1),
        (0, 1, 0.1),
        (0, 2, 0.04),
        (1, 1, 0.4),
        (1, 2, 0.2),
        (2, 2, 0.9),
    ]
    cov_objs = [weights[i] * weights[j] * v for i, j, v in cov]

    model.add_linear_constraint(poi.quicksum(weights) == 1)
    model.add_linear_constraint(
        poi.quicksum(expected_returns[i] * weights[i] for i in range(N)) >= min_return
    )

    trial = 120
    obj_values = []
    for _ in range(trial):
        import random

        random.shuffle(cov_objs)
        obj = poi.quicksum(cov_objs)
        model.set_objective(obj, poi.ObjectiveSense.Minimize)

        model.optimize()

        obj_value = model.get_model_attribute(poi.ModelAttribute.ObjectiveValue)
        obj_values.append(obj_value)

    obj_values = np.array(obj_values)
    # test all values are the same
    assert np.all(np.abs(obj_values - obj_values[0]) < 1e-8)


def test_duplicated_quadratic_terms(model_interface):
    model = model_interface

    x = model.add_m_variables(2, lb=1.0)

    obj = (
        x[0] * x[0]
        + x[0] * x[0]
        + x[1] * x[1]
        + 2 * x[1] * x[1]
        + 0.5 * x[0] * x[1]
        + 0.1 * x[1] * x[0]
    )

    model.set_objective(obj)

    model.optimize()
    obj_value = model.get_model_attribute(poi.ModelAttribute.ObjectiveValue)
    assert obj_value == approx(5.6)
