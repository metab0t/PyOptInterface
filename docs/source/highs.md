# HiGHS

## Initial setup

```python
from pyoptinterface import highs

model = highs.Model()
```

You need to follow the instructions in [Getting Started](getting_started.md#highs) to set up the optimizer correctly.

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

```{include} attribute/highs.md
```


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
