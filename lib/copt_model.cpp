#include "pyoptinterface/solver_common.hpp"
#include "pyoptinterface/copt_model.hpp"
#include "fmt/core.h"

static void check_error(int error)
{
	if (error)
	{
		char errmsg[COPT_BUFFSIZE];

		COPT_GetRetcodeMsg(error, errmsg, COPT_BUFFSIZE);
		throw std::runtime_error(errmsg);
	}
}

char copt_con_sense(ConstraintSense sense)
{
	using enum ConstraintSense;
	switch (sense)
	{
	case LessEqual:
		return COPT_LESS_EQUAL;
	case Equal:
		return COPT_EQUAL;
	case GreaterEqual:
		return COPT_GREATER_EQUAL;
	default:
		throw std::runtime_error("Unknown constraint sense");
	}
}

int copt_obj_sense(ObjectiveSense sense)
{
	using enum ObjectiveSense;
	switch (sense)
	{
	case Minimize:
		return COPT_MINIMIZE;
	case Maximize:
		return COPT_MAXIMIZE;
	default:
		throw std::runtime_error("Unknown objective sense");
	}
}

char copt_vtype(VariableDomain domain)
{
	using enum VariableDomain;
	switch (domain)
	{
	case Continuous:
		return COPT_CONTINUOUS;
	case Integer:
		return COPT_INTEGER;
	case Binary:
		return COPT_BINARY;
	default:
		throw std::runtime_error("Unknown variable domain");
	}
}

VariableDomain copt_vtype_to_domain(char vtype)
{
	using enum VariableDomain;
	switch (vtype)
	{
	case COPT_CONTINUOUS:
		return Continuous;
	case COPT_INTEGER:
		return Integer;
	case COPT_BINARY:
		return Binary;
	default:
		throw std::runtime_error("Unknown variable domain");
	}
}

ConstraintType copt_sostype(int type)
{
	switch (type)
	{
	case COPT_SOS_TYPE1:
		return ConstraintType::SOS1;
	case COPT_SOS_TYPE2:
		return ConstraintType::SOS2;
	default:
		throw std::runtime_error("Unknown SOS type");
	}
}

COPTModel::COPTModel(const COPTEnv &env)
{
	init(env);
}

void COPTModel::init(const COPTEnv &env)
{
	copt_prob *model;
	int error = COPT_CreateProb(env.m_env, &model);
	check_error(error);
	m_model = std::unique_ptr<copt_prob, COPTfreemodelT>(model);
}

VariableIndex COPTModel::add_variable(VariableDomain domain, double lb, double ub, const char *name)
{
	if (name != nullptr && name[0] == '\0')
	{
		name = nullptr;
	}
	IndexT index = m_variable_index.add_index();
	VariableIndex variable(index);

	char vtype = copt_vtype(domain);
	int error = COPT_AddCol(m_model.get(), 0.0, 0, NULL, NULL, vtype, lb, ub, name);
	check_error(error);

	return variable;
}

void COPTModel::delete_variable(const VariableIndex &variable)
{
	if (!is_variable_active(variable))
	{
		throw std::runtime_error("Variable does not exist");
	}

	int variable_column = _variable_index(variable);
	int error = COPT_DelCols(m_model.get(), 1, &variable_column);
	check_error(error);

	m_variable_index.delete_index(variable.index);
}

bool COPTModel::is_variable_active(const VariableIndex &variable)
{
	return m_variable_index.has_index(variable.index);
}

double COPTModel::get_variable_value(const VariableIndex &variable)
{
	return get_variable_info(variable, COPT_DBLINFO_VALUE);
}

std::string COPTModel::pprint_variable(const VariableIndex &variable)
{
	return get_variable_name(variable);
}

ConstraintIndex COPTModel::add_linear_constraint(const ScalarAffineFunction &function,
                                                 ConstraintSense sense, CoeffT rhs)
{
	IndexT index = m_linear_constraint_index.add_index();
	ConstraintIndex constraint_index(ConstraintType::Linear, index);

	AffineFunctionPtrForm<int, int, double> ptr_form;
	ptr_form.make(this, function);

	int numnz = ptr_form.numnz;
	int *cind = ptr_form.index;
	double *cval = ptr_form.value;
	char g_sense = copt_con_sense(sense);
	double g_rhs = rhs - function.constant.value_or(0.0);

	int error = COPT_AddRow(m_model.get(), numnz, cind, cval, g_sense, g_rhs, g_rhs, NULL);
	check_error(error);
	return constraint_index;
}

ConstraintIndex COPTModel::add_quadratic_constraint(const ScalarQuadraticFunction &function,
                                                    ConstraintSense sense, CoeffT rhs)
{
	IndexT index = m_quadratic_constraint_index.add_index();
	ConstraintIndex constraint_index(ConstraintType::Quadratic, index);

	const auto &affine_part = function.affine_part;

	int numlnz = 0;
	int *lind = NULL;
	double *lval = NULL;
	AffineFunctionPtrForm<int, int, double> affine_ptr_form;
	if (affine_part.has_value())
	{
		const auto &affine_function = affine_part.value();
		affine_ptr_form.make(this, affine_function);
		numlnz = affine_ptr_form.numnz;
		lind = affine_ptr_form.index;
		lval = affine_ptr_form.value;
	}

	QuadraticFunctionPtrForm<int, int, double> ptr_form;
	ptr_form.make(this, function);
	int numqnz = ptr_form.numnz;
	int *qrow = ptr_form.row;
	int *qcol = ptr_form.col;
	double *qval = ptr_form.value;

	char g_sense = copt_con_sense(sense);
	double g_rhs = rhs;
	if (affine_part)
		g_rhs -= affine_part->constant.value_or(0.0);

	int error = COPT_AddQConstr(m_model.get(), numlnz, lind, lval, numqnz, qrow, qcol, qval,
	                            g_sense, g_rhs, NULL);
	check_error(error);
	return constraint_index;
}

ConstraintIndex COPTModel::add_sos1_constraint(const Vector<VariableIndex> &variables,
                                               const Vector<CoeffT> &weights)
{
	return _add_sos_constraint(variables, weights, COPT_SOS_TYPE1);
}

ConstraintIndex COPTModel::add_sos2_constraint(const Vector<VariableIndex> &variables,
                                               const Vector<CoeffT> &weights)
{
	return _add_sos_constraint(variables, weights, COPT_SOS_TYPE2);
}

ConstraintIndex COPTModel::_add_sos_constraint(const Vector<VariableIndex> &variables,
                                               const Vector<CoeffT> &weights, int sos_type)
{
	IndexT index = m_sos_constraint_index.add_index();
	ConstraintIndex constraint_index(copt_sostype(sos_type), index);

	int numsos = 1;
	int nummembers = variables.size();
	int types = sos_type;
	int beg[] = {0};
	int cnt[] = {nummembers};
	std::vector<int> ind_v(nummembers);
	for (int i = 0; i < nummembers; i++)
	{
		ind_v[i] = _variable_index(variables[i].index);
	}
	int *ind = ind_v.data();
	double *weight = (double *)weights.data();

	int error = COPT_AddSOSs(m_model.get(), numsos, &types, beg, cnt, ind, weight);
	check_error(error);
	return constraint_index;
}

void COPTModel::delete_constraint(const ConstraintIndex &constraint)
{
	int error = 0;
	int constraint_row = _constraint_index(constraint);
	if (constraint_row >= 0)
	{
		switch (constraint.type)
		{
		case ConstraintType::Linear:
			m_linear_constraint_index.delete_index(constraint.index);
			error = COPT_DelRows(m_model.get(), 1, &constraint_row);
			break;
		case ConstraintType::Quadratic:
			m_quadratic_constraint_index.delete_index(constraint.index);
			error = COPT_DelQConstrs(m_model.get(), 1, &constraint_row);
			break;
		case ConstraintType::SOS1:
		case ConstraintType::SOS2:
			m_sos_constraint_index.delete_index(constraint.index);
			error = COPT_DelSOSs(m_model.get(), 1, &constraint_row);
			break;
		default:
			throw std::runtime_error("Unknown constraint type");
		}
	}
	check_error(error);
}

bool COPTModel::is_constraint_active(const ConstraintIndex &constraint)
{
	switch (constraint.type)
	{
	case ConstraintType::Linear:
		return m_linear_constraint_index.has_index(constraint.index);
	case ConstraintType::Quadratic:
		return m_quadratic_constraint_index.has_index(constraint.index);
	case ConstraintType::SOS1:
	case ConstraintType::SOS2:
		return m_sos_constraint_index.has_index(constraint.index);
	default:
		throw std::runtime_error("Unknown constraint type");
	}
}

void COPTModel::_set_affine_objective(const ScalarAffineFunction &function, ObjectiveSense sense,
                                      bool clear_quadratic)
{
	int error = 0;
	if (clear_quadratic)
	{
		// First delete all quadratic terms
		error = COPT_DelQuadObj(m_model.get());
		check_error(error);
	}

	// Set Obj attribute of each variable
	int n_variables = get_raw_attribute_int(COPT_INTATTR_COLS);
	std::vector<int> ind_v(n_variables);
	for (int i = 0; i < n_variables; i++)
	{
		ind_v[i] = i;
	}
	std::vector<double> obj_v(n_variables, 0.0);

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

	error = COPT_ReplaceColObj(m_model.get(), n_variables, ind_v.data(), obj_v.data());
	check_error(error);
	error = COPT_SetObjConst(m_model.get(), function.constant.value_or(0.0));
	check_error(error);

	int obj_sense = copt_obj_sense(sense);
	error = COPT_SetObjSense(m_model.get(), obj_sense);
	check_error(error);
}

void COPTModel::set_objective(const ScalarAffineFunction &function, ObjectiveSense sense)
{
	_set_affine_objective(function, sense, true);
}

void COPTModel::set_objective(const ScalarQuadraticFunction &function, ObjectiveSense sense)
{
	int error = 0;
	// First delete all quadratic terms
	error = COPT_DelQuadObj(m_model.get());
	check_error(error);

	// Add quadratic term
	int numqnz = function.size();
	if (numqnz > 0)
	{
		QuadraticFunctionPtrForm<int, int, double> ptr_form;
		ptr_form.make(this, function);
		int numqnz = ptr_form.numnz;
		int *qrow = ptr_form.row;
		int *qcol = ptr_form.col;
		double *qval = ptr_form.value;

		error = COPT_SetQuadObj(m_model.get(), numqnz, qrow, qcol, qval);
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

void COPTModel::set_objective(const ExprBuilder &function, ObjectiveSense sense)
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

void COPTModel::optimize()
{
	int error = COPT_Solve(m_model.get());
	check_error(error);
}

extern "C"
{
	int COPT_SearchParamAttr(copt_prob *prob, const char *name, int *p_type);
}
int COPTModel::raw_parameter_attribute_type(const char *name)
{
	int retval;
	int error = COPT_SearchParamAttr(m_model.get(), name, &retval);
	check_error(error);
	return retval;
}

void COPTModel::set_raw_parameter_int(const char *param_name, int value)
{
	int error = COPT_SetIntParam(m_model.get(), param_name, value);
	check_error(error);
}

void COPTModel::set_raw_parameter_double(const char *param_name, double value)
{
	int error = COPT_SetDblParam(m_model.get(), param_name, value);
	check_error(error);
}

int COPTModel::get_raw_parameter_int(const char *param_name)
{
	int retval;
	int error = COPT_GetIntParam(m_model.get(), param_name, &retval);
	check_error(error);
	return retval;
}

double COPTModel::get_raw_parameter_double(const char *param_name)
{
	double retval;
	int error = COPT_GetDblParam(m_model.get(), param_name, &retval);
	check_error(error);
	return retval;
}

int COPTModel::get_raw_attribute_int(const char *attr_name)
{
	int retval;
	int error = COPT_GetIntAttr(m_model.get(), attr_name, &retval);
	check_error(error);
	return retval;
}

double COPTModel::get_raw_attribute_double(const char *attr_name)
{
	double retval;
	int error = COPT_GetDblAttr(m_model.get(), attr_name, &retval);
	check_error(error);
	return retval;
}

double COPTModel::get_variable_info(const VariableIndex &variable, const char *info_name)
{
	auto column = _checked_variable_index(variable);
	double retval;
	int error = COPT_GetColInfo(m_model.get(), info_name, 1, &column, &retval);
	check_error(error);
	return retval;
}

std::string COPTModel::get_variable_name(const VariableIndex &variable)
{
	auto column = _checked_variable_index(variable);
	int error;
	int reqsize;
	error = COPT_GetColName(m_model.get(), column, NULL, 0, &reqsize);
	check_error(error);
	std::string retval(reqsize - 1, '\0');
	error = COPT_GetColName(m_model.get(), column, retval.data(), reqsize, &reqsize);
	check_error(error);
	return retval;
}

void COPTModel::set_variable_name(const VariableIndex &variable, const char *name)
{
	auto column = _checked_variable_index(variable);
	const char *names[] = {name};
	int error = COPT_SetColNames(m_model.get(), 1, &column, names);
	check_error(error);
}

VariableDomain COPTModel::get_variable_type(const VariableIndex &variable)
{
	auto column = _checked_variable_index(variable);
	char vtype;
	int error = COPT_GetColType(m_model.get(), 1, &column, &vtype);
	check_error(error);
	return copt_vtype_to_domain(vtype);
}

void COPTModel::set_variable_type(const VariableIndex &variable, VariableDomain domain)
{
	char vtype = copt_vtype(domain);
	auto column = _checked_variable_index(variable);
	int error = COPT_SetColType(m_model.get(), 1, &column, &vtype);
	check_error(error);
}

void COPTModel::set_variable_lower_bound(const VariableIndex &variable, double lb)
{
	auto column = _checked_variable_index(variable);
	int error = COPT_SetColLower(m_model.get(), 1, &column, &lb);
	check_error(error);
}

void COPTModel::set_variable_upper_bound(const VariableIndex &variable, double ub)
{
	auto column = _checked_variable_index(variable);
	int error = COPT_SetColUpper(m_model.get(), 1, &column, &ub);
	check_error(error);
}

double COPTModel::get_constraint_info(const ConstraintIndex &constraint, const char *info_name)
{
	int row = _checked_constraint_index(constraint);
	double retval;
	int num = 1;
	int error;
	switch (constraint.type)
	{
	case ConstraintType::Linear:
		error = COPT_GetRowInfo(m_model.get(), info_name, num, &row, &retval);
		break;
	case ConstraintType::Quadratic:
		error = COPT_GetQConstrInfo(m_model.get(), info_name, num, &row, &retval);
		break;
	default:
		throw std::runtime_error("Unknown constraint type");
	}
	check_error(error);
	return retval;
}

std::string COPTModel::get_constraint_name(const ConstraintIndex &constraint)
{
	int row = _checked_constraint_index(constraint);
	int error;
	int reqsize;
	switch (constraint.type)
	{
	case ConstraintType::Linear:
		error = COPT_GetRowName(m_model.get(), row, NULL, 0, &reqsize);
		break;
	case ConstraintType::Quadratic:
		error = COPT_GetQConstrName(m_model.get(), row, NULL, 0, &reqsize);
		break;
	default:
		throw std::runtime_error("Unknown constraint type");
	}
	check_error(error);
	std::string retval(reqsize - 1, '\0');
	switch (constraint.type)
	{
	case ConstraintType::Linear:
		error = COPT_GetRowName(m_model.get(), row, retval.data(), reqsize, &reqsize);
		break;
	case ConstraintType::Quadratic:
		error = COPT_GetQConstrName(m_model.get(), row, retval.data(), reqsize, &reqsize);
		break;
	}
	check_error(error);
	return retval;
}

void COPTModel::set_constraint_name(const ConstraintIndex &constraint, const char *name)
{
	int row = _checked_constraint_index(constraint);
	const char *names[] = {name};
	int error;
	switch (constraint.type)
	{
	case ConstraintType::Linear:
		error = COPT_SetRowNames(m_model.get(), 1, &row, names);
		break;
	case ConstraintType::Quadratic:
		error = COPT_SetQConstrNames(m_model.get(), 1, &row, names);
		break;
	default:
		throw std::runtime_error("Unknown constraint type");
	}
	check_error(error);
}

void COPTModel::set_obj_sense(ObjectiveSense sense)
{
	int obj_sense = copt_obj_sense(sense);
	int error = COPT_SetObjSense(m_model.get(), obj_sense);
	check_error(error);
}

void COPTModel::add_mip_start(const Vector<VariableIndex> &variables, const Vector<double> &values)
{
	int numnz = variables.size();
	if (numnz == 0)
		return;

	std::vector<int> ind_v(numnz);
	for (int i = 0; i < numnz; i++)
	{
		ind_v[i] = _variable_index(variables[i].index);
	}
	int *ind = ind_v.data();
	double *val = (double *)values.data();

	int error = COPT_AddMipStart(m_model.get(), numnz, ind, val);
	check_error(error);
}

double COPTModel::get_normalized_rhs(const ConstraintIndex &constraint)
{
	int row = _checked_constraint_index(constraint);
	int num = 1;
	int error;
	switch (constraint.type)
	{
	case ConstraintType::Linear: {
		double lb, ub;
		error = COPT_GetRowInfo(m_model.get(), COPT_DBLINFO_LB, num, &row, &lb);
		check_error(error);
		error = COPT_GetRowInfo(m_model.get(), COPT_DBLINFO_UB, num, &row, &ub);
		check_error(error);

		bool lb_inf = lb < -COPT_INFINITY + 1.0;
		bool ub_inf = ub > COPT_INFINITY - 1.0;

		if (!lb_inf)
			return lb;
		if (!ub_inf)
			return ub;

		throw std::runtime_error("Constraint has no finite bound");
	}
	break;
	case ConstraintType::Quadratic: {
		double rhs;
		error = COPT_GetQConstrRhs(m_model.get(), num, &row, &rhs);
		check_error(error);
		return rhs;
	}
	break;
	default:
		throw std::runtime_error("Unknown constraint type to get_normalized_rhs");
	}
}

void COPTModel::set_normalized_rhs(const ConstraintIndex &constraint, double value)
{
	int row = _checked_constraint_index(constraint);
	int num = 1;
	int error;
	switch (constraint.type)
	{
	case ConstraintType::Linear: {
		double lb, ub;

		error = COPT_GetRowInfo(m_model.get(), COPT_DBLINFO_LB, num, &row, &lb);
		check_error(error);
		error = COPT_GetRowInfo(m_model.get(), COPT_DBLINFO_UB, num, &row, &ub);
		check_error(error);

		bool lb_inf = lb < -COPT_INFINITY + 1.0;
		bool ub_inf = ub > COPT_INFINITY - 1.0;

		if (!lb_inf)
		{
			error = COPT_SetRowLower(m_model.get(), num, &row, &value);
			check_error(error);
		}
		if (!ub_inf)
		{
			error = COPT_SetRowUpper(m_model.get(), num, &row, &value);
			check_error(error);
		}

		if (lb_inf && ub_inf)
		{
			throw std::runtime_error("Constraint has no finite bound");
		}
	}
	break;
	case ConstraintType::Quadratic: {
		error = COPT_SetQConstrRhs(m_model.get(), num, &row, &value);
		check_error(error);
	}
	break;
	default:
		throw std::runtime_error("Unknown constraint type to set_normalized_rhs");
	}
}

double COPTModel::get_normalized_coefficient(const ConstraintIndex &constraint,
                                             const VariableIndex &variable)
{
	if (constraint.type != ConstraintType::Linear)
	{
		throw std::runtime_error("Only linear constraint supports get_normalized_coefficient");
	}
	int row = _checked_constraint_index(constraint);
	int col = _checked_variable_index(variable);
	double retval;
	int error = COPT_GetElem(m_model.get(), col, row, &retval);
	check_error(error);
	return retval;
}

void COPTModel::set_normalized_coefficient(const ConstraintIndex &constraint,
                                           const VariableIndex &variable, double value)
{
	if (constraint.type != ConstraintType::Linear)
	{
		throw std::runtime_error("Only linear constraint supports set_normalized_coefficient");
	}
	int row = _checked_constraint_index(constraint);
	int col = _checked_variable_index(variable);
	int error = COPT_SetElem(m_model.get(), col, row, value);
	check_error(error);
}

double COPTModel::get_objective_coefficient(const VariableIndex &variable)
{
	return get_variable_info(variable, COPT_DBLINFO_OBJ);
}

void COPTModel::set_objective_coefficient(const VariableIndex &variable, double value)
{
	auto column = _checked_variable_index(variable);
	int error = COPT_SetColObj(m_model.get(), 1, &column, &value);
	check_error(error);
}

int COPTModel::_variable_index(const VariableIndex &variable)
{
	return m_variable_index.get_index(variable.index);
}

int COPTModel::_checked_variable_index(const VariableIndex &variable)
{
	int column = _variable_index(variable);
	if (column < 0)
	{
		throw std::runtime_error("Variable does not exist");
	}
	return column;
}

int COPTModel::_constraint_index(const ConstraintIndex &constraint)
{
	switch (constraint.type)
	{
	case ConstraintType::Linear:
		return m_linear_constraint_index.get_index(constraint.index);
	case ConstraintType::Quadratic:
		return m_quadratic_constraint_index.get_index(constraint.index);
	case ConstraintType::SOS1:
	case ConstraintType::SOS2:
		return m_sos_constraint_index.get_index(constraint.index);
	default:
		throw std::runtime_error("Unknown constraint type");
	}
}

int COPTModel::_checked_constraint_index(const ConstraintIndex &constraint)
{
	int row = _constraint_index(constraint);
	if (row < 0)
	{
		throw std::runtime_error("Constraint does not exist");
	}
	return row;
}

void *COPTModel::get_raw_model()
{
	return m_model.get();
}

std::string COPTModel::version_string()
{
	std::string version =
	    fmt::format("v{}.{}.{}", COPT_VERSION_MAJOR, COPT_VERSION_MINOR, COPT_VERSION_TECHNICAL);
	return version;
}

COPTEnv::COPTEnv()
{
	int error = COPT_CreateEnv(&m_env);
	check_error(error);
}

COPTEnv::COPTEnv(COPTEnvConfig &config)
{
	int error = COPT_CreateEnvWithConfig(config.m_config, &m_env);
	check_error(error);
}

COPTEnv::~COPTEnv()
{
	int error = COPT_DeleteEnv(&m_env);
	check_error(error);
}

COPTEnvConfig::COPTEnvConfig()
{
	int error = COPT_CreateEnvConfig(&m_config);
	check_error(error);
}

COPTEnvConfig::~COPTEnvConfig()
{
	int error = COPT_DeleteEnvConfig(&m_config);
	check_error(error);
}

void COPTEnvConfig::set(const char *param_name, const char *value)
{
	int error = COPT_SetEnvConfig(m_config, param_name, value);
	check_error(error);
}
