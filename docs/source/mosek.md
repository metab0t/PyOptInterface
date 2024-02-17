# Mosek

## Initial setup

```python
from pyoptinterface import mosek

model = mosek.Model()
```

You need to install Mosek separately from the [Mosek website](https://mosek.com). After installing the software, you need to ensure the shared library of Mosek is in the system path.

- Windows: The `MOSEK_10_1_BINDIR` environment must be set to the directory of binary files in Mosek installation directory (like `C:\Program Files\Mosek\10.1\tools\platform\win64x86\bin`). The Mosek shared library `mosek64_10_1.dll` is located in the `MOSEK_10_1_BINDIR` directory. The Mosek installer usually sets this environment variable automatically, but you can also set it manually.

- Linux: Add the Mosek shared library to the `LD_LIBRARY_PATH` environment variable. The shared library `libmosek64.so` is located in the `bin` directory of the installation directory.
```bash
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/mosek/10.1/tools/platform/linux64x86/bin
```

If you want to manage the license of Mosek manually, you can create a `mosek.Env` object and pass it to the constructor of the `mosek.Model` object, otherwise we will initialize an implicit global `mosek.Env` object automatically and use it.

```python
env = mosek.Env()

model = mosek.Model(env)
```

## The capability of `mosek.Model`

### Supported constraints

:::{list-table}
:header-rows: 1

*   - Constraint
    - Supported
*   - <project:#model.add_linear_constraint>
    - ✅
*   - <project:#model.add_quadratic_constraint>
    - ✅
*   - <project:#model.add_second_order_cone_constraint>
    - ✅
*   - <project:#model.add_sos_constraint>
    - ❌

:::

### Supported [model attribute](#pyoptinterface.ModelAttribute)

:::{list-table}
:header-rows: 1

*   - Attribute
    - Get
    - Set
*   - Name
    - ❌
    - ❌
*   - ObjectiveSense
    - ✅
    - ✅
*   - DualStatus
    - ✅
    - ❌
*   - PrimalStatus
    - ✅
    - ❌
*   - RawStatusString
    - ✅
    - ❌
*   - TerminationStatus
    - ✅
    - ❌
*   - BarrierIterations
    - ✅
    - ❌
*   - DualObjectiveValue
    - ✅
    - ❌
*   - NodeCount
    - ✅
    - ❌
*   - NumberOfThreads
    - ✅
    - ❌
*   - ObjectiveBound
    - ❌
    - ❌
*   - ObjectiveValue
    - ✅
    - ❌
*   - RelativeGap
    - ✅
    - ✅
*   - Silent
    - ✅
    - ✅
*   - SimplexIterations
    - ✅
    - ❌
*   - SolverName
    - ✅
    - ❌
*   - SolverVersion
    - ✅
    - ❌
*   - SolveTimeSec
    - ✅
    - ❌
*   - TimeLimitSec
    - ✅
    - ✅

:::

### Supported [variable attribute](#pyoptinterface.VariableAttribute)

:::{list-table}
:header-rows: 1

*   - Attribute
    - Get
    - Set
*   - Name
    - ✅
    - ✅
*   - LowerBound
    - ✅
    - ✅
*   - UpperBound
    - ✅
    - ✅
*   - Domain
    - ✅
    - ✅
*   - PrimalStart
    - ❌
    - ✅
*   - Value
    - ✅
    - ❌

:::

### Supported [constraint attribute](#pyoptinterface.ConstraintAttribute)

:::{list-table}
:header-rows: 1

*   - Attribute
    - Get
    - Set
*   - Name
    - ✅
    - ✅
*   - Primal
    - ✅
    - ❌
*   - Dual
    - ✅
    - ❌

:::


## Solver-specific operations

### Parameter

For [solver-specific parameters](https://docs.mosek.com/latest/capi/param-groups.html), we provide `get_raw_parameter` and `set_raw_parameter` methods to get and set the parameters.

```python
model = mosek.Model()

# get the value of the parameter
value = model.get_raw_parameter("MSK_DPAR_OPTIMIZER_MAX_TIME")

# set the value of the parameter
model.set_raw_parameter("MSK_DPAR_OPTIMIZER_MAX_TIME", 10.0)
```

### Information

Mosek provides [information](https://docs.mosek.com/latest/capi/solver-infitems.html) for the model. We provide `model.get_raw_information(name: str)` method to access the value of information.
