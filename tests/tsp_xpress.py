# TSP example using numpy functions (for efficiency)
#
# (C) Fair Isaac Corp., 1983-2025

from typing import List, Tuple
import math
import random
import time
from collections import defaultdict
from itertools import combinations

import pyoptinterface as poi
from pyoptinterface import xpress

import xpress as xp
import numpy as np

XPRS = xpress.XPRS


def cb_preintsol(prob, data, soltype, cutoff):
    """Callback for checking if solution is acceptable"""

    n = data
    xsol = prob.getCallbackSolution()
    xsolf = np.array(xsol)
    xsol = xsolf.reshape(n, n)
    nextc = np.argmax(xsol, axis=1)

    i = 0
    ncities = 1

    while nextc[i] != 0 and ncities < n:
        ncities += 1
        i = nextc[i]

    reject = False
    if ncities < n:
        if soltype != 0:
            reject = True
        else:
            unchecked = np.zeros(n)
            ngroup = 0

            cut_mstart = [0]
            cut_ind = []
            cut_coe = []
            cut_rhs = []

            nnz = 0
            ncuts = 0

            while np.min(unchecked) == 0 and ngroup <= n:
                """Seek a tour"""

                ngroup += 1
                firstcity = np.argmin(unchecked)
                i = firstcity
                ncities = 0
                while True:
                    unchecked[i] = ngroup
                    ncities += 1
                    i = nextc[i]

                    if i == firstcity or ncities > n + 1:
                        break

                S = np.where(unchecked == ngroup)[0].tolist()
                compS = np.where(unchecked != ngroup)[0].tolist()

                indices = [i * n + j for i in S for j in compS]

                if sum(xsolf[i] for i in indices) < 1 - 1e-3:
                    mcolsp, dvalp = [], []

                    drhsp, status = prob.presolverow(
                        rowtype="G",
                        origcolind=indices,
                        origrowcoef=np.ones(len(indices)),
                        origrhs=1,
                        maxcoefs=prob.attributes.cols,
                        colind=mcolsp,
                        rowcoef=dvalp,
                    )
                    assert status == 0

                    nnz += len(mcolsp)
                    ncuts += 1

                    cut_ind.extend(mcolsp)
                    cut_coe.extend(dvalp)
                    cut_rhs.append(drhsp)
                    cut_mstart.append(nnz)

                if ncuts > 0:
                    prob.addcuts(
                        cuttype=[0] * ncuts,
                        rowtype=["G"] * ncuts,
                        rhs=cut_rhs,
                        start=cut_mstart,
                        colind=cut_ind,
                        cutcoef=cut_coe,
                    )

    return (reject, None)


def print_sol(p, n):
    """Print the solution: order of nodes and cost"""

    xsol = np.array(p.getSolution()).reshape(n, n)
    nextc = np.argmax(xsol, axis=1)

    i = 0

    tour = []
    while i != 0 or len(tour) == 0:
        tour.append(str(i))
        i = nextc[i]
    print("->".join(tour), "->0; cost: ", p.attributes.objval, sep="")


def create_initial_tour(n):
    """Returns a permuted trivial solution 0->1->2->...->(n-1)->0"""
    sol = np.zeros((n, n))
    p = np.random.permutation(n)
    for i in range(n):
        sol[p[i], p[(i + 1) % n]] = 1
    return sol.flatten()


def solve_xpress(nodes, distances):
    n = len(nodes)
    nodes = range(n)
    p = xp.problem()
    p.controls.outputlog = 0

    fly = np.array(
        [
            p.addVariable(vartype=xp.binary, name=f"x_{i}_{j}")
            for i in nodes
            for j in nodes
        ],
        dtype=xp.npvar,
    ).reshape(n, n)

    # Outgoing constraints: sum of outgoing arcs from i equals 1
    for i in nodes:
        p.addConstraint(xp.Sum(fly[i, :]) - fly[i, i] == 1)

    # Incoming constraints: sum of incoming arcs to i equals 1
    for i in nodes:
        p.addConstraint(xp.Sum(fly[:, i]) - fly[i, i] == 1)

    # No self-loops
    for i in nodes:
        p.addConstraint(fly[i, i] == 0)

    p.setObjective(xp.Sum(fly[i, j] * distances[i, j] for i in nodes for j in nodes))
    p.addcbpreintsol(cb_preintsol, n)
    p.controls.mipdualreductions = 0

    for k in range(10):
        InitTour = create_initial_tour(n)
        p.addmipsol(solval=InitTour, name=f"InitTour_{k}")

    p.optimize()

    if p.attributes.solstatus not in [xp.SolStatus.OPTIMAL, xp.SolStatus.FEASIBLE]:
        print("Solve status:", p.attributes.solvestatus.name)
        print("Solution status:", p.attributes.solstatus.name)
    else:
        print_sol(p, n)

    xvals = np.array(p.getSolution()).reshape(n, n)
    edges = [(i, j) for i in nodes for j in nodes if xvals[i, j] > 0.5]
    print(edges)

    tour = shortest_subtour(edges)
    objval = p.attributes.objval

    return tour, objval


def shortest_subtour(edges: List[Tuple[int, int]]) -> List[int]:
    node_neighbors = defaultdict(list)
    for i, j in edges:
        node_neighbors[i].append(j)

    # Follow edges to find cycles. Each time a new cycle is found, keep track
    # of the shortest cycle found so far and restart from an unvisited node.
    unvisited = set(node_neighbors)
    shortest = None
    while unvisited:
        cycle = []
        neighbors = list(unvisited)
        while neighbors:
            current = neighbors.pop()
            cycle.append(current)
            unvisited.remove(current)
            neighbors = [j for j in node_neighbors[current] if j in unvisited]
        if shortest is None or len(cycle) < len(shortest):
            shortest = cycle

    assert shortest is not None
    return shortest


def solve_poi(f, nodes, distances):
    n = len(nodes)
    m = f()

    fly = np.array(
        [
            m.add_variable(name=f"x_{i}_{j}", domain=poi.VariableDomain.Binary)
            for i in nodes
            for j in nodes
        ],
        dtype=object,  # Changed from xp.npvar
    ).reshape(n, n)

    # Outgoing constraints: sum of outgoing arcs from i equals 1
    for i in nodes:
        m.add_linear_constraint(poi.quicksum(fly[i, :]) - fly[i, i], poi.Eq, 1)

    # Incoming constraints: sum of incoming arcs to i equals 1
    for i in nodes:
        m.add_linear_constraint(poi.quicksum(fly[:, i]) - fly[i, i], poi.Eq, 1)

    # No self-loops
    for i in nodes:
        m.add_linear_constraint(fly[i, i], poi.Eq, 0)

    m.set_objective(
        poi.quicksum(fly[i, j] * distances[i, j] for i in nodes for j in nodes)
    )

    def eliminate_subtours_poi(model):
        edges = [
            (i, j)
            for (i, j), v in np.ndenumerate(fly)
            if model.cb_get_solution(v) > 0.5
        ]
        tour = shortest_subtour(edges)
        if len(tour) < len(nodes):
            print("                    Shortest subtour:", tour)
            print(
                f"                    Adding new cut with {len(tour)**2 - len(tour)} nonzeros."
            )
            model.cb_add_lazy_constraint(
                poi.quicksum(fly[i, j] + fly[j, i] for i, j in combinations(tour, 2)),
                poi.Leq,
                len(tour) - 1,
            )

    def cb(model, ctx):
        args = model.cb_get_arguments()
        if ctx == XPRS.CB_CONTEXT.MESSAGE and args.msgtype > 0:
            print(f"{ctx.name:>16}: {args.msg}")
        if ctx == XPRS.CB_CONTEXT.BARITERATION:
            print(
                f"{ctx.name:>16}: Barrier iter {model.get_raw_attribute("XPRS_BARITER")}, primal {model.get_raw_attribute("XPRS_BARPRIMALOBJ")}, dual {model.get_raw_attribute("XPRS_BARDUALOBJ")}, primal inf {model.get_raw_attribute("XPRS_BARPRIMALINF")}, dual inf{model.get_raw_attribute("XPRS_BARDUALINF")}, gap {model.get_raw_attribute("XPRS_BARCGAP")}"
            )
        if ctx == XPRS.CB_CONTEXT.BARLOG:
            print(
                f"{ctx.name:>16}: Barrier iter {model.get_raw_attribute("XPRS_BARITER")}, primal {model.get_raw_attribute("XPRS_BARPRIMALOBJ")}, dual {model.get_raw_attribute("XPRS_BARDUALOBJ")}, primal inf {model.get_raw_attribute("XPRS_BARPRIMALINF")}, dual inf{model.get_raw_attribute("XPRS_BARDUALINF")}, gap {model.get_raw_attribute("XPRS_BARCGAP")}"
            )
        if ctx == XPRS.CB_CONTEXT.AFTEROBJECTIVE:
            print(
                f"{ctx.name:>16}: Completed obj solve {model.get_raw_attribute("XPRS_SOLVEDOBJS")}"
            )
        if ctx == XPRS.CB_CONTEXT.BEFOREOBJECTIVE:
            print(
                f"{ctx.name:>16}: Starting obj solve {model.get_raw_attribute("XPRS_SOLVEDOBJS")}"
            )
        if ctx == XPRS.CB_CONTEXT.PRESOLVE:
            runtime = model.get_raw_attribute_dbl_by_id(XPRS.TIME)
            coldel = model.get_raw_attribute_int_by_id(
                XPRS.ORIGINALCOLS
            ) - model.get_raw_attribute_int_by_id(XPRS.COLS)
            rowdel = model.get_raw_attribute_int_by_id(
                XPRS.ORIGINALROWS
            ) - model.get_raw_attribute_int_by_id(XPRS.ROWS)
            print(
                f"{ctx.name:>16}: Runtime: {runtime}, Coldel: {coldel}, Rowdel: {rowdel}"
            )
        if ctx == XPRS.CB_CONTEXT.CHECKTIME:
            print(
                f"{ctx.name:>16}: {model.get_raw_attribute("XPRS_TIME")} seconds have passed."
            )
        if ctx == XPRS.CB_CONTEXT.CHGBRANCHOBJECT:
            print(f"{ctx.name:>16}: Not a lot to print here at the moment")
        if ctx == XPRS.CB_CONTEXT.CUTLOG:
            print(
                f"{ctx.name:>16}: You should see the cutlog somewhere near this message."
            )
        if ctx == XPRS.CB_CONTEXT.CUTROUND:
            print(
                f"{ctx.name:>16}: The optimizer would have done another cut round? {args.ifxpresscuts} - Forcing it."
            )
            args.p_action = 1
        if ctx == XPRS.CB_CONTEXT.DESTROYMT:
            print(f"{ctx.name:>16}: Somewhere someone is killing a MIP Thread. RIP :(")
        if ctx == XPRS.CB_CONTEXT.GAPNOTIFY:
            obj = model.get_raw_attribute_dbl_by_id(XPRS.MIPOBJVAL)
            bound = model.get_raw_attribute_dbl_by_id(XPRS.BESTBOUND)
            gap = 0
            if obj != 0 or bound != 0:
                gap = abs(obj - bound) / max(abs(obj), abs(bound))
            print(f"{ctx.name:>16}: Current gap {gap}, next target set to {gap/2}")
        if ctx == XPRS.CB_CONTEXT.MIPLOG:
            print(
                f"{ctx.name:>16}: Node {model.get_raw_attribute("XPRS_CURRENTNODE")} with depth {model.get_raw_attribute("XPRS_NODEDEPTH")} has just been processed"
            )
        if ctx == XPRS.CB_CONTEXT.INFNODE:
            print(
                f"{ctx.name:>16}: Infeasible node id {model.get_raw_attribute("XPRS_CURRENTNODE")}"
            )
        if ctx == XPRS.CB_CONTEXT.INTSOL:
            print(
                f"{ctx.name:>16}: Integer solution value: {model.get_raw_attribute("XPRS_MIPOBJVAL")}"
            )
        if ctx == XPRS.CB_CONTEXT.LPLOG:
            print(
                f"{ctx.name:>16}: At iteration {model.get_raw_attribute("XPRS_SIMPLEXITER")} objval is {model.get_raw_attribute("XPRS_LPOBJVAL")}"
            )
        if ctx == XPRS.CB_CONTEXT.NEWNODE:
            print(
                f"{ctx.name:>16}: New node id {args.node}, parent node {args.parentnode}, branch {args.branch}"
            )
        # if ctx == XPRS.CB_CONTEXT.MIPTHREAD:
        #    print(f"{ctx.name:>16}: Not a lot to print here at the moment")
        if ctx == XPRS.CB_CONTEXT.NODECUTOFF:
            print(f"{ctx.name:>16}: Node {args.node} cut off.")
        if ctx == XPRS.CB_CONTEXT.NODELPSOLVED:
            obj = model.get_raw_attribute_dbl_by_id(XPRS.LPOBJVAL)
            print(
                f"{ctx.name:>16}: Solved relaxation at node {model.get_raw_attribute("XPRS_CURRENTNODE")}, lp obj {obj}"
            )
        if ctx == XPRS.CB_CONTEXT.OPTNODE:
            obj = model.get_raw_attribute_dbl_by_id(XPRS.LPOBJVAL)
            print(
                f"{ctx.name:>16}: Finished processing node {model.get_raw_attribute("XPRS_CURRENTNODE")}, lp obj {obj}"
            )
        if ctx == XPRS.CB_CONTEXT.PREINTSOL:
            print(
                f"{ctx.name:>16}: Candidate integer solution objective {model.get_raw_attribute("LPOBJVAL")}, soltype: {args.soltype}, p_reject: {args.p_reject}, p_cutoff: {args.p_cutoff}"
            )
            eliminate_subtours_poi(model)
        if ctx == XPRS.CB_CONTEXT.PRENODE:
            print(f"{ctx.name:>16}: Node optimization is about to start...")
        if ctx == XPRS.CB_CONTEXT.USERSOLNOTIFY:
            print(
                f"{ctx.name:>16}: Solution {args.solname} was processed resulting in status {args.status}."
            )

    m.set_callback(
        cb,
        XPRS.CB_CONTEXT.MESSAGE
        | XPRS.CB_CONTEXT.BARITERATION
        | XPRS.CB_CONTEXT.BARLOG
        | XPRS.CB_CONTEXT.AFTEROBJECTIVE
        | XPRS.CB_CONTEXT.BEFOREOBJECTIVE
        | XPRS.CB_CONTEXT.PRESOLVE
        | XPRS.CB_CONTEXT.CHECKTIME
        | XPRS.CB_CONTEXT.CHGBRANCHOBJECT
        | XPRS.CB_CONTEXT.CUTLOG
        | XPRS.CB_CONTEXT.CUTROUND
        | XPRS.CB_CONTEXT.DESTROYMT
        | XPRS.CB_CONTEXT.GAPNOTIFY
        | XPRS.CB_CONTEXT.MIPLOG
        | XPRS.CB_CONTEXT.INFNODE
        | XPRS.CB_CONTEXT.INTSOL
        | XPRS.CB_CONTEXT.LPLOG
        # |XPRS.CB_CONTEXT.MIPTHREAD
        | XPRS.CB_CONTEXT.NEWNODE
        | XPRS.CB_CONTEXT.NODECUTOFF
        | XPRS.CB_CONTEXT.NODELPSOLVED
        | XPRS.CB_CONTEXT.OPTNODE
        | XPRS.CB_CONTEXT.PREINTSOL
        | XPRS.CB_CONTEXT.PRENODE
        | XPRS.CB_CONTEXT.USERSOLNOTIFY,
    )
    m.set_raw_control_int_by_id(XPRS.CALLBACKCHECKTIMEDELAY, 10)
    m.set_raw_control_dbl_by_id(XPRS.MIPRELGAPNOTIFY, 1.0)
    m.set_raw_control("XPRS_MIPDUALREDUCTIONS", 0)
    m.optimize()

    # Extract the solution as a tour
    edges = [(i, j) for (i, j), v in np.ndenumerate(fly) if m.get_value(v) > 0.5]
    tour = shortest_subtour(edges)

    objval = m.get_model_attribute(poi.ModelAttribute.ObjectiveValue)

    return tour, objval


def create_map(npoints, seed):
    # Create n random points in 2D
    random.seed(seed)
    nodes = list(range(npoints))
    points = [(random.randint(0, 100), random.randint(0, 100)) for _ in nodes]

    # Dictionary of Euclidean distance between each pair of points
    distances = {
        (i, j): math.sqrt(sum((points[i][k] - points[j][k]) ** 2 for k in range(2)))
        for i in nodes
        for j in nodes
    }
    return nodes, distances


def test_xpress(npoints_series, seed):
    for npoints in npoints_series:
        nodes, distances = create_map(npoints, seed)

        print(f"npoints = {npoints}")

        t0 = time.time()
        f = xpress.Model
        _, cost2 = solve_poi(f, nodes, distances)
        t1 = time.time()
        print(f"\t poi time: {t1 - t0:g} seconds")
        print(f"POI solution value: {cost2}")

        t0 = time.time()
        _, cost1 = solve_xpress(nodes, distances)
        t1 = time.time()
        print(f"\t xpress time: {t1 - t0:g} seconds")
        print(f"Xpress solution value: {cost1}")


if __name__ == "__main__":
    seed = 987651234

    X = range(20, 10000, 10000)
    test_xpress(X, seed)
