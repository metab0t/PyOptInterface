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
	check_error(error);

	m_kc = std::unique_ptr<KN_context, KNITROFreeProblemT>(kc_ptr);
}

void KNITROModel::close()
{
	m_kc.reset();
}

void KNITROModel::check_error(int error) const
{
	if (error != 0)
	{
		throw std::runtime_error(fmt::format("KNITRO error code: {}", error));
	}
}

double KNITROModel::get_infinity() const
{
	return KN_INFINITY;
}

VariableIndex KNITROModel::add_variable(VariableDomain domain, double lb, double ub,
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

double KNITROModel::get_variable_lb(const VariableIndex &variable)
{
	KNINT indexVar = _variable_index(variable);
	int error;
	double lb;

	error = knitro::KN_get_var_lobnd(m_kc.get(), indexVar, &lb);
	check_error(error);
	return lb;
}

double KNITROModel::get_variable_ub(const VariableIndex &variable)
{
	KNINT indexVar = _variable_index(variable);
	int error;
	double ub;

	error = knitro::KN_get_var_upbnd(m_kc.get(), indexVar, &ub);
	check_error(error);
	return ub;
}

void KNITROModel::set_variable_lb(const VariableIndex &variable, double lb)
{
	KNINT indexVar = _variable_index(variable);
	int error = knitro::KN_set_var_lobnd(m_kc.get(), indexVar, lb);
	check_error(error);
	m_is_dirty = true;
}

void KNITROModel::set_variable_ub(const VariableIndex &variable, double ub)
{
	KNLONG indexVar = _variable_index(variable);
	int error = knitro::KN_set_var_upbnd(m_kc.get(), indexVar, ub);
	check_error(error);
	m_is_dirty = true;
}

void KNITROModel::set_variable_bounds(const VariableIndex &variable, double lb, double ub)
{
	set_variable_lb(variable, lb);
	set_variable_ub(variable, ub);
}

double KNITROModel::get_variable_value(const VariableIndex &variable)
{
	if (!m_result.is_valid)
	{
		throw std::runtime_error("No solution available");
	}
	return m_result.x[_variable_index(variable)];
}

void KNITROModel::set_variable_start(const VariableIndex &variable, double start)
{
	KNINT indexVar = _variable_index(variable);
	int error = knitro::KN_set_var_primal_init_value(m_kc.get(), indexVar, start);
	check_error(error);
}

std::string KNITROModel::get_variable_name(const VariableIndex &variable)
{
	KNINT indexVar = _variable_index(variable);
	return _get_name(indexVar, knitro::KN_get_var_name, "x");
}

void KNITROModel::set_variable_name(const VariableIndex &variable, const std::string &name)
{
	KNINT indexVar = _variable_index(variable);
	_set_name(indexVar, name, knitro::KN_set_var_name);
}

void KNITROModel::set_variable_domain(const VariableIndex &variable, VariableDomain domain)
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

double KNITROModel::get_variable_rc(const VariableIndex &variable)
{
	if (!m_result.is_valid)
	{
		throw std::runtime_error("No solution available");
	}
	KNINT indexVar = _variable_index(variable);
	return m_result.lambda[indexVar];
}

void KNITROModel::delete_variable(const VariableIndex &variable)
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

std::string KNITROModel::pprint_variable(const VariableIndex &variable)
{
	return get_variable_name(variable);
}

KNINT KNITROModel::_variable_index(const VariableIndex &variable)
{
	return variable.index;
}

KNINT KNITROModel::_constraint_index(const ConstraintIndex &constraint)
{
	return constraint.index;
}

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
		std::pair<double, double> interval = {0.0, KN_INFINITY};
		return _add_constraint_impl(ConstraintType::Cone, interval, name, &n_coniccons, setter);
	}
	else
	{
		auto setter = [this, &variables](const ConstraintIndex &constraint) {
			_set_second_order_cone_constraint(constraint, variables);
		};
		std::pair<double, double> interval = {0.0, KN_INFINITY};
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
	auto setter = [this, &graph](const ConstraintIndex &constraint) {
		size_t i = graph.m_constraint_outputs.size() - 1;
		m_graphs[&graph].m_cons[i] = constraint;
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
		return {-KN_INFINITY, rhs};
	case ConstraintSense::Equal:
		return {rhs, rhs};
	case ConstraintSense::GreaterEqual:
		return {rhs, KN_INFINITY};
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

void KNITROModel::_set_second_order_cone_constraint_rotated(const ConstraintIndex &constraint,
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

void KNITROModel::delete_constraint(ConstraintIndex constraint)
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
	case ConstraintType::Cone:
		n_coniccons--;
		break;
	case ConstraintType::KNITRO_NL:
		n_nlcons--;
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

void KNITROModel::set_constraint_name(const ConstraintIndex &constraint, const std::string &name)
{
	KNINT indexCon = _constraint_index(constraint);
	_set_name(indexCon, name, knitro::KN_set_con_name);
}

std::string KNITROModel::get_constraint_name(const ConstraintIndex &constraint)
{
	KNINT indexCon = _constraint_index(constraint);
	return _get_name(indexCon, knitro::KN_get_con_name, "c");
}

double KNITROModel::get_constraint_primal(const ConstraintIndex &constraint)
{
	if (!m_result.is_valid)
	{
		throw std::runtime_error("No solution available");
	}

	KNINT indexCon = _constraint_index(constraint);
	return m_result.con_values[indexCon];
}

double KNITROModel::get_constraint_dual(const ConstraintIndex &constraint)
{
	if (!m_result.is_valid)
	{
		throw std::runtime_error("No solution available");
	}

	KNINT indexCon = _constraint_index(constraint);
	return m_result.con_duals[indexCon];
}

void KNITROModel::set_normalized_rhs(const ConstraintIndex &constraint, double rhs)
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

double KNITROModel::get_normalized_rhs(const ConstraintIndex &constraint)
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

void KNITROModel::set_normalized_coefficient(const ConstraintIndex &constraint,
                                             const VariableIndex &variable, double coefficient)
{
	KNINT indexCon = _constraint_index(constraint);
	KNINT indexVar = _variable_index(variable);
	int error;

	// NOTE: To make sure the coefficient is updated correctly,
	// we need to call KN_update before changing the linear term
	_update();
	error = knitro::KN_chg_con_linear_term(m_kc.get(), indexCon, indexVar, coefficient);
	check_error(error);
	m_is_dirty = true;
}

void KNITROModel::_set_linear_constraint(const ConstraintIndex &constraint,
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

void KNITROModel::_set_quadratic_constraint(const ConstraintIndex &constraint,
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
	m_graphs[&graph].m_objs.push_back(i);
	m_need_to_add_callbacks = true;
	m_obj_flag |= OBJ_NONLINEAR;
	m_is_dirty = true;
	m_result.is_valid = false;
}

void KNITROModel::_add_graph(ExpressionGraph &graph)
{
	if (m_graphs.find(&graph) == m_graphs.end())
	{
		m_graphs[&graph] = KNITROGraph();
	}
}

void KNITROModel::_reset_objective()
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

	_update();
}

void KNITROModel::_set_linear_objective(const ScalarAffineFunction &function)
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

void KNITROModel::_set_quadratic_objective(const ScalarQuadraticFunction &function)
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

double KNITROModel::get_obj_value()
{
	if (!m_result.is_valid)
	{
		throw std::runtime_error("No solution available");
	}
	return m_result.obj_val;
}

void KNITROModel::_add_callbacks()
{
	if (!m_need_to_add_callbacks)
	{
		return;
	}

	for (const auto &[graph, pending] : m_graphs)
	{
		if (graph->has_constraint_output() && !pending.m_cons.empty())
		{
			auto f = cppad_trace_graph_constraints(*graph);
			f.optimize();

			size_t n = f.Domain();
			size_t m = pending.m_cons.size();
			auto load_ptr = std::make_unique<KNITROCallbackLoad>();
			KNITROCallbackLoad *load = load_ptr.get();
			load->fun = f;
			load->indexVars.resize(n);
			load->indexCons.resize(m);
			load->fun_rows.resize(m);
			for (size_t i = 0; i < n; i++)
			{
				load->indexVars[i] = _variable_index(graph->m_variables[i]);
			}
			auto it = pending.m_cons.begin();
			for (size_t k = 0; k < m; k++, it++)
			{
				auto &[j, constraint] = *it;
				load->fun_rows[k] = j;
				load->indexCons[k] = _constraint_index(constraint);
			}
			std::vector<std::set<size_t>> jac_sparsity(m);
			for (size_t k = 0; k < m; k++)
			{
				jac_sparsity[k].insert(load->fun_rows[k]);
			}
			load->jac_pattern = load->fun.RevSparseJac(jac_sparsity.size(), jac_sparsity);
			for (size_t k = 0; k < load->jac_pattern.size(); k++)
			{
				for (size_t i : load->jac_pattern[k])
				{
					load->jac_rows.push_back(load->fun_rows[k]);
					load->jac_cols.push_back(i);
				}
			}
			std::vector<KNINT> jacIndexCons(load->jac_rows.size());
			std::vector<KNINT> jacIndexVars(load->jac_cols.size());
			size_t idx = 0;
			for (size_t k = 0; k < load->jac_pattern.size(); k++)
			{
				for (size_t i : load->jac_pattern[k])
				{
					jacIndexCons[idx] = load->indexCons[k];
					jacIndexVars[idx] = load->indexVars[i];
					idx++;
				}
			}
			load->x.resize(n);
			load->jac.resize(load->jac_cols.size());
			auto cb_eval = [](KN_context *, CB_context *, KN_eval_request *req, KN_eval_result *res,
			                  void *data) -> int {
				KNITROCallbackLoad *load = static_cast<KNITROCallbackLoad *>(data);
				for (size_t i = 0; i < load->indexVars.size(); i++)
				{
					load->x[i] = req->x[load->indexVars[i]];
				}
				auto y = load->fun.Forward(0, load->x);
				for (size_t k = 0; k < load->fun_rows.size(); k++)
				{
					res->c[k] = y[load->fun_rows[k]];
				}
				return 0;
			};
			auto cb_grad = [](KN_context *, CB_context *, KN_eval_request *req, KN_eval_result *res,
			                  void *data) -> int {
				KNITROCallbackLoad *load = static_cast<KNITROCallbackLoad *>(data);
				for (size_t i = 0; i < load->indexVars.size(); i++)
				{
					load->x[i] = req->x[load->indexVars[i]];
				}
				load->fun.SparseJacobianReverse(load->x, load->jac_pattern, load->jac_rows,
				                                load->jac_cols, load->jac, load->jac_work);
				for (size_t i = 0; i < load->jac.size(); i++)
				{
					res->jac[i] = load->jac[i];
				}
				return 0;
			};
			CB_context *cb = nullptr;
			int error;
			error = knitro::KN_add_eval_callback(m_kc.get(), FALSE, load->indexCons.size(),
			                                     load->indexCons.data(), cb_eval, &cb);
			check_error(error);
			error = knitro::KN_set_cb_user_params(m_kc.get(), cb, load);
			check_error(error);
			error = knitro::KN_set_cb_grad(m_kc.get(), cb, 0, NULL, jacIndexCons.size(),
			                               jacIndexCons.data(), jacIndexVars.data(), cb_grad);
			check_error(error);
			m_loads.push_back(std::move(load_ptr));
		}

		if (graph->has_objective_output() && !pending.m_objs.empty())
		{
			auto f = cppad_trace_graph_objective(*graph);
			f.optimize();

			size_t n = f.Domain();
			size_t m = pending.m_objs.size();
			auto load_ptr = std::make_unique<KNITROCallbackLoad>();
			KNITROCallbackLoad *load = load_ptr.get();
			load->fun = f;
			load->indexVars.resize(n);
			load->indexCons.resize(0);
			load->fun_rows.resize(m);
			for (size_t i = 0; i < n; i++)
			{
				load->indexVars[i] = _variable_index(graph->m_variables[i]);
			}
			auto it = pending.m_objs.begin();
			for (size_t k = 0; k < m; k++, it++)
			{
				auto &j = *it;
				load->fun_rows[k] = j;
			}
			std::vector<std::set<size_t>> jac_sparsity(m);
			for (size_t k = 0; k < m; k++)
			{
				jac_sparsity[k].insert(load->fun_rows[k]);
			}
			load->jac_pattern = load->fun.RevSparseJac(jac_sparsity.size(), jac_sparsity);
			for (size_t k = 0; k < load->jac_pattern.size(); k++)
			{
				for (size_t i : load->jac_pattern[k])
				{
					load->jac_rows.push_back(load->fun_rows[k]);
					load->jac_cols.push_back(i);
				}
			}
			std::vector<KNINT> objGradIndexVars(load->jac_cols.size());
			size_t idx = 0;
			for (size_t k = 0; k < load->jac_pattern.size(); k++)
			{
				for (size_t i : load->jac_pattern[k])
				{
					objGradIndexVars[idx] = load->indexVars[i];
					idx++;
				}
			}
			load->x.resize(n);
			load->jac.resize(load->jac_cols.size());
			auto cb_eval = [](KN_context *, CB_context *, KN_eval_request *req, KN_eval_result *res,
			                  void *data) -> int {
				KNITROCallbackLoad *load = static_cast<KNITROCallbackLoad *>(data);
				for (size_t i = 0; i < load->indexVars.size(); i++)
				{
					load->x[i] = req->x[load->indexVars[i]];
				}
				auto y = load->fun.Forward(0, load->x);
				res->obj[0] = 0.0;
				for (size_t k = 0; k < load->fun_rows.size(); k++)
				{
					size_t j = load->fun_rows[k];
					res->obj[0] += y[j];
				}
				return 0;
			};
			auto cb_grad = [](KN_context *, CB_context *, KN_eval_request *req, KN_eval_result *res,
			                  void *data) -> int {
				KNITROCallbackLoad *load = static_cast<KNITROCallbackLoad *>(data);
				for (size_t i = 0; i < load->indexVars.size(); i++)
				{
					load->x[i] = req->x[load->indexVars[i]];
				}
				load->fun.SparseJacobianReverse(load->x, load->jac_pattern, load->jac_rows,
				                                load->jac_cols, load->jac, load->jac_work);
				for (size_t i = 0; i < load->jac.size(); i++)
				{
					res->objGrad[i] = load->jac[i];
				}
				return 0;
			};

			CB_context *cb = nullptr;
			int error;
			error = knitro::KN_add_eval_callback(m_kc.get(), TRUE, 0, NULL, cb_eval, &cb);
			check_error(error);
			error = knitro::KN_set_cb_user_params(m_kc.get(), cb, load);
			check_error(error);
			error = knitro::KN_set_cb_grad(m_kc.get(), cb, objGradIndexVars.size(),
			                              objGradIndexVars.data(), 0, NULL, NULL, cb_grad);
			check_error(error);
			m_loads.push_back(std::move(load_ptr));
		}
	}

	m_graphs.clear();
	m_need_to_add_callbacks = false;
}

void KNITROModel::_update()
{
	_add_callbacks();
	int error = knitro::KN_update(m_kc.get());
	check_error(error);
}

void KNITROModel::_pre_solve()
{
	_add_callbacks();
}

void KNITROModel::_solve()
{
	int error = knitro::KN_solve(m_kc.get());
	// NOTE: KN_solve returns solve status, not an error code
	m_solve_status = error;
}

void KNITROModel::_post_solve()
{
	int error;

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
}

void KNITROModel::optimize()
{
	_pre_solve();
	_solve();
	_post_solve();
	m_is_dirty = false;
}
