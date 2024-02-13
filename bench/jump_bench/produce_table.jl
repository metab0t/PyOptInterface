# Copyright (c) 2022: Miles Lubin and contributors
#
# Use of this source code is governed by an MIT-style license that can be found
# in the LICENSE.md file or at https://opensource.org/licenses/MIT.

import DelimitedFiles
df = DelimitedFiles.readdlm(joinpath(@__DIR__, "benchmarks.csv"))
models = Dict{String,Dict{String,Int}}()
for i in 1:size(df, 1)
    type, model, variables, time = df[i, :]
    inner = get!(models, model, Dict{String,Int}())
    inner[type] = time
    if type == "jump_direct"
        inner["variables"] = variables
    end
end

columns = ["model", "variables", "gurobi", "gurobipy", "poi_gurobi", "jump_direct", "pyomo_appsi"]
keys = [
    "fac-25",
    "fac-50",
    "fac-75",
    "fac-100",
    "lqcp-500",
    "lqcp-1000",
    "lqcp-1500",
    "lqcp-2000",
]
import DataFrames
# Turn the Dict into a DataFrame
column_dict = Dict{String, Vector{Any}}()
column_dict["model"] = keys
for column in columns[2:end]
    column_dict[column] = [models[key][column] for key in keys]
end

df = DataFrames.DataFrame(
    (column => column_dict[column] for column in columns)...
)

@show df