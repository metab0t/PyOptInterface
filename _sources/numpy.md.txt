---
file_format: mystnb
kernelspec:
  name: python3
---

# Numpy Container and N-queens Problem

In the previous [container](container.md) section, we have introduced the `tupledict` container to store and manipulate multi-dimensional data.

However, due to the Bring Your Own Container (BYOC) principle, variables and constraints in PyOptInterface can just simple Python objects that can be stored in Numpy `ndarrays` directly as a multi-dimensional array, and you can enjoy the features of Numpy such like [fancy-indexing](https://numpy.org/doc/stable/user/basics.indexing.html) automatically. 

We will use N-queens problem as example to show how to use Numpy `ndarrays` as container to store 2-dimensional variables and construct optimization model.

Firstly, we import the necessary modules:

```{code-cell}
import numpy as np
import pyoptinterface as poi
from pyoptinterface import highs

model = highs.Model()
```

Then we create a 2-dimensional variable `x` with shape $N \times N$ to represent the placement of queens. Each element of `x` is a binary variable that indicates whether a queen is placed at the corresponding position. We use `object` as the data type of `x` to store the binary variables. The following code snippet creates the variables:

```{code-cell}
N = 8

x = np.empty((N, N), dtype=object)
for i in range(N):
    for j in range(N):
        x[i, j] = model.add_variable(domain=poi.VariableDomain.Binary)
```

Next, we add the constraints to ensure that each row, each column has exact one queen, and each diagonal has at most one queen. 

```{code-cell}
for i in range(N):
    # Row and column
    model.add_linear_constraint(poi.quicksum(x[i, :]), poi.Eq, 1.0)
    model.add_linear_constraint(poi.quicksum(x[:, i]), poi.Eq, 1.0)
for i in range(-N+1, N):
    # Diagonal
    model.add_linear_constraint(poi.quicksum(x.diagonal(i)), poi.Leq, 1.0)
    # Anti-diagonal
    model.add_linear_constraint(poi.quicksum(np.fliplr(x).diagonal(i)), poi.Leq, 1.0)
```

Finally, we solve the model.

```{code-cell}
model.optimize()

print("Termination status:", model.get_model_attribute(poi.ModelAttribute.TerminationStatus))
```

The solution can be obtained and visualized by the following code:

```{code-cell}
get_v = np.vectorize(lambda x: model.get_value(x))
x_value = get_v(x)

print(x_value.astype(int))
```
