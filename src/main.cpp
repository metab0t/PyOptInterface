#include "pyoptinterface/container.hpp"
#include "gurobi_model.cpp"

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

	mv.update();
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
		model.set_objective(ScalarQuadraticFunction(expr), ObjectiveSense::Minimize);

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
	bench();
	return 0;
}