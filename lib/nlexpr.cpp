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

bool is_binary_compare_op(BinaryOperator op)
{
	return (op >= BinaryOperator::Lessthan) && (op <= BinaryOperator::Greaterthan);
}
