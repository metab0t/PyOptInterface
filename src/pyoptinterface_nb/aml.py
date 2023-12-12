from .core_ext import VariableIndex, VariableDomain, ExprBuilder

import xarray as xr
import numpy as np

from typing import Iterable


def make_variable_array(
    model,
    coords: dict[str, Iterable] | list[Iterable],
    domain: VariableDomain = VariableDomain.Continuous,
):
    assert isinstance(coords, dict) or isinstance(coords, list)
    axis_iter = coords
    if isinstance(coords, dict):
        axis_iter = coords.values()
    lengths = [len(axis) for axis in axis_iter]
    L = np.prod(lengths)
    data = np.full((L,), None)
    for i in range(L):
        data[i] = model.add_variable(domain=domain)
    data = data.reshape(lengths)
    array = xr.DataArray(data, coords=coords)
    return array

def quicksum(variables : xr.DataArray):
    expr = ExprBuilder()
    for v in variables.values.flat:
        expr.add_affine_term(v, 1.0)
    return expr

def quicksum_f(variables : xr.DataArray, f):
    expr = ExprBuilder()
    for v in variables.values.flat:
        expr.add(f(v))
    return expr