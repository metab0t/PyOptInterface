---
file_format: mystnb
kernelspec:
  name: python3
---

# Getting Started

## Installation
PyOptInterface is available on PyPI. You can install it via pip:

```
pip install pyoptinterface
```

After installation, you can import the package in Python console:
```python
import pyoptinterface as poi
```

PyOptInterface has no dependencies other than Python itself. However, to use it with a specific optimizer, you need to install the corresponding optimizer manually. The details can be found on [the configurations of optimizers](https://metab0t.github.io/PyOptInterface/getting-started.html).

In order to provide out-of-the-box support for open source optimizers (currently we support [HiGHS](https://github.com/ERGO-Code/HiGHS)), PyOptInterface can also be installed with pre-built optimizers. You can install them via pip:

```
pip install pyoptinterface[highs]
```

It will install a full-featured binary version of HiGHS optimizer via [highsbox](http://github.com/metab0t/highsbox), which can be used with PyOptInterface.

We will introduce how to set up the optimizers to use with PyOptInterface in this page.

## Setup of optimizers

PyOptInterface uses the `Dynamic Loading` technique to load the dynamic library of optimizers at runtime, so the optimizer it uses can be changed manually without recompiling the code.

The set up of optimizers includes two approaches:
1. Automatic detection of the installation directory of the optimizers.
2. Manually specifying the path of the dynamic library of optimizer.

We will introduce the automatic detection and manual configuration in details

## Automatic detection of the installation directory of the optimizers

The automatic detection includes three steps:
1. Environment variable set by the installer of optimizer
2. The official Python binding of the optimizer (if available)
3. Search directly in the system loadable path (e.g. `/usr/lib`, `/usr/local/lib` on Linux or PATH on Windows)

For the 3rd step, we want to explain more for novices.

For example, in order to make the dynamic library of HiGHS `highs.dll`/`libhighs.so`/`libhighs.dylib` loadable, we need to put it in a loadable path recognized by the operating system. The typical loadable path on Linux is `/usr/lib`, `/usr/local/lib`, and the typical loadable path on Windows is `PATH`.

On Linux, we use the `LD_LIBRARY_PATH` environment variable to specify the dynamic library path. On macOS, we use the `DYLD_LIBRARY_PATH` environment variable to specify the dynamic library path.

If the dynamic library is in path `C:\highs\lib`/`/opt/highs/lib`:
- Windows: `set PATH=%PATH%;C:\highs\lib`
- Linux: `export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/highs/lib`
- macOS: `export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:/opt/highs/lib`

The typical paths where the dynamic library of optimizers are located are as follows:

:::{list-table}
:header-rows: 1

*   - Optimizer
    - Windows
    - Linux
    - macOS(ARM)
    - macOS(Intel)
*   - Gurobi
    - `C:\gurobi1101\win64\bin`
    - `/opt/gurobi1100/linux64/lib`
    - `/opt/gurobi1100/macos_universal2/lib`
    - `/opt/gurobi1100/macos_universal2/lib`
*   - COPT
    - `C:\Program Files\copt71\bin`
    - `/opt/copt71/lib`
    - `/opt/copt71/lib`
    - `/opt/copt71/lib`
*   - Mosek
    - `C:\Program Files\Mosek\10.2\tools\platform\win64x86\bin`
    - `/opt/mosek/10.2/tools/platform/linux64x86/bin`
    - `/opt/mosek/10.2/tools/platform/osxaarch64/bin`
    - `/opt/mosek/10.2/tools/platform/osx64x86/bin`
*   - HiGHS
    - `D:\highs\bin`
    - `/opt/highs/lib`
    - `/opt/highs/lib`
    - `/opt/highs/lib`

:::

### Gurobi

The currently supported version is **11.0.x**. Other versions may work but are not tested.

For Gurobi, the automatic detection looks for the following things in order:
1. The environment variable `GUROBI_HOME` set by the installer of Gurobi
2. The installation of `gurobipy`
3. `gurobi110.dll`/`libgurobi110.so`/`libgurobi110.dylib` in the system loadable path

### COPT

The currently supported version is **7.1.4** and newer. Other versions may work but are not tested.

For COPT, the automatic detection looks for the following things in order:
1. The environment variable `COPT_HOME` set by the installer of COPT
2. `copt.dll`/`libcopt.so`/`libcopt.dylib` in the system loadable path

### Mosek

The currently supported version is **10.2.x**. Other versions may work but are not tested.

For Mosek, the automatic detection looks for the following things in order:
1. The environment variable `MOSEK_10_2_BINDIR` set by the installer of Mosek
2. The installation of `mosek` PyPI package
3. `mosek64_10_2.dll`/`libmosek64.so`/`libmosek64.dylib` in the system loadable path

### HiGHS

The currently supported version is **1.7.x**. Other versions may work but are not tested.

For HiGHS, the automatic detection looks for the following things in order:
1. The environment variable `HiGHS_HOME` set by the user
2. The installation of `highsbox` PyPI package (recommended)
3. `highs.dll`/`libhighs.so`/`libhighs.dylib` in the system

For HiGHS, we recommend installing the `highsbox` PyPI package, which provides a full-featured binary version of HiGHS optimizer for you.

## Manually specifying the path of the dynamic library of optimizer

If the automatic detection fails or you want to use the optimizer in a customized location, you can manually specify the path of the dynamic library of the optimizer.

We take HiGHS as an example. Whether the optimizer has been successfully loaded can be checked via the following code:

```python
from pyoptinterface import highs

print(highs.is_library_loaded())
```

If the optimizer has not been successfully loaded, you can manually specify the path of the dynamic library of the optimizer via the following code:

```python
ret = highs.load_library("path/to/libhighs.so")

print(f"Loading from custom path manually: {ret}")
```

The `load_library` function returns `True` if the library is successfully loaded, otherwise it returns `False`.

If you want to revert to use the automatic detection, you can call the `autoload_library` function:

```python
ret = highs.autoload_library()

print(f"Loading from automatically detected location: {ret}")
```

For other optimizers, just replace `highs` with the corresponding optimizer name like `gurobi`, `copt`, `mosek`.

The typical paths where the dynamic library of optimizers are located are as follows:

:::{list-table}
:header-rows: 1

*   - Optimizer
    - Windows
    - Linux
    - macOS(ARM)
    - macOS(Intel)
*   - Gurobi
    - `C:\gurobi1101\win64\bin\gurobi110.dll`
    - `/opt/gurobi1100/linux64/lib/libgurobi110.so`
    - `/opt/gurobi1100/macos_universal2/lib/libgurobi110.dylib`
    - `/opt/gurobi1100/macos_universal2/lib/libgurobi110.dylib`
*   - COPT
    - `C:\Program Files\copt71\bin\copt.dll`
    - `/opt/copt71/lib/libcopt.so`
    - `/opt/copt71/lib/libcopt.dylib`
    - `/opt/copt71/lib/libcopt.dylib`
*   - Mosek
    - `C:\Program Files\Mosek\10.2\tools\platform\win64x86\bin\mosek64_10_1.dll`
    - `/opt/mosek/10.2/tools/platform/linux64x86/bin/libmosek64.so`
    - `/opt/mosek/10.2/tools/platform/osxaarch64/bin/libmosek64.dylib`
    - `/opt/mosek/10.2/tools/platform/osx64x86/bin/libmosek64.dylib`
*   - HiGHS
    - `D:\highs\bin\highs.dll`
    - `/opt/highs/lib/libhighs.so`
    - `/opt/highs/lib/libhighs.dylib`
    - `/opt/highs/lib/libhighs.dylib`

:::

## Let's build a simple model and solve it
After setting up the optimizers, we can build a simple model and solve it.
As the first step, we will solve the following simple Quadratic Programming (QP) problem:

```{math}
\min \quad & x^{2}_{1} + 2x^{2}_{2} \\
\textrm{s.t.}  \quad & x_{1} + x_{2} = 1 \\
               \quad & x_{1}, x_{2} \geq 0
```

First, we need to create a model object:

```{code-cell}
import pyoptinterface as poi
from pyoptinterface import highs
# from pyoptinterface import copt, gurobi, mosek (if you want to use other optimizers)

model = highs.Model()
```

Then, we need to add variables to the model:
```{code-cell}
x1 = model.add_variable(lb=0, name="x1")
x2 = model.add_variable(lb=0, name="x2")
```
The `lb` argument specifies the lower bound of the variable. It is optional and defaults to $-\infty$.

The `name` argument is optional and can be used to specify the name of the variable.

Then, we need to add constraints to the model:
```{code-cell}
con = model.add_linear_constraint(x1+x2, poi.ConstraintSense.Equal, 1, name="con")
```
`model.add_linear_constraint` adds a linear constraint to the model.
- The first argument `x1+x2` is the left-hand side of the constraint.
- The second argument is the sense of the constraint. It can be `poi.ConstraintSense.Equal`, `poi.ConstraintSense.LessEqual` or `poi.ConstraintSense.GreaterEqual`.
- The third argument is the right-hand side of the constraint. It must be a constant.
- The fourth argument is optional and can be used to specify the name of the constraint.

Finally, we need to set the objective function and solve the model:
```{code-cell}
obj = x1*x1 + 2*x2*x2
model.set_objective(obj, poi.ObjectiveSense.Minimize)
```

The model can be solved via:
```{code-cell}
model.optimize()
```
The HiGHS optimizer will be invoked to solve the model and writes the log to the console.

We can query the status of the model via:
```{code-cell}
model.get_model_attribute(poi.ModelAttribute.TerminationStatus)
```

The solution of the model can be queried via:
```{code-cell}
print("x1 = ", model.get_value(x1))
print("x2 = ", model.get_value(x2))
print("obj = ", model.get_value(obj))
```
