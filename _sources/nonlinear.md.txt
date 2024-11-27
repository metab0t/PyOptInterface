---
file_format: mystnb
kernelspec:
  name: python3
---
# Nonlinear Programming

## Introduction

Compared with the linear and quadratic expressions and objectives we have discussed in the previous sections, nonlinear programming is more general and can handle a wider range of problems. In this section, we will introduce how to use PyOptInterface to formulate and solve structured nonlinear programming problems.

:::{note}

Before trying out the code snippets, please ensure that you have completed the installation of PyOptInterface with correct dependencies via `pip install pyoptinterface[nlp]` and nonlinear solver (IPOPT) as described in the [Getting Started](getting_started.md#ipopt) section.
:::

## Structures in nonlinear programming

Although nonlinear programming allows arbitrary nonlinear functions to appear in constraints and objective functions, most nonlinear terms in practical problems can be categorized into a few common structures.

Generally, a nonlinear function mapping from $\mathbb{R}^n$ to $\mathbb{R}^m$ can be represented as a parameterized function $f(x, p)$, where $x \in \mathbb{R}^n$ is the input variable and $p \in \mathbb{R}^k$ is a set of parameters. The same function can be applied to multiple groups of variables and parameters.

In PyOptInterface, a nonlinear function can be declared as a normal Python function that accepts two variables: `vars` and `params`. The `vars` variable is a "magic" object that maps variable names to their values, and the `params` variable is another "magic" object that maps parameter names to their values. The function should return a dictionary that maps output names to their values. Of course, you can omit the `params` variable if the function does not have any parameters.

To make things easier, we start from a very simple bivariate function $f(x, y, p) = p * exp(x) * y^2$. The function has two variables $x$ and $y$, and one parameter $p$. The function can be implemented as follows:

```{code-cell}
from pyoptinterface import nlfunc, ipopt

model = ipopt.Model()

def f(vars, params):
    p = params.p
    result = p * nlfunc.exp(vars.x)
    result *= vars.y ** 2
    return result

reg_f = model.register_function(f)
```

In the code snippet above, we first import the `nlfunc` module, which contains a set of nonlinear functions that can be used in PyOptInterface. Then, we create an `ipopt.Model` object to represent the optimization problem.

Next, we define the function `f` that accepts two arguments: `vars` and `params`. In the function, we can extract the values of variables and parameters by accessing the corresponding attributes of the `vars` and `params` objects. The `nlfunc.exp` function is used to calculate the exponential function. Finally, we return the result.

Finally, we register the function `f` by calling the `register_function` method of the `model` object. The `register_function` method returns an abstract `FunctionIndex` object that can be used in constraints and objectives.

The magic happens in the `register_function` where we use function tracing to capture the variables and parameters used in the function and the whole computational graph of the function. This allows PyOptInterface to automatically differentiate the function and calculate the gradients and Hessians of the function.

It is worth mentioning that `f` is also a simple Python function that can be called directly. For example, we can evaluate the function at a specific point by calling `f` with the corresponding values of variables and parameters:

```{code-cell}
import types, math

vars = types.SimpleNamespace(x=1.0, y=2.0)
params = types.SimpleNamespace(p=3.0)

result = f(vars, params)

assert result == 3.0 * math.exp(1.0) * 2.0 ** 2
```

We also provide `nlfunc.Vars` and `nlfunc.Params` as a convenient way to create a group of variables and parameters. In fact, they are wrappers of `types.SimpleNamespace`:

```{code-cell}
vars = nlfunc.Vars(x=1.0, y=2.0)
params = nlfunc.Params(p=3.0)

result = f(vars, params)

assert result == 3.0 * math.exp(1.0) * 2.0 ** 2
```

If there is no parameter in the function, we can omit the `params` argument in the function definition. For example, the following code snippet defines a function $g(x, y) = x^4 + y^4$:

```{code-cell}
def g(vars):
    return vars.x ** 4 + vars.y ** 4

reg_g = model.register_function(g)
```

In the code snippet above, we define a function `g` that accepts only one argument `vars`. The function calculates the sum of the fourth powers of the two variables $x$ and $y$. We register the function `g` in the same way as we did for the function `f`.

The result of a nonlinear function is not restricted to a scalar value. It can be a scalar or a vector by returning a Python list containing the results. For example, the following code snippet defines a function $h(x, y) = [cos(x + y^2), sin(x^2 - y)]$ mapping from $\mathbb{R}^2$ to $\mathbb{R}^2$: 

```{code-cell}
def h(vars):
    z1 = nlfunc.cos(vars.x + vars.y ** 2)
    z2 = nlfunc.sin(vars.x ** 2 - vars.y)
    return [z1, z2]

reg_h = model.register_function(h)
```

PyOptInterface currently supports the following nonlinear operators:

:::{list-table} **Unary functions**
:header-rows: 1

*   - Operator
    - Example
*   - `-` (negation)
    - `y = -x`
*   - sin
    - `y = nlfunc.sin(x)`
*   - cos
    - `y = nlfunc.cos(x)`
*   - tan
    - `y = nlfunc.tan(x)`
*   - asin
    - `y = nlfunc.asin(x)`
*   - acos
    - `y = nlfunc.acos(x)`
*   - atan
    - `y = nlfunc.atan(x)`
*   - abs
    - `y = nlfunc.abs(x)`
*   - sqrt
    - `y = nlfunc.sqrt(x)`
*   - exp
    - `y = nlfunc.exp(x)`
*   - log
    - `y = nlfunc.log(x)`
*   - log10
    - `y = nlfunc.log10(x)`
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
    - `z = nlfunc.pow(x, y)`
:::

We can also use `nlfunc.ifelse` to implement the conditional operator. For example, the following code snippet defines a absolute value function $f(x) = |x|$:

```{code-cell}
def f(vars):
    return nlfunc.ifelse(vars.x >= 0, vars.x, -vars.x)
```

`nlfunc.ifelse` accepts three arguments: a condition, a value when the condition is true, and a value when the condition is false. The function returns the value of the second argument if the condition is true; otherwise, it returns the value of the third argument.

The condition variable must be a boolean variable, which can be obtained by comparing two variables using the comparison operators `==`, `!=`, `<`, `<=`, `>`, and `>=`.

## Nonlinear constraints and objectives

After defining and registering nonlinear functions, we can use them in constraints and objectives. For example, the following code snippet defines a nonlinear programming problem.

```{code-cell}
def obj(vars):
    return nlfunc.exp(vars.x) + nlfunc.exp(vars.y)

def con(vars):
    x, y = vars.x, vars.y
    return [x ** 4 + y ** 4, - x * y]

obj_f = model.register_function(obj)
con_f = model.register_function(con)

x = model.add_variable(lb = 0.0)
y = model.add_variable(lb = 0.0)

model.add_nl_constraint(con_f, nlfunc.Vars(x=x,y=y), ub = [4.0, -1.0])
model.add_nl_objective(obj_f, nlfunc.Vars(x=x,y=y))

model.optimize()

x_value = model.get_value(x)
y_value = model.get_value(y)

print(f"x = {x_value}, y = {y_value}")
```

Nonlinear constraint can be declared by calling `add_nl_constraint` method of the `Model` object. The method accepts the registered function, the variables/parameters of the constraint, and the bounds of the constraint.

The variables of the constraint should be constructed by `nlfunc.Vars` and the parameters should be constructed by `nlfunc.Params`.

The bounds can be specified as a scalar or a list with the same length as the number of outputs of the function. If it is a scalar, the same bound will be applied to all outputs. If it is a list, each output will have its own bound.

You can declare `eq` for equality constraints, `lb` for lower bounds, and `ub` for upper bounds. `lb` and `ub` can be declared simultaneously to create a bounded constraint.

```{py:function} model.add_nl_constraint(registered_function, vars, [params=None, eq=None, lb=None, ub=None, name=""])

add a nonlinear constraint to the model

:param registered_function: the `FunctionIndex` returned by `register_function`
:param vars: the variables of the constraint, must be constructed by `nlfunc.Vars`
:param params: the parameters of the constraint, must be constructed by `nlfunc.Params`, optional
:param eq: the equality value of the constraint, optional
:param lb: the lower bound of the constraint, optional
:param ub: the upper bound of the constraint, optional
:param str name: the name of the constraint, optional
:return: the handle of the constraint
```

Nonlinear objective can be declared by calling `add_nl_objective` method of the `Model` object. The method accepts the registered function, the variables/parameters of the objective, and the parameters of the objective.

It is noteworthy that `add_nl_objective` only adds a nonlinear term to the objective and can be called multiple times to construct a sum of nonlinear terms as objective.

The nonlinear function in `add_nl_objective` should return a scalar value and PyOptInterface will throw an error if the function returns multiple values.

```{py:function} model.add_nl_objective(registered_function, vars, [params=None, name=""])

add a nonlinear objective term to the model

:param registered_function: the `FunctionIndex` returned by `register_function`
:param vars: the variables of the objective, must be constructed by `nlfunc.Vars`
:param params: the parameters of the objective, must be constructed by `nlfunc.Params`, optional
:param str name: the name of the objective, optional
```

We will use the well-known [Rosenbrock function](https://jump.dev/JuMP.jl/stable/tutorials/nonlinear/simple_examples/#The-Rosenbrock-function) as another example:

```{code-cell}
from pyoptinterface import nlfunc, ipopt

model = ipopt.Model()

x = model.add_variable()
y = model.add_variable()

def rosenbrock(vars):
    x, y = vars.x, vars.y
    return (1 - x) ** 2 + 100 * (y - x ** 2) ** 2

rosenbrock_f = model.register_function(rosenbrock)

model.add_nl_objective(rosenbrock_f, nlfunc.Vars(x=x, y=y))

model.optimize()

x_value = model.get_value(x)
y_value = model.get_value(y)

print(f"x = {x_value}, y = {y_value}")
```

## Mixing nonlinear and linear/quadratic constraints together

Introducing nonlinear functions does not prevent you from using the `add_linear_constraint`, `add_quadratic_constraint` methods. You can add both nonlinear constraints and linear/quadratic constraints in the same optimization problem.

We will use the [clnlbeam problem](https://jump.dev/JuMP.jl/stable/tutorials/nonlinear/simple_examples/#The-clnlbeam-problem) as example:

```{code-cell}
from pyoptinterface import nlfunc, ipopt
import pyoptinterface as poi

model = ipopt.Model()

N = 1000
h = 1 / N
alpha = 350

t = model.add_variables(range(N+1), lb=-1.0, ub=1.0)
x = model.add_variables(range(N+1), lb=-0.05, ub=0.05)
u = model.add_variables(range(N+1))

def obj(vars):
    return 0.5 * h * (vars.u2**2 + vars.u1**2) + 0.5 * alpha * h * (nlfunc.cos(vars.t2) + nlfunc.cos(vars.t1))

obj_f = model.register_function(obj)
for i in range(N):
    model.add_nl_objective(obj_f, nlfunc.Vars(t1=t[i], t2=t[i+1], u1=u[i], u2=u[i+1]))

def con(vars):
    return vars.x2 - vars.x1 - 0.5 * h * (nlfunc.sin(vars.t2) + nlfunc.sin(vars.t1))

con_f = model.register_function(con)
for i in range(N):
    model.add_nl_constraint(con_f, nlfunc.Vars(t1=t[i], t2=t[i+1], x1=x[i], x2=x[i+1]), eq=0.0)

for i in range(N):
    model.add_linear_constraint(t[i+1] - t[i] - 0.5 * h * u[i+1] - 0.5 * h * u[i] , poi.Eq, 0.0)

model.optimize()

objective_value = model.get_model_attribute(poi.ModelAttribute.ObjectiveValue)

print(f"Objective value: {objective_value}")
```

## More complex examples

In practice, the nonlinear constraints may have the same structure but with different parameters, we encourage you to read our [optimal power flow](examples/optimal_power_flow.md) and [optimal control](examples/optimal_control_rocket.md) examples to learn how to construct more complex nonlinear programming problems.
