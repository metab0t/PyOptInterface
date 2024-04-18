# This file is adapted from the examples/python/tsp.py in Gurobi installation.
# We use this file to ensure our callback implementation is correct and the result is compared with gurobipy/coptpy
# this test is currently run manually

# Copyright 2024, Gurobi Optimization, LLC

from typing import List, Tuple
import math
import random
import time
from collections import defaultdict
from itertools import combinations

import pyoptinterface as poi
from pyoptinterface import gurobi, copt

import gurobipy as gp
from gurobipy import GRB

import coptpy as cp
from coptpy import COPT


def shortest_subtour(edges: List[Tuple[int, int]]) -> List[int]:
    node_neighbors = defaultdict(list)
    for i, j in edges:
        node_neighbors[i].append(j)
    assert all(len(neighbors) == 2 for neighbors in node_neighbors.values())

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


class GurobiTSPCallback:
    def __init__(self, nodes, x):
        self.nodes = nodes
        self.x = x

    def __call__(self, model, where):
        if where == GRB.Callback.MIPSOL:
            self.eliminate_subtours_gurobipy(model)

    def eliminate_subtours_gurobipy(self, model):
        values = model.cbGetSolution(self.x)
        edges = [(i, j) for (i, j), v in values.items() if v > 0.5]
        tour = shortest_subtour(edges)
        if len(tour) < len(self.nodes):
            # add subtour elimination constraint for every pair of cities in tour
            model.cbLazy(
                gp.quicksum(self.x[i, j] for i, j in combinations(tour, 2))
                <= len(tour) - 1
            )


def solve_tsp_gurobipy(nodes, distances):
    """
    Solve a dense symmetric TSP using the following base formulation:

    min  sum_ij d_ij x_ij
    s.t. sum_j x_ij == 2   forall i in V
         x_ij binary       forall (i,j) in E

    and subtours eliminated using lazy constraints.
    """

    m = gp.Model()

    x = m.addVars(distances.keys(), obj=distances, vtype=GRB.BINARY, name="e")
    x.update({(j, i): v for (i, j), v in x.items()})

    # Create degree 2 constraints
    for i in nodes:
        m.addConstr(gp.quicksum(x[i, j] for j in nodes if i != j) == 2)

    m.Params.OutputFlag = 0
    m.Params.LazyConstraints = 1
    cb = GurobiTSPCallback(nodes, x)
    m.optimize(cb)

    edges = [(i, j) for (i, j), v in x.items() if v.X > 0.5]
    tour = shortest_subtour(edges)
    assert set(tour) == set(nodes)

    return tour, m.ObjVal


class POITSPCallback:
    def __init__(self, nodes, x):
        self.nodes = nodes
        self.x = x

    def run_gurobi(self, model, where):
        if where == GRB.Callback.MIPSOL:
            self.eliminate_subtours_poi(model)

    def run_copt(self, model, where):
        if where == COPT.CBCONTEXT_MIPSOL:
            self.eliminate_subtours_poi(model)

    def eliminate_subtours_poi(self, model):
        edges = []
        for (i, j), xij in self.x.items():
            v = model.cb_get_solution(xij)
            if v > 0.5:
                edges.append((i, j))
        tour = shortest_subtour(edges)
        if len(tour) < len(self.nodes):
            # add subtour elimination constraint for every pair of cities in tour
            model.cb_add_lazy_constraint(
                poi.quicksum(self.x[i, j] for i, j in combinations(tour, 2)),
                poi.Leq,
                len(tour) - 1,
            )


def solve_tsp_poi(f, nodes, distances):
    m = f()
    x = m.add_variables(distances.keys(), domain=poi.VariableDomain.Binary, name="e")
    m.set_objective(poi.quicksum(distances[k] * x[k] for k in distances))
    for i, j in distances.keys():
        x[j, i] = x[i, j]

    for i in nodes:
        m.add_linear_constraint(
            poi.quicksum(x[i, j] for j in nodes if i != j), poi.Eq, 2
        )

    m.set_model_attribute(poi.ModelAttribute.Silent, True)
    cb = POITSPCallback(nodes, x)
    if isinstance(m, gurobi.Model):
        m.set_raw_parameter("LazyConstraints", 1)
        m.set_callback(cb.run_gurobi)
    elif isinstance(m, copt.Model):
        m.set_callback(cb.run_copt, COPT.CBCONTEXT_MIPSOL)
    m.optimize()

    # Extract the solution as a tour
    edges = [(i, j) for (i, j), v in x.items() if m.get_value(v) > 0.5]
    tour = shortest_subtour(edges)
    assert set(tour) == set(nodes)

    objval = m.get_model_attribute(poi.ModelAttribute.ObjectiveValue)

    return tour, objval


class COPTTSPCallback(cp.CallbackBase):
    def __init__(self, nodes, x):
        super().__init__()
        self.nodes = nodes
        self.x = x

    def callback(self):
        if self.where() == COPT.CBCONTEXT_MIPSOL:
            self.eliminate_subtours_coptpy()

    def eliminate_subtours_coptpy(self):
        values = self.getSolution(self.x)
        edges = [(i, j) for (i, j), v in values.items() if v > 0.5]
        tour = shortest_subtour(edges)
        if len(tour) < len(self.nodes):
            # add subtour elimination constraint for every pair of cities in tour
            self.addLazyConstr(
                cp.quicksum(self.x[i, j] for i, j in combinations(tour, 2))
                <= len(tour) - 1
            )


def solve_tsp_coptpy(nodes, distances):
    env = cp.Envr()
    m = env.createModel("TSP Callback Example")

    x = m.addVars(distances.keys(), vtype=COPT.BINARY, nameprefix="e")
    for (i, j), v in x.items():
        v.setInfo(COPT.Info.Obj, distances[i, j])
    for i, j in distances.keys():
        x[j, i] = x[i, j]

    # Create degree 2 constraints
    for i in nodes:
        m.addConstr(cp.quicksum(x[i, j] for j in nodes if i != j) == 2)

    m.Param.Logging = 0
    cb = COPTTSPCallback(nodes, x)
    m.setCallback(cb, COPT.CBCONTEXT_MIPSOL)
    m.solve()

    edges = [(i, j) for (i, j), v in x.items() if v.x > 0.5]
    tour = shortest_subtour(edges)
    assert set(tour) == set(nodes)

    return tour, m.objval


def create_map(npoints, seed):
    # Create n random points in 2D
    random.seed(seed)
    nodes = list(range(npoints))
    points = [(random.randint(0, 100), random.randint(0, 100)) for i in nodes]

    # Dictionary of Euclidean distance between each pair of points
    distances = {
        (i, j): math.sqrt(sum((points[i][k] - points[j][k]) ** 2 for k in range(2)))
        for i, j in combinations(nodes, 2)
    }

    return nodes, distances


def test_gurobi(npoints_series, seed):
    for npoints in npoints_series:
        nodes, distances = create_map(npoints, seed)

        print(f"npoints = {npoints}")

        t0 = time.time()
        tour1, cost1 = solve_tsp_gurobipy(nodes, distances)
        t1 = time.time()
        print(f"\t gurobipy time: {t1 - t0:g} seconds")

        t0 = time.time()
        f = gurobi.Model
        tour2, cost2 = solve_tsp_poi(f, nodes, distances)
        t1 = time.time()
        print(f"\t poi time: {t1 - t0:g} seconds")

        assert tour1 == tour2
        assert abs(cost1 - cost2) < 1e-6


def test_copt(npoints_series, seed):
    for npoints in npoints_series:
        nodes, distances = create_map(npoints, seed)

        print(f"npoints = {npoints}")

        t0 = time.time()
        tour1, cost1 = solve_tsp_coptpy(nodes, distances)
        t1 = time.time()
        print(f"\t coptpy time: {t1 - t0:g} seconds")

        t0 = time.time()
        f = copt.Model
        tour2, cost2 = solve_tsp_poi(f, nodes, distances)
        t1 = time.time()
        print(f"\t poi time: {t1 - t0:g} seconds")

        assert tour1 == tour2
        assert abs(cost1 - cost2) < 1e-6


if __name__ == "__main__":
    seed = 987651234

    X = range(10, 90, 20)

    test_copt(X, seed)
    test_gurobi(X, seed)
