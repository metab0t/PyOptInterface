PyOptInterface (Python Optimization Interface)
=======

PyOptInterface is an open-source Python library to provide a unified API to construct and solve optimization models with various optimizers.

## Documentation
Find the documentation [here](https://metab0t.github.io/PyOptInterface/).

## Installation
```console
pip install pyoptinterface
```

## Example
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

## License
PyOptInterface is licensed under MPL-2.0 License.

It uses [nanobind](https://github.com/wjakob/nanobind), [fmtlib](https://github.com/fmtlib/fmt) and [martinus/unordered_dense](https://github.com/martinus/unordered_dense) as dependencies.

The design of PyOptInterface is inspired by [JuMP.jl](https://jump.dev).

Some solver-related code is adapted from corresponding solver interface package in `JuMP.jl` 
ecosystem, which is licensed under MIT License.

The code in `bench\jump_bench` is adapted from benchmark suite of paper of `JuMP.jl`.