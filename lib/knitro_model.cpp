#include "pyoptinterface/knitro_model.hpp"
#include "pyoptinterface/solver_common.hpp"

#include "fmt/core.h"
#include "fmt/ranges.h"
#include <cassert>
#include <cmath>
#include <iostream>

namespace knitro
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
} // namespace knitro

KnitroModel::KnitroModel()
{
	init();
}

void KnitroModel::init()
{
	if (!knitro::is_library_loaded())
	{
		throw std::runtime_error("KNITRO library is not loaded");
	}

	KN_context *kc_ptr = nullptr;
	int error = knitro::KN_new(&kc_ptr);
	check_error(error);

	m_kc = std::unique_ptr<KN_context, KnitroFreeProblemT>(kc_ptr);
}

void KnitroModel::close()
{
	m_kc.reset();
}

void KnitroModel::check_error(int error) const
{
	if (error != 0)
	{
		throw std::runtime_error(fmt::format("KNITRO error code: {}", error));
	}
}

double KnitroModel::get_infinity() const
{
	return KN_INFINITY;
}

VariableIndex KnitroModel::add_variable(VariableDomain domain, double lb, double ub,
                                        const char *name)
{
	KNINT indexVar;
	int error = knitro::KN_add_var(m_kc.get(), &indexVar);
	check_error(error);

	VariableIndex variable(indexVar);

	int var_type = knitro_var_type(domain);
	error = knitro::KN_set_var_type(m_kc.get(), indexVar, var_type);
	check_error(error);

	if (var_type == KN_VARTYPE_BINARY)
	{
		lb = (lb < 0.0) ? 0.0 : lb;
		ub = (ub > 1.0) ? 1.0 : ub;
	}

	error = knitro::KN_set_var_lobnd(m_kc.get(), indexVar, lb);
	check_error(error);
	error = knitro::KN_set_var_upbnd(m_kc.get(), indexVar, ub);
	check_error(error);

	if (!is_name_empty(name))
	{
		int error = knitro::KN_set_var_name(m_kc.get(), indexVar, name);
		check_error(error);
	}

	n_vars++;
	m_is_dirty = true;

	return variable;
}

double KnitroModel::get_variable_lb(const VariableIndex &variable)
{
	KNINT indexVar = _variable_index(variable);
	int error;
	double lb;

	error = knitro::KN_get_var_lobnd(m_kc.get(), indexVar, &lb);
	check_error(error);
	return lb;
}

double KnitroModel::get_variable_ub(const VariableIndex &variable)
{
	KNINT indexVar = _variable_index(variable);
	int error;
	double ub;

	error = knitro::KN_get_var_upbnd(m_kc.get(), indexVar, &ub);
	check_error(error);
	return ub;
}

void KnitroModel::set_variable_lb(const VariableIndex &variable, double lb)
{
	KNINT indexVar = _variable_index(variable);
	int error = knitro::KN_set_var_lobnd(m_kc.get(), indexVar, lb);
	check_error(error);
	m_is_dirty = true;
}

void KnitroModel::set_variable_ub(const VariableIndex &variable, double ub)
{
	KNLONG indexVar = _variable_index(variable);
	int error = knitro::KN_set_var_upbnd(m_kc.get(), indexVar, ub);
	check_error(error);
	m_is_dirty = true;
}

void KnitroModel::set_variable_bounds(const VariableIndex &variable, double lb, double ub)
{
	set_variable_lb(variable, lb);
	set_variable_ub(variable, ub);
}

double KnitroModel::get_variable_value(const VariableIndex &variable)
{
	if (!m_result.is_valid)
	{
		throw std::runtime_error("No solution available");
	}
	return m_result.x[_variable_index(variable)];
}

void KnitroModel::set_variable_start(const VariableIndex &variable, double start)
{
	KNINT indexVar = _variable_index(variable);
	int error = knitro::KN_set_var_primal_init_value(m_kc.get(), indexVar, start);
	check_error(error);
}

std::string KnitroModel::get_variable_name(const VariableIndex &variable)
{
	KNINT indexVar = _variable_index(variable);
	return _get_name(indexVar, knitro::KN_get_var_name, "x");
}

void KnitroModel::set_variable_name(const VariableIndex &variable, const std::string &name)
{
	KNINT indexVar = _variable_index(variable);
	_set_name(indexVar, name, knitro::KN_set_var_name);
}

void KnitroModel::set_variable_domain(const VariableIndex &variable, VariableDomain domain)
{
	KNINT indexVar = _variable_index(variable);
	int var_type = knitro_var_type(domain);
	int error;
	double lb = -KN_INFINITY;
	double ub = KN_INFINITY;

	if (var_type == KN_VARTYPE_BINARY)
	{
		error = knitro::KN_get_var_lobnd(m_kc.get(), indexVar, &lb);
		check_error(error);
		error = knitro::KN_get_var_upbnd(m_kc.get(), indexVar, &ub);
		check_error(error);
	}

	error = knitro::KN_set_var_type(m_kc.get(), indexVar, var_type);
	check_error(error);

	if (var_type == KN_VARTYPE_BINARY)
	{
		lb = (lb < 0.0) ? 0.0 : lb;
		ub = (ub > 1.0) ? 1.0 : ub;
		error = knitro::KN_set_var_lobnd(m_kc.get(), indexVar, lb);
		check_error(error);
		error = knitro::KN_set_var_upbnd(m_kc.get(), indexVar, ub);
		check_error(error);
	}

	m_is_dirty = true;
}

double KnitroModel::get_variable_rc(const VariableIndex &variable)
{
	if (!m_result.is_valid)
	{
		throw std::runtime_error("No solution available");
	}
	KNINT indexVar = _variable_index(variable);
	return m_result.lambda[indexVar];
}

void KnitroModel::delete_variable(const VariableIndex &variable)
{
	int indexVar = _variable_index(variable);
	int error;

	error = knitro::KN_set_var_type(m_kc.get(), indexVar, KN_VARTYPE_CONTINUOUS);
	check_error(error);
	error = knitro::KN_set_var_lobnd(m_kc.get(), indexVar, -KN_INFINITY);
	check_error(error);
	error = knitro::KN_set_var_upbnd(m_kc.get(), indexVar, KN_INFINITY);
	check_error(error);

	n_vars--;
	m_is_dirty = true;
}

std::string KnitroModel::pprint_variable(const VariableIndex &variable)
{
	return get_variable_name(variable);
}

KNINT KnitroModel::_variable_index(const VariableIndex &variable)
{
	return variable.index;
}

KNINT KnitroModel::_constraint_index(const ConstraintIndex &constraint)
{
	return constraint.index;
}

ConstraintIndex KnitroModel::add_linear_constraint(const ScalarAffineFunction &function,
                                                   ConstraintSense sense, double rhs,
                                                   const char *name)
{
	auto add = [this](const ScalarAffineFunction &f, const std::tuple<double, double> &interval,
	                  const char *n) { return add_linear_constraint(f, interval, n); };
	return _add_constraint_with_sense(function, sense, rhs, name, add);
}

ConstraintIndex KnitroModel::add_linear_constraint(const ScalarAffineFunction &function,
                                                   const std::tuple<double, double> &interval,
                                                   const char *name)
{
	auto setter = [this, &function](const ConstraintIndex &constraint) {
		_set_linear_constraint(constraint, function);
	};
	return _add_constraint_impl(ConstraintType::Linear, interval, name, &n_lincons, setter);
}

ConstraintIndex KnitroModel::add_quadratic_constraint(const ScalarQuadraticFunction &function,
                                                      ConstraintSense sense, double rhs,
                                                      const char *name)
{
	auto add = [this](const ScalarQuadraticFunction &f, const std::tuple<double, double> &interval,
	                  const char *n) { return add_quadratic_constraint(f, interval, n); };
	return _add_constraint_with_sense(function, sense, rhs, name, add);
}

ConstraintIndex KnitroModel::add_quadratic_constraint(const ScalarQuadraticFunction &function,
                                                      const std::tuple<double, double> &interval,
                                                      const char *name)
{
	auto setter = [this, &function](const ConstraintIndex &constraint) {
		_set_quadratic_constraint(constraint, function);
	};
	return _add_constraint_impl(ConstraintType::Quadratic, interval, name, &n_quadcons, setter);
}

ConstraintIndex KnitroModel::add_second_order_cone_constraint(
    const Vector<VariableIndex> &variables, const char *name, bool rotated)
{

	if (rotated)
	{
		auto setter = [this, &variables](const ConstraintIndex &constraint) {
			_set_second_order_cone_constraint_rotated(constraint, variables);
		};
		std::pair<double, double> interval = {0.0, KN_INFINITY};
		return _add_constraint_impl(ConstraintType::Cone, interval, name, nullptr, setter);
	}
	else
	{
		auto setter = [this, &variables](const ConstraintIndex &constraint) {
			_set_second_order_cone_constraint(constraint, variables);
		};
		std::pair<double, double> interval = {0.0, KN_INFINITY};
		return _add_constraint_impl(ConstraintType::Cone, interval, name, nullptr, setter);
	}
}

ConstraintIndex KnitroModel::add_single_nl_constraint(ExpressionGraph &graph,
														const ExpressionHandle &result,
														const std::tuple<double, double> &interval,
														const char *name)
{
	graph.add_constraint_output(result);
	auto setter = [this, &graph](const ConstraintIndex &constraint) {
		_set_nonlinear_constraint(constraint, graph);
	};

	return _add_constraint_impl(ConstraintType::KNITRO_NL, interval, name, nullptr, setter);
}

std::tuple<double, double> KnitroModel::_sense_to_interval(ConstraintSense sense, double rhs)
{
	switch (sense)
	{
	case ConstraintSense::LessEqual:
		return {-KN_INFINITY, rhs};
	case ConstraintSense::Equal:
		return {rhs, rhs};
	case ConstraintSense::GreaterEqual:
		return {rhs, KN_INFINITY};
	default:
		throw std::runtime_error("Unknown constraint sense");
	}
}

void KnitroModel::_update_con_sense_flags(const ConstraintIndex &constraint, ConstraintSense sense)
{
	KNINT indexCon = _constraint_index(constraint);
	switch (sense)
	{
	case ConstraintSense::Equal:
		m_con_sense_flags[indexCon] |= CON_LOBND;
		break;
	case ConstraintSense::GreaterEqual:
		m_con_sense_flags[indexCon] = CON_LOBND;
		break;
	default:
		break;
	}
}

void KnitroModel::_set_second_order_cone_constraint(const ConstraintIndex &constraint,
                                                    const Vector<VariableIndex> &variables)
{
	KNINT indexCon = _constraint_index(constraint);
	int error;

	KNINT indexCon0;
	error = knitro::KN_add_con(m_kc.get(), &indexCon0);
	check_error(error);

	KNINT indexVar0 = _variable_index(variables[0]);
	error = knitro::KN_add_con_linear_term(m_kc.get(), indexCon0, indexVar0, 1.0);
	check_error(error);

	error = knitro::KN_set_con_lobnd(m_kc.get(), indexCon0, 0.0);
	check_error(error);
	error = knitro::KN_set_con_upbnd(m_kc.get(), indexCon0, KN_INFINITY);
	check_error(error);

	error = knitro::KN_add_con_quadratic_term(m_kc.get(), indexCon, indexVar0, indexVar0, 1.0);
	check_error(error);
	size_t nnz = variables.size() - 1;
	std::vector<KNINT> indexVars(nnz);
	for (size_t i = 0; i < nnz; ++i)
	{
		indexVars[i] = _variable_index(variables[i + 1]);
	}
	std::vector<double> coefs(nnz, -1.0);
	error = knitro::KN_add_con_quadratic_struct_one(m_kc.get(), nnz, indexCon, indexVars.data(),
	                                                indexVars.data(), coefs.data());
	check_error(error);

	m_aux_cons[indexCon] = indexCon0;
}

void KnitroModel::_set_second_order_cone_constraint_rotated(const ConstraintIndex &constraint,
                                                            const Vector<VariableIndex> &variables)
{
	KNINT indexCon = _constraint_index(constraint);
	int error;

	KNINT indexCon0, indexCon1;
	error = knitro::KN_add_con(m_kc.get(), &indexCon0);
	check_error(error);
	error = knitro::KN_add_con(m_kc.get(), &indexCon1);
	check_error(error);

	KNINT indexVar0 = _variable_index(variables[0]);
	KNINT indexVar1 = _variable_index(variables[1]);
	error = knitro::KN_add_con_linear_term(m_kc.get(), indexCon0, indexVar0, 1.0);
	check_error(error);
	error = knitro::KN_add_con_linear_term(m_kc.get(), indexCon1, indexVar1, 1.0);
	check_error(error);

	error = knitro::KN_set_con_lobnd(m_kc.get(), indexCon0, 0.0);
	check_error(error);
	error = knitro::KN_set_con_upbnd(m_kc.get(), indexCon0, KN_INFINITY);
	check_error(error);
	error = knitro::KN_set_con_lobnd(m_kc.get(), indexCon1, 0.0);
	check_error(error);
	error = knitro::KN_set_con_upbnd(m_kc.get(), indexCon1, KN_INFINITY);
	check_error(error);

	size_t nnz = variables.size() - 2;
	std::vector<KNINT> indexVars(nnz);
	for (size_t i = 0; i < nnz; ++i)
	{
		indexVars[i] = _variable_index(variables[i + 2]);
	}
	std::vector<double> coefs(nnz, -1.0);
	error = knitro::KN_add_con_quadratic_struct_one(m_kc.get(), nnz, indexCon, indexVars.data(),
	                                                indexVars.data(), coefs.data());
	check_error(error);
	error = knitro::KN_add_con_quadratic_term(m_kc.get(), indexCon, indexVar0, indexVar1, 2.0);
	check_error(error);

	m_aux_cons[indexCon] = std::make_pair(indexCon0, indexCon1);
}

void KnitroModel::_set_nonlinear_constraint(const ConstraintIndex &constraint, ExpressionGraph &graph)
{
	KNINT indexCon = _constraint_index(constraint);
	int error;

	auto& cb_data_ptr = m_con_nl_data_map[indexCon];
	cb_data_ptr = std::make_unique<NLCallbackData>();
	NLCallbackData* cb_data = cb_data_ptr.get();

	cb_data->idx = graph.m_constraint_outputs.size() - 1;
	cb_data->function = cppad_trace_graph_constraints(graph);
	cb_data->function.optimize();

	size_t m = cb_data->function.Range();
	std::vector<std::set<size_t>> jac_sparsity(m);
	jac_sparsity[cb_data->idx].insert(cb_data->idx);
	cb_data->jac_pattern = cb_data->function.RevSparseJac(m, jac_sparsity);
	for (size_t j : cb_data->jac_pattern[cb_data->idx])
	{
		cb_data->jac_rows.push_back(cb_data->idx);
		cb_data->jac_cols.push_back(j);
	}

	cb_data->indexVars.resize(graph.n_variables());
	for (size_t i = 0; i < graph.m_variables.size(); i++)
	{
		cb_data->indexVars[i] = _variable_index(graph.m_variables[i]);
	}

	CB_context* cb = nullptr;
	auto cb_eval = [](KN_context*, CB_context*, KN_eval_request* req, KN_eval_result* res, void* data) -> int {
		NLCallbackData *cb_data = static_cast<NLCallbackData*>(data);
		CppAD::ADFun<double> &fun = cb_data->function;
		const std::vector<KNINT> &indexVars = cb_data->indexVars;

		std::vector<double> x(indexVars.size());
		for (size_t i = 0; i < indexVars.size(); i++)
		{
			x[i] = req->x[indexVars[i]];
		}
		auto y = fun.Forward(0, x);
		res->c[0] = y[cb_data->idx];
		return 0;
	};

	auto cb_grad = [](KN_context*, CB_context*, KN_eval_request* req, KN_eval_result* res, void* data) -> int {
		NLCallbackData *cb_data = static_cast<NLCallbackData*>(data);
		CppAD::ADFun<double> &fun = cb_data->function;
		const std::vector<KNINT> &indexVars = cb_data->indexVars;
		std::vector<double> x(indexVars.size());
		for (size_t i = 0; i < indexVars.size(); i++)
		{
			x[i] = req->x[indexVars[i]];
		}
		std::vector<double> jac(cb_data->jac_pattern[cb_data->idx].size());
		fun.SparseJacobianReverse(x, cb_data->jac_pattern, cb_data->jac_rows, cb_data->jac_cols, jac, cb_data->jac_work);
		for (size_t i = 0; i < jac.size(); i++)
		{
			res->jac[i] = jac[i];
		}
		return 0;
	};

	error = knitro::KN_add_eval_callback(m_kc.get(), FALSE, 1, &indexCon, cb_eval, &cb);
	check_error(error);

	error = knitro::KN_set_cb_user_params(m_kc.get(), cb, cb_data);
	check_error(error);

	std::vector<KNINT> jacIndexCons(cb_data->jac_rows.size(), indexCon);
	std::vector<KNINT> jacIndexVars(cb_data->jac_cols.size());
	for (size_t i = 0; i < cb_data->jac_cols.size(); i++)
	{
		jacIndexVars[i] = _variable_index(graph.m_variables[cb_data->jac_cols[i]]);
	}
	error = knitro::KN_set_cb_grad(m_kc.get(), cb, 0, NULL, jacIndexCons.size(),
	                                jacIndexCons.data(), jacIndexVars.data(), cb_grad);
	check_error(error);
}

void KnitroModel::delete_constraint(ConstraintIndex constraint)
{
	KNINT indexCon = _constraint_index(constraint);
	int error;

	error = knitro::KN_set_con_lobnd(m_kc.get(), indexCon, -KN_INFINITY);
	check_error(error);
	error = knitro::KN_set_con_upbnd(m_kc.get(), indexCon, KN_INFINITY);
	check_error(error);

	n_cons--;
	switch (constraint.type)
	{
	case ConstraintType::Linear:
		n_lincons--;
		break;
	case ConstraintType::Quadratic:
		n_quadcons--;
		break;
	default:
		break;
	}

	auto it = m_aux_cons.find(indexCon);
	if (it != m_aux_cons.end())
	{
		std::vector<KNINT> aux_cons;
		if (std::holds_alternative<KNINT>(it->second))
		{
			aux_cons.push_back(std::get<KNINT>(it->second));
		}
		else if (std::holds_alternative<std::pair<KNINT, KNINT>>(it->second))
		{
			auto p = std::get<std::pair<KNINT, KNINT>>(it->second);
			aux_cons.push_back(p.first);
			aux_cons.push_back(p.second);
		}

		for (auto const &con : aux_cons)
		{
			error = knitro::KN_set_con_lobnd(m_kc.get(), con, -KN_INFINITY);
			check_error(error);
			error = knitro::KN_set_con_upbnd(m_kc.get(), con, KN_INFINITY);
			check_error(error);
		}

		m_aux_cons.erase(it);
	}

	m_is_dirty = true;
}

void KnitroModel::set_constraint_name(const ConstraintIndex &constraint, const std::string &name)
{
	KNINT indexCon = _constraint_index(constraint);
	_set_name(indexCon, name, knitro::KN_set_con_name);
}

std::string KnitroModel::get_constraint_name(const ConstraintIndex &constraint)
{
	KNINT indexCon = _constraint_index(constraint);
	return _get_name(indexCon, knitro::KN_get_con_name, "c");
}

double KnitroModel::get_constraint_primal(const ConstraintIndex &constraint)
{
	if (!m_result.is_valid)
	{
		throw std::runtime_error("No solution available");
	}

	KNINT indexCon = _constraint_index(constraint);
	return m_result.con_values[indexCon];
}

double KnitroModel::get_constraint_dual(const ConstraintIndex &constraint)
{
	if (!m_result.is_valid)
	{
		throw std::runtime_error("No solution available");
	}

	KNINT indexCon = _constraint_index(constraint);
	return m_result.con_duals[indexCon];
}

void KnitroModel::set_normalized_rhs(const ConstraintIndex &constraint, double rhs)
{
	KNINT indexCon = _constraint_index(constraint);
	auto flag = m_con_sense_flags[indexCon];

	int error;

	if (flag & CON_LOBND)
	{
		error = knitro::KN_set_con_lobnd(m_kc.get(), indexCon, rhs);
		check_error(error);
	}

	if (flag & CON_UPBND)
	{
		error = knitro::KN_set_con_upbnd(m_kc.get(), indexCon, rhs);
		check_error(error);
	}

	m_is_dirty = true;
}

double KnitroModel::get_normalized_rhs(const ConstraintIndex &constraint)
{
	KNINT indexCon = _constraint_index(constraint);
	auto flag = m_con_sense_flags[indexCon];
	double rhs;
	int error;
	if (flag & CON_UPBND)
	{
		error = knitro::KN_get_con_upbnd(m_kc.get(), indexCon, &rhs);
		check_error(error);
	}
	else
	{
		error = knitro::KN_get_con_lobnd(m_kc.get(), indexCon, &rhs);
		check_error(error);
	}
	return rhs;
}

void KnitroModel::set_normalized_coefficient(const ConstraintIndex &constraint,
                                             const VariableIndex &variable, double coefficient)
{
	KNINT indexCon = _constraint_index(constraint);
	KNINT indexVar = _variable_index(variable);
	int error;

	error = knitro::KN_update(m_kc.get());
	check_error(error);
	error = knitro::KN_chg_con_linear_term(m_kc.get(), indexCon, indexVar, coefficient);
	check_error(error);
	m_is_dirty = true;
}

void KnitroModel::_set_linear_constraint(const ConstraintIndex &constraint,
                                         const ScalarAffineFunction &function)
{
	KNINT indexCon = _constraint_index(constraint);
	int error;

	if (function.constant.has_value())
	{
		double constant = function.constant.value();
		if (constant != 0.0)
		{
			error = knitro::KN_add_con_constant(m_kc.get(), indexCon, constant);
			check_error(error);
		}
	}

	AffineFunctionPtrForm<KNLONG, KNINT, double> ptr_form;
	ptr_form.make(this, function);

	KNLONG nnz = ptr_form.numnz;
	if (nnz > 0)
	{
		KNINT *indexVars = ptr_form.index;
		double *coefs = ptr_form.value;

		error = knitro::KN_add_con_linear_struct_one(m_kc.get(), nnz, indexCon, indexVars, coefs);
		check_error(error);
	}
}

void KnitroModel::_set_quadratic_constraint(const ConstraintIndex &constraint,
                                            const ScalarQuadraticFunction &function)
{
	KNINT indexCon = _constraint_index(constraint);
	int error;

	if (function.affine_part.has_value())
	{
		_set_linear_constraint(constraint, function.affine_part.value());
	}

	QuadraticFunctionPtrForm<KNLONG, KNINT, double> ptr_form;
	ptr_form.make(this, function);
	KNLONG nnz = ptr_form.numnz;
	if (nnz > 0)
	{
		KNINT *indexVars1 = ptr_form.row;
		KNINT *indexVars2 = ptr_form.col;
		double *coefs = ptr_form.value;
		error = knitro::KN_add_con_quadratic_struct_one(m_kc.get(), nnz, indexCon, indexVars1,
		                                                indexVars2, coefs);
		check_error(error);
	}
}

void KnitroModel::set_objective(const ScalarAffineFunction &function, ObjectiveSense sense)
{
	_set_objective_impl(sense, [this, &function]() { _set_linear_objective(function); });
}

void KnitroModel::set_objective(const ScalarQuadraticFunction &function, ObjectiveSense sense)
{
	_set_objective_impl(sense, [this, &function]() { _set_quadratic_objective(function); });
}

void KnitroModel::set_objective(const ExprBuilder &expr, ObjectiveSense sense)
{
	auto degree = expr.degree();
	if (degree <= 1)
	{
		ScalarAffineFunction linear(expr);
		set_objective(linear, sense);
	}
	else if (degree == 2)
	{
		ScalarQuadraticFunction quadratic(expr);
		set_objective(quadratic, sense);
	}
	else
	{
		throw std::runtime_error("Objective must be linear or quadratic");
	}
}

void KnitroModel::add_single_nl_objective(ExpressionGraph &graph, const ExpressionHandle &result)
{
	graph.add_objective_output(result);
	_add_nonlinear_objective(graph);
	m_is_dirty = true;
	m_result.is_valid = false;
}

void KnitroModel::_reset_objective()
{
	int error;

	if (m_obj_flag & OBJ_CONSTANT)
	{
		error = knitro::KN_del_obj_constant(m_kc.get());
		check_error(error);
	}
	if (m_obj_flag & OBJ_LINEAR)
	{
		error = knitro::KN_del_obj_linear_struct_all(m_kc.get());
		check_error(error);
	}
	if (m_obj_flag & OBJ_QUADRATIC)
	{
		error = knitro::KN_del_obj_quadratic_struct_all(m_kc.get());
		check_error(error);
	}
	if (m_obj_flag & OBJ_NONLINEAR)
	{
		error = knitro::KN_del_obj_eval_callback_all(m_kc.get());
		check_error(error);
	}

	m_obj_flag = 0;

	error = knitro::KN_update(m_kc.get());
	check_error(error);
}

void KnitroModel::_set_linear_objective(const ScalarAffineFunction &function)
{
	int error;

	if (function.constant.has_value())
	{
		double constant = function.constant.value();
		if (constant != 0.0)
		{
			error = knitro::KN_add_obj_constant(m_kc.get(), constant);
			check_error(error);
			m_obj_flag |= OBJ_CONSTANT;
		}
	}

	AffineFunctionPtrForm<KNLONG, KNINT, double> ptr_form;
	ptr_form.make(this, function);
	KNLONG nnz = ptr_form.numnz;
	if (nnz > 0)
	{
		KNINT *indexVars = ptr_form.index;
		double *coefs = ptr_form.value;
		error = knitro::KN_add_obj_linear_struct(m_kc.get(), nnz, indexVars, coefs);
		check_error(error);
		m_obj_flag |= OBJ_LINEAR;
	}
}

void KnitroModel::_set_quadratic_objective(const ScalarQuadraticFunction &function)
{
	int error;

	if (function.affine_part.has_value())
	{
		_set_linear_objective(function.affine_part.value());
	}

	QuadraticFunctionPtrForm<KNLONG, KNINT, double> ptr_form;
	ptr_form.make(this, function);
	KNLONG nnz = ptr_form.numnz;
	if (nnz > 0)
	{
		KNINT *indexVars1 = ptr_form.row;
		KNINT *indexVars2 = ptr_form.col;
		double *coefs = ptr_form.value;
		error = knitro::KN_add_obj_quadratic_struct(m_kc.get(), nnz, indexVars1, indexVars2, coefs);
		check_error(error);
		m_obj_flag |= OBJ_QUADRATIC;
	}
}

void KnitroModel::_add_nonlinear_objective(ExpressionGraph &graph)
{
	int error;
	auto cb_data_ptr = std::make_unique<NLCallbackData>();
	m_obj_nl_data.push_back(std::move(cb_data_ptr));
	NLCallbackData* cb_data = m_obj_nl_data.back().get();

	cb_data->idx = graph.m_objective_outputs.size() - 1;
	cb_data->function = cppad_trace_graph_objective(graph);
	cb_data->function.optimize();

	size_t m = cb_data->function.Range();
	std::vector<std::set<size_t>> jac_sparsity(m);
	jac_sparsity[cb_data->idx].insert(cb_data->idx);
	cb_data->jac_pattern = cb_data->function.RevSparseJac(m, jac_sparsity);
	for (size_t j : cb_data->jac_pattern[cb_data->idx])
	{
		cb_data->jac_rows.push_back(cb_data->idx);
		cb_data->jac_cols.push_back(j);
	}

	cb_data->indexVars.resize(graph.n_variables());
	for (size_t i = 0; i < graph.m_variables.size(); i++)
	{
		cb_data->indexVars[i] = _variable_index(graph.m_variables[i]);
	}

	CB_context* cb = nullptr;
	auto cb_eval = [](KN_context*, CB_context*, KN_eval_request* req, KN_eval_result* res, void* data) -> int {
		NLCallbackData *cb_data = static_cast<NLCallbackData*>(data);
		CppAD::ADFun<double> &fun = cb_data->function;
		const std::vector<KNINT> &indexVars = cb_data->indexVars;

		std::vector<double> x(indexVars.size());
		for (size_t i = 0; i < indexVars.size(); i++)
		{
			x[i] = req->x[indexVars[i]];
		}
		*res->obj = fun.Forward(0, x)[cb_data->idx];
		return 0;
	};

	auto cb_grad = [](KN_context*, CB_context*, KN_eval_request* req, KN_eval_result* res, void* data) -> int {
		NLCallbackData *cb_data = static_cast<NLCallbackData*>(data);
		CppAD::ADFun<double> &fun = cb_data->function;
		const std::vector<KNINT> &indexVars = cb_data->indexVars;
		std::vector<double> x(indexVars.size());
		for (size_t i = 0; i < indexVars.size(); i++)
		{
			x[i] = req->x[indexVars[i]];
		}
		std::vector<double> jac(cb_data->jac_pattern[cb_data->idx].size());
		fun.SparseJacobianReverse(x, cb_data->jac_pattern, cb_data->jac_rows, cb_data->jac_cols, jac, cb_data->jac_work);
		for (size_t i = 0; i < jac.size(); i++)
		{
			res->objGrad[i] = jac[i];
		}
		return 0;
	};

	error = knitro::KN_add_eval_callback(m_kc.get(), TRUE, 0, NULL, cb_eval, &cb);
	check_error(error);

	error = knitro::KN_set_cb_user_params(m_kc.get(), cb, cb_data);
	check_error(error);

	std::vector<KNINT> objGradIndexVars(cb_data->jac_cols.size());
	for (size_t i = 0; i < cb_data->jac_cols.size(); i++)
	{
		objGradIndexVars[i] = _variable_index(graph.m_variables[cb_data->jac_cols[i]]);
	}
	error = knitro::KN_set_cb_grad(m_kc.get(), cb, objGradIndexVars.size(), objGradIndexVars.data(),
	                                0, NULL, NULL, cb_grad);
	check_error(error);

	m_obj_flag |= OBJ_NONLINEAR;
}

double KnitroModel::get_obj_value()
{
	if (!m_result.is_valid)
	{
		throw std::runtime_error("No solution available");
	}
	return m_result.obj_val;
}

void KnitroModel::optimize()
{
	_optimize();
}

void KnitroModel::_optimize()
{
	int error = knitro::KN_solve(m_kc.get());
	// Note: KN_solve returns solve status, not an error code
	m_solve_status = error;

	// Get solution
	KNINT nV, nC;
	error = knitro::KN_get_number_vars(m_kc.get(), &nV);
	check_error(error);
	error = knitro::KN_get_number_cons(m_kc.get(), &nC);
	check_error(error);

	m_result.x.resize(nV);
	m_result.lambda.resize(nV);
	m_result.con_values.resize(nC);
	m_result.con_duals.resize(nC);

	error = knitro::KN_get_var_primal_values_all(m_kc.get(), m_result.x.data());
	check_error(error);

	error = knitro::KN_get_var_dual_values_all(m_kc.get(), m_result.lambda.data());
	check_error(error);
	for (size_t i = 0; i < nV; i++)
	{
		m_result.lambda[i] = -m_result.lambda[i];
	}

	if (nC > 0)
	{
		error = knitro::KN_get_con_values_all(m_kc.get(), m_result.con_values.data());
		check_error(error);

		error = knitro::KN_get_con_dual_values_all(m_kc.get(), m_result.con_duals.data());
		check_error(error);
		for (size_t i = 0; i < nC; i++)
		{
			m_result.con_duals[i] = -m_result.con_duals[i];
		}
	}

	error = knitro::KN_get_obj_value(m_kc.get(), &m_result.obj_val);
	check_error(error);

	m_result.status = m_solve_status;
	m_result.is_valid = true;
	m_is_dirty = false;
}
