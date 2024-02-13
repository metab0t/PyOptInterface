import pyoptinterface as poi
from pyoptinterface import gurobi
import os, time


def solve_nqueens(n):
    model = gurobi.Model()

    x = poi.make_nd_variable(
        model, range(n), range(n), domain=poi.VariableDomain.Binary
    )

    # one per row
    for i in range(n):
        model.add_linear_constraint(
            poi.quicksum(x[i, j] for j in range(n)), poi.ConstraintSense.Equal, 1.0
        )

    # one per column
    for j in range(n):
        model.add_linear_constraint(
            poi.quicksum(x[i, j] for i in range(n)), poi.ConstraintSense.Equal, 1.0
        )

    # diagonal \
    for i in range(2 * n - 1):
        J = range(max(0, i - n + 1), min(n, i + 1))
        gen = (x[j, i - j] for j in J)
        model.add_linear_constraint(
            poi.quicksum(gen),
            poi.ConstraintSense.LessEqual,
            1.0,
        )

    # diagonal /
    for i in range(2 * n - 1):
        J = range(max(0, i - n + 1), min(n, i + 1))
        gen = (x[n - 1 - j, i - j] for j in J)
        model.add_linear_constraint(
            poi.quicksum(gen),
            poi.ConstraintSense.LessEqual,
            1.0,
        )

    model.set_model_attribute(poi.ModelAttribute.Silent, True)
    model.set_model_attribute(poi.ModelAttribute.TimeLimitSec, 0.0)
    model.set_raw_parameter("Presolve", 0)
    model.optimize()


def main(Ns=range(800, 2001, 400)):
    dir = os.path.realpath(os.path.dirname(__file__))
    for n in Ns:
        start = time.time()
        solve_nqueens(n)
        run_time = round(time.time() - start)
        content = "poi nqueens-%i %i" % (n, run_time)
        print(content)
        with open(dir + "/benchmarks.csv", "a") as io:
            io.write(f"{content}\n")
    return

main()