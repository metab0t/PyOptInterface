# Copyright (c) 2022: Miles Lubin and contributors
#
# Use of this source code is governed by an MIT-style license that can be found
# in the LICENSE.md file or at https://opensource.org/licenses/MIT.

using JuMP
import Gurobi

function solve_lqcp(model, n)
    set_silent(model)
    set_time_limit_sec(model, 0.0)
    set_optimizer_attribute(model, "Presolve", 0)
    m  = n
    n1 = n - 1
    m1 = m - 1
    dx = 1 / n
    T = 1.58
    dt = T / m
    h2 = dx^2
    a = 0.001
    yt = [0.5 * (1 - (j * dx)^2) for j = 0:n]
    @variables(model, begin
        0 <= y[0:m, 0:n] <= 1
        -1 <= u[1:m] <= 1
    end)
    @objective(model, Min,
        1/4*dx*(      (y[m,0] - yt[0+1])^2  +
                2*sum((y[m,j] - yt[j+1])^2 for j in 1:n1) +
                      (y[m,n] - yt[n+1])^2) +
        1/4*a*dt*(2*sum(u[i]^2 for i in 1:m1) + u[m]^2)
    )
    # PDE
    for i in 0:m1, j in 1:n1
        @constraint(model, h2*(y[i+1,j] - y[i,j]) == 0.5*dt*(y[i,j-1] - 2*y[i,j] + y[i,j+1] + y[i+1,j-1] - 2*y[i+1,j] + y[i+1,j+1]) )
    end
    # IC
    for j in 0:n
        @constraint(model, y[0, j] == 0)
    end
    # BC
    for i in 1:m
        @constraint(model, y[i, 2] - 4 * y[i, 1] + 3 * y[i, 0] == 0)
        @constraint(model, y[i, n-2] - 4 * y[i, n1] + 3 * y[i, n] == (2dx) * (u[i] - y[i, n]))
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

function main(io::IO, Ns = [500, 1000, 1500, 2000])
    # for type in ["direct", "default"]
    for type in ["direct"]
        for n in Ns
            start = time()
            model = solve_lqcp(get_model(type), n)
            run_time = round(Int, time() - start)
            num_var = num_variables(model)
            content = "jump_$type lqcp-$n $num_var $run_time"
            println(stdout, content)
            println(io, content)
        end
    end
end

main(stdout, [5])
open(joinpath(@__DIR__, "benchmarks.csv"), "a") do io
    main(io)
end
