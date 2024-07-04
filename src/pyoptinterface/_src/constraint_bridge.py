from .core_ext import ScalarQuadraticFunction, ConstraintSense


def bridge_soc_quadratic_constraint(model, cone_variables, name="", rotated=False):
    """
    Convert a second order cone constraint to a quadratic constraint.
    x[0] >= sqrt(x[1]^2 + ... + x[n]^2)
    to
    x[0]^2 - x[1]^2 - ... - x[n]^2 >= 0
    or
    convert a rotated second order cone constraint to a quadratic constraint.
    2 * x[0] * x[1] >= x[2]^2 + ... + x[n]^2
    to
    2 * x[0] * x[1] - x[2]^2 - ... - x[n]^2 >= 0
    """
    N = len(cone_variables)
    if N < 2:
        raise ValueError(
            "Second order cone constraint must have at least two variables"
        )

    expr = ScalarQuadraticFunction()
    expr.reserve_quadratic(N)

    if not rotated:
        x0 = cone_variables[0]
        expr.add_quadratic_term(x0, x0, 1.0)

        for i in range(1, N):
            xi = cone_variables[i]
            expr.add_quadratic_term(xi, xi, -1.0)

        con = model.add_quadratic_constraint(
            expr, ConstraintSense.GreaterEqual, 0.0, name
        )
    else:
        x0 = cone_variables[0]
        x1 = cone_variables[1]
        expr.add_quadratic_term(x0, x1, 2.0)

        for i in range(2, N):
            xi = cone_variables[i]
            expr.add_quadratic_term(xi, xi, -1.0)

        con = model.add_quadratic_constraint(
            expr, ConstraintSense.GreaterEqual, 0.0, name
        )

    return con
