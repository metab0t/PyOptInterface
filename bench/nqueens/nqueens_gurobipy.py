from gurobipy import *
import os
import time


def solve_nqueens(n):
    model = Model("queens")
    model.setParam("OutputFlag", 0)
    model.setParam("TimeLimit", 0.0)
    model.setParam("Presolve", 0)

    x = model.addVars(n, n, vtype=GRB.BINARY)

    # one per row
    for i in range(n):
        model.addConstr(
            quicksum(x[i, j] for j in range(n)) == 1,
        )

    # one per column
    for j in range(n):
        model.addConstr(quicksum(x[i, j] for i in range(n)) == 1)

    # diagonal \
    for i in range(2 * n - 1):
        J = range(max(0, i - n + 1), min(n, i + 1))
        gen = (x[j, i - j] for j in J)
        model.addConstr(quicksum(gen) <= 1)

    # diagonal /
    for i in range(2 * n - 1):
        J = range(max(0, i - n + 1), min(n, i + 1))
        gen = (x[n - 1 - j, i - j] for j in J)
        model.addConstr(quicksum(gen) <= 1)

    model.optimize()


def main(Ns=range(800, 2001, 400)):
    dir = os.path.realpath(os.path.dirname(__file__))
    for n in Ns:
        start = time.time()
        solve_nqueens(n)
        run_time = round(time.time() - start)
        content = "gurobipy nqueens-%i %i" % (n, run_time)
        print(content)
        with open(dir + "/benchmarks.csv", "a") as io:
            io.write(f"{content}\n")
    return


main()
