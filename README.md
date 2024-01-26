PyOptInterface
=======

# Overview

**PyOptInterface** is an open-source Python library to provide a unified API to construct and solve optimization models with various optimizers.

It is designed as a very thin wrapper of native C API of optimizers and attempts to provide common abstractions of an algebraic modelling environment including model, variable, constraint and expression with the least overhead of performance.

The key features of PyOptInterface include:
- Very fast speed to construct optimization model (10-20x faster than Pyomo, faster than JuMP.jl and some official Python bindings of optimizers)
- Low overhead to modify and re-solve the problem incrementally
- Unified API to cover common usages, write once and the code works for all optimizers
- You still have escape hatch to query or modify solver-specific parameter/attribute/information for different optimizers directly like the vendored Python binding of optimizer

It currently supports the following optimizers:
- [Gurobi](https://www.gurobi.com/)
- [COPT](https://shanshu.ai/copt)
- [MOSEK](https://www.mosek.com/)

It currently supports the following problem types:
- Linear Programming (LP)
- Mixed-Integer Linear Programming (MILP)
- Quadratic Programming (QP)
- Mixed-Integer Quadratic Programming (MIQP)
- Quadratically Constrained Quadratic Programming (QCQP)
- Mixed-Integer Quadratically Constrained Quadratic Programming (MIQCQP)

# Installation
```bash
pip install pyoptinterface
```

# Example
```python
import pyoptinterface as poi
from pyoptinterface import gurobi

model = gurobi.Model()

x = model.add_variable(lb=0, ub=1, domain=poi.VariableDomain.Continuous, name="x")
y = model.add_variable(lb=0, ub=1, domain=poi.VariableDomain.Integer, name="y")

con = model.add_linear_constraint(x+y, poi.ConstraintSense.GreaterEqual, 1.2, name="con")

obj = x*x + y*y
model.set_objective(obj, poi.ObjectiveSense.Minimize)

model.set_model_attribute(poi.ModelAttribute.Silent, False)
model.optimize()

print(model.get_model_attribute(poi.ModelAttribute.TerminationStatus))
# TerminationStatusCode.OPTIMAL

x_val = model.get_variable_attribute(x, poi.VariableAttribute.Value)
# 0.2

y_val = model.get_variable_attribute(y, poi.VariableAttribute.Value)
# 1.0
```

# Documentation

# License
PyOptInterface is licensed under MPL-2.0 License.