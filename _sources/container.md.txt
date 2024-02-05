# Container

In our previous examples, we only use scalar variable, constraint and expression. However, in many cases, we need to handle a large number of variables, constraints and expressions.

In general, PyOptInterface does not restrict the ways to store these objects. You can use a list, a dictionary, or any other data structure to store them, because PyOptInterface only requires the handle of the object to manipulate it and each handle is a compact C++ object that can be easily stored and passed around. In other words, we follow the Bring Your Own Container (BYOC) principle.

However, in the context of optimization, we often needs to represent the model in a more structured way. The most classic example is [`Set`](https://ampl.com/wp-content/uploads/Chapter-5-Simple-Sets-and-Indexing-AMPL-Book.pdf) in AMPL, which provides a flexible way to represent multi-dimensional data with custom indexing. The concept is also widely used in other optimization modeling languages, such as [JuMP](https://jump.dev/JuMP.jl/stable/manual/containers/) in Julia and [Pyomo](https://pyomo.readthedocs.io/en/stable/pyomo_modeling_components/Sets.html) in Python.

In PyOptInterface, we provide a simple container named `tupledict` to represent multidimensional data. It is a dictionary-like object that can be indexed by multiple keys. It is a convenient way to store and manipulate multi-dimensional data in PyOptInterface. The design and usage of `tupledict` is inspired by the `tupledict` in [gurobipy](https://www.gurobi.com/documentation/current/refman/py_tupledict.html).

We will use `tupledict` to demonstrate how to use a container to store and manipulate multi-dimensional data in PyOptInterface.

Simply speaking, `tupledict` is a derived class of Python dict where the keys represent multidimensional indices as n-tuple, and the values are typically variables, constraints or expressions.

## Create a `tupledict`
`tuplelist` can be created by calling the `tupledict` constructor. The following example creates a `tupledict` with two keys:

```python
import pyoptinterface as poi

td = poi.tupledict()
td[1, 2] = 3
td[4, 5] = 6
```

It can also be created by passing a dictionary to the constructor:

```python
td = poi.tupledict({(1, 2): 3, (4, 5): 6})
```

In most cases, we have multiple indices as the axis and define a rule to construct the `tupledict`. `make_tupledict` provide a convenient way to create a `tupledict` from a list of indices and a function that maps the indices to the values. The following example creates a `tupledict` with two keys:

```python
I = range(3)
J = [6, 7]
K = ("Asia", "Europe")

def f(i, j, k):
    return f"value_{i}_{j}_{k}"

td = poi.make_tupledict(I, J, K, rule=f)

"""
{(0, 6, 'Asia'): 'value_0_6_Asia',
 (0, 6, 'Europe'): 'value_0_6_Europe',
 (0, 7, 'Asia'): 'value_0_7_Asia',
 (0, 7, 'Europe'): 'value_0_7_Europe',
 (1, 6, 'Asia'): 'value_1_6_Asia',
 (1, 6, 'Europe'): 'value_1_6_Europe',
 (1, 7, 'Asia'): 'value_1_7_Asia',
 (1, 7, 'Europe'): 'value_1_7_Europe',
 (2, 6, 'Asia'): 'value_2_6_Asia',
 (2, 6, 'Europe'): 'value_2_6_Europe',
 (2, 7, 'Asia'): 'value_2_7_Asia',
 (2, 7, 'Europe'): 'value_2_7_Europe'}
"""
```

Sometimes we need to create a `tupledict` with a sparse pattern where some combinations of indices are missing. `make_tupledict` also provides a convenient way to create a `tupledict` with a sparse pattern, you only need to return `None` when the coprresponding value is unwanted. The following example creates a `tupledict` with a sparse pattern:

```python
I = range(3)
J = [6, 7]
K = ("Asia", "Europe")

def f(i, j, k):
    if i == 0 and j == 6 and k == "Asia":
        return "value_0_6_Asia"
    else:
        return None

td = poi.make_tupledict(I, J, K, rule=f)
    
"""
{(0, 6, 'Asia'): 'value_0_6_Asia'}
"""
```

For highly sparse patterns, you can provide the sparse indices directly to make it more efficient.

```
I = range(2)
J = range(100)
K = range(100)

# We only want to construct (i, j, k) where j=k
JK = [(j, j) for j in J]

def f(i, j, k):
    return f"value_{i}_{j}_{k}"

# JK will be flattened as j and k during construction
td = poi.make_tupledict(I, JK, rule=f)

"""
{(0, 0, 0): 'value_0_0_0',
 (0, 1, 1): 'value_0_1_1',
 (0, 2, 2): 'value_0_2_2',
 ...
 (1, 97, 97): 'value_1_97_97',
 (1, 98, 98): 'value_1_98_98',
 (1, 99, 99): 'value_1_99_99'}
"""
```

## Set/get values
Like normal dictionary in Python, the values of a `tupledict` can be set or get by using the `[]` operator. The following example sets and gets the values of a `tupledict`:

```python
td = poi.tupledict({(1, 2): 3, (4, 5): 6})
td[1, 2] = 4
print(td[1, 2]) # 4

td[4, 8] = 7
print(td[4, 8]) # 7
```

As a representation of multidimensional data, `tupledict` also provides a way to iterate some axis efficiently.
`tupledict.select` can be used to select a subset of the `tupledict` by fixing some indices. The following example selects a subset of the `tupledict`: 

```python

td = poi.make_tupledict(range(3), range(3), range(3), rule=lambda i, j, k: f"value_{i}_{j}_{k}")

# Select the subset where i=1
# "*" means wildcard that matches any value for j and k
# select returns a generator and can be converted to a list
subset_values_iterator = td.select(1, "*", "*")
subset_values = list(subset_values_iterator)

# Select the subset where j=2 and k=1
subset_values = list(td.select("*", 2, 1))

# the iterator can retuen the (key, value) pair if we pass with_key=True to select
subset_kv_iterator = td.select(1, "*", "*", with_key=True)
```

## Building a model with `tupledict`
`tupledict` can be used to store and manipulate multi-dimensional variables, constraints and expressions. Using it correctly will make building model more easily.

The following example demonstrates how to use `tupledict` to build a model:

$$

\min \quad & \sum_{i=0}^{I} \sum_{j=0}^{J} x_{ij}^2 \\
\textrm{s.t.}  \quad & \sum_{i=0}^{I} x_{ij} = 1, \quad \forall j \\
                \quad & x_{ij} \geq 0, \quad \forall i, j

$$

```python
import pyoptinterface as poi
from pyoptinterface import gurobi

model = gurobi.Model()

I = range(10)
J = range(20)

x = poi.make_tupledict(I, J, rule=lambda i, j: model.add_variable(lb=0, name=f"x_{i}_{j}"))

def constraint_rule(j):
    expr = poi.quicksum(x.select("*", j))
    con = model.add_linear_constraint(expr, poi.ConstraintSense.Equal, 1, name=f"con_{j}")
    return con

constraint = poi.make_tupledict(J, rule=constraint_rule)

obj = poi.quicksum_f(x.values(), lambda x: x*x)
model.set_objective(obj, poi.ObjectiveSense.Minimize)

model.optimize()
```

Here we use two utility functions to simplify how we express the sum notation

```{py:function} quicksum(values)

Create a new expression by summing up a list of values.

:param values: iterator of values
:return: the handle of the new expression
```

```{py:function} quicksum_f(values, f)

Create a new expression by summing up a list of values after applying a function to each value.

:param values: iterator of values
:param f: the function to apply to each value
:return: the handle of the new expression
```
