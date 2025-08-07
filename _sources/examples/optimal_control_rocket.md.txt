---
file_format: mystnb
kernelspec:
  name: python3
---
# Optimal Control of a Rocket

This example is adapted from [the JuMP tutorial](https://jump.dev/JuMP.jl/stable/tutorials/nonlinear/rocket_control/).

The goal is to show that there are explicit repeated structures in discretized optimal control problem regarded as a nonlinear program (NLP). We will use the optimal control of a rocket as an example to demonstrate how to exploit these structures to solve the problem more efficiently via PyOptInterface.

## Problem Formulation

The problem is to find the optimal control of a rocket to maximize the altitude at the final time while satisfying the dynamics of the rocket. The dynamics of the rocket are described by the following ordinary differential equations (ODEs):

$$
\begin{align*}
\frac{dh}{dt} &= v \\
\frac{dv}{dt} &= -g(h) + \frac{u-D(h,v)}{m} \\
\frac{dm}{dt} &= -\frac{u}{c}
\end{align*}
$$

where $h$ is the altitude, $v$ is the velocity, $m$ is the mass, $u$ is the thrust, $g(h)$ is the gravitational acceleration, and $D(h,v)$ is the drag force. The thrust $u$ is the control variable.

The drag force is given by $D(h,v) = D_c v^2 \exp(-h_c \frac{h-h_0}{h_0})$, and the gravitational acceleration is given by $g(h) = g_0 (\frac{h_0}{h})^2$.

By discretizing the ODEs, we obtain the following nonlinear program:

$$
\begin{align*}
\frac{h_{t+1}-h_t}{\Delta t} &= v_t \\
\frac{v_{t+1}-v_t}{\Delta t} &= -g(h_t) + \frac{u_t-D(h_t,v_t)}{m_t} \\
\frac{m_{t+1}-m_t}{\Delta t} &= -\frac{u_t}{c}
\end{align*}
$$

where $h_t$, $v_t$, and $m_t$ are the altitude, velocity, and mass at time $t$, respectively, and $\Delta t$ is the time step.

## Implementation

In the discretized optimal control problem, the variables at two adjacent time points share the same algebraic relationship.

```{code-cell}
import math
import pyoptinterface as poi
from pyoptinterface import nl, ipopt

model = ipopt.Model()

h_0 = 1.0
v_0 = 0.0
m_0 = 1.0
g_0 = 1.0
T_c = 3.5
h_c = 500.0
v_c = 620.0
m_c = 0.6

c = 0.5 * math.sqrt(g_0 * h_0)
m_f = m_c * m_0
D_c = 0.5 * v_c * (m_0 / g_0)
T_max = T_c * m_0 * g_0

nh = 1000
```

Then, we declare variables and set boundary conditions.

```{code-cell}
h = model.add_m_variables(nh, lb=1.0)
v = model.add_m_variables(nh, lb=0.0)
m = model.add_m_variables(nh, lb=m_f, ub=m_0)
T = model.add_m_variables(nh, lb=0.0, ub=T_max)
step = model.add_variable(lb=0.0)

# Boundary conditions
model.set_variable_bounds(h[0], h_0, h_0)
model.set_variable_bounds(v[0], v_0, v_0)
model.set_variable_bounds(m[0], m_0, m_0)
model.set_variable_bounds(m[-1], m_f, m_f)
```

Next, we add the dynamics constraints.

```{code-cell}
for i in range(nh - 1):
    with nl.graph():
        h1 = h[i]
        h2 = h[i + 1]
        v1 = v[i]
        v2 = v[i + 1]
        m1 = m[i]
        m2 = m[i + 1]
        T1 = T[i]
        T2 = T[i + 1]

        model.add_nl_constraint(h2 - h1 - 0.5 * step * (v1 + v2) == 0)

        D1 = D_c * v1 * v1 * nl.exp(-h_c * (h1 - h_0)) / h_0
        D2 = D_c * v2 * v2 * nl.exp(-h_c * (h2 - h_0)) / h_0
        g1 = g_0 * h_0 * h_0 / (h1 * h1)
        g2 = g_0 * h_0 * h_0 / (h2 * h2)
        dv1 = (T1 - D1) / m1 - g1
        dv2 = (T2 - D2) / m2 - g2

        model.add_nl_constraint(v2 - v1 - 0.5 * step * (dv1 + dv2) == 0)
        model.add_nl_constraint(m2 - m1 + 0.5 * step * (T1 + T2) / c == 0)
```

Finally, we add the objective function. We want to maximize the altitude at the final time, so we set the objective function to be the negative of the altitude at the final time.

```{code-cell}
model.set_objective(-h[-1])
```

After solving the problem, we can plot the results.

```{code-cell}
model.optimize()
```

```{code-cell}
h_value = []
for i in range(nh):
    h_value.append(model.get_value(h[i]))

print("Optimal altitude: ", h_value[-1])
```

The plot of the altitude of the rocket is shown below.

```{code-cell}
import matplotlib.pyplot as plt

plt.plot(h_value)
plt.xlabel("Time")
plt.ylabel("Altitude")
plt.show()
```
