#pragma once

#include <cstdint>
#include <vector>

#include "ankerl/unordered_dense.h"
#include "core.hpp"

using NodeId = uint32_t;
using EntityId = uint32_t;

struct VariableNode
{
	EntityId id;

	VariableNode(EntityId id) : id(id)
	{
	}
};

struct ConstantNode
{
	double value;

	ConstantNode(double value) : value(value)
	{
	}
};

struct ParameterNode
{
	EntityId id;

	ParameterNode(EntityId id) : id(id)
	{
	}
};

enum class ArrayType
{
	Constant,
	Variable,
	Parameter,
	Unary,
	Binary,
	Ternary,
	Nary
};

enum class UnaryOperator
{
	Neg,
	Sin,
	Cos,
	Tan,
	Asin,
	Acos,
	Atan,
	Abs,
	Sqrt,
	Exp,
	Log,
	Log10
};

enum class BinaryOperator
{
	Sub,
	Div,
	Pow,

	// compare
	LessThan,
	LessEqual,
	Equal,
	NotEqual,
	GreaterEqual,
	GreaterThan,
};

bool is_binary_compare_op(BinaryOperator op);

enum class TernaryOperator
{
	IfThenElse,
};

enum class NaryOperator
{
	Add,
	Mul,
};

std::string unary_operator_to_string(UnaryOperator op);
std::string binary_operator_to_string(BinaryOperator op);
std::string ternary_operator_to_string(TernaryOperator op);
std::string nary_operator_to_string(NaryOperator op);

struct ExpressionHandle
{
	ArrayType array;
	NodeId id;

	bool operator==(const ExpressionHandle &x) const;

	ExpressionHandle() = default;
	ExpressionHandle(ArrayType array, NodeId id) : array(array), id(id)
	{
	}
};

template <>
struct ankerl::unordered_dense::hash<ExpressionHandle>
{
	using is_avalanching = void;

	[[nodiscard]] auto operator()(ExpressionHandle const &x) const noexcept -> uint64_t
	{
		static_assert(std::has_unique_object_representations_v<ExpressionHandle>);
		return detail::wyhash::hash(&x, sizeof(x));
	}
};

struct UnaryNode
{
	UnaryOperator op;
	ExpressionHandle operand;

	UnaryNode(UnaryOperator op, ExpressionHandle operand) : op(op), operand(operand)
	{
	}
};

struct BinaryNode
{
	BinaryOperator op;
	ExpressionHandle left;
	ExpressionHandle right;

	BinaryNode(BinaryOperator op, ExpressionHandle left, ExpressionHandle right)
	    : op(op), left(left), right(right)
	{
	}
};

struct TernaryNode
{
	TernaryOperator op;
	ExpressionHandle left;
	ExpressionHandle middle;
	ExpressionHandle right;

	TernaryNode(TernaryOperator op, ExpressionHandle left, ExpressionHandle middle,
	            ExpressionHandle right)
	    : op(op), left(left), middle(middle), right(right)
	{
	}
};

struct NaryNode
{
	NaryOperator op;
	std::vector<ExpressionHandle> operands;

	NaryNode(NaryOperator op, const std::vector<ExpressionHandle> &operands)
	    : op(op), operands(operands)
	{
	}
};

struct ExpressionGraph
{
	std::vector<VariableNode> m_variables;
	std::vector<ConstantNode> m_constants;
	std::vector<ParameterNode> m_parameters;
	std::vector<UnaryNode> m_unaries;
	std::vector<BinaryNode> m_binaries;
	std::vector<TernaryNode> m_ternaries;
	std::vector<NaryNode> m_naries;

	ExpressionGraph() = default;

	size_t n_variables() const;
	size_t n_parameters() const;

	ExpressionHandle add_variable(EntityId id);

	ExpressionHandle add_constant(double value);

	ExpressionHandle add_parameter(EntityId id);

	ExpressionHandle add_unary(UnaryOperator op, ExpressionHandle operand);

	ExpressionHandle add_binary(BinaryOperator op, ExpressionHandle left, ExpressionHandle right);

	ExpressionHandle add_ternary(TernaryOperator op, ExpressionHandle left, ExpressionHandle middle,
	                             ExpressionHandle right);

	ExpressionHandle add_nary(NaryOperator op, const std::vector<ExpressionHandle> &operands);
	ExpressionHandle add_repeat_nary(NaryOperator op, ExpressionHandle operand, int N);

	void append_nary(const ExpressionHandle &expression, const ExpressionHandle &operand);

	NaryOperator get_nary_operator(const ExpressionHandle &expression) const;

	// Merge VariableIndex/ScalarAffineFunction/ScalarQuadraticFunction/ExprBuilder into
	// ExpressionGraph
	ExpressionHandle merge_variableindex(const VariableIndex &v);
	ExpressionHandle merge_scalaraffinefunction(const ScalarAffineFunction &f);
	ExpressionHandle merge_scalarquadraticfunction(const ScalarQuadraticFunction &f);
	ExpressionHandle merge_exprbuilder(const ExprBuilder &expr);

	// recognize compare expression
	bool is_compare_expression(const ExpressionHandle &expr) const;
};

void unpack_comparison_expression(ExpressionGraph &graph, const ExpressionHandle &expr,
                                  ExpressionHandle &real_expr, double &lb, double &ub);