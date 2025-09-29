#pragma once

#include <memory>

#include "solvers/copt/copt.h"

#include "pyoptinterface/core.hpp"
#include "pyoptinterface/container.hpp"
#include "pyoptinterface/nlexpr.hpp"
#define USE_NLMIXIN
#include "pyoptinterface/solver_common.hpp"
#include "pyoptinterface/dylib.hpp"

extern "C"
{
	int COPT_SearchParamAttr(copt_prob *prob, const char *name, int *p_type);
}

#define APILIST                    \
	B(COPT_GetRetcodeMsg);         \
	B(COPT_CreateProb);            \
	B(COPT_DeleteProb);            \
	B(COPT_WriteMps);              \
	B(COPT_WriteLp);               \
	B(COPT_WriteCbf);              \
	B(COPT_WriteBin);              \
	B(COPT_WriteBasis);            \
	B(COPT_WriteSol);              \
	B(COPT_WriteMst);              \
	B(COPT_WriteParam);            \
	B(COPT_AddCol);                \
	B(COPT_DelCols);               \
	B(COPT_AddRow);                \
	B(COPT_AddQConstr);            \
	B(COPT_AddSOSs);               \
	B(COPT_AddCones);              \
	B(COPT_AddExpCones);           \
	B(COPT_AddNLConstr);           \
	B(COPT_DelRows);               \
	B(COPT_DelQConstrs);           \
	B(COPT_DelSOSs);               \
	B(COPT_DelCones);              \
	B(COPT_DelExpCones);           \
	B(COPT_DelQuadObj);            \
	B(COPT_ReplaceColObj);         \
	B(COPT_SetObjConst);           \
	B(COPT_SetObjSense);           \
	B(COPT_SetQuadObj);            \
	B(COPT_SetNLObj);              \
	B(COPT_Solve);                 \
	B(COPT_SearchParamAttr);       \
	B(COPT_SetIntParam);           \
	B(COPT_SetDblParam);           \
	B(COPT_GetIntParam);           \
	B(COPT_GetDblParam);           \
	B(COPT_GetIntAttr);            \
	B(COPT_GetDblAttr);            \
	B(COPT_GetColInfo);            \
	B(COPT_GetColName);            \
	B(COPT_SetColNames);           \
	B(COPT_GetColType);            \
	B(COPT_SetColType);            \
	B(COPT_SetColLower);           \
	B(COPT_SetColUpper);           \
	B(COPT_GetRowInfo);            \
	B(COPT_GetQConstrInfo);        \
	B(COPT_GetNLConstrInfo);       \
	B(COPT_GetRowName);            \
	B(COPT_GetQConstrName);        \
	B(COPT_GetNLConstrName);       \
	B(COPT_SetRowNames);           \
	B(COPT_SetQConstrNames);       \
	B(COPT_SetNLConstrNames);      \
	B(COPT_AddMipStart);           \
	B(COPT_SetNLPrimalStart);      \
	B(COPT_GetQConstrRhs);         \
	B(COPT_SetRowLower);           \
	B(COPT_SetRowUpper);           \
	B(COPT_SetQConstrRhs);         \
	B(COPT_GetElem);               \
	B(COPT_SetElem);               \
	B(COPT_SetColObj);             \
	B(COPT_GetBanner);             \
	B(COPT_SetCallback);           \
	B(COPT_GetCallbackInfo);       \
	B(COPT_AddCallbackSolution);   \
	B(COPT_AddCallbackLazyConstr); \
	B(COPT_AddCallbackUserCut);    \
	B(COPT_Interrupt);             \
	B(COPT_CreateEnv);             \
	B(COPT_CreateEnvWithConfig);   \
	B(COPT_DeleteEnv);             \
	B(COPT_CreateEnvConfig);       \
	B(COPT_DeleteEnvConfig);       \
	B(COPT_SetEnvConfig);          \
	B(COPT_ComputeIIS);            \
	B(COPT_GetColLowerIIS);        \
	B(COPT_GetColUpperIIS);        \
	B(COPT_GetRowLowerIIS);        \
	B(COPT_GetRowUpperIIS);        \
	B(COPT_GetSOSIIS);

namespace copt
{
#define B DYLIB_EXTERN_DECLARE
APILIST
#undef B

bool is_library_loaded();

bool load_library(const std::string &path);
} // namespace copt

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

	void close();

  private:
	copt_env *m_env;

	friend class COPTModel;
};

struct COPTfreemodelT
{
	void operator()(copt_prob *model) const
	{
		copt::COPT_DeleteProb(&model);
	};
};

class COPTModel;
using COPTCallback = std::function<void(COPTModel *, int)>;

struct COPTCallbackUserdata
{
	void *model = nullptr;
	COPTCallback callback;
	int n_variables = 0;
	int where = 0;
	// store result of cbget
	bool cb_get_mipsol_called = false;
	std::vector<double> mipsol;
	bool cb_get_mipnoderel_called = false;
	std::vector<double> mipnoderel;
	bool cb_get_mipincumbent_called = false;
	std::vector<double> mipincumbent;
	// Cache for cbsolution
	bool cb_set_solution_called = false;
	std::vector<double> heuristic_solution;
	bool cb_requires_submit_solution = false;
};

class COPTModel : public OnesideLinearConstraintMixin<COPTModel>,
                  public TwosideLinearConstraintMixin<COPTModel>,
                  public OnesideQuadraticConstraintMixin<COPTModel>,
                  public TwosideNLConstraintMixin<COPTModel>,
                  public LinearObjectiveMixin<COPTModel>,
                  public PPrintMixin<COPTModel>,
                  public GetValueMixin<COPTModel>
{
  public:
	COPTModel() = default;
	COPTModel(const COPTEnv &env);
	void init(const COPTEnv &env);
	void close();

	double get_infinity() const;

	void write(const std::string &filename);

	VariableIndex add_variable(VariableDomain domain = VariableDomain::Continuous,
	                           double lb = -COPT_INFINITY, double ub = COPT_INFINITY,
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
	ConstraintIndex add_linear_constraint(const ScalarAffineFunction &function,
	                                      const std::tuple<double, double> &interval,
	                                      const char *name = nullptr);
	ConstraintIndex add_quadratic_constraint(const ScalarQuadraticFunction &function,
	                                         ConstraintSense sense, CoeffT rhs,
	                                         const char *name = nullptr);
	ConstraintIndex add_sos_constraint(const Vector<VariableIndex> &variables, SOSType sos_type);
	ConstraintIndex add_sos_constraint(const Vector<VariableIndex> &variables, SOSType sos_type,
	                                   const Vector<CoeffT> &weights);

	// x[0]^2 >= x[1]^2 + x[2]^2 + ... + x[n-1]^2
	ConstraintIndex add_second_order_cone_constraint(const Vector<VariableIndex> &variables,
	                                                 const char *name, bool rotated = false);

	ConstraintIndex add_exp_cone_constraint(const Vector<VariableIndex> &variables,
	                                        const char *name, bool dual = false);

	// Nonlinear constraint
	void decode_expr(const ExpressionGraph &graph, const ExpressionHandle &expr,
	                 std::vector<int> &opcodes, std::vector<double> &constants);
	void decode_graph_prefix_order(ExpressionGraph &graph, const ExpressionHandle &result,
	                               std::vector<int> &opcodes, std::vector<double> &constants);
	ConstraintIndex add_single_nl_constraint(ExpressionGraph &graph, const ExpressionHandle &result,
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
	// NLP start
	void add_nl_start(const Vector<VariableIndex> &variables, const Vector<double> &values);

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

	int _variable_index(const VariableIndex &variable);
	int _checked_variable_index(const VariableIndex &variable);
	int _constraint_index(const ConstraintIndex &constraint);
	int _checked_constraint_index(const ConstraintIndex &constraint);

	// Callback
	void set_callback(const COPTCallback &callback, int cbctx);

	// For callback
	bool has_callback = false;
	void *m_cbdata = nullptr;
	COPTCallbackUserdata m_callback_userdata;

	int cb_get_info_int(const std::string &what);
	double cb_get_info_double(const std::string &what);
	void cb_get_info_doublearray(const std::string &what);

	double cb_get_solution(const VariableIndex &variable);
	double cb_get_relaxation(const VariableIndex &variable);
	double cb_get_incumbent(const VariableIndex &variable);
	void cb_set_solution(const VariableIndex &variable, double value);
	double cb_submit_solution();

	void cb_exit();

	void cb_add_lazy_constraint(const ScalarAffineFunction &function, ConstraintSense sense,
	                            CoeffT rhs);
	void cb_add_lazy_constraint(const ExprBuilder &function, ConstraintSense sense, CoeffT rhs);
	void cb_add_user_cut(const ScalarAffineFunction &function, ConstraintSense sense, CoeffT rhs);
	void cb_add_user_cut(const ExprBuilder &function, ConstraintSense sense, CoeffT rhs);

	// IIS related
	void computeIIS();
	int _get_variable_upperbound_IIS(const VariableIndex &variable);
	int _get_variable_lowerbound_IIS(const VariableIndex &variable);
	int _get_constraint_IIS(const ConstraintIndex &constraint);

  private:
	MonotoneIndexer<int> m_variable_index;

	MonotoneIndexer<int> m_linear_constraint_index;

	MonotoneIndexer<int> m_quadratic_constraint_index;

	MonotoneIndexer<int> m_sos_constraint_index;

	MonotoneIndexer<int> m_cone_constraint_index;

	MonotoneIndexer<int> m_exp_cone_constraint_index;

	MonotoneIndexer<int> m_nl_constraint_index;

	// Store the nonlinear objectives
	int m_nl_objective_num = 0;
	std::vector<int> m_nl_objective_opcodes = {COPT_NL_SUM, 0};
	std::vector<double> m_nl_objective_constants;

	/* COPT part */
	std::unique_ptr<copt_prob, COPTfreemodelT> m_model;
};
