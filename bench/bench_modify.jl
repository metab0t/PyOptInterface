using JuMP
import MathOptInterface as MOI
using Gurobi
using COPT

function bench_jump(model, N::Int)
    I = 1:N
    @variable(model, x[i=I] >= 0.0)

    @objective(model, Min, sum(x[i] for i = 1:div(N, 2)))

    get_after_delete_time = 0.0

    for j in div(N, 2)+1:N
        delete(model, x[j])
        optimize!(model)

        t1 = time()
        for k in 1:div(N, 2)
            value(x[k])
        end
        t2 = time()
        get_after_delete_time += t2 - t1
    end

    println("JuMP: get_after_delete_time: ", get_after_delete_time)
end

function gurobi_model()
    optimizer = optimizer_with_attributes(
        Gurobi.Optimizer,
        "Presolve" => 0,
        "TimeLimit" => 2.0,
        MOI.Silent() => true,
    )
    model = direct_model(optimizer)
    return model
end

function copt_model()
    optimizer = optimizer_with_attributes(
        COPT.Optimizer,
        "Presolve" => 0,
        "TimeLimit" => 2.0,
        MOI.Silent() => true,
    )
    model = direct_model(optimizer)
    return model
end

function bench_gurobi(N::Int)
    model = gurobi_model()
    bench_jump(model, N)
end

function bench_copt(N::Int)
    model = copt_model()
    bench_jump(model, N)
end

function main(N::Int)
    println("N: ", N)
    println("Gurobi starts")
    t1 = time()
    bench_gurobi(N)
    t2 = time()
    println("Gurobi ends and takes ", t2 - t1, " seconds")

    println("COPT starts")
    t1 = time()
    bench_copt(N)
    t2 = time()
    println("COPT ends and takes ", t2 - t1, " seconds")
end