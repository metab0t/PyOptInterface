---
file_format: mystnb
kernelspec:
  name: python3
---

# Model

Model is the central class of the PyOptInterface package. It provides the interface to the solver and the user. The user can add variables, constraints and the objective function to the model. The model can be solved and the solution can be queried.

In this document we will only discuss the common interface of the model. For solver-specific interface, please refer to the documentation of the corresponding solver.

## Create a model
A model is a concrete instance tied to a specific solver. To create a model, we need to import the corresponding module and call the constructor of the model class. For example, to create a HiGHS model, we can do:

```{code-cell}
import pyoptinterface as poi
from pyoptinterface import highs

model = highs.Model()
```

You can replace `highs` with the name of the solver you want to use. The available solvers includes `copt`, `gurobi`, `highs` and `mosek`.

Most commercial solvers require a `Environment`-like object to be initialized before creating a model in order to manage the license.

By default, PyOptInterface creates the global environment for each solver. If you want to create a model in a specific environment, you can pass the environment object to the constructor of the model class. The details can be found on the documentation of the corresponding optimizer: [Gurobi](gurobi.md), [HiGHS](highs.md), [COPT](copt.md), [MOSEK](mosek.md).

```python
env = gurobi.Env()
model = gurobi.Model(env)
```

## Inspect and customize the model
We can query and modify the parameters of the model to manipulate the behavior of the underlying solver.

Like the design in `JuMP.jl`, we define a small subset of parameters that are common to all solvers. They are defined as [pyoptinterface.ModelAttribute](#pyoptinterface.ModelAttribute) enum class.
The meanings of these standard attributes are the same as [Model attributes](https://jump.dev/JuMP.jl/stable/moi/reference/models/#Model-attributes) and [Optimizer attributes](https://jump.dev/JuMP.jl/stable/moi/reference/models/#Optimizer-attributes) in MathOptInterface.jl.

:::{list-table} **Standard [model attributes](#pyoptinterface.ModelAttribute)**
:header-rows: 1
:widths: 20 20

*   - Attribute name
    - Type
*   - Name
    - str
*   - ObjectiveSense
    - [ObjectiveSense](project:#pyoptinterface.ObjectiveSense)
*   - DualStatus
    - [ResultStatusCode](project:#pyoptinterface.ResultStatusCode)
*   - PrimalStatus
    - [ResultStatusCode](project:#pyoptinterface.ResultStatusCode)
*   - RawStatusString
    - str
*   - TerminationStatus
    - [TerminationStatusCode](project:#pyoptinterface.TerminationStatusCode)
*   - BarrierIterations
    - int
*   - DualObjectiveValue
    - float
*   - NodeCount
    - int
*   - NumberOfThreads
    - int
*   - ObjectiveBound
    - float
*   - ObjectiveValue
    - float
*   - RelativeGap
    - float
*   - Silent
    - float
*   - SimplexIterations
    - int
*   - SolverName
    - str
*   - SolverVersion
    - str
*   - SolveTimeSec
    - float
*   - TimeLimitSec
    - float
:::

We can set the value of a parameter by calling the `set_model_attribute` method of the model:

```{code-cell}
# suppress the output of the solver
model.set_model_attribute(poi.ModelAttribute.Silent, False)
# set the time limit to 10 seconds
model.set_model_attribute(poi.ModelAttribute.TimeLimitSec, 10.0)
```

The value of parameter can be queried by calling the `get_model_attribute` method of the model.

For example, we build and solve a simple quadratic programming model, then query the objective value of the model:

```{code-cell}
x = model.add_variables(range(2), lb=0.0, ub=1.0)
model.add_linear_constraint(x[0] + x[1], poi.Eq, 1.0)

model.set_objective(x[0]*x[0] + x[1]*x[1], sense=poi.ObjectiveSense.Minimize)

model.optimize()

objval = model.get_model_attribute(poi.ModelAttribute.ObjectiveValue)

print(f"Objective value: {objval}")
```

Besides the standard attributes, we can also set/get the solver-specific attributes by calling the `set_raw_parameter`/`get_raw_parameter` method of the model:

```python
# Gurobi
model.set_raw_parameter("OutputFlag", 0)
# COPT
model.set_raw_parameter("Presolve", 0)
# MOSEK
model.set_raw_parameter("MSK_IPAR_INTPNT_BASIS", 0)
```

## Solve the model
We can solve the model by calling the `optimize` method of the model:

```python
model.optimize()
```

## Query the solution
We can query the termination status of the model after optimization by query the `TerminationStatus` attribute of the model:

```{code-cell}
# tell if the optimizer obtains the optimal solution
termination_status = model.get_model_attribute(poi.ModelAttribute.TerminationStatus)
assert termination_status == poi.TerminationStatusCode.OPTIMAL
```

For value of variables and expressions, we can use `get_value` method of the model:

```{code-cell}
x0_value = model.get_value(x[0])
expr_value = model.get_value(x[0]*x[0])

print(f"x[0] = {x0_value}")
print(f"x[0]^2 = {expr_value}")
```

## Write the model to file
The optimization model can be written to file in LP, MPS or other formats. The `write` method of the model can be used to write the model to file:

```{code-cell}
model.write("model.lp")
```

The file format is determined by the file extension. Because we use the native IO procedure of the optimizer, their supported file formats and the content of output files may vary. Please refer to the documentation of the corresponding optimizer for more details.

- COPT: [Doc](https://guide.coap.online/copt/en-doc/fileformats.html)
- Gurobi: [Doc](https://www.gurobi.com/documentation/current/refman/c_write.html)
- HiGHS: [Doc](https://ergo-code.github.io/HiGHS/stable/interfaces/c/#Highs_writeModel-Tuple{Any,%20Any})
- Mosek: [Doc](https://docs.mosek.com/latest/capi/supported-file-formats.html)
