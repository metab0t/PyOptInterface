#include "pyoptinterface/cppad_interface.hpp"

static const std::string opt_options = "no_compare_op no_conditional_skip no_cumulative_sum_op";

ADFunDouble dense_jacobian(const ADFunDouble &f)
{
	using CppAD::AD;
	using CppAD::ADFun;
	using CppAD::Independent;
	using Base = double;

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
	aj = af.Jacobian(ax);
	ADFun<Base> jac;
	jac.Dependent(apx, aj);

	jac.optimize(opt_options);

	return jac;
}

JacobianHessianSparsityPattern jacobian_hessian_sparsity(ADFunDouble &f,
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

ADFunDouble sparse_jacobian(const ADFunDouble &f, const sparsity_pattern_t &pattern_jac,
                            const std::vector<double> &x_values,
                            const std::vector<double> &p_values)
{
	using CppAD::AD;
	using CppAD::ADFun;
	using CppAD::Independent;
	using Base = double;

	size_t nx = f.Domain();
	size_t ny = f.Range();
	size_t np = f.size_dyn_ind();
	std::vector<AD<Base>> apx(np + nx), ax(nx), ap(np);
	for (size_t i = 0; i < np; i++)
	{
		apx[i] = p_values[i];
	}
	for (size_t i = 0; i < nx; i++)
	{
		apx[np + i] = x_values[i];
	}
	ADFun<AD<Base>, Base> af = f.base2ad();
	Independent(apx);
	for (size_t i = 0; i < np; i++)
	{
		ap[i] = apx[i];
	}
	for (size_t i = 0; i < nx; i++)
	{
		ax[i] = apx[np + i];
	}
	af.new_dynamic(ap);
	CppAD::sparse_rcv<std::vector<size_t>, std::vector<AD<Base>>> subset(pattern_jac);
	CppAD::sparse_jac_work work;
	std::string coloring = "cppad";
	size_t n_color = af.sparse_jac_rev(ax, subset, pattern_jac, coloring, work);
	ADFun<double> jacobian;
	jacobian.Dependent(apx, subset.val());

	jacobian.optimize(opt_options);

	return jacobian;
}

ADFunDouble sparse_hessian(const ADFunDouble &f, const sparsity_pattern_t &pattern_hes,
                           const sparsity_pattern_t &pattern_subset,
                           const std::vector<double> &x_values, const std::vector<double> &p_values)
{
	using CppAD::AD;
	using CppAD::ADFun;
	using CppAD::Independent;
	using Base = double;

	size_t nx = f.Domain();
	size_t ny = f.Range();
	size_t np = f.size_dyn_ind();
	size_t nw = ny;
	std::vector<AD<Base>> apwx(np + nw + nx), ax(nx), ap(np), aw(nw);
	for (size_t i = 0; i < np; i++)
	{
		apwx[i] = p_values[i];
	}
	for (size_t i = 0; i < nw; i++)
	{
		apwx[np + i] = 1.0;
	}
	for (size_t i = 0; i < nx; i++)
	{
		apwx[np + nw + i] = x_values[i];
	}
	ADFun<AD<Base>, Base> af = f.base2ad();
	Independent(apwx);
	for (size_t i = 0; i < np; i++)
	{
		ap[i] = apwx[i];
	}
	for (size_t i = 0; i < nw; i++)
	{
		aw[i] = apwx[np + i];
	}
	for (size_t i = 0; i < nx; i++)
	{
		ax[i] = apwx[np + nw + i];
	}
	af.new_dynamic(ap);
	CppAD::sparse_rcv<std::vector<size_t>, std::vector<AD<Base>>> subset(pattern_subset);
	CppAD::sparse_hes_work work;
	std::string coloring = "cppad.symmetric";
	size_t n_sweep = af.sparse_hes(ax, aw, subset, pattern_hes, coloring, work);
	ADFun<double> hessian;
	hessian.Dependent(apwx, subset.val());

	hessian.optimize(opt_options);

	return hessian;
}

CppAD::AD<double> cppad_build_unary_expression(UnaryOperator op, const CppAD::AD<double> &operand)
{
	switch (op)
	{
	case UnaryOperator::Neg: {
		return -operand;
	}
	case UnaryOperator::Sin: {
		return CppAD::sin(operand);
	}
	case UnaryOperator::Cos: {
		return CppAD::cos(operand);
	}
	case UnaryOperator::Tan: {
		return CppAD::tan(operand);
	}
	case UnaryOperator::Asin: {
		return CppAD::asin(operand);
	}
	case UnaryOperator::Acos: {
		return CppAD::acos(operand);
	}
	case UnaryOperator::Atan: {
		return CppAD::atan(operand);
	}
	case UnaryOperator::Abs: {
		return CppAD::abs(operand);
	}
	case UnaryOperator::Sqrt: {
		return CppAD::sqrt(operand);
	}
	case UnaryOperator::Exp: {
		return CppAD::exp(operand);
	}
	case UnaryOperator::Log: {
		return CppAD::log(operand);
	}
	default: {
		throw std::runtime_error("Invalid unary operator");
	}
	}
}

CppAD::AD<double> cppad_build_binary_expression(BinaryOperator op, const CppAD::AD<double> &left,
                                                const CppAD::AD<double> &right)
{
	switch (op)
	{
	case BinaryOperator::Sub: {
		return left - right;
	}
	case BinaryOperator::Div: {
		return left / right;
	}
	case BinaryOperator::Pow: {
		return CppAD::pow(left, right);
	}
	case BinaryOperator::LessThan:
	case BinaryOperator::LessEqual:
	case BinaryOperator::Equal:
	case BinaryOperator::NotEqual:
	case BinaryOperator::GreaterEqual:
	case BinaryOperator::GreaterThan: {
		throw std::runtime_error("Currently comparision operator can only be used with ifelse "
		                         "function and cannot be evaluated as value");
	}
	default: {
		throw std::runtime_error("Invalid binary operator");
	}
	}
}

CppAD::AD<double> cppad_build_ternary_expression(BinaryOperator compare_op,
                                                 const CppAD::AD<double> &compare_left,
                                                 const CppAD::AD<double> &compare_right,
                                                 const CppAD::AD<double> &then_result,
                                                 const CppAD::AD<double> &else_result)
{
	switch (compare_op)
	{
	case BinaryOperator::LessThan: {
		return CppAD::CondExpLt(compare_left, compare_right, then_result, else_result);
	}
	case BinaryOperator::LessEqual: {
		return CppAD::CondExpLe(compare_left, compare_right, then_result, else_result);
	}
	case BinaryOperator::Equal: {
		return CppAD::CondExpEq(compare_left, compare_right, then_result, else_result);
	}
	case BinaryOperator::NotEqual: {
		return CppAD::CondExpEq(compare_left, compare_right, else_result, then_result);
	}
	case BinaryOperator::GreaterEqual: {
		return CppAD::CondExpGe(compare_left, compare_right, then_result, else_result);
	}
	case BinaryOperator::GreaterThan: {
		return CppAD::CondExpGt(compare_left, compare_right, then_result, else_result);
	}
	default: {
		throw std::runtime_error("Invalid compare operator");
	}
	}
}

CppAD::AD<double> cppad_build_nary_expression(NaryOperator op,
                                              const std::vector<CppAD::AD<double>> &operands)
{
	if (operands.size() == 0)
		return CppAD::AD<double>(0.0);

	CppAD::AD<double> result = operands[0];
	switch (op)
	{
	case NaryOperator::Add: {
		for (auto i = 1; i < operands.size(); i++)
		{
			result += operands[i];
		}
		break;
	}
	case NaryOperator::Mul: {
		for (auto i = 1; i < operands.size(); i++)
		{
			result *= operands[i];
		}
		break;
	}
	}
	return result;
}

CppAD::AD<double> cppad_trace_expression(
    const ExpressionGraph &graph, const ExpressionHandle &expression,
    const std::vector<CppAD::AD<double>> &x, const std::vector<CppAD::AD<double>> &p,
    ankerl::unordered_dense::map<ExpressionHandle, CppAD::AD<double>> &seen_expressions)
{
	auto it = seen_expressions.find(expression);
	if (it != seen_expressions.end())
	{
		return it->second;
	}
	CppAD::AD<double> result;
	auto id = expression.id;
	switch (expression.array)
	{
	case ArrayType::Variable: {
		result = x[id];
		break;
	}
	case ArrayType::Constant: {
		auto &constant = graph.m_constants[id];
		result = constant.value;
		break;
	}
	case ArrayType::Parameter: {
		result = p[id];
		break;
	}
	case ArrayType::Unary: {
		auto &unary = graph.m_unaries[id];
		auto operand = cppad_trace_expression(graph, unary.operand, x, p, seen_expressions);
		result = cppad_build_unary_expression(unary.op, operand);
		seen_expressions[expression] = result;
		break;
	}
	case ArrayType::Binary: {
		auto &binary = graph.m_binaries[id];
		auto left = cppad_trace_expression(graph, binary.left, x, p, seen_expressions);
		auto right = cppad_trace_expression(graph, binary.right, x, p, seen_expressions);
		result = cppad_build_binary_expression(binary.op, left, right);
		seen_expressions[expression] = result;
		break;
	}
	case ArrayType::Ternary: {
		auto &ifelsethen_body = graph.m_ternaries[id];
		auto &condition = ifelsethen_body.left;
		assert(condition.array == ArrayType::Binary);

		auto &condition_body = graph.m_binaries[condition.id];
		BinaryOperator compare_op = condition_body.op;
		assert(is_binary_compare_op(compare_op));

		CppAD::AD<double> compare_left =
		    cppad_trace_expression(graph, condition_body.left, x, p, seen_expressions);
		CppAD::AD<double> compare_right =
		    cppad_trace_expression(graph, condition_body.right, x, p, seen_expressions);

		CppAD::AD<double> then_result =
		    cppad_trace_expression(graph, ifelsethen_body.middle, x, p, seen_expressions);
		CppAD::AD<double> else_result =
		    cppad_trace_expression(graph, ifelsethen_body.right, x, p, seen_expressions);

		result = cppad_build_ternary_expression(compare_op, compare_left, compare_right,
		                                        then_result, else_result);
		seen_expressions[expression] = result;
		break;
	}
	case ArrayType::Nary: {
		auto &nary = graph.m_naries[id];
		std::vector<CppAD::AD<double>> operand_values;
		for (auto &operand : nary.operands)
		{
			auto operand_value = cppad_trace_expression(graph, operand, x, p, seen_expressions);
			operand_values.push_back(operand_value);
		}
		result = cppad_build_nary_expression(nary.op, operand_values);
		seen_expressions[expression] = result;
		break;
	}
	default: {
		throw std::runtime_error("Invalid array type");
	}
	}
	return result;
}

ADFunDouble cppad_trace_function(const ExpressionGraph &graph,
                                 const std::vector<ExpressionHandle> &outputs)
{
	ankerl::unordered_dense::map<ExpressionHandle, CppAD::AD<double>> seen_expressions;

	auto N_inputs = graph.n_variables();
	std::vector<CppAD::AD<double>> x(N_inputs);

	auto N_parameters = graph.n_parameters();
	std::vector<CppAD::AD<double>> p(N_parameters);
	if (N_parameters > 0)
	{
		CppAD::Independent(x, p);
	}
	else
	{
		CppAD::Independent(x);
	}

	auto N_outputs = outputs.size();
	std::vector<CppAD::AD<double>> y(N_outputs);

	// Trace the outputs
	for (size_t i = 0; i < N_outputs; i++)
	{
		auto &output = outputs[i];
		y[i] = cppad_trace_expression(graph, output, x, p, seen_expressions);
	}

	ADFunDouble f;
	f.Dependent(x, y);

	return f;
}

void cppad_autodiff(ADFunDouble &f, AutodiffSymbolicStructure &structure, CppADAutodiffGraph &graph,
                    const std::vector<double> &x_values, const std::vector<double> &p_values)
{
	auto nx = f.Domain();
	auto np = f.size_dyn_ind();
	assert(x_values.size() == nx);
	assert(p_values.size() == np);

	auto ny = f.Range();

	structure.nx = nx;
	structure.np = np;
	structure.ny = ny;
	structure.has_parameter = np > 0;

	f.to_graph(graph.f_graph);

	auto sparsity = jacobian_hessian_sparsity(f, HessianSparsityType::Upper);

	{
		auto &pattern = sparsity.jacobian;
		auto &m_jacobian_rows = structure.m_jacobian_rows;
		auto &m_jacobian_cols = structure.m_jacobian_cols;
		auto &m_jacobian_nnz = structure.m_jacobian_nnz;
		for (int i = 0; i < pattern.nnz(); i++)
		{
			auto r = pattern.row()[i];
			auto c = pattern.col()[i];
			m_jacobian_rows.push_back(r);
			m_jacobian_cols.push_back(c);
		}
		m_jacobian_nnz = pattern.nnz();
	}

	{
		auto &pattern = sparsity.reduced_hessian;
		auto &m_hessian_rows = structure.m_hessian_rows;
		auto &m_hessian_cols = structure.m_hessian_cols;
		auto &m_hessian_nnz = structure.m_hessian_nnz;
		for (int i = 0; i < pattern.nnz(); i++)
		{
			auto r = pattern.row()[i];
			auto c = pattern.col()[i];
			m_hessian_rows.push_back(r);
			m_hessian_cols.push_back(c);
		}
		m_hessian_nnz = pattern.nnz();
	}

	if (structure.m_jacobian_nnz > 0)
	{
		structure.has_jacobian = true;
		ADFunDouble jacobian = sparse_jacobian(f, sparsity.jacobian, x_values, p_values);
		jacobian.to_graph(graph.jacobian_graph);
	}

	if (structure.m_hessian_nnz > 0)
	{
		structure.has_hessian = true;
		ADFunDouble hessian =
		    sparse_hessian(f, sparsity.hessian, sparsity.reduced_hessian, x_values, p_values);
		hessian.to_graph(graph.hessian_graph);
	}
}
