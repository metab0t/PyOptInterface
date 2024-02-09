# Copyright (c) 2023: Yue Yang
#
# Use of this source code is governed by an MIT-style license that can be found
# in the LICENSE.md file or at https://opensource.org/licenses/MIT.

import pyoptinterface as poi
from pyoptinterface import gurobi
from pyoptinterface import make_tupledict, make_nd_variable
import os
import time


def solve_lqcp(model, N):
    n = N
    m = N
    dx = 1.0 / n
    T = 1.58
    dt = T / n
    h2 = dx**2
    a = 0.001
    ns = range(n)
    ms = range(n)

    y = make_nd_variable(model, ms, ns, lb=0.0, ub=1.0)

    u = make_nd_variable(ms, lb=-1.0, ub=1.0)
    yt = [0.5 * (1 - (j * dx) ** 2) for j in range(n + 1)]
    obj = poi.ExprBuilder()
    obj.add((y[m, 0] - yt[0]) * (y[m, 0] - yt[0]))
    for j in range(1, n):
        obj.add(2 * (y[m, j] - yt[j]) * (y[m, j] - yt[j]))
    obj.add((y[m, n] - yt[n]) * (y[m, n] - yt[n]))
    for i in range(1, m):
        obj.add(0.25 * a * dt * 2 * (u[i] * u[i]))
    obj.add(0.25 * a * dt * (u[m] * u[m]))
    obj.mul(1 / 4 * dx)
    model.set_objective(obj, sense=poi.ObjectiveSense.Minimize)

    def pde_rule(i, j):
        expr = (y[i + 1, j] - y[i, j]) / dt - 0.5 * (
            y[i, j - 1]
            - 2 * y[i, j]
            + y[i, j + 1]
            + y[i + 1, j - 1]
            - 2 * y[i + 1, j]
            + y[i + 1, j + 1]
        ) / h2
        return model.add_linear_constraint(expr, poi.ConstraintSense.Equal, 0.0)

    pde = make_tupledict(range(n - 1), range(1, n - 1), rule=pde_rule)

    def ic_rule(j):
        return model.add_linear_constraint(y[0, j], poi.ConstraintSense.Equal, 0.0)

    ic = make_tupledict(ns, rule=ic_rule)

    def bc1_rule(i):
        expr = y[i, 2] - 4 * y[i, 1] + 3 * y[i, 0]
        return model.add_linear_constraint(expr, poi.ConstraintSense.Equal, 0.0)

    bc1 = make_tupledict(range(1, n), rule=bc1_rule)

    def bc2_rule(i):
        expr = (
            y[i, n - 2]
            - 4 * y[i, n - 1]
            + 3 * y[i, n - 0]
            - (2 * dx) * (u[i] - y[i, n - 0])
        )
        return model.add_linear_constraint(expr, poi.ConstraintSense.Equal, 0.0)

    bc2 = make_tupledict(range(1, n), rule=bc2_rule)

    model.set_model_attribute(poi.ModelAttribute.TimeLimitSec, 0.0)
    model.set_model_raw_parameter("Presolve", 0)
    model.optimize()


def main(Ns=[500, 1000, 1500, 2000]):
    dir = os.path.realpath(os.path.dirname(__file__))
    for n in Ns:
        start = time.time()
        try:
            model = gurobi.Model()
            solve_lqcp(model, n)
        except:
            pass
        run_time = round(time.time() - start)
        print(f"poi lqcp-{n} -1 {run_time}")
        with open(dir + "/benchmarks.csv", "a") as io:
            io.write("poi lqcp-%i -1 %i\n" % (n, run_time))
    return


main()
