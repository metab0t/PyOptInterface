#include "pyoptinterface/gurobi_model.hpp"

char gurobi_con_sense(ConstraintSense sense)
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

int gurobi_obj_sense(ObjectiveSense sense)
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

char gurobi_vtype(VariableDomain domain)
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

const char *gurobi_var_attr(VariableAttribute attr)
{
	switch (attr)
	{
	case VariableAttribute::Name:
		return "VarName";
	case VariableAttribute::Domain:
		return "VType";
	case VariableAttribute::LowerBound:
		return "LB";
	case VariableAttribute::UpperBound:
		return "UB";
	case VariableAttribute::Value:
		return "X";
	}
}

GurobiModel::GurobiModel(const GurobiEnv &env)
{
	init(env);
}

void GurobiModel::init(const GurobiEnv &env)
{
	GRBenv *env_ptr = env.m_env;
	GRBmodel *model;
	int error = GRBnewmodel(env_ptr, &model, NULL, 0, NULL, NULL, NULL, NULL, NULL);
	if (error)
	{
		throw std::runtime_error(GRBgeterrormsg(env_ptr));
	}
	m_env = env_ptr;
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

	return variable;
}

void GurobiModel::delete_variable(const VariableIndex &variable)
{
	if (!m_variable_index.has_index(variable.index))
	{
		throw std::runtime_error("Variable does not exist");
	}

	// Delete the corresponding Gurobi variable
	int variable_column = m_variable_index.get_index(variable.index);
	int error = GRBdelvars(m_model.get(), 1, &variable_column);

	m_variable_index.delete_index(variable.index);
}

bool GurobiModel::is_variable_active(const VariableIndex &variable) const
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
		cind_v[i] = m_variable_index.get_index(function.variables[i]);
		cval_v[i] = function.coefficients[i];
	}
	int *cind = cind_v.data();
	double *cval = cval_v.data();
	char g_sense = gurobi_con_sense(sense);
	double g_rhs = rhs - function.constant.value_or(0.0);

	int error = GRBaddconstr(m_model.get(), numnz, cind, cval, g_sense, g_rhs, NULL);

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
			lind_v[i] = m_variable_index.get_index(affine_function.variables[i]);
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
		qrow_v[i] = m_variable_index.get_index(function.variable_1s[i]);
		qcol_v[i] = m_variable_index.get_index(function.variable_2s[i]);
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
		ind_v[i] = m_variable_index.get_index(variables[i].index);
		weight_v[i] = weights[i];
	}
	int *ind = ind_v.data();
	double *weight = weight_v.data();

	int error = GRBaddsos(m_model.get(), numsos, nummembers, &types, beg, ind, weight);

	return constraint_index;
}

void GurobiModel::delete_constraint(const ConstraintIndex &constraint)
{
	// Delete the corresponding Gurobi constraint
	int constraint_row = -1;
	int error;
	switch (constraint.type)
	{
	case ConstraintType::Linear:
		constraint_row = m_linear_constraint_index.get_index(constraint.index);
		if (constraint_row >= 0)
		{
			m_linear_constraint_index.delete_index(constraint.index);
			error = GRBdelconstrs(m_model.get(), 1, &constraint_row);
		}
		break;
	case ConstraintType::Quadratic:
		constraint_row = m_quadratic_constraint_index.get_index(constraint.index);
		if (constraint_row >= 0)
		{
			m_quadratic_constraint_index.delete_index(constraint.index);
			error = GRBdelqconstrs(m_model.get(), 1, &constraint_row);
		}
		break;
	case ConstraintType::SOS1:
	case ConstraintType::SOS2:
		constraint_row = m_sos_constraint_index.get_index(constraint.index);
		if (constraint_row >= 0)
		{
			m_sos_constraint_index.delete_index(constraint.index);
			error = GRBdelsos(m_model.get(), 1, &constraint_row);
		}
		break;
	}
}

bool GurobiModel::is_constraint_active(const ConstraintIndex &constraint) const
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
		auto column = m_variable_index.get_index(function.variables[i]);
		if (column < 0)
		{
			throw std::runtime_error("Variable does not exist");
		}
		obj_v[column] = function.coefficients[i];
	}

	int error;
	error = GRBsetdblattrarray(m_model.get(), "Obj", 0, n_variables, obj_v.data());
	error = GRBsetdblattr(m_model.get(), "ObjCon", function.constant.value_or(0.0));

	int obj_sense = gurobi_obj_sense(sense);
	error = GRBsetintattr(m_model.get(), "ModelSense", obj_sense);
}

void GurobiModel::set_objective(const ScalarQuadraticFunction &function, ObjectiveSense sense)
{
	int error = 0;
	// First delete all quadratic terms
	error = GRBdelq(m_model.get());

	// Add quadratic term
	int numqnz = function.size();
	if (numqnz > 0)
	{
		std::vector<int> qrow_v(numqnz);
		std::vector<int> qcol_v(numqnz);
		std::vector<double> qval_v(numqnz);
		for (int i = 0; i < numqnz; i++)
		{
			qrow_v[i] = m_variable_index.get_index(function.variable_1s[i]);
			qcol_v[i] = m_variable_index.get_index(function.variable_2s[i]);
			qval_v[i] = function.coefficients[i];
		}
		int *qrow = qrow_v.data();
		int *qcol = qcol_v.data();
		double *qval = qval_v.data();

		error = GRBaddqpterms(m_model.get(), numqnz, qrow, qcol, qval);
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
}

void GurobiModel::update()
{
	int error = GRBupdatemodel(m_model.get());
}

void GurobiModel::set_model_raw_attribute_int(const char *attr_name, int value)
{
	int error = GRBsetintattr(m_model.get(), attr_name, value);
}

void GurobiModel::set_model_raw_attribute_double(const char *attr_name, double value)
{
	int error = GRBsetdblattr(m_model.get(), attr_name, value);
}

void GurobiModel::set_model_raw_attribute_string(const char *attr_name, const char *value)
{
	int error = GRBsetstrattr(m_model.get(), attr_name, value);
}

int GurobiModel::get_model_raw_attribute_int(const char *attr_name)
{
	int retval;
	int error = GRBgetintattr(m_model.get(), attr_name, &retval);
	return retval;
}

double GurobiModel::get_model_raw_attribute_double(const char *attr_name)
{
	double retval;
	int error = GRBgetdblattr(m_model.get(), attr_name, &retval);
	return retval;
}

char *GurobiModel::get_model_raw_attribute_string(const char *attr_name)
{
	char *retval;
	int error = GRBgetstrattr(m_model.get(), attr_name, &retval);
	return retval;
}

void GurobiModel::set_variable_raw_attribute_int(const VariableIndex &variable,
                                                 const char *attr_name, int value)
{
	auto column = m_variable_index.get_index(variable.index);
	if (column < 0)
	{
		throw std::runtime_error("Variable does not exist");
	}
	int error = GRBsetintattrelement(m_model.get(), attr_name, column, value);
}

void GurobiModel::set_variable_raw_attribute_char(const VariableIndex &variable,
                                                  const char *attr_name, char value)
{
	auto column = m_variable_index.get_index(variable.index);
	if (column < 0)
	{
		throw std::runtime_error("Variable does not exist");
	}
	int error = GRBsetcharattrelement(m_model.get(), attr_name, column, value);
}

void GurobiModel::set_variable_raw_attribute_double(const VariableIndex &variable,
                                                    const char *attr_name, double value)
{
	auto column = m_variable_index.get_index(variable.index);
	if (column < 0)
	{
		throw std::runtime_error("Variable does not exist");
	}
	int error = GRBsetdblattrelement(m_model.get(), attr_name, column, value);
}

void GurobiModel::set_variable_raw_attribute_string(const VariableIndex &variable,
                                                    const char *attr_name, const char *value)
{
	auto column = m_variable_index.get_index(variable.index);
	if (column < 0)
	{
		throw std::runtime_error("Variable does not exist");
	}
	int error = GRBsetstrattrelement(m_model.get(), attr_name, column, value);
}

int GurobiModel::get_variable_raw_attribute_int(const VariableIndex &variable,
                                                const char *attr_name)
{
	auto column = m_variable_index.get_index(variable.index);
	if (column < 0)
	{
		throw std::runtime_error("Variable does not exist");
	}
	int retval;
	int error = GRBgetintattrelement(m_model.get(), attr_name, column, &retval);
	return retval;
}

char GurobiModel::get_variable_raw_attribute_char(const VariableIndex &variable,
                                                  const char *attr_name)
{
	auto column = m_variable_index.get_index(variable.index);
	if (column < 0)
	{
		throw std::runtime_error("Variable does not exist");
	}
	char retval;
	int error = GRBgetcharattrelement(m_model.get(), attr_name, column, &retval);
	return retval;
}

double GurobiModel::get_variable_raw_attribute_double(const VariableIndex &variable,
                                                      const char *attr_name)
{
	auto column = m_variable_index.get_index(variable.index);
	if (column < 0)
	{
		throw std::runtime_error("Variable does not exist");
	}
	double retval;
	int error = GRBgetdblattrelement(m_model.get(), attr_name, column, &retval);
	return retval;
}

std::string GurobiModel::get_variable_raw_attribute_string(const VariableIndex &variable,
                                                           const char *attr_name)
{
	auto column = m_variable_index.get_index(variable.index);
	if (column < 0)
	{
		throw std::runtime_error("Variable does not exist");
	}
	char *retval;
	int error = GRBgetstrattrelement(m_model.get(), attr_name, column, &retval);
	return std::string(retval);
}

bool GurobiModel::support_variable_attribute(VariableAttribute attr)
{
	return true;
}

AttributeType GurobiModel::variable_attribute_type(VariableAttribute attr)
{
	switch (attr)
	{
	case VariableAttribute::Name:
		return AttributeType::String;
	case VariableAttribute::Domain:
		return AttributeType::Char;
	case VariableAttribute::LowerBound:
	case VariableAttribute::UpperBound:
	case VariableAttribute::Value:
		return AttributeType::Double;
	}
}

void GurobiModel::set_variable_attribute_int(const VariableIndex &variable, VariableAttribute attr,
                                             int value)
{
	auto attr_name = gurobi_var_attr(attr);
	set_variable_raw_attribute_int(variable, attr_name, value);
}

void GurobiModel::set_variable_attribute_char(const VariableIndex &variable, VariableAttribute attr,
                                              char value)
{
	auto attr_name = gurobi_var_attr(attr);
	set_variable_raw_attribute_char(variable, attr_name, value);
}

void GurobiModel::set_variable_attribute_double(const VariableIndex &variable,
                                                VariableAttribute attr, double value)
{
	auto attr_name = gurobi_var_attr(attr);
	set_variable_raw_attribute_double(variable, attr_name, value);
}

void GurobiModel::set_variable_attribute_string(const VariableIndex &variable,
                                                VariableAttribute attr, const char *value)
{
	auto attr_name = gurobi_var_attr(attr);
	set_variable_raw_attribute_string(variable, attr_name, value);
}

int GurobiModel::get_variable_attribute_int(const VariableIndex &variable, VariableAttribute attr)
{
	auto attr_name = gurobi_var_attr(attr);
	return get_variable_raw_attribute_int(variable, attr_name);
}

char GurobiModel::get_variable_attribute_char(const VariableIndex &variable, VariableAttribute attr)
{
	auto attr_name = gurobi_var_attr(attr);
	return get_variable_raw_attribute_char(variable, attr_name);
}

double GurobiModel::get_variable_attribute_double(const VariableIndex &variable,
                                                  VariableAttribute attr)
{
	auto attr_name = gurobi_var_attr(attr);
	return get_variable_raw_attribute_double(variable, attr_name);
}

std::string GurobiModel::get_variable_attribute_string(const VariableIndex &variable,
                                                       VariableAttribute attr)
{
	auto attr_name = gurobi_var_attr(attr);
	return get_variable_raw_attribute_string(variable, attr_name);
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
