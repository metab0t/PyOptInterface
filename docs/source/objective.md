# Objective

The objective is a function of the variables that the optimization algorithm seeks to minimize or maximize.

Currently PyOptInterface supports the following types of objective functions:
- Linear objective function
- Quadratic objective function

The objective function is typically minimized, but it can also be maximized. The objective function is defined as:

```python
objective = x*x
model.set_objective(objective, poi.ObjectiveSense.Minimize)
```

where `objective` is the handle of the objective function and `sense` is the optimization sense, which can be either `pyoptinterface.ObjectiveSense.Minimize` or `pyoptinterface.ObjectiveSense.Maximize`.

The `set_objective` function can be called multiple times to change the objective function of the model.

## Modify objective function

The linear part of the objective function can be modified by calling the `set_objective_coefficient` method of the model:

```python
obj = 2*x + 3*y
model.set_objective(obj, poi.ObjectiveSense.Minimize)

# modify the coefficient of the variable x
model.set_objective_coefficient(x, 5)
```
