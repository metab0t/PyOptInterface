---
file_format: mystnb
kernelspec:
  name: python3
---
# Constraint

The `Constraint` class is used to define a constraint in the optimization model. It is a subclass
of `Expression` and has the same methods and properties as `Expression`.

PyOptInterface supports the following types of constraints:

- Linear Constraint
- Quadratic Constraint
- Second-Order Cone Constraint
- Special Ordered Set (SOS) Constraint

:::{note}

Not all optimizers support all types of constraints. Please refer to the documentation of the
optimizer you are using to see which types of constraints are supported.
:::

```{code-cell}
import pyoptinterface as poi
from pyoptinterface import copt

model = copt.Model()
```

## Linear Constraint
It is defined as:

$$
\begin{align}
    \text{expr} &= a^T x + b &\leq \text{rhs} \\
    \text{expr} &= a^T x + b &= \text{rhs} \\
    \text{expr} &= a^T x + b &\geq \text{rhs}
\end{align}
$$

It can be added to the model using the `add_linear_constraint` method of the `Model` class.

```{code-cell}
x = model.add_variable(name="x")
y = model.add_variable(name="y")

con = model.add_linear_constraint(2.0*x + 3.0*y, poi.ConstraintSense.LessEqual, 1.0)
```

```{py:function} model.add_linear_constraint(expr, sense, rhs, [name=""])

add a linear constraint to the model

:param expr: the expression of the constraint
:param pyoptinterface.ConstraintSense sense: the sense 
of the constraint, which can be `GreaterEqual`, `Equal`, or `LessEqual`
:param float rhs: the right-hand side of the constraint
:param str name: the name of the constraint, optional
:return: the handle of the constraint
```

:::{note}

PyOptInterface provides <project:#pyoptinterface.Eq>, <project:#pyoptinterface.Leq>, and <project:#pyoptinterface.Geq> as alias of <project:#pyoptinterface.ConstraintSense> to represent the sense of the constraint with a shorter name.
:::

## Quadratic Constraint
Like the linear constraint, it is defined as:

$$
\begin{align}
    \text{expr} &= x^TQx + a^Tx + b &\leq \text{rhs} \\
    \text{expr} &= x^TQx + a^Tx + b &= \text{rhs} \\
    \text{expr} &= x^TQx + a^Tx + b &\geq \text{rhs}
\end{align}
$$

It can be added to the model using the `add_quadratic_constraint` method of the `Model` class.

```{code-cell}
x = model.add_variable(name="x")
y = model.add_variable(name="y")

expr = x*x + 2.0*x*y + 4.0*y*y
con = model.add_quadratic_constraint(expr, poi.ConstraintSense.LessEqual, 1.0)
```

```{py:function} model.add_quadratic_constraint(expr, sense, rhs, [name=""])

add a quadratic constraint to the model

:param expr: the expression of the constraint
:param pyoptinterface.ConstraintSense sense: the sense 
of the constraint, which can be `GreaterEqual`, `Equal`, or `LessEqual`
:param float rhs: the right-hand side of the constraint
:param str name: the name of the constraint, optional
:return: the handle of the constraint
```

## Second-Order Cone Constraint
It is defined as:

$$
variables=(t,x) \in \mathbb{R}^{N} : t \ge \lVert x \rVert_2
$$

It can be added to the model using the `add_second_order_cone_constraint` method of the `Model` 
class.

```{code-cell}
N = 6
vars = [model.add_variable() for i in range(N)]

con = model.add_second_order_cone_constraint(vars)
```

There is another form of second-order cone constraint called as rotated second-order cone constraint, which is defined as:

$$
variables=(t_{1},t_{2},x) \in \mathbb{R}^{N} : 2 t_1 t_2 \ge \lVert x \rVert_2^2
$$

```{py:function} model.add_second_order_cone_constraint(variables, [name="", rotated=False])

add a second order cone constraint to the model

:param variables: the variables of the constraint, can be a list of variables
:param str name: the name of the constraint, optional
:param bool rotated: whether the constraint is a rotated second-order cone constraint, optional
:return: the handle of the constraint
```

## Exponential Cone Constraint
It is defined as:

$$
variables=(t,s,r) \in \mathbb{R}^{3} : t \ge s \exp(\frac{r}{s}), s \ge 0
$$

The dual form is:

$$
variables=(t,s,r) \in \mathbb{R}^{3} : t \ge -r \exp(\frac{s}{r} - 1), r \le 0
$$

Currently, only COPT(after 7.1.4), Mosek support exponential cone constraint. It can be added to the model using the `add_exp_cone_constraint` method of the `Model` class.

```{py:function} model.add_exp_cone_constraint(variables, [name="", dual=False])

add a second order cone constraint to the model

:param variables: the variables of the constraint, can be a list of variables
:param str name: the name of the constraint, optional
:param bool dual: whether the constraint is dual form of exponential cone, optional
:return: the handle of the constraint
```

## Special Ordered Set (SOS) Constraint
SOS constraints are used to model special structures in the optimization problem.
It contains two types: `SOS1` and `SOS2`, the details can be found in [Wikipedia](https://en.wikipedia.org/wiki/Special_ordered_set).

It can be added to the model using the `add_sos_constraint` method of the `Model` class.

```{code-cell}
N = 6
vars = [model.add_variable(domain=poi.VariableDomain.Binary) for i in range(N)]

con = model.add_sos_constraint(vars, poi.SOSType.SOS1)
```

```{py:function} model.add_sos_constraint(variables, sos_type, [weights])

add a special ordered set constraint to the model

:param variables: the variables of the constraint, can be a list of variables
:param pyoptinterface.SOSType sos_type: the type of the SOS constraint, which can be `SOS1` or `SOS2`
:param weights: the weights of the variables, optional, will be set to 1 if not provided
:type weights: list[float]
:return: the handle of the constraint
```

## Constraint Attributes
After a constraint is created, we can query or modify its attributes. The following table lists the 
standard [constraint attributes](#pyoptinterface.ConstraintAttribute):

:::{list-table} **Standard [constraint attributes](#pyoptinterface.ConstraintAttribute)**
:header-rows: 1
:widths: 20 20

*   - Attribute name
    - Type
*   - Name
    - str
*   - Primal
    - float
*   - Dual
    - float
:::

The most common attribute we will use is the `Dual` attribute, which represents the dual multiplier of the constraint after optimization.

```python
# get the dual multiplier of the constraint after optimization
dual = model.get_constraint_attribute(con, poi.ConstraintAttribute.Dual)
```

## Delete constraint
We can delete a constraint by calling the `delete_constraint` method of the model:

```python
model.delete_constraint(con)
```

After a constraint is deleted, it cannot be used in the model anymore, otherwise an exception 
will be raised.

We can query whether a constraint is active by calling the `is_constraint_active` method of the 
model:

```python
is_active = model.is_constraint_active(con)
```

## Modify constraint
For linear and quadratic constraints, we can modify the right-hand side of a constraint by 
calling the `set_normalized_rhs` method of the model.

For linear constraints, we can modify the coefficients of the linear part of the constraint by
calling the `set_normalized_coefficient` method of the model.

```python
con = model.add_linear_constraint(x + y, poi.Leq, 1.0)

# modify the right-hand side of the constraint
model.set_normalized_rhs(con, 2.0)

# modify the coefficient of the linear part of the constraint
model.set_normalized_coefficient(con, x, 2.0)
```
