#include "pyoptinterface/nleval.hpp"
#include <cassert>
#include <span>

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

int NonlinearEvaluator::add_graph_instance()
{
	auto current_graph_index = n_graph_instances;
	n_graph_instances += 1;
	graph_inputs.emplace_back();
	return current_graph_index;
}

void NonlinearEvaluator::finalize_graph_instance(size_t graph_index, const ExpressionGraph &graph)
{
	auto bodyhash = graph.main_structure_hash();

	graph_inputs[graph_index].variables = graph.m_variables;
	graph_inputs[graph_index].constants = graph.m_constants;

	if (graph.has_constraint_output())
	{
		auto hash = graph.constraint_structure_hash(bodyhash);
		constraint_graph_hashes.hashes.push_back(
		    GraphHash{.hash = hash, .index = (int)graph_index});
	}

	if (graph.has_objective_output())
	{
		auto hash = graph.objective_structure_hash(bodyhash);
		objective_graph_hashes.hashes.push_back(GraphHash{.hash = hash, .index = (int)graph_index});
	}
}

int NonlinearEvaluator::aggregate_constraint_groups()
{
	auto &graph_hashes = constraint_graph_hashes;
	auto &group_memberships = constraint_group_memberships;

	// ensure we have enough space for every graph instance
	group_memberships.resize(n_graph_instances, GraphGroupMembership{.group = -1, .rank = -1});

	// graph hashes that has not been aggregated
	std::span<const GraphHash> hashes_to_analyze(graph_hashes.hashes.begin() +
	                                                 graph_hashes.n_hashes_since_last_aggregation,
	                                             graph_hashes.hashes.end());

	for (const auto &graph_hash : hashes_to_analyze)
	{
		auto index = graph_hash.index;
		auto hash = graph_hash.hash;
		auto [iter, inserted] =
		    hash_to_constraint_group.try_emplace(hash, constraint_groups.size());
		auto group_index = iter->second;
		if (inserted)
		{
			constraint_groups.emplace_back();
		}
		group_memberships[index].group = group_index;
		group_memberships[index].rank = (int)constraint_groups[group_index].instance_indices.size();
		constraint_groups[group_index].instance_indices.push_back(index);
	}

	graph_hashes.n_hashes_since_last_aggregation = graph_hashes.hashes.size();

	return constraint_groups.size();
}

int NonlinearEvaluator::get_constraint_group_representative(int group_index) const
{
	auto index = constraint_groups.at(group_index).instance_indices.at(0);
	return index;
}

int NonlinearEvaluator::aggregate_objective_groups()
{
	auto &graph_hashes = objective_graph_hashes;
	auto &group_memberships = objective_group_memberships;

	// ensure we have enough space for every graph instance
	group_memberships.resize(n_graph_instances, GraphGroupMembership{.group = -1, .rank = -1});

	// graph hashes that has not been aggregated
	std::span<const GraphHash> hashes_to_analyze(graph_hashes.hashes.begin() +
	                                                 graph_hashes.n_hashes_since_last_aggregation,
	                                             graph_hashes.hashes.end());

	for (const auto &graph_hash : hashes_to_analyze)
	{
		auto index = graph_hash.index;
		auto hash = graph_hash.hash;
		auto [iter, inserted] = hash_to_objective_group.try_emplace(hash, objective_groups.size());
		auto group_index = iter->second;
		if (inserted)
		{
			objective_groups.emplace_back();
		}
		group_memberships[index].group = group_index;
		group_memberships[index].rank = (int)objective_groups[group_index].instance_indices.size();
		objective_groups[group_index].instance_indices.push_back(index);
	}

	graph_hashes.n_hashes_since_last_aggregation = graph_hashes.hashes.size();

	return objective_groups.size();
}

int NonlinearEvaluator::get_objective_group_representative(int group_index) const
{
	auto index = objective_groups.at(group_index).instance_indices.at(0);
	return index;
}

void NonlinearEvaluator::assign_constraint_group_autodiff_structure(
    int group_index, const AutodiffSymbolicStructure &structure)
{
	constraint_groups[group_index].autodiff_structure = structure;
}

void NonlinearEvaluator::assign_constraint_group_autodiff_evaluator(
    int group_index, const ConstraintAutodiffEvaluator &evaluator)
{
	constraint_groups[group_index].autodiff_evaluator = evaluator;
}

void NonlinearEvaluator::assign_objective_group_autodiff_structure(
    int group_index, const AutodiffSymbolicStructure &structure)
{
	objective_groups[group_index].autodiff_structure = structure;
}

void NonlinearEvaluator::assign_objective_group_autodiff_evaluator(
    int group_index, const ObjectiveAutodiffEvaluator &evaluator)
{
	objective_groups[group_index].autodiff_evaluator = evaluator;
}

void NonlinearEvaluator::calculate_constraint_graph_instances_offset()
{
	// now all graphs are aggregated we need to figure out which index each graph starts
	constraint_indices_offsets.resize(n_graph_instances, -1);
	int counter = 0;
	for (const auto &group : constraint_groups)
	{
		const auto &instance_indices = group.instance_indices;
		auto ny = group.autodiff_structure.ny;
		for (auto instance_index : instance_indices)
		{
			constraint_indices_offsets[instance_index] = counter;
			counter += ny;
		}
	}
}

void NonlinearEvaluator::eval_constraints(const double *restrict x, double *restrict f) const
{
	auto &groups = constraint_groups;
	for (const auto &group : groups)
	{
		auto &instance_indices = group.instance_indices;
		auto n_instances = instance_indices.size();
		auto &structure = group.autodiff_structure;
		auto &evaluator = group.autodiff_evaluator;

		auto ny = structure.ny;

		if (!structure.has_parameter)
		{
			for (int j = 0; j < n_instances; j++)
			{
				auto instance_index = instance_indices[j];
				auto &variables = graph_inputs[instance_index].variables;
				evaluator.f_eval.nop(x, f, variables.data());
				f += ny;
			}
		}
		else
		{
			for (int j = 0; j < n_instances; j++)
			{
				auto instance_index = instance_indices[j];
				auto &variables = graph_inputs[instance_index].variables;
				auto &constant = graph_inputs[instance_index].constants;
				evaluator.f_eval.p(x, constant.data(), f, variables.data());
				f += ny;
			}
		}
	}
}

double NonlinearEvaluator::eval_objective(const double *restrict x) const
{
	auto &groups = objective_groups;
	double obj_value = 0.0;
	for (const auto &group : groups)
	{
		auto &instance_indices = group.instance_indices;
		auto n_instances = instance_indices.size();
		auto &structure = group.autodiff_structure;
		auto &evaluator = group.autodiff_evaluator;

		if (!structure.has_parameter)
		{
			for (int j = 0; j < n_instances; j++)
			{
				auto instance_index = instance_indices[j];
				auto &variables = graph_inputs[instance_index].variables;
				evaluator.f_eval.nop(x, &obj_value, variables.data());
			}
		}
		else
		{
			for (int j = 0; j < n_instances; j++)
			{
				auto instance_index = instance_indices[j];
				auto &variables = graph_inputs[instance_index].variables;
				auto &constant = graph_inputs[instance_index].constants;
				evaluator.f_eval.p(x, constant.data(), &obj_value, variables.data());
			}
		}
	}
	return obj_value;
}

void NonlinearEvaluator::analyze_constraints_jacobian_structure(
    size_t row_base, size_t &global_jacobian_nnz, std::vector<int> &global_jacobian_rows,
    std::vector<int> &global_jacobian_cols)
{
	auto &groups = constraint_groups;

	for (const auto &group : groups)
	{
		auto &instance_indices = group.instance_indices;
		auto n_instances = instance_indices.size();
		auto &structure = group.autodiff_structure;

		if (!structure.has_jacobian)
		{
			continue;
		}

		auto local_jacobian_nnz = structure.m_jacobian_nnz;
		auto &local_jacobian_rows = structure.m_jacobian_rows;
		auto &local_jacobian_cols = structure.m_jacobian_cols;

		for (int j = 0; j < n_instances; j++)
		{
			auto instance_index = instance_indices[j];
			auto &variables = graph_inputs[instance_index].variables;

			for (int k = 0; k < local_jacobian_nnz; k++)
			{
				auto row = local_jacobian_rows[k] + row_base;
				auto col = variables[local_jacobian_cols[k]];
				global_jacobian_rows.push_back(row);
				global_jacobian_cols.push_back(col);
			}

			row_base += structure.ny;
		}

		global_jacobian_nnz += local_jacobian_nnz * n_instances;
	}
}

void NonlinearEvaluator::analyze_objective_gradient_structure(
    std::vector<int> &global_gradient_cols, Hashmap<int, int> &sparse_gradient_map)
{
	auto &groups = objective_groups;

	for (auto &group : groups)
	{
		auto &instance_indices = group.instance_indices;
		auto n_instances = instance_indices.size();
		auto &structure = group.autodiff_structure;

		if (!structure.has_jacobian)
		{
			continue;
		}

		auto local_jacobian_nnz = structure.m_jacobian_nnz;
		auto &local_jacobian_cols = structure.m_jacobian_cols;

		std::vector<int> jacobian_index_buffer(local_jacobian_nnz);
		group.gradient_indices.resize(n_instances * local_jacobian_nnz);

		for (int j = 0; j < n_instances; j++)
		{
			auto instance_index = instance_indices[j];
			auto &variables = graph_inputs[instance_index].variables;

			for (int k = 0; k < local_jacobian_nnz; k++)
			{
				auto col = variables[local_jacobian_cols[k]];

				auto [iter, inserted] =
				    sparse_gradient_map.try_emplace(col, global_gradient_cols.size());
				if (inserted)
				{
					global_gradient_cols.push_back(col);
				}
				jacobian_index_buffer[k] = iter->second;
			}
			std::copy(jacobian_index_buffer.begin(), jacobian_index_buffer.end(),
			          group.gradient_indices.begin() + j * local_jacobian_nnz);
		}
	}
}

void NonlinearEvaluator::eval_constraints_jacobian(const double *restrict x,
                                                   double *restrict jacobian) const
{
	auto &groups = constraint_groups;

	for (const auto &group : groups)
	{
		auto &instance_indices = group.instance_indices;
		auto n_instances = instance_indices.size();
		auto &structure = group.autodiff_structure;
		auto &evaluator = group.autodiff_evaluator;
		if (!structure.has_jacobian)
		{
			continue;
		}
		auto local_jacobian_nnz = structure.m_jacobian_nnz;
		if (!structure.has_parameter)
		{
			for (int j = 0; j < n_instances; j++)
			{
				auto instance_index = instance_indices[j];
				auto &variables = graph_inputs[instance_index].variables;
				evaluator.jacobian_eval.nop(x, jacobian, variables.data());
				jacobian += local_jacobian_nnz;
			}
		}
		else
		{
			for (int j = 0; j < n_instances; j++)
			{
				auto instance_index = instance_indices[j];
				auto &variables = graph_inputs[instance_index].variables;
				auto &constant = graph_inputs[instance_index].constants;
				evaluator.jacobian_eval.p(x, constant.data(), jacobian, variables.data());
				jacobian += local_jacobian_nnz;
			}
		}
	}
}

void NonlinearEvaluator::eval_objective_gradient(const double *restrict x,
                                                 double *restrict grad_f) const
{
	auto &groups = objective_groups;

	for (const auto &group : groups)
	{
		auto &instance_indices = group.instance_indices;
		auto n_instances = instance_indices.size();
		auto &structure = group.autodiff_structure;
		auto &evaluator = group.autodiff_evaluator;
		if (!structure.has_jacobian)
		{
			continue;
		}
		auto local_jacobian_nnz = structure.m_jacobian_nnz;
		const int *grad_index = group.gradient_indices.data();
		if (!structure.has_parameter)
		{
			for (int j = 0; j < n_instances; j++)
			{
				auto instance_index = instance_indices[j];
				auto &variables = graph_inputs[instance_index].variables;
				evaluator.grad_eval.nop(x, grad_f, variables.data(), grad_index);
				grad_index += local_jacobian_nnz;
			}
		}
		else
		{
			for (int j = 0; j < n_instances; j++)
			{
				auto instance_index = instance_indices[j];
				auto &variables = graph_inputs[instance_index].variables;
				auto &constant = graph_inputs[instance_index].constants;
				evaluator.grad_eval.p(x, constant.data(), grad_f, variables.data(), grad_index);
				grad_index += local_jacobian_nnz;
			}
		}
	}
}

void NonlinearEvaluator::analyze_constraints_hessian_structure(
    size_t &global_hessian_nnz, std::vector<int> &global_hessian_rows,
    std::vector<int> &global_hessian_cols, Hashmap<std::tuple<int, int>, int> &hessian_index_map,
    HessianSparsityType hessian_type)
{
	auto &groups = constraint_groups;

	for (auto &group : groups)
	{
		auto &instance_indices = group.instance_indices;
		auto n_instances = instance_indices.size();
		auto &structure = group.autodiff_structure;

		if (!structure.has_hessian)
		{
			continue;
		}

		auto local_hessian_nnz = structure.m_hessian_nnz;
		auto &local_hessian_rows = structure.m_hessian_rows;
		auto &local_hessian_cols = structure.m_hessian_cols;

		std::vector<int> hessian_index_buffer(local_hessian_nnz);
		group.hessian_indices.resize(n_instances * local_hessian_nnz);

		for (int j = 0; j < n_instances; j++)
		{
			auto instance_index = instance_indices[j];
			auto &variables = graph_inputs[instance_index].variables;

			for (int k = 0; k < local_hessian_nnz; k++)
			{
				auto row = variables[local_hessian_rows[k]];
				auto col = variables[local_hessian_cols[k]];

				if (hessian_type == HessianSparsityType::Upper)
				{
					if (row > col)
						std::swap(row, col);
				}
				else
				{
					if (row < col)
						std::swap(row, col);
				}

				auto [iter, inserted] =
				    hessian_index_map.try_emplace({row, col}, global_hessian_nnz);
				if (inserted)
				{
					global_hessian_rows.push_back(row);
					global_hessian_cols.push_back(col);
					global_hessian_nnz += 1;
				}
				auto hessian_index = iter->second;
				hessian_index_buffer[k] = hessian_index;
			}
			std::copy(hessian_index_buffer.begin(), hessian_index_buffer.end(),
			          group.hessian_indices.begin() + j * local_hessian_nnz);
		}
	}
}

void NonlinearEvaluator::analyze_objective_hessian_structure(
    size_t &global_hessian_nnz, std::vector<int> &global_hessian_rows,
    std::vector<int> &global_hessian_cols, Hashmap<std::tuple<int, int>, int> &hessian_index_map,
    HessianSparsityType hessian_type)
{
	auto &groups = objective_groups;

	for (auto &group : groups)
	{
		auto &instance_indices = group.instance_indices;
		auto n_instances = instance_indices.size();
		auto &structure = group.autodiff_structure;

		if (!structure.has_hessian)
		{
			continue;
		}

		auto local_hessian_nnz = structure.m_hessian_nnz;
		auto &local_hessian_rows = structure.m_hessian_rows;
		auto &local_hessian_cols = structure.m_hessian_cols;

		std::vector<int> hessian_index_buffer(local_hessian_nnz);
		group.hessian_indices.resize(n_instances * local_hessian_nnz);

		for (int j = 0; j < n_instances; j++)
		{
			auto instance_index = instance_indices[j];
			auto &variables = graph_inputs[instance_index].variables;

			for (int k = 0; k < local_hessian_nnz; k++)
			{
				auto row = variables[local_hessian_rows[k]];
				auto col = variables[local_hessian_cols[k]];

				if (hessian_type == HessianSparsityType::Upper)
				{
					if (row > col)
						std::swap(row, col);
				}
				else
				{
					if (row < col)
						std::swap(row, col);
				}

				auto [iter, inserted] =
				    hessian_index_map.try_emplace({row, col}, global_hessian_nnz);
				if (inserted)
				{
					global_hessian_rows.push_back(row);
					global_hessian_cols.push_back(col);
					global_hessian_nnz += 1;
				}
				auto hessian_index = iter->second;
				hessian_index_buffer[k] = hessian_index;
			}
			std::copy(hessian_index_buffer.begin(), hessian_index_buffer.end(),
			          group.hessian_indices.begin() + j * local_hessian_nnz);
		}
	}
}

void NonlinearEvaluator::eval_lagrangian_hessian(const double *restrict x,
                                                 const double *restrict lambda,
                                                 const double obj_factor,
                                                 double *restrict hessian) const
{
	// lambda are the multipliers of constraints
	// obj_factor is the multiplier of objective function

	// objective
	{
		auto &groups = objective_groups;
		for (const auto &group : groups)
		{
			auto &instance_indices = group.instance_indices;
			auto n_instances = instance_indices.size();
			auto &structure = group.autodiff_structure;
			auto &evaluator = group.autodiff_evaluator;

			if (!structure.has_hessian)
			{
				continue;
			}

			auto local_hessian_nnz = structure.m_hessian_nnz;
			const int *hessian_index = group.hessian_indices.data();

			if (!structure.has_parameter)
			{
				for (int j = 0; j < n_instances; j++)
				{
					auto instance_index = instance_indices[j];
					auto &variables = graph_inputs[instance_index].variables;
					evaluator.hessian_eval.nop(x, &obj_factor, hessian, variables.data(),
					                           hessian_index);
					hessian_index += local_hessian_nnz;
				}
			}
			else
			{
				for (int j = 0; j < n_instances; j++)
				{
					auto instance_index = instance_indices[j];
					auto &variables = graph_inputs[instance_index].variables;
					auto &constant = graph_inputs[instance_index].constants;
					evaluator.hessian_eval.p(x, constant.data(), &obj_factor, hessian,
					                         variables.data(), hessian_index);
					hessian_index += local_hessian_nnz;
				}
			}
		}
	}

	// constraints
	{
		auto &groups = constraint_groups;
		for (const auto &group : groups)
		{
			auto &instance_indices = group.instance_indices;
			auto n_instances = instance_indices.size();
			auto &structure = group.autodiff_structure;
			auto &evaluator = group.autodiff_evaluator;
			if (!structure.has_hessian)
			{
				continue;
			}
			auto local_hessian_nnz = structure.m_hessian_nnz;
			auto ny = structure.ny;
			const int *hessian_index = group.hessian_indices.data();
			if (!structure.has_parameter)
			{
				for (int j = 0; j < n_instances; j++)
				{
					auto instance_index = instance_indices[j];
					auto &variables = graph_inputs[instance_index].variables;
					evaluator.hessian_eval.nop(x, lambda, hessian, variables.data(), hessian_index);
					hessian_index += local_hessian_nnz;
					lambda += ny;
				}
			}
			else
			{
				for (int j = 0; j < n_instances; j++)
				{
					auto instance_index = instance_indices[j];
					auto &variables = graph_inputs[instance_index].variables;
					auto &constant = graph_inputs[instance_index].constants;
					evaluator.hessian_eval.p(x, constant.data(), lambda, hessian, variables.data(),
					                         hessian_index);
					hessian_index += local_hessian_nnz;
					lambda += ny;
				}
			}
		}
	}
}
