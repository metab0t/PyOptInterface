import pytest
import platform

model_interface_dict = {}

import pyoptinterface.gurobi as gurobi

if gurobi.is_library_loaded():
    model_interface_dict["gurobi"] = gurobi.Model

import pyoptinterface.copt as copt

if copt.is_library_loaded():
    model_interface_dict["copt"] = copt.Model

import pyoptinterface.mosek as mosek

if mosek.is_library_loaded():
    model_interface_dict["mosek"] = mosek.Model

import pyoptinterface.highs as highs

if highs.is_library_loaded():
    model_interface_dict["highs"] = highs.Model


@pytest.fixture(params=model_interface_dict.keys())
def model_interface(request):
    name = request.param
    model_interface_class = model_interface_dict[name]
    return model_interface_class()


ipopt_model_dict = {}

import pyoptinterface.ipopt as ipopt

if ipopt.is_library_loaded():

    def llvm():
        return ipopt.Model(jit="LLVM")

    def c():
        return ipopt.Model(jit="C")

    ipopt_model_dict["ipopt_llvm"] = llvm

    if platform.system() != "Darwin":
        # Skip the C JIT test on macOS, but it works correctly when run in the terminal
        # needs further investigation
        ipopt_model_dict["ipopt_c"] = c


@pytest.fixture(params=ipopt_model_dict.keys())
def ipopt_model_ctor(request):
    name = request.param
    ctor = ipopt_model_dict[name]
    return ctor
