from pyoptinterface import pyoptinterface_ext as ext
import pyoptinterface as pyoi


def b_ext():
    tt = ext.TermsTable()
    for i in range(100):
        tt.add_quadratic_term(i, i, 2.0)
        tt.add_quadratic_term(i, i + 2, 1.0)
    for i in range(100):
        tt.add_affine_term(i, 1.0)
    return ext.t2q(tt)


def b_py():
    tt = pyoi.TermsTable()
    for i in range(100):
        tt.add_quadratic_term(i, i, 2.0)
        tt.add_quadratic_term(i, i + 2, 1.0)
    for i in range(100):
        tt.add_affine_term(i, 1.0)
    return pyoi.t2q(tt)
