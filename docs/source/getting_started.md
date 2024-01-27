# Getting Started

## Installation
PyOptInterface has no dependencies other than Python itself. It can be installed via pip:
```console
pip install pyoptinterface
```

After installation, you can import the package via:
```python
import pyoptinterface as poi
```

## Let's build a simple model and solve it
As the first step, we will solve the following simple Quadratic Programming (QP) problem:

```{math}
\min \quad & x^{2}_{1} + 2x^{2}_{2} \\
\textrm{s.t.}  \quad & x_{1} + x_{2} = 1 \\
               \quad & x_{1}, x_{2} \geq 0
```

First, we need to create a model object:

:::{tip}
You need to acquire a valid license of [supported solvers](#supported-optimizers) to use them. If you want to use another optimizer, you can replace `gurobi` with `copt` or `mosek`)
:::

```python
import pyoptinterface as poi
from pyoptinterface import gurobi
# from pyoptinterface import copt (if you want to use COPT)

model = gurobi.Model()
```

Then, we need to add variables to the model:
```python
x1 = model.add_variable(lb=0, name="x1")
x2 = model.add_variable(lb=0, name="x2")
```
The `lb` argument specifies the lower bound of the variable. It is optional and defaults to $-\infty$.

The `name` argument is optional and can be used to specify the name of the variable.

Then, we need to add constraints to the model:
```python
con = model.add_linear_constraint(x1+x2, poi.ConstraintSense.Equal, 1, name="con")
```
`model.add_linear_constraint` adds a linear constraint to the model.
- The first argument `x1+x2` is the left-hand side of the constraint.
- The second argument is the sense of the constraint. It can be `poi.ConstraintSense.Equal`, `poi.ConstraintSense.LessEqual` or `poi.ConstraintSense.GreaterEqual`.
- The third argument is the right-hand side of the constraint. It must be a constant.
- The fourth argument is optional and can be used to specify the name of the constraint.

Finally, we need to set the objective function and solve the model:
```python
obj = x1*x1 + 2*x2*x2
model.set_objective(obj, poi.ObjectiveSense.Minimize)
```

The model can be solved via:
```python
model.optimize()
```
The Gurobi solver will be invoked to solve the model and writes the log to the console.

We can query the status of the model via:
```python
assert model.get_model_attribute(poi.ModelAttributes.TerminationStatus) == poi.TerminationStatusCode.OPTIMAL
```

The solution of the model can be queried via:
```python
print("x1 = ", model.get_value(x1))
print("x2 = ", model.get_value(x2))
print("obj = ", model.get_value(obj))
```

The whole code is as follows:
```python
import pyoptinterface as poi
from pyoptinterface import gurobi

model = gurobi.Model()
x1 = model.add_variable(lb=0, name="x1")
x2 = model.add_variable(lb=0, name="x2")

con = model.add_linear_constraint(x1+x2, poi.ConstraintSense.Equal, 1, name="con")

obj = x1*x1 + 2*x2*x2
model.set_objective(obj, poi.ObjectiveSense.Minimize)

model.optimize()
assert model.get_model_attribute(poi.ModelAttributes.TerminationStatus) == poi.TerminationStatusCode.OPTIMAL

print("x1 = ", model.get_value(x1))
print("x2 = ", model.get_value(x2))
print("obj = ", model.get_value(obj))
```
