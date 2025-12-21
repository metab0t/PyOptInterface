import pytest
import platform

from pyoptinterface import gurobi, xpress, copt, mosek, highs, ipopt

nlp_model_dict = {}

if ipopt.is_library_loaded():

    def llvm():
        return ipopt.Model(jit="LLVM")

    def c():
        return ipopt.Model(jit="C")

    nlp_model_dict["ipopt_llvm"] = llvm
    system = platform.system()
    if system != "Darwin":
        # On macOS, loading dynamic library of Gurobi/Xpress/COPT/Mosek before loading libtcc will cause memory error
        # The reason is still unclear
        nlp_model_dict["ipopt_c"] = c

if copt.is_library_loaded():
    nlp_model_dict["copt"] = copt.Model


@pytest.fixture(params=nlp_model_dict.keys())
def nlp_model_ctor(request):
    name = request.param
    ctor = nlp_model_dict[name]
    return ctor


model_interface_dict = {}

if gurobi.is_library_loaded():
    model_interface_dict["gurobi"] = gurobi.Model
if xpress.is_library_loaded():
    model_interface_dict["xpress"] = xpress.Model
if copt.is_library_loaded():
    model_interface_dict["copt"] = copt.Model
if mosek.is_library_loaded():
    model_interface_dict["mosek"] = mosek.Model
if highs.is_library_loaded():
    model_interface_dict["highs"] = highs.Model


@pytest.fixture(params=model_interface_dict.keys())
def model_interface(request):
    name = request.param
    model_interface_class = model_interface_dict[name]
    return model_interface_class()
