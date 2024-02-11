import pyoptinterface as poi
import numpy as np
from pytest import approx

from pyoptinterface._src.core_ext import IntMonotoneIndexer


def test_basic():
    vars = [poi.VariableIndex(i) for i in range(10)]
    v = vars[0]
    assert v.index == 0

    t = poi.ExprBuilder()
    t.add(v)
    t.add(v * v)
    assert t.degree() == 2
    assert t.empty() == False

    t = poi.ExprBuilder(v)
    t.add_affine_term(vars[1], 2.0)
    t.add(3.0 * vars[2])
    t.sub(4.0 * vars[3])
    assert t.degree() == 1
    assert t.empty() == False

    saf = poi.ScalarAffineFunction(t)
    saf.canonicalize()
    assert list(saf.variables) == [0, 1, 2, 3]
    assert np.allclose(saf.coefficients, [1.0, 2.0, 3.0, -4.0])

    saf = 2.0 - 1.0 * saf * -4.0 / 2.0 + 1.0
    saf.canonicalize()
    assert list(saf.variables) == [0, 1, 2, 3]
    assert np.allclose(saf.coefficients, [2.0, 4.0, 6.0, -8.0])
    assert saf.constant == approx(3.0)

    sqf = v * v + 2.0 * v + 1.0
    sqf = 2.0 - 1.0 * sqf * -4.0 / 2.0 + 1.0
    sqf.canonicalize()
    assert list(sqf.variable_1s) == [0]
    assert list(sqf.variable_2s) == [0]
    assert np.allclose(sqf.coefficients, [2.0])
    assert list(sqf.affine_part.variables) == [0]
    assert np.allclose(sqf.affine_part.coefficients, [4.0])
    assert sqf.affine_part.constant == approx(5.0)


def test_monotoneindexer():
    indexer = IntMonotoneIndexer()

    N = 200
    for i in range(N):
        index = indexer.add_index()
        assert index == i

    for i in range(N - 1):
        indexer.delete_index(i)
        x = indexer.get_index(i + 1)
        assert x == 0
        x = indexer.get_index(N - 1)
        assert x == N - 1 - (i + 1)
        x = indexer.get_index(0)
        assert x == -1
