---
file_format: mystnb
kernelspec:
  name: python3
---
# Optimal Power Flow

Alternating current optimal power flow (ACOPF) is a fundamental nonlinear optimization problem in power systems. It is used to determine the optimal power flow of a power system to minimize the generation cost while satisfying the power flow equations and system constraints.

In this example, we will show how to use PyOptInterface to solve an single-period optimal power flow problem using the structured nonlinear programming formulation.

## Problem Formulation

$$
\min \quad & \sum_{i \in G} a_i P_{i}^2 + b_i P_{i} + c_i \\
\textrm{s.t.}  \quad & \theta_r = 0 \quad & \forall r \in R \\
               \quad & P_{min,i} \leq P_{i} \leq P_{max,i} \quad & \forall i \in G \\
               \quad & Q_{min,i} \leq Q_{i} \leq Q_{max,i} \quad & \forall i \in G \\
               \quad & V_{min,b} \leq V_{b} \leq V_{max,b} \quad & \forall b \in BUS \\
               \quad & \sum_{i \in G_b} P_{i} - \sum_{i \in D_b} L^P_{i} - G_{sh,b} V_b^2 = \sum_{(i, j) \in BRANCH} P_{ij} \quad & \forall b \in BUS \\
               \quad & \sum_{i \in G_b} Q_{i} - \sum_{i \in D_b} L^Q_{i} + B_{sh,b} V_b^2 = \sum_{(i, j) \in BRANCH} Q_{ij} \quad & \forall b \in BUS \\
               \quad & P_{ij} =  G_{ij} V_i^2 - V_i V_j (G_{ij}\cos(\theta_i-\theta_j)+B_{ij}\sin(\theta_i-\theta_j)) \quad & \forall (i, j) \in BRANCH \\
               \quad & Q_{ij} = -B^C_{ij} V_i^2 - B_{ij} V_i^2 - V_i V_j (G_{ij}\sin(\theta_i-\theta_j)-B_{ij}\cos(\theta_i-\theta_j)) \quad & \forall (i, j) \in BRANCH \\
               \quad & P_{ji} =  G_{ij} V_j^2 - V_i V_j (G_{ij}\cos(\theta_j-\theta_i)+B_{ij}\sin(\theta_j-\theta_i)) \quad & \forall (i, j) \in BRANCH \\
               \quad & Q_{ji} = -B^C_{ij} V_j^2 - B_{ij} V_j^2 - V_i V_j (G_{ij}\sin(\theta_j-\theta_i)-B_{ij}\cos(\theta_j-\theta_i)) \quad & \forall (i, j) \in BRANCH \\
               \quad & -S_{max,ij} \leq P_{ij}^2 + Q_{ij}^2 \leq S_{max,ij} \quad & \forall (i, j) \in BRANCH \\
               \quad & -\Delta\theta_{min,ij} \leq \theta_i - \theta_j \leq -\Delta\theta_{max,ij} \quad & \forall (i, j) \in BRANCH
$$

The decision variables are the active power output of the generators $P_{i}$, reactive power output of the generators $Q_{i}$, voltage magnitude of the buses $V_{b}$, phase angle of the buses $\theta_{b}$ for $b \in BUS$ and the branch power flows $P_{ij}$ and $Q_{ij}$ for $(i, j) \in BRANCH$.

The objective function is the total cost of the generators, which is the sum of the quadratic cost, the linear cost, and the constant cost. The first constraint ensures that the phase angle of the reference bus is zero. The second and third constraints are the active and reactive power output limits of the generators. The fourth constraint is the voltage magnitude limits of the buses. The fifth and sixth constraints are the power balance equations of the buses. The seventh and eighth constraints are the power flow equations of the branches. The ninth and tenth constraints are the power flow equations of the branches in the opposite direction. The eleventh and twelfth constraints are the apparent power limits of the branches. The last constraint is the phase angle difference limits of the branches.

## Implementation

We notice that the branch power flow equations with respect to $P_{ij}$, $Q_{ij}$, $P_{ji}$ and $Q_{ji}$ are nonlinear equations and have the same structure for each branch. We will use a single function to model the nonlinear power flow equations of one branch.

```{code-cell}
import math
import pyoptinterface as poi
from pyoptinterface import nlfunc, ipopt

model = ipopt.Model()

def branch_flow(vars, params):
    G, B, Bc = params.G, params.B, params.Bc
    Vi, Vj, theta_i, theta_j, Pij, Qij, Pji, Qji = (
        vars.Vi,
        vars.Vj,
        vars.theta_i,
        vars.theta_j,
        vars.Pij,
        vars.Qij,
        vars.Pji,
        vars.Qji,
    )

    sin_ij = nlfunc.sin(theta_i - theta_j)
    cos_ij = nlfunc.cos(theta_i - theta_j)

    Pij_eq = G * Vi**2 - Vi * Vj * (G * cos_ij + B * sin_ij) - Pij
    Qij_eq = -(B + Bc) * Vi**2 - Vi * Vj * (G * sin_ij - B * cos_ij) - Qij
    Pji_eq = G * Vj**2 - Vi * Vj * (G * cos_ij - B * sin_ij) - Pji
    Qji_eq = -(B + Bc) * Vj**2 - Vi * Vj * (-G * sin_ij - B * cos_ij) - Qji

    return [Pij_eq, Qij_eq, Pji_eq, Qji_eq]

bf = model.register_function(branch_flow)
```

Here the nonlinear function takes two arguments: `vars` and `params`. Although the power flow constraints take the same form for each branch, the parameters `G` `B` and `Bc`, namely the $\pi$ circuit parameters of the branch, are different for each branch.

Next, we will use PJM 5-bus system as an example to demonstrate the implementation of the optimal power flow problem. The PJM 5-bus system is a small power system with 5 buses and 6 branches. The system data is shown below.

```{code-cell}

branches = [
    # (from, to, R, X, B, angmin, angmax)
    (0, 1, 0.00281, 0.0281, 0.00712, -30.0, 30.0),
    (0, 3, 0.00304, 0.0304, 0.00658, -30.0, 30.0),
    (0, 4, 0.00064, 0.0064, 0.03126, -30.0, 30.0),
    (1, 2, 0.00108, 0.0108, 0.01852, -30.0, 30.0),
    (2, 3, 0.00297, 0.0297, 0.00674, -30.0, 30.0),
    (3, 4, 0.00297, 0.0297, 0.00674, -30.0, 30.0),
]

buses = [
    # (Pd, Qd, Gs, Bs, Vmin, Vmax)
    (0.0, 0.0000, 0.0, 0.0, 0.9, 1.1),
    (3.0, 0.9861, 0.0, 0.0, 0.9, 1.1),
    (3.0, 0.9861, 0.0, 0.0, 0.9, 1.1),
    (4.0, 1.3147, 0.0, 0.0, 0.9, 1.1),
    (0.0, 0.0000, 0.0, 0.0, 0.9, 1.1),
]

generators = [
    # (bus, Pmin, Pmax, Qmin, Qmax, a, b, c)
    (0, 0.0, 0.4, -0.300, 0.300, 0.0, 1400, 0.0),
    (0, 0.0, 1.7, -1.275, 1.275, 0.0, 1500, 0.0),
    (2, 0.0, 5.2, -3.900, 3.900, 0.0, 3000, 0.0),
    (3, 0.0, 2.0, -1.500, 1.500, 0.0, 4000, 0.0),
    (4, 0.0, 6.0, -4.500, 4.500, 0.0, 1000, 0.0),
]

slack_bus = 3
```

Then we declare the variables:

```{code-cell}
N_branch = len(branches)
N_bus = len(buses)
N_gen = len(generators)

Pbr_from = model.add_variables(range(N_branch))
Qbr_from = model.add_variables(range(N_branch))
Pbr_to = model.add_variables(range(N_branch))
Qbr_to = model.add_variables(range(N_branch))

V = model.add_variables(range(N_bus), name="V")
theta = model.add_variables(range(N_bus), name="theta")

for i in range(N_bus):
    Vmin, Vmax = buses[i][4], buses[i][5]
    model.set_variable_bounds(V[i], Vmin, Vmax)

model.set_variable_bounds(theta[slack_bus], 0.0, 0.0)

P = model.add_variables(range(N_gen), name="P")
Q = model.add_variables(range(N_gen), name="Q")

for i in range(N_gen):
    model.set_variable_bounds(P[i], generators[i][1], generators[i][2])
    model.set_variable_bounds(Q[i], generators[i][3], generators[i][4])
```

Next, we add the constraints:

```{code-cell}
# nonlinear constraints
for k in range(N_branch):
    branch = branches[k]
    R, X, Bc2 = branch[2], branch[3], branch[4]

    G = R / (R**2 + X**2)
    B = -X / (R**2 + X**2)
    Bc = Bc2 / 2

    i = branch[0]
    j = branch[1]

    Vi = V[i]
    Vj = V[j]
    theta_i = theta[i]
    theta_j = theta[j]

    Pij = Pbr_from[k]
    Qij = Qbr_from[k]
    Pji = Pbr_to[k]
    Qji = Qbr_to[k]

    model.add_nl_constraint(
        bf,
        vars=nlfunc.Vars(
            Vi=Vi,
            Vj=Vj,
            theta_i=theta_i,
            theta_j=theta_j,
            Pij=Pij,
            Qij=Qij,
            Pji=Pji,
            Qji=Qji,
        ),
        params=nlfunc.Params(G=G, B=B, Bc=Bc),
        eq=0.0,
    )

# power balance constraints
P_balance_eq = [poi.ExprBuilder() for i in range(N_bus)]
Q_balance_eq = [poi.ExprBuilder() for i in range(N_bus)]

for b in range(N_bus):
    Pd, Qd = buses[b][0], buses[b][1]
    Gs, Bs = buses[b][2], buses[b][3]
    Vb = V[b]

    P_balance_eq[b] -= poi.quicksum(
        Pbr_from[k] for k in range(N_branch) if branches[k][0] == b
    )
    P_balance_eq[b] -= poi.quicksum(
        Pbr_to[k] for k in range(N_branch) if branches[k][1] == b
    )
    P_balance_eq[b] += poi.quicksum(P[i] for i in range(N_gen) if generators[i][0] == b)
    P_balance_eq[b] -= Pd
    P_balance_eq[b] -= Gs * Vb * Vb

    Q_balance_eq[b] -= poi.quicksum(
        Qbr_from[k] for k in range(N_branch) if branches[k][0] == b
    )
    Q_balance_eq[b] -= poi.quicksum(
        Qbr_to[k] for k in range(N_branch) if branches[k][1] == b
    )
    Q_balance_eq[b] += poi.quicksum(Q[i] for i in range(N_gen) if generators[i][0] == b)
    Q_balance_eq[b] -= Qd
    Q_balance_eq[b] += Bs * Vb * Vb

    model.add_quadratic_constraint(P_balance_eq[b], poi.Eq, 0.0)
    model.add_quadratic_constraint(Q_balance_eq[b], poi.Eq, 0.0)

for k in range(N_branch):
    branch = branches[k]

    i = branch[0]
    j = branch[1]

    theta_i = theta[i]
    theta_j = theta[j]

    angmin = branch[5] / 180 *  math.pi
    angmax = branch[6] / 180 * math.pi

    model.add_linear_constraint(theta_i - theta_j, poi.In, angmin, angmax)
```

Finally, we set the objective function:

```{code-cell}
cost = poi.ExprBuilder()
for i in range(N_gen):
    a, b, c = generators[i][5], generators[i][6], generators[i][7]
    cost += a * P[i] * P[i] + b * P[i] + c
model.set_objective(cost)
```

After optimization, we can retrieve the optimal solution:

```{code-cell}
model.optimize()
```

```{code-cell}
print(model.get_model_attribute(poi.ModelAttribute.TerminationStatus))

P_value = P.map(model.get_value)

print("Optimal active power output of the generators:")

for i in range(N_gen):
    print(f"Generator {i}: {P_value[i]}")
```

As shown by the result, the generator with less cost efficient has the highest active power output. The total generation power is also greater than the total demand due to the network loss.