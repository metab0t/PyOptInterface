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

KNITROModel::KNITROModel()
{
	init();
}

void KNITROModel::init()
{
	if (!knitro::is_library_loaded())
	{
		throw std::runtime_error("KNITRO library is not loaded");
	}

	KN_context *kc_ptr = nullptr;
	int error = knitro::KN_new(&kc_ptr);
	_check_error(error);

	m_kc = std::unique_ptr<KN_context, KNITROFreeProblemT>(kc_ptr);
}

void KNITROModel::close()
{
	m_kc.reset();
}

void KNITROModel::_check_error(int error) const
{
	if (error != 0)
	{
		throw std::runtime_error(fmt::format("KNITRO error code: {}", error));
	}
}

// Model information
double KNITROModel::get_infinity() const
{
	return KN_INFINITY;
}

std::string KNITROModel::get_solver_name() const
{
	return std::string("KNITRO");
}

std::string KNITROModel::get_release() const
{
	constexpr int buf_size = 20;
	char release[buf_size];
	int error = knitro::KN_get_release(buf_size, release);
	_check_error(error);
	return std::string(release);
}

// Variable functions
VariableIndex KNITROModel::add_variable(VariableDomain domain, double lb, double ub,
                                        const char *name)
{
	KNINT indexVar;
	int error = knitro::KN_add_var(m_kc.get(), &indexVar);
	_check_error(error);

	VariableIndex variable(indexVar);

	int var_type = knitro_var_type(domain);
	_set_value<KNINT, int>(knitro::KN_set_var_type, indexVar, var_type);

	if (var_type == KN_VARTYPE_BINARY)
	{
		lb = (lb < 0.0) ? 0.0 : lb;
		ub = (ub > 1.0) ? 1.0 : ub;
	}

	_set_value<KNINT, double>(knitro::KN_set_var_lobnd, indexVar, lb);
	_set_value<KNINT, double>(knitro::KN_set_var_upbnd, indexVar, ub);

	if (!is_name_empty(name))
	{
		_set_name(knitro::KN_set_var_name, indexVar, name);
	}

	n_vars++;
	m_is_dirty = true;

	return variable;
}

double KNITROModel::get_variable_lb(const VariableIndex &variable) const
{
	KNINT indexVar = _variable_index(variable);
	return _get_value<KNINT, double>(knitro::KN_get_var_lobnd, indexVar);
}

double KNITROModel::get_variable_ub(const VariableIndex &variable) const
{
	KNINT indexVar = _variable_index(variable);
	return _get_value<KNINT, double>(knitro::KN_get_var_upbnd, indexVar);
}

void KNITROModel::set_variable_lb(const VariableIndex &variable, double lb)
{
	KNINT indexVar = _variable_index(variable);
	_set_value<KNINT, double>(knitro::KN_set_var_lobnd, indexVar, lb);
}

void KNITROModel::set_variable_ub(const VariableIndex &variable, double ub)
{
	KNINT indexVar = _variable_index(variable);
	_set_value<KNINT, double>(knitro::KN_set_var_upbnd, indexVar, ub);
}

void KNITROModel::set_variable_bounds(const VariableIndex &variable, double lb, double ub)
{
	set_variable_lb(variable, lb);
	set_variable_ub(variable, ub);
}

double KNITROModel::get_variable_value(const VariableIndex &variable) const
{
	_check_dirty();
	KNINT indexVar = _variable_index(variable);
	return _get_value<KNINT, double>(knitro::KN_get_var_primal_value, indexVar);
}

void KNITROModel::set_variable_start(const VariableIndex &variable, double start)
{
	KNINT indexVar = _variable_index(variable);
	_set_value<KNINT, double>(knitro::KN_set_var_primal_init_value, indexVar, start);
}

std::string KNITROModel::get_variable_name(const VariableIndex &variable) const
{
	KNINT indexVar = _variable_index(variable);
	return _get_name(knitro::KN_get_var_name, indexVar, "x");
}

void KNITROModel::set_variable_name(const VariableIndex &variable, const std::string &name)
{
	KNINT indexVar = _variable_index(variable);
	_set_name(knitro::KN_set_var_name, indexVar, name);
}

void KNITROModel::set_variable_domain(const VariableIndex &variable, VariableDomain domain)
{
	KNINT indexVar = _variable_index(variable);
	int var_type = knitro_var_type(domain);
	double lb = -get_infinity();
	double ub = get_infinity();

	if (var_type == KN_VARTYPE_BINARY)
	{
		lb = _get_value<KNINT, double>(knitro::KN_get_var_lobnd, indexVar);
		ub = _get_value<KNINT, double>(knitro::KN_get_var_upbnd, indexVar);
	}

	_set_value<KNINT, int>(knitro::KN_set_var_type, indexVar, var_type);

	if (var_type == KN_VARTYPE_BINARY)
	{
		lb = (lb < 0.0) ? 0.0 : lb;
		ub = (ub > 1.0) ? 1.0 : ub;
		_set_value<KNINT, double>(knitro::KN_set_var_lobnd, indexVar, lb);
		_set_value<KNINT, double>(knitro::KN_set_var_upbnd, indexVar, ub);
	}

	m_is_dirty = true;
}

double KNITROModel::get_variable_rc(const VariableIndex &variable) const
{
	_check_dirty();
	KNINT indexVar = _variable_index(variable);
	double dual = _get_value<KNINT, double>(knitro::KN_get_var_dual_value, indexVar);
	return -dual;
}

void KNITROModel::delete_variable(const VariableIndex &variable)
{
	KNINT indexVar = _variable_index(variable);
	_set_value<KNINT, int>(knitro::KN_set_var_type, indexVar, KN_VARTYPE_CONTINUOUS);
	_set_value<KNINT, double>(knitro::KN_set_var_lobnd, indexVar, -get_infinity());
	_set_value<KNINT, double>(knitro::KN_set_var_upbnd, indexVar, get_infinity());
	n_vars--;
	m_is_dirty = true;
}

std::string KNITROModel::pprint_variable(const VariableIndex &variable) const
{
	return get_variable_name(variable);
}

// Constraint functions
ConstraintIndex KNITROModel::add_linear_constraint(const ScalarAffineFunction &function,
                                                   ConstraintSense sense, double rhs,
                                                   const char *name)
{
	auto add = [this](const ScalarAffineFunction &f, const std::tuple<double, double> &interval,
	                  const char *n) { return add_linear_constraint(f, interval, n); };
	return _add_constraint_with_sense(function, sense, rhs, name, add);
}

ConstraintIndex KNITROModel::add_linear_constraint(const ScalarAffineFunction &function,
                                                   const std::tuple<double, double> &interval,
                                                   const char *name)
{
	auto setter = [this, &function](const ConstraintIndex &constraint) {
		_set_linear_constraint(constraint, function);
	};
	return _add_constraint_impl(ConstraintType::Linear, interval, name, &n_lincons, setter);
}

ConstraintIndex KNITROModel::add_quadratic_constraint(const ScalarQuadraticFunction &function,
                                                      ConstraintSense sense, double rhs,
                                                      const char *name)
{
	auto add = [this](const ScalarQuadraticFunction &f, const std::tuple<double, double> &interval,
	                  const char *n) { return add_quadratic_constraint(f, interval, n); };
	return _add_constraint_with_sense(function, sense, rhs, name, add);
}

ConstraintIndex KNITROModel::add_quadratic_constraint(const ScalarQuadraticFunction &function,
                                                      const std::tuple<double, double> &interval,
                                                      const char *name)
{
	auto setter = [this, &function](const ConstraintIndex &constraint) {
		_set_quadratic_constraint(constraint, function);
	};
	return _add_constraint_impl(ConstraintType::Quadratic, interval, name, &n_quadcons, setter);
}

ConstraintIndex KNITROModel::add_second_order_cone_constraint(
    const Vector<VariableIndex> &variables, const char *name, bool rotated)
{

	if (rotated)
	{
		auto setter = [this, &variables](const ConstraintIndex &constraint) {
			_set_second_order_cone_constraint_rotated(constraint, variables);
		};
		std::pair<double, double> interval = {0.0, get_infinity()};
		return _add_constraint_impl(ConstraintType::Cone, interval, name, &n_coniccons, setter);
	}
	else
	{
		auto setter = [this, &variables](const ConstraintIndex &constraint) {
			_set_second_order_cone_constraint(constraint, variables);
		};
		std::pair<double, double> interval = {0.0, get_infinity()};
		return _add_constraint_impl(ConstraintType::Cone, interval, name, &n_coniccons, setter);
	}
}

ConstraintIndex KNITROModel::add_single_nl_constraint(ExpressionGraph &graph,
                                                      const ExpressionHandle &result,
                                                      const std::tuple<double, double> &interval,
                                                      const char *name)
{
	_add_graph(graph);
	graph.add_constraint_output(result);
	size_t i = graph.m_constraint_outputs.size() - 1;
	m_pending_outputs[&graph].con_idxs.push_back(i);
	auto setter = [this, &graph](const ConstraintIndex &constraint) {
		m_pending_outputs[&graph].cons.push_back(constraint);
		m_need_to_add_callbacks = true;
	};
	return _add_constraint_impl(ConstraintType::KNITRO_NL, interval, name, &n_nlcons, setter);
}

ConstraintIndex KNITROModel::add_single_nl_constraint_sense_rhs(ExpressionGraph &graph,
                                                                const ExpressionHandle &result,
                                                                ConstraintSense sense, double rhs,
                                                                const char *name)
{
	auto add = [this, &graph, &result](void *, const std::tuple<double, double> &interval,
	                                   const char *n) {
		return add_single_nl_constraint(graph, result, interval, n);
	};
	return _add_constraint_with_sense(nullptr, sense, rhs, name, add);
}

std::tuple<double, double> KNITROModel::_sense_to_interval(ConstraintSense sense, double rhs)
{
	switch (sense)
	{
	case ConstraintSense::LessEqual:
		return {-get_infinity(), rhs};
	case ConstraintSense::Equal:
		return {rhs, rhs};
	case ConstraintSense::GreaterEqual:
		return {rhs, get_infinity()};
	default:
		throw std::runtime_error("Unknown constraint sense");
	}
}

void KNITROModel::_update_con_sense_flags(const ConstraintIndex &constraint, ConstraintSense sense)
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

void KNITROModel::_set_second_order_cone_constraint(const ConstraintIndex &constraint,
                                                    const Vector<VariableIndex> &variables)
{
	KNINT indexCon = _constraint_index(constraint);

	KNINT indexCon0;
	int error = knitro::KN_add_con(m_kc.get(), &indexCon0);
	_check_error(error);

	KNINT indexVar0 = _variable_index(variables[0]);
	error = knitro::KN_add_con_linear_term(m_kc.get(), indexCon0, indexVar0, 1.0);
	_check_error(error);

	_set_value<KNINT, double>(knitro::KN_set_con_lobnd, indexCon0, 0.0);
	_set_value<KNINT, double>(knitro::KN_set_con_upbnd, indexCon0, KN_INFINITY);

	error = knitro::KN_add_con_quadratic_term(m_kc.get(), indexCon, indexVar0, indexVar0, 1.0);
	_check_error(error);
	size_t nnz = variables.size() - 1;
	std::vector<KNINT> indexCons(nnz, indexCon);
	std::vector<KNINT> indexVars(nnz);
	for (size_t i = 0; i < nnz; ++i)
	{
		indexVars[i] = _variable_index(variables[i + 1]);
	}
	std::vector<double> coefs(nnz, -1.0);
	error = knitro::KN_add_con_quadratic_struct(m_kc.get(), nnz, indexCons.data(), indexVars.data(),
	                                            indexVars.data(), coefs.data());
	_check_error(error);

	m_soc_aux_cons[indexCon] = indexCon0;
}

void KNITROModel::_set_second_order_cone_constraint_rotated(const ConstraintIndex &constraint,
                                                            const Vector<VariableIndex> &variables)
{
	KNINT indexCon = _constraint_index(constraint);

	KNINT indexCon0, indexCon1;
	int error = knitro::KN_add_con(m_kc.get(), &indexCon0);
	_check_error(error);
	error = knitro::KN_add_con(m_kc.get(), &indexCon1);
	_check_error(error);

	KNINT indexVar0 = _variable_index(variables[0]);
	KNINT indexVar1 = _variable_index(variables[1]);
	error = knitro::KN_add_con_linear_term(m_kc.get(), indexCon0, indexVar0, 1.0);
	_check_error(error);
	error = knitro::KN_add_con_linear_term(m_kc.get(), indexCon1, indexVar1, 1.0);
	_check_error(error);

	_set_value<KNINT, double>(knitro::KN_set_con_lobnd, indexCon0, 0.0);
	_set_value<KNINT, double>(knitro::KN_set_con_upbnd, indexCon0, KN_INFINITY);
	_set_value<KNINT, double>(knitro::KN_set_con_lobnd, indexCon1, 0.0);
	_set_value<KNINT, double>(knitro::KN_set_con_upbnd, indexCon1, KN_INFINITY);

	size_t nnz = variables.size() - 2;
	std::vector<KNINT> indexCons(nnz, indexCon);
	std::vector<KNINT> indexVars(nnz);
	for (size_t i = 0; i < nnz; ++i)
	{
		indexVars[i] = _variable_index(variables[i + 2]);
	}
	std::vector<double> coefs(nnz, -1.0);
	error = knitro::KN_add_con_quadratic_struct(m_kc.get(), nnz, indexCons.data(), indexVars.data(),
	                                            indexVars.data(), coefs.data());
	_check_error(error);
	error = knitro::KN_add_con_quadratic_term(m_kc.get(), indexCon, indexVar0, indexVar1, 2.0);
	_check_error(error);

	m_soc_aux_cons[indexCon] = std::make_pair(indexCon0, indexCon1);
}

void KNITROModel::delete_constraint(const ConstraintIndex &constraint)
{
	KNINT indexCon = _constraint_index(constraint);
	_set_value<KNINT, double>(knitro::KN_set_con_lobnd, indexCon, -get_infinity());
	_set_value<KNINT, double>(knitro::KN_set_con_upbnd, indexCon, get_infinity());

	n_cons--;
	switch (constraint.type)
	{
	case ConstraintType::Linear:
		n_lincons--;
		break;
	case ConstraintType::Quadratic:
		n_quadcons--;
		break;
	case ConstraintType::Cone:
		n_coniccons--;
		break;
	case ConstraintType::KNITRO_NL:
		n_nlcons--;
		break;
	default:
		break;
	}

	auto it = m_soc_aux_cons.find(indexCon);
	if (it != m_soc_aux_cons.end())
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
			_set_value<KNINT, double>(knitro::KN_set_con_lobnd, con, -get_infinity());
			_set_value<KNINT, double>(knitro::KN_set_con_upbnd, con, get_infinity());
		}

		m_soc_aux_cons.erase(it);
	}

	m_is_dirty = true;
}

void KNITROModel::set_constraint_name(const ConstraintIndex &constraint, const std::string &name)
{
	KNINT indexCon = _constraint_index(constraint);
	_set_name(knitro::KN_set_con_name, indexCon, name);
}

std::string KNITROModel::get_constraint_name(const ConstraintIndex &constraint) const
{
	KNINT indexCon = _constraint_index(constraint);
	return _get_name(knitro::KN_get_con_name, indexCon, "c");
}

double KNITROModel::get_constraint_primal(const ConstraintIndex &constraint) const
{
	_check_dirty();
	KNINT indexCon = _constraint_index(constraint);
	return _get_value<KNINT, double>(knitro::KN_get_con_value, indexCon);
}

double KNITROModel::get_constraint_dual(const ConstraintIndex &constraint) const
{
	_check_dirty();
	KNINT indexCon = _constraint_index(constraint);
	double dual = _get_value<KNINT, double>(knitro::KN_get_con_dual_value, indexCon);
	return -dual;
}

void KNITROModel::set_normalized_rhs(const ConstraintIndex &constraint, double rhs)
{
	KNINT indexCon = _constraint_index(constraint);
	auto flag = m_con_sense_flags[indexCon];

	if (flag & CON_LOBND)
	{
		_set_value<KNINT, double>(knitro::KN_set_con_lobnd, indexCon, rhs);
	}

	if (flag & CON_UPBND)
	{
		_set_value<KNINT, double>(knitro::KN_set_con_upbnd, indexCon, rhs);
	}

	m_is_dirty = true;
}

double KNITROModel::get_normalized_rhs(const ConstraintIndex &constraint) const
{
	KNINT indexCon = _constraint_index(constraint);
	auto it = m_con_sense_flags.find(indexCon);
	uint8_t flag = (it != m_con_sense_flags.end()) ? it->second : CON_UPBND;
	double rhs;
	if (flag & CON_UPBND)
	{
		rhs = _get_value<KNINT, double>(knitro::KN_get_con_upbnd, indexCon);
	}
	else
	{
		rhs = _get_value<KNINT, double>(knitro::KN_get_con_lobnd, indexCon);
	}
	return rhs;
}

void KNITROModel::set_normalized_coefficient(const ConstraintIndex &constraint,
                                             const VariableIndex &variable, double coefficient)
{
	KNINT indexCon = _constraint_index(constraint);
	KNINT indexVar = _variable_index(variable);

	// NOTE: To make sure the coefficient is updated correctly,
	// we need to call KN_update before changing the linear term
	_update();
	int error = knitro::KN_chg_con_linear_term(m_kc.get(), indexCon, indexVar, coefficient);
	_check_error(error);
	m_is_dirty = true;
}

void KNITROModel::_set_linear_constraint(const ConstraintIndex &constraint,
                                         const ScalarAffineFunction &function)
{
	KNINT indexCon = _constraint_index(constraint);

	if (function.constant.has_value())
	{
		double constant = function.constant.value();
		if (constant != 0.0)
		{
			int error = knitro::KN_add_con_constant(m_kc.get(), indexCon, constant);
			_check_error(error);
		}
	}

	AffineFunctionPtrForm<KNLONG, KNINT, double> ptr_form;
	ptr_form.make(this, function);

	KNLONG nnz = ptr_form.numnz;
	if (nnz > 0)
	{
		std::vector<KNINT> indexCons(nnz, indexCon);
		KNINT *indexVars = ptr_form.index;
		double *coefs = ptr_form.value;

		int error =
		    knitro::KN_add_con_linear_struct(m_kc.get(), nnz, indexCons.data(), indexVars, coefs);
		_check_error(error);
	}
}

void KNITROModel::_set_quadratic_constraint(const ConstraintIndex &constraint,
                                            const ScalarQuadraticFunction &function)
{
	KNINT indexCon = _constraint_index(constraint);

	if (function.affine_part.has_value())
	{
		_set_linear_constraint(constraint, function.affine_part.value());
	}

	QuadraticFunctionPtrForm<KNLONG, KNINT, double> ptr_form;
	ptr_form.make(this, function);
	KNLONG nnz = ptr_form.numnz;
	if (nnz > 0)
	{
		std::vector<KNINT> indexCons(nnz, indexCon);
		KNINT *indexVars1 = ptr_form.row;
		KNINT *indexVars2 = ptr_form.col;
		double *coefs = ptr_form.value;
		int error = knitro::KN_add_con_quadratic_struct(m_kc.get(), nnz, indexCons.data(),
		                                                indexVars1, indexVars2, coefs);
		_check_error(error);
	}
}

// Objective functions
void KNITROModel::set_objective(const ScalarAffineFunction &function, ObjectiveSense sense)
{
	_set_objective_impl(sense, [this, &function]() { _set_linear_objective(function); });
}

void KNITROModel::set_objective(const ScalarQuadraticFunction &function, ObjectiveSense sense)
{
	_set_objective_impl(sense, [this, &function]() { _set_quadratic_objective(function); });
}

void KNITROModel::set_objective(const ExprBuilder &expr, ObjectiveSense sense)
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

void KNITROModel::add_single_nl_objective(ExpressionGraph &graph, const ExpressionHandle &result)
{
	_add_graph(graph);
	graph.add_objective_output(result);
	size_t i = graph.m_objective_outputs.size() - 1;
	m_pending_outputs[&graph].obj_idxs.push_back(i);
	m_need_to_add_callbacks = true;
	m_obj_flag |= OBJ_NONLINEAR;
	m_is_dirty = true;
}

void KNITROModel::set_obj_sense(ObjectiveSense sense)
{
	int goal = knitro_obj_goal(sense);
	_set_value<int>(knitro::KN_set_obj_goal, goal);
}

ObjectiveSense KNITROModel::get_obj_sense() const
{
	int goal = _get_value<int>(knitro::KN_get_obj_goal);
	return knitro_obj_sense(goal);
}

void KNITROModel::_add_graph(ExpressionGraph &graph)
{
	if (m_pending_outputs.find(&graph) == m_pending_outputs.end())
	{
		m_pending_outputs[&graph] = Outputs();
	}
}

void KNITROModel::_reset_objective()
{
	if (m_obj_flag & OBJ_CONSTANT)
	{
		int error = knitro::KN_del_obj_constant(m_kc.get());
		_check_error(error);
	}
	if (m_obj_flag & OBJ_LINEAR)
	{
		int error = knitro::KN_del_obj_linear_struct_all(m_kc.get());
		_check_error(error);
	}
	if (m_obj_flag & OBJ_QUADRATIC)
	{
		int error = knitro::KN_del_obj_quadratic_struct_all(m_kc.get());
		_check_error(error);
	}
	if (m_obj_flag & OBJ_NONLINEAR)
	{
		int error = knitro::KN_del_obj_eval_callback_all(m_kc.get());
		_check_error(error);
		for (auto &[graph, outputs] : m_pending_outputs)
		{
			outputs.obj_idxs.clear();
		}
	}
	m_obj_flag = 0;
	_update();
}

void KNITROModel::_set_linear_objective(const ScalarAffineFunction &function)
{
	if (function.constant.has_value())
	{
		double constant = function.constant.value();
		if (constant != 0.0)
		{
			int error = knitro::KN_add_obj_constant(m_kc.get(), constant);
			_check_error(error);
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
		int error = knitro::KN_add_obj_linear_struct(m_kc.get(), nnz, indexVars, coefs);
		_check_error(error);
		m_obj_flag |= OBJ_LINEAR;
	}
}

void KNITROModel::_set_quadratic_objective(const ScalarQuadraticFunction &function)
{
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
		int error =
		    knitro::KN_add_obj_quadratic_struct(m_kc.get(), nnz, indexVars1, indexVars2, coefs);
		_check_error(error);
		m_obj_flag |= OBJ_QUADRATIC;
	}
}

double KNITROModel::get_obj_value() const
{
	_check_dirty();
	return _get_value<double>(knitro::KN_get_obj_value);
}

void KNITROModel::_add_constraint_callback(ExpressionGraph *graph, const Outputs &outputs)
{
	auto f = [](KN_context *, CB_context *, KN_eval_request *req, KN_eval_result *res,
	            void *data) -> int {
		auto *evaluator = static_cast<CallbackEvaluator<double> *>(data);
		evaluator->eval_fun(req->x, res->c);
		return 0;
	};
	auto g = [](KN_context *, CB_context *, KN_eval_request *req, KN_eval_result *res,
	            void *data) -> int {
		auto *evaluator = static_cast<CallbackEvaluator<double> *>(data);
		evaluator->eval_jac(req->x, res->jac);
		return 0;
	};
	auto h = [](KN_context *, CB_context *, KN_eval_request *req, KN_eval_result *res,
	            void *data) -> int {
		auto *evaluator = static_cast<CallbackEvaluator<double> *>(data);
		evaluator->eval_hess(req->x, req->lambda, res->hess);
		return 0;
	};
	auto trace = cppad_trace_graph_constraints;
	_add_callback_impl(*graph, outputs.con_idxs, outputs.cons, trace, f, g, h);
}

void KNITROModel::_add_objective_callback(ExpressionGraph *graph, const Outputs &outputs)
{
	auto f = [](KN_context *, CB_context *, KN_eval_request *req, KN_eval_result *res,
	            void *data) -> int {
		auto *evaluator = static_cast<CallbackEvaluator<double> *>(data);
		res->obj[0] = 0.0;
		evaluator->eval_fun(req->x, res->obj, true);
		return 0;
	};
	auto g = [](KN_context *, CB_context *, KN_eval_request *req, KN_eval_result *res,
	            void *data) -> int {
		auto *evaluator = static_cast<CallbackEvaluator<double> *>(data);
		evaluator->eval_jac(req->x, res->objGrad);
		return 0;
	};
	auto h = [](KN_context *, CB_context *, KN_eval_request *req, KN_eval_result *res,
	            void *data) -> int {
		auto *evaluator = static_cast<CallbackEvaluator<double> *>(data);
		evaluator->eval_hess(req->x, req->sigma, res->hess, true);
		return 0;
	};
	auto trace = cppad_trace_graph_objective;
	_add_callback_impl(*graph, outputs.obj_idxs, {}, trace, f, g, h);
}

void KNITROModel::_add_callbacks()
{
	if (!m_need_to_add_callbacks)
	{
		return;
	}

	for (const auto &[graph, outputs] : m_pending_outputs)
	{
		if (graph->has_constraint_output() && !outputs.con_idxs.empty())
		{
			_add_constraint_callback(graph, outputs);
		}

		if (graph->has_objective_output() && !outputs.obj_idxs.empty())
		{
			_add_objective_callback(graph, outputs);
		}
	}
	m_pending_outputs.clear();
	m_need_to_add_callbacks = false;
}

// Solve functions
void KNITROModel::_update()
{
	_add_callbacks();
	int error = knitro::KN_update(m_kc.get());
	_check_error(error);
}

void KNITROModel::_pre_solve()
{
	_add_callbacks();
}

void KNITROModel::_solve()
{
	m_solve_status = knitro::KN_solve(m_kc.get());
}

void KNITROModel::_post_solve()
{
}

void KNITROModel::optimize()
{
	_pre_solve();
	_solve();
	_post_solve();
	m_is_dirty = false;
}

// Solve information
size_t KNITROModel::get_number_iterations() const
{
	_check_dirty();
	int iters = _get_value<int>(knitro::KN_get_number_iters);
	return static_cast<size_t>(iters);
}

size_t KNITROModel::get_mip_node_count() const
{
	_check_dirty();
	int nodes = _get_value<int>(knitro::KN_get_mip_number_nodes);
	return static_cast<size_t>(nodes);
}

double KNITROModel::get_obj_bound() const
{
	_check_dirty();
	return _get_value<double>(knitro::KN_get_mip_relaxation_bnd);
}

double KNITROModel::get_mip_relative_gap() const
{
	_check_dirty();
	return _get_value<double>(knitro::KN_get_mip_rel_gap);
}

double KNITROModel::get_solve_time() const
{
	_check_dirty();
	return _get_value<double>(knitro::KN_get_solve_time_real);
}

// Internal helpers
void KNITROModel::_check_dirty() const
{
	if (m_is_dirty)
	{
		throw std::runtime_error("Model has been modified since last solve. Call optimize()...");
	}
}

KNINT KNITROModel::_variable_index(const VariableIndex &variable) const
{
	return _get_index(variable);
}

KNINT KNITROModel::_constraint_index(const ConstraintIndex &constraint) const
{
	return _get_index(constraint);
}
