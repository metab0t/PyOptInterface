# Copyright (c) 2022: Miles Lubin and contributors
#
# Use of this source code is governed by an MIT-style license that can be found
# in the LICENSE.md file or at https://opensource.org/licenses/MIT.

import pyomo.environ as pyo
from pyomo.contrib import appsi
from pyomo.opt import SolverFactory
import os
import time


def solve_lqcp(solve_f, N):
    model = pyo.ConcreteModel()
    model.n = N
    model.m = N
    model.dx = 1.0 / model.n
    model.T = 1.58
    model.dt = model.T / model.n
    model.h2 = model.dx**2
    model.a = 0.001
    model.ns = pyo.RangeSet(0, model.n)
    model.ms = pyo.RangeSet(0, model.n)
    model.y = pyo.Var(model.ms, model.ns, bounds=(0.0, 1.0))
    model.u = pyo.Var(model.ms, bounds=(-1.0, 1.0))
    yt = {j: 0.5 * (1 - (j * model.dx) ** 2) for j in range(0, model.n + 1)}
    model.obj = pyo.Objective(
        expr=1 / 4 * model.dx * ((model.y[model.m, 0] - yt[0])) ** 2
        + 2 * sum((model.y[model.m, j] - yt[j]) ** 2 for j in range(1, model.n))
        + (model.y[model.m, model.n] - yt[model.n]) ** 2
        + 1
        / 4
        * model.a
        * model.dt
        * (2 * sum(model.u[i] ** 2 for i in range(1, model.m)) + model.u[model.m] ** 2)
    )

    def pde_rule(model, i, j):
        return (model.y[i + 1, j] - model.y[i, j]) / model.dt == 0.5 * (
            model.y[i, j - 1]
            - 2 * model.y[i, j]
            + model.y[i, j + 1]
            + model.y[i + 1, j - 1]
            - 2 * model.y[i + 1, j]
            + model.y[i + 1, j + 1]
        ) / model.h2

    model.pde = pyo.Constraint(
        pyo.RangeSet(0, model.n - 1), pyo.RangeSet(1, model.n - 1), rule=pde_rule
    )

    def ic_rule(model, j):
        return model.y[0, j] == 0

    model.ic = pyo.Constraint(model.ns, rule=ic_rule)

    def bc1_rule(model, i):
        return model.y[i, 2] - 4 * model.y[i, 1] + 3 * model.y[i, 0] == 0

    model.bc1 = pyo.Constraint(pyo.RangeSet(1, model.n), rule=bc1_rule)

    def bc2_rule(model, i):
        return model.y[i, model.n - 2] - 4 * model.y[i, model.n - 1] + 3 * model.y[
            i, model.n - 0
        ] == (2 * model.dx) * (model.u[i] - model.y[i, model.n - 0])

    model.bc2 = pyo.Constraint(pyo.RangeSet(1, model.n), rule=bc2_rule)

    solve_f(model)
    return model


def gurobi_persistent(model):
    opt = SolverFactory("gurobi_persistent")
    opt.options["timelimit"] = 0.0
    opt.options["presolve"] = False
    opt.set_instance(model)
    opt.solve(tee=False, load_solutions=False)


def appsi_gurobi(model):
    opt = appsi.solvers.Gurobi()
    opt.gurobi_options["OutputFlag"] = 0
    opt.gurobi_options["TimeLimit"] = 0.0
    opt.gurobi_options["Presolve"] = 0
    opt.config.load_solution = False
    opt.solve(model)


def main(Ns=[500, 1000, 1500, 2000]):
    dir = os.path.realpath(os.path.dirname(__file__))
    for solver_f, solver_name in [
        (appsi_gurobi, "appsi"),
        # (gurobi_persistent, "persistent"),
    ]:
        for n in Ns:
            start = time.time()
            model = solve_lqcp(solver_f, n)
            run_time = round(time.time() - start)
            content = "pyomo_%s lqcp-%i -1 %i" % (solver_name, n, run_time)
            print(content)
            with open(dir + "/benchmarks.csv", "a") as io:
                io.write(f"{content}\n")
    return


main()
