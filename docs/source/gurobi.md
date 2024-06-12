# Gurobi

## Initial setup

```python
from pyoptinterface import gurobi

model = gurobi.Model()
```

You need to follow the instructions in [Getting Started](getting_started.md#gurobi) to set up the optimizer correctly.

If you want to manage the license of Gurobi manually, you can create a `gurobi.Env` object and pass it to the constructor of the `gurobi.Model` object, otherwise we will initialize an implicit global `gurobi.Env` object automatically and use it.

```python
env = gurobi.Env()

model = gurobi.Model(env)
```

For example, you can set the parameter of the `gurobi.Env` object to choose the licensing behavior.

```python
env = gurobi.Env(empty=True)
env.set_raw_parameter("ComputeServer", "myserver1:32123")
env.set_raw_parameter("ServerPassword", "pass")
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

```{include} attribute/gurobi.md
```


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

We also provide `gurobi.GRB` to contain all the constants in `gurobipy.GRB`.

For model status:
```python
status = model.get_model_raw_attribute(gurobi.GRB.Attr.Status)

if status == gurobi.GRB.OPTIMAL:
    ...
elif status == gurobi.GRB.INFEASIBLE:
    ...
```

For reduced cost of a variable:
```python
rc = model.get_variable_raw_attribute(variable, gurobi.GRB.Attr.RC)
```

For right-hand side value of a constraint:
```python
rhs = model.get_constraint_raw_attribute(constraint, gurobi.GRB.Attr.RHS)
```
