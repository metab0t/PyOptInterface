# Benchmark

The benchmark is adapted from [the JuMP paper](https://github.com/jump-dev/JuMPPaperBenchmarks). Eight optimization models with different sizes are selected as test cases, including four facility location models and four linear quadratic control problems. We conduct two rounds of benchmark using Gurobi and COPT as optimizer respectively. For each model, we measure the total time of modeling interface to generate model and pass it to the optimizer, and the time limit of optimizer is set to 0.0 seconds to avoid the influence of solution process.

In our benchmark, we include the following modeling interfaces:

1. `C++`: Official C++ interfaces to Gurobi and COPT provided by the optimizers.
2. `PyOptInterface`: The Python interface to Gurobi and COPT implemented by our paper.
3. `JuMP`: An open-source algebraic modeling language in Julia, it is used with `Gurobi.jl` and `COPT.jl` to interact with optimizers. We use the direct mode feature of JuMP to skip an additional cache of the problem. The Julia code is warmed up by running a small case before the benchmark to rule out compilation latency of Julia.
4. `gurobipy` and `coptpy`: Official Python interfaces to Gurobi and COPT provided by the optimizers.
5. `Pyomo`: An open-source algebraic modeling language in Python. We use the in-memory persistent solver interface of Pyomo to interact with Gurobi and COPT without writing the model to disk. The Pyomo support for COPT is bundled with the COPT distribution.

The software used to run benchmark includes `PyOptInterface` v0.1.0, Python v3.11.8, Gurobi v11.0.1, `gurobipy` v11.0.1, COPT v7.1.1, `coptpy` v7.1.1, Julia v1.10.2, `JuMP.jl` v1.20.0, `Gurobi.jl` v1.2.3, `COPT.jl` v1.1.15 and Pyomo v6.7.1. The hardware to run benchmark is a Windows laptop with i7-1360P CPU and 32GB RAM. All code to run the benchmarks is available at [https://github.com/metab0t/PyOptInterface_benchmark](https://github.com/metab0t/PyOptInterface_benchmark).

:::{table} Time (second) to generate model and pass it to Gurobi optimizer.
:widths: auto
:align: center

| Model    | Variables | C++ | PyOptInterface | JuMP | gurobipy | Pyomo |
|----------|-----------|-----|----------------|------|----------|-------|
| fac-25   | 67651     | 0.1 | 0.2            | 0.2  | 1.2      | 4.1   |
| fac-50   | 520301    | 0.8 | 2.0            | 1.8  | 9.7      | 32.7  |
| fac-75   | 1732951   | 2.7 | 7.9            | 7.5  | 32.5     | 119.3 |
| fac-100  | 4080601   | 6.5 | 19.8           | 21.7 | 79.1     | 286.3 |
| lqcp-500 | 251501    | 1.0 | 1.8            | 1.3  | 6.3      | 23.8  |
| lqcp-1000| 1003001   | 3.5 | 7.9            | 7.0  | 26.7     | 106.6 |
| lqcp-1500| 2254501   | 8.1 | 18.9           | 19.9 | 61.8     | 234.0 |
| lqcp-2000| 4006001   | 14.5| 34.8           | 41.6 | 106.9    | 444.1 |

:::

:::{table} Time (second) to generate model and pass it to COPT optimizer.
:widths: auto
:align: center

| Model    | Variables | C++ | PyOptInterface | JuMP | coptpy | Pyomo |
|----------|-----------|-----|----------------|------|--------|-------|
| fac-25   | 67651     | 0.3 | 0.3            | 0.4  | 0.6    | 4.1   |
| fac-50   | 520301    | 1.9 | 2.4            | 3.3  | 5.4    | 32.8  |
| fac-75   | 1732951   | 7.0 | 10.9           | 11.2 | 20.3   | 117.4 |
| fac-100  | 4080601   | 19.8| 35.1           | 33.4 | 58.0   | 284.0 |
| lqcp-500 | 251501    | 3.3 | 3.5            | 3.2  | 6.6    | 26.4  |
| lqcp-1000| 1003001   | 14.0| 16.3           | 15.6 | 28.1   | 112.1 |
| lqcp-1500| 2254501   | 32.5| 37.7           | 37.7 | 64.6   | 249.3 |
| lqcp-2000| 4006001   | 62.7| 71.3           | 73.4 | 118.4  | 502.4 |

:::
