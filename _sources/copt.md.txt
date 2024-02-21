# COPT

## Initial setup

```python
from pyoptinterface import copt

model = copt.Model()
```

You need to install COPT separately from the [COPT release page](https://github.com/COPT-Public/COPT-Release). After installing the software, you need to ensure the shared library of COPT is in the system path.

- Windows: The `COPT_HOME` environment must be set to the COPT installation directory (like `C:\Program Files\copt71`). The COPT shared library `copt.dll` is located in the `bin` directory of the installation directory. The COPT installer usually sets this environment variable automatically, but you can also set it manually.

- Linux: Add the COPT shared library to the `LD_LIBRARY_PATH` environment variable. The shared library `libcopt.so` is located in the `lib` directory of the installation directory.
```bash
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/copt71/lib
```

If you want to manage the license of COPT manually, you can create a `copt.Env` object and pass it to the constructor of the `copt.Model` object, otherwise we will initialize an implicit global `copt.Env` object automatically and use it.

```python
env = copt.Env()

model = copt.Model(env)
```

## The capability of `copt.Model`

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
    - ✅

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
    - ✅
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

For [solver-specific parameters](https://guide.coap.online/copt/en-doc/parameter.html), we provide `get_raw_parameter` and `set_raw_parameter` methods to get and set the parameters.

```python
model = copt.Model()

# get the value of the parameter
value = model.get_raw_parameter("TimeLimit")

# set the value of the parameter
model.set_raw_parameter("TimeLimit", 10.0)
```

### Attribute

COPT supports [attribute](https://guide.coap.online/copt/en-doc/attribute.html) for the model. We provide `model.get_raw_attribute(name:str)` to get the value of attribute.

### Information

COPT provides [information](https://guide.coap.online/copt/en-doc/information.html) for the model components. We provide methods to access the value of information.

- Information of variable: `model.get_variable_info(variable, name: str)`
- Information of constraint: `model.get_constraint_info(constraint, name: str)`
