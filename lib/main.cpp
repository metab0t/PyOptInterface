#include "pyoptinterface/container.hpp"
#include "fmt/core.h"
#include "gurobi_model.cpp"

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

	for (int i = 0; i < 100; i++)
	{
		auto x = cbv.add_index();
		fmt::print("{}\n", x);
	}

	cbv.delete_index(3);
	cbv.delete_index(5);

	for (int i = 0; i < 100; i++)
	{
		auto x = cbv.get_index(i);
		fmt::print("get_index: {}->{}\n", i, x);
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
		GurobiModel model;
		model.init(env);

		VariableIndex x = model.add_variable(VariableDomain::Continuous);
		VariableIndex y = model.add_variable(VariableDomain::Continuous);

		ExprBuilder expr;
		expr.add(x);
		expr.add(y);
		ConstraintIndex con1 = model.add_linear_constraint(expr, ConstraintSense::LessEqual, 10.0);

		expr.clear();
		expr.add(x);
		ConstraintIndex con2 =
		    model.add_linear_constraint(expr, ConstraintSense::GreaterEqual, 2.0);

		expr.clear();
		expr.add(y);
		ConstraintIndex con3 =
		    model.add_linear_constraint(expr, ConstraintSense::GreaterEqual, 2.0);

		expr.clear();
		expr.add_quadratic_term(x, x, 1.0);
		expr.add_quadratic_term(y, y, 1.0);
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

auto main() -> int
{
	test_chunkedbv();
	return 0;
}