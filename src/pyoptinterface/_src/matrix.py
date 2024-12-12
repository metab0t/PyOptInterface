from .tupledict import tupledict
from .core_ext import ScalarAffineFunction


def iterate_sparse_matrix_rows(A):
    """
    Iterate over rows of a sparse matrix and get non-zero elements for each row.

    A is a 2-dimensional scipy sparse matrix
    isinstance(A, scipy.sparse.sparray) = True and A.ndim = 2
    """
    from scipy.sparse import csr_array

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
    import numpy as np
    from scipy.sparse import sparray

    is_ndarray = isinstance(A, np.ndarray)
    is_sparse = isinstance(A, sparray)

    if not is_ndarray and not is_sparse:
        raise ValueError("A must be a numpy array or scipy.sparse array")

    ndim = A.ndim
    if ndim != 2:
        raise ValueError("A must be a 2-dimensional array")

    M, N = A.shape

    # turn x into a list if x is an iterable
    if isinstance(x, np.ndarray):
        xdim = x.ndim
        if xdim != 1:
            raise ValueError("x must be a 1-dimensional array")
    elif isinstance(x, tupledict):
        x = list(x.values())
    else:
        x = list(x)

    if len(x) != N:
        raise ValueError("x must have length equal to the number of columns of A")

    # check b
    if np.isscalar(b):
        b = np.full(M, b)
    elif len(b) != M:
        raise ValueError("b must have length equal to the number of rows of A")

    constraints = np.empty(M, dtype=object)

    if is_ndarray:
        for i in range(M):
            expr = ScalarAffineFunction()
            row = A[i]
            for coef, var in zip(row, x):
                expr.add_term(var, coef)
            con = model.add_linear_constraint(expr, sense, b[i])
            constraints[i] = con
    elif is_sparse:
        for i, (row_indices, row_data), rhs in zip(
            range(M), iterate_sparse_matrix_rows(A), b
        ):
            expr = ScalarAffineFunction()
            for j, coef in zip(row_indices, row_data):
                expr.add_term(x[j], coef)
            con = model.add_linear_constraint(expr, sense, rhs)
            constraints[i] = con

    return constraints
