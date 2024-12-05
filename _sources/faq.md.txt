# Frequently Asked Questions

## How to suppress the output of the optimizer?

There are two kinds of output that you may want to suppress:

1. The log of optimization process.
2. The default license message printed when initializing the optimizer. For example, when using Gurobi, the message is `Academic license - for non-commercial use only - expires yyyy-mm-dd`.

Normally we only want to suppress the log of optimization process, you can use `model.set_model_attribute(poi.ModelAttribute.Silent, True)` to disable the output. For example:

```python
import pyoptinterface as poi
from pyoptinterface import gurobi

model = gurobi.Model()
model.set_model_attribute(poi.ModelAttribute.Silent, True)
```

Suppressing the default license message is a bit tricky and solver-specific. For Gurobi, you can use the following code:

```python
import pyoptinterface as poi
from pyoptinterface import gurobi

env = gurobi.Env(empty=True)
env.set_raw_parameter("OutputFlag", 0)
env.start()

model = gurobi.Model(env)
```

## How to add linear constraints in matrix form like $Ax \leq b$?

In YALMIP, you can use the matrix form $Ax \leq b$ to add linear constraints, which is quite convenient.

In PyOptInterface, you can use the following code to add linear constraints in matrix form:

```python
import pyoptinterface as poi
from pyoptinterface import gurobi

import numpy as np
from scipy.sparse import csr_array, sparray, eye_array


def iterate_sparse_matrix_rows(A):
    """
    Iterate over rows of a sparse matrix and get non-zero elements for each row.

    A is a 2-dimensional scipy sparse matrix
    isinstance(A, scipy.sparse.sparray) = True and A.ndim = 2
    """
    if not isinstance(A, csr_array):
        A = csr_array(A)  # Convert to CSR format if not already

    for i in range(A.shape[0]):
        row_start = A.indptr[i]
        row_end = A.indptr[i + 1]
        row_indices = A.indices[row_start:row_end]
        row_data = A.data[row_start:row_end]
        yield row_indices, row_data


def add_matrix_constraints(model, A, x, sense, b):
    """
    add constraints Ax <= / = / >= b

    A is a 2-dimensional numpy array or scipy sparse matrix
    x is an iterable of variables
    sense is one of (poi.Leq, poi.Eq, poi.Geq)
    b is an iterable of values or a single scalar
    """

    is_ndarray = isinstance(A, np.ndarray)
    is_sparse = isinstance(A, sparray)

    if not is_ndarray and not is_sparse:
        raise ValueError("A must be a numpy array or scipy.sparse array")

    ndim = A.ndim
    if ndim != 2:
        raise ValueError("A must be a 2-dimensional array")

    M, N = A.shape

    # turn x into a list if x is an iterable
    if isinstance(x, poi.tupledict):
        x = x.values()
    x = list(x)

    if len(x) != N:
        raise ValueError("x must have length equal to the number of columns of A")

    # check b
    if np.isscalar(b):
        b = np.full(M, b)
    elif len(b) != M:
        raise ValueError("b must have length equal to the number of rows of A")

    constraints = []

    if is_ndarray:
        for i in range(M):
            expr = poi.ScalarAffineFunction()
            row = A[i]
            for coef, var in zip(row, x):
                expr.add_term(var, coef)
            con = model.add_linear_constraint(expr, sense, b[i])
            constraints.append(con)
    elif is_sparse:
        for (row_indices, row_data), rhs in zip(iterate_sparse_matrix_rows(A), b):
            expr = poi.ScalarAffineFunction()
            for j, coef in zip(row_indices, row_data):
                expr.add_term(x[j], coef)
            con = model.add_linear_constraint(expr, sense, rhs)
            constraints.append(con)

    return constraints


def main():
    model = gurobi.Model()
    N = 200
    x = model.add_variables(range(N))
    A = np.eye(N)
    ub = 3.0
    lb = 1.0
    A_sparse = eye_array(N)
    add_matrix_constraints(model, A, x, poi.Leq, ub)
    add_matrix_constraints(model, A_sparse, x, poi.Geq, lb)

    obj = poi.quicksum(x)
    model.set_objective(obj)
    model.optimize()

    obj_value = model.get_model_attribute(poi.ModelAttribute.ObjectiveValue)
    print("Objective value: ", obj_value)


if __name__ == "__main__":
    main()
```