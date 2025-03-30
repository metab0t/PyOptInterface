#include "pyoptinterface/nlexpr.hpp"

#include <cassert>

bool ExpressionHandle::operator==(const ExpressionHandle &x) const
{
	return array == x.array && id == x.id;
}

size_t ExpressionGraph::n_variables() const
{
	return m_variables.size();
}

size_t ExpressionGraph::n_parameters() const
{
	return m_parameters.size();
}

ExpressionHandle ExpressionGraph::add_variable(EntityId id)
{
	m_variables.emplace_back(id);
	return {ArrayType::Variable, static_cast<NodeId>(m_variables.size() - 1)};
}

ExpressionHandle ExpressionGraph::add_constant(double value)
{
	m_constants.emplace_back(value);
	return {ArrayType::Constant, static_cast<NodeId>(m_constants.size() - 1)};
}

ExpressionHandle ExpressionGraph::add_parameter(EntityId id)
{
	m_parameters.emplace_back(id);
	return {ArrayType::Parameter, static_cast<NodeId>(m_parameters.size() - 1)};
}

ExpressionHandle ExpressionGraph::add_unary(UnaryOperator op, ExpressionHandle operand)
{
	m_unaries.emplace_back(op, operand);
	return {ArrayType::Unary, static_cast<NodeId>(m_unaries.size() - 1)};
}

ExpressionHandle ExpressionGraph::add_binary(BinaryOperator op, ExpressionHandle left,
                                             ExpressionHandle right)
{
	m_binaries.emplace_back(op, left, right);
	return {ArrayType::Binary, static_cast<NodeId>(m_binaries.size() - 1)};
}

ExpressionHandle ExpressionGraph::add_ternary(TernaryOperator op, ExpressionHandle left,
                                              ExpressionHandle middle, ExpressionHandle right)
{
	m_ternaries.emplace_back(op, left, middle, right);
	return {ArrayType::Ternary, static_cast<NodeId>(m_ternaries.size() - 1)};
}

ExpressionHandle ExpressionGraph::add_nary(NaryOperator op,
                                           const std::vector<ExpressionHandle> &operands)
{
	m_naries.emplace_back(op, operands);
	return {ArrayType::Nary, static_cast<NodeId>(m_naries.size() - 1)};
}

ExpressionHandle ExpressionGraph::add_repeat_nary(NaryOperator op, ExpressionHandle operand, int N)
{
	std::vector<ExpressionHandle> operands(N, operand);
	return add_nary(op, operands);
}

void ExpressionGraph::append_nary(const ExpressionHandle &expression,
                                  const ExpressionHandle &operand)
{
	assert(expression.array == ArrayType::Nary);
	m_naries[expression.id].operands.push_back(operand);
}

NaryOperator ExpressionGraph::get_nary_operator(const ExpressionHandle &expression) const
{
	assert(expression.array == ArrayType::Nary);
	return m_naries[expression.id].op;
}

ExpressionHandle ExpressionGraph::merge_variableindex(const VariableIndex &v)
{
	return add_variable(v.index);
}

ExpressionHandle ExpressionGraph::merge_scalaraffinefunction(const ScalarAffineFunction &f)
{
	// Convert it to a n-ary sum of multiplication nodes
	auto N = f.size();
	std::vector<ExpressionHandle> terms;
	terms.reserve(N);
	for (size_t i = 0; i < N; i++)
	{
		auto x = add_variable(f.variables[i]);
		auto c = add_constant(f.coefficients[i]);
		terms.push_back(add_nary(NaryOperator::Mul, {c, x}));
	}
	if (f.constant)
	{
		terms.push_back(add_constant(f.constant.value()));
	}
	return add_nary(NaryOperator::Add, terms);
}

ExpressionHandle ExpressionGraph::merge_scalarquadraticfunction(const ScalarQuadraticFunction &f)
{
	auto N = f.size();
	std::vector<ExpressionHandle> terms;
	terms.reserve(N + 1);
	for (size_t i = 0; i < N; i++)
	{
		auto x1 = add_variable(f.variable_1s[i]);
		auto x2 = add_variable(f.variable_2s[i]);
		auto c = add_constant(f.coefficients[i]);
		terms.push_back(add_nary(NaryOperator::Mul, {c, x1, x2}));
	}
	if (f.affine_part)
	{
		terms.push_back(merge_scalaraffinefunction(f.affine_part.value()));
	}
	return add_nary(NaryOperator::Add, terms);
}

ExpressionHandle ExpressionGraph::merge_exprbuilder(const ExprBuilder &expr)
{
	std::vector<ExpressionHandle> terms;
	terms.reserve(expr.quadratic_terms.size() + expr.affine_terms.size() + 1);
	for (const auto &[varpair, coef] : expr.quadratic_terms)
	{
		auto x1 = add_variable(varpair.var_1);
		auto x2 = add_variable(varpair.var_2);
		auto c = add_constant(coef);
		auto term = add_nary(NaryOperator::Mul, {c, x1, x2});
		terms.push_back(term);
	}
	for (const auto &[var, coef] : expr.affine_terms)
	{
		auto x = add_variable(var);
		auto c = add_constant(coef);
		auto term = add_nary(NaryOperator::Mul, {c, x});
		terms.push_back(term);
	}
	if (expr.constant_term)
	{
		terms.push_back(add_constant(expr.constant_term.value()));
	}
	return add_nary(NaryOperator::Add, terms);
}

bool is_binary_compare_op(BinaryOperator op)
{
	return (op >= BinaryOperator::LessThan) && (op <= BinaryOperator::GreaterThan);
}

bool ExpressionGraph::is_compare_expression(const ExpressionHandle &expr) const
{
	if (expr.array != ArrayType::Binary)
	{
		return false;
	}
	auto &binary = m_binaries[expr.id];
	auto op = binary.op;
	return is_binary_compare_op(op);
}

std::string unary_operator_to_string(UnaryOperator op)
{
	switch (op)
	{
	case UnaryOperator::Neg:
		return "Neg";
	case UnaryOperator::Sin:
		return "Sin";
	case UnaryOperator::Cos:
		return "Cos";
	case UnaryOperator::Tan:
		return "Tan";
	case UnaryOperator::Asin:
		return "Asin";
	case UnaryOperator::Acos:
		return "Acos";
	case UnaryOperator::Atan:
		return "Atan";
	case UnaryOperator::Abs:
		return "Abs";
	case UnaryOperator::Sqrt:
		return "Sqrt";
	case UnaryOperator::Exp:
		return "Exp";
	case UnaryOperator::Log:
		return "Log";
	case UnaryOperator::Log10:
		return "Log10";
	}
}

std::string binary_operator_to_string(BinaryOperator op)
{
	switch (op)
	{
	case BinaryOperator::Sub:
		return "Sub";
	case BinaryOperator::Div:
		return "Div";
	case BinaryOperator::Pow:
		return "Pow";
	case BinaryOperator::LessThan:
		return "LessThan";
	case BinaryOperator::LessEqual:
		return "LessEqual";
	case BinaryOperator::Equal:
		return "Equal";
	case BinaryOperator::NotEqual:
		return "NotEqual";
	case BinaryOperator::GreaterEqual:
		return "GreaterEqual";
	case BinaryOperator::GreaterThan:
		return "GreaterThan";
	}
}

std::string ternary_operator_to_string(TernaryOperator op)
{
	switch (op)
	{
	case TernaryOperator::IfThenElse:
		return "IfThenElse";
	}
}

std::string nary_operator_to_string(NaryOperator op)
{
	switch (op)
	{
	case NaryOperator::Add:
		return "Add";
	case NaryOperator::Mul:
		return "Mul";
	}
}

void unpack_comparison_expression(ExpressionGraph &graph, const ExpressionHandle &expr,
                                  ExpressionHandle &real_expr, double &lb, double &ub)
{
	// Only handles constraint in the form of f <= g or f >= g
	// We need to tell if f or g is constant
	// if not, it will be converted into f-g <= 0 or f-g >= 0
	auto array_type = expr.array;
	auto index = expr.id;

	if (array_type != ArrayType::Binary)
	{
		throw std::runtime_error("Only binary operator is supported for comparison constraint");
	}

	auto &binary = graph.m_binaries[index];
	auto op = binary.op;
	if (op != BinaryOperator::LessEqual && op != BinaryOperator::GreaterEqual)
	{
		throw std::runtime_error("Only <= or >= is supported for comparison constraint");
	}

	auto f = binary.left;
	auto g = binary.right;

	if (op == BinaryOperator::GreaterEqual)
	{
		// swap f and g
		auto temp = f;
		f = g;
		g = temp;
	}

	// Now we only handle f <= g

	// test if f or g is constant
	bool f_is_constant = f.array == ArrayType::Constant;
	bool g_is_constant = g.array == ArrayType::Constant;

	real_expr = f;
	if (f_is_constant)
	{
		lb = graph.m_constants[f.id].value;
		real_expr = g;
	}
	else if (g_is_constant)
	{
		ub = graph.m_constants[g.id].value;
		real_expr = f;
	}
	else
	{
		// f - g <= 0
		real_expr = graph.add_binary(BinaryOperator::Sub, f, g);
		ub = 0.0;
	}
}
