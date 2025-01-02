#pragma once

#include <memory>

#include "solvers/mindopt/Mindopt.h"

#include "pyoptinterface/core.hpp"
#include "pyoptinterface/container.hpp"
#include "pyoptinterface/solver_common.hpp"
#include "pyoptinterface/dylib.hpp"

// define Mindopt C APIs
#define APILIST               \
	B(MDOloadenv);            \
	B(MDOemptyenv);           \
	B(MDOstartenv);           \
	B(MDOfreeenv);            \
	B(MDOexplainerror);       \
	B(MDOnewmodel);           \
	B(MDOfreemodel);          \
	B(MDOwrite);              \
	B(MDOaddvar);             \
	B(MDOdelvars);            \
	B(MDOsetdblattrelement);  \
	B(MDOgetdblattrelement);  \
	B(MDOaddconstr);          \
	B(MDOaddqconstr);         \
	B(MDOaddsos);             \
	B(MDOdelconstrs);         \
	B(MDOdelqconstrs);        \
	B(MDOdelsos);             \
	B(MDOdelq);               \
	B(MDOsetdblattrarray);    \
	B(MDOsetdblattr);         \
	B(MDOgetdblattr);         \
	B(MDOaddqpterms);         \
	B(MDOoptimize);           \
	B(MDOsetintparam);        \
	B(MDOsetdblparam);        \
	B(MDOsetstrparam);        \
	B(MDOgetintparam);        \
	B(MDOgetdblparam);        \
	B(MDOgetstrparam);        \
	B(MDOgetattrinfo);        \
	B(MDOsetintattr);         \
	B(MDOsetstrattr);         \
	B(MDOgetintattr);         \
	B(MDOgetstrattr);         \
	B(MDOgetdblattrarray);    \
	B(MDOgetdblattrlist);     \
	B(MDOsetintattrelement);  \
	B(MDOsetcharattrelement); \
	B(MDOsetstrattrelement);  \
	B(MDOgetintattrelement);  \
	B(MDOgetcharattrelement); \
	B(MDOgetstrattrelement);  \
	B(MDOgetcoeff);           \
	B(MDOchgcoeffs);          \
	B(MDOcomputeIIS);         \
	B(MDOversion);

namespace mindopt
{
#define B DYLIB_EXTERN_DECLARE
APILIST
#undef B

bool is_library_loaded();

bool load_library(const std::string &path);
} // namespace mindopt

class MindoptEnv
{
  public:
	MindoptEnv(bool empty = false);
	~MindoptEnv();

	void start();

  private:
	MDOenv *m_env;

	friend class MindoptModel;
};

struct MindoptfreemodelT
{
	void operator()(MDOmodel *model) const
	{
		mindopt::MDOfreemodel(model);
	};
};

class MindoptModel
{
  public:
	MindoptModel() = default;
	MindoptModel(const MindoptEnv &env);
	void init(const MindoptEnv &env);

	void write(const std::string &filename);

	VariableIndex add_variable(VariableDomain domain = VariableDomain::Continuous,
	                           double lb = -MDO_INFINITY, double ub = MDO_INFINITY,
	                           const char *name = nullptr);
	void delete_variable(const VariableIndex &variable);
	void delete_variables(const Vector<VariableIndex> &variables);
	bool is_variable_active(const VariableIndex &variable);
	double get_variable_value(const VariableIndex &variable);
	std::string pprint_variable(const VariableIndex &variable);
	void set_variable_bounds(const VariableIndex &variable, double lb, double ub);

	void set_variable_name(const VariableIndex &variable, const char *name);
	void set_constraint_name(const ConstraintIndex &constraint, const char *name);

	ConstraintIndex add_linear_constraint(const ScalarAffineFunction &function,
	                                      ConstraintSense sense, CoeffT rhs,
	                                      const char *name = nullptr);
	ConstraintIndex add_quadratic_constraint(const ScalarQuadraticFunction &function,
	                                         ConstraintSense sense, CoeffT rhs,
	                                         const char *name = nullptr);
	ConstraintIndex add_sos_constraint(const Vector<VariableIndex> &variables, SOSType sos_type);
	ConstraintIndex add_sos_constraint(const Vector<VariableIndex> &variables, SOSType sos_type,
	                                   const Vector<CoeffT> &weights);

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

	// parameter
	// int raw_parameter_type(const char *param_name);
	void set_raw_parameter_int(const char *param_name, int value);
	void set_raw_parameter_double(const char *param_name, double value);
	void set_raw_parameter_string(const char *param_name, const char *value);
	int get_raw_parameter_int(const char *param_name);
	double get_raw_parameter_double(const char *param_name);
	std::string get_raw_parameter_string(const char *param_name);

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

	// IIS related
	void computeIIS();

  private:
	MonotoneIndexer<int> m_variable_index;

	MonotoneIndexer<int> m_linear_constraint_index;

	MonotoneIndexer<int> m_quadratic_constraint_index;

	MonotoneIndexer<int> m_sos_constraint_index;

	/* Mindopt part */
	MDOenv *m_env = nullptr;
	std::unique_ptr<MDOmodel, MindoptfreemodelT> m_model;
};

using MindoptModelMixin = CommercialSolverMixin<MindoptModel>;
