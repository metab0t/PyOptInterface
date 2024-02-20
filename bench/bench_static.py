import pyoptinterface as poi
from pyoptinterface import gurobi, copt, mosek

import pyomo.environ as pyo
from pyomo.common.timing import TicTocTimer

import linopy

import gurobipy as gp
import coptpy as cp
import mosek as msk


def bench_poi_base(model, M, N):
    I = range(M)
    J = range(N)
    x = poi.make_nd_variable(model, I, J, lb=0.0)

    expr = poi.quicksum(x)
    con = model.add_linear_constraint(expr, poi.ConstraintSense.Equal, M * N / 2)

    obj = poi.quicksum_f(x, lambda v: v * v)
    model.set_objective(obj, poi.ObjectiveSense.Minimize)

    model.set_model_attribute(poi.ModelAttribute.Silent, True)
    model.set_model_attribute(poi.ModelAttribute.TimeLimitSec, 0.0)
    if isinstance(model, gurobi.Model) or isinstance(model, copt.Model):
        model.set_raw_parameter("Presolve", 0)
    elif isinstance(model, mosek.Model):
        model.set_raw_parameter("MSK_IPAR_PRESOLVE_USE", 0)

    model.optimize()


def bench_poi_gurobi(M, N):
    model = gurobi.Model()
    bench_poi_base(model, M, N)


def bench_poi_copt(M, N):
    model = copt.Model()
    bench_poi_base(model, M, N)


def bench_poi_mosek(M, N):
    model = mosek.Model()
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

    solver.solve(model, load_solutions=False)


def bench_pyomo_gurobi(M, N):
    solver = pyo.SolverFactory("gurobi_direct")
    solver.options["timelimit"] = 0.0
    solver.options["presolve"] = False
    bench_pyomo_base(solver, M, N)


def bench_pyomo_mosek(M, N):
    solver = pyo.SolverFactory("mosek_direct")
    solver.options["dparam.optimizer_max_time"] = 0.0
    solver.options["iparam.presolve_use"] = 0
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


def streamprinter(text):
    pass


def bench_msk(M, N):
    with msk.Env() as env:
        with env.Task(0, 0) as task:
            task.set_Stream(msk.streamtype.log, streamprinter)

            # Total number of variables (M*N for 'x')
            numvar = M * N

            # Add variables
            for j in range(numvar):
                task.appendvars(1)

            # Variable bounds - All variables are greater than 0.0
            for j in range(numvar):
                task.putvarbound(j, msk.boundkey.ra, 0.0, 1e9)

            # Objective
            indx = list(range(M * N))
            val = [2.0] * (M * N)
            task.putqobj(indx, indx, val)
            task.putobjsense(msk.objsense.minimize)

            # Constraint - Sum of elements in 'x' equals M * N / 2
            task.appendcons(1)  # One linear constraint
            indx = list(range(M * N))  # Indices of 'x' variables
            val = [1.0] * (M * N)  # Coefficients are 1
            task.putarow(0, indx, val)
            task.putconbound(0, msk.boundkey.fx, M * N / 2, M * N / 2)

            # Set solver parameters
            task.putdouparam(msk.dparam.optimizer_max_time, 0.0)
            task.putintparam(msk.iparam.presolve_use, msk.presolvemode.off)

            # Solve the problem
            task.optimize()


M = 1000
N = 100
timer = TicTocTimer()

tests = [
    "gurobi",
    "copt",
    # "mosek",
]

if "gurobi" in tests:
    timer.tic("poi_gurobi starts")
    bench_poi_gurobi(M, N)
    timer.toc("poi_gurobi ends")

    timer.tic("linopy_gurobi starts")
    bench_linopy_gurobi(M, N)
    timer.toc("linopy_gurobi ends")

    timer.tic("gurobi starts")
    bench_gp(M, N)
    timer.toc("gurobi ends")

    timer.tic("pyomo_gurobi starts")
    bench_pyomo_gurobi(M, N)
    timer.toc("pyomo_gurobi ends")

if "copt" in tests:
    timer.tic("poi_copt starts")
    bench_poi_copt(M, N)
    timer.toc("poi_copt ends")

    timer.tic("cp starts")
    bench_cp(M, N)
    timer.toc("cp ends")

if "mosek" in tests:
    timer.tic("poi_mosek starts")
    bench_poi_mosek(M, N)
    timer.toc("poi_mosek ends")

    timer.tic("mosek starts")
    bench_msk(M, N)
    timer.toc("mosek ends")

    timer.tic("pyomo_mosek starts")
    bench_pyomo_mosek(M, N)
    timer.toc("pyomo_mosek ends")
