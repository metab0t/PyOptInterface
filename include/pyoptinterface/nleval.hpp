#pragma once

#include <cstdint>
#include <vector>

#include "pyoptinterface/core.hpp"
#include "pyoptinterface/nlexpr.hpp"

enum class HessianSparsityType
{
	Upper,
	Lower
};

struct AutodiffSymbolicStructure
{
	size_t nx = 0, np = 0, ny = 0;

	std::vector<size_t> m_jacobian_rows, m_jacobian_cols;
	size_t m_jacobian_nnz = 0;
	std::vector<size_t> m_hessian_rows, m_hessian_cols;
	size_t m_hessian_nnz = 0;

	bool has_parameter = false;
	bool has_jacobian = false;
	bool has_hessian = false;
};

// define the jit-compiled function pointer
using f_funcptr = void (*)(const double *x, const double *p, double *y, const int *xi);
using jacobian_funcptr = void (*)(const double *x, const double *p, double *jacobian,
                                  const int *xi);
using additive_grad_funcptr = void (*)(const double *x, const double *p, double *grad,
                                       const int *xi, const int *gradi);
using hessian_funcptr = void (*)(const double *x, const double *p, const double *w, double *hessian,
                                 const int *xi, const int *hessiani);

// no parameter version
using f_funcptr_noparam = void (*)(const double *x, double *y, const int *xi);
using jacobian_funcptr_noparam = void (*)(const double *x, double *jacobian, const int *xi);
using additive_grad_funcptr_noparam = void (*)(const double *x, double *grad, const int *xi,
                                               const int *gradi);
using hessian_funcptr_noparam = void (*)(const double *x, const double *w, double *hessian,
                                         const int *xi, const int *hessiani);

struct ConstraintAutodiffEvaluator
{
	union {
		f_funcptr p = nullptr;
		f_funcptr_noparam nop;
	} f_eval;
	union {
		jacobian_funcptr p = nullptr;
		jacobian_funcptr_noparam nop;
	} jacobian_eval;
	union {
		hessian_funcptr p = nullptr;
		hessian_funcptr_noparam nop;
	} hessian_eval;

	ConstraintAutodiffEvaluator() = default;

	ConstraintAutodiffEvaluator(bool has_parameter, uintptr_t fp, uintptr_t jp, uintptr_t hp);
};

struct ObjectiveAutodiffEvaluator
{
	union {
		f_funcptr p = nullptr;
		f_funcptr_noparam nop;
	} f_eval;
	union {
		additive_grad_funcptr p = nullptr;
		additive_grad_funcptr_noparam nop;
	} grad_eval;
	union {
		hessian_funcptr p = nullptr;
		hessian_funcptr_noparam nop;
	} hessian_eval;

	ObjectiveAutodiffEvaluator() = default;

	ObjectiveAutodiffEvaluator(bool has_parameter, uintptr_t fp, uintptr_t ajp, uintptr_t hp);
};

#define restrict __restrict

struct LinearEvaluator
{
	int n_constraints = 0;

	std::vector<double> coefs;
	std::vector<int> indices;

	std::vector<double> constant_values;
	std::vector<int> constant_indices;

	std::vector<int> constraint_intervals = {0};

	void add_row(const ScalarAffineFunction &f);

	void eval_function(const double *restrict x, double *restrict f);
	void analyze_jacobian_structure(size_t &global_jacobian_nnz,
	                                std::vector<int> &global_jacobian_rows,
	                                std::vector<int> &global_jacobian_cols) const;
	void eval_jacobian(const double *restrict x, double *restrict jacobian) const;
};

struct QuadraticEvaluator
{
	int n_constraints = 0;

	std::vector<double> diag_coefs;
	std::vector<int> diag_indices;
	std::vector<int> diag_intervals = {0};

	std::vector<double> offdiag_coefs;
	std::vector<int> offdiag_rows;
	std::vector<int> offdiag_cols;
	std::vector<int> offdiag_intervals = {0};

	std::vector<double> linear_coefs;
	std::vector<int> linear_indices;
	std::vector<int> linear_intervals = {0};

	std::vector<double> linear_constant_values;
	std::vector<int> linear_constant_indices;

	int jacobian_nnz = 0;

	// This is the constant part
	// jacobian_constant.size() = jacobian_nnz
	std::vector<double> jacobian_constant;
	std::vector<int> jacobian_variable_indices;
	std::vector<int> jacobian_constraint_intervals = {0};
	// jacobian_diag_indices.size() = diag_indices.size()
	std::vector<int> jacobian_diag_indices;
	// jacobian_offdiag_row_indices.size() = offdiag_rows.size()
	std::vector<int> jacobian_offdiag_row_indices;
	std::vector<int> jacobian_offdiag_col_indices;

	// Hessian
	// = diag_coefs.size()
	std::vector<int> hessian_diag_indices;
	// = offdiag_coefs.size()
	std::vector<int> hessian_offdiag_indices;

	void add_row(const ScalarQuadraticFunction &f);

	void eval_function(const double *restrict x, double *restrict f) const;
	void analyze_jacobian_structure(size_t row_base, size_t &global_jacobian_nnz,
	                                std::vector<int> &global_jacobian_rows,
	                                std::vector<int> &global_jacobian_cols) const;
	void eval_jacobian(const double *restrict x, double *restrict jacobian) const;
	void analyze_hessian_structure(size_t &global_hessian_nnz,
	                               std::vector<int> &global_hessian_rows,
	                               std::vector<int> &global_hessian_cols,
	                               Hashmap<std::tuple<int, int>, int> &hessian_index_map,
	                               HessianSparsityType hessian_type);
	void eval_lagrangian_hessian(const double *restrict lambda, double *restrict hessian) const;
};

struct NonlinearEvaluator
{
	// How many graph instances are there
	size_t n_graph_instances = 0;
	// record the inputs of graph instances
	struct GraphInput
	{
		std::vector<int> variables;
		std::vector<double> constants;
	};
	std::vector<GraphInput> graph_inputs;
	// record graph instances with constraint output and objective output
	struct GraphHash
	{
		// hash of this graph instance
		uint64_t hash;
		// index of this graph instance
		int index;
	};
	struct GraphHashes
	{
		std::vector<GraphHash> hashes;
		size_t n_hashes_since_last_aggregation;
	} constraint_graph_hashes, objective_graph_hashes;

	// length = n_graph_instances
	// record which group this graph instance belongs to
	struct GraphGroupMembership
	{
		// which group it belongs to
		int group;
		// the rank in that group
		int rank;
	};
	std::vector<GraphGroupMembership> constraint_group_memberships, objective_group_memberships;
	// record which index of constraint this graph starts
	std::vector<int> constraint_indices_offsets;

	// graph groups
	struct ConstraintGraphGroup
	{
		std::vector<int> instance_indices;
		AutodiffSymbolicStructure autodiff_structure;
		ConstraintAutodiffEvaluator autodiff_evaluator;

		// where to store the hessian matrix
		// length = instance_indices.size() * hessian_nnz
		std::vector<int> hessian_indices;
	};
	std::vector<ConstraintGraphGroup> constraint_groups;
	Hashmap<uint64_t, int> hash_to_constraint_group;

	struct ObjectiveGraphGroup
	{
		std::vector<int> instance_indices;
		AutodiffSymbolicStructure autodiff_structure;
		ObjectiveAutodiffEvaluator autodiff_evaluator;
		// where to store the gradient vector
		// length = instance_indices.size() * jacobian_nnz
		std::vector<int> gradient_indices;
		// where to store the hessian matrix
		// length = instance_indices.size() * hessian_nnz
		std::vector<int> hessian_indices;
	};
	std::vector<ObjectiveGraphGroup> objective_groups;
	Hashmap<uint64_t, int> hash_to_objective_group;

	int add_graph_instance();
	void finalize_graph_instance(size_t graph_index, const ExpressionGraph &graph);
	int aggregate_constraint_groups();
	int get_constraint_group_representative(int group_index) const;
	int aggregate_objective_groups();
	int get_objective_group_representative(int group_index) const;

	void assign_constraint_group_autodiff_structure(int group_index,
	                                                const AutodiffSymbolicStructure &structure);
	void assign_constraint_group_autodiff_evaluator(int group_index,
	                                                const ConstraintAutodiffEvaluator &evaluator);
	void assign_objective_group_autodiff_structure(int group_index,
	                                               const AutodiffSymbolicStructure &structure);
	void assign_objective_group_autodiff_evaluator(int group_index,
	                                               const ObjectiveAutodiffEvaluator &evaluator);

	void calculate_constraint_graph_instances_offset();

	// functions to evaluate the nonlinear constraints and objectives

	// f
	void eval_constraints(const double *restrict x, double *restrict f) const;
	double eval_objective(const double *restrict x) const;

	// first order derivative
	void analyze_constraints_jacobian_structure(size_t row_base, size_t &global_jacobian_nnz,
	                                            std::vector<int> &global_jacobian_rows,
	                                            std::vector<int> &global_jacobian_cols);
	void analyze_objective_gradient_structure(std::vector<int> &global_gradient_cols,
	                                          Hashmap<int, int> &sparse_gradient_map);

	void eval_constraints_jacobian(const double *restrict x, double *restrict jacobian) const;
	void eval_objective_gradient(const double *restrict x, double *restrict grad_f) const;

	// second order derivative
	void analyze_constraints_hessian_structure(
	    size_t &global_hessian_nnz, std::vector<int> &global_hessian_rows,
	    std::vector<int> &global_hessian_cols,
	    Hashmap<std::tuple<int, int>, int> &hessian_index_map, HessianSparsityType hessian_type);
	void analyze_objective_hessian_structure(size_t &global_hessian_nnz,
	                                         std::vector<int> &global_hessian_rows,
	                                         std::vector<int> &global_hessian_cols,
	                                         Hashmap<std::tuple<int, int>, int> &hessian_index_map,
	                                         HessianSparsityType hessian_type);

	void eval_lagrangian_hessian(const double *restrict x, const double *restrict lambda,
	                             const double sigma, double *restrict hessian) const;
};