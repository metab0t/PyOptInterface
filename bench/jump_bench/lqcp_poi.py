# Copyright (c) 2023: Yue Yang
#
# Use of this source code is governed by an MIT-style license that can be found
# in the LICENSE.md file or at https://opensource.org/licenses/MIT.

import sys
import pyoptinterface as poi
from pyoptinterface import gurobi, copt
import os
import time


def solve_lqcp(m, N):
    dx = 1.0 / N
    T = 1.58
    dt = T / N
    h2 = dx**2
    a = 0.001

    y = m.add_variables(range(N + 1), range(N + 1), lb=0.0, ub=1.0)
    u = m.add_variables(range(N + 1), lb=-1.0, ub=1.0)

    yt = {j: 0.5 * (1 - (j * dx) ** 2) for j in range(N + 1)}

    obj = poi.ExprBuilder()
    obj.add((y[N, 0] - yt[0]) * (y[N, 0] - yt[0]))
    for j in range(1, N):
        obj.add(2.0 * (y[N, j] - yt[j]) * (y[N, j] - yt[j]))
    obj.add((y[N, N] - yt[N]) * (y[N, N] - yt[N]))
    for i in range(1, N):
        obj.add(0.25 * a * dt * 2 * (u[i] * u[i]))
    obj.add(0.25 * a * dt * (u[N] * u[N]))
    obj.mul(1 / 4 * dx)
    m.set_objective(obj, sense=poi.ObjectiveSense.Minimize)

    for i in range(N):
        for j in range(1, N):
            expr = (y[i + 1, j] - y[i, j]) / dt - 0.5 * (
                y[i, j - 1]
                - 2 * y[i, j]
                + y[i, j + 1]
                + y[i + 1, j - 1]
                - 2 * y[i + 1, j]
                + y[i + 1, j + 1]
            ) / h2
            m.add_linear_constraint(expr, poi.ConstraintSense.Equal, 0.0)

    for j in range(N + 1):
        expr = y[0, j]
        m.add_linear_constraint(expr, poi.ConstraintSense.Equal, 0.0)

    for i in range(1, N):
        expr = y[i, 2] - 4 * y[i, 1] + 3 * y[i, 0]
        m.add_linear_constraint(expr, poi.ConstraintSense.Equal, 0.0)

    for i in range(1, N):
        expr = y[i, N - 2] - 4 * y[i, N - 1] + 3 * y[i, N] - 2 * dx * (u[i] - y[i, N])
        m.add_linear_constraint(expr, poi.ConstraintSense.Equal, 0.0)

    m.set_model_attribute(poi.ModelAttribute.Silent, True)
    m.set_model_attribute(poi.ModelAttribute.TimeLimitSec, 0.0)
    m.set_raw_parameter("Presolve", 0)
    m.optimize()


def main(Ns=[500, 1000, 1500, 2000]):
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
        solve_lqcp(model, n)
        run_time = round(time.time() - start)
        content = f"poi_{solver_name} lqcp-{n} -1 {run_time}"
        print(content)
        with open(dir + "/benchmarks.csv", "a") as io:
            io.write(f"{content}\n")
    return


main()
