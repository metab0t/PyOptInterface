PyOptInterface (Python Optimization Interface)
=======

[![](https://img.shields.io/pypi/v/pyoptinterface.svg?color=brightgreen)](https://pypi.org/pypi/pyoptinterface/)

**PyOptInterface** is an open-source Python library to provide a unified API to construct and solve optimization models with various optimizers.

The detailed documentation can be found [here](https://metab0t.github.io/PyOptInterface/).

## Key features compared with other modeling interfaces
It is designed as a very thin wrapper of native C API of optimizers and attempts to provide common abstractions of an algebraic modelling environment including model, variable, constraint and expression with the least overhead of performance.

The key features of PyOptInterface include:
- Very fast speed to construct optimization model (10x faster than Pyomo, comparable with JuMP.jl and some official Python bindings provided by vendors of optimizer)
- Low overhead to modify and re-solve the problem incrementally (including adding/removing variables/constraints, changing objective function, etc.)
- Unified API to cover common usages, write once and the code works for all optimizers
- You still have escape hatch to query or modify solver-specific parameter/attribute/information for different optimizers directly like the vendor-specific Python binding of optimizer

## Benchmark
The benchmark comparing PyOptInterface with some other modeling interfaces can be found [here](https://metab0t.github.io/PyOptInterface/benchmark.html). PyOptInterface is among the fastest modeling interfaces in terms of model construction time.

## Installation
PyOptInterface is available on PyPI. You can install it via pip:

```
pip install pyoptinterface
```

After installation, you can import the package in Python console:
```python
import pyoptinterface as poi
```

PyOptInterface has no dependencies other than Python itself. However, to use it with a specific optimizer, you need to install the corresponding optimizer manually. The details can be found on [the configurations of optimizers](https://metab0t.github.io/PyOptInterface/getting_started.html).

In order to provide out-of-the-box support for open source optimizers (currently we support [HiGHS](https://github.com/ERGO-Code/HiGHS)), PyOptInterface can also be installed with pre-built optimizers. You can install them via pip:

```
pip install pyoptinterface[highs]
```

It will install a full-featured binary version of HiGHS optimizer via [highsbox](http://github.com/metab0t/highsbox), which can be used with PyOptInterface.

## What kind of problems can PyOptInterface solve?
It currently supports the following problem types:
- Linear Programming (LP)
- Mixed-Integer Linear Programming (MILP)
- Quadratic Programming (QP)
- Mixed-Integer Quadratic Programming (MIQP)
- Quadratically Constrained Quadratic Programming (QCQP)
- Mixed-Integer Quadratically Constrained Quadratic Programming (MIQCQP)
- Second-Order Cone Programming (SOCP)
- Mixed-Integer Second-Order Cone Programming (MISOCP)

## What optimizers does PyOptInterface support?
It currently supports the following optimizers:
- [COPT](https://shanshu.ai/copt) ( Commercial )
- [Gurobi](https://www.gurobi.com/) ( Commercial )
- [HiGHS](https://github.com/ERGO-Code) ( Open source )
- [Mosek](https://www.mosek.com/) ( Commercial )

## Short Example
```python
import pyoptinterface as poi
from pyoptinterface import highs

model = highs.Model()

x = model.add_variable(lb=0, ub=1, domain=poi.VariableDomain.Continuous, name="x")
y = model.add_variable(lb=0, ub=1, domain=poi.VariableDomain.Integer, name="y")

con = model.add_linear_constraint(x+y, poi.Geq, 1.2, name="con")

obj = 2*x
model.set_objective(obj, poi.ObjectiveSense.Minimize)

model.set_model_attribute(poi.ModelAttribute.Silent, False)
model.optimize()

print(model.get_model_attribute(poi.ModelAttribute.TerminationStatus))
# TerminationStatusCode.OPTIMAL

x_val = model.get_value(x)
# 0.2

y_val = model.get_value(y)
# 1.0
```

## License
PyOptInterface is licensed under MPL-2.0 License.

It uses [nanobind](https://github.com/wjakob/nanobind), [fmtlib](https://github.com/fmtlib/fmt) and [martinus/unordered_dense](https://github.com/martinus/unordered_dense) as dependencies.

The design of PyOptInterface is inspired by [JuMP.jl](https://jump.dev).

Some solver-related code in `src` folder is adapted from the corresponding solver interface package in `JuMP.jl` 
ecosystem, which is licensed under MIT License.

The header files in `thirdparty/solvers` directory are from the corresponding distribution of optimizers and are licensed under their own licenses.
