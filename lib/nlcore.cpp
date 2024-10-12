#include <execution>
#include "pyoptinterface/nlcore.hpp"

void NonlinearFunction::init(ADFunD &f_, const std::string &name_,
                             const std::vector<double> &x_values,
                             const std::vector<double> &p_values)
{
	nx = f_.Domain();
	np = f_.size_dyn_ind();
	assert(x_values.size() == nx);
	assert(p_values.size() == np);

	has_parameter = np > 0;
	ny = f_.Range();
	name = name_;

	f_.to_graph(f_graph);

	auto sparsity = jacobian_hessian_sparsity(f_, HessianSparsityType::Upper);
	// printf("jacobian_hessian_sparsity success\n");

	{
		auto &pattern = sparsity.jacobian;
		for (int i = 0; i < pattern.nnz(); i++)
		{
			auto r = pattern.row()[i];
			auto c = pattern.col()[i];
			m_jacobian_rows.push_back(r);
			m_jacobian_cols.push_back(c);
		}
		m_jacobian_nnz = pattern.nnz();
	}

	{
		auto &pattern = sparsity.reduced_hessian;
		for (int i = 0; i < pattern.nnz(); i++)
		{
			auto r = pattern.row()[i];
			auto c = pattern.col()[i];
			m_hessian_rows.push_back(r);
			m_hessian_cols.push_back(c);
		}
		m_hessian_nnz = pattern.nnz();
	}

	if (m_jacobian_nnz > 0)
	{
		has_jacobian = true;
		ADFunD jacobian = sparse_jacobian(f_, sparsity.jacobian, x_values, p_values);
		jacobian.to_graph(jacobian_graph);
	}

	if (m_hessian_nnz > 0)
	{
		has_hessian = true;
		ADFunD hessian =
		    sparse_hessian(f_, sparsity.hessian, sparsity.reduced_hessian, x_values, p_values);
		hessian.to_graph(hessian_graph);
	}
}

void NonlinearFunction::assign_evaluators(uintptr_t fp, uintptr_t jp, uintptr_t ajp, uintptr_t hp)
{
	if (has_parameter)
	{
		f_eval.p = (f_funcptr)fp;
		if (has_jacobian)
		{
			jacobian_eval.p = (jacobian_funcptr)jp;
			grad_eval.p = (additive_grad_funcptr)ajp;
		}
		if (has_hessian)
		{
			hessian_eval.p = (hessian_funcptr)hp;
		}
	}
	else
	{
		f_eval.nop = (f_funcptr_noparam)fp;
		if (has_jacobian)
		{
			jacobian_eval.nop = (jacobian_funcptr_noparam)jp;
			grad_eval.nop = (additive_grad_funcptr_noparam)ajp;
		}
		if (has_hessian)
		{
			hessian_eval.nop = (hessian_funcptr_noparam)hp;
		}
	}
}

void LinearQuadraticModel::add_linear_constraint(const ScalarAffineFunction &f, size_t y)
{
	linear_constraints.push_back(f);
	linear_constraint_indices.push_back(y);
}

void LinearQuadraticModel::add_quadratic_constraint(const ScalarQuadraticFunction &f, size_t y)
{
	quadratic_constraints.push_back(f);
	quadratic_constraint_indices.push_back(y);
}

void LinearQuadraticModel::analyze_jacobian_structure(size_t &m_jacobian_nnz,
                                                      std::vector<size_t> &m_jacobian_rows,
                                                      std::vector<size_t> &m_jacobian_cols)
{
	// analyze linear constraints
	jacobian_constants.clear();
	for (int i = 0; i < linear_constraints.size(); i++)
	{
		auto &f = linear_constraints[i];
		auto N = f.size();
		auto row = linear_constraint_indices[i];
		for (size_t j = 0; j < N; j++)
		{
			m_jacobian_rows.push_back(row);
			m_jacobian_cols.push_back(f.variables[j]);
			jacobian_constants.emplace_back(f.coefficients[j], m_jacobian_nnz + j);
		}
		m_jacobian_nnz += N;
	}

	// analyze quadratic constraints
	jacobian_linear_terms.clear();
	for (int i = 0; i < quadratic_constraints.size(); i++)
	{
		auto &f = quadratic_constraints[i];
		auto row = quadratic_constraint_indices[i];
		auto N = f.size();
		for (size_t j = 0; j < N; j++)
		{
			auto x1 = f.variable_1s[j];
			auto x2 = f.variable_2s[j];
			if (x1 == x2)
			{
				m_jacobian_rows.push_back(row);
				m_jacobian_cols.push_back(x1);
				jacobian_linear_terms.emplace_back(2.0 * f.coefficients[j], x1, m_jacobian_nnz);
				m_jacobian_nnz += 1;
			}
			else
			{
				m_jacobian_rows.push_back(row);
				m_jacobian_cols.push_back(x1);
				jacobian_linear_terms.emplace_back(f.coefficients[j], x2, m_jacobian_nnz);
				m_jacobian_nnz += 1;
				m_jacobian_rows.push_back(row);
				m_jacobian_cols.push_back(x2);
				jacobian_linear_terms.emplace_back(f.coefficients[j], x1, m_jacobian_nnz);
				m_jacobian_nnz += 1;
			}
		}
		if (f.affine_part)
		{
			ScalarAffineFunction &af = f.affine_part.value();
			auto N = af.size();
			for (size_t j = 0; j < N; j++)
			{
				m_jacobian_rows.push_back(row);
				m_jacobian_cols.push_back(af.variables[j]);
				jacobian_constants.emplace_back(af.coefficients[j], m_jacobian_nnz + j);
			}
			m_jacobian_nnz += N;
		}
	}
}

void LinearQuadraticModel::analyze_dense_gradient_structure()
{
	gradient_constants.clear();
	gradient_linear_terms.clear();
	// quadratic part
	{
		auto &terms = lq_objective.quadratic_terms;
		for (const auto &[varpair, c] : terms)
		{
			auto x1 = varpair.var_1;
			auto x2 = varpair.var_2;

			if (x1 == x2)
			{
				size_t grad_index = x1;
				gradient_linear_terms.emplace_back(2.0 * c, x1, grad_index);
			}
			else
			{
				size_t grad_index = x1;
				gradient_linear_terms.emplace_back(c, x2, grad_index);
				grad_index = x2;
				gradient_linear_terms.emplace_back(c, x1, grad_index);
			}
		}
	}
	// linear part
	{
		auto &terms = lq_objective.affine_terms;
		for (const auto &[x, c] : terms)
		{
			size_t grad_index = x;
			gradient_constants.emplace_back(c, grad_index);
		}
	}
}

void LinearQuadraticModel::analyze_sparse_gradient_structure(size_t &gradient_nnz,
                                                             std::vector<size_t> &gradient_cols)
{
	gradient_constants.clear();
	gradient_linear_terms.clear();
	// quadratic part
	{
		auto &terms = lq_objective.quadratic_terms;
		for (const auto &[varpair, c] : terms)
		{
			auto x1 = varpair.var_1;
			auto x2 = varpair.var_2;

			if (x1 == x2)
			{
				size_t grad_index = add_gradient_column(x1, gradient_nnz, gradient_cols);
				gradient_linear_terms.emplace_back(2.0 * c, x1, grad_index);
			}
			else
			{
				size_t grad_index = add_gradient_column(x1, gradient_nnz, gradient_cols);
				;
				gradient_linear_terms.emplace_back(c, x2, grad_index);
				grad_index = add_gradient_column(x2, gradient_nnz, gradient_cols);
				gradient_linear_terms.emplace_back(c, x1, grad_index);
			}
		}
	}
	// linear part
	{
		auto &terms = lq_objective.affine_terms;
		for (const auto &[x, c] : terms)
		{
			size_t grad_index = add_gradient_column(x, gradient_nnz, gradient_cols);
			gradient_constants.emplace_back(c, grad_index);
		}
	}
}

void LinearQuadraticModel::analyze_hessian_structure(size_t &m_hessian_nnz,
                                                     std::vector<size_t> &m_hessian_rows,
                                                     std::vector<size_t> &m_hessian_cols,
                                                     HessianSparsityType hessian_sparsity_type)
{
	// quadratic constraints
	constraint_hessian_linear_terms.clear();
	for (int i = 0; i < quadratic_constraints.size(); i++)
	{
		auto &f = quadratic_constraints[i];
		auto row = quadratic_constraint_indices[i];
		auto N = f.size();

		// hessian part
		for (size_t j = 0; j < N; j++)
		{
			auto x1 = f.variable_1s[j];
			auto x2 = f.variable_2s[j];
			auto hessian_index = add_hessian_index(x1, x2, m_hessian_nnz, m_hessian_rows,
			                                       m_hessian_cols, hessian_sparsity_type);
			double coef = f.coefficients[j];
			if (x1 == x2)
				coef *= 2.0;
			constraint_hessian_linear_terms.emplace_back(coef, row, hessian_index);
		}
	}

	// quadratic objective
	objective_hessian_linear_terms.clear();
	{
		auto &terms = lq_objective.quadratic_terms;
		for (const auto &[varpair, c] : terms)
		{
			auto x1 = varpair.var_1;
			auto x2 = varpair.var_2;

			size_t hessian_index = add_hessian_index(x1, x2, m_hessian_nnz, m_hessian_rows,
			                                         m_hessian_cols, hessian_sparsity_type);
			double coef = c;
			if (x1 == x2)
				coef *= 2.0;
			objective_hessian_linear_terms.emplace_back(coef, hessian_index);
		}
	}
}

void LinearQuadraticModel::eval_objective(const double *restrict x, double *restrict y)
{
	double obj = 0.0;
	// linear and quadratic objective terms
	{
		auto &terms = lq_objective.quadratic_terms;
		for (const auto &[varpair, c] : terms)
		{
			auto x1 = varpair.var_1;
			auto x2 = varpair.var_2;

			obj += c * x[x1] * x[x2];
		}
	}
	// linear part
	{
		auto &terms = lq_objective.affine_terms;
		for (const auto &[xi, c] : terms)
		{
			obj += c * x[xi];
		}
	}
	// constant part
	if (lq_objective.constant_term)
	{
		obj += lq_objective.constant_term.value();
	}

	y[0] += obj;
}

void LinearQuadraticModel::eval_objective_gradient(const double *restrict x, double *restrict grad)
{
	// linear and quadratic modification
	for (const auto &linear : gradient_linear_terms)
	{
		grad[linear.yi] += linear.c * x[linear.xi];
	}
	for (const auto &constant : gradient_constants)
	{
		grad[constant.yi] += constant.c;
	}
}

void LinearQuadraticModel::eval_constraint(const double *restrict x, double *restrict con)
{
	// linear and quadratic part
	for (size_t i = 0; i < linear_constraints.size(); i++)
	{
		auto &f = linear_constraints[i];
		auto N = f.size();
		auto row = linear_constraint_indices[i];
		double sum = 0.0;
		for (size_t j = 0; j < N; j++)
		{
			sum += f.coefficients[j] * x[f.variables[j]];
		}
		if (f.constant)
		{
			sum += f.constant.value();
		}
		con[row] += sum;
	}
	for (size_t i = 0; i < quadratic_constraints.size(); i++)
	{
		auto &f = quadratic_constraints[i];
		auto N = f.size();
		auto row = quadratic_constraint_indices[i];
		double sum = 0.0;
		for (size_t j = 0; j < N; j++)
		{
			sum += f.coefficients[j] * x[f.variable_1s[j]] * x[f.variable_2s[j]];
		}
		if (f.affine_part)
		{
			ScalarAffineFunction &af = f.affine_part.value();
			auto N = af.size();
			for (size_t j = 0; j < N; j++)
			{
				sum += af.coefficients[j] * x[af.variables[j]];
			}
			if (af.constant)
			{
				sum += af.constant.value();
			}
		}
		con[row] += sum;
	}
}

void LinearQuadraticModel::eval_constraint_jacobian(const double *restrict x,
                                                    double *restrict jacobian)
{
	// linear and quadratic modification
	auto policy = std::execution::par_unseq;
	std::for_each(policy, jacobian_constants.begin(), jacobian_constants.end(),
	              [&](const auto &constant) { jacobian[constant.yi] = constant.c; });
	std::for_each(policy, jacobian_linear_terms.begin(), jacobian_linear_terms.end(),
	              [&](const auto &linear) { jacobian[linear.yi] = linear.c * x[linear.xi]; });
}

void LinearQuadraticModel::eval_lagrangian_hessian(const double *restrict x,
                                                   const double *restrict sigma,
                                                   const double *restrict lambda,
                                                   double *restrict hessian)
{
	// linear and quadratic modification
	auto policy = std::execution::par_unseq;
	std::for_each(policy, constraint_hessian_linear_terms.begin(),
	              constraint_hessian_linear_terms.end(),
	              [&](const auto &linear) { hessian[linear.yi] = lambda[linear.xi] * linear.c; });
	std::for_each(policy, objective_hessian_linear_terms.begin(),
	              objective_hessian_linear_terms.end(),
	              [&](const auto &constant) { hessian[constant.yi] = (*sigma) * constant.c; });
}

ParameterIndex NonlinearFunctionModel::add_parameter(double value)
{
	ParameterIndex idx = p.size();
	p.push_back(value);
	return idx;
}

void NonlinearFunctionModel::set_parameter(const ParameterIndex &parameter, double value)
{
	p[parameter.index] = value;
}

FunctionIndex NonlinearFunctionModel::register_function(ADFunD &f, const std::string &name,
                                                        const std::vector<double> &x_values,
                                                        const std::vector<double> &p_values)
{
	FunctionIndex idx;
	idx.index = nl_functions.size();
	NonlinearFunction kernel;
	kernel.init(f, name, x_values, p_values);
	nl_functions.push_back(kernel);
	constraint_function_instances.emplace_back();
	objective_function_instances.emplace_back();

	return idx;
}

NLConstraintIndex NonlinearFunctionModel::add_nl_constraint(const FunctionIndex &k,
                                                            const std::vector<VariableIndex> &xs,
                                                            const std::vector<ParameterIndex> &ps,
                                                            size_t y)
{
	auto &kernel = nl_functions[k.index];
	assert(xs.size() == kernel.nx);
	assert(ps.size() == kernel.np);

	auto ny = kernel.ny;

	std::vector<size_t> x_indices(xs.size());
	std::vector<size_t> p_indices(ps.size());
	for (size_t i = 0; i < xs.size(); i++)
	{
		x_indices[i] = xs[i].index;
	}
	for (size_t i = 0; i < ps.size(); i++)
	{
		p_indices[i] = ps[i].index;
	}

	FunctionInstance inst;
	inst.xs = x_indices;
	inst.ps = p_indices;
	inst.y_start = y;
	inst.eval_y_start = y;

	auto &inst_vec = constraint_function_instances[k.index];
	inst_vec.push_back(inst);

	NLConstraintIndex con;
	con.index = y;
	con.dim = ny;

	return con;
}

void NonlinearFunctionModel::add_nl_objective(const FunctionIndex &k,
                                              const std::vector<VariableIndex> &xs,
                                              const std::vector<ParameterIndex> &ps)
{
	auto &kernel = nl_functions[k.index];
	assert(xs.size() == kernel.nx);
	assert(ps.size() == kernel.np);

	assert(kernel.ny == 1);

	std::vector<size_t> x_indices(xs.size());
	std::vector<size_t> p_indices(ps.size());
	for (size_t i = 0; i < xs.size(); i++)
	{
		x_indices[i] = xs[i].index;
	}
	for (size_t i = 0; i < ps.size(); i++)
	{
		p_indices[i] = ps[i].index;
	}

	FunctionInstance inst;
	inst.xs = x_indices;
	inst.ps = p_indices;

	auto &inst_vec = objective_function_instances[k.index];
	inst_vec.push_back(inst);
}

void NonlinearFunctionModel::clear_nl_objective()
{
	for (auto &inst_vec : objective_function_instances)
	{
		inst_vec.clear();
	}
}

size_t add_gradient_column(size_t column, size_t &gradient_nnz, std::vector<size_t> &gradient_cols)
{
	size_t gradient_index = gradient_nnz;
	gradient_cols.push_back(column);
	gradient_nnz += 1;
	return gradient_index;
}

size_t add_hessian_index(size_t x1, size_t x2, size_t &m_hessian_nnz,
                         std::vector<size_t> &m_hessian_rows, std::vector<size_t> &m_hessian_cols,
                         HessianSparsityType hessian_sparsity_type)
{
	if (hessian_sparsity_type == HessianSparsityType::Upper && x1 > x2)
		std::swap(x1, x2);
	if (hessian_sparsity_type == HessianSparsityType::Lower && x1 < x2)
		std::swap(x1, x2);

	size_t hessian_index = m_hessian_nnz;
	m_hessian_rows.push_back(x1);
	m_hessian_cols.push_back(x2);
	m_hessian_nnz += 1;
	return hessian_index;
}

void preprocess_duplicate_indices_2d(const std::vector<size_t> &rows,
                                     const std::vector<size_t> &cols,
                                     std::vector<size_t> &unique_rows,
                                     std::vector<size_t> &unique_cols,
                                     std::vector<size_t> &permute_indices,
                                     std::vector<size_t> &permute_offsets)
{
	Hashmap<VariablePair, size_t> index_map;
	size_t current_index = 0;

	std::vector<std::vector<size_t>> temp_accum_indices;
	for (size_t i = 0; i < rows.size(); ++i)
	{
		auto [iter, inserted] = index_map.emplace(VariablePair(rows[i], cols[i]), current_index);

		if (inserted)
		{
			unique_rows.push_back(rows[i]);
			unique_cols.push_back(cols[i]);
			temp_accum_indices.push_back(std::vector<size_t>{i});
			++current_index;
		}
		else
		{
			temp_accum_indices[iter->second].push_back(i);
		}
	}

	size_t offset = 0;
	for (const auto &indices : temp_accum_indices)
	{
		permute_offsets.push_back(offset);
		permute_indices.insert(permute_indices.end(), indices.begin(), indices.end());
		offset += indices.size();
	}
	permute_offsets.push_back(offset);
}

void preprocess_duplicate_indices_1d(const std::vector<size_t> &rows,
                                     std::vector<size_t> &unique_rows,
                                     std::vector<size_t> &permute_indices,
                                     std::vector<size_t> &permute_offsets)
{
	Hashmap<size_t, size_t> index_map;
	size_t current_index = 0;

	std::vector<std::vector<size_t>> temp_accum_indices;
	for (size_t i = 0; i < rows.size(); ++i)
	{
		auto [iter, inserted] = index_map.emplace(rows[i], current_index);

		if (inserted)
		{
			unique_rows.push_back(rows[i]);
			temp_accum_indices.push_back(std::vector<size_t>{i});
			++current_index;
		}
		else
		{
			temp_accum_indices[iter->second].push_back(i);
		}
	}

	size_t offset = 0;
	for (const auto &indices : temp_accum_indices)
	{
		permute_offsets.push_back(offset);
		permute_indices.insert(permute_indices.end(), indices.begin(), indices.end());
		offset += indices.size();
	}
	permute_offsets.push_back(offset);
}

void accumulate_duplicate_values(const std::vector<double> &V,
                                 const std::vector<size_t> &permute_indices,
                                 const std::vector<size_t> &permute_offsets, double *result)
{
	for (size_t i = 0; i < permute_offsets.size() - 1; ++i)
	{
		size_t start = permute_offsets[i];
		size_t end = permute_offsets[i + 1];
		for (size_t j = start; j < end; ++j)
		{
			result[i] += V[permute_indices[j]];
		}
	}
}

void NonlinearFunctionModel::analyze_active_functions()
{
	auto Nk = nl_functions.size();
	active_constraint_function_indices.clear();
	active_objective_function_indices.clear();
	for (size_t k = 0; k < Nk; k++)
	{
		auto &inst_vec = constraint_function_instances[k];
		if (inst_vec.size() > 0)
		{
			active_constraint_function_indices.push_back(k);
		}
		auto &inst_vec2 = objective_function_instances[k];
		if (inst_vec2.size() > 0)
		{
			active_objective_function_indices.push_back(k);
		}
	}
}

void NonlinearFunctionModel::analyze_compact_constraint_index(size_t &n_nlcon,
                                                              std::vector<size_t> &ys)
{
	size_t N = 0;
	for (size_t k : active_constraint_function_indices)
	{
		auto &kernel = nl_functions[k];
		auto &inst_vec = constraint_function_instances[k];

		auto ny = kernel.ny;

		for (auto &inst : inst_vec)
		{
			inst.eval_y_start = N;
			for (size_t j = 0; j < ny; j++)
			{
				ys.push_back(N + j);
			}
			N += ny;
		}
	}
	n_nlcon += N;
}

void NonlinearFunctionModel::analyze_jacobian_structure(size_t &m_jacobian_nnz,
                                                        std::vector<size_t> &m_jacobian_rows,
                                                        std::vector<size_t> &m_jacobian_cols)
{
	for (size_t k : active_constraint_function_indices)
	{
		auto &kernel = nl_functions[k];
		auto &inst_vec = constraint_function_instances[k];

		for (auto &inst : inst_vec)
		{
			auto &x_indices = inst.xs;

			for (size_t j = 0; j < kernel.m_jacobian_nnz; j++)
			{
				auto row = inst.y_start + kernel.m_jacobian_rows[j];
				m_jacobian_rows.push_back(row);
				auto column = x_indices[kernel.m_jacobian_cols[j]];
				m_jacobian_cols.push_back(column);
			}
			inst.jacobian_start = m_jacobian_nnz;
			m_jacobian_nnz += kernel.m_jacobian_nnz;
		}
	}
}

void NonlinearFunctionModel::analyze_dense_gradient_structure()
{
	for (size_t k : active_objective_function_indices)
	{
		auto &kernel = nl_functions[k];
		auto &inst_vec = objective_function_instances[k];

		for (auto &inst : inst_vec)
		{
			auto &x_indices = inst.xs;

			auto &grad_indices = inst.grad_indices;
			grad_indices.resize(kernel.m_jacobian_nnz);
			for (size_t j = 0; j < kernel.m_jacobian_nnz; j++)
			{
				auto column = x_indices[kernel.m_jacobian_cols[j]];
				size_t grad_index = column;
				grad_indices[j] = grad_index;
			}
		}
	}
}

void NonlinearFunctionModel::analyze_sparse_gradient_structure(size_t &gradient_nnz,
                                                               std::vector<size_t> &gradient_cols)
{
	for (size_t k : active_objective_function_indices)
	{
		auto &kernel = nl_functions[k];
		auto &inst_vec = objective_function_instances[k];

		for (auto &inst : inst_vec)
		{
			auto &x_indices = inst.xs;

			auto &grad_indices = inst.grad_indices;
			grad_indices.resize(kernel.m_jacobian_nnz);
			for (size_t j = 0; j < kernel.m_jacobian_nnz; j++)
			{
				auto column = x_indices[kernel.m_jacobian_cols[j]];
				size_t grad_index = add_gradient_column(column, gradient_nnz, gradient_cols);
				grad_indices[j] = grad_index;
			}
		}
	}
}

void NonlinearFunctionModel::analyze_hessian_structure(size_t &m_hessian_nnz,
                                                       std::vector<size_t> &m_hessian_rows,
                                                       std::vector<size_t> &m_hessian_cols,
                                                       HessianSparsityType hessian_sparsity_type)
{
	for (size_t k : active_constraint_function_indices)
	{
		auto &kernel = nl_functions[k];
		auto &inst_vec = constraint_function_instances[k];

		for (auto &inst : inst_vec)
		{
			auto &x_indices = inst.xs;

			auto &hessian_indices = inst.hessian_indices;
			hessian_indices.resize(kernel.m_hessian_nnz);
			for (size_t j = 0; j < kernel.m_hessian_nnz; j++)
			{
				auto x1 = x_indices[kernel.m_hessian_rows[j]];
				auto x2 = x_indices[kernel.m_hessian_cols[j]];

				auto hessian_index = add_hessian_index(x1, x2, m_hessian_nnz, m_hessian_rows,
				                                       m_hessian_cols, hessian_sparsity_type);
				hessian_indices[j] = hessian_index;
			}
		}
	}

	for (size_t k : active_objective_function_indices)
	{
		auto &kernel = nl_functions[k];
		auto &inst_vec = objective_function_instances[k];

		for (auto &inst : inst_vec)
		{
			auto &x_indices = inst.xs;

			auto &hessian_indices = inst.hessian_indices;
			hessian_indices.resize(kernel.m_hessian_nnz);
			for (size_t j = 0; j < kernel.m_hessian_nnz; j++)
			{
				auto x1 = x_indices[kernel.m_hessian_rows[j]];
				auto x2 = x_indices[kernel.m_hessian_cols[j]];

				auto hessian_index = add_hessian_index(x1, x2, m_hessian_nnz, m_hessian_rows,
				                                       m_hessian_cols, hessian_sparsity_type);
				hessian_indices[j] = hessian_index;
			}
		}
	}
}

void NonlinearFunctionModel::eval_objective(const double *x, double *y)
{
	double obj = 0.0;
	double temp = 0.0;

	double *p = this->p.data();

	// nonlinear objective terms
	for (auto k : active_objective_function_indices)
	{
		auto &kernel = nl_functions[k];
		bool has_parameter = kernel.has_parameter;
		auto &inst_vec = objective_function_instances[k];

		if (has_parameter)
		{
			for (const auto &inst : inst_vec)
			{
				auto &x_indices = inst.xs;
				auto &p_indices = inst.ps;
				kernel.f_eval.p(x, p, &temp, x_indices.data(), p_indices.data());
			}
		}
		else
		{
			for (const auto &inst : inst_vec)
			{
				auto &x_indices = inst.xs;
				kernel.f_eval.nop(x, &temp, x_indices.data());
			}
		}
		obj += temp;
	}

	y[0] += obj;
}

void NonlinearFunctionModel::eval_objective_gradient(const double *x, double *grad)
{
	double *p = this->p.data();

	// nonlinear objective terms
	for (auto k : active_objective_function_indices)
	{
		auto &kernel = nl_functions[k];
		if (!kernel.has_jacobian)
			continue;

		bool has_parameter = kernel.has_parameter;
		auto &inst_vec = objective_function_instances[k];

		if (has_parameter)
		{
			for (const auto &inst : inst_vec)
			{
				auto &x_indices = inst.xs;
				auto &grad_indices = inst.grad_indices;

				auto &p_indices = inst.ps;
				kernel.grad_eval.p(x, p, grad, x_indices.data(), p_indices.data(),
				                   grad_indices.data());
			}
		}
		else
		{
			for (const auto &inst : inst_vec)
			{
				auto &x_indices = inst.xs;
				auto &grad_indices = inst.grad_indices;

				kernel.grad_eval.nop(x, grad, x_indices.data(), grad_indices.data());
			}
		}
	}
}

void NonlinearFunctionModel::eval_constraint(const double *x, double *con)
{
	double *p = this->p.data();
	for (auto k : active_constraint_function_indices)
	{
		auto &kernel = nl_functions[k];
		bool has_parameter = kernel.has_parameter;
		auto &inst_vec = constraint_function_instances[k];
		if (has_parameter)
		{
			for (const auto &inst : inst_vec)
			{
				auto &x_indices = inst.xs;
				double *y = con + inst.eval_y_start;

				auto &p_indices = inst.ps;
				kernel.f_eval.p(x, p, y, x_indices.data(), p_indices.data());
			}
		}
		else
		{
			for (const auto &inst : inst_vec)
			{
				auto &x_indices = inst.xs;
				double *y = con + inst.eval_y_start;

				kernel.f_eval.nop(x, y, x_indices.data());
			}
		}
	}
}

void NonlinearFunctionModel::eval_constraint_jacobian(const double *x, double *jacobian)
{
	double *p = this->p.data();
	auto policy = std::execution::par_unseq;
	for (auto k : active_constraint_function_indices)
	{
		auto &kernel = nl_functions[k];
		if (!kernel.has_jacobian)
			continue;

		bool has_parameter = kernel.has_parameter;
		auto &inst_vec = constraint_function_instances[k];

		if (has_parameter)
		{
			std::for_each(policy, inst_vec.begin(), inst_vec.end(), [&](const auto &inst) {
				auto &x_indices = inst.xs;
				double *j = jacobian + inst.jacobian_start;

				auto &p_indices = inst.ps;
				kernel.jacobian_eval.p(x, p, j, x_indices.data(), p_indices.data());
			});
		}
		else
		{
			std::for_each(policy, inst_vec.begin(), inst_vec.end(), [&](const auto &inst) {
				auto &x_indices = inst.xs;
				double *j = jacobian + inst.jacobian_start;

				kernel.jacobian_eval.nop(x, j, x_indices.data());
			});
		}
	}
}

void NonlinearFunctionModel::eval_lagrangian_hessian(const double *x, const double *sigma,
                                                     const double *lambda, double *hessian)
{
	double *p = this->p.data();

	auto policy = std::execution::par_unseq;
	for (auto k : active_constraint_function_indices)
	{
		auto &kernel = nl_functions[k];
		if (!kernel.has_hessian)
			continue;

		bool has_parameter = kernel.has_parameter;
		auto &inst_vec = constraint_function_instances[k];
		if (has_parameter)
		{
			std::for_each(policy, inst_vec.begin(), inst_vec.end(), [&](const auto &inst) {
				auto &x_indices = inst.xs;
				auto &hessian_indices = inst.hessian_indices;

				const double *w = lambda + inst.y_start;

				auto &p_indices = inst.ps;
				kernel.hessian_eval.p(x, p, w, hessian, x_indices.data(), p_indices.data(),
				                      hessian_indices.data());
			});
		}
		else
		{
			std::for_each(policy, inst_vec.begin(), inst_vec.end(), [&](const auto &inst) {
				auto &x_indices = inst.xs;
				auto &hessian_indices = inst.hessian_indices;

				const double *w = lambda + inst.y_start;

				kernel.hessian_eval.nop(x, w, hessian, x_indices.data(), hessian_indices.data());
			});
		}
	}
	const double *w = sigma;
	for (auto k : active_objective_function_indices)
	{
		auto &kernel = nl_functions[k];
		if (!kernel.has_hessian)
			continue;

		bool has_parameter = kernel.has_parameter;
		auto &inst_vec = objective_function_instances[k];
		if (has_parameter)
		{
			std::for_each(policy, inst_vec.begin(), inst_vec.end(), [&](const auto &inst) {
				auto &x_indices = inst.xs;
				auto &hessian_indices = inst.hessian_indices;

				auto &p_indices = inst.ps;
				kernel.hessian_eval.p(x, p, w, hessian, x_indices.data(), p_indices.data(),
				                      hessian_indices.data());
			});
			for (const auto &inst : inst_vec)
			{
				auto &x_indices = inst.xs;
				auto &hessian_indices = inst.hessian_indices;

				auto &p_indices = inst.ps;
				kernel.hessian_eval.p(x, p, w, hessian, x_indices.data(), p_indices.data(),
				                      hessian_indices.data());
			}
		}
		else
		{
			std::for_each(policy, inst_vec.begin(), inst_vec.end(), [&](const auto &inst) {
				auto &x_indices = inst.xs;
				auto &hessian_indices = inst.hessian_indices;

				kernel.hessian_eval.nop(x, w, hessian, x_indices.data(), hessian_indices.data());
			});
		}
	}
}
