#include "pyoptinterface/nleval.hpp"
#include <cassert>

ConstraintAutodiffEvaluator::ConstraintAutodiffEvaluator(bool has_parameter, uintptr_t fp,
                                                         uintptr_t jp, uintptr_t hp)
{
	if (has_parameter)
	{
		f_eval.p = (f_funcptr)fp;
		jacobian_eval.p = (jacobian_funcptr)jp;
		hessian_eval.p = (hessian_funcptr)hp;
	}
	else
	{
		f_eval.nop = (f_funcptr_noparam)fp;
		jacobian_eval.nop = (jacobian_funcptr_noparam)jp;
		hessian_eval.nop = (hessian_funcptr_noparam)hp;
	}
}

ObjectiveAutodiffEvaluator::ObjectiveAutodiffEvaluator(bool has_parameter, uintptr_t fp,
                                                       uintptr_t ajp, uintptr_t hp)
{
	if (has_parameter)
	{
		f_eval.p = (f_funcptr)fp;
		grad_eval.p = (additive_grad_funcptr)ajp;
		hessian_eval.p = (hessian_funcptr)hp;
	}
	else
	{
		f_eval.nop = (f_funcptr_noparam)fp;
		grad_eval.nop = (additive_grad_funcptr_noparam)ajp;
		hessian_eval.nop = (hessian_funcptr_noparam)hp;
	}
}

void LinearEvaluator::add_row(const ScalarAffineFunction &f)
{
	coefs.insert(coefs.end(), f.coefficients.begin(), f.coefficients.end());
	indices.insert(indices.end(), f.variables.begin(), f.variables.end());
	constraint_intervals.push_back(coefs.size());

	if (f.constant)
	{
		constant_values.push_back(f.constant.value());
		constant_indices.push_back(n_constraints);
	}

	n_constraints += 1;
}

void LinearEvaluator::eval_function(const double *restrict x, double *restrict f)
{
	for (size_t i = 0; i < n_constraints; i++)
	{
		auto start = constraint_intervals[i];
		auto end = constraint_intervals[i + 1];

		double sum = 0.0;
		for (size_t j = start; j < end; j++)
		{
			sum += coefs[j] * x[indices[j]];
		}
		f[i] = sum;
	}
	for (size_t i = 0; i < constant_indices.size(); i++)
	{
		auto index = constant_indices[i];
		auto value = constant_values[i];
		f[index] += value;
	}
}

void LinearEvaluator::analyze_jacobian_structure(size_t &global_jacobian_nnz,
                                                 std::vector<int> &global_jacobian_rows,
                                                 std::vector<int> &global_jacobian_cols) const
{
	global_jacobian_nnz += indices.size();
	global_jacobian_rows.reserve(global_jacobian_nnz);
	for (size_t i = 0; i < n_constraints; i++)
	{
		auto start = constraint_intervals[i];
		auto end = constraint_intervals[i + 1];
		for (size_t j = start; j < end; j++)
		{
			global_jacobian_rows.push_back(i);
		}
	}
	global_jacobian_cols.insert(global_jacobian_cols.end(), indices.begin(), indices.end());
}

void LinearEvaluator::eval_jacobian(const double *restrict x, double *restrict jacobian) const
{
	std::copy(coefs.begin(), coefs.end(), jacobian);
}

void QuadraticEvaluator::add_row(const ScalarQuadraticFunction &f)
{
	for (int i = 0; i < f.size(); i++)
	{
		auto coef = f.coefficients[i];
		auto x1 = f.variable_1s[i];
		auto x2 = f.variable_2s[i];

		if (x1 == x2)
		{
			diag_coefs.push_back(coef);
			diag_indices.push_back(x1);
		}
		else
		{
			offdiag_coefs.push_back(coef);
			offdiag_rows.push_back(x1);
			offdiag_cols.push_back(x2);
		}
	}

	diag_intervals.push_back(diag_coefs.size());
	offdiag_intervals.push_back(offdiag_coefs.size());

	if (f.affine_part)
	{
		auto &affine = f.affine_part.value();
		linear_coefs.insert(linear_coefs.end(), affine.coefficients.begin(),
		                    affine.coefficients.end());
		linear_indices.insert(linear_indices.end(), affine.variables.begin(),
		                      affine.variables.end());

		if (affine.constant)
		{
			linear_constant_values.push_back(affine.constant.value());
			linear_constant_indices.push_back(n_constraints);
		}
	}
	linear_intervals.push_back(linear_coefs.size());

	Hashmap<int, int> variable_to_jacobian_nnz;

	for (int i = 0; i < f.size(); i++)
	{
		auto coef = f.coefficients[i];
		auto x1 = f.variable_1s[i];
		auto x2 = f.variable_2s[i];

		if (x1 == x2)
		{
			auto [iter, inserted] = variable_to_jacobian_nnz.try_emplace(x1, jacobian_nnz);
			if (inserted)
			{
				jacobian_constant.push_back(0.0);
				jacobian_variable_indices.push_back(x1);
				jacobian_nnz += 1;
			}
			jacobian_diag_indices.push_back(iter->second);
		}
		else
		{
			{
				auto [iter, inserted] = variable_to_jacobian_nnz.try_emplace(x1, jacobian_nnz);
				if (inserted)
				{
					jacobian_constant.push_back(0.0);
					jacobian_variable_indices.push_back(x1);
					jacobian_nnz += 1;
				}
				jacobian_offdiag_row_indices.push_back(iter->second);
			}
			{
				auto [iter, inserted] = variable_to_jacobian_nnz.try_emplace(x2, jacobian_nnz);
				if (inserted)
				{
					jacobian_constant.push_back(0.0);
					jacobian_variable_indices.push_back(x2);
					jacobian_nnz += 1;
				}
				jacobian_offdiag_col_indices.push_back(iter->second);
			}
		}
	}

	if (f.affine_part)
	{
		auto &affine = f.affine_part.value();

		for (int i = 0; i < affine.size(); i++)
		{
			auto coef = affine.coefficients[i];
			auto x = affine.variables[i];
			auto [iter, inserted] = variable_to_jacobian_nnz.try_emplace(x, jacobian_nnz);
			if (inserted)
			{
				jacobian_constant.push_back(coef);
				jacobian_variable_indices.push_back(x);
				jacobian_nnz += 1;
			}
			else
			{
				auto jacobian_index = iter->second;
				jacobian_constant[jacobian_index] += coef;
			}
		}
	}
	jacobian_constraint_intervals.push_back(jacobian_variable_indices.size());

	n_constraints += 1;
}

void QuadraticEvaluator::eval_function(const double *restrict x, double *restrict f) const
{
	for (size_t i = 0; i < n_constraints; i++)
	{
		auto start = diag_intervals[i];
		auto end = diag_intervals[i + 1];
		double sum = 0.0;
		for (size_t j = start; j < end; j++)
		{
			auto c = diag_coefs[j];
			auto v = x[diag_indices[j]];
			sum += c * v * v;
		}
		f[i] = sum;
	}
	for (size_t i = 0; i < n_constraints; i++)
	{
		auto start = offdiag_intervals[i];
		auto end = offdiag_intervals[i + 1];
		double sum = 0.0;
		for (size_t j = start; j < end; j++)
		{
			auto c = offdiag_coefs[j];
			auto v1 = x[offdiag_rows[j]];
			auto v2 = x[offdiag_cols[j]];
			sum += c * v1 * v2;
		}
		f[i] += sum;
	}
	for (size_t i = 0; i < n_constraints; i++)
	{
		auto start = linear_intervals[i];
		auto end = linear_intervals[i + 1];

		double sum = 0.0;
		for (size_t j = start; j < end; j++)
		{
			sum += linear_coefs[j] * x[linear_indices[j]];
		}
		f[i] += sum;
	}
	for (size_t i = 0; i < linear_constant_indices.size(); i++)
	{
		auto index = linear_constant_indices[i];
		auto value = linear_constant_values[i];
		f[index] += value;
	}
}

void QuadraticEvaluator::analyze_jacobian_structure(size_t row_base, size_t &global_jacobian_nnz,
                                                    std::vector<int> &global_jacobian_rows,
                                                    std::vector<int> &global_jacobian_cols) const
{
	global_jacobian_nnz += jacobian_nnz;
	global_jacobian_rows.reserve(global_jacobian_nnz);
	for (size_t i = 0; i < n_constraints; i++)
	{
		auto start = jacobian_constraint_intervals[i];
		auto end = jacobian_constraint_intervals[i + 1];
		for (size_t j = start; j < end; j++)
		{
			global_jacobian_rows.push_back(row_base + i);
		}
	}
	global_jacobian_cols.insert(global_jacobian_cols.end(), jacobian_variable_indices.begin(),
	                            jacobian_variable_indices.end());
}

void QuadraticEvaluator::eval_jacobian(const double *restrict x, double *restrict jacobian) const
{
	std::copy(jacobian_constant.begin(), jacobian_constant.end(), jacobian);

	for (int i = 0; i < diag_coefs.size(); i++)
	{
		auto coef = diag_coefs[i];
		auto x_index = diag_indices[i];
		auto jacobian_index = jacobian_diag_indices[i];
		jacobian[jacobian_index] += 2.0 * coef * x[x_index];
	}

	for (int i = 0; i < offdiag_coefs.size(); i++)
	{
		auto coef = offdiag_coefs[i];
		auto x1_index = offdiag_rows[i];
		auto x2_index = offdiag_cols[i];
		auto jacobian_index = jacobian_offdiag_row_indices[i];
		jacobian[jacobian_index] += coef * x[x2_index];
		jacobian_index = jacobian_offdiag_col_indices[i];
		jacobian[jacobian_index] += coef * x[x1_index];
	}
}

void QuadraticEvaluator::analyze_hessian_structure(
    size_t &global_hessian_nnz, std::vector<int> &global_hessian_rows,
    std::vector<int> &global_hessian_cols, Hashmap<std::tuple<int, int>, int> &hessian_index_map,
    HessianSparsityType hessian_type)
{
	hessian_diag_indices.resize(diag_coefs.size());
	hessian_offdiag_indices.resize(offdiag_coefs.size());

	for (int i = 0; i < diag_coefs.size(); i++)
	{
		auto x = diag_indices[i];

		auto [iter, inserted] = hessian_index_map.try_emplace({x, x}, global_hessian_nnz);
		if (inserted)
		{
			global_hessian_rows.push_back(x);
			global_hessian_cols.push_back(x);
			global_hessian_nnz += 1;
		}
		auto hessian_index = iter->second;
		hessian_diag_indices[i] = hessian_index;
	}

	for (int i = 0; i < offdiag_coefs.size(); i++)
	{
		auto x1 = offdiag_rows[i];
		auto x2 = offdiag_cols[i];
		if (hessian_type == HessianSparsityType::Upper)
		{
			if (x1 > x2)
				std::swap(x1, x2);
		}
		else
		{
			if (x1 < x2)
				std::swap(x1, x2);
		}
		auto [iter, inserted] = hessian_index_map.try_emplace({x1, x2}, global_hessian_nnz);
		if (inserted)
		{
			global_hessian_rows.push_back(x1);
			global_hessian_cols.push_back(x2);
			global_hessian_nnz += 1;
		}
		auto hessian_index = iter->second;
		hessian_offdiag_indices[i] = hessian_index;
	}
}

void QuadraticEvaluator::eval_lagrangian_hessian(const double *restrict lambda,
                                                 double *restrict hessian) const
{
	for (size_t i = 0; i < n_constraints; i++)
	{
		auto start = diag_intervals[i];
		auto end = diag_intervals[i + 1];
		auto multiplier = lambda[i];
		for (size_t j = start; j < end; j++)
		{
			auto coef = diag_coefs[j];
			auto hessian_index = hessian_diag_indices[j];
			hessian[hessian_index] += 2.0 * coef * multiplier;
		}
	}
	for (size_t i = 0; i < n_constraints; i++)
	{
		auto start = offdiag_intervals[i];
		auto end = offdiag_intervals[i + 1];
		auto multiplier = lambda[i];
		for (size_t j = start; j < end; j++)
		{
			auto coef = offdiag_coefs[j];
			auto hessian_index = hessian_offdiag_indices[j];
			hessian[hessian_index] += coef * multiplier;
		}
	}
}
