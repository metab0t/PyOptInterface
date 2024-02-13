from mip.model import *
from mip import GUROBI
import os, time


def solve_nqueens(n):
    model = Model("queens", solver_name=GUROBI)
    # set parameters
    model.solver.set_int_param("OutputFlag", 0)
    model.solver.set_dbl_param("TimeLimit", 0.0)
    model.solver.set_int_param("Presolve", 0)

    x = dict(((i, j), model.add_var(var_type="B")) for i in range(n) for j in range(n))

    # one per row
    for i in range(n):
        model.add_constr(xsum(x[i, j] for j in range(n)) == 1)

    # one per column
    for j in range(n):
        model.add_constr(xsum(x[i, j] for i in range(n)) == 1)

    # diagonal \
    for i in range(2 * n - 1):
        J = range(max(0, i - n + 1), min(n, i + 1))
        gen = (x[j, i - j] for j in J)
        model.add_constr(
            xsum(gen) <= 1,
        )

    # diagonal /
    for i in range(2 * n - 1):
        J = range(max(0, i - n + 1), min(n, i + 1))
        gen = (x[n - 1 - j, i - j] for j in J)
        model.add_constr(
            xsum(gen) <= 1,
        )

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