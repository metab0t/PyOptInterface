from typing import IO
from pathlib import Path

import gurobipy as gp

GRB = gp.GRB

import coptpy as cp

COPT = cp.COPT


def value_to_str(value):
    if isinstance(value, str):
        return f'"{value}"'
    return str(value)


def extract_gurobi_constants():
    # we want to extract all variables with all UPPERCASE names in dir(GRB)
    GRB_constants = []
    for name in dir(GRB):
        if name.isupper():
            GRB_constants.append(name)

    # extract all variables with first letter in UPPERCASE in dir(GRB.Attr) and dir(GRB.Param) and dir(GRB.Callback)
    GRB_Attr_constants = []
    for name in dir(GRB.Attr):
        if name[0].isupper():
            GRB_Attr_constants.append(name)

    GRB_Param_constants = []
    for name in dir(GRB.Param):
        if name[0].isupper():
            GRB_Param_constants.append(name)

    GRB_Callback_constants = []
    for name in dir(GRB.Callback):
        if name[0].isupper():
            GRB_Callback_constants.append(name)

    return {
        "GRB": GRB_constants,
        "GRB.Attr": GRB_Attr_constants,
        "GRB.Param": GRB_Param_constants,
        "GRB.Callback": GRB_Callback_constants,
    }


def export_gurobi_constants(io: IO[str], gurobi_constants):
    io.write('nb::module_ GRB = m.def_submodule("GRB");\n')
    for name in gurobi_constants["GRB"]:
        value = getattr(GRB, name)
        value = value_to_str(value)
        io.write(f'GRB.attr("{name}") = {value};\n')
    io.write("\n")

    io.write('nb::module_ Attr = GRB.def_submodule("Attr");\n')
    Attr = GRB.Attr
    for name in gurobi_constants["GRB.Attr"]:
        value = getattr(Attr, name)
        value = value_to_str(value)
        io.write(f'Attr.attr("{name}") = {value};\n')
    io.write("\n")

    io.write('nb::module_ Param = GRB.def_submodule("Param");\n')
    Param = GRB.Param
    for name in gurobi_constants["GRB.Param"]:
        value = getattr(Param, name)
        value = value_to_str(value)
        io.write(f'Param.attr("{name}") = {value};\n')
    io.write("\n")

    io.write('nb::module_ Callback = GRB.def_submodule("Callback");\n')
    Callback = GRB.Callback
    for name in gurobi_constants["GRB.Callback"]:
        value = getattr(Callback, name)
        value = value_to_str(value)
        io.write(f'Callback.attr("{name}") = {value};\n')


def extract_copt_constants():
    # dir(COPT)
    COPT_constants = []
    for name in dir(COPT):
        if name.isupper():
            COPT_constants.append(name)

    # dir(COPT.Attr)
    COPT_Attr_constants = []
    for name in dir(COPT.Attr):
        if name[0].isupper():
            COPT_Attr_constants.append(name)

    # dir(COPT.Param)
    COPT_Param_constants = []
    for name in dir(COPT.Param):
        if name[0].isupper():
            COPT_Param_constants.append(name)

    # dir(COPT.Info)
    COPT_Info_constants = []
    for name in dir(COPT.Info):
        if name[0].isupper():
            COPT_Info_constants.append(name)

    # dir(COPT.CbInfo)
    COPT_CbInfo_constants = []
    for name in dir(COPT.CbInfo):
        if name[0].isupper():
            COPT_CbInfo_constants.append(name)

    return {
        "COPT": COPT_constants,
        "COPT.Attr": COPT_Attr_constants,
        "COPT.Param": COPT_Param_constants,
        "COPT.Info": COPT_Info_constants,
        "COPT.CbInfo": COPT_CbInfo_constants,
    }


def export_copt_constants(io: IO[str], copt_constants):
    io.write('nb::module_ COPT = m.def_submodule("COPT");\n')
    for name in copt_constants["COPT"]:
        value = getattr(COPT, name)
        value = value_to_str(value)
        io.write(f'COPT.attr("{name}") = {value};\n')
    io.write("\n")

    io.write('nb::module_ Attr = COPT.def_submodule("Attr");\n')
    Attr = COPT.Attr
    for name in copt_constants["COPT.Attr"]:
        value = getattr(Attr, name)
        value = value_to_str(value)
        io.write(f'Attr.attr("{name}") = {value};\n')
    io.write("\n")

    io.write('nb::module_ Param = COPT.def_submodule("Param");\n')
    Param = COPT.Param
    for name in copt_constants["COPT.Param"]:
        value = getattr(Param, name)
        value = value_to_str(value)
        io.write(f'Param.attr("{name}") = {value};\n')
    io.write("\n")

    io.write('nb::module_ Info = COPT.def_submodule("Info");\n')
    Info = COPT.Info
    for name in copt_constants["COPT.Info"]:
        value = getattr(Info, name)
        value = value_to_str(value)
        io.write(f'Info.attr("{name}") = {value};\n')
    io.write("\n")

    io.write('nb::module_ CbInfo = COPT.def_submodule("CbInfo");\n')
    CbInfo = COPT.CbInfo
    for name in copt_constants["COPT.CbInfo"]:
        value = getattr(CbInfo, name)
        value = value_to_str(value)
        io.write(f'CbInfo.attr("{name}") = {value};\n')


if __name__ == "__main__":
    with open("gurobi_constants.txt", "w") as io:
        gurobi_constants = extract_gurobi_constants()
        export_gurobi_constants(io, gurobi_constants)
        print("Done!")

    with open("copt_constants.txt", "w") as io:
        copt_constants = extract_copt_constants()
        export_copt_constants(io, copt_constants)
        print("Done!")
