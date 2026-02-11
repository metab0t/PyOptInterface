---
file_format: mystnb
kernelspec:
  name: python3
---
# Nonlinear Programming

## Introduction

Compared with the linear and quadratic expressions and objectives we have discussed in the previous sections, nonlinear programming is more general and can handle a wider range of problems. In this section, we will introduce how to use PyOptInterface to formulate and solve general nonlinear programming problems.

:::{note}

Before trying out the code snippets, please ensure that you have completed the installation of PyOptInterface with correct dependencies via `pip install pyoptinterface[nlp]` and solvers that support nonlinear programming (IPOPT, COPT, Xpress, Gurobi, KNITRO) as described in the [Getting Started](getting_started.md) section.
:::

## Construct nonlinear expressions

Nonlinear expressions are more complex than linear and quadratic expressions, and they can include various nonlinear functions such as trigonometric functions, exponential functions, logarithmic functions, etc. In PyOptInterface, we must declare a `nl.graph()` context to construct nonlinear expressions. The `nl.graph()` context is used to trace the computational graph of the nonlinear expression, which allows PyOptInterface to automatically differentiate the expression and calculate the gradients and Hessians.

```{code-cell}
import pyoptinterface as poi
from pyoptinterface import nl, ipopt

model = ipopt.Model()

x = model.add_variable(name="x")
y = model.add_variable(name="y")

with nl.graph():
    z = nl.exp(x) * nl.pow(y, 2)
    model.add_nl_constraint(z <= 10.0)
```

In the code snippet above, we first import the `nl` module, which contains the nonlinear programming utilities including a set of nonlinear functions that can be used in PyOptInterface. Then, we create an `ipopt.Model` object to represent the optimization problem.

We then add two variables `x` and `y` to the model. After that, we enter the `nl.graph()` context, where we can construct nonlinear expressions using the functions provided by the `nl` module. In this case, we define a nonlinear expression `z = exp(x) * pow(y, 2)` and add a nonlinear constraint `z <= 10.0` to the model.

In the `nl.graph()` context, we can use various nonlinear functions provided by the `nl` module to construct nonlinear expressions. The functions are designed to be similar to the mathematical notation, making it easy to read and write nonlinear expressions.

PyOptInterface currently supports the following nonlinear operators:

:::{list-table} **Unary functions**
:header-rows: 1

*   - Operator
    - Example
*   - `-` (negation)
    - `y = -x`
*   - sin
    - `y = nl.sin(x)`
*   - cos
    - `y = nl.cos(x)`
*   - tan
    - `y = nl.tan(x)`
*   - asin
    - `y = nl.asin(x)`
*   - acos
    - `y = nl.acos(x)`
*   - atan
    - `y = nl.atan(x)`
*   - abs
    - `y = nl.abs(x)`
*   - sqrt
    - `y = nl.sqrt(x)`
*   - exp
    - `y = nl.exp(x)`
*   - log
    - `y = nl.log(x)`
*   - log10
    - `y = nl.log10(x)`
:::

:::{list-table} **Binary functions**
:header-rows: 1

*   - Operator
    - Example
*   - `+`
    - `z = x + y`
*   - `-`
    - `z = x - y`
*   - `*`
    - `z = x * y`
*   - `/`
    - `z = x / y`
*   - `**`
    - `z = x ** y`
*   - pow
    - `z = nl.pow(x, y)`
:::

We can also use `nl.ifelse` to implement the conditional operator. For example, the following code snippet defines a absolute value function $f(x) = |x|$:

```{code-cell}
def f(x):
    return nl.ifelse(x >= 0, x, -x)

with nl.graph():
    y = f(x)
    model.add_nl_constraint(y <= 2)
```

`nl.ifelse` accepts three arguments: a condition, a value when the condition is true, and a value when the condition is false. The function returns the value of the second argument if the condition is true; otherwise, it returns the value of the third argument. The condition variable must be a boolean variable, which can be obtained by comparing two variables using the comparison operators `==`, `!=`, `<`, `<=`, `>`, and `>=`.

Another interesting fact is that you can construct nonlinear expressions in the context of `nl.graph` that are prohibited outside the context. For example, you can construct $(x+2)^3$ in the following way:

```{code-cell}
with nl.graph():
    z = (x + 2) ** 3

# Illegal outside nl.graph context because it is a nonlinear expression
# z = (x + 2) ** 3  # This will raise an error
```

## Nonlinear constraints and objectives

After constructing nonlinear expressions, we can use them in constraints and objectives. For example, the following code snippet defines a nonlinear programming problem.

```{code-cell}
model = ipopt.Model()
x = model.add_variable(lb = 0.0)
y = model.add_variable(lb = 0.0)

with nl.graph():
    model.add_nl_constraint(x ** 4 + y ** 4 <= 4.0)
    model.add_nl_constraint(x * y >= 1.0)

    model.add_nl_objective(nl.exp(x) + nl.exp(y))

model.optimize()
```

```{code-cell}
x_value = model.get_value(x)
y_value = model.get_value(y)

print(f"x = {x_value}, y = {y_value}")
```

Nonlinear constraint can be declared by calling `add_nl_constraint` method. Like the linear and quadratic constraints, you can specify the sense/right-hand-side of constraint, use an interval of values to represent a two-sided constraint, or use a comparison operator like `<=`, `==`, or `>=` to create the constraint.

```{code-cell}
# One-sided nonlinear constraint
with nl.graph():
    model.add_nl_constraint(x ** 2 + y ** 2 <= 1.0)
    # equivalent to
    model.add_nl_constraint(x ** 2 + y ** 2, poi.Leq, 1.0)

# Two-sided nonlinear constraint
with nl.graph():
    model.add_nl_constraint(x ** 2 + y ** 2, (1.0, 2.0))
    # equivalent to
    model.add_nl_constraint(x ** 2 + y ** 2, poi.Leq, 2.0)
    model.add_nl_constraint(x ** 2 + y ** 2, poi.Geq, 1.0)
```

Similarly, the nonlinear objective can be declared by calling `add_nl_objective` method. It is noteworthy that `add_nl_objective` only adds a nonlinear term to the objective and can be called multiple times to construct a sum of nonlinear terms as objective.

```{code-cell}
# Nonlinear objective
with nl.graph():
    model.add_nl_objective(nl.sin(x) + nl.cos(y))
```

Because the nonlinear expressions are captured by the current `nl.graph()` context, both `add_nl_constraint` and `add_nl_objective` methods must be called within the same `nl.graph()` context. If you try to call them outside the context, it will raise an error.

Finally, we will use the well-known [Rosenbrock function](https://jump.dev/JuMP.jl/stable/tutorials/nonlinear/simple_examples/#The-Rosenbrock-function) as another example:

```{code-cell}
from pyoptinterface import nl, ipopt

model = ipopt.Model()

x = model.add_variable()
y = model.add_variable()

with nl.graph():
    model.add_nl_objective((1 - x) ** 2 + 100 * (y - x ** 2) ** 2)

model.optimize()
```

```{code-cell}
x_value = model.get_value(x)
y_value = model.get_value(y)

print(f"x = {x_value}, y = {y_value}")
```

## Mixing nonlinear and linear/quadratic constraints together

Introducing nonlinear functions does not prevent you from using the `add_linear_constraint`, `add_quadratic_constraint` methods. You can add both nonlinear constraints and linear/quadratic constraints in the same optimization problem.

We will use the [clnlbeam problem](https://jump.dev/JuMP.jl/stable/tutorials/nonlinear/simple_examples/#The-clnlbeam-problem) as example:

```{code-cell}
from pyoptinterface import nl, ipopt
import pyoptinterface as poi

model = ipopt.Model()

N = 1000
h = 1 / N
alpha = 350

t = model.add_m_variables(N + 1, lb=-1.0, ub=1.0)
x = model.add_m_variables(N + 1, lb=-0.05, ub=0.05)
u = model.add_m_variables(N + 1)

for i in range(N):
    with nl.graph():
        model.add_nl_objective(
            0.5 * h * (u[i] * u[i] + u[i + 1] * u[i + 1])
            + 0.5 * alpha * h * (nl.cos(t[i]) + nl.cos(t[i + 1]))
        )
        model.add_nl_constraint(
            x[i + 1] - x[i] - 0.5 * h * (nl.sin(t[i]) + nl.sin(t[i + 1])) == 0.0
        )
        model.add_linear_constraint(
            t[i + 1] - t[i] - 0.5 * h * u[i + 1] - 0.5 * h * u[i] == 0.0
        )

model.optimize()
```

```{code-cell}
objective_value = model.get_model_attribute(poi.ModelAttribute.ObjectiveValue)

print(f"Objective value: {objective_value}")
```

## How to use `nl.graph` to capture the similar structure

You might be wondering how to place the `nl.graph()` context manager correctly in your code. The key is to ensure that all nonlinear constraints and objectives that share the same structure are enclosed within the same `nl.graph()` context.

As a rule of thumb, `nl.graph` should always be used inside the `for` loop instead of outside, so that each graph will have the same pattern and PyOptInterface can recognize the similar structures to accelerate the automatic differentiation process. This is particularly important when you have a large number of nonlinear constraints or objectives that share the same structure, as it can significantly improve the performance of the optimization process as discussed in our research paper [Accelerating Optimal Power Flow with Structure-aware Automatic Differentiation and Code Generation](https://ieeexplore.ieee.org/document/10721402).

In the clnlbeam example above, we have placed the `nl.graph()` context inside the `for` loop, which allows us to capture the structure of the nonlinear constraints and objectives for each iteration.

## More complex examples

In practice, the nonlinear constraints may have the same structure but with different parameters, we encourage you to read our [optimal power flow](examples/optimal_power_flow.md) and [optimal control](examples/optimal_control_rocket.md) examples to learn how to construct more complex nonlinear programming problems.
