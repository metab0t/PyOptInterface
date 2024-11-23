import ctypes
import platform
import os

import tccbox

from .tcc_interface_ext import load_library, TCCInstance

system = platform.system()

libtcc_dir = tccbox.tcc_lib_dir()
# windows: libtcc.dll
# linux: libtcc.so
# macos: libtcc.dylib
sharedlib_suffix = {
    "Windows": "dll",
    "Linux": "so",
    "Darwin": "dylib",
}[system]
libtcc_path = os.path.join(libtcc_dir, f"libtcc.{sharedlib_suffix}")

ret = load_library(libtcc_path)
if not ret:
    raise Exception("Failed to load libtcc from tccbox")

# On Linux/Mac, tcc has lib/tcc/include/ and lib/tcc/libtcc1.a which must be included in compilation
libtcc_extra_include_path = None
libtcc_extra_lib_path = None
libtcc_extra_lib_names = None
if system in ["Linux", "Darwin"]:
    libtcc_extra_include_path = os.path.join(libtcc_dir, "tcc", "include")
    libtcc_extra_lib_path = os.path.join(libtcc_dir, "tcc")
    libtcc_extra_lib_names = ["tcc1", "m"]


class TCCJITCompiler:
    def __init__(self):
        self.instances = []
        self.source_codes = []

    def create_instance(self):
        inst = TCCInstance()
        inst.init()

        # Add extra include path and library path
        if libtcc_extra_include_path:
            inst.add_include_path(libtcc_extra_include_path)
            inst.add_sysinclude_path(libtcc_extra_include_path)
        if libtcc_extra_lib_path:
            inst.add_library_path(libtcc_extra_lib_path)
        if libtcc_extra_lib_names:
            for name in libtcc_extra_lib_names:
                inst.add_library(name)

        self.instances.append(inst)

        return inst

    def compile_string(self, inst, c_code: str):
        inst.compile_string(c_code)

        self.source_codes.append(c_code)
