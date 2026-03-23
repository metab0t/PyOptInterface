# PyOptInterface Modeling Guidelines

When modeling with `pyoptinterface_native`, adhere to these patterns.

## Solver Initialization
- Use `pyoptinterface.<solver>.Model()`. Supported: `highs`, `gurobi`, `copt`, `ipopt`, `knitro`, `mosek`, `xpress`.
- For Gurobi, you can manage environments with `poi.gurobi.Env()`.

## Variables
- `model.add_variable(lb=None, ub=None, domain=poi.VariableDomain.Continuous, name="", start=None)`
- `model.add_m_variables(shape, ...)` returns a NumPy array of variables.

## Constraints
- Linear: `model.add_linear_constraint(expr, sense, rhs)` or `model.add_linear_constraint(lhs <= rhs)`.
- Quadratic: `model.add_quadratic_constraint(expr, sense, rhs)`.
- Matrix: `model.add_m_linear_constraints(A, x, sense, b)`.
- Nonlinear: Wrap in `with nl.graph():` and use `model.add_nl_constraint(expr)`.

## Objective
- `model.set_objective(expr, sense)`.
- Nonlinear: `with nl.graph(): model.add_nl_objective(expr, sense)`.

## Attributes
- Get: `model.get_model_attribute(poi.ModelAttribute.TerminationStatus)`.
- Set: `model.set_variable_attribute(v, poi.VariableAttribute.LowerBound, 0.0)`.
- Results: `model.get_variable_attribute(v, poi.VariableAttribute.Value)`.

## Common Utilities
- `poi.quicksum(terms)`: Efficiently sum variables/expressions.
- `poi.Eq`, `poi.Leq`, `poi.Geq`: Aliases for constraint senses.
