# Callback

:::{attention}
The behavior of callback function highly depends on the optimizer and the specific problem. Please refer to the official documentation of the optimizer for more details.
:::

In most optimization problems, we build the model, set the parameters, and then call the optimizer to solve the problem. However, in some cases, we may want to monitor the optimization process and intervene in the optimization process. For example, we may want to stop the optimization process when a certain condition is met, or we may want to record the intermediate results of the optimization process. In these cases, we can use the callback function. The callback function is a user-defined function that is called by the optimizer at specific points during the optimization process. Callback is especially useful for mixed-integer programming problems, where we can control the branch and bound process in callback functions.

Callback is not supported for all optimizers. Currently, we only support callback for Gurobi and COPT optimizer. Because callback is tightly coupled with the optimizer, we choose not to implement a strictly unified API for callback. Instead, we try to unify the common parts of the callback API of Gurobi and COPT and aims to provide all callback features included in vendored Python bindings of Gurobi and COPT.

In PyOptInterface, the callback function is simply a Python function that takes two arguments:
- `model`: The instance of the [optimization model](model.md)
- `where`: The flag indicates the stage of optimization process when our callback function is invoked. For Gurobi, the value of `where` is [CallbackCodes](https://www.gurobi.com/documentation/current/refman/cb_codes.html#sec:CallbackCodes). For COPT, the value of `where` is called as [callback contexts](https://guide.coap.online/copt/en-doc/callback.html) such as `COPT.CBCONTEXT_MIPNODE` and `COPT.CBCONTEXT_MIPRELAX`.

In the function body of the callback function, we can do the following four kinds of things:
- Query the current information of the optimization process. For scalar information, we can use `model.cb_get_info` function to get the information, and its argument is the value of [`what`](https://www.gurobi.com/documentation/current/refman/cb_codes.html) in Gurobi and the value of [callback information](https://guide.coap.online/copt/en-doc/information.html#chapinfo-cbc) in COPT. For array information such as the MIP solution or relaxation, PyOptInterface provides special functions such as `model.cb_get_solution` and `model.cb_get_relaxation`.
- Add lazy constraint: Use `model.cb_add_lazy_constraint` just like `model.add_linear_constraint` except for the `name` argument.
- Add user cut: Use `model.cb_add_user_cut` just like `model.add_linear_constraint` except for the `name` argument.
- Set a heuristic solution: Use `model.set_solution` to set individual values of variables and use `model.cb_submit_solution` to submit the solution to the optimizer immediately (`model.cb_submit_solution` will be called automatically in the end of callback if `model.set_solution` is called).
- Terminate the optimizer: Use `model.cb_exit`.

Here is an example of a callback function that stops the optimization process when the objective value reaches a certain threshold:

```python
import pyoptinterface as poi
from pyoptinterface import gurobi, copt
GRB = gurobi.GRB
COPT = copt.COPT

def cb_gurobi(model, where):
    if where == GRB.Callback.MIPSOL:
        obj = model.cb_get_info(GRB.Callback.MIPSOL_OBJ)
        if obj < 10:
            model.cb_exit()
            
def cb_copt(model, where):
    if where == COPT.CBCONTEXT_MIPSOL:
        obj = model.cb_get_info("MipCandObj")
        if obj < 10:
            model.cb_exit()
```

To use the callback function, we need to call `model.set_callback(cb)` to pass the callback function to the optimizer. For COPT, `model.set_callback` needs an additional argument `where` to specify the context where the callback function is invoked. For Gurobi, the `where` argument is not needed.

```python
model_gurobi = gurobi.Model()
model_gurobi.set_callback(cb_gurobi)

model_copt = copt.Model()
model_copt.set_callback(cb_copt, COPT.CBCONTEXT_MIPSOL)
# callback can also be registered for multiple contexts
model_copt.set_callback(cb_copt, COPT.CBCONTEXT_MIPSOL + COPT.CBCONTEXT_MIPNODE)
```

In order to help users to migrate code using gurobipy and/or coptpy to PyOptInterface, we list a translation table as follows.

:::{table} Callback in gurobipy and PyOptInterface
:align: left

| gurobipy                               | PyOptInterface                                          |
| -------------------------------------- | ------------------------------------------------------- |
| `model.optimize(cb)`                   | `model.set_callback(cb) `                               |
| `model.cbGet(GRB.Callback.SPX_OBJVAL)` | `model.cb_get_info(GRB.Callback.SPX_OBJVAL)`            |
| `model.cbGetSolution(var)`             | `model.cb_get_solution(var)`                            |
| `model.cbGetNodelRel(var)`             | `model.cb_get_relaxation(var)`                          |
| `model.cbLazy(x[0] + x[1] <= 3)`       | `model.cb_add_lazy_constraint(x[0] + x[1], poi.Leq, 3)` |
| `model.cbCut(x[0] + x[1] <= 3)`        | `model.cb_add_user_cut(x[0] + x[1], poi.Leq, 3)`        |
| `model.cbSetSolution(x, 1.0)`          | `model.cb_set_solution(x, 1.0)`                         |
| `objval = model.cbUseSolution()`       | `objval = model.cb_submit_solution()`                   |
| `model.termimate()`                    | `model.cb_exit()`                                       |

:::

:::{table} Callback in coptpy and PyOptInterface
:align: left

| coptpy                                         | PyOptInterface                                          |
| ---------------------------------------------- | ------------------------------------------------------- |
| `model.setCallback(cb, COPT.CBCONTEXT_MIPSOL)` | `model.set_callback(cb, COPT.CBCONTEXT_MIPSOL)`         |
| `CallbackBase.getInfo(COPT.CbInfo.BestBnd)`    | `model.cb_get_info(COPT.CbInfo.BestBnd)`                |
| `CallbackBase.getSolution(var)`                | `model.cb_get_solution(var)`                            |
| `CallbackBase.getRelaxSol(var)`                | `model.cb_get_relaxation(var)`                          |
| `CallbackBase.getIncumbent(var)`               | `model.cb_get_incumbent(var)`                           |
| `CallbackBase.addLazyConstr(x[0] + x[1] <= 3)` | `model.cb_add_lazy_constraint(x[0] + x[1], poi.Leq, 3)` |
| `CallbackBase.addUserCut(x[0] + x[1] <= 3)`    | `model.cb_add_user_cut(x[0] + x[1], poi.Leq, 3)`        |
| `CallbackBase.setSolution(x, 1.0) `            | `model.cb_set_solution(x, 1.0)`                         |
| `CallbackBase.loadSolution()`                  | `model.cb_submit_solution()`                            |
| `CallbackBase.interrupt()`                     | `model.cb_exit()`                                       |

:::

For a detailed example to use callbacks in PyOptInterface, we provide a [concrete callback example](https://github.com/metab0t/PyOptInterface/blob/master/tests/tsp_cb.py) to solve the Traveling Salesman Problem (TSP) with callbacks in PyOptInterface, gurobipy and coptpy. The example is adapted from the official Gurobi example [tsp.py](https://www.gurobi.com/documentation/current/examples/tsp_py.html).
