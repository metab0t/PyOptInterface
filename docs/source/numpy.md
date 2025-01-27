---
file_format: mystnb
kernelspec:
  name: python3
---

# Matrix Modeling

In the previous [container](container.md) section, we have introduced the `tupledict` container to store and manipulate multidimensional data.

However, due to the Bring Your Own Container (BYOC) principle, variables and constraints in PyOptInterface can just simple Python objects that can be stored in Numpy `ndarray` directly as a multidimensional array, and you can enjoy the features of Numpy such like [fancy-indexing](https://numpy.org/doc/stable/user/basics.indexing.html) automatically.

## N-queen problem

We will use N-queens problem as example to show how to use Numpy `ndarray` as container to store 2-dimensional variables and construct optimization model.

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

## Built-in functions to add variables and constraints as Numpy `ndarray`

Although you can construct the `ndarray` of variables and constraints manually, PyOptInterface provides built-in functions to simplify the process. The following code snippet shows how to use the built-in functions to add variables and constraints as Numpy `ndarray`:

```{code-cell}
model = highs.Model()

x = model.add_m_variables(N)

A = np.eye(N)
b_ub = np.ones(N)
b_lb = np.ones(N)

model.add_m_linear_constraints(A, x, poi.Leq, b_ub)
model.add_m_linear_constraints(A, x, poi.Geq, b_lb)

model.set_objective(poi.quicksum(x))

model.optimize()
```

Here we use two built-in functions `add_m_variables` and `add_m_linear_constraints` to add variables and constraints as Numpy `ndarray` respectively.

The reference of these functions are listed in <project:#model.add_m_variables> and <project:#model.add_m_linear_constraints>.

`add_m_variables` returns a `ndarray` of variables with the specified shape.

`add_m_linear_constraints` adds multiple linear constraints to the model at once formulated as $Ax \le b$ or $Ax = b$ or $Ax \ge b$ where the matrix $A$ can be a dense `numpy.ndarray` or a sparse matrix `scipy.sparse.sparray`.
