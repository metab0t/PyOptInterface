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
		expr += x;
		expr += y;
		ConstraintIndex con1 = model.add_linear_constraint(expr, ConstraintSense::LessEqual, 10.0);

		expr.clear();
		expr += x;
		ConstraintIndex con2 =
		    model.add_linear_constraint(expr, ConstraintSense::GreaterEqual, 2.0);

		expr.clear();
		expr += y;
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

auto main() -> int
{
	test_gurobi();
	return 0;
}