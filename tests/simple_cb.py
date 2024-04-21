import pyoptinterface as poi
from pyoptinterface import gurobi

GRB = gurobi.GRB


def simple_cb():
    model = gurobi.Model()

    x = model.add_variable(lb=0.0, ub=20.0)
    y = model.add_variable(lb=8.0, ub=20.0)

    model.set_variable_attribute(x, poi.VariableAttribute.Name, "x")
    model.set_variable_attribute(y, poi.VariableAttribute.Name, "y")

    obj = x * x + y * y
    model.set_objective(obj, poi.ObjectiveSense.Minimize)

    conexpr = x + y
    con1 = model.add_linear_constraint(
        conexpr, poi.ConstraintSense.GreaterEqual, 10.0, name="con1"
    )

    def cb(model, where):
        if where == GRB.Callback.PRESOLVE:
            runtime = model.cb_get_info(GRB.Callback.RUNTIME)
            coldel = model.cb_get_info(GRB.Callback.PRE_COLDEL)
            rowdel = model.cb_get_info(GRB.Callback.PRE_ROWDEL)
            print(f"Runtime: {runtime}, Coldel: {coldel}, Rowdel: {rowdel}")

    model.set_callback(cb)

    model.optimize()


simple_cb()
