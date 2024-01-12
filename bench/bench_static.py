import pyoptinterface as poi
from pyoptinterface import gurobi, copt

import numpy as np

import pyomo.environ as pyo
from pyomo.common.timing import TicTocTimer

import linopy

import gurobipy as gp
import coptpy as cp


def bench_poi_base(model, M, N):
    I = range(M)
    J = range(N)
    x = poi.make_nd_variable(model, I, J, lb=0.0, name="x")

    expr = poi.quicksum(x)
    con = model.add_linear_constraint(expr, poi.ConstraintSense.Equal, M * N / 2)

    obj = poi.quicksum_f(x, lambda v: v * v)
    model.set_objective(obj, poi.ObjectiveSense.Minimize)

    model.set_model_attribute(poi.ModelAttribute.Silent, True)
    model.set_model_attribute(poi.ModelAttribute.TimeLimitSec, 0.0)
    model.set_raw_parameter("Presolve", 0)

    model.optimize()


def bench_poi_gurobi(M, N):
    model = gurobi.Model()
    bench_poi_base(model, M, N)


def bench_poi_copt(M, N):
    model = copt.Model()
    bench_poi_base(model, M, N)


def bench_pyomo_base(solver, M, N):
    model = pyo.ConcreteModel()

    I = range(M)
    J = range(N)
    model.x = pyo.Var(I, J)

    for var in model.x.values():
        var.setlb(0.0)

    model.c = pyo.Constraint(
        expr=pyo.quicksum(var for var in model.x.values()) == M * N / 2
    )
    model.o = pyo.Objective(expr=sum(v * v for v in model.x.values()))

    solver.options["timelimit"] = 0.0
    solver.options["presolve"] = False
    solver.solve(model, load_solutions=False)


def bench_pyomo_gurobi(M, N):
    solver = pyo.SolverFactory("gurobi_direct")
    bench_pyomo_base(solver, M, N)


def bench_linopy_gurobi(M, N):
    model = linopy.Model()

    I = range(M)
    J = range(N)
    x = model.add_variables(lower=0.0, coords=[I, J], name="x")

    model.add_constraints(x.sum() == M * N / 2)
    model.add_objective((x * x).sum())

    model.solve("gurobi", io_api="direct", OutputFlag=0, TimeLimit=0.0, Presolve=0)


def bench_gp(M, N):
    model = gp.Model()

    I = range(M)
    J = range(N)
    x = model.addVars(I, J, lb=0.0, name="x")

    expr = x.sum()
    con = model.addConstr(expr == M * N / 2)

    obj = gp.quicksum(v * v for v in x.values())
    model.setObjective(obj)

    model.setParam("OutputFlag", 0)
    model.setParam("TimeLimit", 0.0)
    model.setParam("Presolve", 0)

    model.optimize()


def bench_cp(M, N):
    env = cp.Envr()
    model = env.createModel("m")

    I = range(M)
    J = range(N)
    x = model.addVars(I, J, lb=0.0, nameprefix="x")

    expr = x.sum()
    con = model.addConstr(expr == M * N / 2)

    obj = cp.quicksum(v * v for v in x.values())
    model.setObjective(obj)

    model.setParam("Logging", 0)
    model.setParam("TimeLimit", 0.0)
    model.setParam("Presolve", 0)

    model.solve()


M = 1000
N = 1000
timer = TicTocTimer()

timer.tic("pyomo starts")
bench_pyomo_gurobi(M, N)
timer.toc("pyomo ends")

timer.tic("poi_gurobi starts")
bench_poi_gurobi(M, N)
timer.toc("poi_gurobi ends")

# timer.tic("linopy_gurobi starts")
# bench_linopy_gurobi(M, N)
# timer.toc("linopy_gurobi ends")

timer.tic("gurobi starts")
bench_gp(M, N)
timer.toc("gurobi ends")

timer.tic("poi_copt starts")
bench_poi_copt(M, N)
timer.toc("poi_copt ends")

timer.tic("cp starts")
bench_cp(M, N)
timer.toc("cp ends")
