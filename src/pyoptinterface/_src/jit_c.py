import ctypes
import platform
import os

import tccbox

libtcc_dir = tccbox.tcc_lib_dir()
# windows: libtcc.dll
# linux: libtcc.so
# macos: libtcc.dylib
sharedlib_suffix = {
    "Windows": "dll",
    "Linux": "so",
    "Darwin": "dylib",
}[platform.system()]
libtcc_path = os.path.join(libtcc_dir, f"libtcc.{sharedlib_suffix}")

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

        # Create a new TCC state
        self.state = self.libtcc.tcc_new()

        # Ensure the state was successfully created
        if not self.state:
            raise Exception("Failed to create TCC state")

        # Set the output type to memory
        if self.libtcc.tcc_set_output_type(self.state, TCC_OUTPUT_MEMORY) == -1:
            self.cleanup()
            raise Exception("Failed to set output type")

        # relocate has been called
        self.relocated = False

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

    def compile_string(self, c_code):
        # Compile C code string
        if self.libtcc.tcc_compile_string(self.state, c_code) == -1:
            raise Exception("Failed to compile code")

        self.relocated = False

    def add_symbol(self, symbol_name, symbol_address):
        # Add a symbol to the TCC state
        if self.libtcc.tcc_add_symbol(self.state, symbol_name, symbol_address) == -1:
            raise Exception(f"Failed to add symbol {symbol_name} to TCC state")

        self.relocated = False

    def get_symbol(self, symbol_name):
        if not self.relocated:
            if self.libtcc.tcc_relocate(self.state) == -1:
                raise Exception("Failed to relocate")
            self.relocated = True
        # Get the symbol for the compiled function
        symbol = self.libtcc.tcc_get_symbol(self.state, symbol_name)
        if not symbol:
            raise Exception(f"Symbol {symbol_name.decode()} not found")
        return symbol

    def cleanup(self):
        # Clean up the TCC state
        if self.state:
            self.libtcc.tcc_delete(self.state)
            self.state = None

    def __del__(self):
        # Ensure clean up is called when the instance is destroyed
        self.cleanup()
