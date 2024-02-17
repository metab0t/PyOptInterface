# Gurobi

## Initial setup

```python
from pyoptinterface import gurobi

model = gurobi.Model()
```

You need to install Gurobi separately from the [Gurobi website](https://www.gurobi.com/). After installing the software, you need to ensure the shared library of Gurobi is in the system path.

- Windows: The `GUROBI_HOME` environment must be set to the Gurobi installation directory (like `C:\gurobi1100\win64`). The Gurobi shared library `gurobi110.dll` is located in the `bin` directory of the installation directory. The Gurobi installer usually sets this environment variable automatically, but you can also set it manually.

- Linux: Add the Gurobi shared library to the `LD_LIBRARY_PATH` environment variable. The shared library `libgurobi110.so` is located in the `lib` directory of the installation directory.
```bash
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/gurobi1100/linux64/lib
```

If you want to manage the license of Gurobi manually, you can create a `gurobi.Env` object and pass it to the constructor of the `gurobi.Model` object, otherwise we will initialize an implicit global `gurobi.Env` object automatically and use it.

```python
env = gurobi.Env()

model = gurobi.Model(env)
```

For example, you can set the parameter of the `gurobi.Env` object to choose the licensing behavior.

```python
env = gurobi.Env(empty=True)
env.set_raw_parameter_string("ComputeServer", "myserver1:32123")
env.set_raw_parameter_string("ServerPassword", "pass")
env.start()

model = gurobi.Model(env)
```

## The capability of `gurobi.Model`

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
    - ✅
    - ✅
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

For [solver-specific parameters](https://www.gurobi.com/documentation/current/refman/parameters.html#sec:Parameters), we provide `get_raw_parameter` and `set_raw_parameter` methods to get and set the parameters.

```python
model = gurobi.Model()

# get the value of the parameter
value = model.get_raw_parameter("TimeLimit")

# set the value of the parameter
model.set_raw_parameter("TimeLimit", 10.0)
```

### Attribute

Gurobi supports a lot of [attributes](https://www.gurobi.com/documentation/current/refman/attributes.html#sec:Attributes) for the model, variable, and constraint. We provide methods to get or set the value of the attribute.

- Model attribute: `model.get_model_raw_attribute(name: str)` and `model.set_model_raw_attribute(name: str, value: Any)`
- Variable attribute: `model.get_variable_raw_attribute(variable, name: str)` and `model.set_variable_raw_attribute(variable, name: str, value: Any)`
- Constraint attribute: `model.get_constraint_raw_attribute(constraint, name: str)` and `model.set_constraint_raw_attribute(constraint, name: str, value: Any)`
