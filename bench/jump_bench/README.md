## Run benchmark experiments

```
$ gurobi_cpp_build.bat gurobi110

$ julia facility.jl
$ julia lqcp.jl

$ python facility.py
$ python facility_gurobipy.py
$ python facility_poi.py gurobi
$ python lqcp.py
$ python lqcp_gurobipy.py
$ python lqcp_poi.py gurobi

$ julia gurobi_cpp_run.jl

$ julia produce_table.jl

$ ampl facility.run
$ ampl lqcp.run
```