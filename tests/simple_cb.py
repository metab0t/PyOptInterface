import pyoptinterface as poi from pyoptinterface import gurobi, xpress

GRB = gurobi.GRB
XPRS = xpress.XPRS

def simple_cb(f):
    model = f()

    x = model.add_variable(lb=0.0, ub=20.0)
    y = model.add_variable(lb=8.0, ub=20.0)

    model.set_variable_attribute(x, poi.VariableAttribute.Name, "x")
    model.set_variable_attribute(y, poi.VariableAttribute.Name, "y")

    obj = x * x + y * y
    model.set_objective(obj, poi.ObjectiveSense.Minimize)

    conexpr = x + y
    model.add_linear_constraint(conexpr, poi.ConstraintSense.GreaterEqual, 10.0, name="con1")

    def cb(model, where):
        runtime = 0.0
        coldel = 0
        rowdel = 0
        if isinstance(model, gurobi.Model) and where == GRB.Callback.PRESOLVE:
            runtime = model.cb_get_info(GRB.Callback.RUNTIME)
            coldel = model.cb_get_info(GRB.Callback.PRE_COLDEL)
            rowdel = model.cb_get_info(GRB.Callback.PRE_ROWDEL)
            print(f"Runtime: {runtime}, Coldel: {coldel}, Rowdel: {rowdel}")
        if isinstance(model, xpress.Model) and where == XPRS.CB_CONTEXT.PRESOLVE:
            runtime = model.get_raw_attribute_dbl_by_id(XPRS.TIME)
            coldel = model.get_raw_attribute_int_by_id(XPRS.ORIGINALCOLS) - model.get_raw_attribute_int_by_id(XPRS.COLS)
            rowdel = model.get_raw_attribute_int_by_id(XPRS.ORIGINALROWS) - model.get_raw_attribute_int_by_id(XPRS.ROWS)
            print(f"CB[AFTER-PRESOLVE] >> Runtime: {runtime}, Coldel: {coldel}, Rowdel: {rowdel}")
        if isinstance(model, xpress.Model) and where == XPRS.CB_CONTEXT.MESSAGE:
            args = model.cb_get_arguments()
            print(f"CB[MESSAGE-{args.msgtype}] >> {args.msg}")


    if isinstance(model, gurobi.Model):
        model.set_callback(cb)
    elif isinstance(model, xpress.Model):
        model.set_callback(cb, XPRS.CB_CONTEXT.PRESOLVE | XPRS.CB_CONTEXT.MESSAGE)

    model.set_model_attribute(poi.ModelAttribute.Silent, False)
    model.optimize()


if xpress.is_library_loaded():
    simple_cb(xpress.Model)
if gurobi.is_library_loaded():
    simple_cb(gurobi.Model)
