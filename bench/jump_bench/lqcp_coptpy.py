# Copyright (c) 2022: Miles Lubin and contributors
#
# Use of this source code is governed by an MIT-style license that can be found
# in the LICENSE.md file or at https://opensource.org/licenses/MIT.

from coptpy import *
import coptpy as cp
import os
import time


def solve_lqcp(N):
    env = cp.Envr()
    m = env.createModel("lqcp")

    dx = 1.0 / N
    T = 1.58
    dt = T / N
    h2 = dx**2
    a = 0.001

    y = m.addVars(range(N + 1), range(N + 1), lb=0.0, ub=1.0)
    u = m.addVars(range(N + 1), lb=-1.0, ub=1.0)

    yt = {j: 0.5 * (1 - (j * dx) ** 2) for j in range(N + 1)}

    m.setObjective(
        1 / 4 * dx * ((y[N, 0] - yt[0])) ** 2
        + 2 * quicksum((y[N, j] - yt[j]) ** 2 for j in range(1, N))
        + (y[N, N] - yt[N]) ** 2
        + 1 / 4 * a * dt * (2 * quicksum(u[i] ** 2 for i in range(1, N)) + u[N] ** 2),
        COPT.MINIMIZE,
    )

    for i in range(N):
        for j in range(1, N):
            m.addConstr(
                (y[i + 1, j] - y[i, j]) / dt
                == 0.5
                * (
                    y[i, j - 1]
                    - 2 * y[i, j]
                    + y[i, j + 1]
                    + y[i + 1, j - 1]
                    - 2 * y[i + 1, j]
                    + y[i + 1, j + 1]
                )
                / h2
            )

    for j in range(N + 1):
        m.addConstr(y[0, j] == 0)

    for i in range(1, N):
        m.addConstr(y[i, 2] - 4 * y[i, 1] + 3 * y[i, 0] == 0)

    for i in range(1, N):
        m.addConstr(
            y[i, N - 2] - 4 * y[i, N - 1] + 3 * y[i, N] == 2 * dx * (u[i] - y[i, N])
        )

    m.setParam("Logging", 0)
    m.setParam("TimeLimit", 0.0)
    m.setParam("Presolve", 0)
    m.solve()

    return m


def main(Ns=[500, 1000, 1500, 2000]):
    dir = os.path.realpath(os.path.dirname(__file__))
    for n in Ns:
        start = time.time()
        model = solve_lqcp(n)
        run_time = round(time.time() - start)
        content = "coptpy lqcp-%i -1 %i" % (n, run_time)
        print(content)
        with open(dir + "/benchmarks.csv", "a") as io:
            io.write(f"{content}\n")
    return


main()
