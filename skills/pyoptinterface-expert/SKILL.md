---
name: pyoptinterface-expert
description: Specialized guidance for modeling mathematical optimization problems using the pyoptinterface_native library. Use when building optimization models (LP, QP, MIP, NLP), interfacing with solvers (Gurobi, HiGHS, Ipopt, etc.), and performing matrix-based or nonlinear modeling in Python.
---

# PyOptInterface Expert Skill

This skill provides expert guidance for using `pyoptinterface` (POI), a high-performance Python modeling interface for mathematical optimization solvers.

## Core Concepts

POI follows a common modeling pattern:
1. Create a solver-specific model (e.g., `poi.highs.Model()`).
2. Add variables using `model.add_variable()` or `model.add_m_variables()`.
3. Set the objective using `model.set_objective()`.
4. Add constraints using `model.add_linear_constraint()`, `model.add_quadratic_constraint()`, or `model.add_nl_constraint()`.
5. Call `model.optimize()` and retrieve results.

## Key Workflows

### 1. Basic Modeling
```python
import pyoptinterface as poi
from pyoptinterface import highs

model = highs.Model()
x = model.add_variable(lb=0, name="x")
y = model.add_variable(lb=0, name="y")

model.set_objective(x + 2 * y, poi.ObjectiveSense.Minimize)
model.add_linear_constraint(x + y >= 10, name="c1")

model.optimize()
status = model.get_model_attribute(poi.ModelAttribute.TerminationStatus)
if status == poi.TerminationStatusCode.OPTIMAL:
    print(f"x: {model.get_variable_attribute(x, poi.VariableAttribute.Value)}")
```

### 2. Matrix-based Modeling
Use `add_m_variables` and `add_m_linear_constraints` for performance with NumPy/SciPy.
```python
import numpy as np
x = model.add_m_variables((10,), lb=0)
A = np.random.rand(5, 10)
b = np.ones(5)
model.add_m_linear_constraints(A, x, poi.Leq, b)
```

### 3. Nonlinear Modeling
Nonlinear expressions must be wrapped in `nl.graph()` context.
```python
from pyoptinterface import nl
with nl.graph():
    model.add_nl_objective(x * x + nl.sin(y))
    model.add_nl_constraint(x * y >= 5)
```

## Reference Documentation
- API Reference: [references/api.md](references/api.md)
- Modeling Examples: [references/examples.md](references/examples.md)

## Best Practices
- Use `poi.quicksum()` for large summations.
- Check `TerminationStatus` before accessing variable values.
- For large-scale models, prefer the Matrix API (`add_m_...`).
