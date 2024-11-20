#pragma once

#include "cppad/cppad.hpp"
#include "pyoptinterface/nlexpr.hpp"
#include "pyoptinterface/nleval.hpp"

using ADFunDouble = CppAD::ADFun<double>;

ADFunDouble dense_jacobian(const ADFunDouble &f);

using sparsity_pattern_t = CppAD::sparse_rc<std::vector<size_t>>;

struct JacobianHessianSparsityPattern
{
	sparsity_pattern_t jacobian;
	sparsity_pattern_t hessian;
	sparsity_pattern_t reduced_hessian;
};

JacobianHessianSparsityPattern jacobian_hessian_sparsity(ADFunDouble &f,
                                                         HessianSparsityType hessian_sparsity);

// [p, x] -> Jacobian
ADFunDouble sparse_jacobian(const ADFunDouble &f, const sparsity_pattern_t &pattern_jac,
                            const std::vector<double> &x_values,
                            const std::vector<double> &p_values);

// [p, w, x] -> \Sigma w_i * Hessian_i
ADFunDouble sparse_hessian(const ADFunDouble &f, const sparsity_pattern_t &pattern_hes,
                           const sparsity_pattern_t &pattern_subset,
                           const std::vector<double> &x_values,
                           const std::vector<double> &p_values);

// Transform ExpressionGraph to CppAD function
ADFunDouble cppad_trace_function(const ExpressionGraph &graph,
                                 const std::vector<ExpressionHandle> &outputs);

struct CppADAutodiffGraph
{
	CppAD::cpp_graph f_graph, jacobian_graph, hessian_graph;
};

// Generate computational graph for the CppAD function (itself, Jacobian and Hessian)
// Analyze its sparsity as well
void cppad_autodiff(ADFunDouble &f, AutodiffSymbolicStructure &structure, CppADAutodiffGraph &graph,
                    const std::vector<double> &x_values, const std::vector<double> &p_values);
