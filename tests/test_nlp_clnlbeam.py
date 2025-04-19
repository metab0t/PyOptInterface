import pyoptinterface as poi
from pyoptinterface import nl

import pytest


def test_clnlbeam(nlp_model_ctor):
    model = nlp_model_ctor()

    N = 1000
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
                x[i + 1] - x[i] - 0.5 * h * (nl.sin(t[i]) + nl.sin(t[i + 1]))
                == 0.0
            )
            model.add_linear_constraint(
                t[i + 1] - t[i] - 0.5 * h * u[i + 1] - 0.5 * h * u[i] == 0.0
            )

    model.optimize()

    objective_value = model.get_model_attribute(poi.ModelAttribute.ObjectiveValue)

    assert objective_value == pytest.approx(350.0, abs=1e-8)
