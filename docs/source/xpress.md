# Xpress

## Initial setup

```python
from pyoptinterface import xpress
model = xpress.Model()
```

You need to follow the instructions in [Getting Started](getting_started.md#xpress) to set up the optimizer correctly.

If you want to manage the license of Xpress manually, you can create a `xpress.Env` object and pass it to the constructor of the `xpress.Model` object, otherwise we will initialize an implicit global `xpress.Env` object automatically and use it.

```python
env = xpress.Env()
model = xpress.Model(env)
```

For users who want to release the license immediately after the optimization, you can call the `close` method of all models created and the `xpress.Env` object.

```python
env = xpress.Env()
model = xpress.Model(env)
# do something with the model
model.close()
env.close()
```

## The capability of `xpress.Model`

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
    - ✅
*   - <project:#model.add_sos_constraint>
    - ✅
*   - <project:#model.add_nl_constraint>
    - ✅
:::

```{include} attribute/xpress.md
```

## Solver-specific operations

### Controls and Attributes

Xpress uses different terminology than PyOptInterface:
- **Controls** govern the solution procedure and output format (similar to PyOptInterface parameters)
- **Attributes** are read-only properties of the problem and solution

PyOptInterface maps these as follows:
- PyOptInterface **parameters** correspond to Xpress controls
- PyOptInterface **attributes** may access Xpress controls, attributes, or variable/constraint properties through dedicated methods

### Controls (Parameters)

For [solver-specific controls](https://www.fico.com/fico-xpress-optimization/docs/latest/solver/optimizer/HTML/chapter7.html), we provide `get_raw_control` and `set_raw_control` methods.

```python
model = xpress.Model()
# Get the value of a control
value = model.get_raw_control("XPRS_TIMELIMIT")
# Set the value of a control
model.set_raw_control("XPRS_TIMELIMIT", 10.0)
```

### Attributes

For [problem attributes](https://www.fico.com/fico-xpress-optimization/docs/latest/solver/optimizer/HTML/chapter8.html), we provide `get_raw_attribute` method.

```python
# Get number of columns in the problem
cols = model.get_raw_attribute("XPRS_COLS")
```

We also provide `xpress.XPRS` to contain common constants from the Xpress C API.

```python
# Using constants
value = model.get_raw_control_dbl_by_id(xpress.XPRS.TIMELIMIT)
```

### Variable and Constraint Properties

Common variable and constraint properties are provided through PyOptInterface dedicated methods:

**Variable methods:**
- **Bounds**: `set_variable_lowerbound`, `get_variable_lowerbound`, `set_variable_upperbound`, `get_variable_upperbound`, `set_variable_bounds`
- **Objective**: `set_objective_coefficient`, `get_objective_coefficient`
- **Type and name**: `set_variable_type`, `get_variable_type`, `set_variable_name`, `get_variable_name`
- **Solution values**: `get_variable_value`, `get_variable_rc`, `get_variable_primal_ray`
- **Basis status**: `is_variable_basic`, `is_variable_nonbasic_lb`, `is_variable_nonbasic_ub`, `is_variable_superbasic`
- **IIS information**: `is_variable_lowerbound_IIS`, `is_variable_upperbound_IIS`

**Constraint methods:**
- **Definition**: `set_constraint_sense`, `get_constraint_sense`, `set_constraint_rhs`, `get_constraint_rhs`, `set_constraint_name`, `get_constraint_name`
- **Coefficients**: `set_normalized_coefficient`, `get_normalized_coefficient`, `set_normalized_rhs`, `get_normalized_rhs`
- **Solution values**: `get_constraint_dual`, `get_constraint_slack`, `get_constraint_dual_ray`
- **Basis status**: `is_constraint_basic`, `is_constraint_nonbasic_lb`, `is_constraint_nonbasic_ub`, `is_constraint_superbasic`
- **IIS information**: `is_constraint_in_IIS`

**Usage examples:**

Variable properties:
```python
# Bounds
model.set_variable_lowerbound(variable, 0.0)
lb = model.get_variable_lowerbound(variable)
model.set_variable_upperbound(variable, 10.0)
ub = model.get_variable_upperbound(variable)

# Objective coefficient
model.set_objective_coefficient(variable, 2.0)
coef = model.get_objective_coefficient(variable)

# Type and name
model.set_variable_type(variable, VariableDomain.Integer)
vtype = model.get_variable_type(variable)
model.set_variable_name(variable, "x")
name = model.get_variable_name(variable)

# Solution values
value = model.get_variable_value(variable)
rc = model.get_variable_rc(variable)
ray = model.get_variable_primal_ray(variable)

# Basis status
if model.is_variable_basic(variable):
    ...
```

Constraint properties:
```python
# Sense and RHS
model.set_constraint_sense(constraint, ConstraintSense.LessEqual)
sense = model.get_constraint_sense(constraint)
model.set_constraint_rhs(constraint, 5.0)
rhs = model.get_constraint_rhs(constraint)

# Name
model.set_constraint_name(constraint, "c1")
name = model.get_constraint_name(constraint)

# Solution values
dual = model.get_constraint_dual(constraint)
slack = model.get_constraint_slack(constraint)
ray = model.get_constraint_dual_ray(constraint)

# Basis status
if model.is_constraint_basic(constraint):
    ...
```
