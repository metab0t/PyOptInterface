# Copyright (c) 2022: Miles Lubin and contributors
#
# Use of this source code is governed by an MIT-style license that can be found
# in the LICENSE.md file or at https://opensource.org/licenses/MIT.

import pyoptinterface as poi
from pyoptinterface import gurobi
from pyoptinterface import make_tupledict, make_nd_variable
import os
import time


def solve_facility(model, G, F):
    Grid = range(G + 1)
    Facs = range(1, F + 1)
    Dims = (1, 2)

    y = make_nd_variable(model, Facs, Dims, lb=0.0, ub=1.0)
    s = make_nd_variable(model, Grid, Grid, Facs, lb=0.0)
    z = make_nd_variable(model, Grid, Grid, Facs, domain=poi.VariableDomain.Binary)
    r = make_nd_variable(model, Grid, Grid, Facs, Dims)
    d = model.add_variable()

    obj = poi.ExprBuilder()
    obj.add(d)
    model.set_objective(obj, sense=poi.ObjectiveSense.Minimize)

    def assmt_rule(i, j):
        expr = poi.quicksum(z[i, j, f] for f in Facs)
        con = model.add_linear_constraint(expr, poi.ConstraintSense.Equal, 1.0)
        return con

    assmt = make_tupledict(Grid, Grid, rule=assmt_rule)
    M = 2 * 1.414

    def quadrhs_rule(i, j, f):
        expr = s[i, j, f] - d - M * (1 - z[i, j, f])
        con = model.add_linear_constraint(expr, poi.ConstraintSense.Equal, 0.0)
        return con

    quadrhs = make_tupledict(Grid, Grid, Facs, rule=quadrhs_rule)

    def quaddistk1_rule(i, j, f):
        expr = r[i, j, f, 1] + y[f, 1] - i / G
        con = model.add_linear_constraint(expr, poi.ConstraintSense.Equal, 0.0)
        return con

    quaddistk1 = make_tupledict(Grid, Grid, Facs, rule=quaddistk1_rule)

    def quaddistk2_rule(i, j, f):
        expr = r[i, j, f, 2] + y[f, 2] - j / G
        con = model.add_linear_constraint(expr, poi.ConstraintSense.Equal, 0.0)
        return con

    quaddistk2 = make_tupledict(Grid, Grid, Facs, rule=quaddistk2_rule)

    def quaddist_rule(i, j, f):
        expr = (
            r[i, j, f, 1] * r[i, j, f, 1]
            + r[i, j, f, 2] * r[i, j, f, 2]
            - s[i, j, f] * s[i, j, f]
        )
        con = model.add_quadratic_constraint(expr, poi.ConstraintSense.LessEqual, 0.0)
        return con

    quaddist = make_tupledict(Grid, Grid, Facs, rule=quaddist_rule)

    model.set_model_attribute(poi.ModelAttribute.TimeLimitSec, 0.0)
    model.set_model_raw_parameter("Presolve", 0)
    model.optimize()


def main(Ns=[25, 50, 75, 100]):
    dir = os.path.realpath(os.path.dirname(__file__))
    for n in Ns:
        start = time.time()
        try:
            model = gurobi.Model()
            solve_facility(model, n, n)
        except:
            pass
        run_time = round(time.time() - start)
        with open(dir + "/benchmarks.csv", "a") as io:
            io.write("poi fac-%i -1 %i\n" % (n, run_time))
    return


main()
