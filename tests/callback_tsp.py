# This file is adapted from the examples/python/tsp.py in Gurobi installation.
# We use this file to ensure our callback implementation is correct and the result is compared with gurobipy
# this test is currently run manually

# Copyright 2024, Gurobi Optimization, LLC

# Solve a traveling salesman problem on a randomly generated set of points
# using lazy constraints.  The base MIP model only includes 'degree-2'
# constraints, requiring each node to have exactly two incident edges.
# Solutions to this model may contain subtours - tours that don't visit every
# city.  The lazy constraint callback adds new constraints to cut them off.

import sys
import logging
import math
import random
import time
from collections import defaultdict
from itertools import combinations

import pyoptinterface as poi
from pyoptinterface import gurobi

import gurobipy as gp
from gurobipy import GRB


def shortest_subtour(edges):
    """Given a list of edges, return the shortest subtour (as a list of nodes)
    found by following those edges. It is assumed there is exactly one 'in'
    edge and one 'out' edge for every node represented in the edge list."""

    # Create a mapping from each node to its neighbours
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


class TSPCallback:
    """Callback class implementing lazy constraints for the TSP.  At MIPSOL
    callbacks, solutions are checked for subtours and subtour elimination
    constraints are added if needed."""

    def __init__(self, nodes, x):
        self.nodes = nodes
        self.x = x

    def run_poi(self, model, where):
        """Callback entry point: call lazy constraints routine when new
        solutions are found. Stop the optimization if there is an exception in
        user code."""
        if where == GRB.Callback.MIPSOL:
            try:
                self.eliminate_subtours_poi(model)
            except Exception:
                logging.exception("Exception occurred in MIPSOL callback")
                exit()

    def eliminate_subtours_poi(self, model):
        """Extract the current solution, check for subtours, and formulate lazy
        constraints to cut off the current solution if subtours are found.
        Assumes we are at MIPSOL."""
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

    def __call__(self, model, where):
        """Callback entry point: call lazy constraints routine when new
        solutions are found. Stop the optimization if there is an exception in
        user code."""
        if where == GRB.Callback.MIPSOL:
            try:
                self.eliminate_subtours_gurobipy(model)
            except Exception:
                logging.exception("Exception occurred in MIPSOL callback")
                exit()

    def eliminate_subtours_gurobipy(self, model):
        """Extract the current solution, check for subtours, and formulate lazy
        constraints to cut off the current solution if subtours are found.
        Assumes we are at MIPSOL."""
        values = model.cbGetSolution(self.x)
        edges = [(i, j) for (i, j), v in values.items() if v > 0.5]
        tour = shortest_subtour(edges)
        if len(tour) < len(self.nodes):
            # add subtour elimination constraint for every pair of cities in tour
            model.cbLazy(
                gp.quicksum(self.x[i, j] for i, j in combinations(tour, 2))
                <= len(tour) - 1
            )


def solve_tsp_poi(nodes, distances):
    """
    Solve a dense symmetric TSP using the following base formulation:

    min  sum_ij d_ij x_ij
    s.t. sum_j x_ij == 2   forall i in V
         x_ij binary       forall (i,j) in E

    and subtours eliminated using lazy constraints.
    """

    m = gurobi.Model()

    # Create variables, and add symmetric keys to the resulting dictionary
    # 'x', such that (i, j) and (j, i) refer to the same variable.
    x = m.add_variables(distances.keys(), domain=poi.VariableDomain.Binary, name="e")
    m.set_objective(poi.quicksum(distances[k] * x[k] for k in distances))
    for i, j in distances.keys():
        x[j, i] = x[i, j]

    # Create degree 2 constraints
    for i in nodes:
        m.add_linear_constraint(
            poi.quicksum(x[i, j] for j in nodes if i != j), poi.Eq, 2
        )

    # Optimize model using lazy constraints to eliminate subtours
    m.set_raw_parameter("OutputFlag", 0)
    m.set_raw_parameter("LazyConstraints", 1)
    cb = TSPCallback(nodes, x)
    m.set_callback(cb.run_poi)
    m.optimize()

    # Extract the solution as a tour
    edges = [(i, j) for (i, j), v in x.items() if m.get_value(v) > 0.5]
    tour = shortest_subtour(edges)
    assert set(tour) == set(nodes)

    objval = m.get_model_attribute(poi.ModelAttribute.ObjectiveValue)

    return tour, objval


def solve_tsp_gurobipy(nodes, distances):
    """
    Solve a dense symmetric TSP using the following base formulation:

    min  sum_ij d_ij x_ij
    s.t. sum_j x_ij == 2   forall i in V
         x_ij binary       forall (i,j) in E

    and subtours eliminated using lazy constraints.
    """

    with gp.Env() as env, gp.Model(env=env) as m:
        # Create variables, and add symmetric keys to the resulting dictionary
        # 'x', such that (i, j) and (j, i) refer to the same variable.
        x = m.addVars(distances.keys(), obj=distances, vtype=GRB.BINARY, name="e")
        x.update({(j, i): v for (i, j), v in x.items()})

        # Create degree 2 constraints
        for i in nodes:
            m.addConstr(gp.quicksum(x[i, j] for j in nodes if i != j) == 2)

        # Optimize model using lazy constraints to eliminate subtours
        m.Params.OutputFlag = 0
        m.Params.LazyConstraints = 1
        cb = TSPCallback(nodes, x)
        m.optimize(cb)

        # Extract the solution as a tour
        edges = [(i, j) for (i, j), v in x.items() if v.X > 0.5]
        tour = shortest_subtour(edges)
        assert set(tour) == set(nodes)

        return tour, m.ObjVal


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


if __name__ == "__main__":
    seed = 87651234

    for npoints in range(10, 80, 10):
        nodes, distances = create_map(npoints, seed)

        print(f"npoints = {npoints}")

        t0 = time.time()
        tour1, cost1 = solve_tsp_gurobipy(nodes, distances)
        t1 = time.time()
        print(f"\t gurobipy time: {t1 - t0:g} seconds")

        t0 = time.time()
        tour2, cost2 = solve_tsp_poi(nodes, distances)
        t1 = time.time()
        print(f"\t poi time: {t1 - t0:g} seconds")

        assert tour1 == tour2
        assert abs(cost1 - cost2) < 1e-6
