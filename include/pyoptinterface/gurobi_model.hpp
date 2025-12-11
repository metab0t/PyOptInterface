#pragma once

#include <memory>

#include "solvers/gurobi/gurobi_c.h"

#include "pyoptinterface/core.hpp"
#include "pyoptinterface/container.hpp"
#include "pyoptinterface/nlexpr.hpp"
#define USE_NLMIXIN
#include "pyoptinterface/solver_common.hpp"
#include "pyoptinterface/dylib.hpp"

// define Gurobi C APIs
#define APILIST               \
	B(GRBnewmodel);           \
	B(GRBfreemodel);          \
	B(GRBreset);              \
	B(GRBgetenv);             \
	B(GRBwrite);              \
	B(GRBaddvar);             \
	B(GRBdelvars);            \
	B(GRBaddconstr);          \
	B(GRBaddqconstr);         \
	B(GRBaddsos);             \
	B(GRBaddgenconstrNL);     \
	B(GRBdelconstrs);         \
	B(GRBdelqconstrs);        \
	B(GRBdelsos);             \
	B(GRBdelgenconstrs);      \
	B(GRBdelq);               \
	B(GRBsetdblattrarray);    \
	B(GRBaddqpterms);         \
	B(GRBoptimize);           \
	B(GRBupdatemodel);        \
	B(GRBgetparamtype);       \
	B(GRBsetintparam);        \
	B(GRBsetdblparam);        \
	B(GRBsetstrparam);        \
	B(GRBgetintparam);        \
	B(GRBgetdblparam);        \
	B(GRBgetstrparam);        \
	B(GRBgetattrinfo);        \
	B(GRBsetintattr);         \
	B(GRBsetdblattr);         \
	B(GRBsetstrattr);         \
	B(GRBgetintattr);         \
	B(GRBgetdblattr);         \
	B(GRBgetstrattr);         \
	B(GRBgetdblattrarray);    \
	B(GRBgetdblattrlist);     \
	B(GRBsetdblattrlist);     \
	B(GRBsetintattrelement);  \
	B(GRBsetcharattrelement); \
	B(GRBsetdblattrelement);  \
	B(GRBsetstrattrelement);  \
	B(GRBgetintattrelement);  \
	B(GRBgetcharattrelement); \
	B(GRBgetdblattrelement);  \
	B(GRBgetstrattrelement);  \
	B(GRBgetcoeff);           \
	B(GRBchgcoeffs);          \
	B(GRBgeterrormsg);        \
	B(GRBversion);            \
	B(GRBsetcallbackfunc);    \
	B(GRBcbget);              \
	B(GRBcbproceed);          \
	B(GRBterminate);          \
	B(GRBcbsolution);         \
	B(GRBcblazy);             \
	B(GRBcbcut);              \
	B(GRBemptyenv);           \
	B(GRBloadenv);            \
	B(GRBfreeenv);            \
	B(GRBstartenv);           \
	B(GRBsetlogcallbackfunc); \
	B(GRBconverttofixed);     \
	B(GRBcomputeIIS);

namespace gurobi
{
#define B DYLIB_EXTERN_DECLARE
APILIST
#undef B

bool is_library_loaded();

bool load_library(const std::string &path);
} // namespace gurobi

class GurobiEnv
{
  public:
	GurobiEnv(bool empty = false);
	~GurobiEnv();

	// parameter
	int raw_parameter_type(const char *param_name);
	void set_raw_parameter_int(const char *param_name, int value);
	void set_raw_parameter_double(const char *param_name, double value);
	void set_raw_parameter_string(const char *param_name, const char *value);

	void start();
	void close();

	void check_error(int error);

	GRBenv *m_env = nullptr;
};

struct GRBfreemodelT
{
	void operator()(GRBmodel *model) const
	{
		gurobi::GRBfreemodel(model);
	};
};

class GurobiModel;
using GurobiCallback = std::function<void(GurobiModel *, int)>;

struct GurobiCallbackUserdata
{
	void *model = nullptr;
	GurobiCallback callback;
	int n_variables = 0;
	int where = 0;
	// store result of cbget
	bool cb_get_mipsol_called = false;
	std::vector<double> mipsol;
	bool cb_get_mipnoderel_called = false;
	std::vector<double> mipnoderel;
	// Cache for cbsolution
	bool cb_set_solution_called = false;
	std::vector<double> heuristic_solution;
	bool cb_requires_submit_solution = false;
};

using GurobiLoggingCallback = std::function<void(const char *)>;

struct GurobiLoggingCallbackUserdata
{
	GurobiLoggingCallback callback;
};

class GurobiModel : public OnesideLinearConstraintMixin<GurobiModel>,
                    public OnesideQuadraticConstraintMixin<GurobiModel>,
                    public TwosideNLConstraintMixin<GurobiModel>,
                    public LinearObjectiveMixin<GurobiModel>,
                    public PPrintMixin<GurobiModel>,
                    public GetValueMixin<GurobiModel>
{
  public:
	GurobiModel() = default;
	GurobiModel(const GurobiEnv &env);
	void init(const GurobiEnv &env);
	void close();

	void _reset(int clearall);

	double get_infinity() const;

	void write(const std::string &filename);

	VariableIndex add_variable(VariableDomain domain = VariableDomain::Continuous,
	                           double lb = -GRB_INFINITY, double ub = GRB_INFINITY,
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

	// Nonlinear constraint
	void information_of_expr(const ExpressionGraph &graph, const ExpressionHandle &expr,
	                         int &opcode, double &data);
	void decode_graph(const ExpressionGraph &graph, const ExpressionHandle &result,
	                  std::vector<int> &opcodes, std::vector<int> &parents,
	                  std::vector<double> &datas);
	ConstraintIndex add_single_nl_constraint(const ExpressionGraph &graph,
	                                         const ExpressionHandle &result,
	                                         const std::tuple<double, double> &interval,
	                                         const char *name = nullptr);

	void delete_constraint(const ConstraintIndex &constraint);
	bool is_constraint_active(const ConstraintIndex &constraint);

	void _set_affine_objective(const ScalarAffineFunction &function, ObjectiveSense sense,
	                           bool clear_quadratic);
	void set_objective(const ScalarAffineFunction &function, ObjectiveSense sense);
	void set_objective(const ScalarQuadraticFunction &function, ObjectiveSense sense);
	void set_objective(const ExprBuilder &function, ObjectiveSense sense);

	void add_single_nl_objective(ExpressionGraph &graph, const ExpressionHandle &result);
	void set_nl_objective();

	void optimize();
	void update();
	void *get_raw_model();
	std::string version_string();

	// parameter
	int raw_parameter_type(const char *param_name);
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

	// Gurobi-specific convertofixed
	void _converttofixed();

	// IIS related
	void computeIIS();

	// Non-exported functions
	void check_error(int error);

	// Control logging
	void set_logging(const GurobiLoggingCallback &callback);

	GurobiLoggingCallbackUserdata m_logging_callback_userdata;

	// Callback
	void set_callback(const GurobiCallback &callback);

	// For callback
	bool has_callback = false;
	void *m_cbdata = nullptr;
	GurobiCallbackUserdata m_callback_userdata;

	int cb_get_info_int(int what);
	double cb_get_info_double(int what);
	void cb_get_info_doublearray(int what);

	double cb_get_solution(const VariableIndex &variable);
	double cb_get_relaxation(const VariableIndex &variable);
	void cb_set_solution(const VariableIndex &variable, double value);
	double cb_submit_solution();

	void cb_exit();

	void cb_add_lazy_constraint(const ScalarAffineFunction &function, ConstraintSense sense,
	                            CoeffT rhs);
	void cb_add_lazy_constraint(const ExprBuilder &function, ConstraintSense sense, CoeffT rhs);
	void cb_add_user_cut(const ScalarAffineFunction &function, ConstraintSense sense, CoeffT rhs);
	void cb_add_user_cut(const ExprBuilder &function, ConstraintSense sense, CoeffT rhs);

  private:
	MonotoneIndexer<int> m_variable_index;

	MonotoneIndexer<int> m_linear_constraint_index;

	MonotoneIndexer<int> m_quadratic_constraint_index;

	MonotoneIndexer<int> m_sos_constraint_index;

	MonotoneIndexer<int> m_general_constraint_index;
	// Gurobi only accepts y = f(x) style nonlinear constraint
	// so for each lb <= f(x) <= ub, we need to convert it to
	// lb <= y <= ub, and add y = f(x) as a nonlinear constraint
	// y is called the result variable (resvar)
	Hashmap<IndexT, IndexT> m_nlcon_resvar_map;
	// for each nonlinear term in objective function, we need to record the resvar to set their
	// coefficient in objective as 1.0
	// add f(x) to the objective is divided into two steps:
	// 1. add a new nonlinear constraint y = f(x)
	// 2. set the coefficient of y in objective to 1.0
	int m_nlobj_num = 0;
	std::vector<int> m_nlobj_con_indices;
	std::vector<int> m_nlobj_resvar_indices;

	/* flag to indicate whether the model needs update */
	enum : std::uint64_t
	{
		m_variable_creation = 1,
		m_variable_deletion = 1 << 1,
		m_linear_constraint_creation = 1 << 2,
		m_linear_constraint_deletion = 1 << 3,
		m_quadratic_constraint_creation = 1 << 4,
		m_quadratic_constraint_deletion = 1 << 5,
		m_sos_constraint_creation = 1 << 6,
		m_sos_constraint_deletion = 1 << 7,
		m_general_constraint_creation = 1 << 8,
		m_general_constraint_deletion = 1 << 9,
		m_objective_update = 1 << 10,
		m_attribute_update = 1 << 11,
		m_constraint_coefficient_update = 1 << 12,
	};
	std::uint64_t m_update_flag = 0;
	void _update_for_information();
	void _update_for_variable_index();
	void _update_for_constraint_index(ConstraintType type);

	/* Gurobi part */
	GRBenv *m_env = nullptr;
	std::unique_ptr<GRBmodel, GRBfreemodelT> m_model;
};
