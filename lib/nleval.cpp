#include "pyoptinterface/nleval.hpp"
#include <cassert>

AutodiffEvaluator::AutodiffEvaluator(const AutodiffSymbolicStructure &structure, uintptr_t fp,
                                     uintptr_t jp, uintptr_t ajp, uintptr_t hp)
{
	if (structure.has_parameter)
	{
		f_eval.p = (f_funcptr)fp;
		if (structure.has_jacobian)
		{
			jacobian_eval.p = (jacobian_funcptr)jp;
			grad_eval.p = (additive_grad_funcptr)ajp;
		}
		if (structure.has_hessian)
		{
			hessian_eval.p = (hessian_funcptr)hp;
		}
	}
	else
	{
		f_eval.nop = (f_funcptr_noparam)fp;
		if (structure.has_jacobian)
		{
			jacobian_eval.nop = (jacobian_funcptr_noparam)jp;
			grad_eval.nop = (additive_grad_funcptr_noparam)ajp;
		}
		if (structure.has_hessian)
		{
			hessian_eval.nop = (hessian_funcptr_noparam)hp;
		}
	}
}

void LinearQuadraticEvaluator::add_linear_constraint(const ScalarAffineFunction &f, size_t y)
{
	linear_constraints.push_back(f);
	linear_constraint_indices.push_back(y);
}

void LinearQuadraticEvaluator::add_quadratic_constraint(const ScalarQuadraticFunction &f, size_t y)
{
	quadratic_constraints.push_back(f);
	quadratic_constraint_indices.push_back(y);
}

void LinearQuadraticEvaluator::analyze_jacobian_structure(size_t &m_jacobian_nnz,
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

void LinearQuadraticEvaluator::analyze_dense_gradient_structure()
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

void LinearQuadraticEvaluator::analyze_sparse_gradient_structure(
    size_t &gradient_nnz, std::vector<size_t> &gradient_cols,
    Hashmap<size_t, size_t> &gradient_index_map)
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
				size_t grad_index =
				    add_gradient_column(x1, gradient_nnz, gradient_cols, gradient_index_map);
				gradient_linear_terms.emplace_back(2.0 * c, x1, grad_index);
			}
			else
			{
				size_t grad_index =
				    add_gradient_column(x1, gradient_nnz, gradient_cols, gradient_index_map);
				;
				gradient_linear_terms.emplace_back(c, x2, grad_index);
				grad_index =
				    add_gradient_column(x2, gradient_nnz, gradient_cols, gradient_index_map);
				gradient_linear_terms.emplace_back(c, x1, grad_index);
			}
		}
	}
	// linear part
	{
		auto &terms = lq_objective.affine_terms;
		for (const auto &[x, c] : terms)
		{
			size_t grad_index =
			    add_gradient_column(x, gradient_nnz, gradient_cols, gradient_index_map);
			gradient_constants.emplace_back(c, grad_index);
		}
	}
}

void LinearQuadraticEvaluator::analyze_hessian_structure(
    size_t &m_hessian_nnz, std::vector<size_t> &m_hessian_rows, std::vector<size_t> &m_hessian_cols,
    Hashmap<VariablePair, size_t> &m_hessian_index_map, HessianSparsityType hessian_sparsity_type)
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
			auto hessian_index =
			    add_hessian_index(x1, x2, m_hessian_nnz, m_hessian_rows, m_hessian_cols,
			                      m_hessian_index_map, hessian_sparsity_type);
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

			size_t hessian_index =
			    add_hessian_index(x1, x2, m_hessian_nnz, m_hessian_rows, m_hessian_cols,
			                      m_hessian_index_map, hessian_sparsity_type);
			double coef = c;
			if (x1 == x2)
				coef *= 2.0;
			objective_hessian_linear_terms.emplace_back(coef, hessian_index);
		}
	}
}

void LinearQuadraticEvaluator::eval_objective(const double *restrict x, double *restrict y)
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

void LinearQuadraticEvaluator::eval_objective_gradient(const double *restrict x,
                                                       double *restrict grad)
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

void LinearQuadraticEvaluator::eval_constraint(const double *restrict x, double *restrict con)
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

void LinearQuadraticEvaluator::eval_constraint_jacobian(const double *restrict x,
                                                        double *restrict jacobian)
{
	// linear and quadratic modification
	for (const auto &constant : jacobian_constants)
	{
		jacobian[constant.yi] += constant.c;
	}
	for (const auto &linear : jacobian_linear_terms)
	{
		jacobian[linear.yi] += linear.c * x[linear.xi];
	}
}

void LinearQuadraticEvaluator::eval_lagrangian_hessian(const double *restrict x,
                                                       const double *restrict sigma,
                                                       const double *restrict lambda,
                                                       double *restrict hessian)
{
	// linear and quadratic modification
	for (const auto &linear : constraint_hessian_linear_terms)
	{
		hessian[linear.yi] += lambda[linear.xi] * linear.c;
	}
	for (const auto &constant : objective_hessian_linear_terms)
	{
		hessian[constant.yi] += (*sigma) * constant.c;
	}
}

ParameterIndex NonlinearFunctionEvaluator::add_parameter(double value)
{
	ParameterIndex idx = p.size();
	p.push_back(value);
	return idx;
}

void NonlinearFunctionEvaluator::set_parameter(const ParameterIndex &parameter, double value)
{
	p[parameter.index] = value;
}

FunctionIndex NonlinearFunctionEvaluator::register_function(
    const AutodiffSymbolicStructure &structure)
{
	FunctionIndex idx;
	idx.index = nl_function_structures.size();

	nl_function_structures.push_back(structure);
	nl_function_evaluators.emplace_back(std::nullopt);
	constraint_function_instances.emplace_back();
	objective_function_instances.emplace_back();

	return idx;
}

void NonlinearFunctionEvaluator::set_function_evaluator(const FunctionIndex &k,
                                                        const AutodiffEvaluator &evaluator)
{
	nl_function_evaluators[k.index] = evaluator;
}

bool NonlinearFunctionEvaluator::has_function_evaluator(const FunctionIndex &k)
{
	return nl_function_evaluators[k.index].has_value();
}

NLConstraintIndex NonlinearFunctionEvaluator::add_nl_constraint(
    const FunctionIndex &k, const std::vector<VariableIndex> &xs,
    const std::vector<ParameterIndex> &ps, size_t y)
{
	auto &kernel = nl_function_structures[k.index];
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

void NonlinearFunctionEvaluator::add_nl_objective(const FunctionIndex &k,
                                                  const std::vector<VariableIndex> &xs,
                                                  const std::vector<ParameterIndex> &ps)
{
	auto &kernel = nl_function_structures[k.index];
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

void NonlinearFunctionEvaluator::clear_nl_objective()
{
	for (auto &inst_vec : objective_function_instances)
	{
		inst_vec.clear();
	}
}

size_t add_gradient_column(size_t column, size_t &gradient_nnz, std::vector<size_t> &gradient_cols,
                           Hashmap<size_t, size_t> &grad_index_map)
{
	size_t gradient_index;
	auto iter = grad_index_map.find(column);
	if (iter != grad_index_map.end())
	{
		gradient_index = iter->second;
	}
	else
	{
		gradient_index = gradient_nnz;
		grad_index_map[column] = gradient_nnz;
		gradient_cols.push_back(column);
		gradient_nnz += 1;
	}
	return gradient_index;
}

size_t add_hessian_index(size_t x1, size_t x2, size_t &m_hessian_nnz,
                         std::vector<size_t> &m_hessian_rows, std::vector<size_t> &m_hessian_cols,
                         Hashmap<VariablePair, size_t> &m_hessian_index_map,
                         HessianSparsityType hessian_sparsity_type)
{
	if (hessian_sparsity_type == HessianSparsityType::Upper && x1 > x2)
		std::swap(x1, x2);
	if (hessian_sparsity_type == HessianSparsityType::Lower && x1 < x2)
		std::swap(x1, x2);

	size_t hessian_index;
	VariablePair varpair(x1, x2);
	auto [iter, inserted] = m_hessian_index_map.emplace(varpair, m_hessian_nnz);
	if (inserted)
	{
		hessian_index = m_hessian_nnz;
		m_hessian_rows.push_back(x1);
		m_hessian_cols.push_back(x2);
		m_hessian_nnz += 1;
	}
	else
	{
		hessian_index = iter->second;
	}
	return hessian_index;
}

void NonlinearFunctionEvaluator::analyze_active_functions()
{
	auto Nk = nl_function_structures.size();
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

void NonlinearFunctionEvaluator::analyze_compact_constraint_index(size_t &n_nlcon,
                                                                  std::vector<size_t> &ys)
{
	size_t N = 0;
	for (size_t k : active_constraint_function_indices)
	{
		auto &kernel = nl_function_structures[k];
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

void NonlinearFunctionEvaluator::analyze_jacobian_structure(size_t &m_jacobian_nnz,
                                                            std::vector<size_t> &m_jacobian_rows,
                                                            std::vector<size_t> &m_jacobian_cols)
{
	for (size_t k : active_constraint_function_indices)
	{
		auto &kernel = nl_function_structures[k];
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

void NonlinearFunctionEvaluator::analyze_dense_gradient_structure()
{
	for (size_t k : active_objective_function_indices)
	{
		auto &kernel = nl_function_structures[k];
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

void NonlinearFunctionEvaluator::analyze_sparse_gradient_structure(
    size_t &gradient_nnz, std::vector<size_t> &gradient_cols,
    Hashmap<size_t, size_t> &gradient_index_map)
{
	for (size_t k : active_objective_function_indices)
	{
		auto &kernel = nl_function_structures[k];
		auto &inst_vec = objective_function_instances[k];

		for (auto &inst : inst_vec)
		{
			auto &x_indices = inst.xs;

			auto &grad_indices = inst.grad_indices;
			grad_indices.resize(kernel.m_jacobian_nnz);
			for (size_t j = 0; j < kernel.m_jacobian_nnz; j++)
			{
				auto column = x_indices[kernel.m_jacobian_cols[j]];
				size_t grad_index =
				    add_gradient_column(column, gradient_nnz, gradient_cols, gradient_index_map);
				grad_indices[j] = grad_index;
			}
		}
	}
}

void NonlinearFunctionEvaluator::analyze_hessian_structure(
    size_t &m_hessian_nnz, std::vector<size_t> &m_hessian_rows, std::vector<size_t> &m_hessian_cols,
    Hashmap<VariablePair, size_t> &m_hessian_index_map, HessianSparsityType hessian_sparsity_type)
{
	for (size_t k : active_constraint_function_indices)
	{
		auto &kernel = nl_function_structures[k];
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

				auto hessian_index =
				    add_hessian_index(x1, x2, m_hessian_nnz, m_hessian_rows, m_hessian_cols,
				                      m_hessian_index_map, hessian_sparsity_type);
				hessian_indices[j] = hessian_index;
			}
		}
	}

	for (size_t k : active_objective_function_indices)
	{
		auto &kernel = nl_function_structures[k];
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

				auto hessian_index =
				    add_hessian_index(x1, x2, m_hessian_nnz, m_hessian_rows, m_hessian_cols,
				                      m_hessian_index_map, hessian_sparsity_type);
				hessian_indices[j] = hessian_index;
			}
		}
	}
}

void NonlinearFunctionEvaluator::eval_objective(const double *x, double *y)
{
	double obj = 0.0;

	double *p = this->p.data();

	// nonlinear objective terms
	for (auto k : active_objective_function_indices)
	{
		auto &kernel = nl_function_structures[k];
		auto &evaluator = nl_function_evaluators[k].value();
		bool has_parameter = kernel.has_parameter;
		auto &inst_vec = objective_function_instances[k];

		if (has_parameter)
		{
			for (const auto &inst : inst_vec)
			{
				auto &x_indices = inst.xs;
				auto &p_indices = inst.ps;
				evaluator.f_eval.p(x, p, &obj, x_indices.data(), p_indices.data());
			}
		}
		else
		{
			for (const auto &inst : inst_vec)
			{
				auto &x_indices = inst.xs;
				evaluator.f_eval.nop(x, &obj, x_indices.data());
			}
		}
	}

	y[0] += obj;
}

void NonlinearFunctionEvaluator::eval_objective_gradient(const double *x, double *grad)
{
	double *p = this->p.data();

	// nonlinear objective terms
	for (auto k : active_objective_function_indices)
	{
		auto &kernel = nl_function_structures[k];
		auto &evaluator = nl_function_evaluators[k].value();
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
				evaluator.grad_eval.p(x, p, grad, x_indices.data(), p_indices.data(),
				                      grad_indices.data());
			}
		}
		else
		{
			for (const auto &inst : inst_vec)
			{
				auto &x_indices = inst.xs;
				auto &grad_indices = inst.grad_indices;

				evaluator.grad_eval.nop(x, grad, x_indices.data(), grad_indices.data());
			}
		}
	}
}

void NonlinearFunctionEvaluator::eval_constraint(const double *x, double *con)
{
	double *p = this->p.data();
	for (auto k : active_constraint_function_indices)
	{
		auto &kernel = nl_function_structures[k];
		auto &evaluator = nl_function_evaluators[k].value();
		bool has_parameter = kernel.has_parameter;
		auto &inst_vec = constraint_function_instances[k];
		if (has_parameter)
		{
			for (const auto &inst : inst_vec)
			{
				auto &x_indices = inst.xs;
				double *y = con + inst.eval_y_start;

				auto &p_indices = inst.ps;
				evaluator.f_eval.p(x, p, y, x_indices.data(), p_indices.data());
			}
		}
		else
		{
			for (const auto &inst : inst_vec)
			{
				auto &x_indices = inst.xs;
				double *y = con + inst.eval_y_start;

				evaluator.f_eval.nop(x, y, x_indices.data());
			}
		}
	}
}

void NonlinearFunctionEvaluator::eval_constraint_jacobian(const double *x, double *jacobian)
{
	double *p = this->p.data();
	for (auto k : active_constraint_function_indices)
	{
		auto &kernel = nl_function_structures[k];
		auto &evaluator = nl_function_evaluators[k].value();
		if (!kernel.has_jacobian)
			continue;

		bool has_parameter = kernel.has_parameter;
		auto &inst_vec = constraint_function_instances[k];

		if (has_parameter)
		{
			for (const auto &inst : inst_vec)
			{
				auto &x_indices = inst.xs;
				double *j = jacobian + inst.jacobian_start;

				auto &p_indices = inst.ps;
				evaluator.jacobian_eval.p(x, p, j, x_indices.data(), p_indices.data());
			}
		}
		else
		{
			for (const auto &inst : inst_vec)
			{
				auto &x_indices = inst.xs;
				double *j = jacobian + inst.jacobian_start;

				evaluator.jacobian_eval.nop(x, j, x_indices.data());
			}
		}
	}
}

void NonlinearFunctionEvaluator::eval_lagrangian_hessian(const double *x, const double *sigma,
                                                         const double *lambda, double *hessian)
{
	double *p = this->p.data();
	for (auto k : active_constraint_function_indices)
	{
		auto &kernel = nl_function_structures[k];
		auto &evaluator = nl_function_evaluators[k].value();
		if (!kernel.has_hessian)
			continue;

		bool has_parameter = kernel.has_parameter;
		auto &inst_vec = constraint_function_instances[k];
		if (has_parameter)
		{
			for (const auto &inst : inst_vec)
			{
				auto &x_indices = inst.xs;
				auto &hessian_indices = inst.hessian_indices;

				const double *w = lambda + inst.y_start;

				auto &p_indices = inst.ps;
				evaluator.hessian_eval.p(x, p, w, hessian, x_indices.data(), p_indices.data(),
				                         hessian_indices.data());
			}
		}
		else
		{
			for (const auto &inst : inst_vec)
			{
				auto &x_indices = inst.xs;
				auto &hessian_indices = inst.hessian_indices;

				const double *w = lambda + inst.y_start;

				evaluator.hessian_eval.nop(x, w, hessian, x_indices.data(), hessian_indices.data());
			}
		}
	}
	const double *w = sigma;
	for (auto k : active_objective_function_indices)
	{
		auto &kernel = nl_function_structures[k];
		auto &evaluator = nl_function_evaluators[k].value();
		if (!kernel.has_hessian)
			continue;

		bool has_parameter = kernel.has_parameter;
		auto &inst_vec = objective_function_instances[k];
		if (has_parameter)
		{
			for (const auto &inst : inst_vec)
			{
				auto &x_indices = inst.xs;
				auto &hessian_indices = inst.hessian_indices;

				auto &p_indices = inst.ps;
				evaluator.hessian_eval.p(x, p, w, hessian, x_indices.data(), p_indices.data(),
				                         hessian_indices.data());
			}
		}
		else
		{
			for (const auto &inst : inst_vec)
			{
				auto &x_indices = inst.xs;
				auto &hessian_indices = inst.hessian_indices;

				evaluator.hessian_eval.nop(x, w, hessian, x_indices.data(), hessian_indices.data());
			}
		}
	}
}