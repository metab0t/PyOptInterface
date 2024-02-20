import pyoptinterface as poi
from pyoptinterface import gurobi, copt


import gurobipy as gp
import coptpy as cp

import time


def bench_poi_base(model, N, M):
    I = range(N)
    x = poi.make_nd_variable(model, I)

    model.set_model_attribute(poi.ModelAttribute.Silent, True)
    model.set_model_attribute(poi.ModelAttribute.TimeLimitSec, 0.0)
    model.set_raw_parameter("Presolve", 0)

    expr = poi.ExprBuilder()
    for i in range(N):
        expr.add(i / N * x[i])
    model.set_objective(expr, poi.ObjectiveSense.Minimize)

    expr = poi.quicksum(x)
    model.add_linear_constraint(expr, poi.ConstraintSense.GreaterEqual, N)

    # for i in range(0, N, M):
    for i in [0]:
        last_variable = min(i + M, N)
        deleted_variables = [x[j] for j in range(i, last_variable)]
        model.delete_variables(deleted_variables)
        model.optimize()


def bench_poi_gurobi(N, M):
    model = gurobi.Model()
    bench_poi_base(model, N, M)


def bench_poi_copt(N, M):
    model = copt.Model()
    bench_poi_base(model, N, M)


def bench_gp(N, M):
    model = gp.Model()

    I = range(N)
    x = model.addVars(I)

    obj = gp.quicksum(i / N * x[i] for i in range(N))
    model.setObjective(obj)

    model.addConstr(x.sum() == N)

    model.setParam("OutputFlag", 0)
    model.setParam("TimeLimit", 0.0)
    model.setParam("Presolve", 0)

    # for j in range(0, N, M):
    for j in [0]:
        last_variable = min(j + M, N)
        for k in range(j, last_variable):
            model.remove(x[k])
        model.optimize()


def bench_cp(N, M):
    env = cp.Envr()
    model = env.createModel("m")

    I = range(N)
    x = model.addVars(I)

    model.setParam("Logging", 0)
    model.setParam("TimeLimit", 0.0)
    model.setParam("Presolve", 0)

    obj = cp.quicksum(i / N * x[i] for i in range(N))
    model.setObjective(obj)

    model.addConstr(x.sum() == N)

    # for j in range(0, N, M):
    for j in [0]:
        last_variable = min(j + M, N)
        for k in range(j, last_variable):
            x[k].remove()
        model.solve()


def main(N, M):
    result = dict()

    t1 = time.perf_counter()
    bench_poi_gurobi(N, M)
    t2 = time.perf_counter()
    result["poi_gurobi"] = t2 - t1

    t1 = time.perf_counter()
    bench_gp(N, M)
    t2 = time.perf_counter()
    result["gurobipy"] = t2 - t1

    t1 = time.perf_counter()
    bench_poi_copt(N, M)
    t2 = time.perf_counter()
    result["poi_copt"] = t2 - t1

    t1 = time.perf_counter()
    bench_cp(N, M)
    t2 = time.perf_counter()
    result["coptpy"] = t2 - t1

    return result


result_dict = dict()
for N in [1000000]:
    for M in [100000]:
        print(f"N = {N}, M = {M}")
        result = main(N, M)
        result_dict[(N, M)] = result

print(result_dict)
