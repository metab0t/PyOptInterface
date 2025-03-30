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
libtcc_extra_include_paths = []
libtcc_extra_lib_paths = []
libtcc_extra_lib_names = []
if system in ["Linux", "Darwin"]:
    libtcc_extra_include_paths.append(os.path.join(libtcc_dir, "tcc", "include"))
    libtcc_extra_lib_paths.append(os.path.join(libtcc_dir, "tcc"))
    libtcc_extra_lib_names.append("m")
if system == "Linux":
    libtcc_extra_include_paths.extend(
        [
            "/usr/include",
            "/usr/local/include",
            "/usr/include/x86_64-linux-gnu",
            # arm
            "/usr/include/aarch64-linux-gnu",
        ]
    )
    libtcc_extra_lib_paths.extend(
        [
            "/usr/lib",
            "/usr/local/lib",
            "/usr/lib/x86_64-linux-gnu",
            # arm
            "/usr/lib/aarch64-linux-gnu",
        ]
    )


class TCCJITCompiler:
    def __init__(self):
        self.instances = []
        self.source_codes = []

    def create_instance(self):
        inst = TCCInstance()
        inst.init()

        # Add extra include path and library path
        for path in libtcc_extra_include_paths:
            inst.add_include_path(path)
            inst.add_sysinclude_path(path)
        for path in libtcc_extra_lib_paths:
            inst.add_library_path(path)
        for name in libtcc_extra_lib_names:
            inst.add_library(name)

        self.instances.append(inst)

        return inst

    def compile_string(self, inst, c_code: str):
        inst.compile_string(c_code)

        self.source_codes.append(c_code)
