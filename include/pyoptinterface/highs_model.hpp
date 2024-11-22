#pragma once

#include <memory>

#include "interfaces/highs_c_api.h"
#include "lp_data/HConst.h"

#include "pyoptinterface/core.hpp"
#include "pyoptinterface/container.hpp"
#include "pyoptinterface/solver_common.hpp"
#include "pyoptinterface/dylib.hpp"

#define APILIST                     \
	B(Highs_create);                \
	B(Highs_destroy);               \
	B(Highs_writeModel);            \
	B(Highs_addCol);                \
	B(Highs_getNumCol);             \
	B(Highs_changeColIntegrality);  \
	B(Highs_deleteColsBySet);       \
	B(Highs_addRow);                \
	B(Highs_getNumRow);             \
	B(Highs_deleteRowsBySet);       \
	B(Highs_passHessian);           \
	B(Highs_changeColsCostByRange); \
	B(Highs_changeObjectiveOffset); \
	B(Highs_changeObjectiveSense);  \
	B(Highs_run);                   \
	B(Highs_getNumCols);            \
	B(Highs_getNumRows);            \
	B(Highs_getModelStatus);        \
	B(Highs_getDualRay);            \
	B(Highs_getPrimalRay);          \
	B(Highs_getIntInfoValue);       \
	B(Highs_getSolution);           \
	B(Highs_getHessianNumNz);       \
	B(Highs_getBasis);              \
	B(Highs_version);               \
	B(Highs_getRunTime);            \
	B(Highs_getOptionType);         \
	B(Highs_setBoolOptionValue);    \
	B(Highs_setIntOptionValue);     \
	B(Highs_setDoubleOptionValue);  \
	B(Highs_setStringOptionValue);  \
	B(Highs_getBoolOptionValue);    \
	B(Highs_getIntOptionValue);     \
	B(Highs_getDoubleOptionValue);  \
	B(Highs_getStringOptionValue);  \
	B(Highs_getInfoType);           \
	B(Highs_getInt64InfoValue);     \
	B(Highs_getDoubleInfoValue);    \
	B(Highs_getColIntegrality);     \
	B(Highs_changeColsBoundsBySet); \
	B(Highs_getColsBySet);          \
	B(Highs_getObjectiveSense);     \
	B(Highs_getObjectiveValue);     \
	B(Highs_getColsByRange);        \
	B(Highs_setSolution);           \
	B(Highs_getRowsBySet);          \
	B(Highs_changeRowsBoundsBySet); \
	B(Highs_changeCoeff);           \
	B(Highs_changeColCost);

namespace highs
{
#define B DYLIB_EXTERN_DECLARE
APILIST
#undef B

bool is_library_loaded();

bool load_library(const std::string &path);
} // namespace highs

struct HighsfreemodelT
{
	void operator()(void *model) const
	{
		highs::Highs_destroy(model);
	};
};

enum class HighsSolutionStatus
{
	OPTIMIZE_NOT_CALLED,
	OPTIMIZE_OK,
	OPTIMIZE_ERROR,
};

struct POIHighsSolution
{
	HighsSolutionStatus status = HighsSolutionStatus::OPTIMIZE_NOT_CALLED;
	HighsInt model_status;
	std::vector<double> colvalue;
	std::vector<double> coldual;
	std::vector<HighsInt> colstatus;
	std::vector<double> rowvalue;
	std::vector<double> rowdual;
	std::vector<HighsInt> rowstatus;
	HighsInt primal_solution_status;
	HighsInt dual_solution_status;
	bool has_primal_ray;
	bool has_dual_ray;
	std::vector<double> primal_ray;
	std::vector<double> dual_ray;
};

class POIHighsModel
{
  public:
	POIHighsModel();
	void init();

	void write(const std::string &filename);

	VariableIndex add_variable(VariableDomain domain = VariableDomain::Continuous,
	                           double lb = -kHighsInf, double ub = kHighsInf,
	                           const char *name = nullptr);
	void delete_variable(const VariableIndex &variable);
	void delete_variables(const Vector<VariableIndex> &variables);
	bool is_variable_active(const VariableIndex &variable);
	double get_variable_value(const VariableIndex &variable);
	std::string pprint_variable(const VariableIndex &variable);
	void set_variable_bounds(const VariableIndex &variable, double lb, double ub);

	ConstraintIndex add_linear_constraint(const ScalarAffineFunction &function,
	                                      ConstraintSense sense, CoeffT rhs,
	                                      const char *name = nullptr);
	ConstraintIndex add_quadratic_constraint(const ScalarQuadraticFunction &function,
	                                         ConstraintSense sense, CoeffT rhs,
	                                         const char *name = nullptr);

	void delete_constraint(const ConstraintIndex &constraint);
	bool is_constraint_active(const ConstraintIndex &constraint);

	void _set_affine_objective(const ScalarAffineFunction &function, ObjectiveSense sense,
	                           bool clear_quadratic);
	void set_objective(const ScalarAffineFunction &function, ObjectiveSense sense);
	void set_objective(const ScalarQuadraticFunction &function, ObjectiveSense sense);
	void set_objective(const ExprBuilder &function, ObjectiveSense sense);

	void optimize();
	void *get_raw_model();
	std::string version_string();

	double getruntime();
	int getnumrow();
	int getnumcol();

	// option
	int raw_option_type(const char *param_name);
	void set_raw_option_bool(const char *param_name, bool value);
	void set_raw_option_int(const char *param_name, int value);
	void set_raw_option_double(const char *param_name, double value);
	void set_raw_option_string(const char *param_name, const char *value);
	bool get_raw_option_bool(const char *param_name);
	int get_raw_option_int(const char *param_name);
	double get_raw_option_double(const char *param_name);
	std::string get_raw_option_string(const char *param_name);

	// information
	int raw_info_type(const char *info_name);
	int get_raw_info_int(const char *info_name);
	std::int64_t get_raw_info_int64(const char *info_name);
	double get_raw_info_double(const char *info_name);

	// Accessing information of m_problem
	std::string get_variable_name(const VariableIndex &variable);
	void set_variable_name(const VariableIndex &variable, const char *name);
	VariableDomain get_variable_type(const VariableIndex &variable);
	void set_variable_type(const VariableIndex &variable, VariableDomain domain);
	double get_variable_lower_bound(const VariableIndex &variable);
	double get_variable_upper_bound(const VariableIndex &variable);
	void set_variable_lower_bound(const VariableIndex &variable, double lb);
	void set_variable_upper_bound(const VariableIndex &variable, double ub);

	std::string get_constraint_name(const ConstraintIndex &constraint);
	void set_constraint_name(const ConstraintIndex &constraint, const char *name);
	double get_constraint_primal(const ConstraintIndex &constraint);
	double get_constraint_dual(const ConstraintIndex &constraint);

	ObjectiveSense get_obj_sense();
	void set_obj_sense(ObjectiveSense sense);
	double get_obj_value();

	HighsInt _variable_index(const VariableIndex &variable);
	HighsInt _checked_variable_index(const VariableIndex &variable);
	HighsInt _constraint_index(const ConstraintIndex &constraint);
	HighsInt _checked_constraint_index(const ConstraintIndex &constraint);

	// Primal start
	void set_primal_start(const Vector<VariableIndex> &variables, const Vector<double> &values);

	// Modifications of model
	// 1. set/get RHS of a constraint
	double get_normalized_rhs(const ConstraintIndex &constraint);
	void set_normalized_rhs(const ConstraintIndex &constraint, double value);
	// 2. set/get coefficient of variable in constraint
	double get_normalized_coefficient(const ConstraintIndex &constraint,
	                                  const VariableIndex &variable);
	void set_normalized_coefficient(const ConstraintIndex &constraint,
	                                const VariableIndex &variable, double value);
	// 3. set/get linear coefficient of variable in objective
	double get_objective_coefficient(const VariableIndex &variable);
	void set_objective_coefficient(const VariableIndex &variable, double value);

  private:
	MonotoneIndexer<HighsInt> m_variable_index;

	MonotoneIndexer<HighsInt> m_linear_constraint_index;

	// Highs does not discriminate between integer variable and binary variable
	// So we need to keep track of binary variables
	Hashset<IndexT> binary_variables;

	// Store the names internally because use HiGHS API to set them is very expensive
	Hashmap<IndexT, std::string> m_var_names, m_con_names;

	/* Highs part */
	std::unique_ptr<void, HighsfreemodelT> m_model;

  public:
	// cache the solution
	POIHighsSolution m_solution;
	// cache the number of variable and constraints because querying them via HiGHS API is very
	// expensive
	HighsInt m_n_variables = 0;
	HighsInt m_n_constraints = 0;
};

using HighsModelMixin = CommercialSolverMixin<POIHighsModel>;
