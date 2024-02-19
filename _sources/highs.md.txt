# HiGHS

## Initial setup

```python
from pyoptinterface import highs

model = highs.Model()
```

You need to install HiGHS separately from the [HiGHS website](https://ergo-code.github.io/HiGHS/stable/installation/#Install-HiGHS). The easiest way to install HiGHS is our [GitHub release page](https://github.com/metab0t/highs_autobuild/releases), download the zip file of your platform and extract it to a directory.

After installing the software, you need to ensure the shared library of HiGHS is in the system path.

- Windows: The `HiGHS_HOME` environment must be set to the directory of binary files in HiGHS installation directory (like `D:\highs`). The HiGHS shared library `highs.dll` is located in the `bin` directory under `HiGHS_HOME`.
- Linux: Add the HiGHS shared library to the `LD_LIBRARY_PATH` environment variable. The shared library `libhighs.so` is located in the `lib` directory of the installation directory.
```bash
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/highs/lib
```

## The capability of `highs.Model`

### Supported constraints

:::{list-table}
:header-rows: 1

*   - Constraint
    - Supported
*   - <project:#model.add_linear_constraint>
    - ✅
*   - <project:#model.add_quadratic_constraint>
    - ❌
*   - <project:#model.add_second_order_cone_constraint>
    - ❌
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

For [solver-specific parameters](https://ergo-code.github.io/HiGHS/stable/options/definitions/), we provide `get_raw_parameter` and `set_raw_parameter` methods to get and set the parameters.

```python
model = highs.Model()

# get the value of the parameter
value = model.get_raw_parameter("time_limit")

# set the value of the parameter
model.set_raw_parameter("time_limit", 10.0)
```

### Information

HiGHS provides [information](https://ergo-code.github.io/HiGHS/stable/structures/classes/HighsInfo/) for the model. We provide `model.get_raw_information(name: str)` method to access the value of information.
