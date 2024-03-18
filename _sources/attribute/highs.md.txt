### Supported [model attribute](#pyoptinterface.ModelAttribute)

:::{list-table}
:header-rows: 1

*   - Attribute
    - Get
    - Set
*   - Name
    - ❌
    - ❌
*   - ObjectiveSense
    - ✅
    - ✅
*   - DualStatus
    - ✅
    - ❌
*   - PrimalStatus
    - ✅
    - ❌
*   - RawStatusString
    - ✅
    - ❌
*   - TerminationStatus
    - ✅
    - ❌
*   - BarrierIterations
    - ❌
    - ❌
*   - DualObjectiveValue
    - ❌
    - ❌
*   - NodeCount
    - ❌
    - ❌
*   - NumberOfThreads
    - ✅
    - ✅
*   - ObjectiveBound
    - ❌
    - ❌
*   - ObjectiveValue
    - ✅
    - ❌
*   - RelativeGap
    - ✅
    - ✅
*   - Silent
    - ✅
    - ✅
*   - SimplexIterations
    - ❌
    - ❌
*   - SolverName
    - ✅
    - ❌
*   - SolverVersion
    - ✅
    - ❌
*   - SolveTimeSec
    - ✅
    - ❌
*   - TimeLimitSec
    - ✅
    - ✅
:::

### Supported [variable attribute](#pyoptinterface.VariableAttribute)

:::{list-table}
:header-rows: 1

*   - Attribute
    - Get
    - Set
*   - Value
    - ✅
    - ❌
*   - LowerBound
    - ✅
    - ✅
*   - UpperBound
    - ✅
    - ✅
*   - Domain
    - ✅
    - ✅
*   - PrimalStart
    - ✅
    - ✅
*   - Name
    - ✅
    - ✅
:::

### Supported [constraint attribute](#pyoptinterface.ConstraintAttribute)

:::{list-table}
:header-rows: 1

*   - Attribute
    - Get
    - Set
*   - Name
    - ✅
    - ✅
*   - Primal
    - ✅
    - ❌
*   - Dual
    - ✅
    - ❌
:::

