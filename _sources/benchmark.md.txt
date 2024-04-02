# Benchmark

The benchmark is adapted from [the JuMP paper](https://github.com/jump-dev/JuMPPaperBenchmarks). Eight optimization models with different sizes are selected as test cases, including four facility location models and four linear quadratic control problems. We conduct two rounds of benchmark using Gurobi and COPT as optimizer respectively. For each model, we measure the total time of modeling interface to generate model and pass it to the optimizer, and the time limit of optimizer is set to 0.0 seconds to avoid the influence of solution process.

All code to run the benchmarks is available at [https://github.com/metab0t/PyOptInterface_benchmark](https://github.com/metab0t/PyOptInterface_benchmark).

:::{table} Time (second) to generate model and pass it to Gurobi optimizer.
:widths: auto
:align: center

| Model    | Variables | C++ | PyOptInterface | JuMP | gurobipy | Pyomo |
|----------|-----------|-----|----------------|------|----------|-------|
| fac-25   | 67651     | 0.2 | 0.2            | 0.2  | 1.2      | 4.1   |
| fac-50   | 520301    | 0.8 | 1.2            | 1.8  | 9.7      | 32.7  |
| fac-75   | 1732951   | 2.7 | 4.1            | 6.6  | 32.5     | 119.3 |
| fac-100  | 4080601   | 6.3 | 10.0           | 17.8 | 79.1     | 286.3 |
| lqcp-500 | 251501    | 0.9 | 1.5            | 1.3  | 6.3      | 23.8  |
| lqcp-1000| 1003001   | 3.7 | 6.0            | 6.1  | 26.7     | 106.6 |
| lqcp-1500| 2254501   | 8.3 | 14.0           | 17.7 | 61.8     | 234.0 |
| lqcp-2000| 4006001   | 14.5| 24.9           | 38.3 | 106.9    | 444.1 |

:::

:::{table} Time (second) to generate model and pass it to COPT optimizer.
:widths: auto
:align: center

| Model    | Variables | C++ | PyOptInterface | JuMP | coptpy | Pyomo |
|----------|-----------|-----|----------------|------|--------|-------|
| fac-25   | 67651     | 0.3 | 0.2            | 0.3  | 0.6    | 4.1   |
| fac-50   | 520301    | 2.2 | 1.5            | 2.7  | 5.4    | 32.8  |
| fac-75   | 1732951   | 8.1 | 6.6            | 10.2 | 20.3   | 117.4 |
| fac-100  | 4080601   | 22.4| 23.4           | 30.3 | 58.0   | 284.0 |
| lqcp-500 | 251501    | 3.8 | 3.1            | 3.0  | 6.6    | 26.4  |
| lqcp-1000| 1003001   | 16.0| 15.5           | 13.9 | 28.1   | 112.1 |
| lqcp-1500| 2254501   | 37.6| 32.4           | 33.7 | 64.6   | 249.3 |
| lqcp-2000| 4006001   | 68.2| 60.3           | 66.2 | 118.4  | 502.4 |

:::
