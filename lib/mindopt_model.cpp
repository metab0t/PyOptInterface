#include "pyoptinterface/mindopt_model.hpp"
#include "fmt/core.h"

namespace mindopt
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
} // namespace mindopt

static void check_error(int error)
{
	if (error)
	{
		const char *errmsg = mindopt::MDOexplainerror(error);
		throw std::runtime_error(errmsg);
	}
}

static char mindopt_con_sense(ConstraintSense sense)
{
	switch (sense)
	{
	case ConstraintSense::LessEqual:
		return MDO_LESS_EQUAL;
	case ConstraintSense::Equal:
		return MDO_EQUAL;
	case ConstraintSense::GreaterEqual:
		return MDO_GREATER_EQUAL;
	default:
		throw std::runtime_error("Unknown constraint sense");
	}
}

static int mindopt_obj_sense(ObjectiveSense sense)
{
	switch (sense)
	{
	case ObjectiveSense::Minimize:
		return MDO_MINIMIZE;
	case ObjectiveSense::Maximize:
		return MDO_MAXIMIZE;
	default:
		throw std::runtime_error("Unknown objective sense");
	}
}

static char mindopt_vtype(VariableDomain domain)
{
	switch (domain)
	{
	case VariableDomain::Continuous:
		return MDO_CONTINUOUS;
	case VariableDomain::Integer:
		return MDO_INTEGER;
	case VariableDomain::Binary:
		return MDO_BINARY;
	case VariableDomain::SemiContinuous:
		return MDO_SEMICONT;
	default:
		throw std::runtime_error("Unknown variable domain");
	}
}

static int mindopt_sostype(SOSType type)
{
	switch (type)
	{
	case SOSType::SOS1:
		return MDO_SOS_TYPE1;
	case SOSType::SOS2:
		return MDO_SOS_TYPE2;
	default:
		throw std::runtime_error("Unknown SOS type");
	}
}

MindoptModel::MindoptModel(const MindoptEnv &env)
{
	init(env);
}

void MindoptModel::init(const MindoptEnv &env)
{
	if (!mindopt::is_library_loaded())
	{
		throw std::runtime_error("Mindopt library is not loaded");
	}

	MDOmodel *model;
	m_env = env.m_env;
	int error = mindopt::MDOnewmodel(env.m_env, &model, NULL, 0, NULL, NULL, NULL, NULL, NULL);
	check_error(error);
	m_model = std::unique_ptr<MDOmodel, MindoptfreemodelT>(model);
}

void MindoptModel::write(const std::string &filename)
{
	int error = mindopt::MDOwrite(m_model.get(), filename.c_str());
	check_error(error);
}

VariableIndex MindoptModel::add_variable(VariableDomain domain, double lb, double ub,
                                         const char *name)
{
	if (name != nullptr && name[0] == '\0')
	{
		name = nullptr;
	}
	IndexT index = m_variable_index.add_index();
	VariableIndex variable(index);

	// Create a new Mindopt variable
	char vtype = mindopt_vtype(domain);
	int error = mindopt::MDOaddvar(m_model.get(), 0, NULL, NULL, 0.0, lb, ub, vtype, name);
	check_error(error);

	return variable;
}

void MindoptModel::delete_variable(const VariableIndex &variable)
{
	if (!is_variable_active(variable))
	{
		throw std::runtime_error("Variable does not exist");
	}

	// Delete the corresponding Mindopt variable
	int variable_column = _variable_index(variable);
	int error = mindopt::MDOdelvars(m_model.get(), 1, &variable_column);
	check_error(error);

	m_variable_index.delete_index(variable.index);
}

void MindoptModel::delete_variables(const Vector<VariableIndex> &variables)
{
	int n_variables = variables.size();
	if (n_variables == 0)
		return;

	std::vector<int> columns;
	columns.reserve(n_variables);
	for (int i = 0; i < n_variables; i++)
	{
		if (!is_variable_active(variables[i]))
		{
			continue;
		}
		auto column = _variable_index(variables[i]);
		columns.push_back(column);
	}

	int error = mindopt::MDOdelvars(m_model.get(), columns.size(), columns.data());
	check_error(error);

	for (int i = 0; i < n_variables; i++)
	{
		m_variable_index.delete_index(variables[i].index);
	}
}

bool MindoptModel::is_variable_active(const VariableIndex &variable)
{
	return m_variable_index.has_index(variable.index);
}

double MindoptModel::get_variable_value(const VariableIndex &variable)
{
	return get_variable_raw_attribute_double(variable, MDO_DBL_ATTR_X);
}

std::string MindoptModel::pprint_variable(const VariableIndex &variable)
{
	return get_variable_raw_attribute_string(variable, MDO_STR_ATTR_VAR_NAME);
}

void MindoptModel::set_variable_bounds(const VariableIndex &variable, double lb, double ub)
{
	auto column = _checked_variable_index(variable);
	int error;
	error = mindopt::MDOsetdblattrelement(m_model.get(), MDO_DBL_ATTR_LB, column, lb);
	check_error(error);
	error = mindopt::MDOsetdblattrelement(m_model.get(), MDO_DBL_ATTR_UB, column, ub);
	check_error(error);
}

void MindoptModel::set_variable_name(const VariableIndex &variable, const char *name)
{
	set_variable_raw_attribute_string(variable, MDO_STR_ATTR_VAR_NAME, name);
}

void MindoptModel::set_constraint_name(const ConstraintIndex &constraint, const char *name)
{
	const char *attr_name;
	switch (constraint.type)
	{
	case ConstraintType::Linear:
		attr_name = MDO_STR_ATTR_CONSTR_NAME;
		break;
	case ConstraintType::Quadratic:
		attr_name = MDO_STR_ATTR_Q_C_NAME;
		break;
	default:
		throw std::runtime_error("Unknown constraint type to set name!");
	}
	set_constraint_raw_attribute_string(constraint, attr_name, name);
}

ConstraintIndex MindoptModel::add_linear_constraint(const ScalarAffineFunction &function,
                                                    ConstraintSense sense, CoeffT rhs,
                                                    const char *name)
{
	IndexT index = m_linear_constraint_index.add_index();
	ConstraintIndex constraint_index(ConstraintType::Linear, index);

	// Create a new Mindopt linear constraint
	AffineFunctionPtrForm<int, int, double> ptr_form;
	ptr_form.make(this, function);

	int numnz = ptr_form.numnz;
	int *cind = ptr_form.index;
	double *cval = ptr_form.value;
	char g_sense = mindopt_con_sense(sense);
	double g_rhs = rhs - function.constant.value_or(0.0);

	if (name != nullptr && name[0] == '\0')
	{
		name = nullptr;
	}

	int error = mindopt::MDOaddconstr(m_model.get(), numnz, cind, cval, g_sense, g_rhs, name);
	check_error(error);

	return constraint_index;
}

ConstraintIndex MindoptModel::add_quadratic_constraint(const ScalarQuadraticFunction &function,
                                                       ConstraintSense sense, CoeffT rhs,
                                                       const char *name)
{
	IndexT index = m_quadratic_constraint_index.add_index();
	ConstraintIndex constraint_index(ConstraintType::Quadratic, index);

	// Create a new Mindopt quadratic constraint
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

	char g_sense = mindopt_con_sense(sense);
	double g_rhs = rhs;
	if (affine_part)
		g_rhs -= affine_part->constant.value_or(0.0);

	if (name != nullptr && name[0] == '\0')
	{
		name = nullptr;
	}

	int error = mindopt::MDOaddqconstr(m_model.get(), numlnz, lind, lval, numqnz, qrow, qcol, qval,
	                                   g_sense, g_rhs, name);
	check_error(error);

	return constraint_index;
}

ConstraintIndex MindoptModel::add_sos_constraint(const Vector<VariableIndex> &variables,
                                                 SOSType sos_type)
{
	Vector<CoeffT> weights(variables.size(), 1.0);
	return add_sos_constraint(variables, sos_type, weights);
}

ConstraintIndex MindoptModel::add_sos_constraint(const Vector<VariableIndex> &variables,
                                                 SOSType sos_type, const Vector<CoeffT> &weights)
{
	IndexT index = m_sos_constraint_index.add_index();
	ConstraintIndex constraint_index(ConstraintType::SOS, index);

	// Create a new Mindopt SOS constraint
	int numsos = 1;
	int nummembers = variables.size();
	int types = mindopt_sostype(sos_type);
	int beg[] = {0, nummembers};
	std::vector<int> ind_v(nummembers);
	for (int i = 0; i < nummembers; i++)
	{
		ind_v[i] = _variable_index(variables[i].index);
	}
	int *ind = ind_v.data();
	double *weight = (double *)weights.data();

	int error = mindopt::MDOaddsos(m_model.get(), numsos, nummembers, &types, beg, ind, weight);
	check_error(error);

	return constraint_index;
}

void MindoptModel::delete_constraint(const ConstraintIndex &constraint)
{
	// Delete the corresponding Mindopt constraint
	int error = 0;
	int constraint_row = _constraint_index(constraint);
	if (constraint_row >= 0)
	{
		switch (constraint.type)
		{
		case ConstraintType::Linear:
			m_linear_constraint_index.delete_index(constraint.index);
			error = mindopt::MDOdelconstrs(m_model.get(), 1, &constraint_row);
			break;
		case ConstraintType::Quadratic:
			m_quadratic_constraint_index.delete_index(constraint.index);
			error = mindopt::MDOdelqconstrs(m_model.get(), 1, &constraint_row);
			break;
		case ConstraintType::SOS:
			m_sos_constraint_index.delete_index(constraint.index);
			error = mindopt::MDOdelsos(m_model.get(), 1, &constraint_row);
			break;
		default:
			throw std::runtime_error("Unknown constraint type");
		}
	}
	check_error(error);
}

bool MindoptModel::is_constraint_active(const ConstraintIndex &constraint)
{
	switch (constraint.type)
	{
	case ConstraintType::Linear:
		return m_linear_constraint_index.has_index(constraint.index);
	case ConstraintType::Quadratic:
		return m_quadratic_constraint_index.has_index(constraint.index);
	case ConstraintType::SOS:
		return m_sos_constraint_index.has_index(constraint.index);
	default:
		throw std::runtime_error("Unknown constraint type");
	}
}

void MindoptModel::_set_affine_objective(const ScalarAffineFunction &function, ObjectiveSense sense,
                                         bool clear_quadratic)
{
	int error = 0;
	if (clear_quadratic)
	{
		// First delete all quadratic terms
		error = mindopt::MDOdelq(m_model.get());
		check_error(error);
	}

	// Set Obj attribute of each variable
	int n_variables = get_model_raw_attribute_int(MDO_INT_ATTR_NUM_VARS);
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

	error = mindopt::MDOsetdblattrarray(m_model.get(), "Obj", 0, n_variables, obj_v.data());
	check_error(error);
	error = mindopt::MDOsetdblattr(m_model.get(), "ObjCon", function.constant.value_or(0.0));
	check_error(error);

	int obj_sense = mindopt_obj_sense(sense);
	set_model_raw_attribute_int("ModelSense", obj_sense);
}

void MindoptModel::set_objective(const ScalarAffineFunction &function, ObjectiveSense sense)
{
	_set_affine_objective(function, sense, true);
}

void MindoptModel::set_objective(const ScalarQuadraticFunction &function, ObjectiveSense sense)
{
	int error = 0;
	// First delete all quadratic terms
	error = mindopt::MDOdelq(m_model.get());
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

		error = mindopt::MDOaddqpterms(m_model.get(), numqnz, qrow, qcol, qval);
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

void MindoptModel::set_objective(const ExprBuilder &function, ObjectiveSense sense)
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

void MindoptModel::optimize()
{
	int error = mindopt::MDOoptimize(m_model.get());
	check_error(error);
}

void MindoptModel::set_raw_parameter_int(const char *param_name, int value)
{
	int error = mindopt::MDOsetintparam(m_env, param_name, value);
	check_error(error);
}

void MindoptModel::set_raw_parameter_double(const char *param_name, double value)
{
	int error = mindopt::MDOsetdblparam(m_env, param_name, value);
	check_error(error);
}

void MindoptModel::set_raw_parameter_string(const char *param_name, const char *value)
{
	int error = mindopt::MDOsetstrparam(m_env, param_name, value);
	check_error(error);
}

int MindoptModel::get_raw_parameter_int(const char *param_name)
{
	int retval;
	int error = mindopt::MDOgetintparam(m_env, param_name, &retval);
	check_error(error);
	return retval;
}

double MindoptModel::get_raw_parameter_double(const char *param_name)
{
	double retval;
	int error = mindopt::MDOgetdblparam(m_env, param_name, &retval);
	check_error(error);
	return retval;
}

std::string MindoptModel::get_raw_parameter_string(const char *param_name)
{
	char retval[MDO_MAX_STRLEN];
	int error = mindopt::MDOgetstrparam(m_env, param_name, retval);
	check_error(error);
	return std::string(retval);
}

int MindoptModel::raw_attribute_type(const char *attr_name)
{
	int datatypeP;
	int error = mindopt::MDOgetattrinfo(m_model.get(), attr_name, &datatypeP, NULL, NULL);
	check_error(error);
	return datatypeP;
}

void MindoptModel::set_model_raw_attribute_int(const char *attr_name, int value)
{
	int error = mindopt::MDOsetintattr(m_model.get(), attr_name, value);
	check_error(error);
}

void MindoptModel::set_model_raw_attribute_double(const char *attr_name, double value)
{
	int error = mindopt::MDOsetdblattr(m_model.get(), attr_name, value);
	check_error(error);
}

void MindoptModel::set_model_raw_attribute_string(const char *attr_name, const char *value)
{
	int error = mindopt::MDOsetstrattr(m_model.get(), attr_name, value);
	check_error(error);
}

int MindoptModel::get_model_raw_attribute_int(const char *attr_name)
{
	int retval;
	int error = mindopt::MDOgetintattr(m_model.get(), attr_name, &retval);
	check_error(error);
	return retval;
}

double MindoptModel::get_model_raw_attribute_double(const char *attr_name)
{
	double retval;
	int error = mindopt::MDOgetdblattr(m_model.get(), attr_name, &retval);
	check_error(error);
	return retval;
}

std::string MindoptModel::get_model_raw_attribute_string(const char *attr_name)
{
	char *retval;
	int error = mindopt::MDOgetstrattr(m_model.get(), attr_name, &retval);
	check_error(error);
	return std::string(retval);
}

std::vector<double> MindoptModel::get_model_raw_attribute_vector_double(const char *attr_name,
                                                                        int start, int len)
{
	std::vector<double> retval(len);
	int error = mindopt::MDOgetdblattrarray(m_model.get(), attr_name, start, len, retval.data());
	check_error(error);
	return retval;
}

std::vector<double> MindoptModel::get_model_raw_attribute_list_double(const char *attr_name,
                                                                      const std::vector<int> &ind)
{
	std::vector<double> retval(ind.size());
	int error = mindopt::MDOgetdblattrlist(m_model.get(), attr_name, ind.size(), (int *)ind.data(),
	                                       retval.data());
	check_error(error);
	return retval;
}

void MindoptModel::set_variable_raw_attribute_int(const VariableIndex &variable,
                                                  const char *attr_name, int value)
{
	auto column = _checked_variable_index(variable);
	int error = mindopt::MDOsetintattrelement(m_model.get(), attr_name, column, value);
	check_error(error);
}

void MindoptModel::set_variable_raw_attribute_char(const VariableIndex &variable,
                                                   const char *attr_name, char value)
{
	auto column = _checked_variable_index(variable);
	int error = mindopt::MDOsetcharattrelement(m_model.get(), attr_name, column, value);
	check_error(error);
}

void MindoptModel::set_variable_raw_attribute_double(const VariableIndex &variable,
                                                     const char *attr_name, double value)
{
	auto column = _checked_variable_index(variable);
	int error = mindopt::MDOsetdblattrelement(m_model.get(), attr_name, column, value);
	check_error(error);
}

void MindoptModel::set_variable_raw_attribute_string(const VariableIndex &variable,
                                                     const char *attr_name, const char *value)
{
	auto column = _checked_variable_index(variable);
	int error = mindopt::MDOsetstrattrelement(m_model.get(), attr_name, column, (char *)value);
	check_error(error);
}

int MindoptModel::get_variable_raw_attribute_int(const VariableIndex &variable,
                                                 const char *attr_name)
{
	auto column = _checked_variable_index(variable);
	int retval;
	int error = mindopt::MDOgetintattrelement(m_model.get(), attr_name, column, &retval);
	check_error(error);
	return retval;
}

char MindoptModel::get_variable_raw_attribute_char(const VariableIndex &variable,
                                                   const char *attr_name)
{
	auto column = _checked_variable_index(variable);
	char retval;
	int error = mindopt::MDOgetcharattrelement(m_model.get(), attr_name, column, &retval);
	check_error(error);
	return retval;
}

double MindoptModel::get_variable_raw_attribute_double(const VariableIndex &variable,
                                                       const char *attr_name)
{
	auto column = _checked_variable_index(variable);
	double retval;
	int error = mindopt::MDOgetdblattrelement(m_model.get(), attr_name, column, &retval);
	check_error(error);
	return retval;
}

std::string MindoptModel::get_variable_raw_attribute_string(const VariableIndex &variable,
                                                            const char *attr_name)
{
	auto column = _checked_variable_index(variable);
	char *retval;
	int error = mindopt::MDOgetstrattrelement(m_model.get(), attr_name, column, &retval);
	check_error(error);
	return std::string(retval);
}

int MindoptModel::_variable_index(const VariableIndex &variable)
{
	return m_variable_index.get_index(variable.index);
}

int MindoptModel::_checked_variable_index(const VariableIndex &variable)
{
	int column = _variable_index(variable);
	if (column < 0)
	{
		throw std::runtime_error("Variable does not exist");
	}
	return column;
}

void MindoptModel::set_constraint_raw_attribute_int(const ConstraintIndex &constraint,
                                                    const char *attr_name, int value)
{
	int row = _checked_constraint_index(constraint);
	int error = mindopt::MDOsetintattrelement(m_model.get(), attr_name, row, value);
	check_error(error);
}

void MindoptModel::set_constraint_raw_attribute_char(const ConstraintIndex &constraint,
                                                     const char *attr_name, char value)
{
	int row = _checked_constraint_index(constraint);
	int error = mindopt::MDOsetcharattrelement(m_model.get(), attr_name, row, value);
	check_error(error);
}

void MindoptModel::set_constraint_raw_attribute_double(const ConstraintIndex &constraint,
                                                       const char *attr_name, double value)
{
	int row = _checked_constraint_index(constraint);
	int error = mindopt::MDOsetdblattrelement(m_model.get(), attr_name, row, value);
	check_error(error);
}

void MindoptModel::set_constraint_raw_attribute_string(const ConstraintIndex &constraint,
                                                       const char *attr_name, const char *value)
{
	int row = _checked_constraint_index(constraint);
	int error = mindopt::MDOsetstrattrelement(m_model.get(), attr_name, row, (char *)value);
	check_error(error);
}

int MindoptModel::get_constraint_raw_attribute_int(const ConstraintIndex &constraint,
                                                   const char *attr_name)
{
	int row = _checked_constraint_index(constraint);
	int retval;
	int error = mindopt::MDOgetintattrelement(m_model.get(), attr_name, row, &retval);
	check_error(error);
	return retval;
}

char MindoptModel::get_constraint_raw_attribute_char(const ConstraintIndex &constraint,
                                                     const char *attr_name)
{
	int row = _checked_constraint_index(constraint);
	char retval;
	int error = mindopt::MDOgetcharattrelement(m_model.get(), attr_name, row, &retval);
	check_error(error);
	return retval;
}

double MindoptModel::get_constraint_raw_attribute_double(const ConstraintIndex &constraint,
                                                         const char *attr_name)
{
	int row = _checked_constraint_index(constraint);
	double retval;
	int error = mindopt::MDOgetdblattrelement(m_model.get(), attr_name, row, &retval);
	check_error(error);
	return retval;
}

std::string MindoptModel::get_constraint_raw_attribute_string(const ConstraintIndex &constraint,
                                                              const char *attr_name)
{
	int row = _checked_constraint_index(constraint);
	char *retval;
	int error = mindopt::MDOgetstrattrelement(m_model.get(), attr_name, row, &retval);
	check_error(error);
	return std::string(retval);
}

double MindoptModel::get_normalized_rhs(const ConstraintIndex &constraint)
{
	const char *name;
	switch (constraint.type)
	{
	case ConstraintType::Linear:
		name = MDO_DBL_ATTR_RHS;
		break;
	case ConstraintType::Quadratic:
		name = MDO_DBL_ATTR_Q_C_RHS;
		break;
	default:
		throw std::runtime_error("Unknown constraint type to get_normalized_rhs");
	}
	return get_constraint_raw_attribute_double(constraint, name);
}

void MindoptModel::set_normalized_rhs(const ConstraintIndex &constraint, double value)
{
	const char *name;
	switch (constraint.type)
	{
	case ConstraintType::Linear:
		name = MDO_DBL_ATTR_RHS;
		break;
	case ConstraintType::Quadratic:
		name = MDO_DBL_ATTR_Q_C_RHS;
		break;
	default:
		throw std::runtime_error("Unknown constraint type to set_normalized_rhs");
	}
	set_constraint_raw_attribute_double(constraint, name, value);
}

double MindoptModel::get_normalized_coefficient(const ConstraintIndex &constraint,
                                                const VariableIndex &variable)
{
	if (constraint.type != ConstraintType::Linear)
	{
		throw std::runtime_error("Only linear constraint supports get_normalized_coefficient");
	}
	int row = _checked_constraint_index(constraint);
	int col = _checked_variable_index(variable);
	double retval;
	int error = mindopt::MDOgetcoeff(m_model.get(), row, col, &retval);
	check_error(error);
	return retval;
}

void MindoptModel::set_normalized_coefficient(const ConstraintIndex &constraint,
                                              const VariableIndex &variable, double value)
{
	if (constraint.type != ConstraintType::Linear)
	{
		throw std::runtime_error("Only linear constraint supports set_normalized_coefficient");
	}
	int row = _checked_constraint_index(constraint);
	int col = _checked_variable_index(variable);
	int error = mindopt::MDOchgcoeffs(m_model.get(), 1, &row, &col, &value);
	check_error(error);
}

double MindoptModel::get_objective_coefficient(const VariableIndex &variable)
{
	return get_variable_raw_attribute_double(variable, MDO_DBL_ATTR_OBJ);
}

void MindoptModel::set_objective_coefficient(const VariableIndex &variable, double value)
{
	set_variable_raw_attribute_double(variable, MDO_DBL_ATTR_OBJ, value);
}

void MindoptModel::computeIIS()
{
	int error = mindopt::MDOcomputeIIS(m_model.get());
	check_error(error);
}

int MindoptModel::_constraint_index(const ConstraintIndex &constraint)
{
	switch (constraint.type)
	{
	case ConstraintType::Linear:
		return m_linear_constraint_index.get_index(constraint.index);
	case ConstraintType::Quadratic:
		return m_quadratic_constraint_index.get_index(constraint.index);
	case ConstraintType::SOS:
		return m_sos_constraint_index.get_index(constraint.index);
	default:
		throw std::runtime_error("Unknown constraint type");
	}
}

int MindoptModel::_checked_constraint_index(const ConstraintIndex &constraint)
{
	int row = _constraint_index(constraint);
	if (row < 0)
	{
		throw std::runtime_error("Variable does not exist");
	}
	return row;
}

void *MindoptModel::get_raw_model()
{
	return m_model.get();
}

std::string MindoptModel::version_string()
{
	int major, minor, techinical;
	mindopt::MDOversion(&major, &minor, &techinical);
	std::string version = fmt::format("v{}.{}.{}", major, minor, techinical);
	return version;
}

MindoptEnv::MindoptEnv(bool empty)
{
	if (!mindopt::is_library_loaded())
	{
		throw std::runtime_error("Mindopt library is not loaded");
	}

	int error = 0;
	if (empty)
	{
		error = mindopt::MDOemptyenv(&m_env);
	}
	else
	{
		error = mindopt::MDOloadenv(&m_env, NULL);
	}
	check_error(error);
}

MindoptEnv::~MindoptEnv()
{
	mindopt::MDOfreeenv(m_env);
}

void MindoptEnv::start()
{
	int error = mindopt::MDOstartenv(m_env);
	check_error(error);
}
