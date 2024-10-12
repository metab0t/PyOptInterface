#include "pyoptinterface/ipopt_model.hpp"
#include "pyoptinterface/solver_common.hpp"

#include "fmt/core.h"
#include "fmt/ranges.h"
#include "pyoptinterface/dylib.hpp"

static bool is_name_empty(const char *name)
{
	return name == nullptr || name[0] == '\0';
}

namespace ipopt
{
#define B(f) decltype(&::f) f = nullptr

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

#define B(f)                                                          \
	{                                                                 \
		auto ptr = reinterpret_cast<decltype(f)>(lib.get_symbol(#f)); \
		if (ptr == nullptr)                                           \
		{                                                             \
			fmt::print("function {} is not loaded correctly", #f);    \
			return false;                                             \
		}                                                             \
		f = ptr;                                                      \
	}
	APILIST
#undef B

	is_loaded = true;

	return true;
}
} // namespace ipopt

IpoptModel::IpoptModel()
{
	if (!ipopt::is_library_loaded())
	{
		throw std::runtime_error("IPOPT library is not loaded");
	}
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

double IpoptModel::get_expression_value(const ScalarAffineFunction &function)
{
	return ::get_affine_expression_value(this, function);
}

double IpoptModel::get_expression_value(const ScalarQuadraticFunction &function)
{
	return ::get_quadratic_expression_value(this, function);
}

double IpoptModel::get_expression_value(const ExprBuilder &function)
{
	return ::get_expression_builder_value(this, function);
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

std::string IpoptModel::pprint_expression(const ScalarAffineFunction &function, int precision)
{
	return ::pprint_affine_expression(this, function, precision);
}

std::string IpoptModel::pprint_expression(const ScalarQuadraticFunction &function, int precision)
{
	return ::pprint_quadratic_expression(this, function, precision);
}

std::string IpoptModel::pprint_expression(const ExprBuilder &function, int precision)
{
	return ::pprint_expression_builder(this, function, precision);
}

ParameterIndex IpoptModel::add_parameter(double value)
{
	return m_function_model.add_parameter(value);
}

void IpoptModel::set_parameter(const ParameterIndex &parameter, double value)
{
	m_function_model.set_parameter(parameter, value);
}

double IpoptModel::get_obj_value()
{
	return m_result.obj_val;
}

double IpoptModel::get_constraint_primal(IndexT index)
{
	return m_result.g[index];
}

double IpoptModel::get_constraint_dual(IndexT index)
{
	return m_result.mult_g[index];
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
	return add_linear_constraint(f, ConstraintSense::Within, lb, ub, name);
}

ConstraintIndex IpoptModel::add_linear_constraint(const ScalarAffineFunction &f,
                                                  ConstraintSense sense, double lb, double ub,
                                                  const char *name)
{
	if (sense != ConstraintSense::Within)
	{
		throw std::runtime_error(
		    "Only 'Within' constraint sense is supported when LB and UB is used together");
	}
	ConstraintIndex con(ConstraintType::Linear, n_constraints);
	m_lq_model.add_linear_constraint(f, n_constraints);
	m_con_lb.push_back(lb);
	m_con_ub.push_back(ub);
	n_constraints += 1;

	if (!is_name_empty(name))
	{
		m_con_names.emplace(con.index, name);
	}

	return con;
}

ConstraintIndex IpoptModel::add_linear_constraint(const ExprBuilder &f, ConstraintSense sense,
                                                  double rhs, const char *name)
{
	return add_linear_constraint(ScalarAffineFunction(f), sense, rhs, rhs, name);
}

ConstraintIndex IpoptModel::add_linear_constraint(const ExprBuilder &f, ConstraintSense sense,
                                                  double lb, double ub, const char *name)
{
	return add_linear_constraint(ScalarAffineFunction(f), sense, lb, ub, name);
}

ConstraintIndex IpoptModel::add_linear_constraint(const VariableIndex &f, ConstraintSense sense,
                                                  double rhs, const char *name)
{
	return add_linear_constraint(ScalarAffineFunction(f), sense, rhs, rhs, name);
}

ConstraintIndex IpoptModel::add_linear_constraint(const VariableIndex &f, ConstraintSense sense,
                                                  double lb, double ub, const char *name)
{
	return add_linear_constraint(ScalarAffineFunction(f), sense, lb, ub, name);
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
	return add_quadratic_constraint(f, ConstraintSense::Within, lb, ub, name);
}

ConstraintIndex IpoptModel::add_quadratic_constraint(const ScalarQuadraticFunction &f,
                                                     ConstraintSense sense, double lb, double ub,
                                                     const char *name)
{
	if (sense != ConstraintSense::Within)
	{
		throw std::runtime_error(
		    "Only 'Within' constraint sense is supported when LB and UB is used together");
	}
	ConstraintIndex con(ConstraintType::Quadratic, n_constraints);
	m_lq_model.add_quadratic_constraint(f, n_constraints);
	m_con_lb.push_back(lb);
	m_con_ub.push_back(ub);
	n_constraints += 1;

	if (!is_name_empty(name))
	{
		m_con_names.emplace(con.index, name);
	}

	return con;
}

ConstraintIndex IpoptModel::add_quadratic_constraint(const ExprBuilder &f, ConstraintSense sense,
                                                     double rhs, const char *name)
{
	return add_quadratic_constraint(ScalarQuadraticFunction(f), sense, rhs, name);
}

ConstraintIndex IpoptModel::add_quadratic_constraint(const ExprBuilder &f, ConstraintSense sense,
                                                     double lb, double ub, const char *name)
{
	return add_quadratic_constraint(ScalarQuadraticFunction(f), sense, lb, ub, name);
}

FunctionIndex IpoptModel::register_function(ADFunD &f, const std::string &name,
                                            const std::vector<double> &x_values,
                                            const std::vector<double> &p_values)
{
	return m_function_model.register_function(f, name, x_values, p_values);
}

NLConstraintIndex IpoptModel::add_empty_nl_constraint(int dim, ConstraintSense sense,
                                                      const std::vector<double> &rhss)
{
	NLConstraintIndex con;
	con.index = n_constraints;
	con.dim = dim;
	n_constraints += dim;

	auto ny = dim;
	if (sense == ConstraintSense::LessEqual)
	{
		for (size_t i = 0; i < ny; i++)
		{
			m_con_lb.push_back(-INFINITY);
			m_con_ub.push_back(rhss[i]);
		}
	}
	else if (sense == ConstraintSense::GreaterEqual)
	{
		for (size_t i = 0; i < ny; i++)
		{
			m_con_lb.push_back(rhss[i]);
			m_con_ub.push_back(INFINITY);
		}
	}
	else if (sense == ConstraintSense::Equal)
	{
		for (size_t i = 0; i < ny; i++)
		{
			m_con_lb.push_back(rhss[i]);
			m_con_ub.push_back(rhss[i]);
		}
	}

	return con;
}

NLConstraintIndex IpoptModel::add_empty_nl_constraint(int dim, ConstraintSense sense,
                                                      const std::vector<double> &lbs,
                                                      const std::vector<double> &ubs)
{
	if (sense != ConstraintSense::Within)
	{
		throw std::runtime_error(
		    "Only 'Within' constraint sense is supported when LB and UB is used together");
	}

	NLConstraintIndex con;
	con.index = n_constraints;
	con.dim = dim;
	n_constraints += dim;

	auto ny = dim;
	for (size_t i = 0; i < ny; i++)
	{
		m_con_lb.push_back(lbs[i]);
		m_con_ub.push_back(ubs[i]);
	}

	return con;
}

NLConstraintIndex IpoptModel::add_nl_constraint(const FunctionIndex &k,
                                                const std::vector<VariableIndex> &xs,
                                                const std::vector<ParameterIndex> &ps,
                                                ConstraintSense sense,
                                                const std::vector<double> &rhss)
{
	if (sense == ConstraintSense::Within)
	{
		throw std::runtime_error("'Within' constraint sense must have both LB and UB");
	}

	auto ny = m_function_model.nl_functions[k.index].ny;
	auto nlcon = m_function_model.add_nl_constraint(k, xs, ps, n_constraints);
	n_constraints += ny;

	if (sense == ConstraintSense::LessEqual)
	{
		for (size_t i = 0; i < ny; i++)
		{
			m_con_lb.push_back(-INFINITY);
			m_con_ub.push_back(rhss[i]);
		}
	}
	else if (sense == ConstraintSense::GreaterEqual)
	{
		for (size_t i = 0; i < ny; i++)
		{
			m_con_lb.push_back(rhss[i]);
			m_con_ub.push_back(INFINITY);
		}
	}
	else if (sense == ConstraintSense::Equal)
	{
		for (size_t i = 0; i < ny; i++)
		{
			m_con_lb.push_back(rhss[i]);
			m_con_ub.push_back(rhss[i]);
		}
	}

	return nlcon;
}

NLConstraintIndex IpoptModel::add_nl_constraint(const FunctionIndex &k,
                                                const std::vector<VariableIndex> &xs,
                                                const std::vector<double> &ps,
                                                ConstraintSense sense,
                                                const std::vector<double> &rhss)
{
	std::vector<ParameterIndex> real_ps;
	real_ps.reserve(ps.size());
	for (auto p : ps)
	{
		real_ps.push_back(add_parameter(p));
	}
	return add_nl_constraint(k, xs, real_ps, sense, rhss);
}

NLConstraintIndex IpoptModel::add_nl_constraint(const FunctionIndex &k,
                                                const std::vector<VariableIndex> &xs,
                                                ConstraintSense sense,
                                                const std::vector<double> &rhss)
{
	std::vector<ParameterIndex> ps;
	return add_nl_constraint(k, xs, ps, sense, rhss);
}

NLConstraintIndex IpoptModel::add_nl_constraint(const FunctionIndex &k,
                                                const std::vector<VariableIndex> &xs,
                                                const std::vector<ParameterIndex> &ps,
                                                ConstraintSense sense,
                                                const std::vector<double> &lbs,
                                                const std::vector<double> &ubs)
{
	if (sense != ConstraintSense::Within)
	{
		throw std::runtime_error(
		    "Only 'Within' constraint sense is supported when LB and UB is used together");
	}

	auto ny = m_function_model.nl_functions[k.index].ny;
	auto nlcon = m_function_model.add_nl_constraint(k, xs, ps, n_constraints);
	n_constraints += ny;

	for (size_t i = 0; i < ny; i++)
	{
		m_con_lb.push_back(lbs[i]);
		m_con_ub.push_back(ubs[i]);
	}

	return nlcon;
}

NLConstraintIndex IpoptModel::add_nl_constraint(
    const FunctionIndex &k, const std::vector<VariableIndex> &xs, const std::vector<double> &ps,
    ConstraintSense sense, const std::vector<double> &lbs, const std::vector<double> &ubs)
{
	std::vector<ParameterIndex> real_ps;
	real_ps.reserve(ps.size());
	for (auto p : ps)
	{
		real_ps.push_back(add_parameter(p));
	}
	return add_nl_constraint(k, xs, real_ps, sense, lbs, ubs);
}

NLConstraintIndex IpoptModel::add_nl_constraint(const FunctionIndex &k,
                                                const std::vector<VariableIndex> &xs,
                                                ConstraintSense sense,
                                                const std::vector<double> &lbs,
                                                const std::vector<double> &ubs)
{
	std::vector<ParameterIndex> ps;
	return add_nl_constraint(k, xs, ps, sense, lbs, ubs);
}

void IpoptModel::add_nl_expression(const NLConstraintIndex &constraint, const FunctionIndex &k,
                                   const std::vector<VariableIndex> &xs,
                                   const std::vector<ParameterIndex> &ps)
{
	auto dim = constraint.dim;
	auto ny = m_function_model.nl_functions[k.index].ny;
	assert(ny == dim);

	m_function_model.add_nl_constraint(k, xs, ps, constraint.index);
}

void IpoptModel::add_nl_expression(const NLConstraintIndex &constraint, const FunctionIndex &k,
                                   const std::vector<VariableIndex> &xs,
                                   const std::vector<double> &ps)
{
	std::vector<ParameterIndex> real_ps;
	real_ps.reserve(ps.size());
	for (auto p : ps)
	{
		real_ps.push_back(add_parameter(p));
	}
	add_nl_expression(constraint, k, xs, real_ps);
}

void IpoptModel::add_nl_expression(const NLConstraintIndex &constraint, const FunctionIndex &k,
                                   const std::vector<VariableIndex> &xs)
{
	std::vector<ParameterIndex> ps;
	add_nl_expression(constraint, k, xs, ps);
}

void IpoptModel::add_nl_objective(const FunctionIndex &k, const std::vector<VariableIndex> &xs)
{
	std::vector<ParameterIndex> ps;
	add_nl_objective(k, xs, ps);
}

void IpoptModel::add_nl_objective(const FunctionIndex &k, const std::vector<VariableIndex> &xs,
                                  const std::vector<ParameterIndex> &ps)
{
	m_function_model.add_nl_objective(k, xs, ps);
}

void IpoptModel::add_nl_objective(const FunctionIndex &k, const std::vector<VariableIndex> &xs,
                                  const std::vector<double> &ps)
{
	std::vector<ParameterIndex> real_ps;
	real_ps.reserve(ps.size());
	for (auto p : ps)
	{
		real_ps.push_back(add_parameter(p));
	}
	add_nl_objective(k, xs, real_ps);
}

void IpoptModel::clear_nl_objective()
{
	m_function_model.clear_nl_objective();
}

static bool eval_f(ipindex n, ipnumber *x, bool new_x, ipnumber *obj_value, UserDataPtr user_data)
{
	IpoptModel &model = *static_cast<IpoptModel *>(user_data);
	obj_value[0] = 0.0;
	model.m_function_model.eval_objective(x, obj_value);
	model.m_lq_model.eval_objective(x, obj_value);
	return true;
}

static bool eval_grad_f(ipindex n, ipnumber *x, bool new_x, ipnumber *grad_f, UserDataPtr user_data)
{
	IpoptModel &model = *static_cast<IpoptModel *>(user_data);
	std::fill(grad_f, grad_f + n, 0.0);
	model.m_function_model.eval_objective_gradient(x, grad_f);
	model.m_lq_model.eval_objective_gradient(x, grad_f);
	return true;
}

static bool eval_g(ipindex n, ipnumber *x, bool new_x, ipindex m, ipnumber *g,
                   UserDataPtr user_data)
{
	IpoptModel &model = *static_cast<IpoptModel *>(user_data);
	std::fill(g, g + m, 0.0);
	model.m_function_model.eval_constraint(x, g);
	model.m_lq_model.eval_constraint(x, g);
	return true;
}

static bool eval_jac_g(ipindex n, ipnumber *x, bool new_x, ipindex m, ipindex nele_jac,
                       ipindex *iRow, ipindex *jCol, ipnumber *values, UserDataPtr user_data)
{
	IpoptModel &model = *static_cast<IpoptModel *>(user_data);
	if (iRow != nullptr)
	{
		auto &rows = model.m_jacobian_rows;
		auto &cols = model.m_jacobian_cols;
		std::copy(rows.begin(), rows.end(), iRow);
		std::copy(cols.begin(), cols.end(), jCol);
	}
	else
	{
		std::fill(values, values + nele_jac, 0.0);
		model.m_function_model.eval_constraint_jacobian(x, values);
		model.m_lq_model.eval_constraint_jacobian(x, values);
	}
	return true;
}

static bool eval_h(ipindex n, ipnumber *x, bool new_x, ipnumber obj_factor, ipindex m,
                   ipnumber *lambda, bool new_lambda, ipindex nele_hess, ipindex *iRow,
                   ipindex *jCol, ipnumber *values, UserDataPtr user_data)
{
	IpoptModel &model = *static_cast<IpoptModel *>(user_data);
	if (iRow != nullptr)
	{
		if (model.if_compact_hessian)
		{
			auto &rows = model.m_hessian_rows_compact;
			auto &cols = model.m_hessian_cols_compact;
			std::copy(rows.begin(), rows.end(), iRow);
			std::copy(cols.begin(), cols.end(), jCol);
		}
		else
		{
			auto &rows = model.m_hessian_rows;
			auto &cols = model.m_hessian_cols;
			std::copy(rows.begin(), rows.end(), iRow);
			std::copy(cols.begin(), cols.end(), jCol);
		}
	}
	else
	{
		std::fill(values, values + nele_hess, 0.0);

		if (model.if_compact_hessian)
		{
			double *hessian_buffer = model.m_hessian_buffer.data();
			model.m_function_model.eval_lagrangian_hessian(x, &obj_factor, lambda, hessian_buffer);
			model.m_lq_model.eval_lagrangian_hessian(x, &obj_factor, lambda, hessian_buffer);

			// compact the Hessian
			accumulate_duplicate_values(model.m_hessian_buffer, model.m_hessian_permute_indices,
			                            model.m_hessian_permute_offsets, values);
		}
		else
		{
			model.m_function_model.eval_lagrangian_hessian(x, &obj_factor, lambda, values);
			model.m_lq_model.eval_lagrangian_hessian(x, &obj_factor, lambda, values);
		}
	}
	return true;
}

void IpoptModel::optimize()
{
	// analyze structure
	m_function_model.analyze_active_functions();
	m_function_model.analyze_dense_gradient_structure();
	m_function_model.analyze_jacobian_structure(m_jacobian_nnz, m_jacobian_rows, m_jacobian_cols);
	m_function_model.analyze_hessian_structure(m_hessian_nnz, m_hessian_rows, m_hessian_cols,
	                                           HessianSparsityType::Lower);

	m_lq_model.analyze_dense_gradient_structure();
	m_lq_model.analyze_jacobian_structure(m_jacobian_nnz, m_jacobian_rows, m_jacobian_cols);
	m_lq_model.analyze_hessian_structure(m_hessian_nnz, m_hessian_rows, m_hessian_cols,
	                                     HessianSparsityType::Lower);

	// compress the nonzeros of Hessian
	if (if_compact_hessian)
	{
		preprocess_duplicate_indices_2d(m_hessian_rows, m_hessian_cols, m_hessian_rows_compact,
		                                m_hessian_cols_compact, m_hessian_permute_indices,
		                                m_hessian_permute_offsets);
		m_hessian_nnz_compact = m_hessian_rows_compact.size();
		m_hessian_buffer.resize(m_hessian_nnz);
	}
	else
	{
		m_hessian_nnz_compact = m_hessian_nnz;
	}

	/*fmt::print("Problem has {} variables and {} constraints\n", n_variables, n_constraints);
	fmt::print("Jacobian has {} nonzeros\n", m_jacobian_nnz);
	fmt::print("Jacobian rows : {}\n", m_jacobian_rows);
	fmt::print("Jacobian cols : {}\n", m_jacobian_cols);
	fmt::print("Hessian has {} nonzeros\n", m_hessian_nnz);
	fmt::print("Hessian rows : {}\n", m_hessian_rows);
	fmt::print("Hessian cols : {}\n", m_hessian_cols);*/

	auto problem_ptr = ipopt::CreateIpoptProblem(n_variables, m_var_lb.data(), m_var_ub.data(),
	                                             n_constraints, m_con_lb.data(), m_con_ub.data(),
	                                             m_jacobian_nnz, m_hessian_nnz_compact, 0, &eval_f,
	                                             &eval_g, &eval_grad_f, &eval_jac_g, &eval_h);

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
