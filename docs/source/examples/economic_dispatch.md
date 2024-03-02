---
file_format: mystnb
kernelspec:
  name: python3
---
# Economic dispatch

Economic dispatch is a classic optimization problem in power systems. It is used to determine the optimal power output of a set of generators to meet the demand at the lowest cost. The problem can be formulated as a linear programming problem or a quadratic programming problem depending on the formulation used to model the economic cost.

In this example, we will show how to use PyOptInterface to solve an economic dispatch problem using the quadratic programming formulation.

## Problem Formulation

$$
\min \quad & \sum_{t=1}^T \sum_{i=1}^{N_G} a_i P_{i,t}^2 + b_i P_{i,t} + c_i \\
\textrm{s.t.}  \quad & \sum_{i=1}^{N_G} P_{i,t} = \sum_{i=1}^{N_D} D_{i,t}\\
               \quad & P_{min,i} \leq P_{i,t} \leq P_{max,i} \\
               \quad & -Rdn_{i} \leq P_{i,t} - P_{i,t-1} \leq Rup_{i}
$$

We consider a system with $N_G$ generators and $N_D$ demands. The decision variables are the power output of the generators $P_{i,t}$ for $i=1,\ldots,N_G$ and $t=1,\ldots,T$. The objective function is the total cost of the generators, which is the sum of the quadratic cost, the linear cost, and the constant cost. The first constraint ensures that the total power output of the generators meets the demand. The second and third constraints are the power output limits of the generators. The last constraint is the ramping limits of the generators.

## Implementation

Firstly, we need to create a model object. We will use the HiGHS solver in this example because it is an excellent open-source solver and supports quadratic programming problems. You need to read the [installation guide](../highs.md#initial-setup) to install the HiGHS solver manually and set the path to shared library of HiGHS as described in the guide.

```{code-cell}
import pyoptinterface as poi
from pyoptinterface import highs

model = highs.Model()
```

Then, we need to create new variables in the model to represent the power output of the generators. We will use the [`add_variables`](#model.add_variables) method to add multidimensional variables to the model.

```{code-cell}
T = 24
N_G = 3
P_min = [50, 50, 50]
P_max = [100, 100, 100]
a_cost = [0.01, 0.015, 0.02]
b_cost = [1, 1.5, 2]
c_cost = [0, 0, 0]
Rup = [20, 20, 20]
Rdn = [20, 20, 20]

P = model.add_variables(range(N_G), range(T), name="P")
```

For the lower and upper bound of variables, we can set the corresponding [variable attribute](#pyoptinterface.VariableAttribute) to set them.

```{code-cell}
for i in range(N_G):
    for t in range(T):
        model.set_variable_attribute(P[i, t], poi.VariableAttribute.LowerBound, P_min[i])
        model.set_variable_attribute(P[i, t], poi.VariableAttribute.UpperBound, P_max[i])
```

Next, we need to add the power balance constraint to the model. We will use the [`add_linear_constraint`](#model.add_linear_constraint) method to add a linear constraint to the model. The multidimensional constraint is managed by `tupledict` and we use the `make_tupledict` method to create a multidimensional constraint.

The total demand is assumed to be a constant in this example.

```{code-cell}
total_demand = [220 for _ in range(T)]

def con(t):
    lhs = poi.quicksum(P[i, t] for i in range(N_G))
    rhs = total_demand[t]
    return model.add_linear_constraint(lhs, poi.Eq, rhs, name=f"powerbalance_{t}")

powebalance_constraints = poi.make_tupledict(range(T), rule=con)
```

```{code-cell}
def rampup_con(i, t):
    if t == 0:
        return None
    lhs = P[i, t] - P[i, t-1]
    rhs = Rup[i]
    return model.add_linear_constraint(lhs, poi.Leq, rhs, name=f"rampup_{i}_{t}")

rampup_constraints = poi.make_tupledict(range(N_G), range(T), rule=rampup_con)

def rampdown_con(i, t):
    if t == 0:
        return None
    lhs = P[i, t] - P[i, t-1]
    rhs = -Rdn[i]
    return model.add_linear_constraint(lhs, poi.Geq, rhs, name=f"rampdown_{i}_{t}")

rampdown_constraints = poi.make_tupledict(range(N_G), range(T), rule=rampdown_con)
```

Then, we need to add the quadratic objective function to the model. We will use the [`set_objective`](#model.set_objective) method to set the objective function of the model.

```{code-cell}
obj = poi.ExprBuilder()
for t in range(T):
    for i in range(N_G):
        obj += a_cost[i] * P[i, t] * P[i, t] + b_cost[i] * P[i, t] + c_cost[i]
model.set_objective(obj)
```

Finally, we can solve the model and query the solution.

```{code-cell}
model.optimize()

print(model.get_model_attribute(poi.ModelAttribute.TerminationStatus))
print("Objective value: ", model.get_value(obj))
```

The optimal value of decision variables can be queried via `get_value` function.

```{code-cell}
import numpy as np

P_value = np.fromiter(
    (model.get_value(P[i, t]) for i in range(N_G) for t in range(T)), float
).reshape(N_G, T)

P_value
```

## Change the load and solve the model again

We can change the load and solve the model again without creating a new model from scratch by modifying the right-hand side of the power balance constraint.

For example, we increase the load and solve the model again.

```{code-cell}
total_demand = [220 + 1.0 * t for t in range(T)]

for t in range(T):
    model.set_normalized_rhs(powebalance_constraints[t], total_demand[t])

model.optimize()
print(model.get_model_attribute(poi.ModelAttribute.TerminationStatus))
print("Objective value: ", model.get_value(obj))

P_value = np.fromiter(
    (model.get_value(P[i, t]) for i in range(N_G) for t in range(T)), float
).reshape(N_G, T)

print(P_value)
```