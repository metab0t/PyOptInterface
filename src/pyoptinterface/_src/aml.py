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


def quicksum(terms: tupledict):
    expr = ExprBuilder()
    for v in terms.values():
        expr.add(v)
    return expr


def quicksum_f(terms: tupledict, f):
    expr = ExprBuilder()
    for v in terms.values():
        expr.add(f(v))
    return expr
