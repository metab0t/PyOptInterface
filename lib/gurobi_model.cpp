#include "pyoptinterface/gurobi_model.hpp"
#include "fmt/core.h"

extern "C"
{
	int __stdcall GRBloadenvinternal(GRBenv **envP, const char *logfilename, int major, int minor,
	                                 int tech);
	int __stdcall GRBemptyenvinternal(GRBenv **envP, int major, int minor, int tech);
	int __stdcall GRBloadenv_1200(GRBenv **envP, const char *logfile);
	int __stdcall GRBemptyenv_1200(GRBenv **envP);
}

namespace gurobi
{
#define B DYLIB_DECLARE
APILIST
#undef B

// Gurobi 12.0.0 workarounds
DYLIB_DECLARE(GRBloadenvinternal);
DYLIB_DECLARE(GRBemptyenvinternal);

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

	// We have to deal with Gurobi 12.0.0 where some functions are absent from the dynamic library
	{
		int major, minor, techinical;
		auto GRBversion_p =
		    reinterpret_cast<decltype(GRBversion)>(_function_pointers["GRBversion"]);
		if (GRBversion_p != nullptr)
		{
			GRBversion_p(&major, &minor, &techinical);

			if (major == 12 && minor == 0 && techinical == 0)
			{
				DYLIB_LOAD_FUNCTION(GRBloadenvinternal);
				DYLIB_LOAD_FUNCTION(GRBemptyenvinternal);
				_function_pointers["GRBloadenv"] = reinterpret_cast<void *>(&GRBloadenv_1200);
				_function_pointers["GRBemptyenv"] = reinterpret_cast<void *>(&GRBemptyenv_1200);

				// Now check there is no nullptr in _function_pointers
				_load_success = true;
				for (auto &pair : _function_pointers)
				{
					if (pair.second == nullptr)
					{
						fmt::print("function {} is not loaded correctly\n", pair.first);
						_load_success = false;
					}
				}

				if (_load_success)
				{
					DYLIB_SAVE_FUNCTION(GRBloadenvinternal);
					DYLIB_SAVE_FUNCTION(GRBemptyenvinternal);
				}
			}
		}
	}

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
} // namespace gurobi

int GRBloadenv_1200(GRBenv **envP, const char *logfile)
{
	return gurobi::GRBloadenvinternal(envP, logfile, 12, 0, 0);
}

int GRBemptyenv_1200(GRBenv **envP)
{
	return gurobi::GRBemptyenvinternal(envP, 12, 0, 0);
}

static char gurobi_con_sense(ConstraintSense sense)
{
	switch (sense)
	{
	case ConstraintSense::LessEqual:
		return GRB_LESS_EQUAL;
	case ConstraintSense::Equal:
		return GRB_EQUAL;
	case ConstraintSense::GreaterEqual:
		return GRB_GREATER_EQUAL;
	default:
		throw std::runtime_error("Unknown constraint sense");
	}
}

static int gurobi_obj_sense(ObjectiveSense sense)
{
	switch (sense)
	{
	case ObjectiveSense::Minimize:
		return GRB_MINIMIZE;
	case ObjectiveSense::Maximize:
		return GRB_MAXIMIZE;
	default:
		throw std::runtime_error("Unknown objective sense");
	}
}

static char gurobi_vtype(VariableDomain domain)
{
	switch (domain)
	{
	case VariableDomain::Continuous:
		return GRB_CONTINUOUS;
	case VariableDomain::Integer:
		return GRB_INTEGER;
	case VariableDomain::Binary:
		return GRB_BINARY;
	case VariableDomain::SemiContinuous:
		return GRB_SEMICONT;
	default:
		throw std::runtime_error("Unknown variable domain");
	}
}

static int gurobi_sostype(SOSType type)
{
	switch (type)
	{
	case SOSType::SOS1:
		return GRB_SOS_TYPE1;
	case SOSType::SOS2:
		return GRB_SOS_TYPE2;
	default:
		throw std::runtime_error("Unknown SOS type");
	}
}

GurobiModel::GurobiModel(const GurobiEnv &env)
{
	init(env);
}

void GurobiModel::init(const GurobiEnv &env)
{
	if (!gurobi::is_library_loaded())
	{
		throw std::runtime_error("Gurobi library is not loaded");
	}

	GRBmodel *model;
	int error = gurobi::GRBnewmodel(env.m_env, &model, NULL, 0, NULL, NULL, NULL, NULL, NULL);
	check_error(error);
	m_env = gurobi::GRBgetenv(model);
	m_model = std::unique_ptr<GRBmodel, GRBfreemodelT>(model);
}

void GurobiModel::write(const std::string &filename)
{
	int error = gurobi::GRBwrite(m_model.get(), filename.c_str());
	check_error(error);
}

VariableIndex GurobiModel::add_variable(VariableDomain domain, double lb, double ub,
                                        const char *name)
{
	if (name != nullptr && name[0] == '\0')
	{
		name = nullptr;
	}
	IndexT index = m_variable_index.add_index();
	VariableIndex variable(index);

	// Create a new Gurobi variable
	char vtype = gurobi_vtype(domain);
	int error = gurobi::GRBaddvar(m_model.get(), 0, NULL, NULL, 0.0, lb, ub, vtype, name);
	check_error(error);

	m_update_flag |= m_variable_creation;

	return variable;
}

void GurobiModel::delete_variable(const VariableIndex &variable)
{
	if (!is_variable_active(variable))
	{
		throw std::runtime_error("Variable does not exist");
	}

	// Delete the corresponding Gurobi variable
	int variable_column = _variable_index(variable);
	int error = gurobi::GRBdelvars(m_model.get(), 1, &variable_column);
	check_error(error);

	m_variable_index.delete_index(variable.index);

	m_update_flag |= m_variable_deletion;
}

void GurobiModel::delete_variables(const Vector<VariableIndex> &variables)
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

	int error = gurobi::GRBdelvars(m_model.get(), columns.size(), columns.data());
	check_error(error);

	for (int i = 0; i < n_variables; i++)
	{
		m_variable_index.delete_index(variables[i].index);
	}

	m_update_flag |= m_variable_deletion;
}

bool GurobiModel::is_variable_active(const VariableIndex &variable)
{
	return m_variable_index.has_index(variable.index);
}

double GurobiModel::get_variable_value(const VariableIndex &variable)
{
	return get_variable_raw_attribute_double(variable, GRB_DBL_ATTR_X);
}

std::string GurobiModel::pprint_variable(const VariableIndex &variable)
{
	return get_variable_raw_attribute_string(variable, GRB_STR_ATTR_VARNAME);
}

void GurobiModel::set_variable_bounds(const VariableIndex &variable, double lb, double ub)
{
	auto column = _checked_variable_index(variable);
	int error;
	error = gurobi::GRBsetdblattrelement(m_model.get(), GRB_DBL_ATTR_LB, column, lb);
	check_error(error);
	error = gurobi::GRBsetdblattrelement(m_model.get(), GRB_DBL_ATTR_UB, column, ub);
	check_error(error);
	m_update_flag |= m_attribute_update;
}

void GurobiModel::set_variable_name(const VariableIndex &variable, const char *name)
{
	set_variable_raw_attribute_string(variable, GRB_STR_ATTR_VARNAME, name);
}

void GurobiModel::set_constraint_name(const ConstraintIndex &constraint, const char *name)
{
	const char *attr_name;
	switch (constraint.type)
	{
	case ConstraintType::Linear:
		attr_name = GRB_STR_ATTR_CONSTRNAME;
		break;
	case ConstraintType::Quadratic:
		attr_name = GRB_STR_ATTR_QCNAME;
		break;
	default:
		throw std::runtime_error("Unknown constraint type to set name!");
	}
	set_constraint_raw_attribute_string(constraint, attr_name, name);
}

ConstraintIndex GurobiModel::add_linear_constraint(const ScalarAffineFunction &function,
                                                   ConstraintSense sense, CoeffT rhs,
                                                   const char *name)
{
	IndexT index = m_linear_constraint_index.add_index();
	ConstraintIndex constraint_index(ConstraintType::Linear, index);

	// Create a new Gurobi linear constraint
	AffineFunctionPtrForm<int, int, double> ptr_form;
	ptr_form.make(this, function);

	int numnz = ptr_form.numnz;
	int *cind = ptr_form.index;
	double *cval = ptr_form.value;
	char g_sense = gurobi_con_sense(sense);
	double g_rhs = rhs - function.constant.value_or(0.0);

	if (name != nullptr && name[0] == '\0')
	{
		name = nullptr;
	}

	int error = gurobi::GRBaddconstr(m_model.get(), numnz, cind, cval, g_sense, g_rhs, name);
	check_error(error);
	// _require_update();

	m_update_flag |= m_linear_constraint_creation;

	return constraint_index;
}

ConstraintIndex GurobiModel::add_quadratic_constraint(const ScalarQuadraticFunction &function,
                                                      ConstraintSense sense, CoeffT rhs,
                                                      const char *name)
{
	IndexT index = m_quadratic_constraint_index.add_index();
	ConstraintIndex constraint_index(ConstraintType::Quadratic, index);

	// Create a new Gurobi quadratic constraint
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

	char g_sense = gurobi_con_sense(sense);
	double g_rhs = rhs;
	if (affine_part)
		g_rhs -= affine_part->constant.value_or(0.0);

	if (name != nullptr && name[0] == '\0')
	{
		name = nullptr;
	}

	int error = gurobi::GRBaddqconstr(m_model.get(), numlnz, lind, lval, numqnz, qrow, qcol, qval,
	                                  g_sense, g_rhs, name);
	check_error(error);

	m_update_flag |= m_quadratic_constraint_creation;

	return constraint_index;
}

ConstraintIndex GurobiModel::add_sos_constraint(const Vector<VariableIndex> &variables,
                                                SOSType sos_type)
{
	Vector<CoeffT> weights(variables.size(), 1.0);
	return add_sos_constraint(variables, sos_type, weights);
}

ConstraintIndex GurobiModel::add_sos_constraint(const Vector<VariableIndex> &variables,
                                                SOSType sos_type, const Vector<CoeffT> &weights)
{
	IndexT index = m_sos_constraint_index.add_index();
	ConstraintIndex constraint_index(ConstraintType::SOS, index);

	// Create a new Gurobi SOS constraint
	int numsos = 1;
	int nummembers = variables.size();
	int types = gurobi_sostype(sos_type);
	int beg[] = {0, nummembers};
	std::vector<int> ind_v(nummembers);
	for (int i = 0; i < nummembers; i++)
	{
		ind_v[i] = _variable_index(variables[i].index);
	}
	int *ind = ind_v.data();
	double *weight = (double *)weights.data();

	int error = gurobi::GRBaddsos(m_model.get(), numsos, nummembers, &types, beg, ind, weight);
	check_error(error);

	m_update_flag |= m_sos_constraint_creation;

	return constraint_index;
}

void GurobiModel::delete_constraint(const ConstraintIndex &constraint)
{
	// Delete the corresponding Gurobi constraint
	int error = 0;
	int constraint_row = _constraint_index(constraint);
	if (constraint_row >= 0)
	{
		switch (constraint.type)
		{
		case ConstraintType::Linear:
			m_linear_constraint_index.delete_index(constraint.index);
			error = gurobi::GRBdelconstrs(m_model.get(), 1, &constraint_row);
			m_update_flag |= m_linear_constraint_deletion;
			break;
		case ConstraintType::Quadratic:
			m_quadratic_constraint_index.delete_index(constraint.index);
			error = gurobi::GRBdelqconstrs(m_model.get(), 1, &constraint_row);
			m_update_flag |= m_quadratic_constraint_deletion;
			break;
		case ConstraintType::SOS:
			m_sos_constraint_index.delete_index(constraint.index);
			error = gurobi::GRBdelsos(m_model.get(), 1, &constraint_row);
			m_update_flag |= m_sos_constraint_deletion;
			break;
		default:
			throw std::runtime_error("Unknown constraint type");
		}
	}
	check_error(error);
}

bool GurobiModel::is_constraint_active(const ConstraintIndex &constraint)
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

void GurobiModel::_set_affine_objective(const ScalarAffineFunction &function, ObjectiveSense sense,
                                        bool clear_quadratic)
{
	int error = 0;
	if (clear_quadratic)
	{
		// First delete all quadratic terms
		error = gurobi::GRBdelq(m_model.get());
		check_error(error);
	}

	// Set Obj attribute of each variable
	int n_variables = get_model_raw_attribute_int(GRB_INT_ATTR_NUMVARS);
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

	error = gurobi::GRBsetdblattrarray(m_model.get(), "Obj", 0, n_variables, obj_v.data());
	check_error(error);
	error = gurobi::GRBsetdblattr(m_model.get(), "ObjCon", function.constant.value_or(0.0));
	check_error(error);

	int obj_sense = gurobi_obj_sense(sense);
	set_model_raw_attribute_int("ModelSense", obj_sense);

	m_update_flag |= m_objective_update;
}

void GurobiModel::set_objective(const ScalarAffineFunction &function, ObjectiveSense sense)
{
	_set_affine_objective(function, sense, true);
}

void GurobiModel::set_objective(const ScalarQuadraticFunction &function, ObjectiveSense sense)
{
	int error = 0;
	// First delete all quadratic terms
	error = gurobi::GRBdelq(m_model.get());
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

		error = gurobi::GRBaddqpterms(m_model.get(), numqnz, qrow, qcol, qval);
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

void GurobiModel::set_objective(const ExprBuilder &function, ObjectiveSense sense)
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

void GurobiModel::optimize()
{
	if (has_callback)
	{
		// Store the number of variables for the callback
		m_callback_userdata.n_variables = get_model_raw_attribute_int(GRB_INT_ATTR_NUMVARS);
	}
	m_update_flag = 0;
	int error = gurobi::GRBoptimize(m_model.get());
	check_error(error);
}

void GurobiModel::update()
{
	int error = gurobi::GRBupdatemodel(m_model.get());
	check_error(error);
	m_update_flag = 0;
}

int GurobiModel::raw_parameter_type(const char *param_name)
{
	return gurobi::GRBgetparamtype(m_env, param_name);
}

void GurobiModel::set_raw_parameter_int(const char *param_name, int value)
{
	int error = gurobi::GRBsetintparam(m_env, param_name, value);
	check_error(error);
}

void GurobiModel::set_raw_parameter_double(const char *param_name, double value)
{
	int error = gurobi::GRBsetdblparam(m_env, param_name, value);
	check_error(error);
}

void GurobiModel::set_raw_parameter_string(const char *param_name, const char *value)
{
	int error = gurobi::GRBsetstrparam(m_env, param_name, value);
	check_error(error);
}

int GurobiModel::get_raw_parameter_int(const char *param_name)
{
	int retval;
	int error = gurobi::GRBgetintparam(m_env, param_name, &retval);
	check_error(error);
	return retval;
}

double GurobiModel::get_raw_parameter_double(const char *param_name)
{
	double retval;
	int error = gurobi::GRBgetdblparam(m_env, param_name, &retval);
	check_error(error);
	return retval;
}

std::string GurobiModel::get_raw_parameter_string(const char *param_name)
{
	char retval[GRB_MAX_STRLEN];
	int error = gurobi::GRBgetstrparam(m_env, param_name, retval);
	check_error(error);
	return std::string(retval);
}

int GurobiModel::raw_attribute_type(const char *attr_name)
{
	int datatypeP;
	int error = gurobi::GRBgetattrinfo(m_model.get(), attr_name, &datatypeP, NULL, NULL);
	check_error(error);
	return datatypeP;
}

void GurobiModel::set_model_raw_attribute_int(const char *attr_name, int value)
{
	int error = gurobi::GRBsetintattr(m_model.get(), attr_name, value);
	check_error(error);
	m_update_flag |= m_attribute_update;
}

void GurobiModel::set_model_raw_attribute_double(const char *attr_name, double value)
{
	int error = gurobi::GRBsetdblattr(m_model.get(), attr_name, value);
	check_error(error);
	m_update_flag |= m_attribute_update;
}

void GurobiModel::set_model_raw_attribute_string(const char *attr_name, const char *value)
{
	int error = gurobi::GRBsetstrattr(m_model.get(), attr_name, value);
	check_error(error);
	m_update_flag |= m_attribute_update;
}

int GurobiModel::get_model_raw_attribute_int(const char *attr_name)
{
	_update_for_information();
	int retval;
	int error = gurobi::GRBgetintattr(m_model.get(), attr_name, &retval);
	check_error(error);
	return retval;
}

double GurobiModel::get_model_raw_attribute_double(const char *attr_name)
{
	_update_for_information();
	double retval;
	int error = gurobi::GRBgetdblattr(m_model.get(), attr_name, &retval);
	check_error(error);
	return retval;
}

std::string GurobiModel::get_model_raw_attribute_string(const char *attr_name)
{
	_update_for_information();
	char *retval;
	int error = gurobi::GRBgetstrattr(m_model.get(), attr_name, &retval);
	check_error(error);
	return std::string(retval);
}

std::vector<double> GurobiModel::get_model_raw_attribute_vector_double(const char *attr_name,
                                                                       int start, int len)
{
	_update_for_information();
	std::vector<double> retval(len);
	int error = gurobi::GRBgetdblattrarray(m_model.get(), attr_name, start, len, retval.data());
	check_error(error);
	return retval;
}

std::vector<double> GurobiModel::get_model_raw_attribute_list_double(const char *attr_name,
                                                                     const std::vector<int> &ind)
{
	_update_for_information();
	std::vector<double> retval(ind.size());
	int error = gurobi::GRBgetdblattrlist(m_model.get(), attr_name, ind.size(), (int *)ind.data(),
	                                      retval.data());
	check_error(error);
	return retval;
}

void GurobiModel::set_variable_raw_attribute_int(const VariableIndex &variable,
                                                 const char *attr_name, int value)
{
	auto column = _checked_variable_index(variable);
	int error = gurobi::GRBsetintattrelement(m_model.get(), attr_name, column, value);
	check_error(error);
	m_update_flag |= m_attribute_update;
}

void GurobiModel::set_variable_raw_attribute_char(const VariableIndex &variable,
                                                  const char *attr_name, char value)
{
	auto column = _checked_variable_index(variable);
	int error = gurobi::GRBsetcharattrelement(m_model.get(), attr_name, column, value);
	check_error(error);
	m_update_flag |= m_attribute_update;
}

void GurobiModel::set_variable_raw_attribute_double(const VariableIndex &variable,
                                                    const char *attr_name, double value)
{
	auto column = _checked_variable_index(variable);
	int error = gurobi::GRBsetdblattrelement(m_model.get(), attr_name, column, value);
	check_error(error);
	m_update_flag |= m_attribute_update;
}

void GurobiModel::set_variable_raw_attribute_string(const VariableIndex &variable,
                                                    const char *attr_name, const char *value)
{
	auto column = _checked_variable_index(variable);
	int error = gurobi::GRBsetstrattrelement(m_model.get(), attr_name, column, value);
	check_error(error);
	m_update_flag |= m_attribute_update;
}

int GurobiModel::get_variable_raw_attribute_int(const VariableIndex &variable,
                                                const char *attr_name)
{
	_update_for_information();
	auto column = _checked_variable_index(variable);
	int retval;
	int error = gurobi::GRBgetintattrelement(m_model.get(), attr_name, column, &retval);
	check_error(error);
	return retval;
}

char GurobiModel::get_variable_raw_attribute_char(const VariableIndex &variable,
                                                  const char *attr_name)
{
	_update_for_information();
	auto column = _checked_variable_index(variable);
	char retval;
	int error = gurobi::GRBgetcharattrelement(m_model.get(), attr_name, column, &retval);
	check_error(error);
	return retval;
}

double GurobiModel::get_variable_raw_attribute_double(const VariableIndex &variable,
                                                      const char *attr_name)
{
	_update_for_information();
	auto column = _checked_variable_index(variable);
	double retval;
	int error = gurobi::GRBgetdblattrelement(m_model.get(), attr_name, column, &retval);
	check_error(error);
	return retval;
}

std::string GurobiModel::get_variable_raw_attribute_string(const VariableIndex &variable,
                                                           const char *attr_name)
{
	_update_for_information();
	auto column = _checked_variable_index(variable);
	char *retval;
	int error = gurobi::GRBgetstrattrelement(m_model.get(), attr_name, column, &retval);
	check_error(error);
	return std::string(retval);
}

int GurobiModel::_variable_index(const VariableIndex &variable)
{
	_update_for_variable_index();
	return m_variable_index.get_index(variable.index);
}

int GurobiModel::_checked_variable_index(const VariableIndex &variable)
{
	int column = _variable_index(variable);
	if (column < 0)
	{
		throw std::runtime_error("Variable does not exist");
	}
	return column;
}

void GurobiModel::set_constraint_raw_attribute_int(const ConstraintIndex &constraint,
                                                   const char *attr_name, int value)
{
	int row = _checked_constraint_index(constraint);
	int error = gurobi::GRBsetintattrelement(m_model.get(), attr_name, row, value);
	check_error(error);
	m_update_flag |= m_attribute_update;
}

void GurobiModel::set_constraint_raw_attribute_char(const ConstraintIndex &constraint,
                                                    const char *attr_name, char value)
{
	int row = _checked_constraint_index(constraint);
	int error = gurobi::GRBsetcharattrelement(m_model.get(), attr_name, row, value);
	check_error(error);
	m_update_flag |= m_attribute_update;
}

void GurobiModel::set_constraint_raw_attribute_double(const ConstraintIndex &constraint,
                                                      const char *attr_name, double value)
{
	int row = _checked_constraint_index(constraint);
	int error = gurobi::GRBsetdblattrelement(m_model.get(), attr_name, row, value);
	check_error(error);
	m_update_flag |= m_attribute_update;
}

void GurobiModel::set_constraint_raw_attribute_string(const ConstraintIndex &constraint,
                                                      const char *attr_name, const char *value)
{
	int row = _checked_constraint_index(constraint);
	int error = gurobi::GRBsetstrattrelement(m_model.get(), attr_name, row, value);
	check_error(error);
	m_update_flag |= m_attribute_update;
}

int GurobiModel::get_constraint_raw_attribute_int(const ConstraintIndex &constraint,
                                                  const char *attr_name)
{
	_update_for_information();
	int row = _checked_constraint_index(constraint);
	int retval;
	int error = gurobi::GRBgetintattrelement(m_model.get(), attr_name, row, &retval);
	check_error(error);
	return retval;
}

char GurobiModel::get_constraint_raw_attribute_char(const ConstraintIndex &constraint,
                                                    const char *attr_name)
{
	_update_for_information();
	int row = _checked_constraint_index(constraint);
	char retval;
	int error = gurobi::GRBgetcharattrelement(m_model.get(), attr_name, row, &retval);
	check_error(error);
	return retval;
}

double GurobiModel::get_constraint_raw_attribute_double(const ConstraintIndex &constraint,
                                                        const char *attr_name)
{
	_update_for_information();
	int row = _checked_constraint_index(constraint);
	double retval;
	int error = gurobi::GRBgetdblattrelement(m_model.get(), attr_name, row, &retval);
	check_error(error);
	return retval;
}

std::string GurobiModel::get_constraint_raw_attribute_string(const ConstraintIndex &constraint,
                                                             const char *attr_name)
{
	_update_for_information();
	int row = _checked_constraint_index(constraint);
	char *retval;
	int error = gurobi::GRBgetstrattrelement(m_model.get(), attr_name, row, &retval);
	check_error(error);
	return std::string(retval);
}

double GurobiModel::get_normalized_rhs(const ConstraintIndex &constraint)
{
	const char *name;
	switch (constraint.type)
	{
	case ConstraintType::Linear:
		name = GRB_DBL_ATTR_RHS;
		break;
	case ConstraintType::Quadratic:
		name = GRB_DBL_ATTR_QCRHS;
		break;
	default:
		throw std::runtime_error("Unknown constraint type to get_normalized_rhs");
	}
	return get_constraint_raw_attribute_double(constraint, name);
}

void GurobiModel::set_normalized_rhs(const ConstraintIndex &constraint, double value)
{
	const char *name;
	switch (constraint.type)
	{
	case ConstraintType::Linear:
		name = GRB_DBL_ATTR_RHS;
		break;
	case ConstraintType::Quadratic:
		name = GRB_DBL_ATTR_QCRHS;
		break;
	default:
		throw std::runtime_error("Unknown constraint type to set_normalized_rhs");
	}
	set_constraint_raw_attribute_double(constraint, name, value);
}

double GurobiModel::get_normalized_coefficient(const ConstraintIndex &constraint,
                                               const VariableIndex &variable)
{
	if (constraint.type != ConstraintType::Linear)
	{
		throw std::runtime_error("Only linear constraint supports get_normalized_coefficient");
	}
	_update_for_information();
	int row = _checked_constraint_index(constraint);
	int col = _checked_variable_index(variable);
	double retval;
	int error = gurobi::GRBgetcoeff(m_model.get(), row, col, &retval);
	check_error(error);
	return retval;
}

void GurobiModel::set_normalized_coefficient(const ConstraintIndex &constraint,
                                             const VariableIndex &variable, double value)
{
	if (constraint.type != ConstraintType::Linear)
	{
		throw std::runtime_error("Only linear constraint supports set_normalized_coefficient");
	}
	int row = _checked_constraint_index(constraint);
	int col = _checked_variable_index(variable);
	int error = gurobi::GRBchgcoeffs(m_model.get(), 1, &row, &col, &value);
	check_error(error);
	m_update_flag |= m_constraint_coefficient_update;
}

double GurobiModel::get_objective_coefficient(const VariableIndex &variable)
{
	return get_variable_raw_attribute_double(variable, GRB_DBL_ATTR_OBJ);
}

void GurobiModel::set_objective_coefficient(const VariableIndex &variable, double value)
{
	set_variable_raw_attribute_double(variable, GRB_DBL_ATTR_OBJ, value);
}

int GurobiModel::_constraint_index(const ConstraintIndex &constraint)
{
	_update_for_constraint_index(constraint.type);
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

int GurobiModel::_checked_constraint_index(const ConstraintIndex &constraint)
{
	int row = _constraint_index(constraint);
	if (row < 0)
	{
		throw std::runtime_error("Variable does not exist");
	}
	return row;
}

void GurobiModel::check_error(int error)
{
	if (error)
	{
		throw std::runtime_error(gurobi::GRBgeterrormsg(m_env));
	}
}

void GurobiModel::_update_for_information()
{
	if (m_update_flag)
	{
		update();
	}
}

void GurobiModel::_update_for_variable_index()
{
	if (m_update_flag & m_variable_deletion)
	{
		update();
	}
}

void GurobiModel::_update_for_constraint_index(ConstraintType type)
{
	bool need_update = false;
	switch (type)
	{
	case ConstraintType::Linear:
		// Adding new linear constraints does not need to update the model
		need_update = m_update_flag & m_linear_constraint_deletion;
		break;
	case ConstraintType::Quadratic:
		need_update =
		    m_update_flag & (m_quadratic_constraint_creation | m_quadratic_constraint_deletion);
		break;
	case ConstraintType::SOS:
		need_update = m_update_flag & (m_sos_constraint_creation | m_sos_constraint_deletion);
		break;
	}
	if (need_update)
	{
		update();
	}
}

void *GurobiModel::get_raw_model()
{
	return m_model.get();
}

std::string GurobiModel::version_string()
{
	int major, minor, techinical;
	gurobi::GRBversion(&major, &minor, &techinical);
	std::string version = fmt::format("v{}.{}.{}", major, minor, techinical);
	return version;
}

GurobiEnv::GurobiEnv(bool empty)
{
	if (!gurobi::is_library_loaded())
	{
		throw std::runtime_error("Gurobi library is not loaded");
	}

	int error = 0;
	if (empty)
	{
		error = gurobi::GRBemptyenv(&m_env);
	}
	else
	{
		error = gurobi::GRBloadenv(&m_env, NULL);
	}
	check_error(error);
}

GurobiEnv::~GurobiEnv()
{
	gurobi::GRBfreeenv(m_env);
}

int GurobiEnv::raw_parameter_type(const char *param_name)
{
	return gurobi::GRBgetparamtype(m_env, param_name);
}

void GurobiEnv::set_raw_parameter_int(const char *param_name, int value)
{
	int error = gurobi::GRBsetintparam(m_env, param_name, value);
	check_error(error);
}

void GurobiEnv::set_raw_parameter_double(const char *param_name, double value)
{
	int error = gurobi::GRBsetdblparam(m_env, param_name, value);
	check_error(error);
}

void GurobiEnv::set_raw_parameter_string(const char *param_name, const char *value)
{
	int error = gurobi::GRBsetstrparam(m_env, param_name, value);
	check_error(error);
}

void GurobiEnv::start()
{
	int error = gurobi::GRBstartenv(m_env);
	check_error(error);
}

void GurobiEnv::check_error(int error)
{
	if (error)
	{
		throw std::runtime_error(gurobi::GRBgeterrormsg(m_env));
	}
}

// Callback
int RealGurobiCallbackFunction(GRBmodel *, void *cbdata, int where, void *usrdata)
{
	auto real_userdata = static_cast<GurobiCallbackUserdata *>(usrdata);
	auto model = static_cast<GurobiModelMixin *>(real_userdata->model);
	auto &callback = real_userdata->callback;

	model->m_cbdata = cbdata;
	model->m_callback_userdata.where = where;
	model->m_callback_userdata.cb_get_mipsol_called = false;
	model->m_callback_userdata.cb_get_mipnoderel_called = false;
	model->m_callback_userdata.cb_set_solution_called = false;
	model->m_callback_userdata.cb_requires_submit_solution = false;
	callback(model, where);

	if (model->m_callback_userdata.cb_requires_submit_solution)
	{
		model->cb_submit_solution();
	}

	return 0;
}

void GurobiModel::set_callback(const GurobiCallback &callback)
{
	m_callback_userdata.model = this;
	m_callback_userdata.callback = callback;

	int error =
	    gurobi::GRBsetcallbackfunc(m_model.get(), RealGurobiCallbackFunction, &m_callback_userdata);
	check_error(error);

	has_callback = true;
}

int GurobiModel::cb_get_info_int(int what)
{
	int retval;
	int error = gurobi::GRBcbget(m_cbdata, m_callback_userdata.where, what, &retval);
	check_error(error);
	return retval;
}

double GurobiModel::cb_get_info_double(int what)
{
	double retval;
	int error = gurobi::GRBcbget(m_cbdata, m_callback_userdata.where, what, &retval);
	check_error(error);
	return retval;
}

void GurobiModel::cb_get_info_doublearray(int what)
{
	int n_vars = m_callback_userdata.n_variables;
	double *val = nullptr;
	if (what == GRB_CB_MIPSOL_SOL)
	{
		m_callback_userdata.mipsol.resize(n_vars);
		val = m_callback_userdata.mipsol.data();
	}
	else if (what == GRB_CB_MIPNODE_REL)
	{
		m_callback_userdata.mipnoderel.resize(n_vars);
		val = m_callback_userdata.mipnoderel.data();
	}
	else
	{
		throw std::runtime_error("Invalid what for cb_get_info_doublearray");
	}
	int error = gurobi::GRBcbget(m_cbdata, m_callback_userdata.where, what, val);
	check_error(error);
}

double GurobiModel::cb_get_solution(const VariableIndex &variable)
{
	auto &userdata = m_callback_userdata;
	if (!userdata.cb_get_mipsol_called)
	{
		cb_get_info_doublearray(GRB_CB_MIPSOL_SOL);
		userdata.cb_get_mipsol_called = true;
	}
	auto index = _variable_index(variable);
	return userdata.mipsol[index];
}

double GurobiModel::cb_get_relaxation(const VariableIndex &variable)
{
	auto &userdata = m_callback_userdata;
	if (!userdata.cb_get_mipnoderel_called)
	{
		cb_get_info_doublearray(GRB_CB_MIPNODE_REL);
		userdata.cb_get_mipnoderel_called = true;
	}
	auto index = _variable_index(variable);
	return userdata.mipnoderel[index];
}

void GurobiModel::cb_set_solution(const VariableIndex &variable, double value)
{
	auto &userdata = m_callback_userdata;
	if (!userdata.cb_set_solution_called)
	{
		userdata.heuristic_solution.resize(userdata.n_variables, GRB_UNDEFINED);
		userdata.cb_set_solution_called = true;
	}
	userdata.heuristic_solution[_variable_index(variable)] = value;
	m_callback_userdata.cb_requires_submit_solution = true;
}

double GurobiModel::cb_submit_solution()
{
	if (!m_callback_userdata.cb_set_solution_called)
	{
		throw std::runtime_error("No solution is set in the callback!");
	}
	double obj;
	int error =
	    gurobi::GRBcbsolution(m_cbdata, m_callback_userdata.heuristic_solution.data(), &obj);
	check_error(error);
	m_callback_userdata.cb_requires_submit_solution = false;
	return obj;
}

void GurobiModel::cb_add_lazy_constraint(const ScalarAffineFunction &function,
                                         ConstraintSense sense, CoeffT rhs)
{
	AffineFunctionPtrForm<int, int, double> ptr_form;
	ptr_form.make(this, function);

	int numnz = ptr_form.numnz;
	int *cind = ptr_form.index;
	double *cval = ptr_form.value;
	char g_sense = gurobi_con_sense(sense);
	double g_rhs = rhs - function.constant.value_or(0.0);

	int error = gurobi::GRBcblazy(m_cbdata, numnz, cind, cval, g_sense, g_rhs);
	check_error(error);
}

void GurobiModel::cb_exit()
{
	gurobi::GRBterminate(m_model.get());
}

void GurobiModel::cb_add_lazy_constraint(const ExprBuilder &function, ConstraintSense sense,
                                         CoeffT rhs)
{
	ScalarAffineFunction f(function);
	cb_add_lazy_constraint(f, sense, rhs);
}

void GurobiModel::cb_add_user_cut(const ScalarAffineFunction &function, ConstraintSense sense,
                                  CoeffT rhs)
{
	AffineFunctionPtrForm<int, int, double> ptr_form;
	ptr_form.make(this, function);

	int numnz = ptr_form.numnz;
	int *cind = ptr_form.index;
	double *cval = ptr_form.value;
	char g_sense = gurobi_con_sense(sense);
	double g_rhs = rhs - function.constant.value_or(0.0);

	int error = gurobi::GRBcbcut(m_cbdata, numnz, cind, cval, g_sense, g_rhs);
	check_error(error);
}

void GurobiModel::cb_add_user_cut(const ExprBuilder &function, ConstraintSense sense, CoeffT rhs)
{
	ScalarAffineFunction f(function);
	cb_add_user_cut(f, sense, rhs);
}
