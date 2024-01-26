#pragma once

#include <string>
#include <concepts>
#include "fmt/format.h"
#include "pyoptinterface/core.hpp"

template <typename CommercialSolverT>
concept CommercialSolverConstraint = requires(CommercialSolverT *model, const char *name) {
	{
		model->add_linear_constraint(std::declval<const ScalarAffineFunction &>(),
		                             ConstraintSense{}, CoeffT{}, name)
	} -> std::same_as<ConstraintIndex>;
	{
		model->add_quadratic_constraint(std::declval<const ScalarQuadraticFunction &>(),
		                                ConstraintSense{}, CoeffT{}, name)
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
	                                                ConstraintSense sense, CoeffT rhs,
	                                                const char *name = nullptr);
	ConstraintIndex add_quadratic_constraint_from_expr(const ExprBuilder &function,
	                                                   ConstraintSense sense, CoeffT rhs,
	                                                   const char *name = nullptr);

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
    const ExprBuilder &function, ConstraintSense sense, CoeffT rhs, const char *name)
{
	ScalarAffineFunction f(function);
	return get_base()->add_linear_constraint(f, sense, rhs, name);
}

template <CommercialSolverConstraint T>
ConstraintIndex CommercialSolverMixin<T>::add_quadratic_constraint_from_expr(
    const ExprBuilder &function, ConstraintSense sense, CoeffT rhs, const char *name)
{
	ScalarQuadraticFunction f(function);
	return get_base()->add_quadratic_constraint(f, sense, rhs, name);
}

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
		std::string var2_str;
		if (function.variable_1s[i] == function.variable_2s[i])
		{
			var2_str = var1_str;
		}
		else
		{
			var2_str = model->pprint_variable(function.variable_2s[i]);
		}
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
		std::string var2_str;
		if (varpair.var_1 == varpair.var_2)
		{
			var2_str = var1_str;
		}
		else
		{
			var2_str = model->pprint_variable(varpair.var_2);
		}
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

template <typename T>
concept VarIndexModel = requires(T *model, const VariableIndex &v) {
	{
		model->_variable_index(v)
	} -> std::convertible_to<IndexT>;
};

template <std::integral NZT, std::integral IDXT, std::floating_point VALT>
struct AffineFunctionPtrForm
{
	NZT numnz;
	IDXT *index;
	VALT *value;
	std::vector<IDXT> index_storage;
	std::vector<VALT> value_storage;

	template <VarIndexModel T>
	void make(T *model, const ScalarAffineFunction &function)
	{
		auto f_numnz = function.size();
		numnz = f_numnz;
		index_storage.resize(numnz);
		for (int i = 0; i < numnz; ++i)
		{
			index_storage[i] = model->_variable_index(function.variables[i]);
		}
		index = index_storage.data();
		if constexpr (std::is_same_v<VALT, CoeffT>)
		{
			value = (VALT *)function.coefficients.data();
		}
		else
		{
			value_storage.resize(numnz);
			for (int i = 0; i < numnz; ++i)
			{
				value_storage[i] = function.coefficients[i];
			}
		}
	}
};

template <std::integral NZT, std::integral IDXT, std::floating_point VALT>
struct QuadraticFunctionPtrForm
{
	NZT numnz;
	IDXT *row;
	IDXT *col;
	VALT *value;
	std::vector<IDXT> row_storage;
	std::vector<IDXT> col_storage;
	std::vector<VALT> value_storage;

	template <VarIndexModel T>
	void make(T *model, const ScalarQuadraticFunction &function)
	{
		auto f_numnz = function.size();
		numnz = f_numnz;
		row_storage.resize(numnz);
		col_storage.resize(numnz);
		for (int i = 0; i < numnz; ++i)
		{
			row_storage[i] = model->_variable_index(function.variable_1s[i]);
			if (function.variable_1s[i] == function.variable_2s[i])
			{
				col_storage[i] = row_storage[i];
			}
			else
			{
				col_storage[i] = model->_variable_index(function.variable_2s[i]);
			}
		}
		row = row_storage.data();
		col = col_storage.data();
		if constexpr (std::is_same_v<VALT, CoeffT>)
		{
			value = (VALT *)function.coefficients.data();
		}
		else
		{
			value_storage.resize(numnz);
			for (int i = 0; i < numnz; ++i)
			{
				value_storage[i] = function.coefficients[i];
			}
		}
	}
};
