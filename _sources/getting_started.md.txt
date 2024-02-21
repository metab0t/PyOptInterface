# Getting Started

## Installation
PyOptInterface has no dependencies other than Python itself. It can be installed via pip:
```
pip install pyoptinterface
```

After installation, you can import the package via:
```python
import pyoptinterface as poi
```

PyOptInterface does not include any solvers. You need to install a solver separately. We will introduce how to set up the solvers to use with PyOptInterface.

## Setup of optimizers

The setup of optimizer aims to put the shared library of the optimizer in a loadable path so that PyOptInterface can load them correctly.

On Windows, we use different environment variables to specify the installation directory of the optimizers.

The table below shows the environment variables used by PyOptInterface on Windows:

:::{list-table}
:header-rows: 1

*   - Optimizer
    - Environment variable
    - Typical value
*   - Gurobi
    - `GUROBI_HOME`
    - `C:\gurobi1100\win64`
*   - COPT
    - `COPT_HOME`
    - `C:\Program Files\copt71`
*   - Mosek
    - `MOSEK_10_1_BINDIR`
    - `C:\Program Files\Mosek\10.1\tools\platform\win64x86\bin`
*   - HiGHS
    - `HiGHS_HOME`
    - `D:\highs`

:::

On Linux, we use the `LD_LIBRARY_PATH` environment variable to specify the shared library path. On macOS, we use the `DYLD_LIBRARY_PATH` environment variable to specify the shared library path.

For example, in order to make the shared library of HiGHS `libhighs.so` in path `/opt/highs/lib` loadable, you need to add the following line to your `.bashrc` or `.zshrc` file:

- Linux: `export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/highs/lib`
- macOS: `export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:/opt/highs/lib`

The table below shows the typical path of shared library on Linux and macOS:

:::{list-table}
:header-rows: 1

*   - Optimizer
    - Linux
    - macOS(ARM)
    - macOS(Intel)
*   - Gurobi
    - `/opt/gurobi1100/linux64/lib`
    - `/opt/gurobi1100/macos_universal2/lib`
    - `/opt/gurobi1100/macos_universal2/lib`
*   - COPT
    - `/opt/copt71/lib`
    - `/opt/copt71/lib`
    - `/opt/copt71/lib`
*   - Mosek
    - `/opt/mosek/10.1/tools/platform/linux64x86/bin`
    - `/opt/mosek/10.1/tools/platform/osxaarch64/bin`
    - `/opt/mosek/10.1/tools/platform/osx64x86/bin`
*   - HiGHS
    - `/opt/highs/lib`
    - `/opt/highs/lib`
    - `/opt/highs/lib`

:::


The detailed setup of each optimizer is as follows:
### Gurobi
Follow the `Full installation` part of the [Gurobi installation guide](https://support.gurobi.com/hc/en-us/articles/4534161999889-How-do-I-install-Gurobi-Optimizer) to install Gurobi and set your license.

You can check if Gurobi is installed correctly:

#### Windows:
Run this command in `cmd` console
```cmd
> echo %GUROBI_HOME%
C:\gurobi1100\win64
```

Or run this command in `powershell` console
```powershell
> $env:GUROBI_HOME
C:\gurobi1100\win64
```

#### Linux and macOS
Run this command in terminal and enter an interactive shell without error
```bash
gurobi_cl
```

### COPT
Download the installer from the [COPT release page](https://github.com/COPT-Public/COPT-Release) and follow the [installation guide](https://guide.coap.online/copt/en-doc/install.html).

You can check if COPT is installed correctly:

#### Windows:
Run this command in `cmd` console
```cmd
> echo %COPT_HOME%
C:\Program Files\copt71
```

Or run this command in `powershell` console
```powershell
> $env:COPT_HOME
C:\Program Files\copt71
```

#### Linux and macOS
Run this command in terminal and enter an interactive shell without error
```bash
copt_cmd
```

### Mosek
Download the installer from the [Mosek download page](https://www.mosek.com/downloads/) and follow the [installation guide](https://docs.mosek.com/latest/install/installation.html).

You can check if Mosek is installed correctly:

#### Windows:
Run this command in `cmd` console
```cmd
> echo %MOSEK_10_1_BINDIR%
C:\Program Files\Mosek\10.1\tools\platform\win64x86\bin
```

Or run this command in `powershell` console
```powershell
> $env:MOSEK_10_1_BINDIR
C:\Program Files\Mosek\10.1\tools\platform\win64x86\bin
```

#### Linux
Add the Mosek shared library to the `LD_LIBRARY_PATH` environment variable. The shared library `libmosek64.so` is located in the `10.1/tools/platform/linux64x86/bin` directory of the installation directory.

Run this command in terminal
```bash
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:/opt/mosek/10.1/tools/platform/linux64x86/bin
```

You can also add the above command to your `.bashrc` or `.zshrc` file to make it permanent.

#### macOS
Add the Mosek shared library to the `DYLD_LIBRARY_PATH` environment variable. The shared library `libmosek64.dylib` is located in the `lib` directory of the installation directory.

Run this command in terminal (ARM-based Mac)
```bash
export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:/mosek/10.1/tools/platform/osxaarch64/bin
```
or (Intel-based Mac)
```bash
export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:/mosek/10.1/tools/platform/osx64x86/bin
```

You can also add the above command to your `.bashrc` or `.zshrc` file to make it permanent.

Run this command in terminal to solve a simple problem without error
```bash
msktestlic
```

The output will end with
```
************************************
A license was checked out correctly.
************************************
```

### HiGHS
HiGHS is an open-source solver, you can install it from source following the instructions [HiGHS website](https://ergo-code.github.io/HiGHS/stable/installation/#Install-HiGHS).

We also provide pre-built HiGHS on our [GitHub release page](https://github.com/metab0t/highs_autobuild/releases), download the zip file of your platform and extract it to a directory.

The directory of HiGHS should contain the following files:
```
<HiGHS_HOME>
|-- bin
|   |-- highs.dll (Windows)
|   |-- highs.exe (Windows) or highs (Linux/macOS)
|-- include
|   |-- highs
|       |-- Highs.h
|       |-- ...
|-- lib
|   |-- highs.lib (Windows) or libhighs.so (Linux) or libhighs.dylib (macOS)
|   |-- ...
```

#### Windows:
You need to set the `HiGHS_HOME` environment to the directory of HiGHS installation directory (like `D:\highs`). The HiGHS shared library `highs.dll` is located in the `bin` directory under `HiGHS_HOME`.
Optionally, you can add the `%HiGHS_HOME%\bin` directory to the system PATH.

Run this command in `cmd` console to check
```cmd
> echo %HiGHS_HOME%
D:\highs
```

Or run this command in `powershell` console
```powershell
> $env:HiGHS_HOME
D:\highs
```

#### Linux
Add the HiGHS shared library to the `LD_LIBRARY_PATH` environment variable. The shared library `libhighs.so` is located in the `lib` directory of the installation directory.

Run this command in terminal
```bash
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/highs/lib
```

You can also add the above command to your `.bashrc` or `.zshrc` file to make it permanent.

#### macOS
Add the HiGHS shared library to the `DYLD_LIBRARY_PATH` environment variable. The shared library `libhighs.dylib` is located in the `lib` directory of the installation directory.

Run this command in terminal
```bash
export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:/opt/highs/lib
```

You can also add the above command to your `.bashrc` or `.zshrc` file to make it permanent.


## Let's build a simple model and solve it
After setting up the solvers, we can build a simple model and solve it.
As the first step, we will solve the following simple Quadratic Programming (QP) problem:

```{math}
\min \quad & x^{2}_{1} + 2x^{2}_{2} \\
\textrm{s.t.}  \quad & x_{1} + x_{2} = 1 \\
               \quad & x_{1}, x_{2} \geq 0
```

First, we need to create a model object:

```python
import pyoptinterface as poi
from pyoptinterface import gurobi
# from pyoptinterface import copt, highs, mosek (if you want to use other optimizers)

model = gurobi.Model()
```

Then, we need to add variables to the model:
```python
x1 = model.add_variable(lb=0, name="x1")
x2 = model.add_variable(lb=0, name="x2")
```
The `lb` argument specifies the lower bound of the variable. It is optional and defaults to $-\infty$.

The `name` argument is optional and can be used to specify the name of the variable.

Then, we need to add constraints to the model:
```python
con = model.add_linear_constraint(x1+x2, poi.ConstraintSense.Equal, 1, name="con")
```
`model.add_linear_constraint` adds a linear constraint to the model.
- The first argument `x1+x2` is the left-hand side of the constraint.
- The second argument is the sense of the constraint. It can be `poi.ConstraintSense.Equal`, `poi.ConstraintSense.LessEqual` or `poi.ConstraintSense.GreaterEqual`.
- The third argument is the right-hand side of the constraint. It must be a constant.
- The fourth argument is optional and can be used to specify the name of the constraint.

Finally, we need to set the objective function and solve the model:
```python
obj = x1*x1 + 2*x2*x2
model.set_objective(obj, poi.ObjectiveSense.Minimize)
```

The model can be solved via:
```python
model.optimize()
```
The Gurobi solver will be invoked to solve the model and writes the log to the console.

We can query the status of the model via:
```python
assert model.get_model_attribute(poi.ModelAttribute.TerminationStatus) == poi.TerminationStatusCode.OPTIMAL
```

The solution of the model can be queried via:
```python
print("x1 = ", model.get_value(x1))
print("x2 = ", model.get_value(x2))
print("obj = ", model.get_value(obj))
```

The whole code is as follows:
```python
import pyoptinterface as poi
from pyoptinterface import gurobi

model = gurobi.Model()
x1 = model.add_variable(lb=0, name="x1")
x2 = model.add_variable(lb=0, name="x2")

con = model.add_linear_constraint(x1+x2, poi.ConstraintSense.Equal, 1, name="con")

obj = x1*x1 + 2*x2*x2
model.set_objective(obj, poi.ObjectiveSense.Minimize)

model.optimize()
assert model.get_model_attribute(poi.ModelAttribute.TerminationStatus) == poi.TerminationStatusCode.OPTIMAL

print("x1 = ", model.get_value(x1))
print("x2 = ", model.get_value(x2))
print("obj = ", model.get_value(obj))
```
