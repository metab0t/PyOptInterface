---
file_format: mystnb
kernelspec:
  name: python3
---

# Infeasibility Analysis

The optimization model is not ways feasible, and the optimizer may tell us some information about the infeasibility to diagnose the problem. There are two ways to  handle the infeasibilities:

- Find the IIS (Irreducible Infeasible Set) to identify the minimal set of constraints that cause the infeasibility.
- Relax the constraints and solve a weaker problem to find out which constraints are violated and how much.

PyOptInterface currently supports the first method to find the IIS (only with Gurobi, Xpress, and COPT). The following code snippet shows how to find the IIS of an infeasible model:

```{code-cell}
import pyoptinterface as poi
from pyoptinterface import copt

model = copt.Model()

x = model.add_variable(lb=0.0, name="x")
y = model.add_variable(lb=0.0, name="y")

con1 = model.add_linear_constraint(x + y, poi.Geq, 5.0)
con2 = model.add_linear_constraint(x + 2 * y, poi.Leq, 1.0)

model.set_objective(x)

model.computeIIS()

con1_iis = model.get_constraint_attribute(con1, poi.ConstraintAttribute.IIS)
con2_iis = model.get_constraint_attribute(con2, poi.ConstraintAttribute.IIS)

print(f"Constraint 1 IIS: {con1_iis}")
print(f"Constraint 2 IIS: {con2_iis}")
```

This code snippet creates an infeasible model with two constraints and finds the IIS of the model. Obviously, the constraints are contradictory because `x + 2 * y <= 1` and `x + y >= 5` cannot be satisfied at the same time when `x` and `y` are non-negative. The optimizer will detect that the model is infeasible and return the IIS, which is the set of constraints that cause the infeasibility. We can query whether a constraint is in the IIS by calling `get_constraint_attribute` with the `ConstraintAttribute.IIS` attribute.

Sometimes, the bounds of the variables are not consistent with the constraints, and we need to query the IIS of the bounds of variables by calling `get_variable_attribute` with the `VariableAttribute.IISLowerBound` and `VariableAttribute.IISUpperBound` attributes.

The following code snippet shows how to tell if the bounds of a variable are in the IIS:

```{code-cell}
model = copt.Model()

x = model.add_variable(lb=0.0, ub=2.0, name="x")
y = model.add_variable(lb=0.0, ub=3.0, name="y")

con1 = model.add_linear_constraint(x + y, poi.Geq, 6.0)

model.set_objective(x)

model.computeIIS()

con1_iis = model.get_constraint_attribute(con1, poi.ConstraintAttribute.IIS)
x_lb_iis = model.get_variable_attribute(x, poi.VariableAttribute.IISLowerBound)
x_ub_iis = model.get_variable_attribute(x, poi.VariableAttribute.IISUpperBound)
y_lb_iis = model.get_variable_attribute(y, poi.VariableAttribute.IISLowerBound)
y_ub_iis = model.get_variable_attribute(y, poi.VariableAttribute.IISUpperBound)

print(f"Constraint 1 IIS: {con1_iis}")
print(f"Variable x lower bound IIS: {x_lb_iis}")
print(f"Variable x upper bound IIS: {x_ub_iis}")
print(f"Variable y lower bound IIS: {y_lb_iis}")
print(f"Variable y upper bound IIS: {y_ub_iis}")
```
