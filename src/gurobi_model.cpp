#include "pyoptinterface/gurobi_model.hpp"
#include <format>

char gurobi_con_sense(ConstraintSense sense)
{
	using enum ConstraintSense;
	switch (sense)
	{
	case LessEqual:
		return GRB_LESS_EQUAL;
	case Equal:
		return GRB_EQUAL;
	case GreaterEqual:
		return GRB_GREATER_EQUAL;
	default:
		throw std::runtime_error("Unknown constraint sense");
	}
}

int gurobi_obj_sense(ObjectiveSense sense)
{
	using enum ObjectiveSense;
	switch (sense)
	{
	case Minimize:
		return GRB_MINIMIZE;
	case Maximize:
		return GRB_MAXIMIZE;
	default:
		throw std::runtime_error("Unknown objective sense");
	}
}

char gurobi_vtype(VariableDomain domain)
{
	using enum VariableDomain;
	switch (domain)
	{
	case Continuous:
		return GRB_CONTINUOUS;
	case Integer:
		return GRB_INTEGER;
	case Binary:
		return GRB_BINARY;
	case SemiContinuous:
		return GRB_SEMICONT;
	default:
		throw std::runtime_error("Unknown variable domain");
	}
}

ConstraintType gurobi_sostype(int type)
{
	switch (type)
	{
	case GRB_SOS_TYPE1:
		return ConstraintType::SOS1;
	case GRB_SOS_TYPE2:
		return ConstraintType::SOS2;
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
	GRBmodel *model;
	int error = GRBnewmodel(env.m_env, &model, NULL, 0, NULL, NULL, NULL, NULL, NULL);
	check_error(error);
	m_env = GRBgetenv(model);
	m_model = std::unique_ptr<GRBmodel, GRBfreemodelT>(model);
}

VariableIndex GurobiModel::add_variable(VariableDomain domain)
{
	IndexT index = m_variable_index.add_index();
	VariableIndex variable(index);

	// Create a new Gurobi variable
	char vtype = gurobi_vtype(domain);
	int error =
	    GRBaddvar(m_model.get(), 0, NULL, NULL, 0.0, -GRB_INFINITY, GRB_INFINITY, vtype, NULL);
	check_error(error);

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
	int error = GRBdelvars(m_model.get(), 1, &variable_column);
	check_error(error);

	m_variable_index.delete_index(variable.index);
}

bool GurobiModel::is_variable_active(const VariableIndex &variable)
{
	return m_variable_index.has_index(variable.index);
}

ConstraintIndex GurobiModel::add_linear_constraint(const ScalarAffineFunction function,
                                                   ConstraintSense sense, CoeffT rhs)
{
	IndexT index = m_linear_constraint_index.add_index();
	ConstraintIndex constraint_index(ConstraintType::Linear, index);

	// Create a new Gurobi linear constraint
	int numnz = function.size();
	std::vector<int> cind_v(numnz);
	std::vector<double> cval_v(numnz);
	for (int i = 0; i < numnz; i++)
	{
		cind_v[i] = _variable_index(function.variables[i]);
		cval_v[i] = function.coefficients[i];
	}
	int *cind = cind_v.data();
	double *cval = cval_v.data();
	char g_sense = gurobi_con_sense(sense);
	double g_rhs = rhs - function.constant.value_or(0.0);

	int error = GRBaddconstr(m_model.get(), numnz, cind, cval, g_sense, g_rhs, NULL);
	check_error(error);
	return constraint_index;
}

ConstraintIndex GurobiModel::add_quadratic_constraint(const ScalarQuadraticFunction function,
                                                      ConstraintSense sense, CoeffT rhs)
{
	IndexT index = m_quadratic_constraint_index.add_index();
	ConstraintIndex constraint_index(ConstraintType::Quadratic, index);

	// Create a new Gurobi quadratic constraint
	const auto &affine_part = function.affine_part;

	int numlnz = 0;
	int *lind = NULL;
	double *lval = NULL;
	if (affine_part.has_value())
	{
		const auto &affine_function = affine_part.value();
		numlnz = affine_function.coefficients.size();
		std::vector<int> lind_v(numlnz);
		std::vector<double> lval_v(numlnz);
		for (int i = 0; i < numlnz; i++)
		{
			lind_v[i] = _variable_index(affine_function.variables[i]);
			lval_v[i] = function.coefficients[i];
		}
		lind = lind_v.data();
		lval = lval_v.data();
	}

	int numqnz = function.size();
	std::vector<int> qrow_v(numqnz);
	std::vector<int> qcol_v(numqnz);
	std::vector<double> qval_v(numqnz);
	for (int i = 0; i < numqnz; i++)
	{
		qrow_v[i] = _variable_index(function.variable_1s[i]);
		qcol_v[i] = _variable_index(function.variable_2s[i]);
		qval_v[i] = function.coefficients[i];
	}
	int *qrow = qrow_v.data();
	int *qcol = qcol_v.data();
	double *qval = qval_v.data();

	char g_sense = gurobi_con_sense(sense);
	double g_rhs = rhs;
	if (affine_part)
		g_rhs -= affine_part->constant.value_or(0.0);

	int error = GRBaddqconstr(m_model.get(), numlnz, lind, lval, numqnz, qrow, qcol, qval, g_sense,
	                          g_rhs, NULL);
	check_error(error);
	return constraint_index;
}

ConstraintIndex GurobiModel::add_sos1_constraint(const Vector<VariableIndex> &variables,
                                                 const Vector<CoeffT> &weights)
{
	return _add_sos_constraint(variables, weights, GRB_SOS_TYPE1);
}

ConstraintIndex GurobiModel::add_sos2_constraint(const Vector<VariableIndex> &variables,
                                                 const Vector<CoeffT> &weights)
{
	return _add_sos_constraint(variables, weights, GRB_SOS_TYPE2);
}

ConstraintIndex GurobiModel::_add_sos_constraint(const Vector<VariableIndex> &variables,
                                                 const Vector<CoeffT> &weights, int sos_type)
{
	IndexT index = m_sos_constraint_index.add_index();
	ConstraintIndex constraint_index(gurobi_sostype(sos_type), index);

	// Create a new Gurobi SOS constraint
	int numsos = 1;
	int nummembers = variables.size();
	int types = sos_type;
	int beg[] = {0, nummembers};
	std::vector<int> ind_v(nummembers);
	std::vector<double> weight_v(nummembers);
	for (int i = 0; i < nummembers; i++)
	{
		ind_v[i] = _variable_index(variables[i].index);
		weight_v[i] = weights[i];
	}
	int *ind = ind_v.data();
	double *weight = weight_v.data();

	int error = GRBaddsos(m_model.get(), numsos, nummembers, &types, beg, ind, weight);
	check_error(error);
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
			error = GRBdelconstrs(m_model.get(), 1, &constraint_row);
			break;
		case ConstraintType::Quadratic:
			m_quadratic_constraint_index.delete_index(constraint.index);
			error = GRBdelqconstrs(m_model.get(), 1, &constraint_row);
			break;
		case ConstraintType::SOS1:
		case ConstraintType::SOS2:
			m_sos_constraint_index.delete_index(constraint.index);
			error = GRBdelsos(m_model.get(), 1, &constraint_row);
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
	case ConstraintType::SOS1:
	case ConstraintType::SOS2:
		return m_sos_constraint_index.has_index(constraint.index);
	default:
		throw std::runtime_error("Unknown constraint type");
	}
}

void GurobiModel::set_objective(const ScalarAffineFunction &function, ObjectiveSense sense)
{
	// Set Obj attribute of each variable
	int n_variables = m_variable_index.num_active_indices();
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

	int error;
	error = GRBsetdblattrarray(m_model.get(), "Obj", 0, n_variables, obj_v.data());
	check_error(error);
	error = GRBsetdblattr(m_model.get(), "ObjCon", function.constant.value_or(0.0));
	check_error(error);

	int obj_sense = gurobi_obj_sense(sense);
	error = GRBsetintattr(m_model.get(), "ModelSense", obj_sense);
	check_error(error);
}

void GurobiModel::set_objective(const ScalarQuadraticFunction &function, ObjectiveSense sense)
{
	int error = 0;
	// First delete all quadratic terms
	error = GRBdelq(m_model.get());
	check_error(error);

	// Add quadratic term
	int numqnz = function.size();
	if (numqnz > 0)
	{
		std::vector<int> qrow_v(numqnz);
		std::vector<int> qcol_v(numqnz);
		std::vector<double> qval_v(numqnz);
		for (int i = 0; i < numqnz; i++)
		{
			qrow_v[i] = _variable_index(function.variable_1s[i]);
			qcol_v[i] = _variable_index(function.variable_2s[i]);
			qval_v[i] = function.coefficients[i];
		}
		int *qrow = qrow_v.data();
		int *qcol = qcol_v.data();
		double *qval = qval_v.data();

		error = GRBaddqpterms(m_model.get(), numqnz, qrow, qcol, qval);
		check_error(error);
	}

	// Affine part
	const auto &affine_part = function.affine_part;
	if (affine_part)
	{
		const auto &affine_function = affine_part.value();
		set_objective(affine_function, sense);
	}
	else
	{
		ScalarAffineFunction zero;
		set_objective(zero, sense);
	}
}

void GurobiModel::optimize()
{
	int error = GRBoptimize(m_model.get());
	check_error(error);
}

void GurobiModel::update()
{
	int error = GRBupdatemodel(m_model.get());
	check_error(error);
}

int GurobiModel::raw_parameter_type(const char *param_name)
{
	return GRBgetparamtype(m_env, param_name);
}

void GurobiModel::set_parameter_int(const char *param_name, int value)
{
	int error = GRBsetintparam(m_env, param_name, value);
	check_error(error);
}

void GurobiModel::set_parameter_double(const char *param_name, double value)
{
	int error = GRBsetdblparam(m_env, param_name, value);
	check_error(error);
}

void GurobiModel::set_parameter_string(const char *param_name, const char *value)
{
	int error = GRBsetstrparam(m_env, param_name, value);
	check_error(error);
}

int GurobiModel::get_parameter_int(const char *param_name)
{
	int retval;
	int error = GRBgetintparam(m_env, param_name, &retval);
	check_error(error);
	return retval;
}

double GurobiModel::get_parameter_double(const char *param_name)
{
	double retval;
	int error = GRBgetdblparam(m_env, param_name, &retval);
	check_error(error);
	return retval;
}

std::string GurobiModel::get_parameter_string(const char *param_name)
{
	char retval[GRB_MAX_STRLEN];
	int error = GRBgetstrparam(m_env, param_name, retval);
	check_error(error);
	return std::string(retval);
}

int GurobiModel::raw_attribute_type(const char *attr_name)
{
	int datatypeP;
	int error = GRBgetattrinfo(m_model.get(), attr_name, &datatypeP, NULL, NULL);
	check_error(error);
	return datatypeP;
}

void GurobiModel::set_model_raw_attribute_int(const char *attr_name, int value)
{
	int error = GRBsetintattr(m_model.get(), attr_name, value);
	check_error(error);
}

void GurobiModel::set_model_raw_attribute_double(const char *attr_name, double value)
{
	int error = GRBsetdblattr(m_model.get(), attr_name, value);
	check_error(error);
}

void GurobiModel::set_model_raw_attribute_string(const char *attr_name, const char *value)
{
	int error = GRBsetstrattr(m_model.get(), attr_name, value);
	check_error(error);
}

int GurobiModel::get_model_raw_attribute_int(const char *attr_name)
{
	int retval;
	int error = GRBgetintattr(m_model.get(), attr_name, &retval);
	check_error(error);
	return retval;
}

double GurobiModel::get_model_raw_attribute_double(const char *attr_name)
{
	double retval;
	int error = GRBgetdblattr(m_model.get(), attr_name, &retval);
	check_error(error);
	return retval;
}

std::string GurobiModel::get_model_raw_attribute_string(const char *attr_name)
{
	char *retval;
	int error = GRBgetstrattr(m_model.get(), attr_name, &retval);
	check_error(error);
	return std::string(retval);
}

std::vector<double> GurobiModel::get_model_raw_attribute_vector_double(const char *attr_name,
                                                                       int start, int len)
{
	std::vector<double> retval(len);
	int error = GRBgetdblattrarray(m_model.get(), attr_name, start, len, retval.data());
	check_error(error);
	return retval;
}

std::vector<double> GurobiModel::get_model_raw_attribute_list_double(const char *attr_name,
                                                                     const std::vector<int> &ind)
{
	std::vector<double> retval(ind.size());
	int error =
	    GRBgetdblattrlist(m_model.get(), attr_name, ind.size(), (int *)ind.data(), retval.data());
	check_error(error);
	return retval;
}

void GurobiModel::set_variable_raw_attribute_int(const VariableIndex &variable,
                                                 const char *attr_name, int value)
{
	auto column = _checked_variable_index(variable);
	int error = GRBsetintattrelement(m_model.get(), attr_name, column, value);
	check_error(error);
}

void GurobiModel::set_variable_raw_attribute_char(const VariableIndex &variable,
                                                  const char *attr_name, char value)
{
	auto column = _checked_variable_index(variable);
	int error = GRBsetcharattrelement(m_model.get(), attr_name, column, value);
	check_error(error);
}

void GurobiModel::set_variable_raw_attribute_double(const VariableIndex &variable,
                                                    const char *attr_name, double value)
{
	auto column = _checked_variable_index(variable);
	int error = GRBsetdblattrelement(m_model.get(), attr_name, column, value);
	check_error(error);
}

void GurobiModel::set_variable_raw_attribute_string(const VariableIndex &variable,
                                                    const char *attr_name, const char *value)
{
	auto column = _checked_variable_index(variable);
	int error = GRBsetstrattrelement(m_model.get(), attr_name, column, value);
	check_error(error);
}

int GurobiModel::get_variable_raw_attribute_int(const VariableIndex &variable,
                                                const char *attr_name)
{
	auto column = _checked_variable_index(variable);
	int retval;
	int error = GRBgetintattrelement(m_model.get(), attr_name, column, &retval);
	check_error(error);
	return retval;
}

char GurobiModel::get_variable_raw_attribute_char(const VariableIndex &variable,
                                                  const char *attr_name)
{
	auto column = _checked_variable_index(variable);
	char retval;
	int error = GRBgetcharattrelement(m_model.get(), attr_name, column, &retval);
	check_error(error);
	return retval;
}

double GurobiModel::get_variable_raw_attribute_double(const VariableIndex &variable,
                                                      const char *attr_name)
{
	auto column = _checked_variable_index(variable);
	double retval;
	int error = GRBgetdblattrelement(m_model.get(), attr_name, column, &retval);
	check_error(error);
	return retval;
}

std::string GurobiModel::get_variable_raw_attribute_string(const VariableIndex &variable,
                                                           const char *attr_name)
{
	auto column = _checked_variable_index(variable);
	char *retval;
	int error = GRBgetstrattrelement(m_model.get(), attr_name, column, &retval);
	check_error(error);
	return std::string(retval);
}

int GurobiModel::_variable_index(const VariableIndex &variable)
{
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
	int error = GRBsetintattrelement(m_model.get(), attr_name, row, value);
	check_error(error);
}

void GurobiModel::set_constraint_raw_attribute_char(const ConstraintIndex &constraint,
                                                    const char *attr_name, char value)
{
	int row = _checked_constraint_index(constraint);
	int error = GRBsetcharattrelement(m_model.get(), attr_name, row, value);
	check_error(error);
}

void GurobiModel::set_constraint_raw_attribute_double(const ConstraintIndex &constraint,
                                                      const char *attr_name, double value)
{
	int row = _checked_constraint_index(constraint);
	int error = GRBsetdblattrelement(m_model.get(), attr_name, row, value);
	check_error(error);
}

void GurobiModel::set_constraint_raw_attribute_string(const ConstraintIndex &constraint,
                                                      const char *attr_name, const char *value)
{
	int row = _checked_constraint_index(constraint);
	int error = GRBsetstrattrelement(m_model.get(), attr_name, row, value);
	check_error(error);
}

int GurobiModel::get_constraint_raw_attribute_int(const ConstraintIndex &constraint,
                                                  const char *attr_name)
{
	int row = _checked_constraint_index(constraint);
	int retval;
	int error = GRBgetintattrelement(m_model.get(), attr_name, row, &retval);
	check_error(error);
	return retval;
}

char GurobiModel::get_constraint_raw_attribute_char(const ConstraintIndex &constraint,
                                                    const char *attr_name)
{
	int row = _checked_constraint_index(constraint);
	char retval;
	int error = GRBgetcharattrelement(m_model.get(), attr_name, row, &retval);
	check_error(error);
	return retval;
}

double GurobiModel::get_constraint_raw_attribute_double(const ConstraintIndex &constraint,
                                                        const char *attr_name)
{
	int row = _checked_constraint_index(constraint);
	double retval;
	int error = GRBgetdblattrelement(m_model.get(), attr_name, row, &retval);
	check_error(error);
	return retval;
}

std::string GurobiModel::get_constraint_raw_attribute_string(const ConstraintIndex &constraint,
                                                             const char *attr_name)
{
	int row = _checked_constraint_index(constraint);
	char *retval;
	int error = GRBgetstrattrelement(m_model.get(), attr_name, row, &retval);
	check_error(error);
	return std::string(retval);
}

int GurobiModel::_constraint_index(const ConstraintIndex &constraint)
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
		throw std::runtime_error(GRBgeterrormsg(m_env));
	}
}

void *GurobiModel::get_raw_model()
{
	return m_model.get();
}

std::string GurobiModel::version_string()
{
	std::string version =
	    std::format("v{}.{}.{}", GRB_VERSION_MAJOR, GRB_VERSION_MINOR, GRB_VERSION_TECHNICAL);
	return version;
}

GurobiEnv::GurobiEnv()
{
	int error = GRBloadenv(&m_env, NULL);
	if (error)
	{
		throw std::runtime_error(GRBgeterrormsg(m_env));
	}
}

GurobiEnv::~GurobiEnv()
{
	GRBfreeenv(m_env);
}
