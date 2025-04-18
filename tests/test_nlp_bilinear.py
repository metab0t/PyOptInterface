import pyoptinterface as poi

import pytest


def test_bilinear(nlp_model_ctor):
    model = nlp_model_ctor()

    x = model.add_m_variables(3, lb=0.0)

    obj = -(x[0] + x[1]) * (x[0] + x[2])

    expr = (x[0] + x[1]) * (x[0] + x[1]) + (x[0] + x[2]) * (x[0] + x[2])
    model.add_quadratic_constraint(expr == 4.0)

    model.set_objective(obj)
    model.optimize()

    objective_value = model.get_model_attribute(poi.ModelAttribute.ObjectiveValue)
    assert objective_value == pytest.approx(-2.0, abs=1e-8)
