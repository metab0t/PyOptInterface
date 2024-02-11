PyOptInterface
=======

## Overview

### What is PyOptInterface?

**PyOptInterface** is an open-source Python library to provide a unified API to construct and solve optimization models with various optimizers.

### Why another optimization modeling library after we have Pyomo/cvxpy/JuMP.jl?
It is designed as a very thin wrapper of native C API of optimizers and attempts to provide common abstractions of an algebraic modelling environment including model, variable, constraint and expression with the least overhead of performance.

The key features of PyOptInterface include:
- Very fast speed to construct optimization model (10-20x faster than Pyomo, comparable with JuMP.jl and some official Python bindings provided by vendor of optimizer)
- Low overhead to modify and re-solve the problem incrementally
- Unified API to cover common usages, write once and the code works for all optimizers
- You still have escape hatch to query or modify solver-specific parameter/attribute/information for different optimizers directly like the vendored Python binding of optimizer

### Benchmark result of time to construct optimization model
:::{table}
:widths: grid
:align: center

| Model     | Gurobi C++ | Gurobi Python | PyOptInterface | JuMP.jl(direct) | JuMP.jl(default) | Pyomo |
|-----------|------------|---------------|----------------|-----------------|------------------|-------|
| fac-25    | 0          | 1             | 0              | 1               | 0                | 5     |
| fac-50    | 1          | 10            | 2              | 2               | 4                | 34    |
| fac-75    | 3          | 34            | 6              | 8               | 11               | 115   |
| fac-100   | 6          | 84            | 12             | 26              | 27               | 284   |
| lqcp-500  | 1          | 6             | 2              | 2               | 3                | 24    |
| lqcp-1000 | 4          | 27            | 7              | 8               | 8                | 100   |
| lqcp-1500 | 10         | 61            | 15             | 22              | 20               | 232   |
| lqcp-2000 | 17         | 118           | 26             | 48              | 40               | 403   |
:::

:::{note}

Time measured in seconds, smaller is better
:::

### What kind of problems can PyOptInterface solve?
It currently supports the following problem types:
- Linear Programming (LP)
- Mixed-Integer Linear Programming (MILP)
- Quadratic Programming (QP)
- Mixed-Integer Quadratic Programming (MIQP)
- Quadratically Constrained Quadratic Programming (QCQP)
- Mixed-Integer Quadratically Constrained Quadratic Programming (MIQCQP)
- Second-Order Cone Programming (SOCP)
- Mixed-Integer Second-Order Cone Programming (MISOCP)

(supported-optimizers)=
### What optimizers does PyOptInterface support?
It currently supports the following optimizers:
- [Gurobi](https://www.gurobi.com/)(Commercial)
- [COPT](https://shanshu.ai/copt)(Commercial)
- [MOSEK](https://www.mosek.com/)(Commercial)
- [HiGHS](https://www.highs.dev/)(Open source)

## Contents
```{toctree}
:maxdepth: 2
:titlesonly:
:caption: Usage Guide
:glob:

getting_started.md
model.md
variable.md
expression.md
constraint.md
objective.md
container.md
common_model_interface.md
```

```{toctree}
:maxdepth: 1
:titlesonly:
:caption: API docs

api/pyoptinterface.rst
```

## Indices and tables

- {ref}`genindex`
- {ref}`modindex`
- {ref}`search`