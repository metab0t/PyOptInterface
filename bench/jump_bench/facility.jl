# Copyright (c) 2022: Miles Lubin and contributors
#
# Use of this source code is governed by an MIT-style license that can be found
# in the LICENSE.md file or at https://opensource.org/licenses/MIT.

using JuMP
import Gurobi

function solve_facility(model, G, F)
    set_silent(model)
    set_time_limit_sec(model, 0.0)
    set_optimizer_attribute(model, "Presolve", 0)
    @variables(model, begin
        0 <= y[1:F, 1:2] <= 1
        s[0:G, 0:G, 1:F] >= 0
        z[0:G, 0:G, 1:F], Bin
        r[0:G, 0:G, 1:F, 1:2]
        d
    end)
    @objective(model, Min, d)
    @constraint(model, [i in 0:G, j in 0:G], sum(z[i,j,f] for f in 1:F) == 1)
    # d is the maximum of distances between customers
    # and their facilities. The original constraint is # d >= ||x - y|| - M(1-z)
    # where M = 1 for our data. Because Gurobi/CPLEX can't take SOCPs directly,
    # we need to rewite as a set of constraints and auxiliary variables:
    # s = d + M(1 - z) >= 0
    # r = x - y
    # r'r <= s^2
    M = 2 * sqrt(2)
    for i in 0:G, j in 0:G, f in 1:F
        @constraints(model, begin
            s[i, j, f] == d + M * (1 - z[i, j, f])
            r[i, j, f, 1] == i / G - y[f, 1]
            r[i, j, f, 2] == j / G - y[f, 2]
            sum(r[i, j, f, k]^2 for k in 1:2) <= s[i, j, f]^2
        end)
    end
    optimize!(model)
    return model
end

function get_model(arg)
    if arg == "direct"
        return direct_model(Gurobi.Optimizer())
    else
        return Model(Gurobi.Optimizer)
    end
end

function main(io::IO, Ns = [25, 50, 75, 100])
    # for type in ["direct", "default"]
    for type in ["direct"]
        for n in Ns
            start = time()
            model = solve_facility(get_model(type), n, n)
            run_time = round(Int, time() - start)
            num_var = num_variables(model)
            content = "jump_$type fac-$n $num_var $run_time"
            println(stdout, content)
            println(io, content)
        end
    end
end


main(stdout, [5])
open(joinpath(@__DIR__, "benchmarks.csv"), "a") do io
    main(io)
end
