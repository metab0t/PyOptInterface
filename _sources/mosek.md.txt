# Mosek

## Initial setup

```python
from pyoptinterface import mosek

model = mosek.Model()
```

You need to follow the instructions in [Getting Started](getting_started.md#mosek) to set up the optimizer correctly.

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
*   - <project:#model.add_exp_cone_constraint>
    - ✅
*   - <project:#model.add_sos_constraint>
    - ❌

:::

```{include} attribute/mosek.md
```

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
