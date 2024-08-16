#include "pyoptinterface/container.hpp"
#include "fmt/core.h"
#include "pyoptinterface/gurobi_model.hpp"
#include "pyoptinterface/copt_model.hpp"
#include "pyoptinterface/mosek_model.hpp"
#include "pyoptinterface/highs_model.hpp"

#include <chrono>

struct MapIndexer
{
	Hashmap<IndexT, int> map;

	IndexT add_index()
	{
		auto y = map.size();
		map.emplace(y, y);
		return y;
	}

	int get_index(const IndexT &x)
	{
		auto it = map.find(x);
		if (it == map.end())
		{
			return -1;
		}
		return it->second;
	}
};

struct VecIndexer
{
	Vector<int> vec;

	IndexT add_index()
	{
		auto y = vec.size();
		vec.emplace_back(y);
		return y;
	}

	int get_index(const IndexT &x)
	{
		if (x >= vec.size())
		{
			return -1;
		}
		return vec[x];
	}
};

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
		some_easy_operations(mv);
	}
	{
		ChunkedBitVector<std::uint64_t, int> cbv;
		some_easy_operations(cbv);
	}
	{
		MapIndexer mi;
		some_easy_operations(mi);
	}
	{
		VecIndexer vi;
		some_easy_operations(vi);
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

	auto N = 127;
	for (int i = 0; i < N; i++)
	{
		auto x = cbv.add_index();
		fmt::print("added={}\n", x);
	}

	// auto M = 2;
	// auto x = cbv.add_indices(M);
	// fmt::print("added={}\n", x);

	for (int i = 0; i < N - 1; i++)
	{
		cbv.delete_index(i);
		auto x = cbv.get_index(i + 1);
		if (x != 0)
		{
			fmt::print("wrong get_index: {}->{}\n", i + 1, x);
		}
	}
}

auto test_gurobi() -> void
{
	GurobiEnv env;
	GurobiModelMixin model;
	model.init(env);

	VariableIndex x = model.add_variable(VariableDomain::Continuous, 0.0, 1.0);
	VariableIndex y = model.add_variable(VariableDomain::Continuous, 0.0, 1.0);

	ExprBuilder expr;
	// expr.add(x);
	// expr.add(y);
	// ConstraintIndex con1 = model.add_linear_constraint(expr, ConstraintSense::LessEqual, 10.0);

	expr.clear();
	expr += (x);
	expr += (y);
	model.set_objective(expr, ObjectiveSense::Minimize);

	model.optimize();
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
		expr += (v);
	}
	ScalarAffineFunction saf(expr);

	int k = 0;
}

auto test_copt() -> void
{
	COPTEnv env;
	COPTModelMixin model(env);

	auto N = 100;
	std::vector<VariableIndex> x;
	for (auto i = 0; i < N; i++)
	{
		x.push_back(model.add_variable(VariableDomain::Continuous, 0.0, 1.0, nullptr));
	}

	ExprBuilder expr;
	for (const auto &v : x)
	{
		expr += (v);
	}
	auto con = model.add_linear_constraint_from_expr(expr, ConstraintSense::GreaterEqual, N);

	ExprBuilder obj;
	for (const auto &v : x)
	{
		obj += (v * v);
	}
	model.set_objective(obj, ObjectiveSense::Minimize);
	model.set_raw_parameter_int("Logging", 0);
	model.set_raw_parameter_int("Presolve", 0);
	model.set_raw_parameter_double("TimeLimit", 1.0);

	model.optimize();

	double lb = model.get_constraint_info(con, COPT_DBLINFO_LB);
	double ub = model.get_constraint_info(con, COPT_DBLINFO_UB);

	/*obj = ExprBuilder();
	model.set_objective(obj, ObjectiveSense::Minimize);
	for (int i = 0; i < N - 1; i++)
	{
	    model.delete_variable(x[i]);
	    model.optimize();
	    fmt::print("deleted {}\n", i);
	}*/
}

auto test_mosek() -> void
{
	MOSEKEnv env;
	MOSEKModelMixin model(env);

	auto N = 1000;
	std::vector<VariableIndex> x;
	for (auto i = 0; i < N; i++)
	{
		x.push_back(model.add_variable(VariableDomain::Continuous, 0.0, 1.0, nullptr));
	}

	ExprBuilder obj;
	for (const auto &v : x)
	{
		obj += (v * v);
	}
	model.set_objective(obj, ObjectiveSense::Minimize);
	model.enable_log();

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

auto test_highs() -> void
{
	HighsModelMixin model;

	auto N = 3;
	std::vector<VariableIndex> x;
	for (auto i = 0; i < N; i++)
	{
		x.push_back(model.add_variable(VariableDomain::Continuous, 0.0, 1.0, nullptr));
	}

	ExprBuilder obj;
	for (const auto &v : x)
	{
		obj += (v * v);
	}
	model.set_objective(obj, ObjectiveSense::Minimize);

	ExprBuilder expr;
	for (const auto &v : x)
	{
		expr += (v);
	}
	auto con =
	    model.add_linear_constraint_from_expr(expr, ConstraintSense::GreaterEqual, N / 2.0, "con1");

	model.optimize();

	model.set_objective(x[1] + 2 * x[2], ObjectiveSense::Minimize);
	model.optimize();
}

auto debug_highs_passname(int N) -> void
{
	void *highs = highs::Highs_create();
	
	auto start = std::chrono::high_resolution_clock::now();

	HighsModelMixin model;
	for (auto i = 0; i < N; i++)
	{
		highs::Highs_addCol(highs, 0.0, 0.0, 0.0, 0, nullptr, nullptr);

		auto col = highs::Highs_getNumCol(highs);

		assert(col == i + 1);

		auto name = fmt::format("x{}", i);
		// highs::Highs_passColName(highs, i, name.c_str());
	}

	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	fmt::print("N={}, time={} milliseconds\n", N, duration.count());
}

void debug_highs(int N)
{
	auto start = std::chrono::high_resolution_clock::now();

	HighsModelMixin model;
	for (auto i = 0; i < N; i++)
	{
		model.add_variable(VariableDomain::Continuous, 0.0, 1.0, nullptr);
	}

	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	fmt::print("N={}, time={} milliseconds\n", N, duration.count());
}

void test_highs_add_variable()
{
	std::vector<int> Ns{1000, 10000, 100000, 1000000};

	for (const auto& N : Ns)
	{
		debug_highs_passname(N);
	}
}

void test_highs_qp()
{
	HighsModelMixin model;

	auto x = model.add_variable();
	auto y = model.add_variable();

	model.set_objective(x * y, ObjectiveSense::Minimize);
	model.optimize();
}

auto main() -> int
{
	highs::load_library("E:\\HiGHS\\build\\bin\\Release\\highs.dll");

	test_highs_qp();
	return 0;
}