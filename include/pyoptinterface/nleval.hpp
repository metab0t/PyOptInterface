#include <cstdint>
#include <vector>

#include "core.hpp"

enum class HessianSparsityType
{
	Full,
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

struct AutodiffEvaluator
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
		additive_grad_funcptr p = nullptr;
		additive_grad_funcptr_noparam nop;
	} grad_eval;
	union {
		hessian_funcptr p = nullptr;
		hessian_funcptr_noparam nop;
	} hessian_eval;

	AutodiffEvaluator() = default;

	AutodiffEvaluator(const AutodiffSymbolicStructure &structure, uintptr_t fp, uintptr_t jp,
	                  uintptr_t ajp, uintptr_t hp);
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

struct NLConstraintIndex
{
	// the index in all constraints
	IndexT index;
	IndexT dim;

	NLConstraintIndex() = default;
	NLConstraintIndex(IndexT v, IndexT d) : index(v), dim(d)
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

struct LinearQuadraticEvaluator
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

	template <typename T>
	void set_objective(const T &expr)
	{
		lq_objective = expr;
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

struct NonlinearFunctionEvaluator
{
	std::vector<AutodiffSymbolicStructure> nl_function_structures;
	std::vector<AutodiffEvaluator> nl_function_evaluators;
	std::vector<FunctionInstances> constraint_function_instances;
	std::vector<size_t> active_constraint_function_indices;
	std::vector<FunctionInstances> objective_function_instances;
	std::vector<size_t> active_objective_function_indices;

	std::vector<double> p;

	ParameterIndex add_parameter(double value = 0.0);
	void set_parameter(const ParameterIndex &parameter, double value);

	FunctionIndex register_function(const AutodiffSymbolicStructure &structure);
	void set_function_evaluator(const FunctionIndex &k, const AutodiffEvaluator &evaluator);

	NLConstraintIndex add_nl_constraint(const FunctionIndex &k,
	                                    const std::vector<VariableIndex> &xs,
	                                    const std::vector<ParameterIndex> &ps, size_t y);

	void add_nl_objective(const FunctionIndex &k, const std::vector<VariableIndex> &xs,
	                      const std::vector<ParameterIndex> &ps);

	void clear_nl_objective();

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