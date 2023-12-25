#include "fmt/format.h"
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

std::string CommercialSolverBase::pprint_expression(const ScalarAffineFunction &function, int precision)
{
	auto N = function.size();
	std::vector<std::string> terms;
	terms.reserve(N + 1);

	for (auto i = 0; i < N; ++i)
	{
		auto &coef = function.coefficients[i];
		std::string var_str = pprint_variable(function.variables[i]);
		std::string term;
		if (coef > 0)
		{
			term = fmt::format("{:.{}g}*{}", coef, precision, var_str);
		}
		else if (coef < 0)
		{
			term = fmt::format("({:.{}g})*{}", coef, precision, var_str);
		}
		terms.push_back(term);
	}
	if (function.constant)
	{
		terms.push_back(fmt::format("{:.{}g}", function.constant.value(), precision));
	}
	return fmt::format("{}", fmt::join(terms, "+"));
}

std::string CommercialSolverBase::pprint_expression(const ScalarQuadraticFunction &function, int precision)
{
	auto N = function.size();
	std::vector<std::string> terms;
	terms.reserve(N + 1);

	for (auto i = 0; i < N; ++i)
	{
		auto &coef = function.coefficients[i];
		std::string var1_str = pprint_variable(function.variable_1s[i]);
		std::string var2_str = pprint_variable(function.variable_2s[i]);
		std::string term;
		if (coef > 0)
		{
			term = fmt::format("{:.{}g}*{}*{}", coef, precision, var1_str, var2_str);
		}
		else if (coef < 0)
		{
			term = fmt::format("({:.{}g})*{}*{}", coef, precision, var1_str, var2_str);
		}
		terms.push_back(term);
	}
	if (function.affine_part)
	{
		terms.push_back(pprint_expression(function.affine_part.value(), precision));
	}
	return fmt::format("{}", fmt::join(terms, "+"));
}

std::string CommercialSolverBase::pprint_expression(const ExprBuilder &function, int precision)
{
	std::vector<std::string> terms;
	terms.reserve(function.quadratic_terms.size() + function.affine_terms.size() + 1);

	for (const auto &[varpair, coef] : function.quadratic_terms)
	{
		std::string var1_str = pprint_variable(varpair.var_1);
		std::string var2_str = pprint_variable(varpair.var_2);
		std::string term;
		if (coef > 0)
		{
			term = fmt::format("{:.{}g}*{}*{}", coef, precision, var1_str, var2_str);
		}
		else if (coef < 0)
		{
			term = fmt::format("({:.{}g})*{}*{}", coef, precision, var1_str, var2_str);
		}
		terms.push_back(term);
	}
	for (const auto &[var, coef] : function.affine_terms)
	{
		std::string var_str = pprint_variable(var);
		std::string term;
		if (coef > 0)
		{
			term = fmt::format("{:.{}g}*{}", coef, precision, var_str);
		}
		else if (coef < 0)
		{
			term = fmt::format("({:.{}g})*{}", coef, precision, var_str);
		}
		terms.push_back(term);
	}
	if (function.constant_term)
	{
		terms.push_back(fmt::format("{:.{}g}", function.constant_term.value(), precision));
	}
	return fmt::format("{}", fmt::join(terms, "+"));
}
