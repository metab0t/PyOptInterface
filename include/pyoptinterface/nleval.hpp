#pragma once

#include <cstdint>
#include <vector>

#include "core.hpp"

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