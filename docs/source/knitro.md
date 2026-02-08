# KNITRO

## Initial setup

```python
from pyoptinterface import knitro

model = knitro.Model()
```

You need to follow the instructions in [Getting Started](getting_started.md#knitro) to set up the optimizer correctly.

## The capability of `knitro.Model`

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
*   - <project:#model.add_exp_cone_constraint>
    - ❌
*   - <project:#model.add_sos_constraint>
    - ❌
*   - <project:#model.add_nl_constraint>
    - ✅
:::

```{include} attribute/knitro.md
```

## Solver-specific operations

### Parameters

For [solver-specific parameters](https://www.artelys.com/app/docs/knitro/2_userGuide/knitroOptions.html), we provide `get_raw_parameter` and `set_raw_parameter` methods to get and set the parameters.

```python
model = knitro.Model()

# Set the value of a parameter by name
model.set_raw_parameter("algorithm", 1)
model.set_raw_parameter("feastol", 1e-8)
model.set_raw_parameter("opttol", 1e-8)

# Set the value of a parameter by ID (using knitro.KN constants)
model.set_raw_parameter(knitro.KN.PARAM_MAXIT, 1000)
```

We also provide `knitro.KN` to contain common constants from the KNITRO C API.

```python
# Using constants for parameter IDs
model.set_raw_parameter(knitro.KN.PARAM_FEASTOL, 1e-6)

# Algorithm selection
model.set_raw_parameter(knitro.KN.PARAM_NLP_ALGORITHM, knitro.KN.NLP_ALG_BAR_DIRECT)

# Output level
model.set_raw_parameter(knitro.KN.PARAM_OUTLEV, knitro.KN.OUTLEV_ITER)

# MIP parameters
model.set_raw_parameter(knitro.KN.PARAM_MIP_METHOD, knitro.KN.MIP_METHOD_BB)
model.set_raw_parameter(knitro.KN.PARAM_MIP_OPTGAPREL, 1e-4)
```

### Variable and Constraint Properties

Common variable and constraint properties are provided through PyOptInterface dedicated methods:

**Variable methods:**
- **Bounds**: `set_variable_lb`, `get_variable_lb`, `set_variable_ub`, `get_variable_ub`
- **Type and name**: `set_variable_name`, `get_variable_name`, `set_variable_domain`
- **Starting point**: `set_variable_start`
- **Solution values**: `get_value`, `get_variable_rc`

**Constraint methods:**
- **Name**: `set_constraint_name`, `get_constraint_name`
- **Solution values**: `get_constraint_primal`, `get_constraint_dual`

**Usage examples:**

Variable properties:
```python
# Bounds
model.set_variable_lb(variable, 0.0)
lb = model.get_variable_lb(variable)
model.set_variable_ub(variable, 10.0)
ub = model.get_variable_ub(variable)

# Type and name
model.set_variable_name(variable, "x")
name = model.get_variable_name(variable)

# Starting point
model.set_variable_start(variable, 1.0)

# Solution values
value = model.get_value(variable)
rc = model.get_variable_rc(variable)
```

Constraint properties:
```python
# Name
model.set_constraint_name(constraint, "c1")
name = model.get_constraint_name(constraint)

# Solution values
primal = model.get_constraint_primal(constraint)
dual = model.get_constraint_dual(constraint)
```
