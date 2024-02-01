from .core_ext import ScalarQuadraticFunction, ConstraintSense


def bridge_soc_quadratic_constraint(model, cone_variables, name=""):
    """
    Convert a second order cone constraint to a quadratic constraint.
    x[0] >= sqrt(x[1]^2 + ... + x[n]^2)
    to
    x[0]^2 - x[1]^2 - ... - x[n]^2 >= 0
    """
    N = len(cone_variables)
    if N < 2:
        raise ValueError(
            "Second order cone constraint must have at least two variables"
        )

    expr = ScalarQuadraticFunction()
    expr.reserve_quadratic(N)

    x0 = cone_variables[0]
    expr.add_quadratic_term(x0, x0, 1.0)

    for i in range(1, N):
        xi = cone_variables[i]
        expr.add_quadratic_term(xi, xi, -1.0)

    con = model.add_quadratic_constraint(expr, ConstraintSense.GreaterEqual, 0.0, name)

    return con
