# Modeling Examples

## 1. Knapsack Problem (MIP)
Maximize total value given a weight limit.
```python
import pyoptinterface as poi
from pyoptinterface import highs

values = [10, 15, 20, 8]
weights = [2, 3, 5, 2]
limit = 8

model = highs.Model()
x = model.add_m_variables(len(values), domain=poi.VariableDomain.Binary)

model.set_objective(poi.quicksum(values[i] * x[i] for i in range(len(values))), poi.ObjectiveSense.Maximize)
model.add_linear_constraint(poi.quicksum(weights[i] * x[i] for i in range(len(weights))) <= limit)

model.optimize()
if model.get_model_attribute(poi.ModelAttribute.TerminationStatus) == poi.TerminationStatusCode.OPTIMAL:
    print("Selected items:", [i for i in range(len(values)) if model.get_variable_attribute(x[i], poi.VariableAttribute.Value) > 0.5])
```

## 2. Sparse Matrix Model (LP)
Solve $Ax \le b$ using the matrix API.
```python
import numpy as np
from scipy import sparse
import pyoptinterface as poi
from pyoptinterface import highs

m, n = 100, 500
A = sparse.random(m, n, density=0.1)
b = np.random.rand(m)
c = np.random.rand(n)

model = highs.Model()
x = model.add_m_variables(n, lb=0)

model.set_objective(poi.quicksum(c[i] * x[i] for i in range(n)))
model.add_m_linear_constraints(A, x, poi.Leq, b)

model.optimize()
```

## 3. Nonlinear Rosenbrock Function (NLP)
Minimize $f(x, y) = (1-x)^2 + 100(y-x^2)^2$.
```python
import pyoptinterface as poi
from pyoptinterface import ipopt, nl

model = ipopt.Model()
x = model.add_variable(lb=-5, ub=5)
y = model.add_variable(lb=-5, ub=5)

with nl.graph():
    model.add_nl_objective((1 - x)**2 + 100 * (y - x**2)**2, poi.ObjectiveSense.Minimize)

model.optimize()
print(f"Optimal x: {model.get_value(x)}, y: {model.get_value(y)}")
```
