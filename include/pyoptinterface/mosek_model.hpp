#pragma once

#include <memory>

#ifdef _MSC_VER
#include "solvers/mosek/mosek_win.h"
#else
#include "solvers/mosek/mosek_linux.h"
#endif

#include "pyoptinterface/core.hpp"
#include "pyoptinterface/container.hpp"
#include "pyoptinterface/solver_common.hpp"
#include "pyoptinterface/dylib.hpp"

#define APILIST                        \
	B(MSK_getcodedesc);                \
	B(MSK_makeemptytask);              \
	B(MSK_deletetask);                 \
	B(MSK_writedata);                  \
	B(MSK_writesolutionfile);          \
	B(MSK_appendvars);                 \
	B(MSK_getnumvar);                  \
	B(MSK_putvartype);                 \
	B(MSK_putvarbound);                \
	B(MSK_putvarname);                 \
	B(MSK_removevars);                 \
	B(MSK_getxxslice);                 \
	B(MSK_appendcons);                 \
	B(MSK_getnumcon);                  \
	B(MSK_putarow);                    \
	B(MSK_putconbound);                \
	B(MSK_putconname);                 \
	B(MSK_putqconk);                   \
	B(MSK_getnumafe);                  \
	B(MSK_appendafes);                 \
	B(MSK_putafefentrylist);           \
	B(MSK_appendquadraticconedomain);  \
	B(MSK_appendrquadraticconedomain); \
	B(MSK_appendprimalexpconedomain);  \
	B(MSK_appenddualexpconedomain);    \
	B(MSK_appendacc);                  \
	B(MSK_putaccname);                 \
	B(MSK_getaccn);                    \
	B(MSK_getaccafeidxlist);           \
	B(MSK_appendrdomain);              \
	B(MSK_putacc);                     \
	B(MSK_removecons);                 \
	B(MSK_putqobj);                    \
	B(MSK_putcslice);                  \
	B(MSK_putcfix);                    \
	B(MSK_optimize);                   \
	B(MSK_whichparam);                 \
	B(MSK_putnaintparam);              \
	B(MSK_putnadouparam);              \
	B(MSK_putnastrparam);              \
	B(MSK_getnaintparam);              \
	B(MSK_getnadouparam);              \
	B(MSK_getnastrparam);              \
	B(MSK_getnaintinf);                \
	B(MSK_getnadouinf);                \
	B(MSK_getprosta);                  \
	B(MSK_getsolsta);                  \
	B(MSK_getprimalobj);               \
	B(MSK_getdualobj);                 \
	B(MSK_linkfunctotaskstream);       \
	B(MSK_getvarname);                 \
	B(MSK_getvartype);                 \
	B(MSK_getvarbound);                \
	B(MSK_putxxslice);                 \
	B(MSK_getxcslice);                 \
	B(MSK_getyslice);                  \
	B(MSK_getconnamelen);              \
	B(MSK_getconname);                 \
	B(MSK_getobjsense);                \
	B(MSK_putobjsense);                \
	B(MSK_getconbound);                \
	B(MSK_getaij);                     \
	B(MSK_putaij);                     \
	B(MSK_getcj);                      \
	B(MSK_putcj);                      \
	B(MSK_getversion);                 \
	B(MSK_solutiondef);                \
	B(MSK_makeenv);                    \
	B(MSK_deleteenv);                  \
	B(MSK_putlicensecode);

namespace mosek
{
#define B(f) extern decltype(&::f) f

APILIST

#undef B

bool is_library_loaded();

bool load_library(const std::string &path);
} // namespace mosek

class MOSEKEnv
{
  public:
	MOSEKEnv();
	~MOSEKEnv();
	void close();

	void putlicensecode(const std::vector<MSKint32t> &code);

  private:
	MSKenv_t m_env;

	friend class MOSEKModel;
};

struct MOSEKfreemodelT
{
	void operator()(MSKtask *model) const
	{
		mosek::MSK_deletetask(&model);
	};
};

class MOSEKModel
{
  public:
	MOSEKModel() = default;
	MOSEKModel(const MOSEKEnv &env);
	void init(const MOSEKEnv &env);
	void close();

	void write(const std::string &filename);

	VariableIndex add_variable(VariableDomain domain = VariableDomain::Continuous,
	                           double lb = -MSK_INFINITY, double ub = MSK_INFINITY,
	                           const char *name = nullptr);
	/*VariableIndex add_variables(int N, VariableDomain domain = VariableDomain::Continuous,
	                            double lb = -MSK_INFINITY, double ub = MSK_INFINITY);*/
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

	std::vector<MSKint64t> add_variables_as_afe(const Vector<VariableIndex> &variables);
	ConstraintIndex add_variables_in_cone_constraint(const Vector<VariableIndex> &variables,
	                                                 MSKint64t domain_index, const char *name);

	ConstraintIndex add_second_order_cone_constraint(const Vector<VariableIndex> &variables,
	                                                 const char *name, bool rotated = false);

	ConstraintIndex add_exp_cone_constraint(const Vector<VariableIndex> &variables,
	                                        const char *name, bool dual = false);

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
	MSKsoltypee get_current_solution();
	std::optional<MSKsoltypee> select_available_solution_after_optimization();

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

	MSKint32t _variable_index(const VariableIndex &variable);
	MSKint32t _checked_variable_index(const VariableIndex &variable);
	MSKint32t _constraint_index(const ConstraintIndex &constraint);
	MSKint32t _checked_constraint_index(const ConstraintIndex &constraint);

  private:
	MonotoneIndexer<MSKint32t> m_variable_index;

	MonotoneIndexer<MSKint32t> m_linear_quadratic_constraint_index;

	// ACC cannot be removed from the model, so we just keeps track of the state whether it is
	// deleted If a constraint is deleted, we will not remove it from the model, but just mark it as
	// deleted and set the domain to R^n (i.e. no constraint)
	std::vector<bool> m_acc_index;

	// Mosek does not discriminate between integer variable and binary variable
	// So we need to keep track of binary variables
	Hashset<IndexT> binary_variables;

	// Cache current available solution after optimization
	std::optional<MSKsoltypee> m_soltype;

	/* MOSEK part */
	std::unique_ptr<MSKtask, MOSEKfreemodelT> m_model;
};

using MOSEKModelMixin = CommercialSolverMixin<MOSEKModel>;
