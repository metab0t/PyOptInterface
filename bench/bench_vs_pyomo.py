import pyoptinterface as poi
from pyoptinterface import gurobi

import numpy as np

import pyomo.environ as pyo
from pyomo.common.timing import TicTocTimer

import linopy


def bench_poi(M, N):
    model = gurobi.Model()

    I = range(M)
    J = range(N)
    x = poi.make_nd_variable(model, I, J, lb=0.0)

    expr = poi.quicksum(x)
    con = model.add_linear_constraint(expr, poi.ConstraintSense.Equal, M * N / 2)

    obj = poi.quicksum_f(x, lambda v: v * v)
    model.set_objective(obj, poi.ObjectiveSense.Minimize)

    model.set_model_attribute(poi.ModelAttribute.Silent, True)

    model.optimize()

    # get_val = np.frompyfunc(
    #     lambda v: model.get_variable_attribute(v, poi.VariableAttribute.Value), 1, 1
    # )

    # x_val = xr.apply_ufunc(get_val, x).astype(np.float_)


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
    solver.solve(model, load_solutions=False)

    # x_val = xr.DataArray(
    #     np.array([[model.x[i, j].value for j in J] for i in I]), dims=("i", "j")
    # )


def bench_linopy(M, N):
    model = linopy.Model()

    I = range(M)
    J = range(N)
    x = model.add_variables(lower=0.0, coords=[I, J])

    model.add_constraints(x.sum() == M * N / 2)
    model.add_objective((x * x).sum())

    model.solve("gurobi", io_api="direct", OutputFlag=0)

    # x_val = x.solution


M = 400
N = 500
timer = TicTocTimer()

timer.tic("pyomo starts")
bench_pyomo(M, N)
timer.toc("pyomo ends")

timer.tic("poi starts")
bench_poi(M, N)
timer.toc("poi ends")

timer.tic("linopy starts")
bench_linopy(M, N)
timer.toc("linopy ends")
