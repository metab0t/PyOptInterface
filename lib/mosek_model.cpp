#include "pyoptinterface/solver_common.hpp"
#include "pyoptinterface/mosek_model.hpp"
#include "fmt/core.h"

// specialize the template for MOSEK
template <>
template <VarIndexModel T>
void QuadraticFunctionPtrForm<MSKint32t, MSKint32t, MSKrealt>::make(
    T *model, const ScalarQuadraticFunction &function)
{
	auto f_numnz = function.size();
	numnz = f_numnz;
	row_storage.resize(numnz);
	col_storage.resize(numnz);
	for (int i = 0; i < numnz; ++i)
	{
		auto v1 = model->_variable_index(function.variable_1s[i]);
		auto v2 = v1;
		if (function.variable_1s[i] != function.variable_2s[i])
		{
			v2 = model->_variable_index(function.variable_2s[i]);
			// MOSEK only accepts the lower triangle (i >= j)
			if (v1 < v2)
			{
				std::swap(v1, v2);
			}
		}

		row_storage[i] = v1;
		col_storage[i] = v2;
	}
	row = row_storage.data();
	col = col_storage.data();

	// MOSEK has 1/2 * x^T @ Q @ x, so we need to multiply the coefficient by 2
	value_storage.resize(numnz);
	for (int i = 0; i < numnz; ++i)
	{
		value_storage[i] = 2.0 * function.coefficients[i];
	}
	value = value_storage.data();
}

static void check_error(MSKrescodee error)
{
	if (error != MSK_RES_OK)
	{
		char symname[MSK_MAX_STR_LEN];
		char desc[MSK_MAX_STR_LEN];
		MSK_getcodedesc(error, symname, desc);

		std::string errmsg = fmt::format("Error {} : {}", symname, desc);

		throw std::runtime_error(errmsg);
	}
}

static MSKboundkeye mosek_con_sense(ConstraintSense sense)
{
	using enum ConstraintSense;
	switch (sense)
	{
	case LessEqual:
		return MSK_BK_UP;
	case Equal:
		return MSK_BK_FX;
	case GreaterEqual:
		return MSK_BK_LO;
	default:
		throw std::runtime_error("Unknown constraint sense");
	}
}

static MSKobjsensee mosek_obj_sense(ObjectiveSense sense)
{
	using enum ObjectiveSense;
	switch (sense)
	{
	case Minimize:
		return MSK_OBJECTIVE_SENSE_MINIMIZE;
	case Maximize:
		return MSK_OBJECTIVE_SENSE_MAXIMIZE;
	default:
		throw std::runtime_error("Unknown objective sense");
	}
}

static MSKvariabletypee mosek_vtype(VariableDomain domain)
{
	using enum VariableDomain;
	switch (domain)
	{
	case Continuous:
		return MSK_VAR_TYPE_CONT;
	case Integer:
	case Binary:
		return MSK_VAR_TYPE_INT;
	default:
		throw std::runtime_error("Unknown variable domain");
	}
}

static VariableDomain mosek_vtype_to_domain(MSKvariabletypee vtype)
{
	using enum VariableDomain;
	switch (vtype)
	{
	case MSK_VAR_TYPE_CONT:
		return Continuous;
	case MSK_VAR_TYPE_INT:
		return Integer;
	default:
		throw std::runtime_error("Unknown variable domain");
	}
}

MOSEKModel::MOSEKModel(const MOSEKEnv &env)
{
	init(env);
}

void MOSEKModel::init(const MOSEKEnv &env)
{
	MSKtask_t model;
	auto error = MSK_makeemptytask(env.m_env, &model);
	check_error(error);
	m_model = std::unique_ptr<MSKtask, MOSEKfreemodelT>(model);
}

VariableIndex MOSEKModel::add_variable(VariableDomain domain, double lb, double ub,
                                       const char *name)
{
	if (name != nullptr && name[0] == '\0')
	{
		name = nullptr;
	}
	IndexT index = m_variable_index.add_index();
	VariableIndex variable(index);

	auto error = MSK_appendvars(m_model.get(), 1);
	check_error(error);

	MSKint32t column;
	error = MSK_getnumvar(m_model.get(), &column);
	check_error(error);
	// 0-based indexing
	column -= 1;

	auto vtype = mosek_vtype(domain);
	error = MSK_putvartype(m_model.get(), column, vtype);
	check_error(error);

	MSKboundkeye bk;
	if (domain == VariableDomain::Binary)
	{
		bk = MSK_BK_RA;
		lb = 0.0;
		ub = 1.0;
		binary_variables.insert(index);
	}
	else
	{
		bool lb_inf = lb < 1.0 - MSK_INFINITY;
		bool ub_inf = ub > MSK_INFINITY - 1.0;
		if (lb_inf && ub_inf)
			bk = MSK_BK_FR;
		else if (lb_inf)
			bk = MSK_BK_UP;
		else if (ub_inf)
			bk = MSK_BK_LO;
		else
			bk = MSK_BK_RA;
	}
	error = MSK_putvarbound(m_model.get(), column, bk, lb, ub);
	check_error(error);

	if (name)
	{
		error = MSK_putvarname(m_model.get(), column, name);
		check_error(error);
	}

	return variable;
}

VariableIndex MOSEKModel::add_variables(int N, VariableDomain domain, double lb, double ub)
{
	IndexT index = m_variable_index.add_indices(N);
	VariableIndex variable(index);

	auto error = MSK_appendvars(m_model.get(), N);
	check_error(error);

	MSKint32t column;
	error = MSK_getnumvar(m_model.get(), &column);
	check_error(error);
	// 0-based indexing
	column -= N;
	std::vector<MSKint32t> columns(N);
	for (int i = 0; i < N; i++)
	{
		columns[i] = column + i;
	}

	MSKvariabletypee vtype = mosek_vtype(domain);
	std::vector<MSKvariabletypee> vtypes(N, vtype);
	error = MSK_putvartypelist(m_model.get(), N, columns.data(), vtypes.data());
	check_error(error);

	MSKboundkeye bk;
	if (domain == VariableDomain::Binary)
	{
		bk = MSK_BK_RA;
		lb = 0.0;
		ub = 1.0;
		for (int i = 0; i < N; i++)
		{
			binary_variables.insert(index + i);
		}
	}
	else
	{
		bool lb_inf = lb < 1.0 - MSK_INFINITY;
		bool ub_inf = ub > MSK_INFINITY - 1.0;
		if (lb_inf && ub_inf)
			bk = MSK_BK_FR;
		else if (lb_inf)
			bk = MSK_BK_UP;
		else if (ub_inf)
			bk = MSK_BK_LO;
		else
			bk = MSK_BK_RA;
	}
	std::vector<MSKboundkeye> bks(N, bk);
	std::vector<MSKrealt> lbs(N, lb);
	std::vector<MSKrealt> ubs(N, ub);
	error =
	    MSK_putvarboundlist(m_model.get(), N, columns.data(), bks.data(), lbs.data(), ubs.data());
	check_error(error);

	return variable;
}

void MOSEKModel::delete_variable(const VariableIndex &variable)
{
	if (!is_variable_active(variable))
	{
		throw std::runtime_error("Variable does not exist");
	}

	int variable_column = _variable_index(variable);
	auto error = MSK_removevars(m_model.get(), 1, &variable_column);
	check_error(error);

	m_variable_index.delete_index(variable.index);
}

bool MOSEKModel::is_variable_active(const VariableIndex &variable)
{
	return m_variable_index.has_index(variable.index);
}

double MOSEKModel::get_variable_value(const VariableIndex &variable)
{
	auto column = _checked_variable_index(variable);
	MSKrealt retval;
	auto soltype = get_current_solution();
	auto error = MSK_getxxslice(m_model.get(), soltype, column, column + 1, &retval);
	check_error(error);
	return retval;
}

std::string MOSEKModel::pprint_variable(const VariableIndex &variable)
{
	return get_variable_name(variable);
}

ConstraintIndex MOSEKModel::add_linear_constraint(const ScalarAffineFunction &function,
                                                  ConstraintSense sense, CoeffT rhs)
{
	IndexT index = m_linear_quadratic_constraint_index.add_index();
	ConstraintIndex constraint_index(ConstraintType::Linear, index);

	auto error = MSK_appendcons(m_model.get(), 1);
	check_error(error);

	MSKint32t row;
	error = MSK_getnumcon(m_model.get(), &row);
	check_error(error);
	// 0-based indexing
	row -= 1;

	AffineFunctionPtrForm<MSKint32t, MSKint32t, MSKrealt> ptr_form;
	ptr_form.make(this, function);

	MSKint32t numnz = ptr_form.numnz;
	MSKint32t *cind = ptr_form.index;
	MSKrealt *cval = ptr_form.value;
	MSKboundkeye g_sense = mosek_con_sense(sense);
	MSKrealt g_rhs = rhs - function.constant.value_or(0.0);

	error = MSK_putarow(m_model.get(), row, numnz, cind, cval);
	check_error(error);
	error = MSK_putconbound(m_model.get(), row, g_sense, g_rhs, g_rhs);
	check_error(error);

	return constraint_index;
}

ConstraintIndex MOSEKModel::add_quadratic_constraint(const ScalarQuadraticFunction &function,
                                                     ConstraintSense sense, CoeffT rhs)
{
	IndexT index = m_linear_quadratic_constraint_index.add_index();
	ConstraintIndex constraint_index(ConstraintType::Quadratic, index);

	auto error = MSK_appendcons(m_model.get(), 1);
	check_error(error);

	MSKint32t row;
	error = MSK_getnumcon(m_model.get(), &row);
	check_error(error);
	// 0-based indexing
	row -= 1;

	const auto &affine_part = function.affine_part;

	MSKint32t numlnz = 0;
	MSKint32t *lind = NULL;
	MSKrealt *lval = NULL;
	AffineFunctionPtrForm<MSKint32t, MSKint32t, MSKrealt> affine_ptr_form;
	if (affine_part.has_value())
	{
		const auto &affine_function = affine_part.value();
		affine_ptr_form.make(this, affine_function);
		numlnz = affine_ptr_form.numnz;
		lind = affine_ptr_form.index;
		lval = affine_ptr_form.value;
	}

	QuadraticFunctionPtrForm<MSKint32t, MSKint32t, MSKrealt> ptr_form;
	ptr_form.make(this, function);
	MSKint32t numqnz = ptr_form.numnz;
	MSKint32t *qrow = ptr_form.row;
	MSKint32t *qcol = ptr_form.col;
	MSKrealt *qval = ptr_form.value;

	MSKboundkeye g_sense = mosek_con_sense(sense);
	MSKrealt g_rhs = rhs;
	if (affine_part)
		g_rhs -= affine_part->constant.value_or(0.0);

	error = MSK_putarow(m_model.get(), row, numlnz, lind, lval);
	check_error(error);
	error = MSK_putqconk(m_model.get(), row, numqnz, qrow, qcol, qval);
	check_error(error);
	error = MSK_putconbound(m_model.get(), row, g_sense, g_rhs, g_rhs);
	check_error(error);

	return constraint_index;
}

void MOSEKModel::delete_constraint(const ConstraintIndex &constraint)
{
	MSKrescodee error;
	MSKint32t constraint_row = _constraint_index(constraint);
	if (constraint_row >= 0)
	{
		switch (constraint.type)
		{
		case ConstraintType::Linear:
		case ConstraintType::Quadratic:
			m_linear_quadratic_constraint_index.delete_index(constraint.index);
			error = MSK_removecons(m_model.get(), 1, &constraint_row);
			break;
		default:
			throw std::runtime_error("Unknown constraint type");
		}
	}
	check_error(error);
}

bool MOSEKModel::is_constraint_active(const ConstraintIndex &constraint)
{
	switch (constraint.type)
	{
	case ConstraintType::Linear:
	case ConstraintType::Quadratic:
		return m_linear_quadratic_constraint_index.has_index(constraint.index);
	default:
		throw std::runtime_error("Unknown constraint type");
	}
}

void MOSEKModel::_set_affine_objective(const ScalarAffineFunction &function, ObjectiveSense sense,
                                       bool clear_quadratic)
{
	MSKrescodee error;
	if (clear_quadratic)
	{
		// First delete all quadratic terms
		error = MSK_putqobj(m_model.get(), 0, nullptr, nullptr, nullptr);
		check_error(error);
	}

	// Set Obj attribute of each variable
	MSKint32t n_variables;
	error = MSK_getnumvar(m_model.get(), &n_variables);
	check_error(error);
	std::vector<MSKrealt> obj_v(n_variables, 0.0);

	int numnz = function.size();
	for (int i = 0; i < numnz; i++)
	{
		auto column = _variable_index(function.variables[i]);
		if (column < 0)
		{
			throw std::runtime_error("Variable does not exist");
		}
		obj_v[column] = function.coefficients[i];
	}

	error = MSK_putcslice(m_model.get(), 0, n_variables, obj_v.data());
	check_error(error);
	error = MSK_putcfix(m_model.get(), function.constant.value_or(0.0));
	check_error(error);

	MSKobjsensee obj_sense = mosek_obj_sense(sense);
	error = MSK_putobjsense(m_model.get(), obj_sense);
	check_error(error);
}

void MOSEKModel::set_objective(const ScalarAffineFunction &function, ObjectiveSense sense)
{
	_set_affine_objective(function, sense, true);
}

void MOSEKModel::set_objective(const ScalarQuadraticFunction &function, ObjectiveSense sense)
{
	MSKrescodee error;

	// Add quadratic term
	int numqnz = function.size();
	if (numqnz > 0)
	{
		QuadraticFunctionPtrForm<MSKint32t, MSKint32t, MSKrealt> ptr_form;
		ptr_form.make(this, function);
		MSKint32t numqnz = ptr_form.numnz;
		MSKint32t *qrow = ptr_form.row;
		MSKint32t *qcol = ptr_form.col;
		MSKrealt *qval = ptr_form.value;

		error = MSK_putqobj(m_model.get(), numqnz, qrow, qcol, qval);
		check_error(error);
	}
	else
	{
		// delete all quadratic terms
		error = MSK_putqobj(m_model.get(), 0, nullptr, nullptr, nullptr);
		check_error(error);
	}

	// Affine part
	const auto &affine_part = function.affine_part;
	if (affine_part)
	{
		const auto &affine_function = affine_part.value();
		_set_affine_objective(affine_function, sense, false);
	}
	else
	{
		ScalarAffineFunction zero;
		_set_affine_objective(zero, sense, false);
	}
}

void MOSEKModel::set_objective(const ExprBuilder &function, ObjectiveSense sense)
{
	auto deg = function.degree();
	if (deg <= 1)
	{
		ScalarAffineFunction f(function);
		set_objective(f, sense);
	}
	else if (deg == 2)
	{
		ScalarQuadraticFunction f(function);
		set_objective(f, sense);
	}
	else
	{
		throw std::runtime_error("Objective must be linear or quadratic");
	}
}

int MOSEKModel::optimize()
{
	auto error = MSK_optimize(m_model.get());
	m_soltype = select_available_solution_after_optimization();
	return error;
}

int MOSEKModel::raw_parameter_type(const char *name)
{
	MSKparametertypee type;
	MSKint32t index;
	auto error = MSK_whichparam(m_model.get(), name, &type, &index);
	check_error(error);
	return type;
}

void MOSEKModel::set_raw_parameter_int(const char *param_name, int value)
{
	auto error = MSK_putnaintparam(m_model.get(), param_name, value);
	check_error(error);
}

void MOSEKModel::set_raw_parameter_double(const char *param_name, double value)
{
	auto error = MSK_putnadouparam(m_model.get(), param_name, value);
	check_error(error);
}

void MOSEKModel::set_raw_parameter_string(const char *param_name, const char *value)
{
	auto error = MSK_putnastrparam(m_model.get(), param_name, value);
	check_error(error);
}

int MOSEKModel::get_raw_parameter_int(const char *param_name)
{
	int retval;
	auto error = MSK_getnaintparam(m_model.get(), param_name, &retval);
	check_error(error);
	return retval;
}

double MOSEKModel::get_raw_parameter_double(const char *param_name)
{
	double retval;
	auto error = MSK_getnadouparam(m_model.get(), param_name, &retval);
	check_error(error);
	return retval;
}

std::string MOSEKModel::get_raw_parameter_string(const char *param_name)
{
	char retval[MSK_MAX_STR_LEN];
	MSKint32t len;
	auto error = MSK_getnastrparam(m_model.get(), param_name, strlen(param_name), &len, retval);
	check_error(error);
	return std::string(retval, len);
}

int MOSEKModel::get_raw_information_int(const char *attr_name)
{
	int retval;
	auto error = MSK_getnaintinf(m_model.get(), attr_name, &retval);
	check_error(error);
	return retval;
}

double MOSEKModel::get_raw_information_double(const char *attr_name)
{
	double retval;
	auto error = MSK_getnadouinf(m_model.get(), attr_name, &retval);
	check_error(error);
	return retval;
}

MSKint32t MOSEKModel::getnumvar()
{
	MSKint32t retval;
	auto error = MSK_getnumvar(m_model.get(), &retval);
	check_error(error);
	return retval;
}

MSKint32t MOSEKModel::getnumcon()
{
	MSKint32t retval;
	auto error = MSK_getnumcon(m_model.get(), &retval);
	check_error(error);
	return retval;
}

int MOSEKModel::getprosta()
{
	MSKprostae retval;
	auto error = MSK_getprosta(m_model.get(), get_current_solution(), &retval);
	check_error(error);
	return retval;
}

int MOSEKModel::getsolsta()
{
	MSKsolstae retval;
	auto error = MSK_getsolsta(m_model.get(), get_current_solution(), &retval);
	check_error(error);
	return retval;
}

double MOSEKModel::getprimalobj()
{
	MSKrealt retval;
	auto error = MSK_getprimalobj(m_model.get(), get_current_solution(), &retval);
	check_error(error);
	return retval;
}

double MOSEKModel::getdualobj()
{
	MSKrealt retval;
	auto error = MSK_getdualobj(m_model.get(), get_current_solution(), &retval);
	check_error(error);
	return retval;
}

static void printstr(void *handle, const char *str)
{
	printf("%s", str);
	fflush(stdout);
}
void MOSEKModel::enable_log()
{
	auto error = MSK_linkfunctotaskstream(m_model.get(), MSK_STREAM_LOG, NULL, printstr);
	check_error(error);
}

void MOSEKModel::disable_log()
{
	auto error = MSK_linkfunctotaskstream(m_model.get(), MSK_STREAM_LOG, NULL, NULL);
	check_error(error);
}

std::string MOSEKModel::get_variable_name(const VariableIndex &variable)
{
	auto column = _checked_variable_index(variable);
	char name[MSK_MAX_STR_LEN];
	auto error = MSK_getvarname(m_model.get(), column, MSK_MAX_STR_LEN, name);
	check_error(error);
	return name;
}

void MOSEKModel::set_variable_name(const VariableIndex &variable, const char *name)
{
	auto column = _checked_variable_index(variable);
	auto error = MSK_putvarname(m_model.get(), column, name);
	check_error(error);
}

VariableDomain MOSEKModel::get_variable_type(const VariableIndex &variable)
{
	if (binary_variables.contains(variable.index))
	{
		return VariableDomain::Binary;
	}
	auto column = _checked_variable_index(variable);
	MSKvariabletypee vtype;
	auto error = MSK_getvartype(m_model.get(), column, &vtype);
	check_error(error);
	return mosek_vtype_to_domain(vtype);
}

void MOSEKModel::set_variable_type(const VariableIndex &variable, VariableDomain domain)
{
	MSKvariabletypee vtype = mosek_vtype(domain);
	auto column = _checked_variable_index(variable);
	auto error = MSK_putvartype(m_model.get(), column, vtype);
	check_error(error);

	if (domain == VariableDomain::Binary)
	{
		MSKrealt lb = 0.0;
		MSKrealt ub = 1.0;
		binary_variables.insert(variable.index);
		error = MSK_putvarbound(m_model.get(), column, MSK_BK_RA, lb, ub);
		check_error(error);
	}
	else
	{
		binary_variables.erase(variable.index);
	}
}

double MOSEKModel::get_variable_lower_bound(const VariableIndex &variable)
{
	MSKboundkeye bound_type;
	MSKrealt lb, ub;
	auto column = _checked_variable_index(variable);
	auto error = MSK_getvarbound(m_model.get(), column, &bound_type, &lb, &ub);
	check_error(error);

	switch (bound_type)
	{
	case MSK_BK_FR:
	case MSK_BK_UP:
		lb = -MSK_INFINITY;
		break;
	case MSK_BK_LO:
	case MSK_BK_RA:
	case MSK_BK_FX:
		break;
	default:
		throw std::runtime_error("Unknown bound type");
	}
	return lb;
}

double MOSEKModel::get_variable_upper_bound(const VariableIndex &variable)
{
	MSKboundkeye bound_type;
	MSKrealt lb, ub;
	auto column = _checked_variable_index(variable);
	auto error = MSK_getvarbound(m_model.get(), column, &bound_type, &lb, &ub);
	check_error(error);

	switch (bound_type)
	{
	case MSK_BK_FR:
	case MSK_BK_LO:
		ub = MSK_INFINITY;
		break;
	case MSK_BK_UP:
	case MSK_BK_RA:
	case MSK_BK_FX:
		break;
	default:
		throw std::runtime_error("Unknown bound type");
	}
	return ub;
}

void MOSEKModel::set_variable_lower_bound(const VariableIndex &variable, double lb)
{
	MSKboundkeye bound_type_old, bound_key;
	MSKrealt lb_old, ub_old;
	bool needs_reset = false;
	auto column = _checked_variable_index(variable);
	auto error = MSK_getvarbound(m_model.get(), column, &bound_type_old, &lb_old, &ub_old);
	check_error(error);

	switch (bound_type_old)
	{
	case MSK_BK_FR:
		bound_key = MSK_BK_LO;
		needs_reset = true;
		break;
	case MSK_BK_LO:
		if (lb_old < lb)
		{
			needs_reset = false;
		}
		else
		{
			bound_key = MSK_BK_LO;
			needs_reset = true;
		}
		break;
	case MSK_BK_UP:
		bound_key = MSK_BK_RA;
		needs_reset = true;
		break;
	case MSK_BK_RA:
		if (lb_old < lb)
		{
			needs_reset = false;
		}
		else
		{
			bound_key = MSK_BK_RA;
			needs_reset = true;
		}
		break;
	case MSK_BK_FX:
		throw std::runtime_error("Cannot set lower bound for fixed variable");
	default:
		throw std::runtime_error("Unknown bound type");
	}

	if (needs_reset)
	{
		error = MSK_putvarbound(m_model.get(), column, bound_key, lb, ub_old);
		check_error(error);
	}
}

void MOSEKModel::set_variable_upper_bound(const VariableIndex &variable, double ub)
{
	MSKboundkeye bound_type_old, bound_key;
	MSKrealt lb_old, ub_old;
	bool needs_reset = false;
	auto column = _checked_variable_index(variable);
	auto error = MSK_getvarbound(m_model.get(), column, &bound_type_old, &lb_old, &ub_old);
	check_error(error);

	switch (bound_type_old)
	{
	case MSK_BK_FR:
		bound_key = MSK_BK_UP;
		needs_reset = true;
		break;
	case MSK_BK_UP:
		if (ub_old > ub)
		{
			needs_reset = false;
		}
		else
		{
			bound_key = MSK_BK_UP;
			needs_reset = true;
		}
		break;
	case MSK_BK_LO:
		bound_key = MSK_BK_RA;
		needs_reset = true;
		break;
	case MSK_BK_RA:
		if (ub_old > ub)
		{
			needs_reset = false;
		}
		else
		{
			bound_key = MSK_BK_RA;
			needs_reset = true;
		}
		break;
	case MSK_BK_FX:
		throw std::runtime_error("Cannot set upper bound for fixed variable");
	default:
		throw std::runtime_error("Unknown bound type");
	}

	if (needs_reset)
	{
		error = MSK_putvarbound(m_model.get(), column, bound_key, lb_old, ub);
		check_error(error);
	}
}

void MOSEKModel::set_variable_primal(const VariableIndex &variable, double primal)
{
	auto column = _checked_variable_index(variable);
	MSKrealt val = primal;
	auto error = MSK_putxxslice(m_model.get(), MSK_SOL_ITG, column, column + 1, &val);
	check_error(error);
}

double MOSEKModel::get_constraint_primal(const ConstraintIndex &constraint)
{
	int row = _checked_constraint_index(constraint);
	auto soltype = get_current_solution();
	MSKrealt retval;
	int num = 1;
	MSKrescodee error;
	switch (constraint.type)
	{
	case ConstraintType::Linear:
	case ConstraintType::Quadratic:
		error = MSK_getxcslice(m_model.get(), soltype, num, num + 1, &retval);
		break;
	default:
		throw std::runtime_error("Unknown constraint type");
	}
	check_error(error);
	return retval;
}

double MOSEKModel::get_constraint_dual(const ConstraintIndex &constraint)
{
	int row = _checked_constraint_index(constraint);
	auto soltype = get_current_solution();
	MSKrealt retval;
	int num = 1;
	MSKrescodee error;
	switch (constraint.type)
	{
	case ConstraintType::Linear:
	case ConstraintType::Quadratic:
		error = MSK_getyslice(m_model.get(), soltype, num, num + 1, &retval);
		break;
	default:
		throw std::runtime_error("Unknown constraint type");
	}
	check_error(error);
	return retval;
}

std::string MOSEKModel::get_constraint_name(const ConstraintIndex &constraint)
{
	auto row = _checked_constraint_index(constraint);
	MSKrescodee error;
	MSKint32t reqsize;
	switch (constraint.type)
	{
	case ConstraintType::Linear:
	case ConstraintType::Quadratic:
		error = MSK_getconnamelen(m_model.get(), row, &reqsize);
		break;
	default:
		throw std::runtime_error("Unknown constraint type");
	}
	check_error(error);
	std::string retval(reqsize - 1, '\0');
	switch (constraint.type)
	{
	case ConstraintType::Linear:
	case ConstraintType::Quadratic:
		error = MSK_getconname(m_model.get(), row, reqsize, retval.data());
		break;
	}
	check_error(error);
	return retval;
}

void MOSEKModel::set_constraint_name(const ConstraintIndex &constraint, const char *name)
{
	auto row = _checked_constraint_index(constraint);
	MSKrescodee error;
	switch (constraint.type)
	{
	case ConstraintType::Linear:
	case ConstraintType::Quadratic:
		error = MSK_putconname(m_model.get(), row, name);
		break;
	default:
		throw std::runtime_error("Unknown constraint type");
	}
	check_error(error);
}

ObjectiveSense MOSEKModel::get_obj_sense()
{
	MSKobjsensee obj_sense;
	auto error = MSK_getobjsense(m_model.get(), &obj_sense);
	check_error(error);
	auto sense = obj_sense == MSK_OBJECTIVE_SENSE_MINIMIZE ? ObjectiveSense::Minimize
	                                                       : ObjectiveSense::Maximize;
	return sense;
}

void MOSEKModel::set_obj_sense(ObjectiveSense sense)
{
	auto obj_sense = mosek_obj_sense(sense);
	auto error = MSK_putobjsense(m_model.get(), obj_sense);
	check_error(error);
}

double MOSEKModel::get_normalized_rhs(const ConstraintIndex &constraint)
{
	auto row = _checked_constraint_index(constraint);
	MSKrescodee error;
	switch (constraint.type)
	{
	case ConstraintType::Linear:
	case ConstraintType::Quadratic: {
		MSKboundkeye bk;
		MSKrealt lb, ub;
		error = MSK_getconbound(m_model.get(), row, &bk, &lb, &ub);
		check_error(error);

		double rhs;
		switch (bk)
		{
		case MSK_BK_UP:
			rhs = ub;
			break;
		case MSK_BK_LO:
			rhs = lb;
			break;
		case MSK_BK_FX:
			rhs = lb;
			break;
		case MSK_BK_FR:
			throw std::runtime_error("Constraint has no finite bound");
			break;
		case MSK_BK_RA:
			throw std::runtime_error("Constraint has two finite bounds");
			break;
		default:
			throw std::runtime_error("Unknown bound type");
		}

		return rhs;
	}
	default:
		throw std::runtime_error("Unknown constraint type to get_normalized_rhs");
	}
}

void MOSEKModel::set_normalized_rhs(const ConstraintIndex &constraint, double value)
{
	auto row = _checked_constraint_index(constraint);
	MSKrescodee error;
	switch (constraint.type)
	{
	case ConstraintType::Linear:
	case ConstraintType::Quadratic: {
		MSKboundkeye bk;
		MSKrealt lb, ub;
		error = MSK_getconbound(m_model.get(), row, &bk, &lb, &ub);
		check_error(error);

		switch (bk)
		{
		case MSK_BK_UP:
			ub = value;
			break;
		case MSK_BK_LO:
			lb = value;
			break;
		case MSK_BK_FX:
			ub = value;
			lb = value;
			break;
		case MSK_BK_FR:
			throw std::runtime_error("Constraint has no finite bound");
			break;
		case MSK_BK_RA:
			throw std::runtime_error("Constraint has two finite bounds");
			break;
		default:
			throw std::runtime_error("Unknown bound type");
		}

		error = MSK_putconbound(m_model.get(), row, bk, lb, ub);
		check_error(error);
	}
	break;
	default:
		throw std::runtime_error("Unknown constraint type to set_normalized_rhs");
	}
}

double MOSEKModel::get_normalized_coefficient(const ConstraintIndex &constraint,
                                              const VariableIndex &variable)
{
	if (constraint.type != ConstraintType::Linear && constraint.type != ConstraintType::Quadratic)
	{
		throw std::runtime_error(
		    "Only linear and quadratic constraint supports get_normalized_coefficient");
	}
	auto row = _checked_constraint_index(constraint);
	auto col = _checked_variable_index(variable);
	MSKrealt retval;
	auto error = MSK_getaij(m_model.get(), row, col, &retval);
	check_error(error);
	return retval;
}

void MOSEKModel::set_normalized_coefficient(const ConstraintIndex &constraint,
                                            const VariableIndex &variable, double value)
{
	if (constraint.type != ConstraintType::Linear && constraint.type != ConstraintType::Quadratic)
	{
		throw std::runtime_error(
		    "Only linear and quadratic constraint supports set_normalized_coefficient");
	}
	auto row = _checked_constraint_index(constraint);
	auto col = _checked_variable_index(variable);
	auto error = MSK_putaij(m_model.get(), row, col, value);
	check_error(error);
}

double MOSEKModel::get_objective_coefficient(const VariableIndex &variable)
{
	auto column = _checked_variable_index(variable);
	MSKrealt retval;
	auto error = MSK_getcj(m_model.get(), column, &retval);
	check_error(error);
	return retval;
}

void MOSEKModel::set_objective_coefficient(const VariableIndex &variable, double value)
{
	auto column = _checked_variable_index(variable);
	auto error = MSK_putcj(m_model.get(), column, value);
	check_error(error);
}

MSKint32t MOSEKModel::_variable_index(const VariableIndex &variable)
{
	return m_variable_index.get_index(variable.index);
}

MSKint32t MOSEKModel::_checked_variable_index(const VariableIndex &variable)
{
	MSKint32t column = _variable_index(variable);
	if (column < 0)
	{
		throw std::runtime_error("Variable does not exist");
	}
	return column;
}

MSKint32t MOSEKModel::_constraint_index(const ConstraintIndex &constraint)
{
	switch (constraint.type)
	{
	case ConstraintType::Linear:
	case ConstraintType::Quadratic:
		return m_linear_quadratic_constraint_index.get_index(constraint.index);
		break;
	default:
		throw std::runtime_error("Unknown constraint type");
	}
}

MSKint32t MOSEKModel::_checked_constraint_index(const ConstraintIndex &constraint)
{
	MSKint32t row = _constraint_index(constraint);
	if (row < 0)
	{
		throw std::runtime_error("Constraint does not exist");
	}
	return row;
}

void *MOSEKModel::get_raw_model()
{
	return m_model.get();
}

std::string MOSEKModel::version_string()
{
	std::string version =
	    fmt::format("v{}.{}.{}", MSK_VERSION_MAJOR, MSK_VERSION_MINOR, MSK_VERSION_REVISION);
	return version;
}

MSKsoltypee MOSEKModel::get_current_solution()
{
	if (m_soltype)
	{
		return m_soltype.value();
	}
	throw std::runtime_error("No solution type is available");
}

std::optional<MSKsoltypee> MOSEKModel::select_available_solution_after_optimization()
{
	std::vector<MSKsoltypee> soltypes{
	    MSK_SOL_ITR,
	    MSK_SOL_ITG,
	    MSK_SOL_BAS,
	};
	std::vector<MSKsoltypee> available_soltypes;
	std::vector<MSKsoltypee> optimal_soltypes;
	for (auto soltype : soltypes)
	{
		MSKbooleant available;
		auto error = MSK_solutiondef(m_model.get(), soltype, &available);
		check_error(error);
		if (available)
		{
			available_soltypes.push_back(soltype);

			MSKsolstae solsta;
			auto error = MSK_getsolsta(m_model.get(), soltype, &solsta);
			check_error(error);

			if (solsta == MSK_SOL_STA_OPTIMAL || solsta == MSK_SOL_STA_INTEGER_OPTIMAL)
			{
				optimal_soltypes.push_back(soltype);
			}
		}
	}

	if (!optimal_soltypes.empty())
	{
		return optimal_soltypes[0];
	}
	if (!available_soltypes.empty())
	{
		return available_soltypes[0];
	}
	return std::nullopt;
}

MOSEKEnv::MOSEKEnv()
{
	auto error = MSK_makeenv(&m_env, NULL);
	check_error(error);
}

MOSEKEnv::~MOSEKEnv()
{
	auto error = MSK_deleteenv(&m_env);
	check_error(error);
}

void MOSEKEnv::putlicensecode(const std::vector<MSKint32t> &code)
{
	auto error = MSK_putlicensecode(m_env, code.data());
	check_error(error);
}
