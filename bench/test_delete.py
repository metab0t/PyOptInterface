import gurobipy as gp
import coptpy as cp
import pyoptinterface as poi
from pyoptinterface import gurobi, copt


def bench_poi_base(N):
    model = copt.Model()
    I = range(N)
    x = poi.make_nd_variable(model, I, lb=0.0)

    model.add_linear_constraint(x[0] + x[N - 1], poi.ConstraintSense.GreaterEqual, 1.0)

    model.set_model_attribute(poi.ModelAttribute.Silent, False)
    model.set_model_attribute(poi.ModelAttribute.TimeLimitSec, 2.0)
    model.set_raw_parameter("Presolve", 0)

    obj = poi.ExprBuilder()
    for i in range(N):
        obj.add(x[i] * x[i])
    model.set_objective(obj, poi.ObjectiveSense.Minimize)

    for i in range(N // 2):
        model.delete_variable(x[i])
    model.optimize()


def gp_delete(N):
    model = gp.Model()

    I = range(N)
    x = model.addVars(I, lb=0.0)

    model.addConstr(x[0] + x[N - 1] >= 1.0)

    obj = gp.quicksum(x[i] * x[i] for i in range(N))
    model.setObjective(obj)

    model.setParam("OutputFlag", 1)
    model.setParam("TimeLimit", 2.0)
    model.setParam("Presolve", 0)

    for j in range(N // 2):
        model.remove(x[j])
    model.optimize()


def cp_delete(N):
    env = cp.Envr()
    model = env.createModel("m")

    I = range(N)
    x = model.addVars(I, lb=0.0)

    model.addConstr(x[0] + x[N - 1] >= 1.0)

    obj = cp.quicksum(x[i] * x[i] for i in range(N))
    model.setObjective(obj)

    model.setParam("Logging", 1)
    model.setParam("TimeLimit", 2.0)
    model.setParam("Presolve", 0)

    for j in range(N // 2):
        x[j].remove()
    model.solve()


if __name__ == "__main__":
    N = 20
    cp_delete(N)
