#pragma once

#include <concepts>
#include <vector>
#include <span>

// This file defines some common utilities to store the optimization model in a compact way

template <std::integral ColumnIndexT, std::integral VariableIndexT,
          std::floating_point CoefficientT>
struct LinearExpressionCache
{
	std::vector<ColumnIndexT> column_ptr = {0};
	std::vector<VariableIndexT> variables;
	std::vector<CoefficientT> coefficients;

	template <std::integral IT, std::floating_point FT>
	void add_row(std::span<const IT> row_variables, std::span<const FT> row_coefficients)
	{
		variables.insert(variables.end(), row_variables.begin(), row_variables.end());
		coefficients.insert(coefficients.end(), row_coefficients.begin(), row_coefficients.end());
		column_ptr.push_back(variables.size());
	}
};

template <std::integral ColumnIndexT, std::integral VariableIndexT,
          std::floating_point CoefficientT>
struct QuadraticExpressionCache
{
	std::vector<ColumnIndexT> column_ptr = {0};
	std::vector<VariableIndexT> variable_1s;
	std::vector<VariableIndexT> variable_2s;
	std::vector<CoefficientT> coefficients;

	std::vector<ColumnIndexT> lin_column_ptr = {0};
	std::vector<VariableIndexT> lin_variables;
	std::vector<CoefficientT> lin_coefficients;

	template <std::integral IT, std::floating_point FT>
	void add_row(std::span<const IT> row_variable_1s, std::span<const IT> row_variable_2s,
	             std::span<const FT> row_quadratic_coefficients,
	             std::span<const IT> row_lin_variables, std::span<const FT> row_lin_coefficients)
	{
		variable_1s.insert(variable_1s.end(), row_variable_1s.begin(), row_variable_1s.end());
		variable_2s.insert(variable_2s.end(), row_variable_2s.begin(), row_variable_2s.end());
		coefficients.insert(coefficients.end(), row_quadratic_coefficients.begin(),
		                    row_quadratic_coefficients.end());
		column_ptr.push_back(variable_1s.size());

		lin_variables.insert(lin_variables.end(), row_lin_variables.begin(),
		                     row_lin_variables.end());
		lin_coefficients.insert(lin_coefficients.end(), row_lin_coefficients.begin(),
		                        row_lin_coefficients.end());
		lin_column_ptr.push_back(lin_variables.size());
	}
};
