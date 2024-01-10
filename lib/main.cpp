#include "pyoptinterface/container.hpp"
#include "fmt/core.h"
#include "pyoptinterface/gurobi_model.hpp"
#include "pyoptinterface/copt_model.hpp"

template <typename T>
void some_operations(T &t)
{
	auto N = 5e8;
	for (auto i = 0; i < N; i++)
	{
		t.add_index();
	}

	for (int i = 0.25 * N; i < 0.25 * N * 7 / 6; i++)
	{
		t.delete_index(i);
	}
	for (auto i = 0; i < N; i++)
	{
		t.get_index(i);
	}

	for (int i = 0.65 * N; i < 0.75 * N; i++)
	{
		t.delete_index(i);
	}
	for (auto i = 0; i < N; i++)
	{
		t.get_index(i);
	}
}

template <typename T>
void some_easy_operations(T &t)
{
	auto N = 1e8;
	for (auto i = 0; i < N; i++)
	{
		t.add_index();
	}

	for (auto i = 0; i < N; i++)
	{
		t.get_index(i);
	}
}

void bench_container()
{
	{
		MonotoneVector<int> mv;
		some_operations(mv);
	}
	{
		ChunkedBitVector<std::uint64_t, int> cbv;
		some_operations(cbv);
	}
}

auto test_monotone() -> void
{
	MonotoneVector<int> mv;
	for (auto i = 0; i < 10; i++)
	{
		mv.add_index();
	}
	mv.delete_index(5);
	mv.delete_index(8);
	mv.delete_index(2);

	for (auto i = 0; i < 10; i++)
	{
		mv.add_index();
	}
	mv.delete_index(1);
	mv.delete_index(0);

	for (auto i = 0; i < 5; i++)
	{
		mv.add_index();
	}
	mv.delete_index(15);
	mv.delete_index(20);
	mv.delete_index(21);
	mv.delete_index(22);

	int x = mv.get_index(24);
}

auto test_chunkedbv() -> void
{
	ChunkedBitVector<std::uint64_t, int> cbv;

	auto N = 10000;
	for (int i = 0; i < N; i++)
	{
		auto x = cbv.add_index();
		fmt::print("added={}\n", x);
	}

	for (int i = 0; i < N - 1; i++)
	{
		cbv.delete_index(i);
		auto x = cbv.get_index(i + 1);
		if (x != 0)
		{
			fmt::print("get_index: {}->{}\n", i + 1, x);
		}
	}
}

auto test_gurobi() -> void
{
	GRBenv *env;
	int error;

	error = GRBloadenv(&env, NULL);
	if (error)
	{
		printf("%s\n", GRBgeterrormsg(env));
		goto end;
	}

	{
		GurobiEnv env;
		GurobiModelMixin model;
		model.init(env);

		VariableIndex x = model.add_variable(VariableDomain::Continuous, 0.0, 1.0);
		VariableIndex y = model.add_variable(VariableDomain::Continuous, 0.0, 1.0);

		ExprBuilder expr;
		//expr.add(x);
		//expr.add(y);
		//ConstraintIndex con1 = model.add_linear_constraint(expr, ConstraintSense::LessEqual, 10.0);

		expr.clear();
		expr.add(x);
		expr.add(y);
		model.set_objective(expr, ObjectiveSense::Minimize);

		model.optimize();
	}

end:
	GRBfreeenv(env);
}

void bench()
{
	auto N = 50000;
	std::vector<VariableIndex> x(N);
	for (auto i = 0; i < N; i++)
	{
		x[i].index = i;
	}

	ExprBuilder expr;
	for (const auto &v : x)
	{
		expr.add(v);
	}
	ScalarAffineFunction saf(expr);

	int k = 0;
}

auto test_copt() -> void
{
	COPTEnv env;
	COPTModelMixin model(env);

	auto N = 10000;
	std::vector<VariableIndex> x;
	for (auto i = 0; i < N; i++)
	{
		x.push_back(model.add_variable(VariableDomain::Continuous, 0.0, 1.0, nullptr));
	}

	ExprBuilder obj;
	for (const auto &v : x)
	{
		obj.add(v * v);
	}
	model.set_objective(obj, ObjectiveSense::Minimize);
	model.set_raw_parameter_int("Logging", 0);
	model.set_raw_parameter_int("Presolve", 0);
	model.set_raw_parameter_double("TimeLimit", 0.0);

	model.optimize();

	obj = ExprBuilder();
	model.set_objective(obj, ObjectiveSense::Minimize);
	for (int i = 0; i < N - 1; i++)
	{
		model.delete_variable(x[i]);
		model.optimize();
		fmt::print("deleted {}\n", i);
	}
}

auto main() -> int
{
	test_gurobi();
	return 0;
}