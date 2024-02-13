# Copyright (c) 2022: Miles Lubin and contributors
#
# Use of this source code is governed by an MIT-style license that can be found
# in the LICENSE.md file or at https://opensource.org/licenses/MIT.

import pyoptinterface as poi
from pyoptinterface import gurobi, copt
import os
import time


def solve_facility(m, G, F):
    y = [[m.add_variable(lb=0.0, ub=1.0) for _ in range(2)] for _ in range(F + 1)]
    s = [
        [[m.add_variable(lb=0.0) for _ in range(F + 1)] for _ in range(G + 1)]
        for _ in range(G + 1)
    ]
    z = [
        [
            [m.add_variable(domain=poi.VariableDomain.Binary) for _ in range(F + 1)]
            for _ in range(G + 1)
        ]
        for _ in range(G + 1)
    ]
    r = [
        [
            [[m.add_variable() for _ in range(2)] for _ in range(F + 1)]
            for _ in range(G + 1)
        ]
        for _ in range(G + 1)
    ]
    d = m.add_variable()

    obj = poi.ExprBuilder()
    obj.add(d)
    m.set_objective(obj, sense=poi.ObjectiveSense.Minimize)

    for i in range(G + 1):
        for j in range(G + 1):
            zij = z[i][j]
            expr = poi.quicksum(zij[f] for f in range(1, F + 1))
            m.add_linear_constraint(expr, poi.ConstraintSense.Equal, 1.0)

    M = 2 * 1.414

    for i in range(G + 1):
        for j in range(G + 1):
            for f in range(1, F + 1):
                m.add_linear_constraint(
                    s[i][j][f] - d - M * (1.0 - z[i][j][f]),
                    poi.ConstraintSense.Equal,
                    0.0,
                )
                m.add_linear_constraint(
                    r[i][j][f][0] + y[f][0] - (1.0 * i) / G,
                    poi.ConstraintSense.Equal,
                    0.0,
                )
                m.add_linear_constraint(
                    r[i][j][f][1] + y[f][1] - (1.0 * j) / G,
                    poi.ConstraintSense.Equal,
                    0.0,
                )
                # m.add_quadratic_constraint(
                #     r[i][j][f][0] * r[i][j][f][0]
                #     + r[i][j][f][1] * r[i][j][f][1]
                #     - s[i][j][f] * s[i][j][f],
                #     poi.ConstraintSense.LessEqual,
                #     0.0,
                # )
                m.add_second_order_cone_constraint(
                    [
                        s[i][j][f],
                        r[i][j][f][0],
                        r[i][j][f][1],
                    ]
                )

    m.set_model_attribute(poi.ModelAttribute.Silent, True)
    m.set_model_attribute(poi.ModelAttribute.TimeLimitSec, 0.0)
    m.set_raw_parameter("Presolve", 1)
    m.optimize()


def main(Ns=[25, 50, 75, 100]):
    dir = os.path.realpath(os.path.dirname(__file__))
    for n in Ns:
        start = time.time()
        model = copt.Model()
        solve_facility(model, n, n)
        run_time = round(time.time() - start)
        content = "poi fac-%i -1 %i" % (n, run_time)
        print(content)
        with open(dir + "/benchmarks.csv", "a") as io:
            io.write(f"{content}\n")
    return


main()
