# Common Model Interface

Generally speaking, the following APIs are common to all optimizers except for adding constraint
because different optimizers may support different types of constraints.

## Model

### Get/set model attributes

```{py:function} model.set_model_attribute(attr, value)

set the value of a model attribute

:param pyoptinterface.ModelAttribute attr: the attribute to set
:param value: the value to set
```

```{py:function} model.get_model_attribute(attr)

get the value of a model attribute

:param pyoptinterface.ModelAttribute attr: the attribute to get
:return: the value of the attribute
```

## Variable

### Add a variable to the model

```{py:function} model.add_variable([lb=-inf, ub=+inf, domain=pyoptinterface.VariableDomain.Continuous, name=""])

add a variable to the model

:param float lb: the lower bound of the variable, optional, defaults to $-\infty$
:param float ub: the upper bound of the variable, optional, defaults to $+\infty$
:param pyoptinterface.VariableDomain domain: the domain of the variable, optional, defaults to 
continuous
:param str name: the name of the variable, optional
:return: the handle of the variable
```

### Add multi-dimensional variables to the model as <project:#pyoptinterface.tupledict>

```{py:function} model.add_variables(*coords, [lb=-inf, ub=+inf, domain=pyoptinterface.VariableDomain.Continuous, name=""])

add a multi-dimensional variable to the model

:param coords: the coordinates of the variable, can be a list of Iterables
:param float lb: the lower bound of the variable, optional, defaults to $-\infty$
:param float ub: the upper bound of the variable, optional, defaults to $+\infty$
:param pyoptinterface.VariableDomain domain: the domain of the variable, optional, defaults to 
continuous
:param str name: the name of the variable, optional
:return: the multi-dimensional variable
:rtype: pyoptinterface.tupledict
```

### Get/set variable attributes

```{py:function} model.set_variable_attribute(var, attr, value)

set the value of a variable attribute

:param var: the handle of the variable
:param pyoptinterface.VariableAttribute attr: the attribute to set
:param value: the value to set
```

```{py:function} model.get_variable_attribute(var, attr)

get the value of a variable attribute

:param var: the handle of the variable
:param pyoptinterface.VariableAttribute attr: the attribute to get
:return: the value of the attribute
```

### Delete variable

```{py:function} model.delete_variable(var)

delete a variable from the model

:param var: the handle of the variable
```

```{py:function} model.is_variable_active(var)

query whether a variable is active

:param var: the handle of the variable
:return: whether the variable is active
:rtype: bool
```

## Expression

### Get the value of an expression (including variable)

```{py:function} model.get_value(expr_or_var)

get the value of an expression or a variable after optimization

:param expr_or_var: the handle of the expression or the variable
:return: the value of the expression or the variable
:rtype: float
```

### Pretty print expression (including variable)

```{py:function} model.pprint(expr_or_var)

pretty print an expression in a human-readable format

:param expr_or_var: the handle of the expression or the variable
:return: the human-readable format of the expression
:rtype: str
```

## Constraint

### Add a constraint to the model

- <project:#model.add_linear_constraint>
- <project:#model.add_quadratic_constraint>
- <project:#model.add_second_order_cone_constraint>
- <project:#model.add_sos_constraint>


### Get/set constraint attributes

```{py:function} model.set_constraint_attribute(con, attr, value)

set the value of a constraint attribute

:param con: the handle of the constraint
:param pyoptinterface.ConstraintAttribute attr: the attribute to set
:param value: the value to set
```

```{py:function} model.get_constraint_attribute(con, attr)

get the value of a constraint attribute

:param con: the handle of the constraint
:param pyoptinterface.ConstraintAttribute attr: the attribute to get
:return: the value of the attribute
```

### Delete constraint

```{py:function} model.delete_constraint(con)

delete a constraint from the model

:param con: the handle of the constraint
```

```{py:function} model.is_constraint_active(con)

query whether a constraint is active

:param con: the handle of the constraint
:return: whether the variable is active
:rtype: bool
```

### Modify constraint

```{py:function} model.set_normalized_rhs(con, value)

set the right-hand side of a normalized constraint

:param con: the handle of the constraint
:param value: the new right-hand side value
```

```{py:function} model.get_normalized_rhs(con)

get the right-hand side of a normalized constraint

:param con: the handle of the constraint
:return: the right-hand side value
```

```{py:function} model.set_normalized_coefficient(con, var, value)

set the coefficient of a variable in a normalized constraint

:param con: the handle of the constraint
:param var: the handle of the variable
:param value: the new coefficient value
```

```{py:function} model.get_normalized_coefficient(con, var)

get the coefficient of a variable in a normalized constraint

:param con: the handle of the constraint
:param var: the handle of the variable
:return: the coefficient value
```

## Objective

### Set the objective function

```{py:function} model.set_objective(expr, [sense=pyoptinterface.ObjectiveSense.Minimize])

set the objective function of the model

:param expr: the handle of the expression
:param pyoptinterface.ObjectiveSense sense: the sense of the objective function (Minimize/Maximize), defaults to Minimize
```

### Modify the linear part of the objective function

```{py:function} model.set_objective_coefficient(var, value)

modify the coefficient of a variable in the linear part of the objective function

:param var: the handle of the variable
:param float value: the new coefficient value
```

```{py:function} model.get_objective_coefficient(var)

get the coefficient of a variable in the linear part of the objective function

:param var: the handle of the variable
:return: the coefficient value
```