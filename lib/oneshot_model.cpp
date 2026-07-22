#include "pyoptinterface/oneshot_model.hpp"
#include <numeric>
#include "fmt/core.h"
#include "fmt/format.h"
#include "fmt/args.h"

VariableIndex OneShotModel::add_variable(VariableDomain domain, double lb, double ub,
                                         const char *name)
{
	int index = num_variables;
	num_variables++;

	variable_lbs.push_back(lb);
	variable_ubs.push_back(ub);
	variable_domains.push_back(domain);

	variable_names.add_name(name);

	return VariableIndex(index);
}

int OneShotModel::batch_add_variables(const std::vector<int> &shape, VariableDomain domain, double lb, double ub,
                                       const char *name)
{
	int index = num_variables;
	auto N = std::accumulate(shape.begin(), shape.end(), 1, std::multiplies<>());
	num_variables += N;

	variable_domains.insert(variable_domains.end(), N, domain);
	variable_lbs.insert(variable_lbs.end(), N, lb);
	variable_ubs.insert(variable_ubs.end(), N, ub);

	if (name && name[0] != '\0')
	{
		// Precompute strides for row-major (C order) index computation
		int ndim = static_cast<int>(shape.size());
		std::vector<int> strides(ndim);
		if (ndim > 0)
		{
			strides[ndim - 1] = 1;
			for (int d = ndim - 2; d >= 0; d--)
			{
				strides[d] = strides[d + 1] * shape[d + 1];
			}
		}

		// Pre-build format string: "name({}, {}, ...)" or "name({},)" for 1D
		// to match Python's str(tuple) convention
		std::string fmt_str;
		fmt_str.append(name);
		fmt_str.push_back('(');
		for (int d = 0; d < ndim; d++)
		{
			if (d > 0)
				fmt_str.append(", ");
			fmt_str.append("{}");
		}
		if (ndim == 1)
			fmt_str.push_back(',');
		fmt_str.push_back(')');

		// Reuse buffer and arg store across iterations
		fmt::memory_buffer buf;
		fmt::dynamic_format_arg_store<fmt::format_context> store;
		store.reserve(ndim, 0);

		for (int i = 0; i < N; i++)
		{
			buf.clear();
			store.clear();

			int remainder = i;
			for (int d = 0; d < ndim; d++)
			{
				int idx = remainder / strides[d];
				remainder %= strides[d];
				store.push_back(idx);
			}

			fmt::vformat_to(std::back_inserter(buf), fmt_str, store);
			variable_names.add_name(std::string_view(buf.data(), buf.size()));
		}
	}
	else
	{
		variable_names.batch_add_empty(N);
	}

	return index;
}

ConstraintIndex OneShotModel::add_linear_constraint(const ScalarAffineFunction &function,
                                                     ConstraintSense sense, CoeffT rhs,
                                                     const char *name)
{
	int index = num_linear_constraints;
	num_linear_constraints++;

	std::span<const IndexT> vars(function.variables.data(), function.variables.size());
	std::span<const CoeffT> coefs(function.coefficients.data(), function.coefficients.size());
	A_cache.add_row(vars, coefs);

	linear_con_senses.push_back(sense);

	// Convert sense + rhs into lb/ub representation
	double con_rhs = rhs;
	if (function.constant)
	{
		con_rhs -= function.constant.value();
	}
	switch (sense)
	{
	case ConstraintSense::LessEqual:
		linear_con_lbs.push_back(-inf_d);
		linear_con_ubs.push_back(con_rhs);
		break;
	case ConstraintSense::GreaterEqual:
		linear_con_lbs.push_back(con_rhs);
		linear_con_ubs.push_back(inf_d);
		break;
	case ConstraintSense::Equal:
		linear_con_lbs.push_back(con_rhs);
		linear_con_ubs.push_back(con_rhs);
		break;
	default:
		throw std::runtime_error("Unsupported constraint sense for linear constraint");
	}

	linear_con_names.add_name(name);

	return ConstraintIndex(ConstraintType::Linear, index);
}

void OneShotModel::set_objective(const ScalarAffineFunction &function, ObjectiveSense sense)
{
	linear_objective = function;
	objective_sense = sense;
}
