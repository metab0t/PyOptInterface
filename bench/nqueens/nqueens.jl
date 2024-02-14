using JuMP
import Gurobi
import LinearAlgebra

function solve_nqueens(N)
    model = direct_model(Gurobi.Optimizer())
    set_silent(model)
    set_time_limit_sec(model, 0.0)
    set_optimizer_attribute(model, "Presolve", 0)

    @variable(model, x[1:N, 1:N], Bin)

    for i in 1:N
        @constraint(model, sum(x[i, :]) == 1)
        @constraint(model, sum(x[:, i]) == 1)
    end

    for i in -(N - 1):(N-1)
        @constraint(model, sum(LinearAlgebra.diag(x, i)) <= 1)
        @constraint(model, sum(LinearAlgebra.diag(reverse(x; dims=1), i)) <= 1)
    end

    optimize!(model)
end

function main(io::IO, Ns = 800:400:2000)
    for n in Ns
        start = time()
        model = solve_nqueens(n)
        run_time = round(Int, time() - start)
        content = "jump nqueens-$n $run_time"
        println(stdout, content)
        println(io, content)
    end
end

main(stdout, [5])
open(joinpath(@__DIR__, "benchmarks.csv"), "a") do io
    main(io)
end