# Ipopt

## Initial setup

```python
from pyoptinterface import ipopt

model = ipopt.Model()
```

You need to follow the instructions in [Getting Started](getting_started.md#ipopt) to set up the optimizer correctly.

:::{attention}

Due to the internal API design of Ipopt, the `ipopt.Model` lacks some features compared to other solvers. Some of these restrictions may be lifted in the future.

- It does not support `delete_variable` and `delete_constraint` methods.
- It does not support incremental modification of linear constraint like `set_normalized_rhs` and `set_normalized_coefficient`.
- It only supports to minimize the objective function. If you want to maximize the objective function, you need to multiply the objective function by -1 manually.
:::

## The capability of `ipopt.Model`

### Supported constraints

:::{list-table}
:header-rows: 1

*   - Constraint
    - Supported
*   - <project:#model.add_linear_constraint>
    - ✅
*   - <project:#model.add_quadratic_constraint>
    - ✅
*   - <project:#model.add_second_order_cone_constraint>
    - ❌
*   - <project:#model.add_sos_constraint>
    - ❌
*   - <project:#model.add_nl_constraint>
    - ✅

:::

```{include} attribute/ipopt.md
```


## Solver-specific operations

### Parameter

For [solver-specific parameters](https://coin-or.github.io/Ipopt/OPTIONS.html), we provide `set_raw_parameter` methods to get and set the parameters.

```python
model = ipopt.Model()

# set the value of the parameter
model.set_raw_parameter("tol", 1e-5)
model.set_raw_parameter("max_iter", 200)

# For HSL library
model.set_raw_parameter("hsllib", "/path/to/libhsl.so")
model.set_raw_parameter("linear_solver", "ma27")
```

## JIT compiler used by Ipopt interface

The interface of Ipopt uses the JIT compiler to compile the nonlinear objective function, constraints and their derivatives. We have two implementations of JIT based on `llvmlite` and `tccbox`(Tiny C Compiler). The default JIT compiler is `llvmlite` and we advise you to use it for better performance brought by optimization capability of LLVM. If you want to use `tccbox`, you can specify `jit="C"` when creating the `ipopt.Model` object.

```python
model = ipopt.Model()
# equivalent to
model = ipopt.Model(jit="LLVM")

# If you want to use tccbox
model = ipopt.Model(jit="C")
```
