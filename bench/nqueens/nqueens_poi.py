import pyoptinterface as poi
from pyoptinterface import gurobi
import numpy as np
import os
import time


def solve_nqueens(N):
    model = gurobi.Model()

    x = np.empty((N, N), dtype=object)
    for i in range(N):
        for j in range(N):
            x[i, j] = model.add_variable(domain=poi.VariableDomain.Binary)

    for i in range(N):
        # Row and column
        model.add_linear_constraint(poi.quicksum(x[i, :]), poi.Eq, 1.0)
        model.add_linear_constraint(poi.quicksum(x[:, i]), poi.Eq, 1.0)
    flipx = np.fliplr(x)
    for i in range(-N+1, N):
        # Diagonal
        model.add_linear_constraint(poi.quicksum(x.diagonal(i)), poi.Leq, 1.0)
        # Anti-diagonal
        model.add_linear_constraint(poi.quicksum(flipx.diagonal(i)), poi.Leq, 1.0)

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
