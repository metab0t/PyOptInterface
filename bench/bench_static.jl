using JuMP
using Gurobi
using COPT

function bench_jump(model, M, N)
    @variable(model, x[1:M, 1:N] >= 0)

    @constraint(model, sum(x) == M * N / 2)

    @objective(model, Min, sum(x[i, j]^2 for i in 1:M, j in 1:N))

    set_silent(model)
    set_time_limit_sec(model, 0.0)
    set_optimizer_attribute(model, "Presolve", 0)

    optimize!(model)
end

function bench_gurobi(M::Int, N::Int)
    model = direct_model(Gurobi.Optimizer())
    bench_jump(model, M, N)
end

function bench_copt(M::Int, N::Int)
    model = direct_model(COPT.Optimizer())
    bench_jump(model, M, N)
end

function main(M::Int, N::Int)
    println("Gurobi starts")
    t1 = time()
    bench_gurobi(M, N)
    t2 = time()
    println("Gurobi ends and takes ", t2 - t1, " seconds")

    println("COPT starts")
    t1 = time()
    bench_copt(M, N)
    t2 = time()
    println("COPT ends and takes ", t2 - t1, " seconds")
end