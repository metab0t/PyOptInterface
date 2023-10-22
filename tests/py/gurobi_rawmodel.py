import pyoptinterface_nb as core
from pyoptinterface_nb import gurobi
import ctypes

Model = gurobi.Model


def main():
    model = Model()

    model_ptr = model.c_pointer

    libgurobi = ctypes.CDLL("gurobi100.dll")

    libgurobi.GRBgetintattr.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.POINTER(ctypes.c_int)]
    libgurobi.GRBgetintattr.restype = ctypes.c_int

    status = ctypes.c_int()
    libgurobi.GRBgetintattr(model_ptr, b"Status", ctypes.byref(status))

    print(status.value)

if __name__ == "__main__":
    main()