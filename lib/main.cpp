#include "pyoptinterface/nleval.hpp"
#include "pyoptinterface/cppad_interface.hpp"

void test_linear_eval()
{
	LinearEvaluator evaluator;
	{
		ScalarAffineFunction f;
		f.add_constant(1.0);
		f.add_term(0, 2.0);

		evaluator.add_row(f);
	}

	{
		ScalarAffineFunction f;
		f.add_term(1, 3.0);
		f.add_term(0, 4.0);
		evaluator.add_row(f);
	}

	std::vector<double> x = {1.0, 2.0};
	std::vector<double> f(2);
	evaluator.eval_function(x.data(), f.data());

	size_t global_jabobian_nnz = 0;
	std::vector<int> global_jacobian_rows, global_jacobian_cols;
	evaluator.analyze_jacobian_structure(global_jabobian_nnz, global_jacobian_rows,
	                                     global_jacobian_cols);

	std::vector<double> jacobian(global_jabobian_nnz);
	evaluator.eval_jacobian(x.data(), jacobian.data());
}

void test_quadratic_eval()
{
	QuadraticEvaluator evaluator;
	{
		ScalarQuadraticFunction f;
		f.add_constant(1.0);
		f.add_quadratic_term(0, 1, 4.0);
		f.add_quadratic_term(0, 0, 1.0);
		f.add_affine_term(0, 4.0);
		evaluator.add_row(f);
	}
	std::vector<double> x = {1.0, 2.0};
	std::vector<double> f(1);
	evaluator.eval_function(x.data(), f.data());
	size_t global_jabobian_nnz = 0;
	std::vector<int> global_jacobian_rows, global_jacobian_cols;
	evaluator.analyze_jacobian_structure(0, global_jabobian_nnz, global_jacobian_rows,
	                                     global_jacobian_cols);
	std::vector<double> jacobian(global_jabobian_nnz);
	evaluator.eval_jacobian(x.data(), jacobian.data());
	size_t global_hessian_nnz = 0;
	std::vector<int> global_hessian_rows, global_hessian_cols;
	Hashmap<std::tuple<int, int>, int> global_hessian_index_map;
	HessianSparsityType global_hessian_type = HessianSparsityType::Upper;
	evaluator.analyze_hessian_structure(global_hessian_nnz, global_hessian_rows,
	                                    global_hessian_cols, global_hessian_index_map,
	                                    global_hessian_type);
	std::vector<double> hessian(global_hessian_nnz);
	std::vector<double> lambda = {1.0};
	evaluator.eval_lagrangian_hessian(lambda.data(), hessian.data());
}

void test_cppad_pow_var_par()
{
	std::vector<CppAD::AD<double>> x(2);

	auto N_parameters = 2;
	std::vector<CppAD::AD<double>> p(N_parameters);
	for (size_t i = 0; i < N_parameters; i++)
	{
		p[i] = CppAD::AD<double>(i + 1);
	}
	if (N_parameters > 0)
	{
		CppAD::Independent(x, p);
	}
	else
	{
		CppAD::Independent(x);
	}

	auto N_outputs = 1;
	std::vector<CppAD::AD<double>> y(N_outputs);

	// Trace the outputs
	for (size_t i = 0; i < N_outputs; i++)
	{
		y[i] = CppAD::pow(x[i], p[i]);
	}

	ADFunDouble f;
	f.Dependent(x, y);

	CppAD::cpp_graph f_graph;
	f.to_graph(f_graph);

	// Print the graph
	f_graph.print(std::cout);
}

auto main() -> int
{
	test_cppad_pow_var_par();
	return 0;
}