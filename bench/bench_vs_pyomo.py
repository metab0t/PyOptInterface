import pyoptinterface as poi
from pyoptinterface import gurobi

import numpy as np

import pyomo.environ as pyo
from pyomo.common.timing import TicTocTimer

import linopy

import gurobipy as gp


def bench_poi(M, N):
    model = gurobi.Model()

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

    # f = lambda v: model.get_variable_attribute(v, poi.VariableAttribute.Value)
    # x_val = {k: f(v) for k, v in x.items()}


def bench_pyomo(M, N):
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

    solver = pyo.SolverFactory("gurobi_direct")
    solver.options["timelimit"] = 0.0
    solver.options["presolve"] = False
    solver.solve(model, load_solutions=False)

    # x_val = xr.DataArray(
    #     np.array([[model.x[i, j].value for j in J] for i in I]), dims=("i", "j")
    # )


def bench_linopy(M, N):
    model = linopy.Model()

    I = range(M)
    J = range(N)
    x = model.add_variables(lower=0.0, coords=[I, J], name="x")

    model.add_constraints(x.sum() == M * N / 2)
    model.add_objective((x * x).sum())

    model.solve("gurobi", io_api="direct", OutputFlag=0, TimeLimit = 0.0, Presolve=0)

    # x_val = x.solution

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

    # f = lambda v: model.get_variable_attribute(v, poi.VariableAttribute.Value)
    # x_val = {k: f(v) for k, v in x.items()}

M = 2000
N = 1000
timer = TicTocTimer()

timer.tic("pyomo starts")
# bench_pyomo(M, N)
timer.toc("pyomo ends")

timer.tic("poi starts")
bench_poi(M, N)
timer.toc("poi ends")

timer.tic("linopy starts")
bench_linopy(M, N)
timer.toc("linopy ends")

timer.tic("gurobi starts")
bench_gp(M, N)
timer.toc("gurobi ends")
