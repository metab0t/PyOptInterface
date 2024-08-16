#include "pyoptinterface/dylib.hpp"
#include "pyoptinterface/solver_common.hpp"
#include "pyoptinterface/highs_model.hpp"
#include "fmt/core.h"

namespace highs
{
#define B(f) decltype(&::f) f = nullptr

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

#define B(f)                                                          \
	{                                                                 \
		auto ptr = reinterpret_cast<decltype(f)>(lib.get_symbol(#f)); \
		if (ptr == nullptr)                                           \
		{                                                             \
			fmt::print("function {} is not loaded correctly", #f);    \
			return false;                                             \
		}                                                             \
		f = ptr;                                                      \
	}
	APILIST
#undef B

	is_loaded = true;

	return true;
}
} // namespace highs

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
	switch (domain)
	{
	case VariableDomain::Continuous:
		return kHighsVarTypeContinuous;
	case VariableDomain::Integer:
	case VariableDomain::Binary:
		return kHighsVarTypeInteger;
	case VariableDomain::SemiContinuous:
		return kHighsVarTypeSemiContinuous;
	default:
		throw std::runtime_error("Unknown variable domain");
	}
}

static VariableDomain highs_vtype_to_domain(HighsInt vtype)
{
	switch (vtype)
	{
	case kHighsVarTypeContinuous:
		return VariableDomain::Continuous;
	case kHighsVarTypeInteger:
		return VariableDomain::Integer;
	case kHighsVarTypeSemiContinuous:
		return VariableDomain::SemiContinuous;
	default:
		throw std::runtime_error("Unknown variable domain");
	}
}

static HighsInt highs_obj_sense(ObjectiveSense sense)
{
	switch (sense)
	{
	case ObjectiveSense::Minimize:
		return kHighsObjSenseMinimize;
	case ObjectiveSense::Maximize:
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
	if (!highs::is_library_loaded())
	{
		throw std::runtime_error("HiGHS library is not loaded");
	}
	void *model = highs::Highs_create();
	m_model = std::unique_ptr<void, HighsfreemodelT>(model);
}

void POIHighsModel::write(const std::string &filename)
{
	auto error = highs::Highs_writeModel(m_model.get(), filename.c_str());
	check_error(error);
}

VariableIndex POIHighsModel::add_variable(VariableDomain domain, double lb, double ub,
                                          const char *name)
{
	IndexT index = m_variable_index.add_index();
	VariableIndex variable(index);

	if (domain == VariableDomain::Binary)
	{
		lb = 0.0;
		ub = 1.0;
	}
	auto error = highs::Highs_addCol(m_model.get(), 0.0, lb, ub, 0, nullptr, nullptr);
	check_error(error);

	auto column = m_n_variables;

	if (domain != VariableDomain::Continuous)
	{
		auto vtype = highs_vtype(domain);
		if (domain == VariableDomain::Binary)
		{
			binary_variables.insert(index);
		}
		error = highs::Highs_changeColIntegrality(m_model.get(), column, vtype);
		check_error(error);
	}

	if (name != nullptr && name[0] == '\0')
	{
		name = nullptr;
	}
	if (name)
	{
		m_var_names.insert({variable.index, name});
	}

	m_n_variables++;
	return variable;
}

void POIHighsModel::delete_variable(const VariableIndex &variable)
{
	if (!is_variable_active(variable))
	{
		throw std::runtime_error("Variable does not exist");
	}

	int variable_column = _variable_index(variable);
	auto error = highs::Highs_deleteColsBySet(m_model.get(), 1, &variable_column);
	check_error(error);

	m_variable_index.delete_index(variable.index);
	binary_variables.erase(variable.index);

	m_n_variables--;
	m_var_names.erase(variable.index);
}

void POIHighsModel::delete_variables(const Vector<VariableIndex> &variables)
{
	int n_variables = variables.size();
	if (n_variables == 0)
		return;

	std::vector<HighsInt> columns;
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

	int error = highs::Highs_deleteColsBySet(m_model.get(), columns.size(), columns.data());
	check_error(error);

	for (int i = 0; i < n_variables; i++)
	{
		m_variable_index.delete_index(variables[i].index);
		m_var_names.erase(variables[i].index);
	}
	m_n_variables -= columns.size();
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
	ConstraintIndex constraint(ConstraintType::Linear, index);

	AffineFunctionPtrForm<HighsInt, HighsInt, double> ptr_form;
	ptr_form.make(this, function);

	HighsInt numnz = ptr_form.numnz;
	HighsInt *cind = ptr_form.index;
	double *cval = ptr_form.value;

	double lb = -kHighsInf;
	double ub = kHighsInf;
	double g_rhs = rhs - function.constant.value_or(0.0);
	switch (sense)
	{
	case ConstraintSense::LessEqual:
		ub = g_rhs;
		break;
	case ConstraintSense::GreaterEqual:
		lb = g_rhs;
		break;
	case ConstraintSense::Equal:
		lb = g_rhs;
		ub = g_rhs;
		break;
	}

	auto error = highs::Highs_addRow(m_model.get(), lb, ub, numnz, cind, cval);
	check_error(error);

	HighsInt row = m_n_constraints;

	if (name != nullptr && name[0] == '\0')
	{
		name = nullptr;
	}
	if (name)
	{
		m_con_names.insert({constraint.index, name});
	}

	m_n_constraints++;

	return constraint;
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
	auto error = highs::Highs_deleteRowsBySet(m_model.get(), 1, &constraint_row);
	check_error(error);

	m_linear_constraint_index.delete_index(constraint.index);
	m_con_names.erase(constraint.index);

	m_n_constraints--;
}

bool POIHighsModel::is_constraint_active(const ConstraintIndex &constraint)
{
	return m_linear_constraint_index.has_index(constraint.index);
}

void POIHighsModel::_set_affine_objective(const ScalarAffineFunction &function,
                                          ObjectiveSense sense, bool clear_quadratic)
{
	HighsInt error;

	HighsInt n_variables = m_n_variables;
	if (clear_quadratic)
	{
		// First delete all quadratic terms
		std::vector<HighsInt> colstarts(n_variables, 0);
		error =
		    highs::Highs_passHessian(m_model.get(), n_variables, 0, kHighsHessianFormatTriangular,
		                             colstarts.data(), nullptr, nullptr);

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

	error = highs::Highs_changeColsCostByRange(m_model.get(), 0, n_variables - 1, obj_v.data());
	check_error(error);
	error = highs::Highs_changeObjectiveOffset(m_model.get(), function.constant.value_or(0.0));
	check_error(error);

	HighsInt obj_sense = highs_obj_sense(sense);
	error = highs::Highs_changeObjectiveSense(m_model.get(), obj_sense);
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
	HighsInt n_variables = m_n_variables;
	if (numqnz > 0)
	{
		CSCMatrix<HighsInt, HighsInt, double> csc;
		csc.make(this, function, n_variables, HessianTriangular::Lower);

		// Highs optimizes 0.5 * x' * Q * x
		// so the coefficient must be multiplied by 2.0
		for (auto &v : csc.values_CSC)
		{
			v *= 2.0;
		}

		error = highs::Highs_passHessian(m_model.get(), n_variables, numqnz,
		                                 kHighsHessianFormatTriangular, csc.colStarts_CSC.data(),
		                                 csc.rows_CSC.data(), csc.values_CSC.data());
		check_error(error);
	}
	else
	{
		std::vector<HighsInt> colstarts(n_variables, 0);
		error =
		    highs::Highs_passHessian(m_model.get(), n_variables, 0, kHighsHessianFormatTriangular,
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
	HighsInt error = highs::Highs_run(m_model.get());

	POIHighsSolution &x = m_solution;
	x.status = error == kHighsStatusError ? HighsSolutionStatus::OPTIMIZE_ERROR
	                                      : HighsSolutionStatus::OPTIMIZE_OK;

	void *model = m_model.get();

	x.primal_solution_status = kHighsSolutionStatusNone;
	x.dual_solution_status = kHighsSolutionStatusNone;
	x.has_dual_ray = false;
	x.has_primal_ray = false;
	auto numCols = m_n_variables;
	auto numRows = m_n_constraints;
	x.model_status = highs::Highs_getModelStatus(model);

	HighsInt status;
	HighsInt *statusP = &status;
	if (x.model_status == kHighsModelStatusInfeasible)
	{
		x.dual_ray.resize(numRows);
		error = highs::Highs_getDualRay(model, statusP, x.dual_ray.data());
		x.has_dual_ray = (error == kHighsStatusOk) && (*statusP == 1);
	}
	else if (x.model_status == kHighsModelStatusUnbounded)
	{
		x.primal_ray.resize(numCols);
		error = highs::Highs_getPrimalRay(model, statusP, x.primal_ray.data());
		x.has_primal_ray = (error == kHighsStatusOk) && (*statusP == 1);
	}
	else
	{
		highs::Highs_getIntInfoValue(model, "primal_solution_status", statusP);
		x.primal_solution_status = *statusP;
		highs::Highs_getIntInfoValue(model, "dual_solution_status", statusP);
		x.dual_solution_status = *statusP;
		if (x.primal_solution_status != kHighsSolutionStatusNone)
		{
			x.colvalue.resize(numCols);
			x.coldual.resize(numCols);
			x.rowvalue.resize(numRows);
			x.rowdual.resize(numRows);
			highs::Highs_getSolution(model, x.colvalue.data(), x.coldual.data(), x.rowvalue.data(),
			                         x.rowdual.data());

			// HighsModel &h_model = ((Highs *)m_model.get())->model_;
			// auto &hessian = h_model.hessian_;
			auto hessian_nz = highs::Highs_getHessianNumNz(model);
			if (hessian_nz == 0)
			{
				// No basis is present in a QP.
				x.colstatus.resize(numCols);
				x.rowstatus.resize(numRows);
				highs::Highs_getBasis(model, x.colstatus.data(), x.rowstatus.data());
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
	auto version = highs::Highs_version();
	return version;
}

double POIHighsModel::getruntime()
{
	double runtime = highs::Highs_getRunTime(m_model.get());
	return runtime;
}

int POIHighsModel::getnumrow()
{
	return highs::Highs_getNumRow(m_model.get());
}

int POIHighsModel::getnumcol()
{
	return highs::Highs_getNumCol(m_model.get());
}

int POIHighsModel::raw_option_type(const char *param_name)
{
	HighsInt retval;
	auto error = highs::Highs_getOptionType(m_model.get(), param_name, &retval);
	check_error(error);
	return retval;
}

void POIHighsModel::set_raw_option_bool(const char *param_name, bool value)
{
	auto error = highs::Highs_setBoolOptionValue(m_model.get(), param_name, value);
	check_error(error);
}

void POIHighsModel::set_raw_option_int(const char *param_name, int value)
{
	auto error = highs::Highs_setIntOptionValue(m_model.get(), param_name, value);
	check_error(error);
}

void POIHighsModel::set_raw_option_double(const char *param_name, double value)
{
	auto error = highs::Highs_setDoubleOptionValue(m_model.get(), param_name, value);
	check_error(error);
}

void POIHighsModel::set_raw_option_string(const char *param_name, const char *value)
{
	auto error = highs::Highs_setStringOptionValue(m_model.get(), param_name, value);
	check_error(error);
}

bool POIHighsModel::get_raw_option_bool(const char *param_name)
{
	HighsInt retval;
	auto error = highs::Highs_getBoolOptionValue(m_model.get(), param_name, &retval);
	check_error(error);
	return retval;
}

int POIHighsModel::get_raw_option_int(const char *param_name)
{
	HighsInt retval;
	auto error = highs::Highs_getIntOptionValue(m_model.get(), param_name, &retval);
	check_error(error);
	return retval;
}

double POIHighsModel::get_raw_option_double(const char *param_name)
{
	double retval;
	auto error = highs::Highs_getDoubleOptionValue(m_model.get(), param_name, &retval);
	check_error(error);
	return retval;
}

std::string POIHighsModel::get_raw_option_string(const char *param_name)
{
	char retval[kHighsMaximumStringLength];
	auto error = highs::Highs_getStringOptionValue(m_model.get(), param_name, retval);
	check_error(error);
	return std::string(retval);
}

int POIHighsModel::raw_info_type(const char *info_name)
{
	HighsInt retval;
	auto error = highs::Highs_getInfoType(m_model.get(), info_name, &retval);
	check_error(error);
	return retval;
}

int POIHighsModel::get_raw_info_int(const char *info_name)
{
	HighsInt retval;
	auto error = highs::Highs_getIntInfoValue(m_model.get(), info_name, &retval);
	check_error(error);
	return retval;
}

std::int64_t POIHighsModel::get_raw_info_int64(const char *info_name)
{
	int64_t retval;
	auto error = highs::Highs_getInt64InfoValue(m_model.get(), info_name, &retval);
	check_error(error);
	return retval;
}

double POIHighsModel::get_raw_info_double(const char *info_name)
{
	double retval;
	auto error = highs::Highs_getDoubleInfoValue(m_model.get(), info_name, &retval);
	check_error(error);
	return retval;
}

std::string POIHighsModel::get_variable_name(const VariableIndex &variable)
{
	auto iter = m_var_names.find(variable.index);
	if (iter != m_var_names.end())
	{
		return iter->second;
	}
	else
	{
		return fmt::format("x{}", variable.index);
	}
}

void POIHighsModel::set_variable_name(const VariableIndex &variable, const char *name)
{
	m_var_names[variable.index] = name;
}

VariableDomain POIHighsModel::get_variable_type(const VariableIndex &variable)
{
	if (binary_variables.contains(variable.index))
	{
		return VariableDomain::Binary;
	}
	auto column = _checked_variable_index(variable);
	HighsInt vtype;
	auto error = highs::Highs_getColIntegrality(m_model.get(), column, &vtype);
	check_error(error);
	return highs_vtype_to_domain(vtype);
}

void POIHighsModel::set_variable_type(const VariableIndex &variable, VariableDomain domain)
{
	auto vtype = highs_vtype(domain);
	auto column = _checked_variable_index(variable);
	auto error = highs::Highs_changeColIntegrality(m_model.get(), column, vtype);
	check_error(error);

	if (domain == VariableDomain::Binary)
	{
		double lb = 0.0;
		double ub = 1.0;
		binary_variables.insert(variable.index);
		error = highs::Highs_changeColsBoundsBySet(m_model.get(), 1, &column, &lb, &ub);
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
	auto error = highs::Highs_getColsBySet(m_model.get(), 1, &column, &numcol, &c, &lb, &ub, &nz,
	                                       nullptr, nullptr, nullptr);
	check_error(error);
	return lb;
}

double POIHighsModel::get_variable_upper_bound(const VariableIndex &variable)
{
	auto column = _checked_variable_index(variable);
	HighsInt numcol, nz;
	double c, lb, ub;
	auto error = highs::Highs_getColsBySet(m_model.get(), 1, &column, &numcol, &c, &lb, &ub, &nz,
	                                       nullptr, nullptr, nullptr);
	check_error(error);
	return ub;
}

void POIHighsModel::set_variable_lower_bound(const VariableIndex &variable, double lb)
{
	double new_lb = lb;
	auto column = _checked_variable_index(variable);
	HighsInt numcol, nz;
	double c, ub;
	auto error = highs::Highs_getColsBySet(m_model.get(), 1, &column, &numcol, &c, &lb, &ub, &nz,
	                                       nullptr, nullptr, nullptr);
	check_error(error);
	error = highs::Highs_changeColsBoundsBySet(m_model.get(), 1, &column, &new_lb, &ub);
	check_error(error);
}

void POIHighsModel::set_variable_upper_bound(const VariableIndex &variable, double ub)
{
	double new_ub = ub;
	auto column = _checked_variable_index(variable);
	HighsInt numcol, nz;
	double c, lb;
	auto error = highs::Highs_getColsBySet(m_model.get(), 1, &column, &numcol, &c, &lb, &ub, &nz,
	                                       nullptr, nullptr, nullptr);
	check_error(error);
	error = highs::Highs_changeColsBoundsBySet(m_model.get(), 1, &column, &lb, &new_ub);
	check_error(error);
}

std::string POIHighsModel::get_constraint_name(const ConstraintIndex &constraint)
{
	auto iter = m_con_names.find(constraint.index);
	if (iter != m_con_names.end())
	{
		return iter->second;
	}
	else
	{
		return fmt::format("con{}", constraint.index);
	}
}

void POIHighsModel::set_constraint_name(const ConstraintIndex &constraint, const char *name)
{
	m_con_names[constraint.index] = name;
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
	auto error = highs::Highs_getObjectiveSense(m_model.get(), &obj_sense);
	check_error(error);
	return obj_sense == kHighsObjSenseMinimize ? ObjectiveSense::Minimize
	                                           : ObjectiveSense::Maximize;
}

void POIHighsModel::set_obj_sense(ObjectiveSense sense)
{
	auto obj_sense = highs_obj_sense(sense);
	auto error = highs::Highs_changeObjectiveSense(m_model.get(), obj_sense);
	check_error(error);
}

double POIHighsModel::get_obj_value()
{
	double obj = highs::Highs_getObjectiveValue(m_model.get());
	return obj;
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

	auto numcol = m_n_variables;
	if (numcol == 0)
		return;

	HighsInt _numcol, nz;
	std::vector<double> c(numcol), lb(numcol), ub(numcol);
	auto error = highs::Highs_getColsByRange(m_model.get(), 0, numcol - 1, &_numcol, c.data(),
	                                         lb.data(), ub.data(), &nz, nullptr, nullptr, nullptr);
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
	error = highs::Highs_setSolution(m_model.get(), vals.data(), nullptr, nullptr, nullptr);
	check_error(error);
}

double POIHighsModel::get_normalized_rhs(const ConstraintIndex &constraint)
{
	auto row = _checked_constraint_index(constraint);
	HighsInt numrow, nz;
	double ub, lb;
	auto error = highs::Highs_getRowsBySet(m_model.get(), 1, &row, &numrow, &lb, &ub, &nz, nullptr,
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
	auto error = highs::Highs_getRowsBySet(m_model.get(), 1, &row, &numrow, &lb, &ub, &nz, nullptr,
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

	error = highs::Highs_changeRowsBoundsBySet(m_model.get(), 1, &row, &lb, &ub);
	check_error(error);
}

double POIHighsModel::get_normalized_coefficient(const ConstraintIndex &constraint,
                                                 const VariableIndex &variable)
{
	auto row = _checked_constraint_index(constraint);
	HighsInt numrow, nz;
	double ub, lb;
	auto error = highs::Highs_getRowsBySet(m_model.get(), 1, &row, &numrow, &lb, &ub, &nz, nullptr,
	                                       nullptr, nullptr);
	check_error(error);

	std::vector<HighsInt> matrix_start(nz);
	std::vector<HighsInt> matrix_index(nz);
	std::vector<double> matrix_value(nz);

	error =
	    highs::Highs_getRowsBySet(m_model.get(), 1, &row, &numrow, &lb, &ub, &nz,
	                              matrix_start.data(), matrix_index.data(), matrix_value.data());
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
	auto error = highs::Highs_changeCoeff(m_model.get(), row, col, value);
	check_error(error);
}

double POIHighsModel::get_objective_coefficient(const VariableIndex &variable)
{
	auto column = _checked_variable_index(variable);
	HighsInt numcol, nz;
	double c, lb, ub;
	auto error = highs::Highs_getColsBySet(m_model.get(), 1, &column, &numcol, &c, &lb, &ub, &nz,
	                                       nullptr, nullptr, nullptr);
	check_error(error);
	return c;
}

void POIHighsModel::set_objective_coefficient(const VariableIndex &variable, double value)
{
	auto column = _checked_variable_index(variable);
	auto error = highs::Highs_changeColCost(m_model.get(), column, value);
	check_error(error);
}
