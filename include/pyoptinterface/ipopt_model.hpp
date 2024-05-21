#pragma once

#include "solvers/ipopt/IpStdCInterface.h"
#include "pyoptinterface/nlcore.hpp"

#define APILIST                 \
	B(CreateIpoptProblem);      \
	B(FreeIpoptProblem);        \
	B(AddIpoptStrOption);       \
	B(AddIpoptNumOption);       \
	B(AddIpoptIntOption);       \
	B(OpenIpoptOutputFile);     \
	B(SetIpoptProblemScaling);  \
	B(SetIntermediateCallback); \
	B(IpoptSolve);              \
	B(GetIpoptCurrentIterate);  \
	B(GetIpoptCurrentViolations);

namespace ipopt
{
#define B(f) extern decltype(&::f) f

APILIST

#undef B

bool is_library_loaded();

bool load_library(const std::string &path);
} // namespace ipopt

struct IpoptfreeproblemT
{
	void operator()(IpoptProblemInfo *model) const
	{
		ipopt::FreeIpoptProblem(model);
	};
};

struct IpoptResult
{
	// store results
	std::vector<double> x, g, mult_g, mult_x_L, mult_x_U;
	double obj_val;
};

struct IpoptModel
{
	/* Methods */
	IpoptModel();

	VariableIndex add_variable(double lb = -INFINITY, double ub = INFINITY, double start = 0.0);
	void change_variable_lb(const VariableIndex &variable, double lb);
	void change_variable_ub(const VariableIndex &variable, double ub);
	void change_variable_bounds(const VariableIndex &variable, double lb, double ub);
	double get_variable_value(const VariableIndex &variable);

	ParameterIndex add_parameter(double value = 0.0);
	void set_parameter(const ParameterIndex &parameter, double value);

	ConstraintIndex add_linear_constraint(const ScalarAffineFunction &f, ConstraintSense sense,
	                                      double rhs);
	ConstraintIndex add_linear_constraint(const ScalarAffineFunction &f, ConstraintSense sense,
	                                      double lb, double ub);
	ConstraintIndex add_linear_constraint(const ExprBuilder &f, ConstraintSense sense, double rhs);
	ConstraintIndex add_linear_constraint(const ExprBuilder &f, ConstraintSense sense, double lb,
	                                      double ub);
	ConstraintIndex add_linear_constraint(const VariableIndex &f, ConstraintSense sense,
	                                      double rhs);
	ConstraintIndex add_linear_constraint(const VariableIndex &f, ConstraintSense sense, double lb,
	                                      double ub);

	ConstraintIndex add_quadratic_constraint(const ScalarQuadraticFunction &f,
	                                         ConstraintSense sense, double rhs);
	ConstraintIndex add_quadratic_constraint(const ScalarQuadraticFunction &f,
	                                         ConstraintSense sense, double lb, double ub);
	ConstraintIndex add_quadratic_constraint(const ExprBuilder &f, ConstraintSense sense,
	                                         double rhs);
	ConstraintIndex add_quadratic_constraint(const ExprBuilder &f, ConstraintSense sense, double lb,
	                                         double ub);

	template <typename T>
	void add_objective(const T &expr)
	{
		m_lq_model.add_objective(expr);
	}

	FunctionIndex register_function(ADFunD &f, const std::string &name);

	NLConstraintIndex add_empty_nl_constraint(int dim, ConstraintSense sense,
	                                          const std::vector<double> &rhss);
	NLConstraintIndex add_empty_nl_constraint(int dim, ConstraintSense sense,
	                                          const std::vector<double> &lbs,
	                                          const std::vector<double> &ubs);

	NLConstraintIndex add_nl_constraint(const FunctionIndex &k,
	                                    const std::vector<VariableIndex> &xs,
	                                    const std::vector<ParameterIndex> &ps,
	                                    ConstraintSense sense, const std::vector<double> &rhss);
	NLConstraintIndex add_nl_constraint(const FunctionIndex &k,
	                                    const std::vector<VariableIndex> &xs,
	                                    const std::vector<double> &ps,
	                                    ConstraintSense sense, const std::vector<double> &rhss);
	NLConstraintIndex add_nl_constraint(const FunctionIndex &k,
	                                    const std::vector<VariableIndex> &xs, ConstraintSense sense,
	                                    const std::vector<double> &rhss);

	NLConstraintIndex add_nl_constraint(const FunctionIndex &k,
	                                    const std::vector<VariableIndex> &xs,
	                                    const std::vector<ParameterIndex> &ps,
	                                    ConstraintSense sense, const std::vector<double> &lbs,
	                                    const std::vector<double> &ubs);
	NLConstraintIndex add_nl_constraint(const FunctionIndex &k,
	                                    const std::vector<VariableIndex> &xs,
	                                    const std::vector<double> &ps,
	                                    ConstraintSense sense, const std::vector<double> &lbs,
	                                    const std::vector<double> &ubs);
	NLConstraintIndex add_nl_constraint(const FunctionIndex &k,
	                                    const std::vector<VariableIndex> &xs, ConstraintSense sense,
	                                    const std::vector<double> &lbs,
	                                    const std::vector<double> &ubs);

	void add_nl_expression(const NLConstraintIndex &constraint, const FunctionIndex &k,
	                       const std::vector<VariableIndex> &xs,
	                       const std::vector<ParameterIndex> &ps);
	void add_nl_expression(const NLConstraintIndex &constraint, const FunctionIndex &k,
	                       const std::vector<VariableIndex> &xs,
	                       const std::vector<double> &ps);
	void add_nl_expression(const NLConstraintIndex &constraint, const FunctionIndex &k,
	                       const std::vector<VariableIndex> &xs);

	void add_nl_objective(const FunctionIndex &k, const std::vector<VariableIndex> &xs,
	                      const std::vector<ParameterIndex> &ps);

	void optimize();

	// set options
	void set_option_int(const std::string &name, int value);
	void set_option_num(const std::string &name, double value);
	void set_option_str(const std::string &name, const std::string &value);

	/* Members */

	size_t n_variables = 0;
	size_t n_constraints = 0;

	std::vector<double> m_var_lb, m_var_ub, m_var_init;
	std::vector<double> m_con_lb, m_con_ub;

	size_t m_jacobian_nnz = 0;
	std::vector<size_t> m_jacobian_rows, m_jacobian_cols;

	size_t m_hessian_nnz = 0;
	std::vector<size_t> m_hessian_rows, m_hessian_cols;
	Hashmap<VariablePair, size_t> m_hessian_index_map;

	NonlinearFunctionModel m_function_model;
	LinearQuadraticModel m_lq_model;

	// The options of the Ipopt solver, we cache them before constructing the m_problem
	Hashmap<std::string, int> m_options_int;
	Hashmap<std::string, double> m_options_num;
	Hashmap<std::string, std::string> m_options_str;

	IpoptResult m_result;
	enum ApplicationReturnStatus m_status;

	std::unique_ptr<IpoptProblemInfo, IpoptfreeproblemT> m_problem = nullptr;
};
