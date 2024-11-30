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
