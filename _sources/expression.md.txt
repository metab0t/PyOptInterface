# Expression

## Basic expression
PyOptInterface currently supports polynomial expressions with degree up to 2, including
- quadratic expression
- linear expression
- constant expression

The expression can be expressed by arithmetic operations of variables and constants. For example, we can create a quadratic expression by adding two quadratic expressions, multiplying a linear expression with a constant expression, etc.

```python
import pyoptinterface as poi
from pyoptinterface import gurobi

model = gurobi.Model()

x = model.add_variable()

# create a quadratic expression
expr1 = x * x + 2 * x + 1
# create a linear expression
expr2 = 2 * x + 1

# create a quadratic expression by adding two quadratic expressions
expr3 = x * expr2 + expr1
```

## Efficient expression construction
PyOptInterface provides a special class `ExprBuilder` to construct expressions efficiently. It is especially useful when we need to construct a large expression with many terms.

It supports the following in-place assignment operations:
- `+=`: add a term to the expression
- `-=`: subtract a term from the expression
- `*=`: multiply the expression with a constant or another expression
- `/=`: divide the expression with a constant

For example, we can use `ExprBuilder` to construct the following expression efficiently:

$$
\frac{1}{2} \sum_{i=1}^N x_i^2 - \sum_{i=1}^N x_i
$$

```python
expr = poi.ExprBuilder()

N = 1000
xs = [model.add_variable() for _ in range(N)]

for i in range(N):
    expr += x[i] * x[i]
    expr -= 2 * x[i]

expr *= 0.5
```

## Pretty print expression
If the names of variables are specified, We can use the `pprint` method to print the expression in a human-readable format:

```python
x = model.add_variable(name="x")
y = model.add_variable(name="y")

expr = x * x + 2 * x * y + y * y

model.pprint(expr)
# output: 1*x*x + 2*x*y + 1*y*y
```

## Value of expression
We can use the `get_value` method to get the value of an expression after optimization:

```python
expr = x*y + x*x
expr_value = model.get_value(expr)
```
