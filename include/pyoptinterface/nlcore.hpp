#pragma once

#include "cppad/cppad.hpp"
#include "core.hpp"

struct NLConstraintIndex
{
	// the index in all constraints
	IndexT index;
	IndexT dim;

	NLConstraintIndex() = default;
};

static const std::string opt_options = "no_compare_op no_conditional_skip no_cumulative_sum_op";

template <typename Base>
CppAD::ADFun<Base> dense_jacobian(const CppAD::ADFun<Base> &f)
{
	using CppAD::AD;
	using CppAD::ADFun;
	using CppAD::Independent;

	size_t nx = f.Domain();
	size_t ny = f.Range();
	size_t np = f.size_dyn_ind();
	std::vector<AD<Base>> apx(np + nx), ax(nx), ap(np), aj(nx * ny);
	ADFun<AD<Base>, Base> af = f.base2ad();
	Independent(apx);
	for (size_t i = 0; i < np; i++)
		ap[i] = apx[i];
	for (size_t i = 0; i < nx; i++)
		ax[i] = apx[np + i];
	af.new_dynamic(ap);
	aj = af.jacobianan(ax);
	ADFun<Base> jac;
	jac.Dependent(apx, aj);

	jac.optimize(opt_options);

	return jac;
}

enum class HessianSparsityType
{
	Full,
	Upper,
	Lower
};

using sparsity_pattern_t = CppAD::sparse_rc<std::vector<size_t>>;

struct JacobianHessianSparsityPattern
{
	sparsity_pattern_t jacobian;
	sparsity_pattern_t hessian;
	sparsity_pattern_t reduced_hessian;
};

template <typename Base>
JacobianHessianSparsityPattern jacobian_hessian_sparsity(CppAD::ADFun<Base> &f,
                                                         HessianSparsityType hessian_sparsity)
{
	using s_vector = std::vector<size_t>;
	using CppAD::sparse_rc;

	size_t nx = f.Domain();
	size_t ny = f.Range();
	size_t np = f.size_dyn_ind();

	JacobianHessianSparsityPattern jachess;

	// We must compute the jacobian sparsity first
	{
		// sparsity pattern for the identity matrix
		size_t nr = nx;
		size_t nc = nx;
		size_t nnz_in = nx;
		sparsity_pattern_t pattern_jac_in(nr, nc, nnz_in);
		for (size_t k = 0; k < nnz_in; k++)
		{
			size_t r = k;
			size_t c = k;
			pattern_jac_in.set(k, r, c);
		}
		// compute sparsity pattern for J(x) = F'(x)
		const bool transpose = false;
		const bool dependency = false;
		const bool internal_bool = true;
		sparsity_pattern_t pattern_jac;
		f.for_jac_sparsity(pattern_jac_in, transpose, dependency, internal_bool, pattern_jac);

		jachess.jacobian = pattern_jac;
	}

	std::vector<bool> select_range(ny, true);
	const bool transpose = false;
	const bool internal_bool = true;
	sparsity_pattern_t pattern_hes;
	f.rev_hes_sparsity(select_range, transpose, internal_bool, pattern_hes);

	jachess.hessian = pattern_hes;
	if (hessian_sparsity == HessianSparsityType::Full)
	{
		jachess.reduced_hessian = pattern_hes;
		return jachess;
	}

	// Filter the sparsity pattern
	sparsity_pattern_t pattern_hes_partial;
	pattern_hes_partial.resize(nx, nx, 0);
	if (hessian_sparsity == HessianSparsityType::Upper)
	{
		for (int i = 0; i < pattern_hes.nnz(); i++)
		{
			auto r = pattern_hes.row()[i];
			auto c = pattern_hes.col()[i];
			if (r <= c)
			{
				pattern_hes_partial.push_back(r, c);
			}
		}
	}
	else if (hessian_sparsity == HessianSparsityType::Lower)
	{
		for (int i = 0; i < pattern_hes.nnz(); i++)
		{
			auto r = pattern_hes.row()[i];
			auto c = pattern_hes.col()[i];
			if (r >= c)
			{
				pattern_hes_partial.push_back(r, c);
			}
		}
	}
	jachess.reduced_hessian = pattern_hes_partial;
	return jachess;
}

// [p, x] -> jacobian
template <typename Base>
CppAD::ADFun<Base> sparse_jacobian(const CppAD::ADFun<Base> &f,
                                   const sparsity_pattern_t &pattern_jac)
{
	using CppAD::AD;
	using CppAD::ADFun;
	using CppAD::Independent;

	size_t nx = f.Domain();
	size_t ny = f.Range();
	size_t np = f.size_dyn_ind();
	std::vector<AD<Base>> apx(np + nx), ax(nx), ap(np);
	ADFun<AD<Base>, Base> af = f.base2ad();
	Independent(apx);
	for (size_t i = 0; i < np; i++)
		ap[i] = apx[i];
	for (size_t i = 0; i < nx; i++)
		ax[i] = apx[np + i];
	af.new_dynamic(ap);
	CppAD::sparse_rcv<std::vector<size_t>, std::vector<AD<Base>>> subset(pattern_jac);
	CppAD::sparse_jac_work work;
	std::string coloring = "cppad";
	size_t n_color = af.sparse_jac_rev(ax, subset, pattern_jac, coloring, work);
	ADFun<double> jacobian;
	jacobian.Dependent(apx, subset.val());

	const std::string opt_options = "no_compare_op no_conditional_skip no_cumulative_sum_op";
	jacobian.optimize(opt_options);

	return jacobian;
}

// [p, w, x] -> \Sigma w_i * Hessian_i
template <typename Base>
CppAD::ADFun<Base> sparse_hessian(const CppAD::ADFun<Base> &f,
                                  const sparsity_pattern_t &pattern_hes,
                                  const sparsity_pattern_t &pattern_subset)
{
	using CppAD::AD;
	using CppAD::ADFun;
	using CppAD::Independent;

	size_t nx = f.Domain();
	size_t ny = f.Range();
	size_t np = f.size_dyn_ind();
	size_t nw = ny;
	std::vector<AD<Base>> apwx(np + nw + nx), ax(nx), ap(np), aw(nw);
	ADFun<AD<Base>, Base> af = f.base2ad();
	Independent(apwx);
	for (size_t i = 0; i < np; i++)
		ap[i] = apwx[i];
	for (size_t i = 0; i < nw; i++)
		aw[i] = apwx[np + i];
	for (size_t i = 0; i < nx; i++)
		ax[i] = apwx[np + nw + i];
	af.new_dynamic(ap);
	CppAD::sparse_rcv<std::vector<size_t>, std::vector<AD<Base>>> subset(pattern_subset);
	CppAD::sparse_hes_work work;
	std::string coloring = "cppad.symmetric";
	size_t n_sweep = af.sparse_hes(ax, aw, subset, pattern_hes, coloring, work);
	ADFun<double> hessian;
	hessian.Dependent(apwx, subset.val());

	const std::string opt_options = "no_compare_op no_conditional_skip no_cumulative_sum_op";
	hessian.optimize(opt_options);

	return hessian;
}

using ADFunD = CppAD::ADFun<double>;
using cpp_graph = CppAD::cpp_graph;

// define the jit-compiled function pointer
using f_funcptr = void (*)(const double *x, const double *p, double *y, const size_t *xi,
                           const size_t *pi);
using jacobian_funcptr = void (*)(const double *x, const double *p, double *jacobian,
                                  const size_t *xi, const size_t *pi);
using additive_grad_funcptr = void (*)(const double *x, const double *p, double *grad,
                                       const size_t *xi, const size_t *pi, const size_t *gradi);
using hessian_funcptr = void (*)(const double *x, const double *p, const double *w, double *hessian,
                                 const size_t *xi, const size_t *pi, const size_t *hessiani);

// no parameter version
using f_funcptr_noparam = void (*)(const double *x, double *y, const size_t *xi);
using jacobian_funcptr_noparam = void (*)(const double *x, double *jacobian, const size_t *xi);
using additive_grad_funcptr_noparam = void (*)(const double *x, double *grad, const size_t *xi,
                                               const size_t *gradi);
using hessian_funcptr_noparam = void (*)(const double *x, const double *w, double *hessian,
                                         const size_t *xi, const size_t *hessiani);

struct NonlinearFunction
{
	std::string name;
	size_t nx = 0, np = 0, ny = 0;
	bool has_parameter = false;

	std::vector<size_t> m_jacobian_rows, m_jacobian_cols;
	size_t m_jacobian_nnz = 0;
	std::vector<size_t> m_hessian_rows, m_hessian_cols;
	size_t m_hessian_nnz = 0;

	cpp_graph f_graph, jacobian_graph, hessian_graph;

	union {
		f_funcptr p = nullptr;
		f_funcptr_noparam nop;
	} f_eval;
	union {
		jacobian_funcptr p = nullptr;
		jacobian_funcptr_noparam nop;
	} jacobian_eval;
	union {
		additive_grad_funcptr p = nullptr;
		additive_grad_funcptr_noparam nop;
	} grad_eval;
	union {
		hessian_funcptr p = nullptr;
		hessian_funcptr_noparam nop;
	} hessian_eval;

	NonlinearFunction(ADFunD &f_, const std::string &name_);

	void assign_evaluators(uintptr_t fp, uintptr_t jp, uintptr_t ajp, uintptr_t hp);
};

struct FunctionInstance
{
	std::vector<size_t> xs, ps;
	// The output in all outputs
	size_t y_start;
	// defaults to y_start, some optimizers only need the nonlinear parts
	size_t eval_y_start;
	size_t jacobian_start;
	std::vector<size_t> hessian_indices;
	std::vector<size_t> grad_indices;
};

using FunctionInstances = std::vector<FunctionInstance>;

struct ParameterIndex
{
	IndexT index;

	ParameterIndex() = default;
	ParameterIndex(IndexT v) : index(v)
	{
	}
};

struct FunctionIndex
{
	IndexT index;

	FunctionIndex() = default;
	FunctionIndex(IndexT v) : index(v)
	{
	}
};

struct ConstantDelta
{
	double c;
	size_t yi;
	ConstantDelta() = default;
	ConstantDelta(double c_, size_t yi_) : c(c_), yi(yi_)
	{
	}
};

struct AffineDelta
{
	double c;
	size_t xi;
	size_t yi;
	AffineDelta() = default;
	AffineDelta(double c_, size_t xi_, size_t yi_) : c(c_), xi(xi_), yi(yi_)
	{
	}
};

size_t add_gradient_column(size_t column, size_t &gradient_nnz, std::vector<size_t> &gradient_cols,
                           Hashmap<size_t, size_t> &grad_index_map);
size_t add_hessian_index(size_t x1, size_t x2, size_t &m_hessian_nnz,
                         std::vector<size_t> &m_hessian_rows, std::vector<size_t> &m_hessian_cols,
                         Hashmap<VariablePair, size_t> &m_hessian_index_map,
                         HessianSparsityType hessian_sparsity_type);

struct LinearQuadraticModel
{
	std::vector<ScalarAffineFunction> linear_constraints;
	std::vector<size_t> linear_constraint_indices;

	std::vector<ScalarQuadraticFunction> quadratic_constraints;
	std::vector<size_t> quadratic_constraint_indices;

	ExprBuilder lq_objective;
	std::vector<size_t> lq_objective_hessian_indices;

	// jacobian[yi] += c
	std::vector<ConstantDelta> jacobian_constants;
	// jacobian[yi] += c * x[xi]
	std::vector<AffineDelta> jacobian_linear_terms;

	// grad[yi] += c
	std::vector<ConstantDelta> gradient_constants;
	// grad[yi] += c * x[xi]
	std::vector<AffineDelta> gradient_linear_terms;

	// hessian[yi] += lambda[xi] * c
	std::vector<AffineDelta> constraint_hessian_linear_terms;
	// hessian[yi] += sigma * c
	std::vector<ConstantDelta> objective_hessian_linear_terms;

	void add_linear_constraint(const ScalarAffineFunction &f, size_t y);
	void add_quadratic_constraint(const ScalarQuadraticFunction &f, size_t y);

	template <typename T>
	void add_objective(const T &expr)
	{
		lq_objective += expr;
	}

	void analyze_jacobian_structure(size_t &m_jacobian_nnz, std::vector<size_t> &m_jacobian_rows,
	                                std::vector<size_t> &m_jacobian_cols);
	void analyze_dense_gradient_structure();
	void analyze_sparse_gradient_structure(size_t &gradient_nnz, std::vector<size_t> &gradient_cols,
	                                       Hashmap<size_t, size_t> &gradient_index_map);
	void analyze_hessian_structure(size_t &m_hessian_nnz, std::vector<size_t> &m_hessian_rows,
	                               std::vector<size_t> &m_hessian_cols,
	                               Hashmap<VariablePair, size_t> &m_hessian_index_map,
	                               HessianSparsityType hessian_sparsity_type);

#define restrict __restrict

	void eval_objective(const double *restrict x, double *restrict y);

	void eval_objective_gradient(const double *restrict x, double *restrict grad);

	void eval_constraint(const double *restrict x, double *restrict con);

	void eval_constraint_jacobian(const double *restrict x, double *restrict jacobian);

	void eval_lagrangian_hessian(const double *restrict x, const double *restrict sigma,
	                             const double *restrict lambda, double *restrict hessian);
};

struct NonlinearFunctionModel
{
	std::vector<NonlinearFunction> nl_functions;
	std::vector<FunctionInstances> constraint_function_instances;
	std::vector<size_t> active_constraint_function_indices;
	std::vector<FunctionInstances> objective_function_instances;
	std::vector<size_t> active_objective_function_indices;

	std::vector<double> p;

	ParameterIndex add_parameter(double value = 0.0);
	void set_parameter(const ParameterIndex &parameter, double value);

	FunctionIndex register_function(ADFunD &f, const std::string &name);

	NLConstraintIndex add_nl_constraint(const FunctionIndex &k,
	                                    const std::vector<VariableIndex> &xs,
	                                    const std::vector<ParameterIndex> &ps, size_t y);

	void add_nl_objective(const FunctionIndex &k, const std::vector<VariableIndex> &xs,
	                      const std::vector<ParameterIndex> &ps);

	void analyze_active_functions();

	// renumber all nonlinear constraints from 0 and collect their indices
	void analyze_compact_constraint_index(size_t &n_nlcon, std::vector<size_t> &ys);

	void analyze_jacobian_structure(size_t &m_jacobian_nnz, std::vector<size_t> &m_jacobian_rows,
	                                std::vector<size_t> &m_jacobian_cols);
	void analyze_dense_gradient_structure();
	void analyze_sparse_gradient_structure(size_t &gradient_nnz, std::vector<size_t> &gradient_cols,
	                                       Hashmap<size_t, size_t> &gradient_index_map);
	void analyze_hessian_structure(size_t &m_hessian_nnz, std::vector<size_t> &m_hessian_rows,
	                               std::vector<size_t> &m_hessian_cols,
	                               Hashmap<VariablePair, size_t> &m_hessian_index_map,
	                               HessianSparsityType hessian_sparsity_type);

	void eval_objective(const double *x, double *y);

	void eval_objective_gradient(const double *x, double *grad);

	void eval_constraint(const double *x, double *con);

	void eval_constraint_jacobian(const double *x, double *jacobian);

	void eval_lagrangian_hessian(const double *x, const double *sigma, const double *lambda,
	                             double *hessian);
};
