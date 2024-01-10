from .core_ext import VariableDomain, ExprBuilder
from .attributes import VariableAttribute

from .tupledict import tupledict, make_tupledict

from typing import Iterable


def make_nd_variable(
    model,
    *coords: Iterable,
    domain=None,
    lb=None,
    ub=None,
    name=None,
):
    kw_args = dict()
    if domain is not None:
        kw_args["domain"] = domain
    if lb is not None:
        kw_args["lb"] = lb
    if ub is not None:
        kw_args["ub"] = ub

    def f(*args):
        suffix = str(args)
        if name is not None:
            kw_args["name"] = f"{name}{suffix}"
        return model.add_variable(**kw_args)

    td = make_tupledict(*coords, rule=f)
    return td


def quicksum(terms):
    expr = ExprBuilder()
    if isinstance(terms, dict):
        iter = terms.values()
    else:
        iter = terms
    for v in iter:
        expr.add(v)
    return expr


def quicksum_f(terms, f):
    expr = ExprBuilder()
    if isinstance(terms, dict):
        iter = terms.values()
    else:
        iter = terms
    for v in iter:
        expr.add(f(v))
    return expr
