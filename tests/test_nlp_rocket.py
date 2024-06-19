import pyoptinterface as poi
from pyoptinterface import ipopt

import math
import pytest


def rocket_model(model: ipopt.Model, nh: int):
    h_0 = 1.0
    v_0 = 0.0
    m_0 = 1.0
    g_0 = 1.0
    T_c = 3.5
    h_c = 500.0
    v_c = 620.0
    m_c = 0.6

    c = 0.5 * math.sqrt(g_0 * h_0)
    m_f = m_c * m_0
    D_c = 0.5 * v_c * (m_0 / g_0)
    T_max = T_c * m_0 * g_0

    h = [model.add_variable(lb=1.0, start=1.0) for _ in range(nh)]
    v = [model.add_variable(lb=0.0, start=i / nh * (1.0 - i / nh)) for i in range(nh)]
    m = [
        model.add_variable(lb=m_f, ub=m_0, start=(m_f - m_0) * (i / nh) + m_0)
        for i in range(nh)
    ]
    T = [model.add_variable(lb=0.0, ub=T_max, start=T_max / 2.0) for i in range(nh)]
    step = model.add_variable(lb=0.0, start=1.0 / nh)

    model.set_objective(-1.0 * h[-1])

    def dynamics_eq(vars):
        h1 = vars["h1"]
        h2 = vars["h2"]
        v1 = vars["v1"]
        v2 = vars["v2"]
        m1 = vars["m1"]
        m2 = vars["m2"]
        T1 = vars["T1"]
        T2 = vars["T2"]

        step = vars["step"]

        eq_h = h2 - h1 - 0.5 * step * (v1 + v2)

        D1 = D_c * v1 * v1 * poi.exp(-h_c * (h1 - h_0)) / h_0
        D2 = D_c * v2 * v2 * poi.exp(-h_c * (h2 - h_0)) / h_0
        g1 = g_0 * h_0 * h_0 / (h1 * h1)
        g2 = g_0 * h_0 * h_0 / (h2 * h2)
        dv1 = (T1 - D1) / m1 - g1
        dv2 = (T2 - D2) / m2 - g2

        eq_v = v2 - v1 - 0.5 * step * (dv1 + dv2)

        eq_m = m2 - m1 + 0.5 * step * (T1 + T2) / c

        return [eq_h, eq_v, eq_m]

    dynamic_eq_f = model.register_function(
        dynamics_eq,
        var=["h1", "h2", "v1", "v2", "m1", "m2", "T1", "T2", "step"],
        name="dynamics_eq",
    )
    for i in range(nh - 1):
        model.add_nl_constraint(
            dynamic_eq_f,
            [h[i], h[i + 1], v[i], v[i + 1], m[i], m[i + 1], T[i], T[i + 1], step],
            poi.Eq,
            [0.0, 0.0, 0.0],
        )

    # Boundary conditions
    model.set_variable_bounds(h[0], h_0, h_0)
    model.set_variable_bounds(v[0], v_0, v_0)
    model.set_variable_bounds(m[0], m_0, m_0)
    model.set_variable_bounds(m[-1], m_f, m_f)

    model.h = h


def test_rocket():
    if not ipopt.is_library_loaded():
        pytest.skip("Ipopt library is not loaded")
    nh = 400
    model = ipopt.Model()
    rocket_model(model, nh)
    model.optimize()

    obj = model.get_value(model.h[-1])

    assert obj == pytest.approx(1.01283, rel=1e-4)


if __name__ == "__main__":
    test_rocket()
