#include <memory>

#include "gurobi_c.h"

#include "pyoptinterface/core.hpp"
#include "pyoptinterface/container.hpp"

class GurobiEnv
{
  public:
	GurobiEnv();
	~GurobiEnv();

  private:
	GRBenv *m_env;

	friend class GurobiModel;
};

struct GRBfreemodelT
{
	void operator()(GRBmodel *model) const
	{
		GRBfreemodel(model);
	};
};

class GurobiModel
{
  public:
	GurobiModel() = default;
	GurobiModel(const GurobiEnv &env);
	void init(const GurobiEnv &env);

	VariableIndex add_variable(VariableDomain domain = VariableDomain::Continuous);
	void delete_variable(const VariableIndex &variable);
	bool is_variable_active(const VariableIndex &variable);

	ConstraintIndex add_linear_constraint(const ScalarAffineFunction &function,
	                                      ConstraintSense sense, CoeffT rhs);
	ConstraintIndex add_linear_constraint(const ExprBuilder &function, ConstraintSense sense,
	                                      CoeffT rhs);
	ConstraintIndex add_quadratic_constraint(const ScalarQuadraticFunction &function,
	                                         ConstraintSense sense, CoeffT rhs);
	ConstraintIndex add_quadratic_constraint(const ExprBuilder &function, ConstraintSense sense,
	                                         CoeffT rhs);
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
	void update();
	void *get_raw_model();
	std::string version_string();

	// parameter
	int raw_parameter_type(const char *param_name);
	void set_parameter_int(const char *param_name, int value);
	void set_parameter_double(const char *param_name, double value);
	void set_parameter_string(const char *param_name, const char *value);
	int get_parameter_int(const char *param_name);
	double get_parameter_double(const char *param_name);
	std::string get_parameter_string(const char *param_name);

	// attribute
	int raw_attribute_type(const char *attr_name);

	// model attribute
	void set_model_raw_attribute_int(const char *attr_name, int value);
	void set_model_raw_attribute_double(const char *attr_name, double value);
	void set_model_raw_attribute_string(const char *attr_name, const char *value);
	int get_model_raw_attribute_int(const char *attr_name);
	double get_model_raw_attribute_double(const char *attr_name);
	std::string get_model_raw_attribute_string(const char *attr_name);

	std::vector<double> get_model_raw_attribute_vector_double(const char *attr_name, int start,
	                                                          int len);
	std::vector<double> get_model_raw_attribute_list_double(const char *attr_name,
	                                                        const std::vector<int> &ind);

	// variable attribute
	void set_variable_raw_attribute_int(const VariableIndex &variable, const char *attr_name,
	                                    int value);
	void set_variable_raw_attribute_char(const VariableIndex &variable, const char *attr_name,
	                                     char value);
	void set_variable_raw_attribute_double(const VariableIndex &variable, const char *attr_name,
	                                       double value);
	void set_variable_raw_attribute_string(const VariableIndex &variable, const char *attr_name,
	                                       const char *value);
	int get_variable_raw_attribute_int(const VariableIndex &variable, const char *attr_name);
	char get_variable_raw_attribute_char(const VariableIndex &variable, const char *attr_name);
	double get_variable_raw_attribute_double(const VariableIndex &variable, const char *attr_name);
	std::string get_variable_raw_attribute_string(const VariableIndex &variable,
	                                              const char *attr_name);

	int _variable_index(const VariableIndex &variable);
	int _checked_variable_index(const VariableIndex &variable);

	// constraint attribute
	void set_constraint_raw_attribute_int(const ConstraintIndex &constraint, const char *attr_name,
	                                      int value);
	void set_constraint_raw_attribute_char(const ConstraintIndex &constraint, const char *attr_name,
	                                       char value);
	void set_constraint_raw_attribute_double(const ConstraintIndex &constraint,
	                                         const char *attr_name, double value);
	void set_constraint_raw_attribute_string(const ConstraintIndex &constraint,
	                                         const char *attr_name, const char *value);
	int get_constraint_raw_attribute_int(const ConstraintIndex &constraint, const char *attr_name);
	char get_constraint_raw_attribute_char(const ConstraintIndex &constraint,
	                                       const char *attr_name);
	double get_constraint_raw_attribute_double(const ConstraintIndex &constraint,
	                                           const char *attr_name);
	std::string get_constraint_raw_attribute_string(const ConstraintIndex &constraint,
	                                                const char *attr_name);

	int _constraint_index(const ConstraintIndex &constraint);
	int _checked_constraint_index(const ConstraintIndex &constraint);

	// Non-exported functions
	void check_error(int error);

  private:
	MonotoneVector<int> m_variable_index;

	MonotoneVector<int> m_linear_constraint_index;

	MonotoneVector<int> m_quadratic_constraint_index;

	MonotoneVector<int> m_sos_constraint_index;

	/* Gurobi part */
	GRBenv *m_env = nullptr;
	std::unique_ptr<GRBmodel, GRBfreemodelT> m_model;
};