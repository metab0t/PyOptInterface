#pragma once

#include <string>
#include <concepts>
#include "fmt/format.h"
#include "pyoptinterface/core.hpp"

template <typename CommercialSolverT>
concept CommercialSolverConstraint = requires(CommercialSolverT *model) {
	{
		model->add_linear_constraint(std::declval<const ScalarAffineFunction &>(),
		                             ConstraintSense{}, CoeffT{})
	} -> std::same_as<ConstraintIndex>;
	{
		model->add_quadratic_constraint(std::declval<const ScalarQuadraticFunction &>(),
		                                ConstraintSense{}, CoeffT{})
	} -> std::same_as<ConstraintIndex>;
	{
		model->get_variable_value(std::declval<const VariableIndex &>())
	} -> std::convertible_to<double>;
	{
		model->pprint_variable(std::declval<const VariableIndex &>())
	} -> std::convertible_to<std::string>;
};

template <CommercialSolverConstraint T>
class CommercialSolverMixin : public T
{
  private:
	T *get_base();

  public:
	ConstraintIndex add_linear_constraint_from_expr(const ExprBuilder &function,
	                                                ConstraintSense sense, CoeffT rhs);
	ConstraintIndex add_quadratic_constraint_from_expr(const ExprBuilder &function,
	                                                   ConstraintSense sense, CoeffT rhs);

	double get_expression_value(const ScalarAffineFunction &function);
	double get_expression_value(const ScalarQuadraticFunction &function);
	double get_expression_value(const ExprBuilder &function);

	std::string pprint_expression(const ScalarAffineFunction &function, int precision = 4);
	std::string pprint_expression(const ScalarQuadraticFunction &function, int precision = 4);
	std::string pprint_expression(const ExprBuilder &function, int precision = 4);
};

template <CommercialSolverConstraint T>
T *CommercialSolverMixin<T>::get_base()
{
	return static_cast<T *>(this);
}

template <CommercialSolverConstraint T>
ConstraintIndex CommercialSolverMixin<T>::add_linear_constraint_from_expr(
    const ExprBuilder &function, ConstraintSense sense, CoeffT rhs)
{
	ScalarAffineFunction f(function);
	return get_base()->add_linear_constraint(f, sense, rhs);
}

template <CommercialSolverConstraint T>
ConstraintIndex CommercialSolverMixin<T>::add_quadratic_constraint_from_expr(
    const ExprBuilder &function, ConstraintSense sense, CoeffT rhs)
{
	ScalarQuadraticFunction f(function);
	return get_base()->add_quadratic_constraint(f, sense, rhs);
}

template <typename T>
concept VarValueModel = requires(T *model, const VariableIndex &variable) {
	{
		model->get_variable_value(variable)
	} -> std::convertible_to<double>;
};

template <CommercialSolverConstraint T>
double CommercialSolverMixin<T>::get_expression_value(const ScalarAffineFunction &function)
{
	T *model = get_base();
	auto N = function.size();
	double value = 0.0;
	for (auto i = 0; i < N; ++i)
	{
		value += function.coefficients[i] * model->get_variable_value(function.variables[i]);
	}
	value += function.constant.value_or(0.0);
	return value;
}

template <CommercialSolverConstraint T>
double CommercialSolverMixin<T>::get_expression_value(const ScalarQuadraticFunction &function)
{
	T *model = get_base();
	auto N = function.size();
	double value = 0.0;
	for (auto i = 0; i < N; ++i)
	{
		auto var1 = function.variable_1s[i];
		auto var2 = function.variable_2s[i];
		auto coef = function.coefficients[i];
		auto v1 = model->get_variable_value(var1);
		if (var1 == var2)
		{
			value += coef * v1 * v1;
		}
		else
		{
			auto v2 = model->get_variable_value(var2);
			value += coef * v1 * v2;
		}
	}
	if (function.affine_part)
	{
		auto affine_value = get_expression_value(function.affine_part.value());
		value += affine_value;
	}
	return value;
}

template <CommercialSolverConstraint T>
double CommercialSolverMixin<T>::get_expression_value(const ExprBuilder &function)
{
	T *model = get_base();
	double value = 0.0;
	for (const auto &[varpair, coef] : function.quadratic_terms)
	{
		auto var1 = varpair.var_1;
		auto var2 = varpair.var_2;

		auto v1 = model->get_variable_value(var1);
		if (var1 == var2)
		{
			value += coef * v1 * v1;
		}
		else
		{
			auto v2 = model->get_variable_value(var2);
			value += coef * v1 * v2;
		}
	}
	for (const auto &[var, coef] : function.affine_terms)
	{
		value += coef * model->get_variable_value(var);
	}
	value += function.constant_term.value_or(0.0);
	return value;
}

template <CommercialSolverConstraint T>
std::string CommercialSolverMixin<T>::pprint_expression(const ScalarAffineFunction &function,
                                                        int precision)
{
	T *model = get_base();
	auto N = function.size();
	std::vector<std::string> terms;
	terms.reserve(N + 1);

	for (auto i = 0; i < N; ++i)
	{
		auto &coef = function.coefficients[i];
		std::string var_str = model->pprint_variable(function.variables[i]);
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

template <CommercialSolverConstraint T>
std::string CommercialSolverMixin<T>::pprint_expression(const ScalarQuadraticFunction &function,
                                                        int precision)
{
	T *model = get_base();
	auto N = function.size();
	std::vector<std::string> terms;
	terms.reserve(N + 1);

	for (auto i = 0; i < N; ++i)
	{
		auto &coef = function.coefficients[i];
		std::string var1_str = model->pprint_variable(function.variable_1s[i]);
		std::string var2_str = model->pprint_variable(function.variable_2s[i]);
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

template <CommercialSolverConstraint T>
std::string CommercialSolverMixin<T>::pprint_expression(const ExprBuilder &function, int precision)
{
	T *model = get_base();
	std::vector<std::string> terms;
	terms.reserve(function.quadratic_terms.size() + function.affine_terms.size() + 1);

	for (const auto &[varpair, coef] : function.quadratic_terms)
	{
		std::string var1_str = model->pprint_variable(varpair.var_1);
		std::string var2_str = model->pprint_variable(varpair.var_2);
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
		std::string var_str = model->pprint_variable(var);
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

struct AffineFunctionPtrForm
{
	int numnz;
	int *index;
	double *value;
	std::vector<int> index_storage;
};

struct QuadraticFunctionPtrForm
{
	int numnz;
	int *row;
	int *col;
	double *value;
	std::vector<int> row_storage;
	std::vector<int> col_storage;
};

template <typename T>
concept VarIndexModel = requires(T *model, const VariableIndex &v) {
	{
		model->_variable_index(v)
	} -> std::convertible_to<IndexT>;
};

template <VarIndexModel T>
void make_affine_ptr_form(T *model, const ScalarAffineFunction &function,
                          AffineFunctionPtrForm &ptr_form)
{
	int numnz = function.size();
	ptr_form.numnz = numnz;
	ptr_form.index_storage.resize(numnz);
	for (int i = 0; i < numnz; ++i)
	{
		ptr_form.index_storage[i] = model->_variable_index(function.variables[i]);
	}
	ptr_form.index = ptr_form.index_storage.data();
	ptr_form.value = (double *)function.coefficients.data();
}

template <VarIndexModel T>
void make_quadratic_ptr_form(T *model, const ScalarQuadraticFunction &function,
                             QuadraticFunctionPtrForm &ptr_form)
{
	int numnz = function.size();
	ptr_form.numnz = numnz;
	ptr_form.row_storage.resize(numnz);
	ptr_form.col_storage.resize(numnz);
	for (int i = 0; i < numnz; ++i)
	{
		ptr_form.row_storage[i] = model->_variable_index(function.variable_1s[i]);
		ptr_form.col_storage[i] = model->_variable_index(function.variable_2s[i]);
	}
	ptr_form.row = ptr_form.row_storage.data();
	ptr_form.col = ptr_form.col_storage.data();
	ptr_form.value = (double *)function.coefficients.data();
}