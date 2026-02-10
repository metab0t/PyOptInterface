import pyoptinterface as poi
from pyoptinterface import nl

import pytest

import numpy as np


def test_clnlbeam(nlp_model_ctor):
    model = nlp_model_ctor()

    N = 100
    h = 1 / N
    alpha = 350

    t = model.add_m_variables(N + 1, lb=-1.0, ub=1.0)
    x = model.add_m_variables(N + 1, lb=-0.05, ub=0.05)
    u = model.add_m_variables(N + 1)

    for i in range(N):
        with nl.graph():
            model.add_nl_objective(
                0.5 * h * (u[i] * u[i] + u[i + 1] * u[i + 1])
                + 0.5 * alpha * h * (nl.cos(t[i]) + nl.cos(t[i + 1]))
            )
            model.add_nl_constraint(
                x[i + 1] - x[i] - 0.5 * h * (nl.sin(t[i]) + nl.sin(t[i + 1])) == 0.0
            )
            model.add_linear_constraint(
                t[i + 1] - t[i] - 0.5 * h * u[i + 1] - 0.5 * h * u[i] == 0.0
            )

    model.optimize()

    tv = np.zeros(N + 1)
    xv = np.zeros(N + 1)
    uv = np.zeros(N + 1)
    for i in range(N + 1):
        tv[i] = model.get_value(t[i])
        xv[i] = model.get_value(x[i])
        uv[i] = model.get_value(u[i])

    con_expr_val = np.zeros(N)
    for i in range(N):
        con_expr_val[i] = xv[i + 1] - xv[i] - 0.5 * h * (np.sin(tv[i]) + np.sin(tv[i + 1]))

    assert np.allclose(con_expr_val, 0.0, atol=1e-8)

    lin_con_expr_val = np.zeros(N)
    for i in range(N):
        lin_con_expr_val[i] = tv[i + 1] - tv[i] - 0.5 * h * uv[i + 1] - 0.5 * h * uv[i]

    assert np.allclose(lin_con_expr_val, 0.0, atol=1e-8)

    obj_expr_val = 0.0
    for i in range(N):
        obj_expr_val += 0.5 * h * (uv[i] * uv[i] + uv[i + 1] * uv[i + 1])
        obj_expr_val += 0.5 * alpha * h * (np.cos(tv[i]) + np.cos(tv[i + 1]))

    objective_value = model.get_model_attribute(poi.ModelAttribute.ObjectiveValue)

    assert np.isclose(obj_expr_val, objective_value, atol=1e-8)
    assert (
        objective_value == pytest.approx(328.0967, abs=1e-4)
        or objective_value == pytest.approx(350.0, abs=1e-8)
    )
