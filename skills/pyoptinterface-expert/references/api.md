# API Reference

## Model Classes
Imported from solver submodules:
- `pyoptinterface.highs.Model`
- `pyoptinterface.gurobi.Model`
- `pyoptinterface.copt.Model`
- `pyoptinterface.ipopt.Model`
- `pyoptinterface.knitro.Model`
- `pyoptinterface.mosek.Model`
- `pyoptinterface.xpress.Model`

## Core Methods
### Model
- `add_variable(lb, ub, domain, name, start)`: Add a single variable.
- `add_m_variables(shape, lb, ub, domain, name, start)`: Add a NumPy array of variables.
- `add_linear_constraint(expr, sense, rhs, name)`: Add a linear constraint.
- `add_quadratic_constraint(expr, sense, rhs, name)`: Add a quadratic constraint.
- `add_m_linear_constraints(A, x, sense, rhs)`: Add constraints in matrix form $Ax \sim b$.
- `add_nl_constraint(expr)`: Add a nonlinear constraint (within `nl.graph()`).
- `set_objective(expr, sense)`: Set objective function.
- `set_nl_objective(expr, sense)`: Set nonlinear objective.
- `optimize()`: Solve the model.
- `get_model_attribute(attr)`: Get model information (Status, ObjVal, etc.).
- `get_variable_attribute(v, attr)`: Get variable information (Value, ReducedCost, etc.).
- `set_variable_attribute(v, attr, value)`: Set variable bounds, starts, etc.
- `get_value(expr)`: Evaluate expression at the current solution.

## Enums
### `poi.VariableDomain`
- `Continuous`
- `Integer`
- `Binary`

### `poi.ConstraintSense`
- `Equal` (or `poi.Eq`)
- `LessEqual` (or `poi.Leq`)
- `GreaterEqual` (or `poi.Geq`)

### `poi.ObjectiveSense`
- `Minimize`
- `Maximize`
- `Feasibility`

### `poi.TerminationStatusCode`
- `OPTIMAL`
- `INFEASIBLE`
- `UNBOUNDED`
- `TIME_LIMIT`
- `ITERATION_LIMIT`

### `poi.ModelAttribute`
- `TerminationStatus`
- `ObjectiveValue`
- `SolveTimeSec`
- `RelativeGap`
- `Silent`

### `poi.VariableAttribute`
- `Value`
- `Name`
- `LowerBound`
- `UpperBound`
- `Domain`
- `PrimalStart`
