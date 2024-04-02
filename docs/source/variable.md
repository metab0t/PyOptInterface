# Variable

Variable represents a decision variable in the optimization problem. It can be created by calling the [`add_variable`](#model.add_variable) method of the model:

```python
import pyoptinterface as poi
from pyoptinterface import highs

model = highs.Model()

x = model.add_variable(lb=0, ub=1.0, domain=poi.VariableDomain.Continuous, name="x")
```

## Variable Attributes
After a variable is created, we can query or modify its attributes. The following table lists the standard [variable attributes](#pyoptinterface.VariableAttribute):

:::{list-table} **Standard [variable attributes](#pyoptinterface.VariableAttribute)**
:header-rows: 1
:widths: 20 20

*   - Attribute name
    - Type
*   - Name
    - str
*   - LowerBound
    - float
*   - UpperBound
    - float
*   - Domain
    - [VariableDomain](project:#pyoptinterface.VariableDomain)
*   - PrimalStart
    - float
*   - Value
    - float
:::

```python
# set the lower bound of the variable
model.set_variable_attribute(x, poi.VariableAttribute.LowerBound, 0.0)
# set the upper bound of the variable
model.set_variable_attribute(x, poi.VariableAttribute.UpperBound, 1.0)

# For mixed-integer programming, we can set the initial value of the variable
model.set_variable_attribute(x, poi.VariableAttribute.PrimalStart, 0.5)

# get the value of the variable after optimization
x_value = model.get_variable_attribute(x, poi.VariableAttribute.Value)
```

## Delete variable
We can delete a variable by calling the [`delete_variable`](#model.delete_variable) method of the model:

```python
model.delete_variable(x)
```

After a variable is deleted, it cannot be used in the model anymore, otherwise an exception will be raised.

We can query whether a variable is active by calling the [`is_variable_active`](#model.is_variable_active) method of the model:

```python
is_active = model.is_variable_active(x)
```
