from mip.model import *
from mip import GUROBI
import numpy as np
import os
import time


def solve_nqueens(N):
    model = Model("queens", solver_name=GUROBI)
    # set parameters
    model.solver.set_int_param("OutputFlag", 0)
    model.solver.set_dbl_param("TimeLimit", 0.0)
    model.solver.set_int_param("Presolve", 0)

    x = np.empty((N, N), dtype=object)
    for i in range(N):
        for j in range(N):
            x[i, j] = model.add_var(var_type="B")

    for i in range(N):
        # Row and column
        model.add_constr(xsum(x[i, :]) == 1.0)
        model.add_constr(xsum(x[:, i]) == 1.0)
    flipx = np.fliplr(x)
    for i in range(-N + 1, N):
        # Diagonal
        model.add_constr(xsum(x.diagonal(i)) <= 1.0)
        # Anti-diagonal
        model.add_constr(xsum(flipx.diagonal(i)) <= 1.0)

    model.optimize()


def main(Ns=range(800, 2001, 400)):
    dir = os.path.realpath(os.path.dirname(__file__))
    for n in Ns:
        start = time.time()
        solve_nqueens(n)
        run_time = round(time.time() - start)
        content = "pythonmip nqueens-%i %i" % (n, run_time)
        print(content)
        with open(dir + "/benchmarks.csv", "a") as io:
            io.write(f"{content}\n")
    return


main()
