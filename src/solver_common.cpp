#include "pyoptinterface/solver_common.hpp"

ConstraintIndex CommercialSolverBase::add_linear_constraint_from_expr(const ExprBuilder &function,
                                                                      ConstraintSense sense,
                                                                      CoeffT rhs)
{
	ScalarAffineFunction f(function);
	return add_linear_constraint(f, sense, rhs);
}

ConstraintIndex CommercialSolverBase::add_quadratic_constraint_from_expr(
    const ExprBuilder &function, ConstraintSense sense, CoeffT rhs)
{
	ScalarQuadraticFunction f(function);
	return add_quadratic_constraint(f, sense, rhs);
}

double CommercialSolverBase::get_expression_value(const ScalarAffineFunction &function)
{
	auto N = function.size();
	double value = 0.0;
	for (auto i = 0; i < N; ++i)
	{
		value += function.coefficients[i] * get_variable_value(function.variables[i]);
	}
	value += function.constant.value_or(0.0);
	return value;
}

double CommercialSolverBase::get_expression_value(const ScalarQuadraticFunction &function)
{
	auto N = function.size();
	double value = 0.0;
	for (auto i = 0; i < N; ++i)
	{
		auto var1 = function.variable_1s[i];
		auto var2 = function.variable_2s[i];
		value += function.coefficients[i] * get_variable_value(var1) * get_variable_value(var2);
	}
	if (function.affine_part)
	{
		auto affine_value = get_expression_value(function.affine_part.value());
		value += affine_value;
	}
	return value;
}

double CommercialSolverBase::get_expression_value(const ExprBuilder &function)
{
	double value = 0.0;
	for (const auto &[varpair, coef] : function.quadratic_terms)
	{
		auto var1 = varpair.var_1;
		auto var2 = varpair.var_2;
		value += coef * get_variable_value(var1) * get_variable_value(var2);
	}
	for (const auto &[var, coef] : function.affine_terms)
	{
		value += coef * get_variable_value(var);
	}
	value += function.constant_term.value_or(0.0);
	return value;
}
