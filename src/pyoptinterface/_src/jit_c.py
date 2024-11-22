import ctypes
import platform
import os

import tccbox

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

# On Linux/Mac, tcc has lib/tcc/include/ and lib/tcc/libtcc1.a which must be included in compilation
libtcc_extra_include_path = None
libtcc_extra_lib_path = None
libtcc_extra_lib_name = None
if system in ["Linux", "Darwin"]:
    libtcc_extra_include_path = os.path.join(libtcc_dir, "tcc", "include")
    libtcc_extra_lib_path = os.path.join(libtcc_dir, "tcc")
    libtcc_extra_lib_name = "libtcc1.a"

# Define types
TCCState = ctypes.c_void_p
TCCReallocFunc = ctypes.CFUNCTYPE(ctypes.c_void_p, ctypes.c_void_p, ctypes.c_ulong)
TCCErrorFunc = ctypes.CFUNCTYPE(None, ctypes.c_void_p, ctypes.c_char_p)

# Define the output type constant for in-memory execution
TCC_OUTPUT_MEMORY = ctypes.c_int(1)


class TCCJITCompiler:
    def __init__(self, libtcc_path=libtcc_path):
        # Load the libtcc shared library
        self.libtcc = ctypes.CDLL(libtcc_path)

        # Initialize libtcc function prototypes
        self._initialize_function_prototypes()

        # store all TCC states
        self.states = []

        self.source_codes = []

    def create_state(self):
        # Create a new TCC state
        state = self.libtcc.tcc_new()

        # Ensure the state was successfully created
        if not state:
            raise Exception("Failed to create TCC state")

        # Set the output type to memory
        if self.libtcc.tcc_set_output_type(state, TCC_OUTPUT_MEMORY) == -1:
            raise Exception("Failed to set output type")

        # Add extra include path and library path
        if libtcc_extra_include_path:
            if (
                self.libtcc.tcc_add_include_path(
                    state, libtcc_extra_include_path.encode()
                )
                == -1
            ):
                raise Exception("Failed to add extra include path")
            if (
                self.libtcc.tcc_add_sysinclude_path(
                    state, libtcc_extra_include_path.encode()
                )
                == -1
            ):
                raise Exception("Failed to add extra sysinclude path")
        if libtcc_extra_lib_path:
            if (
                self.libtcc.tcc_add_library_path(state, libtcc_extra_lib_path.encode())
                == -1
            ):
                raise Exception("Failed to add extra library path")
        if libtcc_extra_lib_name:
            if self.libtcc.tcc_add_library(state, libtcc_extra_lib_name.encode()) == -1:
                raise Exception("Failed to add extra library")

        self.states.append(state)

        return state

    def _initialize_function_prototypes(self):
        libtcc = self.libtcc
        libtcc.tcc_new.restype = TCCState
        libtcc.tcc_delete.argtypes = [TCCState]
        libtcc.tcc_set_output_type.argtypes = [TCCState, ctypes.c_int]
        libtcc.tcc_set_output_type.restype = ctypes.c_int
        libtcc.tcc_compile_string.argtypes = [TCCState, ctypes.c_char_p]
        libtcc.tcc_relocate.argtypes = [TCCState]
        libtcc.tcc_add_symbol.argtypes = [TCCState, ctypes.c_char_p, ctypes.c_void_p]
        libtcc.tcc_add_symbol.restype = ctypes.c_int
        libtcc.tcc_get_symbol.argtypes = [TCCState, ctypes.c_char_p]
        libtcc.tcc_get_symbol.restype = ctypes.c_void_p
        libtcc.tcc_add_include_path.argtypes = [TCCState, ctypes.c_char_p]
        libtcc.tcc_add_include_path.restype = ctypes.c_int
        libtcc.tcc_add_sysinclude_path.argtypes = [TCCState, ctypes.c_char_p]
        libtcc.tcc_add_sysinclude_path.restype = ctypes.c_int
        libtcc.tcc_add_library_path.argtypes = [TCCState, ctypes.c_char_p]
        libtcc.tcc_add_library_path.restype = ctypes.c_int
        libtcc.tcc_add_library.argtypes = [TCCState, ctypes.c_char_p]
        libtcc.tcc_add_library.restype = ctypes.c_int

    def compile_string(self, state, c_code: str):
        if self.libtcc.tcc_compile_string(state, c_code.encode()) == -1:
            raise Exception("Failed to compile code")

        if self.libtcc.tcc_relocate(state) == -1:
            raise Exception("Failed to relocate")

        self.source_codes.append(c_code)

    def add_symbol(self, state, symbol_name: str, symbol_address):
        # Add a symbol to the TCC state
        if (
            self.libtcc.tcc_add_symbol(state, symbol_name.encode(), symbol_address)
            == -1
        ):
            raise Exception(f"Failed to add symbol {symbol_name} to TCC state")

    def get_symbol(self, state, symbol_name: str):
        # Get the symbol for the compiled function
        symbol = self.libtcc.tcc_get_symbol(state, symbol_name.encode())
        if not symbol:
            raise Exception(f"Symbol {symbol_name} not found")
        return symbol

    def __del__(self):
        for state in self.states:
            self.libtcc.tcc_delete(state)
