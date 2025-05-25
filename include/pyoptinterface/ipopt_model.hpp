#pragma once

#include "solvers/ipopt/IpStdCInterface.h"
#include "pyoptinterface/nlexpr.hpp"
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

struct IpoptModel : public OnesideLinearConstraintMixin<IpoptModel>,
                    public TwosideLinearConstraintMixin<IpoptModel>,
                    public OnesideQuadraticConstraintMixin<IpoptModel>,
                    public TwosideQuadraticConstraintMixin<IpoptModel>,
                    public LinearObjectiveMixin<IpoptModel>,
                    public PPrintMixin<IpoptModel>,
                    public GetValueMixin<IpoptModel>
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

	double get_obj_value();
	int _constraint_internal_index(const ConstraintIndex &constraint);
	double get_constraint_primal(const ConstraintIndex &constraint);
	double get_constraint_dual(const ConstraintIndex &constraint);

	ConstraintIndex add_linear_constraint(const ScalarAffineFunction &f, ConstraintSense sense,
	                                      double rhs, const char *name = nullptr);
	ConstraintIndex add_linear_constraint(const ScalarAffineFunction &f,
	                                      const std::tuple<double, double> &interval,
	                                      const char *name = nullptr);

	ConstraintIndex add_quadratic_constraint(const ScalarQuadraticFunction &f,
	                                         ConstraintSense sense, double rhs,
	                                         const char *name = nullptr);
	ConstraintIndex add_quadratic_constraint(const ScalarQuadraticFunction &f,
	                                         const std::tuple<double, double> &interval,
	                                         const char *name = nullptr);

	void set_objective(const ScalarAffineFunction &expr, ObjectiveSense sense);
	void set_objective(const ScalarQuadraticFunction &expr, ObjectiveSense sense);
	void set_objective(const ExprBuilder &expr, ObjectiveSense sense);

	void _set_linear_objective(const ScalarAffineFunction &expr);
	void _set_quadratic_objective(const ScalarQuadraticFunction &expr);

	// new implementation
	size_t n_graph_instances = 0;
	std::vector<std::vector<int>> m_graph_instance_variables;
	std::vector<std::vector<double>> m_graph_instance_constants;
	// record graph instances with constraint output and objective output
	struct GraphInstancesInfo
	{
		// hash of this graph instance
		std::vector<uint64_t> hashes;
		// index of this graph instance
		std::vector<int> instance_indices;

		size_t n_instances_since_last_aggregation;
	} nl_constraint_info, nl_objective_info;

	// length = n_graph_instances
	struct GraphInstancesGroupInfo
	{
		// which group it belongs to
		std::vector<int> group_indices;
		// the number in that group
		std::vector<int> group_orders;
	} nl_constraint_group_info, nl_objective_group_info;

	// graph groups
	struct
	{
		Hashmap<uint64_t, int> hash_to_group;
		size_t n_group = 0;
		std::vector<int> representative_graph_indices;
		std::vector<std::vector<int>> instance_indices;
		std::vector<AutodiffSymbolicStructure> autodiff_structures;
		std::vector<ConstraintAutodiffEvaluator> autodiff_evaluators;

		// where to store the hessian matrix, each group length = n_instance * local_hessian_nnz
		std::vector<std::vector<int>> hessian_indices;
	} nl_constraint_groups;

	struct
	{
		Hashmap<uint64_t, int> hash_to_group;
		size_t n_group = 0;
		std::vector<int> representative_graph_indices;
		std::vector<std::vector<int>> instance_indices;
		std::vector<AutodiffSymbolicStructure> autodiff_structures;
		std::vector<ObjectiveAutodiffEvaluator> autodiff_evaluators;

		// where to store the gradient vector, each group length = n_instance * local_jacobian_nnz
		std::vector<std::vector<int>> gradient_indices;
		// where to store the hessian matrix, each group length = n_instance * local_hessian_nnz
		std::vector<std::vector<int>> hessian_indices;
	} nl_objective_groups;

	int add_graph_index();
	void record_graph_hash(size_t graph_index, const ExpressionGraph &graph);
	int aggregate_graph_constraint_groups();
	int get_graph_constraint_group_representative(int group_index) const;
	int aggregate_graph_objective_groups();
	int get_graph_objective_group_representative(int group_index) const;

	void assign_constraint_group_autodiff_structure(int group_index,
	                                                const AutodiffSymbolicStructure &structure);
	void assign_constraint_group_autodiff_evaluator(int group_index,
	                                                const ConstraintAutodiffEvaluator &evaluator);
	void assign_objective_group_autodiff_structure(int group_index,
	                                               const AutodiffSymbolicStructure &structure);
	void assign_objective_group_autodiff_evaluator(int group_index,
	                                               const ObjectiveAutodiffEvaluator &evaluator);

	ConstraintIndex add_single_nl_constraint(size_t graph_index, const ExpressionGraph &graph,
	                                         double lb, double ub);

	// void clear_nl_objective();

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

	size_t n_nl_constraints = 0;
	/*
	 * record the constraint indices mapping from the monotonic one (the order of adding
	 * constraint) to the reordered one (linear, quadratic, NL group 0 -> con0, con1 ,..., conN0, NL
	 * group1 -> con0, con1,..., conN1)
	 */
	// these two vectors are maintained when adding NL constraint
	// which graph instance this constraint belongs to
	std::vector<int> nl_constraint_graph_instance_indices;
	// the order of this constraint in the graph instance
	std::vector<int> nl_constraint_graph_instance_orders;

	// these two vectors are constructed before optimization
	// ext means the external monotonic order
	// int means the internal order that passes to Ipopt
	std::vector<int> nl_constraint_map_ext2int;

	// we need a sparse vector to store the gradient
	std::vector<double> sparse_gradient_values;
	std::vector<int> sparse_gradient_indices;

	std::vector<double> m_var_lb, m_var_ub, m_var_init;
	std::vector<double> m_linear_con_lb, m_linear_con_ub, m_quadratic_con_lb, m_quadratic_con_ub,
	    m_nl_con_lb, m_nl_con_ub, m_con_lb, m_con_ub;

	Hashmap<IndexT, std::string> m_var_names;

	size_t m_jacobian_nnz = 0;
	std::vector<int> m_jacobian_rows, m_jacobian_cols;

	size_t m_hessian_nnz = 0;
	std::vector<int> m_hessian_rows, m_hessian_cols;
	Hashmap<std::tuple<int, int>, int> m_hessian_index_map;

	LinearEvaluator m_linear_con_evaluator;
	QuadraticEvaluator m_quadratic_con_evaluator;

	std::optional<LinearEvaluator> m_linear_obj_evaluator;
	std::optional<QuadraticEvaluator> m_quadratic_obj_evaluator;

	// The options of the Ipopt solver, we cache them before constructing the m_problem
	Hashmap<std::string, int> m_options_int;
	Hashmap<std::string, double> m_options_num;
	Hashmap<std::string, std::string> m_options_str;

	IpoptResult m_result;
	enum ApplicationReturnStatus m_status;

	std::unique_ptr<IpoptProblemInfo, IpoptfreeproblemT> m_problem = nullptr;
};
