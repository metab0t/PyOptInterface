import pyoptinterface as poi
from pyoptinterface import nl
import pytest


def test_hs071(nlp_model_ctor):
    model = nlp_model_ctor()

    x = model.add_m_variables(4, lb=1.0, ub=5.0, name="x")

    model.set_variable_attribute(x[0], poi.VariableAttribute.PrimalStart, 1.0)
    model.set_variable_attribute(x[1], poi.VariableAttribute.PrimalStart, 5.0)
    model.set_variable_attribute(x[2], poi.VariableAttribute.PrimalStart, 5.0)
    model.set_variable_attribute(x[3], poi.VariableAttribute.PrimalStart, 1.0)

    with nl.graph():
        model.add_nl_objective(x[0] * x[3] * (x[0] + x[1] + x[2]) + x[2])

    with nl.graph():
        model.add_nl_constraint(x[0] * x[1] * x[2] * x[3] >= 25.0)
        model.add_quadratic_constraint(
            x[0] ** 2 + x[1] ** 2 + x[2] ** 2 + x[3] ** 2 == 40.0
        )

    model.optimize()

    x1_val = model.get_value(x[0])
    x2_val = model.get_value(x[1])
    x3_val = model.get_value(x[2])
    x4_val = model.get_value(x[3])

    sum_sq = x1_val**2 + x2_val**2 + x3_val**2 + x4_val**2
    product = x1_val * x2_val * x3_val * x4_val
    assert sum_sq == pytest.approx(40.0, rel=1e-6)
    assert 25.0 - 1e-6 <= product <= 100.0 + 1e-6

    objective_value = model.get_model_attribute(poi.ModelAttribute.ObjectiveValue)
    assert objective_value == pytest.approx(17.014, rel=1e-3)
