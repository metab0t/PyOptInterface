import pytest

import pyoptinterface as poi
from pyoptinterface import ipopt, nl, copt


def test_nlp_lukvle10(nlp_model_ctor):
    # LUKSAN-VLCEK Problem 10
    #
    # min  sum[i=1..n] (x[2i]^2)^(x[2i+1]^2 + 1) + (x[2i+1]^2)^(x[2i]^2 + 1)
    # s.t. (3 - 2*x[i+1])*x[i+1] + 1 - x[i] - 2*x[i+2] = 0,  i = 1,...,2n-2
    #
    # Starting point: x[2i] = -1, x[2i+1] = 1
    model = nlp_model_ctor()
    if isinstance(model, ipopt.Model):
        # LUKVLE10 is too large and IPOPT raises a bad_alloc error.
        pytest.skip("lukvle10 is too large to be supported with IPOPT")
    if isinstance(model, copt.Model):
        # LUKVLE10 is too large the current license of COpt supports up
        # to 2000 variables.
        pytest.skip("lukvle10 is too large to be supported with COpt")

    n = 1250
    x = model.add_m_variables(2 * n, name="x")

    for i in range(2 * n - 2):
        model.add_quadratic_constraint(
            (3 - 2 * x[i + 1]) * x[i + 1] + 1 - x[i] - 2 * x[i + 2],
            poi.ConstraintSense.Equal,
            0.0,
            name=f"c{i}",
        )

    with nl.graph():
        for i in range(n):
            model.add_nl_objective(
                nl.pow(x[2 * i] ** 2, (x[2 * i + 1] ** 2) + 1)
                + nl.pow(x[2 * i + 1] ** 2, (x[2 * i] ** 2) + 1)
            )

    for i in range(n):
        model.set_variable_attribute(x[2 * i], poi.VariableAttribute.PrimalStart, -1.0)
        model.set_variable_attribute(
            x[2 * i + 1], poi.VariableAttribute.PrimalStart, 1.0
        )

    model.optimize()
    model.set_model_attribute(poi.ModelAttribute.Silent, False)

    termination_status = model.get_model_attribute(poi.ModelAttribute.TerminationStatus)
    assert termination_status == poi.TerminationStatusCode.OPTIMAL
