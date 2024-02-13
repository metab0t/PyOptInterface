# Copyright (c) 2022: Miles Lubin and contributors
#
# Use of this source code is governed by an MIT-style license that can be found
# in the LICENSE.md file or at https://opensource.org/licenses/MIT.

from coptpy import *
import coptpy as cp
import os
import time


def solve_facility(G, F):
    env = cp.Envr()
    m = env.createModel("facility")

    # Create variables
    y = m.addVars(range(1, F + 1), range(1, 3), lb=0.0, ub=1.0)
    s = m.addVars(range(G + 1), range(G + 1), range(1, F + 1), lb=0.0)
    z = m.addVars(range(G + 1), range(G + 1), range(1, F + 1), vtype=COPT.BINARY)
    r = m.addVars(range(G + 1), range(G + 1), range(1, F + 1), range(1, 3))
    d = m.addVar()

    # Set objective
    m.setObjective(d, COPT.MINIMIZE)

    # Add constraints
    for i in range(G + 1):
        for j in range(G + 1):
            m.addConstr(z.sum(i, j, "*") == 1)

    M = 2 * 1.414
    for i in range(G + 1):
        for j in range(G + 1):
            for f in range(1, F + 1):
                m.addConstr(s[i, j, f] == d + M * (1 - z[i, j, f]))
                m.addConstr(r[i, j, f, 1] == (1.0 * i) / G - y[f, 1])
                m.addConstr(r[i, j, f, 2] == (1.0 * j) / G - y[f, 2])
                m.addCone([s[i, j, f], r[i, j, f, 1], r[i, j, f, 2]], COPT.CONE_QUAD)

    # Optimize model
    m.setParam("Logging", 0)
    m.setParam("TimeLimit", 0.0)
    m.setParam("Presolve", 1)
    m.solve()

    return m


def main(Ns=[25, 50, 75, 100]):
    dir = os.path.realpath(os.path.dirname(__file__))
    for n in Ns:
        start = time.time()
        model = solve_facility(n, n)
        run_time = round(time.time() - start)
        content = "coptpy fac-%i -1 %i" % (n, run_time)
        print(content)
        with open(dir + "/benchmarks.csv", "a") as io:
            io.write(f"{content}\n")
    return


main()
