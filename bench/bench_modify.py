import pyoptinterface as poi
from pyoptinterface import gurobi, copt

from pyomo.common.timing import TicTocTimer

import gurobipy as gp
from gurobipy import GRB
import coptpy as cp

import time


def bench_poi_base(model, N):
    I = range(N)
    x = poi.make_nd_variable(model, I, lb=0.0)

    model.set_model_attribute(poi.ModelAttribute.Silent, True)
    model.set_model_attribute(poi.ModelAttribute.TimeLimitSec, 2.0)
    model.set_raw_parameter("Presolve", 0)

    obj = poi.ExprBuilder()
    for i in range(N // 2):
        obj.add(x[i])
    model.set_objective(obj, poi.ObjectiveSense.Minimize)

    get_after_delete_time = 0.0

    for i in range(N // 2, N):
        model.delete_variable(x[i])
        model.optimize()

        t1 = time.perf_counter()
        for k in range(N // 2):
            model.get_value(x[k])
        t2 = time.perf_counter()
        get_after_delete_time += t2 - t1

        # print(f"delete {i}")

    print(f"poi: get_after_delete_time: {get_after_delete_time}")


def bench_poi_gurobi(N):
    model = gurobi.Model()
    bench_poi_base(model, N)


def bench_poi_copt(N):
    model = copt.Model()
    bench_poi_base(model, N)


def bench_gp(N):
    model = gp.Model()

    I = range(N)
    x = model.addVars(I, lb=0.0)

    obj = gp.quicksum(x[i] for i in range(N // 2))
    model.setObjective(obj)

    model.setParam("OutputFlag", 0)
    model.setParam("TimeLimit", 2.0)
    model.setParam("Presolve", 0)

    get_after_delete_time = 0.0

    for j in range(N // 2, N):
        model.remove(x[j])
        model.optimize()

        t1 = time.perf_counter()
        for k in range(N // 2):
            x[k].getAttr(GRB.Attr.X)
        t2 = time.perf_counter()
        get_after_delete_time += t2 - t1

    print(f"gurobipy: get_after_delete_time: {get_after_delete_time}")


def bench_cp(N):
    env = cp.Envr()
    model = env.createModel("m")

    I = range(N)
    x = model.addVars(I, lb=0.0)

    model.setParam("Logging", 0)
    model.setParam("TimeLimit", 2.0)
    model.setParam("Presolve", 0)

    obj = cp.quicksum(x[i] for i in range(N // 2))
    model.setObjective(obj)

    get_after_delete_time = 0.0

    for j in range(N // 2, N):
        x[j].remove()
        model.solve()

        t1 = time.perf_counter()
        for k in range(N // 2):
            x[k].x
        t2 = time.perf_counter()
        get_after_delete_time += t2 - t1

    print(f"coptpy: get_after_delete_time: {get_after_delete_time}")


N = 10000
timer = TicTocTimer()

timer.tic("poi_gurobi starts")
bench_poi_gurobi(N)
timer.toc("poi_gurobi ends")

# timer.tic("gurobi starts")
# bench_gp(N)
# timer.toc("gurobi ends")

timer.tic("poi_copt starts")
bench_poi_copt(N)
timer.toc("poi_copt ends")

# timer.tic("copt starts")
# bench_cp(N)
# timer.toc("copt ends")
