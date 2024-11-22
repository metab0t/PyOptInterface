import pytest

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

    def f():
        return ipopt.Model(jit="C")

    def g():
        return ipopt.Model(jit="LLVM")

    ipopt_model_dict["ipopt_c"] = f
    ipopt_model_dict["ipopt_llvm"] = g


@pytest.fixture(params=ipopt_model_dict.keys())
def ipopt_model_ctor(request):
    name = request.param
    ctor = ipopt_model_dict[name]
    return ctor
