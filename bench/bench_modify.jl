using JuMP
import MathOptInterface as MOI
using Gurobi
using COPT

function bench_jump(model, N::Int, M::Int)
    I = 1:N
    @variable(model, x[I])

    @constraint(model, sum(x[i] for i = I) == N)

    @objective(model, Min, sum(i / N * x[i] for i = I))

    # for j in 1:M:N
    for j in [1]
        last_variable = j + M - 1
        if last_variable > N
            last_variable = N
        end
        deleted_variables = [x[i] for i in j:last_variable]
        delete(model, deleted_variables)
        optimize!(model)
    end
end

function gurobi_model()
    optimizer = optimizer_with_attributes(
        Gurobi.Optimizer,
        "Presolve" => 0,
        "TimeLimit" => 0.0,
        MOI.Silent() => true,
    )
    model = direct_model(optimizer)
    return model
end

function copt_model()
    optimizer = optimizer_with_attributes(
        COPT.Optimizer,
        "Presolve" => 0,
        "TimeLimit" => 0.0,
        MOI.Silent() => true,
    )
    model = direct_model(optimizer)
    return model
end

function bench_gurobi(N, M)
    model = gurobi_model()
    bench_jump(model, N, M)
end

function bench_copt(N, M)
    model = copt_model()
    bench_jump(model, N, M)
end

function main(N, M)
    time_dict = Dict{String,Float64}()

    println("N: $N, M: $M")
    println("Gurobi starts")
    t1 = time()
    bench_gurobi(N, M)
    t2 = time()
    println("Gurobi ends and takes ", t2 - t1, " seconds")
    time_dict["gurobi"] = t2 - t1

    println("COPT starts")
    t1 = time()
    bench_copt(N, M)
    t2 = time()
    println("COPT ends and takes ", t2 - t1, " seconds")
    time_dict["copt"] = t2 - t1

    return time_dict
end

main(5, 1)

result_dict = Dict()
for N in [1000000]
    for M in [10000]
        result_dict[(N, M)] = main(N, M)
    end
end

result_dict