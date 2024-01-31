# Model

Model is the central class of the PyOptInterface package. It provides the interface to the solver and the user. The user can add variables, constraints and the objective function to the model. The model can be solved and the solution can be queried.

In this document we will only discuss the common interface of the model. For solver-specific interface, please refer to the documentation of the corresponding solver.

## Create a model
A model is a concrete instance tied to a specific solver. To create a model, we need to import the corresponding module and call the constructor of the model class. For example, to create a Gurobi model, we can do:

```python
import pyoptinterface as poi
from pyoptinterface import gurobi

model = gurobi.Model()
```

Most commercial solvers require a `Environment`-like object to be initialized before creating a model in order to manage the license.

By default, PyOptInterface creates the global environment for each solver. If you want to create a model in a specific environment, you can pass the environment object to the constructor of the model class:

```python
env = gurobi.Env()
model = gurobi.Model(env)
```

## Inspect and customize the model
We can query and modify the parameters of the model to manipulate the behavior of underlying solver.

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

```python
# suppress the output of the solver
model.set_model_attribute(poi.ModelAttribute.Silent, True)
# set the time limit to 10 seconds
model.set_model_attribute(poi.ModelAttribute.TimeLimitSec, 10.0)
```

The value of parameter can be queried by calling the `get_model_attribute` method of the model:

```python
objval = model.get_model_attribute(poi.ModelAttribute.ObjectiveValue)
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

```python
# tell if the optimizer obtains the optimal solution
termination_status = model.get_model_attribute(poi.ModelAttribute.TerminationStatus)
assert termination_status == poi.TerminationStatusCode.Optimal
```

For value of variables and expressions, we can use `get_value` method of the model:

```python
x_value = model.get_value(x)
expr_value = model.get_value(x*x)
```
