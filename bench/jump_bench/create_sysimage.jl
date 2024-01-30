# Copyright (c) 2022: Miles Lubin and contributors
#
# Use of this source code is governed by an MIT-style license that can be found
# in the LICENSE.md file or at https://opensource.org/licenses/MIT.

using PackageCompiler, Libdl
PackageCompiler.create_sysimage(
    ["JuMP", "Gurobi"],
    sysimage_path = joinpath(@__DIR__, "sysimage." * Libdl.dlext),
    precompile_execution_file = [
        joinpath(@__DIR__, "facility.jl"),
        joinpath(@__DIR__, "lqcp.jl"),
    ],
)
