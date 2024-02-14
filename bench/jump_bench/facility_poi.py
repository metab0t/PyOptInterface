# Copyright (c) 2023: Yue Yang
#
# Use of this source code is governed by an MIT-style license that can be found
# in the LICENSE.md file or at https://opensource.org/licenses/MIT.

import sys
import pyoptinterface as poi
from pyoptinterface import gurobi, copt
import os
import time


def solve_facility(m, G, F):
    # Create variables
    y = m.add_variables(range(1, F + 1), range(1, 3), lb=0.0, ub=1.0)
    s = m.add_variables(range(G + 1), range(G + 1), range(1, F + 1), lb=0.0)
    z = m.add_variables(
        range(G + 1), range(G + 1), range(1, F + 1), domain=poi.VariableDomain.Binary
    )
    r = m.add_variables(range(G + 1), range(G + 1), range(1, F + 1), range(1, 3))
    d = m.add_variable()

    # Set objective
    m.set_objective(d * 1.0)

    # Add constraints
    for i in range(G + 1):
        for j in range(G + 1):
            expr = poi.quicksum(z.select(i, j, "*"))
            m.add_linear_constraint(expr, poi.ConstraintSense.Equal, 1.0)

    M = 2 * 1.414
    for i in range(G + 1):
        for j in range(G + 1):
            for f in range(1, F + 1):
                expr = s[i, j, f] - d - M * (1 - z[i, j, f])
                m.add_linear_constraint(expr, poi.ConstraintSense.Equal, 0.0)
                expr = r[i, j, f, 1] - (1.0 * i) / G + y[f, 1]
                m.add_linear_constraint(expr, poi.ConstraintSense.Equal, 0.0)
                expr = r[i, j, f, 2] - (1.0 * j) / G + y[f, 2]
                m.add_linear_constraint(expr, poi.ConstraintSense.Equal, 0.0)
                # expr = (
                #     r[i, j, f, 1] * r[i, j, f, 1]
                #     + r[i, j, f, 2] * r[i, j, f, 2]
                #     - s[i, j, f] * s[i, j, f]
                # )
                # m.add_quadratic_constraint(expr, poi.ConstraintSense.LessEqual, 0.0)
                m.add_second_order_cone_constraint(
                    [s[i, j, f], r[i, j, f, 1], r[i, j, f, 2]]
                )

    # Optimize model
    m.set_model_attribute(poi.ModelAttribute.Silent, True)
    m.set_model_attribute(poi.ModelAttribute.TimeLimitSec, 0.0)
    # close presolve for gurobi and open for copt
    solver_name = m.get_model_attribute(poi.ModelAttribute.SolverName)
    if solver_name.lower() == "gurobi":
        m.set_raw_parameter("Presolve", 0)
    elif solver_name.lower() == "copt":
        m.set_raw_parameter("Presolve", 1)
    m.optimize()


def main(Ns=[25, 50, 75, 100]):
    dir = os.path.realpath(os.path.dirname(__file__))

    # get the first argument as the solver name
    solver_name = sys.argv[1]
    print(solver_name)

    model_constructor = {
        "gurobi": gurobi.Model,
        "copt": copt.Model,
    }.get(solver_name.lower(), None)
    if model_constructor is None:
        raise ValueError(f"Unknown solver {solver_name}")

    solver_name = solver_name.lower()

    for n in Ns:
        start = time.time()
        model = model_constructor()
        solve_facility(model, n, n)
        run_time = round(time.time() - start)
        content = f"poi_{solver_name} fac-{n} -1 {run_time}"
        print(content)
        with open(dir + "/benchmarks.csv", "a") as io:
            io.write(f"{content}\n")
    return


main()
