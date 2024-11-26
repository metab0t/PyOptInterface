import pyoptinterface as poi
from pyoptinterface import nlfunc

import pytest


def test_clnlbeam(ipopt_model_ctor):
    model = ipopt_model_ctor()

    N = 1000
    h = 1 / N
    alpha = 350

    t = model.add_variables(range(N + 1), lb=-1.0, ub=1.0)
    x = model.add_variables(range(N + 1), lb=-0.05, ub=0.05)
    u = model.add_variables(range(N + 1))

    def obj(vars):
        return 0.5 * h * (vars.u2**2 + vars.u1**2) + 0.5 * alpha * h * (
            nlfunc.cos(vars.t2) + nlfunc.cos(vars.t1)
        )

    obj_f = model.register_function(obj)
    for i in range(N):
        model.add_nl_objective(
            obj_f, nlfunc.Vars(t1=t[i], t2=t[i + 1], u1=u[i], u2=u[i + 1])
        )

    def con(vars):
        return vars.x2 - vars.x1 - 0.5 * h * (nlfunc.sin(vars.t2) + nlfunc.sin(vars.t1))

    con_f = model.register_function(con)
    for i in range(N):
        model.add_nl_constraint(
            con_f, nlfunc.Vars(t1=t[i], t2=t[i + 1], x1=x[i], x2=x[i + 1]), eq=0.0
        )

    for i in range(N):
        model.add_linear_constraint(
            t[i + 1] - t[i] - 0.5 * h * u[i + 1] - 0.5 * h * u[i], poi.Eq, 0.0
        )

    model.optimize()

    objective_value = model.get_model_attribute(poi.ModelAttribute.ObjectiveValue)

    assert objective_value == pytest.approx(350.0, abs=1e-8)
