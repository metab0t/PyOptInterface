from gurobipy import *
import numpy as np
import os
import time


def solve_nqueens(N):
    model = Model("queens")
    model.setParam("OutputFlag", 0)
    model.setParam("TimeLimit", 0.0)
    model.setParam("Presolve", 0)

    x = np.empty((N, N), dtype=object)
    for i in range(N):
        for j in range(N):
            x[i, j] = model.addVar(vtype=GRB.BINARY)

    for i in range(N):
        # Row and column
        model.addConstr(quicksum(x[i, :]) == 1.0)
        model.addConstr(quicksum(x[:, i]) == 1.0)
    flipx = np.fliplr(x)
    for i in range(-N + 1, N):
        # Diagonal
        model.addConstr(quicksum(x.diagonal(i)) <= 1.0)
        # Anti-diagonal
        model.addConstr(quicksum(flipx.diagonal(i)) <= 1.0)

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
