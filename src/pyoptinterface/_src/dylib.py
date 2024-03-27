import platform


def dylib_suffix():
    system = platform.system()
    if system == "Linux":
        return "so"
    elif system == "Darwin":
        return "dylib"
    elif system == "Windows":
        return "dll"
    else:
        raise RuntimeError("Unknown platform: %s" % system)
