from .core_ext import VariableIndex, VariableDomain, ExprBuilder
import numpy as np

from yds import tupledict, make_tupledict

from typing import Iterable


def make_variable_array(
    model,
    *coords: Iterable,
    domain: VariableDomain = VariableDomain.Continuous,
):
    f = lambda *args: model.add_variable(domain)
    return make_tupledict(*coords, rule=f)


def quicksum(variables: tupledict):
    expr = ExprBuilder()
    for v in variables.values():
        expr.add_affine_term(v, 1.0)
    return expr


def quicksum_f(variables: tupledict, f):
    expr = ExprBuilder()
    for v in variables.values():
        expr.add(f(v))
    return expr
