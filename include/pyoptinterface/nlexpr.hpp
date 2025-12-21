#pragma once

#include <cstdint>
#include <vector>

#include "ankerl/unordered_dense.h"
#include "core.hpp"

using NodeId = uint32_t;
using EntityId = int;

using VariableNode = EntityId;
using ConstantNode = double;
using ParameterNode = EntityId;

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

	// Compatibility issue where some solvers only accepts two-arg multiplication
	Add2,
	Mul2
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

	std::string to_string() const;
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
	Hashmap<EntityId, size_t> m_variable_index_map;
	std::vector<VariableNode> m_variables;
	std::vector<ConstantNode> m_constants;
	std::vector<ParameterNode> m_parameters;
	std::vector<UnaryNode> m_unaries;
	std::vector<BinaryNode> m_binaries;
	std::vector<TernaryNode> m_ternaries;
	std::vector<NaryNode> m_naries;

	std::vector<ExpressionHandle> m_constraint_outputs;
	std::vector<ExpressionHandle> m_objective_outputs;

	ExpressionGraph() = default;

	std::string to_string() const;

	size_t n_variables() const;
	size_t n_constants() const;
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

	void add_constraint_output(const ExpressionHandle &expression);
	void add_objective_output(const ExpressionHandle &expression);
	bool has_constraint_output() const;
	bool has_objective_output() const;

	// Merge VariableIndex/ScalarAffineFunction/ScalarQuadraticFunction/ExprBuilder into
	// ExpressionGraph
	ExpressionHandle merge_variableindex(const VariableIndex &v);
	ExpressionHandle merge_scalaraffinefunction(const ScalarAffineFunction &f);
	ExpressionHandle merge_scalarquadraticfunction(const ScalarQuadraticFunction &f);
	ExpressionHandle merge_exprbuilder(const ExprBuilder &expr);

	// recognize compare expression
	bool is_compare_expression(const ExpressionHandle &expr) const;

	// tag the structure
	uint64_t main_structure_hash() const;
	uint64_t constraint_structure_hash(uint64_t hash) const;
	uint64_t objective_structure_hash(uint64_t hash) const;
};

void unpack_comparison_expression(ExpressionGraph &graph, const ExpressionHandle &expr,
                                  ExpressionHandle &real_expr, double &lb, double &ub);
