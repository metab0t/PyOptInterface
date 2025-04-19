import pyoptinterface as poi
from pyoptinterface import nlfunc

import math
import pytest


def rocket_model(model, nh: int):
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

    h = model.add_m_variables(nh, lb=1.0)
    v = model.add_m_variables(nh, lb=0.0)
    m = model.add_m_variables(nh, lb=m_f, ub=m_0)
    T = model.add_m_variables(nh, lb=0.0, ub=T_max)
    step = model.add_variable(lb=0.0)

    model.set_objective(-1.0 * h[-1])

    for i in range(nh - 1):
        with nlfunc.graph():
            h1 = h[i]
            h2 = h[i + 1]
            v1 = v[i]
            v2 = v[i + 1]
            m1 = m[i]
            m2 = m[i + 1]
            T1 = T[i]
            T2 = T[i + 1]

            model.add_nl_constraint(h2 - h1 - 0.5 * step * (v1 + v2) == 0)

            D1 = D_c * v1 * v1 * nlfunc.exp(-h_c * (h1 - h_0)) / h_0
            D2 = D_c * v2 * v2 * nlfunc.exp(-h_c * (h2 - h_0)) / h_0
            g1 = g_0 * h_0 * h_0 / (h1 * h1)
            g2 = g_0 * h_0 * h_0 / (h2 * h2)
            dv1 = (T1 - D1) / m1 - g1
            dv2 = (T2 - D2) / m2 - g2

            model.add_nl_constraint(v2 - v1 - 0.5 * step * (dv1 + dv2) == 0)
            model.add_nl_constraint(m2 - m1 + 0.5 * step * (T1 + T2) / c == 0)

    # Boundary conditions
    model.set_variable_bounds(h[0], h_0, h_0)
    model.set_variable_bounds(v[0], v_0, v_0)
    model.set_variable_bounds(m[0], m_0, m_0)
    model.set_variable_bounds(m[-1], m_f, m_f)

    model.h = h


def test_rocket(nlp_model_ctor):
    nh = 400
    model = nlp_model_ctor()
    rocket_model(model, nh)
    model.optimize()

    obj = model.get_value(model.h[-1])

    assert obj == pytest.approx(1.01283, rel=1e-4)


if __name__ == "__main__":
    from pyoptinterface import copt, ipopt

    def c():
        return ipopt.Model(jit="C")

    test_rocket(c)

    def llvm():
        return ipopt.Model(jit="LLVM")

    test_rocket(llvm)

    test_rocket(copt.Model)
