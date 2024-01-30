#include "pyoptinterface/solver_common.hpp"
#include "pyoptinterface/highs_model.hpp"
#include "fmt/core.h"

static void check_error(HighsInt error)
{
	if (error == kHighsStatusError)
	{
		std::string errmsg = fmt::format(
		    "Encountered an error in HiGHS (Status {}). Check the log for details.", error);

		throw std::runtime_error(errmsg);
	}
}

static HighsInt highs_vtype(VariableDomain domain)
{
	using enum VariableDomain;
	switch (domain)
	{
	case Continuous:
		return kHighsVarTypeContinuous;
	case Integer:
	case Binary:
		return kHighsVarTypeInteger;
	case SemiContinuous:
		return kHighsVarTypeSemiContinuous;
	default:
		throw std::runtime_error("Unknown variable domain");
	}
}

static VariableDomain highs_vtype_to_domain(HighsInt vtype)
{
	using enum VariableDomain;
	switch (vtype)
	{
	case kHighsVarTypeContinuous:
		return Continuous;
	case kHighsVarTypeInteger:
		return Integer;
	case kHighsVarTypeSemiContinuous:
		return SemiContinuous;
	default:
		throw std::runtime_error("Unknown variable domain");
	}
}

static HighsInt highs_obj_sense(ObjectiveSense sense)
{
	using enum ObjectiveSense;
	switch (sense)
	{
	case Minimize:
		return kHighsObjSenseMinimize;
	case Maximize:
		return kHighsObjSenseMaximize;
	default:
		throw std::runtime_error("Unknown objective sense");
	}
}

POIHighsModel::POIHighsModel()
{
	init();
}

void POIHighsModel::init()
{
	void *model = Highs_create();
	m_model = std::unique_ptr<void, HighsfreemodelT>(model);
}

VariableIndex POIHighsModel::add_variable(VariableDomain domain, double lb, double ub,
                                          const char *name)
{
	if (name != nullptr && name[0] == '\0')
	{
		name = nullptr;
	}
	IndexT index = m_variable_index.add_index();
	VariableIndex variable(index);

	if (domain == VariableDomain::Binary)
	{
		lb = 0.0;
		ub = 1.0;
	}
	auto error = Highs_addCol(m_model.get(), 0.0, lb, ub, 0, nullptr, nullptr);
	check_error(error);

	auto column = Highs_getNumCol(m_model.get());
	// 0-based indexing
	column -= 1;

	if (domain != VariableDomain::Continuous)
	{
		auto vtype = highs_vtype(domain);
		if (domain == VariableDomain::Binary)
		{
			binary_variables.insert(index);
		}
		error = Highs_changeColIntegrality(m_model.get(), column, vtype);
		check_error(error);
	}

	if (name)
	{
		error = Highs_passColName(m_model.get(), column, name);
		check_error(error);
	}

	return variable;
}

void POIHighsModel::delete_variable(const VariableIndex &variable)
{
	if (!is_variable_active(variable))
	{
		throw std::runtime_error("Variable does not exist");
	}

	int variable_column = _variable_index(variable);
	auto error = Highs_deleteColsBySet(m_model.get(), 1, &variable_column);
	check_error(error);

	m_variable_index.delete_index(variable.index);
	binary_variables.erase(variable.index);
}

bool POIHighsModel::is_variable_active(const VariableIndex &variable)
{
	return m_variable_index.has_index(variable.index);
}

double POIHighsModel::get_variable_value(const VariableIndex &variable)
{
	auto column = _checked_variable_index(variable);
	if (m_solution.primal_solution_status != kHighsSolutionStatusNone)
	{
		return m_solution.colvalue[column];
	}
	throw std::runtime_error("No solution available");
}

std::string POIHighsModel::pprint_variable(const VariableIndex &variable)
{
	return get_variable_name(variable);
}

ConstraintIndex POIHighsModel::add_linear_constraint(const ScalarAffineFunction &function,
                                                     ConstraintSense sense, CoeffT rhs,
                                                     const char *name)
{
	IndexT index = m_linear_constraint_index.add_index();
	ConstraintIndex constraint_index(ConstraintType::Linear, index);

	AffineFunctionPtrForm<HighsInt, HighsInt, double> ptr_form;
	ptr_form.make(this, function);

	HighsInt numnz = ptr_form.numnz;
	HighsInt *cind = ptr_form.index;
	double *cval = ptr_form.value;

	double lb = -kHighsInf;
	double ub = kHighsInf;
	switch (sense)
	{
	case ConstraintSense::LessEqual:
		ub = rhs;
		break;
	case ConstraintSense::GreaterEqual:
		lb = rhs;
		break;
	case ConstraintSense::Equal:
		lb = rhs;
		ub = rhs;
		break;
	}

	auto error = Highs_addRow(m_model.get(), lb, ub, numnz, cind, cval);
	check_error(error);

	HighsInt row = Highs_getNumRow(m_model.get());
	// 0-based indexing
	row -= 1;
	if (name != nullptr && name[0] == '\0')
	{
		name = nullptr;
	}
	if (name)
	{
		error = Highs_passRowName(m_model.get(), row, name);
		check_error(error);
	}

	return constraint_index;
}

ConstraintIndex POIHighsModel::add_quadratic_constraint(const ScalarQuadraticFunction &function,
                                                        ConstraintSense sense, CoeffT rhs,
                                                        const char *name)
{
	throw std::runtime_error("HIGHS does not support quadratic constraint!");
}

void POIHighsModel::delete_constraint(const ConstraintIndex &constraint)
{
	if (!is_constraint_active(constraint))
	{
		throw std::runtime_error("Constraint does not exist");
	}

	int constraint_row = _constraint_index(constraint);
	auto error = Highs_deleteRowsBySet(m_model.get(), 1, &constraint_row);
	check_error(error);

	m_linear_constraint_index.delete_index(constraint.index);
}

bool POIHighsModel::is_constraint_active(const ConstraintIndex &constraint)
{
	return m_linear_constraint_index.has_index(constraint.index);
}

// #define private public
// #include "Highs.h"
void POIHighsModel::_set_affine_objective(const ScalarAffineFunction &function,
                                          ObjectiveSense sense, bool clear_quadratic)
{
	HighsInt error;

	HighsInt n_variables = Highs_getNumCol(m_model.get());
	if (clear_quadratic)
	{
		// First delete all quadratic terms
		std::vector<HighsInt> colstarts(n_variables, 0);
		error = Highs_passHessian(m_model.get(), n_variables, 0, kHighsHessianFormatTriangular,
		                          colstarts.data(), nullptr, nullptr);

		// HighsModel &model = ((Highs *)m_model.get())->model_;
		// auto &hessian = model.hessian_;

		check_error(error);
	}

	// Set Obj attribute of each variable
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

	error = Highs_changeColsCostByRange(m_model.get(), 0, n_variables - 1, obj_v.data());
	check_error(error);
	error = Highs_changeObjectiveOffset(m_model.get(), function.constant.value_or(0.0));
	check_error(error);

	HighsInt obj_sense = highs_obj_sense(sense);
	error = Highs_changeObjectiveSense(m_model.get(), obj_sense);
	check_error(error);
}

void POIHighsModel::set_objective(const ScalarAffineFunction &function, ObjectiveSense sense)
{
	_set_affine_objective(function, sense, true);
}

void POIHighsModel::set_objective(const ScalarQuadraticFunction &function, ObjectiveSense sense)
{
	HighsInt error;

	// Add quadratic term
	int numqnz = function.size();
	HighsInt n_variables = Highs_getNumCol(m_model.get());
	if (numqnz > 0)
	{
		CSCMatrix<HighsInt, HighsInt, double> csc;
		csc.make(this, function, n_variables, HessianTriangular::Upper);

		error =
		    Highs_passHessian(m_model.get(), n_variables, numqnz, kHighsHessianFormatTriangular,
		                      csc.colStarts_CSC.data(), csc.rows_CSC.data(), csc.values_CSC.data());
		check_error(error);
	}
	else
	{
		std::vector<HighsInt> colstarts(n_variables, 0);
		error = Highs_passHessian(m_model.get(), n_variables, 0, kHighsHessianFormatTriangular,
		                          colstarts.data(), nullptr, nullptr);
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

void POIHighsModel::set_objective(const ExprBuilder &function, ObjectiveSense sense)
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

void POIHighsModel::optimize()
{
	auto hessian_nz = Highs_getHessianNumNz(m_model.get());
	HighsInt error = Highs_run(m_model.get());
	hessian_nz = Highs_getHessianNumNz(m_model.get());

	POIHighsSolution &x = m_solution;
	x.status = error == kHighsStatusError ? HighsSolutionStatus::OPTIMIZE_ERROR
	                                      : HighsSolutionStatus::OPTIMIZE_OK;

	void *model = m_model.get();

	x.primal_solution_status = kHighsSolutionStatusNone;
	x.dual_solution_status = kHighsSolutionStatusNone;
	x.has_dual_ray = false;
	x.has_primal_ray = false;
	auto numCols = Highs_getNumCols(model);
	auto numRows = Highs_getNumRows(model);
	x.model_status = Highs_getModelStatus(model);

	HighsInt status;
	HighsInt *statusP = &status;
	if (x.model_status == kHighsModelStatusInfeasible)
	{
		x.dual_ray.resize(numRows);
		error = Highs_getDualRay(model, statusP, x.dual_ray.data());
		x.has_dual_ray = (error == kHighsStatusOk) && (*statusP == 1);
	}
	else if (x.model_status == kHighsModelStatusUnbounded)
	{
		x.primal_ray.resize(numCols);
		error = Highs_getPrimalRay(model, statusP, x.primal_ray.data());
		x.has_primal_ray = (error == kHighsStatusOk) && (*statusP == 1);
	}
	else
	{
		Highs_getIntInfoValue(model, "primal_solution_status", statusP);
		x.primal_solution_status = *statusP;
		Highs_getIntInfoValue(model, "dual_solution_status", statusP);
		x.dual_solution_status = *statusP;
		if (x.primal_solution_status != kHighsSolutionStatusNone)
		{
			x.colvalue.resize(numCols);
			x.coldual.resize(numCols);
			x.rowvalue.resize(numRows);
			x.rowdual.resize(numRows);
			Highs_getSolution(model, x.colvalue.data(), x.coldual.data(), x.rowvalue.data(),
			                  x.rowdual.data());

			// HighsModel &h_model = ((Highs *)m_model.get())->model_;
			// auto &hessian = h_model.hessian_;
			auto hessian_nz = Highs_getHessianNumNz(model);
			if (hessian_nz == 0)
			{
				// No basis is present in a QP.
				x.colstatus.resize(numCols);
				x.rowstatus.resize(numRows);
				Highs_getBasis(model, x.colstatus.data(), x.rowstatus.data());
			}
		}
	}
}

void *POIHighsModel::get_raw_model()
{
	return m_model.get();
}

std::string POIHighsModel::version_string()
{
	std::string version =
	    fmt::format("v{}.{}.{}", HIGHS_VERSION_MAJOR, HIGHS_VERSION_MINOR, HIGHS_VERSION_PATCH);
	return version;
}

double POIHighsModel::getruntime()
{
	double runtime = Highs_getRunTime(m_model.get());
	return runtime;
}

int POIHighsModel::getnumrow()
{
	return Highs_getNumRow(m_model.get());
}

int POIHighsModel::getnumcol()
{
	return Highs_getNumCol(m_model.get());
}

int POIHighsModel::raw_option_type(const char *param_name)
{
	HighsInt retval;
	auto error = Highs_getOptionType(m_model.get(), param_name, &retval);
	check_error(error);
	return retval;
}

void POIHighsModel::set_raw_option_bool(const char *param_name, bool value)
{
	auto error = Highs_setBoolOptionValue(m_model.get(), param_name, value);
	check_error(error);
}

void POIHighsModel::set_raw_option_int(const char *param_name, int value)
{
	auto error = Highs_setIntOptionValue(m_model.get(), param_name, value);
	check_error(error);
}

void POIHighsModel::set_raw_option_double(const char *param_name, double value)
{
	auto error = Highs_setDoubleOptionValue(m_model.get(), param_name, value);
	check_error(error);
}

void POIHighsModel::set_raw_option_string(const char *param_name, const char *value)
{
	auto error = Highs_setStringOptionValue(m_model.get(), param_name, value);
	check_error(error);
}

bool POIHighsModel::get_raw_option_bool(const char *param_name)
{
	HighsInt retval;
	auto error = Highs_getBoolOptionValue(m_model.get(), param_name, &retval);
	check_error(error);
	return retval;
}

int POIHighsModel::get_raw_option_int(const char *param_name)
{
	HighsInt retval;
	auto error = Highs_getIntOptionValue(m_model.get(), param_name, &retval);
	check_error(error);
	return retval;
}

double POIHighsModel::get_raw_option_double(const char *param_name)
{
	double retval;
	auto error = Highs_getDoubleOptionValue(m_model.get(), param_name, &retval);
	check_error(error);
	return retval;
}

std::string POIHighsModel::get_raw_option_string(const char *param_name)
{
	char retval[kHighsMaximumStringLength];
	auto error = Highs_getStringOptionValue(m_model.get(), param_name, retval);
	check_error(error);
	return std::string(retval);
}

int POIHighsModel::raw_info_type(const char *info_name)
{
	HighsInt retval;
	auto error = Highs_getInfoType(m_model.get(), info_name, &retval);
	check_error(error);
	return retval;
}

int POIHighsModel::get_raw_info_int(const char *info_name)
{
	HighsInt retval;
	auto error = Highs_getIntInfoValue(m_model.get(), info_name, &retval);
	check_error(error);
	return retval;
}

std::int64_t POIHighsModel::get_raw_info_int64(const char *info_name)
{
	int64_t retval;
	auto error = Highs_getInt64InfoValue(m_model.get(), info_name, &retval);
	check_error(error);
	return retval;
}

double POIHighsModel::get_raw_info_double(const char *info_name)
{
	double retval;
	auto error = Highs_getDoubleInfoValue(m_model.get(), info_name, &retval);
	check_error(error);
	return retval;
}

std::string POIHighsModel::get_variable_name(const VariableIndex &variable)
{
	auto column = _checked_variable_index(variable);
	char name[kHighsMaximumStringLength];
	auto error = Highs_getColName(m_model.get(), column, name);
	check_error(error);
	return std::string(name);
}

void POIHighsModel::set_variable_name(const VariableIndex &variable, const char *name)
{
	auto column = _checked_variable_index(variable);
	auto error = Highs_passColName(m_model.get(), column, name);
	check_error(error);
}

VariableDomain POIHighsModel::get_variable_type(const VariableIndex &variable)
{
	if (binary_variables.contains(variable.index))
	{
		return VariableDomain::Binary;
	}
	auto column = _checked_variable_index(variable);
	HighsInt vtype;
	auto error = Highs_getColIntegrality(m_model.get(), column, &vtype);
	check_error(error);
	return highs_vtype_to_domain(vtype);
}

void POIHighsModel::set_variable_type(const VariableIndex &variable, VariableDomain domain)
{
	auto vtype = highs_vtype(domain);
	auto column = _checked_variable_index(variable);
	auto error = Highs_changeColIntegrality(m_model.get(), column, vtype);
	check_error(error);

	if (domain == VariableDomain::Binary)
	{
		double lb = 0.0;
		double ub = 1.0;
		binary_variables.insert(variable.index);
		error = Highs_changeColsBoundsBySet(m_model.get(), 1, &column, &lb, &ub);
		check_error(error);
	}
	else
	{
		binary_variables.erase(variable.index);
	}
}

double POIHighsModel::get_variable_lower_bound(const VariableIndex &variable)
{
	auto column = _checked_variable_index(variable);
	HighsInt numcol, nz;
	double c, lb, ub;
	auto error = Highs_getColsBySet(m_model.get(), 1, &column, &numcol, &c, &lb, &ub, &nz, nullptr,
	                                nullptr, nullptr);
	check_error(error);
	return lb;
}

double POIHighsModel::get_variable_upper_bound(const VariableIndex &variable)
{
	auto column = _checked_variable_index(variable);
	HighsInt numcol, nz;
	double c, lb, ub;
	auto error = Highs_getColsBySet(m_model.get(), 1, &column, &numcol, &c, &lb, &ub, &nz, nullptr,
	                                nullptr, nullptr);
	check_error(error);
	return ub;
}

void POIHighsModel::set_variable_lower_bound(const VariableIndex &variable, double lb)
{
	double new_lb = lb;
	auto column = _checked_variable_index(variable);
	HighsInt numcol, nz;
	double c, ub;
	auto error = Highs_getColsBySet(m_model.get(), 1, &column, &numcol, &c, &lb, &ub, &nz, nullptr,
	                                nullptr, nullptr);
	check_error(error);
	error = Highs_changeColsBoundsBySet(m_model.get(), 1, &column, &new_lb, &ub);
	check_error(error);
}

void POIHighsModel::set_variable_upper_bound(const VariableIndex &variable, double ub)
{
	double new_ub = ub;
	auto column = _checked_variable_index(variable);
	HighsInt numcol, nz;
	double c, lb;
	auto error = Highs_getColsBySet(m_model.get(), 1, &column, &numcol, &c, &lb, &ub, &nz, nullptr,
	                                nullptr, nullptr);
	check_error(error);
	error = Highs_changeColsBoundsBySet(m_model.get(), 1, &column, &lb, &new_ub);
	check_error(error);
}

std::string POIHighsModel::get_constraint_name(const ConstraintIndex &constraint)
{
	auto row = _checked_constraint_index(constraint);
	char name[kHighsMaximumStringLength];
	auto error = Highs_getRowName(m_model.get(), row, name);
	check_error(error);
	return std::string(name);
}

void POIHighsModel::set_constraint_name(const ConstraintIndex &constraint, const char *name)
{
	auto row = _checked_constraint_index(constraint);
	auto error = Highs_passRowName(m_model.get(), row, name);
	check_error(error);
}

double POIHighsModel::get_constraint_primal(const ConstraintIndex &constraint)
{
	auto row = _checked_constraint_index(constraint);
	if (m_solution.primal_solution_status != kHighsSolutionStatusNone)
	{
		return m_solution.rowvalue[row];
	}
	throw std::runtime_error("No solution available");
}

double POIHighsModel::get_constraint_dual(const ConstraintIndex &constraint)
{
	auto row = _checked_constraint_index(constraint);
	if (m_solution.primal_solution_status != kHighsSolutionStatusNone)
	{
		return m_solution.rowdual[row];
	}
	throw std::runtime_error("No solution available");
}

ObjectiveSense POIHighsModel::get_obj_sense()
{
	HighsInt obj_sense;
	auto error = Highs_getObjectiveSense(m_model.get(), &obj_sense);
	check_error(error);
	return obj_sense == kHighsObjSenseMinimize ? ObjectiveSense::Minimize
	                                           : ObjectiveSense::Maximize;
}

void POIHighsModel::set_obj_sense(ObjectiveSense sense)
{
	auto obj_sense = highs_obj_sense(sense);
	auto error = Highs_changeObjectiveSense(m_model.get(), obj_sense);
	check_error(error);
}

double POIHighsModel::get_obj_value()
{
	return Highs_getObjectiveValue(m_model.get());
}

HighsInt POIHighsModel::_variable_index(const VariableIndex &variable)
{
	return m_variable_index.get_index(variable.index);
}

HighsInt POIHighsModel::_checked_variable_index(const VariableIndex &variable)
{
	HighsInt column = _variable_index(variable);
	if (column < 0)
	{
		throw std::runtime_error("Variable does not exist");
	}
	return column;
}

HighsInt POIHighsModel::_constraint_index(const ConstraintIndex &constraint)
{
	switch (constraint.type)
	{
	case ConstraintType::Linear:
		return m_linear_constraint_index.get_index(constraint.index);
	default:
		throw std::runtime_error("Unknown constraint type");
	}
}

HighsInt POIHighsModel::_checked_constraint_index(const ConstraintIndex &constraint)
{
	HighsInt row = _constraint_index(constraint);
	if (row < 0)
	{
		throw std::runtime_error("Constraint does not exist");
	}
	return row;
}

void POIHighsModel::set_primal_start(const Vector<VariableIndex> &variables,
                                     const Vector<double> &values)
{
	if (variables.size() != values.size())
	{
		throw std::runtime_error("Number of variables and values do not match");
	}
	int numnz = variables.size();
	if (numnz == 0)
		return;

	auto numcol = Highs_getNumCol(m_model.get());
	if (numcol == 0)
		return;

	HighsInt _numcol, nz;
	std::vector<double> c(numcol), lb(numcol), ub(numcol);
	auto error = Highs_getColsByRange(m_model.get(), 0, numcol - 1, &_numcol, c.data(), lb.data(),
	                                  ub.data(), &nz, nullptr, nullptr, nullptr);
	std::vector<double> vals(numcol);
	for (int i = 0; i < numcol; i++)
	{
		double L = lb[i];
		double U = ub[i];
		bool L_inf = L < -kHighsInf + 1.0;
		bool U_inf = U > kHighsInf - 1.0;

		double initial = L;
		if (L_inf)
		{
			if (U_inf)
			{
				initial = 0.0;
			}
			else
			{
				initial = U;
			}
		}
		vals[i] = initial;
	}

	for (auto i = 0; i < variables.size(); i++)
	{
		auto column = _checked_variable_index(variables[i]);
		vals[column] = values[i];
	}
	error = Highs_setSolution(m_model.get(), vals.data(), nullptr, nullptr, nullptr);
	check_error(error);
}

double POIHighsModel::get_normalized_rhs(const ConstraintIndex &constraint)
{
	auto row = _checked_constraint_index(constraint);
	HighsInt numrow, nz;
	double ub, lb;
	auto error = Highs_getRowsBySet(m_model.get(), 1, &row, &numrow, &lb, &ub, &nz, nullptr,
	                                nullptr, nullptr);
	check_error(error);

	bool lb_inf = lb < -kHighsInf + 1.0;
	bool ub_inf = ub > kHighsInf - 1.0;

	if (!lb_inf)
		return lb;
	if (!ub_inf)
		return ub;

	throw std::runtime_error("Constraint has no finite bound");
}

void POIHighsModel::set_normalized_rhs(const ConstraintIndex &constraint, double value)
{
	auto row = _checked_constraint_index(constraint);
	HighsInt numrow, nz;
	double ub, lb;
	auto error = Highs_getRowsBySet(m_model.get(), 1, &row, &numrow, &lb, &ub, &nz, nullptr,
	                                nullptr, nullptr);
	check_error(error);

	bool lb_inf = lb < -kHighsInf + 1.0;
	bool ub_inf = ub > kHighsInf - 1.0;

	if (!lb_inf)
		lb = value;
	if (!ub_inf)
		ub = value;
	if (lb_inf && ub_inf)
	{
		throw std::runtime_error("Constraint has no finite bound");
	}

	error = Highs_changeRowsBoundsBySet(m_model.get(), 1, &row, &lb, &ub);
	check_error(error);
}

double POIHighsModel::get_normalized_coefficient(const ConstraintIndex &constraint,
                                                 const VariableIndex &variable)
{
	auto row = _checked_constraint_index(constraint);
	HighsInt numrow, nz;
	double ub, lb;
	auto error = Highs_getRowsBySet(m_model.get(), 1, &row, &numrow, &lb, &ub, &nz, nullptr,
	                                nullptr, nullptr);
	check_error(error);

	std::vector<HighsInt> matrix_start(nz);
	std::vector<HighsInt> matrix_index(nz);
	std::vector<double> matrix_value(nz);

	error = Highs_getRowsBySet(m_model.get(), 1, &row, &numrow, &lb, &ub, &nz, matrix_start.data(),
	                           matrix_index.data(), matrix_value.data());
	check_error(error);

	double coef = 0.0;
	auto col = _checked_variable_index(variable);
	for (auto i = 0; i < nz; i++)
	{
		if (matrix_index[i] == col)
		{
			coef = matrix_value[i];
			break;
		}
	}
	return coef;
}

void POIHighsModel::set_normalized_coefficient(const ConstraintIndex &constraint,
                                               const VariableIndex &variable, double value)
{
	auto row = _checked_constraint_index(constraint);
	auto col = _checked_variable_index(variable);
	auto error = Highs_changeCoeff(m_model.get(), row, col, value);
	check_error(error);
}

double POIHighsModel::get_objective_coefficient(const VariableIndex &variable)
{
	auto column = _checked_variable_index(variable);
	HighsInt numcol, nz;
	double c, lb, ub;
	auto error = Highs_getColsBySet(m_model.get(), 1, &column, &numcol, &c, &lb, &ub, &nz, nullptr,
	                                nullptr, nullptr);
	check_error(error);
	return c;
}

void POIHighsModel::set_objective_coefficient(const VariableIndex &variable, double value)
{
	auto column = _checked_variable_index(variable);
	auto error = Highs_changeColCost(m_model.get(), column, value);
	check_error(error);
}
