#pragma once

#include "solvers/ipopt/IpStdCInterface.h"
#include "pyoptinterface/nleval.hpp"
#include "pyoptinterface/dylib.hpp"
#include "pyoptinterface/solver_common.hpp"
#include <cmath>
#include <tuple>

#define APILIST            \
	B(CreateIpoptProblem); \
	B(FreeIpoptProblem);   \
	B(AddIpoptStrOption);  \
	B(AddIpoptNumOption);  \
	B(AddIpoptIntOption);  \
	B(IpoptSolve);

namespace ipopt
{
#define B DYLIB_EXTERN_DECLARE
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
	bool is_valid = false;
	// store results
	std::vector<double> x, g, mult_g, mult_x_L, mult_x_U;
	double obj_val;
};

struct IpoptModel
{
	/* Methods */
	IpoptModel();
	void close();

	VariableIndex add_variable(double lb = -INFINITY, double ub = INFINITY, double start = 0.0,
	                           const char *name = nullptr);
	double get_variable_lb(const VariableIndex &variable);
	double get_variable_ub(const VariableIndex &variable);
	void set_variable_lb(const VariableIndex &variable, double lb);
	void set_variable_ub(const VariableIndex &variable, double ub);
	void set_variable_bounds(const VariableIndex &variable, double lb, double ub);

	double get_variable_start(const VariableIndex &variable);
	void set_variable_start(const VariableIndex &variable, double start);

	std::string get_variable_name(const VariableIndex &variable);
	void set_variable_name(const VariableIndex &variable, const std::string &name);

	double get_variable_value(const VariableIndex &variable);

	std::string pprint_variable(const VariableIndex &variable);

	ParameterIndex add_parameter(double value = 0.0);
	void set_parameter(const ParameterIndex &parameter, double value);

	double get_obj_value();
	double get_constraint_primal(IndexT index);
	double get_constraint_dual(IndexT index);

	ConstraintIndex add_linear_constraint(const ScalarAffineFunction &f, ConstraintSense sense,
	                                      double rhs, const char *name = nullptr);
	ConstraintIndex add_linear_constraint(const ScalarAffineFunction &f, ConstraintSense sense,
	                                      const std::tuple<double, double> &interval,
	                                      const char *name = nullptr);
	ConstraintIndex add_linear_constraint(const ExprBuilder &f, ConstraintSense sense, double rhs,
	                                      const char *name = nullptr);
	ConstraintIndex add_linear_constraint(const ExprBuilder &f, ConstraintSense sense,
	                                      const std::tuple<double, double> &interval,
	                                      const char *name = nullptr);
	ConstraintIndex add_linear_constraint(const VariableIndex &f, ConstraintSense sense, double rhs,
	                                      const char *name = nullptr);
	ConstraintIndex add_linear_constraint(const VariableIndex &f, ConstraintSense sense,
	                                      const std::tuple<double, double> &interval,
	                                      const char *name = nullptr);

	ConstraintIndex add_quadratic_constraint(const ScalarQuadraticFunction &f,
	                                         ConstraintSense sense, double rhs,
	                                         const char *name = nullptr);
	ConstraintIndex add_quadratic_constraint(const ScalarQuadraticFunction &f,
	                                         ConstraintSense sense,
	                                         const std::tuple<double, double> &interval,
	                                         const char *name = nullptr);
	ConstraintIndex add_quadratic_constraint(const ExprBuilder &f, ConstraintSense sense,
	                                         double rhs, const char *name = nullptr);
	ConstraintIndex add_quadratic_constraint(const ExprBuilder &f, ConstraintSense sense,
	                                         const std::tuple<double, double> &interval,
	                                         const char *name = nullptr);

	template <typename T>
	void add_objective(const T &expr)
	{
		m_lq_model.add_objective(expr);
	}

	template <typename T>
	void set_objective(const T &expr, ObjectiveSense sense = ObjectiveSense::Minimize,
	                   bool clear_nl = false)
	{
		if (sense != ObjectiveSense::Minimize)
		{
			throw std::runtime_error(
			    "Currently Ipopt only supports ObjectiveSense::Minimize, please negate the "
			    "objective manually if you intend to maximize the objective");
		}

		m_lq_model.set_objective(expr);
		if (clear_nl)
		{
			clear_nl_objective();
		}
	}

	FunctionIndex _register_function(const AutodiffSymbolicStructure &structure);
	void _set_function_evaluator(const FunctionIndex &k, const AutodiffEvaluator &evaluator);
	bool _has_function_evaluator(const FunctionIndex &k);

	NLConstraintIndex _add_fn_constraint_bounds(const FunctionIndex &k,
	                                            const std::vector<VariableIndex> &xs,
	                                            const std::vector<ParameterIndex> &ps,
	                                            const std::vector<double> &lbs,
	                                            const std::vector<double> &ubs);

	NLConstraintIndex _add_fn_constraint_eq(const FunctionIndex &k,
	                                        const std::vector<VariableIndex> &xs,
	                                        const std::vector<ParameterIndex> &ps,
	                                        const std::vector<double> &eqs);

	void _add_fn_objective(const FunctionIndex &k, const std::vector<VariableIndex> &xs,
	                       const std::vector<ParameterIndex> &ps);

	void clear_nl_objective();

	void analyze_structure();
	void optimize();

	// load current solution as	initial guess
	void load_current_solution();

	// set options
	void set_raw_option_int(const std::string &name, int value);
	void set_raw_option_double(const std::string &name, double value);
	void set_raw_option_string(const std::string &name, const std::string &value);

	/* Members */

	size_t n_variables = 0;
	size_t n_constraints = 0;

	std::vector<double> m_var_lb, m_var_ub, m_var_init;
	std::vector<double> m_con_lb, m_con_ub;

	Hashmap<IndexT, std::string> m_var_names, m_con_names;

	size_t m_jacobian_nnz = 0;
	std::vector<size_t> m_jacobian_rows, m_jacobian_cols;

	size_t m_hessian_nnz = 0;
	std::vector<size_t> m_hessian_rows, m_hessian_cols;
	Hashmap<VariablePair, size_t> m_hessian_index_map;

	NonlinearFunctionEvaluator m_function_model;
	LinearQuadraticEvaluator m_lq_model;

	// The options of the Ipopt solver, we cache them before constructing the m_problem
	Hashmap<std::string, int> m_options_int;
	Hashmap<std::string, double> m_options_num;
	Hashmap<std::string, std::string> m_options_str;

	IpoptResult m_result;
	enum ApplicationReturnStatus m_status;

	std::unique_ptr<IpoptProblemInfo, IpoptfreeproblemT> m_problem = nullptr;
};

using IpoptModelMixin = CommercialSolverMixin<IpoptModel>;