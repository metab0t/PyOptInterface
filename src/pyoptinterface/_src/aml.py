from .core_ext import VariableDomain, ExprBuilder
from .attributes import VariableAttribute

from yds import tupledict, make_tupledict

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
    f = lambda *args: model.add_variable(**kw_args)
    td = make_tupledict(*coords, rule=f)
    if name is not None:
        for k, v in td.items():
            model.set_variable_attribute(v, VariableAttribute.Name, name + str(k))
    return td


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
