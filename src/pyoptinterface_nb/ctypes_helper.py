import ctypes


def pycapsule_to_cvoidp(capsule):
    ctypes.pythonapi.PyCapsule_GetName.restype = ctypes.c_char_p
    ctypes.pythonapi.PyCapsule_GetName.argtypes = [ctypes.py_object]
    name = ctypes.pythonapi.PyCapsule_GetName(capsule)

    ctypes.pythonapi.PyCapsule_GetPointer.restype = ctypes.c_void_p
    ctypes.pythonapi.PyCapsule_GetPointer.argtypes = [ctypes.py_object, ctypes.c_char_p]
    addr = ctypes.pythonapi.PyCapsule_GetPointer(capsule, name)

    addr = ctypes.c_void_p(addr)
    return addr
