import pytest
import platform

from pyoptinterface import gurobi, xpress, copt, mosek, highs, ipopt, knitro

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

if knitro.is_library_loaded():
    nlp_model_dict["knitro"] = knitro.Model


@pytest.fixture(params=nlp_model_dict.keys())
def nlp_model_ctor(request):
    name = request.param
    ctor = nlp_model_dict[name]
    return ctor


model_interface_dict_full = {}

if gurobi.is_library_loaded():
    model_interface_dict_full["gurobi"] = gurobi.Model
if xpress.is_library_loaded():
    model_interface_dict_full["xpress"] = xpress.Model
if copt.is_library_loaded():
    model_interface_dict_full["copt"] = copt.Model
if mosek.is_library_loaded():
    model_interface_dict_full["mosek"] = mosek.Model
if highs.is_library_loaded():
    model_interface_dict_full["highs"] = highs.Model
if knitro.is_library_loaded():
    model_interface_dict_full["knitro"] = knitro.Model


@pytest.fixture(params=model_interface_dict_full.keys())
def model_interface(request):
    name = request.param
    model_interface_class = model_interface_dict_full[name]
    return model_interface_class()


model_interface_dict_oneshot = model_interface_dict_full.copy()
if ipopt.is_library_loaded():
    model_interface_dict_oneshot["ipopt"] = ipopt.Model


@pytest.fixture(params=model_interface_dict_oneshot.keys())
def model_interface_oneshot(request):
    name = request.param
    model_interface_class = model_interface_dict_oneshot[name]
    return model_interface_class()
