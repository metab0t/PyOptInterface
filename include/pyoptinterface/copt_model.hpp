#include <memory>

#include "copt.h"

#include "pyoptinterface/core.hpp"
#include "pyoptinterface/container.hpp"
#include "pyoptinterface/solver_common.hpp"

class COPTEnvConfig
{
  public:
	COPTEnvConfig();
	~COPTEnvConfig();

	void set(const char *param_name, const char *value);

  private:
	copt_env_config *m_config;

	friend class COPTEnv;
};

class COPTEnv
{
  public:
	COPTEnv();
	COPTEnv(COPTEnvConfig &config);
	~COPTEnv();

  private:
	copt_env *m_env;

	friend class COPTModel;
};

struct COPTfreemodelT
{
	void operator()(copt_prob *model) const
	{
		COPT_DeleteProb(&model);
	};
};

class COPTModel
{
  public:
	COPTModel() = default;
	COPTModel(const COPTEnv &env);
	void init(const COPTEnv &env);

	VariableIndex add_variable(VariableDomain domain = VariableDomain::Continuous,
	                           double lb = -COPT_INFINITY, double ub = COPT_INFINITY,
	                           const char *name = nullptr);
	void delete_variable(const VariableIndex &variable);
	bool is_variable_active(const VariableIndex &variable);
	double get_variable_value(const VariableIndex &variable);
	std::string pprint_variable(const VariableIndex &variable);

	ConstraintIndex add_linear_constraint(const ScalarAffineFunction &function,
	                                      ConstraintSense sense, CoeffT rhs);
	ConstraintIndex add_quadratic_constraint(const ScalarQuadraticFunction &function,
	                                         ConstraintSense sense, CoeffT rhs);
	ConstraintIndex add_sos1_constraint(const Vector<VariableIndex> &variables,
	                                    const Vector<CoeffT> &weights);
	ConstraintIndex add_sos2_constraint(const Vector<VariableIndex> &variables,
	                                    const Vector<CoeffT> &weights);
	ConstraintIndex _add_sos_constraint(const Vector<VariableIndex> &variables,
	                                    const Vector<CoeffT> &weights, int sos_type);

	void delete_constraint(const ConstraintIndex &constraint);
	bool is_constraint_active(const ConstraintIndex &constraint);

	void set_objective(const ScalarAffineFunction &function, ObjectiveSense sense);
	void set_objective(const ScalarQuadraticFunction &function, ObjectiveSense sense);
	void set_objective(const ExprBuilder &function, ObjectiveSense sense);

	void optimize();
	void *get_raw_model();
	std::string version_string();

	/*
	 * Returns the type of a COPT parameter or attribute, given its name.
	 * -1: unknown
	 *  0: double parameter
	 *  1: int parameter
	 *  2: double attribute
	 *  3: int attribute
	 *
	 * Use undocumented COPT function
	 * int COPT_SearchParamAttr(copt_prob* prob, const char* name, int* p_type)
	 */
	int raw_parameter_attribute_type(const char *name);

	// parameter
	void set_raw_parameter_int(const char *param_name, int value);
	void set_raw_parameter_double(const char *param_name, double value);
	int get_raw_parameter_int(const char *param_name);
	double get_raw_parameter_double(const char *param_name);

	// attribute
	int get_raw_attribute_int(const char *attr_name);
	double get_raw_attribute_double(const char *attr_name);

	// Accessing information of problem
	double get_variable_info(const VariableIndex &variable, const char *info_name);
	std::string get_variable_name(const VariableIndex &variable);
	void set_variable_name(const VariableIndex &variable, const char *name);
	VariableDomain get_variable_type(const VariableIndex &variable);
	void set_variable_type(const VariableIndex &variable, VariableDomain domain);
	void set_variable_lower_bound(const VariableIndex &variable, double lb);
	void set_variable_upper_bound(const VariableIndex &variable, double ub);

	double get_constraint_info(const ConstraintIndex &constraint, const char *info_name);
	std::string get_constraint_name(const ConstraintIndex &constraint);
	void set_constraint_name(const ConstraintIndex &constraint, const char *name);

	void set_obj_sense(ObjectiveSense sense);

	// MIPStart
	void add_mip_start(const Vector<VariableIndex> &variables, const Vector<double> &values);

	int _variable_index(const VariableIndex &variable);
	int _checked_variable_index(const VariableIndex &variable);
	int _constraint_index(const ConstraintIndex &constraint);
	int _checked_constraint_index(const ConstraintIndex &constraint);

  private:
	MonotoneIndexer<int> m_variable_index;

	MonotoneIndexer<int> m_linear_constraint_index;

	MonotoneIndexer<int> m_quadratic_constraint_index;

	MonotoneIndexer<int> m_sos_constraint_index;

	/* COPT part */
	std::unique_ptr<copt_prob, COPTfreemodelT> m_model;
};

using COPTModelMixin = CommercialSolverMixin<COPTModel>;
