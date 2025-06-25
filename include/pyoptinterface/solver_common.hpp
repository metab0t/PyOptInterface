#pragma once

#include <algorithm>
#include <string>
#include <concepts>
#include <tuple>
#include <numeric>
#include "fmt/format.h"
#include "fmt/ranges.h"
#include "pyoptinterface/core.hpp"

template <typename T>
concept OnesideLinearConstraintMixinConcept = requires(T &model) {
	{
		model.add_linear_constraint(ScalarAffineFunction(), ConstraintSense::Equal, 0.0, "")
	} -> std::same_as<ConstraintIndex>;
};

template <typename T>
class OnesideLinearConstraintMixin
{
  private:
	T *get_base()
	{
		static_assert(OnesideLinearConstraintMixinConcept<T>);
		return static_cast<T *>(this);
	}

  public:
	ConstraintIndex add_linear_constraint_from_var(const VariableIndex &variable,
	                                               ConstraintSense sense, CoeffT rhs,
	                                               const char *name = nullptr)
	{
		ScalarAffineFunction f(variable);
		return get_base()->add_linear_constraint(f, sense, rhs, name);
	}
	ConstraintIndex add_linear_constraint_from_expr(const ExprBuilder &function,
	                                                ConstraintSense sense, CoeffT rhs,
	                                                const char *name = nullptr)
	{
		if (function.degree() >= 2)
		{
			throw std::runtime_error("add_linear_constraint expects linear expression but receives a quadratic expression.");
		}
		ScalarAffineFunction f(function);
		return get_base()->add_linear_constraint(f, sense, rhs, name);
	}
};

template <typename T>
concept TwosideLinearConstraintMixinConcept = requires(T &model) {
	{
		model.add_linear_constraint(ScalarAffineFunction(), std::make_tuple(0.0, 1.0), "")
	} -> std::same_as<ConstraintIndex>;
};

template <typename T>
class TwosideLinearConstraintMixin
{
  private:
	T *get_base()
	{
		static_assert(TwosideLinearConstraintMixinConcept<T>);
		return static_cast<T *>(this);
	}

  public:
	ConstraintIndex add_linear_interval_constraint_from_var(
	    const VariableIndex &variable, const std::tuple<double, double> &interval,
	    const char *name = nullptr)
	{
		ScalarAffineFunction f(variable);
		return get_base()->add_linear_constraint(f, interval, name);
	}
	ConstraintIndex add_linear_interval_constraint_from_expr(
	    const ExprBuilder &function, const std::tuple<double, double> &interval,
	    const char *name = nullptr)
	{
		ScalarAffineFunction f(function);
		return get_base()->add_linear_constraint(f, interval, name);
	}
};

template <typename T>
concept OnesideQuadraticConstraintMixinConcept = requires(T &model) {
	{
		model.add_quadratic_constraint(ScalarQuadraticFunction(), ConstraintSense::Equal, 0.0, "")
	} -> std::same_as<ConstraintIndex>;
};

template <typename T>
class OnesideQuadraticConstraintMixin
{
  private:
	T *get_base()
	{
		static_assert(OnesideQuadraticConstraintMixinConcept<T>);
		return static_cast<T *>(this);
	}

  public:
	ConstraintIndex add_quadratic_constraint_from_var(const VariableIndex &variable,
	                                                  ConstraintSense sense, CoeffT rhs,
	                                                  const char *name = nullptr)
	{
		ScalarQuadraticFunction f(variable);
		return get_base()->add_quadratic_constraint(f, sense, rhs, name);
	}
	ConstraintIndex add_quadratic_constraint_from_saf(const ScalarAffineFunction &function,
	                                                  ConstraintSense sense, CoeffT rhs,
	                                                  const char *name = nullptr)
	{
		ScalarQuadraticFunction f(function);
		return get_base()->add_quadratic_constraint(f, sense, rhs, name);
	}
	ConstraintIndex add_quadratic_constraint_from_expr(const ExprBuilder &function,
	                                                   ConstraintSense sense, CoeffT rhs,
	                                                   const char *name = nullptr)
	{
		ScalarQuadraticFunction f(function);
		return get_base()->add_quadratic_constraint(f, sense, rhs, name);
	}
};

template <typename T>
concept TwosideQuadraticConstraintMixinConcept = requires(T &model) {
	{
		model.add_quadratic_constraint(ScalarQuadraticFunction(), std::make_tuple(0.0, 1.0), "")
	} -> std::same_as<ConstraintIndex>;
};

template <typename T>
class TwosideQuadraticConstraintMixin
{
  private:
	T *get_base()
	{
		static_assert(TwosideQuadraticConstraintMixinConcept<T>);
		return static_cast<T *>(this);
	}

  public:
	ConstraintIndex add_quadratic_interval_constraint_from_var(
	    const VariableIndex &variable, const std::tuple<double, double> &interval,
	    const char *name = nullptr)
	{
		ScalarQuadraticFunction f(variable);
		return get_base()->add_quadratic_constraint(f, interval, name);
	}
	ConstraintIndex add_quadratic_interval_constraint_from_expr(
	    const ExprBuilder &function, const std::tuple<double, double> &interval,
	    const char *name = nullptr)
	{
		ScalarQuadraticFunction f(function);
		return get_base()->add_quadratic_constraint(f, interval, name);
	}
};

#ifdef USE_NLMIXIN
#include "pyoptinterface/nlexpr.hpp"

template <typename T>
concept TwosideNLConstraintMixinConcept = requires(T &model) {
	{
		model.add_single_nl_constraint(std::declval<ExpressionGraph &>(), ExpressionHandle(),
		                               std::make_tuple(0.0, 1.0), "")
	} -> std::same_as<ConstraintIndex>;
	{ model.get_infinity() } -> std::convertible_to<double>;
};

template <typename T>
class TwosideNLConstraintMixin
{
  private:
	T *get_base()
	{
		static_assert(TwosideNLConstraintMixinConcept<T>);
		return static_cast<T *>(this);
	}

  public:
	ConstraintIndex add_single_nl_constraint_sense_rhs(ExpressionGraph &graph,
	                                                   const ExpressionHandle &result,
	                                                   ConstraintSense sense, CoeffT rhs,
	                                                   const char *name = nullptr)
	{
		double infinity = get_base()->get_infinity();

		double lb, ub;

		if (sense == ConstraintSense::Equal)
		{
			lb = rhs;
			ub = rhs;
		}
		else if (sense == ConstraintSense::LessEqual)
		{
			lb = -infinity;
			ub = rhs;
		}
		else if (sense == ConstraintSense::GreaterEqual)
		{
			lb = rhs;
			ub = infinity;
		}

		return get_base()->add_single_nl_constraint(graph, result, {lb, ub}, name);
	}

	ConstraintIndex add_single_nl_constraint_from_comparison(ExpressionGraph &graph,
	                                                         const ExpressionHandle &expr,
	                                                         const char *name)
	{
		ExpressionHandle real_expr;
		double lb = -get_base()->get_infinity(), ub = get_base()->get_infinity();
		unpack_comparison_expression(graph, expr, real_expr, lb, ub);
		auto constraint = get_base()->add_single_nl_constraint(graph, real_expr, {lb, ub}, name);
		return constraint;
	}
};
#endif

template <typename T>
concept GetValueMixinConcept = requires(T &model) {
	{ model.get_variable_value(VariableIndex()) } -> std::convertible_to<double>;
};

template <typename T>
class GetValueMixin
{
  private:
	T *get_base()
	{
		static_assert(GetValueMixinConcept<T>);
		return static_cast<T *>(this);
	}

  public:
	double get_expression_value(const ScalarAffineFunction &function)
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
	double get_expression_value(const ScalarQuadraticFunction &function)
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
			auto affine_value = model->get_expression_value(function.affine_part.value());
			value += affine_value;
		}
		return value;
	}
	double get_expression_value(const ExprBuilder &function)
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
};

template <typename T>
concept PPrintMixinConcept = requires(T &model) {
	{ model.pprint_variable(VariableIndex()) } -> std::convertible_to<std::string>;
};

template <typename T>
class PPrintMixin
{
  private:
	T *get_base()
	{
		static_assert(PPrintMixinConcept<T>);
		return static_cast<T *>(this);
	}

  public:
	std::string pprint_expression(const ScalarAffineFunction &function, int precision = 4)
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
	std::string pprint_expression(const ScalarQuadraticFunction &function, int precision = 4)
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
			auto affine_value = model->pprint_expression(function.affine_part.value(), precision);
			terms.push_back(affine_value);
		}
		return fmt::format("{}", fmt::join(terms, "+"));
	}
	std::string pprint_expression(const ExprBuilder &function, int precision = 4)
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
			auto term = fmt::format("{:.{}g}*{}", coef, precision, model->pprint_variable(var));
			terms.push_back(term);
		}
		if (function.constant_term)
		{
			auto term = fmt::format("{:.{}g}", function.constant_term.value(), precision);
			terms.push_back(term);
		}
		return fmt::format("{}", fmt::join(terms, "+"));
	}
};

template <typename T>
concept LinearObjectiveMixinConcept = requires(T &model) {
	{ model.set_objective(ScalarAffineFunction(), ObjectiveSense()) } -> std::same_as<void>;
};

template <typename T>
class LinearObjectiveMixin
{
  private:
	T *get_base()
	{
		static_assert(LinearObjectiveMixinConcept<T>);
		return static_cast<T *>(this);
	}

  public:
	void set_objective_as_constant(CoeffT c, ObjectiveSense sense)
	{
		T *model = get_base();
		model->set_objective(ScalarAffineFunction(c), sense);
	}
	void set_objective_as_variable(const VariableIndex &variable, ObjectiveSense sense)
	{
		T *model = get_base();
		model->set_objective(ScalarAffineFunction(variable), sense);
	}
};

/* This concept combined with partial specialization causes ICE on gcc 10 */
// template <typename T>
// concept VarIndexModel = requires(T *model, const VariableIndex &v) {
//	{
//		model->_variable_index(v)
//	} -> std::convertible_to<IndexT>;
// };
#define VarIndexModel typename

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

enum class HessianTriangular
{
	Upper,
	Lower,
};

template <std::integral NZT, std::integral IDXT, std::floating_point VALT>
struct CSCMatrix
{
	NZT numnz;
	NZT numcol;
	std::vector<VALT> values_CSC;
	std::vector<IDXT> rows_CSC;
	std::vector<IDXT> colStarts_CSC;

	template <VarIndexModel T>
	void make(T *model, const ScalarQuadraticFunction &function, int i_numcol,
	          HessianTriangular triangular_format)
	{
		auto f_numnz = function.size();
		numnz = f_numnz;
		numcol = i_numcol;

		std::vector<IDXT> rows(numnz);   // Row indices
		std::vector<IDXT> cols(numnz);   // Column indices
		std::vector<VALT> values(numnz); // Values
		for (int i = 0; i < numnz; ++i)
		{
			auto v1 = model->_variable_index(function.variable_1s[i]);
			auto v2 = v1;
			if (function.variable_1s[i] != function.variable_2s[i])
			{
				v2 = model->_variable_index(function.variable_2s[i]);
			}

			if (triangular_format == HessianTriangular::Upper)
			{
				if (v1 > v2)
				{
					std::swap(v1, v2);
				}
			}
			else
			{
				if (v1 < v2)
				{
					std::swap(v1, v2);
				}
			}

			if (v1 < 0 || v2 < 0)
			{
				throw std::runtime_error(
				    "Variable index in quadratic function cannot be negative!");
			}

			rows[i] = v1;
			cols[i] = v2;

			auto coef = function.coefficients[i];
			if (v1 != v2)
			{
				// Non-diagonal element, should multiply by 0.5
				coef *= 0.5;
			}
			values[i] = coef;
		}

		// Sorting based on column indices
		std::vector<IDXT> idx(numnz);
		std::iota(idx.begin(), idx.end(), 0);
		std::sort(idx.begin(), idx.end(), [&](int i, int j) { return cols[i] < cols[j]; });

		// Creating CSC arrays
		values_CSC.reserve(numnz);
		rows_CSC.reserve(numnz);
		colStarts_CSC.resize(numcol + 1, 0);

		int currentCol = 0;
		for (auto i : idx)
		{
			while (currentCol < cols[i])
			{
				colStarts_CSC[currentCol + 1] = values_CSC.size();
				currentCol++;
			}
			values_CSC.push_back(values[i]);
			rows_CSC.push_back(rows[i]);
		}

		// Filling up remaining columns in colStarts_CSC
		std::fill(colStarts_CSC.begin() + currentCol + 1, colStarts_CSC.end(), numnz);
	}
};
