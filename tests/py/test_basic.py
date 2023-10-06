from pyoptinterface_nb import core_ext as ext
import numpy as np


def test_basic():
    vars = [ext.VariableIndex(i) for i in range(10)]
    v = vars[0]
    assert v.index == 0

    t = ext.TermsTable()
    t += v
    t += v * v
    assert t.degree() == 2
    assert t.empty() == False

    t = ext.TermsTable(v)
    t.add_affine_term(vars[1], 2.0)
    t += 3.0 * vars[2]
    t -= 4.0 * vars[3]
    assert t.degree() == 1
    assert t.empty() == False

    saf = ext.ScalarAffineFunction(t)
    saf.canonicalize()
    assert list(saf.variables) == [0, 1, 2, 3]
    assert np.allclose(saf.coefficients, [1.0, 2.0, 3.0, -4.0])
