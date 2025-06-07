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

In PyOptInterface, you can use [`model.add_m_linear_constraints`](<project:#model.add_m_linear_constraints>) to add linear constraints in matrix form.

## Will PyOptInterface support new optimizers in the future?

In short, no, there are no plans to support new optimizers. Supporting a new optimizer is not a trivial task, as it requires a lot of work to implement, test and maintain the interface.

Basically, a new optimizer should satisfy the following criteria to be considered for support:

- Actively maintained
- Good performance (open source or commercial)
- Not difficult to acquire an academic license
- Have well-defined C/C++ API

Support for a new optimizer will only happen if one or more of the following conditions are met:

- I am personally interested in the optimizer and plan to use it in my research, so I am willing to invest time in implementing it.
- Someone steps up to implement and maintain the interface for the optimizer in PyOptInterface.
- External funding or sponsorship become available to support the development and maintenance of the optimizer interface.

Finally, we are always open to external contributions. If you have a specific optimizer in mind and plan to implement it, feel free to open an issue on our GitHub repository to discuss it.
