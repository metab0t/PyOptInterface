from .core_ext import ExprBuilder
from .tupledict import make_tupledict

from collections.abc import Collection


def make_nd_variable(
    model,
    *coords: Collection,
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
        if name is not None:
            suffix = str(args)
            kw_args["name"] = f"{name}{suffix}"
        return model.add_variable(**kw_args)

    td = make_tupledict(*coords, rule=f)
    return td


# def make_nd_variable_batch(
#     model,
#     *coords: Collection,
#     domain=None,
#     lb=None,
#     ub=None,
#     name=None,
# ):
#     assert model.supports_batch_add_variables()
#
#     kw_args = dict()
#     if domain is not None:
#         kw_args["domain"] = domain
#     if lb is not None:
#         kw_args["lb"] = lb
#     if ub is not None:
#         kw_args["ub"] = ub
#
#     N = math.prod(len(c) for c in coords)
#
#     start_vi = model.add_variables(N, **kw_args)
#     start_index = start_vi.index
#
#     kvs = []
#     assert len(coords) > 0
#     for i, coord in enumerate(product(*coords)):
#         coord = tuple(flatten_tuple(coord))
#         value = VariableIndex(start_index + i)
#         if len(coord) == 1:
#             coord = coord[0]
#         if value is not None:
#             kvs.append((coord, value))
#
#         suffix = str(coord)
#         if name is not None:
#             model.set_variable_name(value, f"{name}{suffix}")
#     return tupledict(kvs)


def quicksum_(expr: ExprBuilder, terms, f=None):
    if isinstance(terms, dict):
        iter = terms.values()
    else:
        iter = terms
    if f:
        iter = map(f, iter)
    for v in iter:
        expr += v


def quicksum(terms, f=None):
    expr = ExprBuilder()
    quicksum_(expr, terms, f)
    return expr
