---
file_format: mystnb
kernelspec:
  name: python3
---

# Building Bigger Optimization Model

In this document, we will introduce how to decompose the process of building a large optimization model into smaller parts and how to compose them together. This is a common practice in optimization modeling to make the code more readable and maintainable.

Generally speaking, we need two important parts to build an optimization model:

1. A [model](model.md) object that represents the optimization problem and communicates with the underlying optimizer. Due to the lightweight design philosophy of PyOptInterface, the model object is just a thin wrapper around the optimizer API, and it does not store the optimization problem itself. The model object is responsible for creating and managing variables, constraints, and the objective function, as well as solving the optimization problem.
2. A dict-like container to store the variables and constraints. We recommend the [`types.SimpleNamespace`](https://docs.python.org/3/library/types.html#types.SimpleNamespace) object as the container to store the variables and constraints. It is a simple way to store and manipulate the optimization problem in Python. You can use attribute access to get and set the variables and constraints.
```{code-cell}
import types

container = types.SimpleNamespace()
container.x = 1
container.y = 2

print(container.x, container.y)
```

Thus, we can define a class to represent the optimization model and use the container to store the variables and constraints.
```{code-cell}
import numpy as np
import pyoptinterface as poi
from pyoptinterface import highs

class OptModel:
    def __init__(self):
        self.container = types.SimpleNamespace()
        self.model = highs.Model()
```

Then we can define small functions to declare the variables and constraints and add them to the model. We take the N-queens problem as an example.
```{code-cell}
def declare_queen_variables(m:OptModel, N):
    model = m.model
    x = np.empty((N, N), dtype=object)
    for i in range(N):
        for j in range(N):
            x[i, j] = model.add_variable(domain=poi.VariableDomain.Binary)
    container = m.container
    container.x = x
    container.N = N

def add_row_column_constraints(m:OptModel):
    container = m.container
    N = container.N
    x = container.x
    model = m.model

    for i in range(N):
        # Row and column
        model.add_linear_constraint(poi.quicksum(x[i, :]), poi.Eq, 1.0)
        model.add_linear_constraint(poi.quicksum(x[:, i]), poi.Eq, 1.0)

def add_diagonal_constraints(m:OptModel):
    container = m.container
    N = container.N
    x = container.x
    model = m.model

    for i in range(-N+1, N):
        # Diagonal
        model.add_linear_constraint(poi.quicksum(x.diagonal(i)), poi.Leq, 1.0)
        # Anti-diagonal
        model.add_linear_constraint(poi.quicksum(np.fliplr(x).diagonal(i)), poi.Leq, 1.0)
```

Finally, we can compose these functions to build the optimization model and solve it.
```{code-cell}
m = OptModel()

N = 8
declare_queen_variables(m, N)
add_row_column_constraints(m)
add_diagonal_constraints(m)

m.model.optimize()

print("Termination status:", m.model.get_model_attribute(poi.ModelAttribute.TerminationStatus))
```
