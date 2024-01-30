## Run benchmark experiments

```
$ gurobi_cpp_build.bat gurobi110

$ julia create_sysimage.jl
$ julia --sysimage sysimage facility.jl --run
$ julia --sysimage sysimage lqcp.jl --run

$ python facility.py
$ python facility_poi.py
$ python lqcp.py
$ python lqcp_poi.py

$ julia gurobi_cpp_run.jl

$ julia produce_table.jl
```