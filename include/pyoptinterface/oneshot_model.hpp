#pragma once

#include <concepts>
#include <vector>
#include <span>

#include "pyoptinterface/core.hpp"

// This file defines some common utilities to store the optimization model in a compact way

template <std::integral ColumnIndexT, std::integral VariableIndexT,
          std::floating_point CoefficientT>
struct LinearExpressionCache
{
	std::vector<ColumnIndexT> row_ptr = {0};
	std::vector<VariableIndexT> variables;
	std::vector<CoefficientT> coefficients;

	template <std::integral IT, std::floating_point FT>
	void add_row(std::span<const IT> row_variables, std::span<const FT> row_coefficients,
	             bool deduplicate = false)
	{
		if (!deduplicate)
		{
			variables.insert(variables.end(), row_variables.begin(), row_variables.end());
			coefficients.insert(coefficients.end(), row_coefficients.begin(),
			                    row_coefficients.end());
		}
		else
		{
			Hashmap<VariableIndexT, CoefficientT> var_coef_map;
			var_coef_map.reserve(row_variables.size());
			for (size_t i = 0; i < row_variables.size(); i++)
			{
				VariableIndexT var = static_cast<VariableIndexT>(row_variables[i]);
				CoefficientT coef = static_cast<CoefficientT>(row_coefficients[i]);
				auto [iter, inserted] = var_coef_map.try_emplace(var, coef);
				if (!inserted)
				{
					iter->second += coef;
				}
			}
			for (const auto &[var, coef] : var_coef_map)
			{
				variables.push_back(var);
				coefficients.push_back(coef);
			}
		}
		row_ptr.push_back(variables.size());
	}
};

template <std::integral ColumnIndexT, std::integral VariableIndexT,
          std::floating_point CoefficientT>
struct QuadraticExpressionCache
{
	std::vector<ColumnIndexT> row_ptr = {0};
	std::vector<VariableIndexT> variable_1s;
	std::vector<VariableIndexT> variable_2s;
	std::vector<CoefficientT> coefficients;

	std::vector<ColumnIndexT> lin_row_ptr = {0};
	std::vector<VariableIndexT> lin_variables;
	std::vector<CoefficientT> lin_coefficients;

	template <std::integral IT, std::floating_point FT>
	void add_row(std::span<const IT> row_variable_1s, std::span<const IT> row_variable_2s,
	             std::span<const FT> row_quadratic_coefficients,
	             std::span<const IT> row_lin_variables, std::span<const FT> row_lin_coefficients,
	             bool deduplicate = false)
	{
		if (!deduplicate)
		{
			variable_1s.insert(variable_1s.end(), row_variable_1s.begin(), row_variable_1s.end());
			variable_2s.insert(variable_2s.end(), row_variable_2s.begin(), row_variable_2s.end());
			coefficients.insert(coefficients.end(), row_quadratic_coefficients.begin(),
			                    row_quadratic_coefficients.end());

			lin_variables.insert(lin_variables.end(), row_lin_variables.begin(),
			                     row_lin_variables.end());
			lin_coefficients.insert(lin_coefficients.end(), row_lin_coefficients.begin(),
			                        row_lin_coefficients.end());
		}
		else
		{
			// Deduplicate quadratic terms: pack (var1, var2) into uint64_t key
			static_assert(sizeof(VariableIndexT) <= 4,
			              "VariableIndexT must be at most 32 bits for packing into uint64_t");
			auto pack_key = [](VariableIndexT v1, VariableIndexT v2) -> uint64_t {
				return (static_cast<uint64_t>(static_cast<uint32_t>(v1)) << 32) |
				       static_cast<uint64_t>(static_cast<uint32_t>(v2));
			};
			Hashmap<uint64_t, CoefficientT> quad_map;
			quad_map.reserve(row_variable_1s.size());
			for (size_t i = 0; i < row_variable_1s.size(); i++)
			{
				VariableIndexT v1 = static_cast<VariableIndexT>(row_variable_1s[i]);
				VariableIndexT v2 = static_cast<VariableIndexT>(row_variable_2s[i]);
				uint64_t key = pack_key(v1, v2);
				CoefficientT coef = static_cast<CoefficientT>(row_quadratic_coefficients[i]);
				auto [iter, inserted] = quad_map.try_emplace(key, coef);
				if (!inserted)
				{
					iter->second += coef;
				}
			}
			for (const auto &[key, coef] : quad_map)
			{
				VariableIndexT v1 = static_cast<VariableIndexT>(key >> 32);
				VariableIndexT v2 = static_cast<VariableIndexT>(key & 0xFFFFFFFF);
				variable_1s.push_back(v1);
				variable_2s.push_back(v2);
				coefficients.push_back(coef);
			}

			// Deduplicate linear terms
			Hashmap<VariableIndexT, CoefficientT> lin_map;
			lin_map.reserve(row_lin_variables.size());
			for (size_t i = 0; i < row_lin_variables.size(); i++)
			{
				VariableIndexT var = static_cast<VariableIndexT>(row_lin_variables[i]);
				CoefficientT coef = static_cast<CoefficientT>(row_lin_coefficients[i]);
				auto [iter, inserted] = lin_map.try_emplace(var, coef);
				if (!inserted)
				{
					iter->second += coef;
				}
			}
			for (const auto &[var, coef] : lin_map)
			{
				lin_variables.push_back(var);
				lin_coefficients.push_back(coef);
			}
		}
		row_ptr.push_back(variable_1s.size());
		lin_row_ptr.push_back(lin_variables.size());
	}
};

class CompactNameStorage
{
  private:
	std::vector<char> characters = {'\0'};
	std::vector<uint32_t> start_indices;

  public:
	void add_name(std::string_view name)
	{
		if (name.empty())
		{
			add_empty();
			return;
		}

		start_indices.push_back(static_cast<uint32_t>(characters.size()));
		characters.insert(characters.end(), name.begin(), name.end());

		// Null terminate the string for C compatibility
		characters.push_back('\0');
	}

	void add_empty()
	{
		start_indices.push_back(0);
	}

	void batch_add_empty(int N)
	{
		start_indices.insert(start_indices.end(), N, 0);
	}

	bool is_essentially_empty() const
	{
		return characters.size() == 1;
	}

	const char *c_str(int index) const
	{
		auto start = start_indices[index];
		const char *base_ptr = characters.data();
		return base_ptr + start;
	}

	std::vector<const char *> c_str_array() const
	{
		std::vector<const char *> result;
		result.reserve(start_indices.size());
		for (int index = 0; index < start_indices.size(); index++)
		{
			result.push_back(c_str(index));
		}
		return result;
	}
};

constexpr double inf_d = std::numeric_limits<double>::infinity();

struct OneShotModel
{
	// Variable
	int num_variables = 0;
	std::vector<double> variable_lbs, variable_ubs;
	std::vector<VariableDomain> variable_domains;
	CompactNameStorage variable_names;

	// Linear constraint
	int num_linear_constraints = 0;
	LinearExpressionCache<int, int, double> A_cache;
	std::vector<ConstraintSense> linear_con_senses;
	std::vector<double> linear_con_lbs, linear_con_ubs;
	CompactNameStorage linear_con_names;

	// Linear objective
	ScalarAffineFunction linear_objective;
	ObjectiveSense objective_sense;

	VariableIndex add_variable(VariableDomain domain = VariableDomain::Continuous,
	                           double lb = -inf_d, double ub = inf_d, const char *name = nullptr);
	int batch_add_variables(const std::vector<int> &shape, VariableDomain domain = VariableDomain::Continuous,
	                        double lb = -inf_d, double ub = inf_d, const char *name = nullptr);
	ConstraintIndex add_linear_constraint(const ScalarAffineFunction &function,
	                                      ConstraintSense sense, CoeffT rhs,
	                                      const char *name = nullptr);
	void set_objective(const ScalarAffineFunction &function, ObjectiveSense sense);
};