#include "pyoptinterface/ipopt_model.hpp"
#include "pyoptinterface/solver_common.hpp"

#include "fmt/core.h"
#include "fmt/ranges.h"
#include "pyoptinterface/dylib.hpp"
#include <cassert>

static bool is_name_empty(const char *name)
{
	return name == nullptr || name[0] == '\0';
}

namespace ipopt
{
#define B DYLIB_DECLARE
APILIST
#undef B

static DynamicLibrary lib;
static bool is_loaded = false;

bool is_library_loaded()
{
	return is_loaded;
}

bool load_library(const std::string &path)
{
	bool success = lib.try_load(path.c_str());
	if (!success)
	{
		return false;
	}

	DYLIB_LOAD_INIT;

#define B DYLIB_LOAD_FUNCTION
	APILIST
#undef B

	if (IS_DYLIB_LOAD_SUCCESS)
	{
#define B DYLIB_SAVE_FUNCTION
		APILIST
#undef B
		is_loaded = true;
		return true;
	}
	else
	{
		return false;
	}
}
} // namespace ipopt

IpoptModel::IpoptModel()
{
	if (!ipopt::is_library_loaded())
	{
		throw std::runtime_error("IPOPT library is not loaded");
	}
}

void IpoptModel::close()
{
	m_problem.reset();
}

VariableIndex IpoptModel::add_variable(double lb, double ub, double start, const char *name)
{
	VariableIndex vi(n_variables);
	m_var_lb.push_back(lb);
	m_var_ub.push_back(ub);
	m_var_init.push_back(start);
	n_variables += 1;

	if (!is_name_empty(name))
	{
		m_var_names.emplace(vi.index, name);
	}

	m_is_dirty = true;

	return vi;
}

double IpoptModel::get_variable_lb(const VariableIndex &variable)
{
	return m_var_lb[variable.index];
}

double IpoptModel::get_variable_ub(const VariableIndex &variable)
{
	return m_var_ub[variable.index];
}

void IpoptModel::set_variable_lb(const VariableIndex &variable, double lb)
{
	m_var_lb[variable.index] = lb;
}

void IpoptModel::set_variable_ub(const VariableIndex &variable, double ub)
{
	m_var_ub[variable.index] = ub;
}

void IpoptModel::set_variable_bounds(const VariableIndex &variable, double lb, double ub)
{
	m_var_lb[variable.index] = lb;
	m_var_ub[variable.index] = ub;
}

double IpoptModel::get_variable_start(const VariableIndex &variable)
{
	return m_var_init[variable.index];
}

void IpoptModel::set_variable_start(const VariableIndex &variable, double start)
{
	m_var_init[variable.index] = start;
}

double IpoptModel::get_variable_value(const VariableIndex &variable)
{
	return m_result.x[variable.index];
}

std::string IpoptModel::get_variable_name(const VariableIndex &variable)
{
	auto iter = m_var_names.find(variable.index);
	if (iter != m_var_names.end())
	{
		return iter->second;
	}
	else
	{
		return fmt::format("x{}", variable.index);
	}
}

void IpoptModel::set_variable_name(const VariableIndex &variable, const std::string &name)
{
	m_var_names[variable.index] = name;
}

std::string IpoptModel::pprint_variable(const VariableIndex &variable)
{
	return get_variable_name(variable);
}

double IpoptModel::get_obj_value()
{
	return m_result.obj_val;
}

int IpoptModel::_constraint_internal_index(const ConstraintIndex &constraint)
{
	switch (constraint.type)
	{
	case ConstraintType::Linear:
		return constraint.index;
	case ConstraintType::Quadratic:
		return m_linear_con_evaluator.n_constraints + constraint.index;
	case ConstraintType::NL: {
		auto base = m_linear_con_evaluator.n_constraints + m_quadratic_con_evaluator.n_constraints;
		auto internal_nl_index = nl_constraint_map_ext2int[constraint.index];
		return base + internal_nl_index;
	}
	default:
		throw std::runtime_error("Invalid constraint type");
	}
}

double IpoptModel::get_constraint_primal(const ConstraintIndex &constraint)
{
	int index = _constraint_internal_index(constraint);
	return m_result.g[index];
}

double IpoptModel::get_constraint_dual(const ConstraintIndex &constraint)
{
	int index = _constraint_internal_index(constraint);
	auto dual = -m_result.mult_g[index];
	return dual;
}

ConstraintIndex IpoptModel::add_linear_constraint(const ScalarAffineFunction &f,
                                                  ConstraintSense sense, double rhs,
                                                  const char *name)
{
	double lb = -INFINITY;
	double ub = INFINITY;
	if (sense == ConstraintSense::LessEqual)
	{
		ub = rhs;
	}
	else if (sense == ConstraintSense::GreaterEqual)
	{
		lb = rhs;
	}
	else if (sense == ConstraintSense::Equal)
	{
		lb = rhs;
		ub = rhs;
	}
	return add_linear_constraint(f, {lb, ub}, name);
}

ConstraintIndex IpoptModel::add_linear_constraint(const ScalarAffineFunction &f,
                                                  const std::tuple<double, double> &interval,
                                                  const char *name)
{
	ConstraintIndex con(ConstraintType::Linear, m_linear_con_evaluator.n_constraints);
	m_linear_con_evaluator.add_row(f);

	auto lb = std::get<0>(interval);
	auto ub = std::get<1>(interval);
	m_linear_con_lb.push_back(lb);
	m_linear_con_ub.push_back(ub);

	m_is_dirty = true;

	return con;
}

ConstraintIndex IpoptModel::add_quadratic_constraint(const ScalarQuadraticFunction &f,
                                                     ConstraintSense sense, double rhs,
                                                     const char *name)
{
	double lb = -INFINITY;
	double ub = INFINITY;
	if (sense == ConstraintSense::LessEqual)
	{
		ub = rhs;
	}
	else if (sense == ConstraintSense::GreaterEqual)
	{
		lb = rhs;
	}
	else if (sense == ConstraintSense::Equal)
	{
		lb = rhs;
		ub = rhs;
	}
	else
	{
		throw std::runtime_error("'Within' constraint sense must have both LB and UB");
	}
	return add_quadratic_constraint(f, {lb, ub}, name);
}

ConstraintIndex IpoptModel::add_quadratic_constraint(const ScalarQuadraticFunction &f,
                                                     const std::tuple<double, double> &interval,
                                                     const char *name)
{
	ConstraintIndex con(ConstraintType::Quadratic, m_quadratic_con_evaluator.n_constraints);
	m_quadratic_con_evaluator.add_row(f);

	auto lb = std::get<0>(interval);
	auto ub = std::get<1>(interval);
	m_quadratic_con_lb.push_back(lb);
	m_quadratic_con_ub.push_back(ub);

	m_is_dirty = true;

	return con;
}

void IpoptModel::set_objective(const ScalarAffineFunction &expr, ObjectiveSense sense)
{
	_set_linear_objective(expr);
}

void IpoptModel::set_objective(const ScalarQuadraticFunction &expr, ObjectiveSense sense)
{
	_set_quadratic_objective(expr);
}

void IpoptModel::set_objective(const ExprBuilder &expr, ObjectiveSense sense)
{
	auto degree = expr.degree();
	if (degree <= 1)
	{
		_set_linear_objective(expr);
	}
	else if (degree == 2)
	{
		_set_quadratic_objective(expr);
	}
	else
	{
		throw std::runtime_error("Only linear and quadratic objective is supported");
	}

	m_is_dirty = true;
}

void IpoptModel::_set_linear_objective(const ScalarAffineFunction &expr)
{
	LinearEvaluator evaluator;
	evaluator.add_row(expr);
	m_linear_obj_evaluator = evaluator;
	m_quadratic_obj_evaluator.reset();
}

void IpoptModel::_set_quadratic_objective(const ScalarQuadraticFunction &expr)
{
	QuadraticEvaluator evaluator;
	evaluator.add_row(expr);
	m_linear_obj_evaluator.reset();
	m_quadratic_obj_evaluator = evaluator;
}

int IpoptModel::add_graph_index()
{
	return m_nl_evaluator.add_graph_instance();
}

void IpoptModel::finalize_graph_instance(size_t graph_index, const ExpressionGraph &graph)
{
	m_nl_evaluator.finalize_graph_instance(graph_index, graph);
}

int IpoptModel::aggregate_nl_constraint_groups()
{
	return m_nl_evaluator.aggregate_constraint_groups();
}

int IpoptModel::get_nl_constraint_group_representative(int group_index) const
{
	return m_nl_evaluator.get_constraint_group_representative(group_index);
}

int IpoptModel::aggregate_nl_objective_groups()
{
	return m_nl_evaluator.aggregate_objective_groups();
}

int IpoptModel::get_nl_objective_group_representative(int group_index) const
{
	return m_nl_evaluator.get_objective_group_representative(group_index);
}

void IpoptModel::assign_nl_constraint_group_autodiff_structure(
    int group_index, const AutodiffSymbolicStructure &structure)
{
	m_nl_evaluator.assign_constraint_group_autodiff_structure(group_index, structure);
}

void IpoptModel::assign_nl_constraint_group_autodiff_evaluator(
    int group_index, const ConstraintAutodiffEvaluator &evaluator)
{
	m_nl_evaluator.assign_constraint_group_autodiff_evaluator(group_index, evaluator);
}

void IpoptModel::assign_nl_objective_group_autodiff_structure(
    int group_index, const AutodiffSymbolicStructure &structure)
{
	m_nl_evaluator.assign_objective_group_autodiff_structure(group_index, structure);
}

void IpoptModel::assign_nl_objective_group_autodiff_evaluator(
    int group_index, const ObjectiveAutodiffEvaluator &evaluator)
{
	m_nl_evaluator.assign_objective_group_autodiff_evaluator(group_index, evaluator);
}

ConstraintIndex IpoptModel::add_single_nl_constraint(size_t graph_index,
                                                     const ExpressionGraph &graph, double lb,
                                                     double ub)
{
	m_nl_con_lb.push_back(lb);
	m_nl_con_ub.push_back(ub);
	auto constraint_index = n_nl_constraints;
	n_nl_constraints += 1;

	nl_constraint_graph_memberships.push_back(ConstraintGraphMembership{
	    .graph = (int)graph_index, .rank = (int)graph.m_constraint_outputs.size() - 1});

	m_is_dirty = true;

	return ConstraintIndex(ConstraintType::NL, constraint_index);
}

static bool eval_f(ipindex n, ipnumber *x, bool new_x, ipnumber *obj_value, UserDataPtr user_data)
{
	IpoptModel &model = *static_cast<IpoptModel *>(user_data);
	*obj_value = 0.0;
	// fmt::print("Before linear and quad objective, obj_value: {}\n", *obj_value);
	if (model.m_linear_obj_evaluator)
	{
		model.m_linear_obj_evaluator->eval_function(x, obj_value);
	}
	else if (model.m_quadratic_obj_evaluator)
	{
		model.m_quadratic_obj_evaluator->eval_function(x, obj_value);
	}
	// fmt::print("After linear and quad objective, obj_value: {}\n", *obj_value);

	// nonlinear part
	double nl_obj = model.m_nl_evaluator.eval_objective(x);

	*obj_value += nl_obj;

	return true;
}

static bool eval_grad_f(ipindex n, ipnumber *x, bool new_x, ipnumber *grad_f, UserDataPtr user_data)
{
	IpoptModel &model = *static_cast<IpoptModel *>(user_data);
	std::fill(grad_f, grad_f + n, 0.0);

	// fmt::print("Enters eval_grad_f\n");

	// fill sparse_gradient_values
	auto &sparse_gradient_values = model.sparse_gradient_values;
	std::fill(sparse_gradient_values.begin(), sparse_gradient_values.end(), 0.0);

	// analytical part
	if (model.m_linear_obj_evaluator)
	{
		model.m_linear_obj_evaluator->eval_jacobian(x, sparse_gradient_values.data());
	}
	else if (model.m_quadratic_obj_evaluator)
	{
		model.m_quadratic_obj_evaluator->eval_jacobian(x, sparse_gradient_values.data());
	}

	// nonlinear part
	model.m_nl_evaluator.eval_objective_gradient(x, sparse_gradient_values.data());

	// copy to grad_f
	for (size_t i = 0; i < model.sparse_gradient_indices.size(); i++)
	{
		auto index = model.sparse_gradient_indices[i];
		auto value = sparse_gradient_values[i];
		grad_f[index] += value;
	}

	// debug
	/*fmt::print("Current x: {}\n", std::vector<double>(x, x + n));
	fmt::print("Current gradient: {}\n", std::vector<double>(grad_f, grad_f + n));*/

	return true;
}

static bool eval_g(ipindex n, ipnumber *x, bool new_x, ipindex m, ipnumber *g,
                   UserDataPtr user_data)
{
	IpoptModel &model = *static_cast<IpoptModel *>(user_data);
	// std::fill(g, g + m, 0.0);

	// fmt::print("Enters eval_g\n");
	auto original_g = g;

	// linear part
	model.m_linear_con_evaluator.eval_function(x, g);

	// quadratic part
	g += model.m_linear_con_evaluator.n_constraints;
	model.m_quadratic_con_evaluator.eval_function(x, g);

	// nonlinear part
	g += model.m_quadratic_con_evaluator.n_constraints;
	model.m_nl_evaluator.eval_constraints(x, g);

	// debug
	/*fmt::print("Current x: {}\n", std::vector<double>(x, x + n));
	fmt::print("Current g: {}\n", std::vector<double>(original_g, original_g + m));*/

	return true;
}

static bool eval_jac_g(ipindex n, ipnumber *x, bool new_x, ipindex m, ipindex nele_jac,
                       ipindex *iRow, ipindex *jCol, ipnumber *values, UserDataPtr user_data)
{
	IpoptModel &model = *static_cast<IpoptModel *>(user_data);

	// fmt::print("Enters eval_jac_g\n");

	if (iRow != nullptr)
	{
		auto &rows = model.m_jacobian_rows;
		auto &cols = model.m_jacobian_cols;
		std::copy(rows.begin(), rows.end(), iRow);
		std::copy(cols.begin(), cols.end(), jCol);
	}
	else
	{
		// std::fill(values, values + nele_jac, 0.0);
		auto original_jacobian = values;

		// fmt::print("Initial jacobian: {}\n", std::vector<double>(values, values + nele_jac));

		// linear part
		model.m_linear_con_evaluator.eval_jacobian(x, values);

		// quadratic part
		/*fmt::print("jacobian forwards {} for linear part\n",
		           model.m_linear_con_evaluator.coefs.size());*/
		values += model.m_linear_con_evaluator.coefs.size();
		model.m_quadratic_con_evaluator.eval_jacobian(x, values);

		// nonlinear part
		/*fmt::print("jacobian forwards {} for quadratic part\n",
		           model.m_quadratic_con_evaluator.jacobian_nnz);*/
		values += model.m_quadratic_con_evaluator.jacobian_nnz;
		model.m_nl_evaluator.eval_constraints_jacobian(x, values);

		// debug
		/*fmt::print("Current x: {}\n", std::vector<double>(x, x + n));
		fmt::print("Current jacobian: {}\n",
		           std::vector<double>(original_jacobian, original_jacobian + nele_jac));*/
	}
	return true;
}

static bool eval_h(ipindex n, ipnumber *x, bool new_x, ipnumber obj_factor, ipindex m,
                   ipnumber *lambda, bool new_lambda, ipindex nele_hess, ipindex *iRow,
                   ipindex *jCol, ipnumber *values, UserDataPtr user_data)
{
	IpoptModel &model = *static_cast<IpoptModel *>(user_data);

	// fmt::print("Enters eval_h\n");

	if (iRow != nullptr)
	{
		auto &rows = model.m_hessian_rows;
		auto &cols = model.m_hessian_cols;
		std::copy(rows.begin(), rows.end(), iRow);
		std::copy(cols.begin(), cols.end(), jCol);
	}
	else
	{
		std::fill(values, values + nele_hess, 0.0);

		// objective

		// quadratic part
		if (model.m_quadratic_obj_evaluator)
		{
			model.m_quadratic_obj_evaluator->eval_lagrangian_hessian(&obj_factor, values);
		}

		// constraint

		// quadratic part
		lambda += model.m_linear_con_evaluator.n_constraints;
		model.m_quadratic_con_evaluator.eval_lagrangian_hessian(lambda, values);

		// nonlinear part
		lambda += model.m_quadratic_con_evaluator.n_constraints;
		model.m_nl_evaluator.eval_lagrangian_hessian(x, lambda, obj_factor, values);

		// debug
		/*fmt::print("Current x: {}\n", std::vector<double>(x, x + n));
		fmt::print("Current obj_factor: {}\n", obj_factor);
		fmt::print("Current lambda: {}\n", std::vector<double>(lambda, lambda + m));
		fmt::print("Current hessian: {}\n", std::vector<double>(values, values + nele_hess));*/
	}
	return true;
}

void IpoptModel::analyze_structure()
{
	// init variables
	m_jacobian_nnz = 0;
	m_jacobian_rows.clear();
	m_jacobian_cols.clear();
	m_hessian_nnz = 0;
	m_hessian_rows.clear();
	m_hessian_cols.clear();
	m_hessian_index_map.clear();

	// constraints

	// analyze linear part
	m_linear_con_evaluator.analyze_jacobian_structure(m_jacobian_nnz, m_jacobian_rows,
	                                                  m_jacobian_cols);

	// analyze quadratic part
	m_quadratic_con_evaluator.analyze_jacobian_structure(
	    m_linear_con_evaluator.n_constraints, m_jacobian_nnz, m_jacobian_rows, m_jacobian_cols);
	m_quadratic_con_evaluator.analyze_hessian_structure(m_hessian_nnz, m_hessian_rows,
	                                                    m_hessian_cols, m_hessian_index_map,
	                                                    HessianSparsityType::Lower);

	// objective
	sparse_gradient_indices.clear();
	sparse_gradient_values.clear();
	Hashmap<int, int> sparse_gradient_map;

	// linear and quadratic objective
	if (m_linear_obj_evaluator)
	{
		auto &evaluator = m_linear_obj_evaluator.value();
		sparse_gradient_indices.insert(sparse_gradient_indices.end(), evaluator.indices.begin(),
		                               evaluator.indices.end());
	}
	else if (m_quadratic_obj_evaluator)
	{
		auto &evaluator = m_quadratic_obj_evaluator.value();
		sparse_gradient_indices.insert(sparse_gradient_indices.end(),
		                               evaluator.jacobian_variable_indices.begin(),
		                               evaluator.jacobian_variable_indices.end());
		evaluator.analyze_hessian_structure(m_hessian_nnz, m_hessian_rows, m_hessian_cols,
		                                    m_hessian_index_map, HessianSparsityType::Lower);
	}
	// update map
	for (int i = 0; i < sparse_gradient_indices.size(); i++)
	{
		auto index = sparse_gradient_indices[i];
		sparse_gradient_map.emplace(index, i);
	}

	// analyze nonlinear constraints and objectives
	{
		auto &evaluator = m_nl_evaluator;

		auto constraint_counter =
		    m_linear_con_evaluator.n_constraints + m_quadratic_con_evaluator.n_constraints;
		evaluator.analyze_constraints_jacobian_structure(constraint_counter, m_jacobian_nnz,
		                                                 m_jacobian_rows, m_jacobian_cols);
		evaluator.analyze_objective_gradient_structure(sparse_gradient_indices,
		                                               sparse_gradient_map);
		evaluator.analyze_constraints_hessian_structure(m_hessian_nnz, m_hessian_rows,
		                                                m_hessian_cols, m_hessian_index_map,
		                                                HessianSparsityType::Lower);
		evaluator.analyze_objective_hessian_structure(m_hessian_nnz, m_hessian_rows, m_hessian_cols,
		                                              m_hessian_index_map,
		                                              HessianSparsityType::Lower);
	}

	sparse_gradient_values.resize(sparse_gradient_indices.size());

	// update the mapping of nl constraint
	nl_constraint_map_ext2int.resize(n_nl_constraints);
	m_nl_evaluator.calculate_constraint_graph_instances_offset();
	for (int i = 0; i < n_nl_constraints; i++)
	{
		auto i_nl_con = i;
		auto i_graph_instance = nl_constraint_graph_memberships[i].graph;
		auto i_graph_rank = nl_constraint_graph_memberships[i].rank;

		auto index_base = m_nl_evaluator.constraint_indices_offsets[i_graph_instance];

		nl_constraint_map_ext2int[i_nl_con] = index_base + i_graph_rank;
	}

	// construct the lower bound and upper bound of the constraints
	auto n_constraints = m_linear_con_evaluator.n_constraints +
	                     m_quadratic_con_evaluator.n_constraints + n_nl_constraints;
	m_con_lb.resize(n_constraints);
	m_con_ub.resize(n_constraints);
	std::copy(m_linear_con_lb.begin(), m_linear_con_lb.end(), m_con_lb.begin());
	std::copy(m_linear_con_ub.begin(), m_linear_con_ub.end(), m_con_ub.begin());
	std::copy(m_quadratic_con_lb.begin(), m_quadratic_con_lb.end(),
	          m_con_lb.begin() + m_linear_con_evaluator.n_constraints);
	std::copy(m_quadratic_con_ub.begin(), m_quadratic_con_ub.end(),
	          m_con_ub.begin() + m_linear_con_evaluator.n_constraints);

	// nonlinear parts need mapping
	auto nl_constraint_start =
	    m_linear_con_evaluator.n_constraints + m_quadratic_con_evaluator.n_constraints;
	for (int i = 0; i < n_nl_constraints; i++)
	{
		auto index = nl_constraint_map_ext2int[i];
		m_con_lb[nl_constraint_start + index] = m_nl_con_lb[i];
		m_con_ub[nl_constraint_start + index] = m_nl_con_ub[i];
	}
}

void IpoptModel::optimize()
{
	analyze_structure();

	auto n_constraints = m_linear_con_evaluator.n_constraints +
	                     m_quadratic_con_evaluator.n_constraints + n_nl_constraints;

	/*fmt::print("Problem has {} variables and {} constraints.\n", n_variables, n_constraints);
	fmt::print("Variable LB: {}\n", m_var_lb);
	fmt::print("Variable UB: {}\n", m_var_ub);
	fmt::print("Constraint LB: {}\n", m_con_lb);
	fmt::print("Constraint UB: {}\n", m_con_ub);
	fmt::print("Jacobian has {} nonzeros\n", m_jacobian_nnz);
	fmt::print("Jacobian rows : {}\n", m_jacobian_rows);
	fmt::print("Jacobian cols : {}\n", m_jacobian_cols);
	fmt::print("Hessian has {} nonzeros\n", m_hessian_nnz);
	fmt::print("Hessian rows : {}\n", m_hessian_rows);
	fmt::print("Hessian cols : {}\n", m_hessian_cols);*/

	/*if (m_quadratic_obj_evaluator)
	{
	    auto &evaluator = m_quadratic_obj_evaluator.value();
	    fmt::print("Diag coefs : {}\n", evaluator.diag_coefs);
	    fmt::print("Diag indices : {}\n", evaluator.diag_indices);
	    fmt::print("Diag intervals : {}\n", evaluator.diag_intervals);
	    fmt::print("OffDiag coefs : {}\n", evaluator.offdiag_coefs);
	    fmt::print("OffDiag rows : {}\n", evaluator.offdiag_rows);
	    fmt::print("OffDiag cols : {}\n", evaluator.offdiag_cols);
	    fmt::print("OffDiag intervals : {}\n", evaluator.offdiag_intervals);
	    fmt::print("hessian_diag_indices : {}\n", evaluator.hessian_diag_indices);
	    fmt::print("hessian_offdiag_indices : {}\n", evaluator.hessian_offdiag_indices);
	}

	{
	    auto &evaluator = m_quadratic_con_evaluator;
	    fmt::print("Diag coefs : {}\n", evaluator.diag_coefs);
	    fmt::print("Diag indices : {}\n", evaluator.diag_indices);
	    fmt::print("Diag intervals : {}\n", evaluator.diag_intervals);
	    fmt::print("OffDiag coefs : {}\n", evaluator.offdiag_coefs);
	    fmt::print("OffDiag rows : {}\n", evaluator.offdiag_rows);
	    fmt::print("OffDiag cols : {}\n", evaluator.offdiag_cols);
	    fmt::print("OffDiag intervals : {}\n", evaluator.offdiag_intervals);
	    fmt::print("hessian_diag_indices : {}\n", evaluator.hessian_diag_indices);
	    fmt::print("hessian_offdiag_indices : {}\n", evaluator.hessian_offdiag_indices);
	}*/

	auto problem_ptr =
	    ipopt::CreateIpoptProblem(n_variables, m_var_lb.data(), m_var_ub.data(), n_constraints,
	                              m_con_lb.data(), m_con_ub.data(), m_jacobian_nnz, m_hessian_nnz,
	                              0, &eval_f, &eval_g, &eval_grad_f, &eval_jac_g, &eval_h);

	m_problem = std::unique_ptr<IpoptProblemInfo, IpoptfreeproblemT>(problem_ptr);

	// set options
	for (auto &[key, value] : m_options_int)
	{
		bool ret = ipopt::AddIpoptIntOption(problem_ptr, (char *)key.c_str(), value);
		if (!ret)
		{
			fmt::print("Failed to set integer option {}\n", key);
		}
	}
	for (auto &[key, value] : m_options_num)
	{
		bool ret = ipopt::AddIpoptNumOption(problem_ptr, (char *)key.c_str(), value);
		if (!ret)
		{
			fmt::print("Failed to set number option {}\n", key);
		}
	}
	for (auto &[key, value] : m_options_str)
	{
		bool ret =
		    ipopt::AddIpoptStrOption(problem_ptr, (char *)key.c_str(), (char *)value.c_str());
		if (!ret)
		{
			fmt::print("Failed to set string option {}\n", key);
		}
	}

	// initialize the solution
	m_result.x.resize(n_variables);
	std::copy(m_var_init.begin(), m_var_init.end(), m_result.x.begin());
	m_result.mult_x_L.resize(n_variables);
	m_result.mult_x_U.resize(n_variables);
	m_result.g.resize(n_constraints);
	m_result.mult_g.resize(n_constraints);

	m_status = ipopt::IpoptSolve(problem_ptr, m_result.x.data(), m_result.g.data(),
	                             &m_result.obj_val, m_result.mult_g.data(),
	                             m_result.mult_x_L.data(), m_result.mult_x_U.data(), (void *)this);
	m_result.is_valid = true;
	m_is_dirty = false;
}

void IpoptModel::load_current_solution()
{
	if (!m_result.is_valid)
	{
		throw std::runtime_error("No valid solution to load");
	}

	std::copy(m_result.x.begin(), m_result.x.end(), m_var_init.begin());
}

void IpoptModel::set_raw_option_int(const std::string &name, int value)
{
	m_options_int[name] = value;
}

void IpoptModel::set_raw_option_double(const std::string &name, double value)
{
	m_options_num[name] = value;
}

void IpoptModel::set_raw_option_string(const std::string &name, const std::string &value)
{
	m_options_str[name] = value;
}
