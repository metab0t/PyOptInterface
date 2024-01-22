#include <memory>

#include "mosek.h"

#include "pyoptinterface/core.hpp"
#include "pyoptinterface/container.hpp"
#include "pyoptinterface/solver_common.hpp"

class MOSEKEnv
{
  public:
	MOSEKEnv();
	~MOSEKEnv();

	void putlicensecode(const std::vector<MSKint32t>& code);

  private:
	MSKenv_t m_env;

	friend class MOSEKModel;
};

struct MOSEKfreemodelT
{
	void operator()(MSKtask *model) const
	{
		MSK_deletetask(&model);
	};
};

class MOSEKModel
{
  public:
	MOSEKModel() = default;
	MOSEKModel(const MOSEKEnv &env);
	void init(const MOSEKEnv &env);

	VariableIndex add_variable(VariableDomain domain = VariableDomain::Continuous,
	                           double lb = -MSK_INFINITY, double ub = MSK_INFINITY,
	                           const char *name = nullptr);
	void delete_variable(const VariableIndex &variable);
	bool is_variable_active(const VariableIndex &variable);
	double get_variable_value(const VariableIndex &variable);
	std::string pprint_variable(const VariableIndex &variable);

	ConstraintIndex add_linear_constraint(const ScalarAffineFunction &function,
	                                      ConstraintSense sense, CoeffT rhs);
	ConstraintIndex add_quadratic_constraint(const ScalarQuadraticFunction &function,
	                                         ConstraintSense sense, CoeffT rhs);

	void delete_constraint(const ConstraintIndex &constraint);
	bool is_constraint_active(const ConstraintIndex &constraint);

	void _set_affine_objective(const ScalarAffineFunction &function, ObjectiveSense sense,
	                           bool clear_quadratic);
	void set_objective(const ScalarAffineFunction &function, ObjectiveSense sense);
	void set_objective(const ScalarQuadraticFunction &function, ObjectiveSense sense);
	void set_objective(const ExprBuilder &function, ObjectiveSense sense);

	int optimize();
	void *get_raw_model();
	std::string version_string();

	// solution
	MSKsoltypee select_available_solution();

	// parameter
	int raw_parameter_type(const char *name);

	void set_raw_parameter_int(const char *param_name, int value);
	void set_raw_parameter_double(const char *param_name, double value);
	void set_raw_parameter_string(const char *param_name, const char *value);
	int get_raw_parameter_int(const char *param_name);
	double get_raw_parameter_double(const char *param_name);
	std::string get_raw_parameter_string(const char *param_name);

	// information
	int get_raw_information_int(const char *attr_name);
	double get_raw_information_double(const char *attr_name);

	MSKint32t getnumvar();
	MSKint32t getnumcon();
	int getprosta();
	int getsolsta();
	double getprimalobj();
	double getdualobj();

	void enable_log();
	void disable_log();

	// Accessing information of problem
	std::string get_variable_name(const VariableIndex &variable);
	void set_variable_name(const VariableIndex &variable, const char *name);
	VariableDomain get_variable_type(const VariableIndex &variable);
	void set_variable_type(const VariableIndex &variable, VariableDomain domain);
	double get_variable_lower_bound(const VariableIndex &variable);
	double get_variable_upper_bound(const VariableIndex &variable);
	void set_variable_lower_bound(const VariableIndex &variable, double lb);
	void set_variable_upper_bound(const VariableIndex &variable, double ub);
	void set_variable_primal(const VariableIndex &variable, double primal);

	double get_constraint_primal(const ConstraintIndex &constraint);
	double get_constraint_dual(const ConstraintIndex &constraint);
	std::string get_constraint_name(const ConstraintIndex &constraint);
	void set_constraint_name(const ConstraintIndex &constraint, const char *name);

	ObjectiveSense get_obj_sense();
	void set_obj_sense(ObjectiveSense sense);

	MSKint32t _variable_index(const VariableIndex &variable);
	MSKint32t _checked_variable_index(const VariableIndex &variable);
	MSKint32t _constraint_index(const ConstraintIndex &constraint);
	MSKint32t _checked_constraint_index(const ConstraintIndex &constraint);

  private:
	MonotoneIndexer<MSKint32t> m_variable_index;

	MonotoneIndexer<MSKint32t> m_linear_quadratic_constraint_index;

	// Mosek does not discriminate between integer variable and binary variable
	// So we need to keep track of binary variables
	Hashset<IndexT> binary_variables;

	/* MOSEK part */
	std::unique_ptr<MSKtask, MOSEKfreemodelT> m_model;
};

using MOSEKModelMixin = CommercialSolverMixin<MOSEKModel>;
