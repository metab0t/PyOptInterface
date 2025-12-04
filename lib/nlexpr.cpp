#include "pyoptinterface/nlexpr.hpp"

#include <cassert>
#include "fmt/core.h"

bool ExpressionHandle::operator==(const ExpressionHandle &x) const
{
	return array == x.array && id == x.id;
}

std::string ExpressionHandle::to_string() const
{
	switch (array)
	{
	case ArrayType::Constant:
		return fmt::format("c{}", id);
	case ArrayType::Variable:
		return fmt::format("v{}", id);
	case ArrayType::Parameter:
		return fmt::format("p{}", id);
	case ArrayType::Unary:
		return fmt::format("u{}", id);
	case ArrayType::Binary:
		return fmt::format("b{}", id);
	case ArrayType::Ternary:
		return fmt::format("t{}", id);
	case ArrayType::Nary:
		return fmt::format("n{}", id);
	}
}

std::string ExpressionGraph::to_string() const
{
	fmt::memory_buffer buf;

	fmt::format_to(fmt::appender(buf), "Variables: {}\n", m_variables.size());
	for (size_t i = 0; i < m_variables.size(); i++)
	{
		fmt::format_to(fmt::appender(buf), "\tv{}: {}\n", i, m_variables[i]);
	}

	fmt::format_to(fmt::appender(buf), "Constants: {}\n", m_constants.size());
	for (size_t i = 0; i < m_constants.size(); i++)
	{
		fmt::format_to(fmt::appender(buf), "\tc{}: {}\n", i, m_constants[i]);
	}

	fmt::format_to(fmt::appender(buf), "Parameters: {}\n", m_parameters.size());
	for (size_t i = 0; i < m_parameters.size(); i++)
	{
		fmt::format_to(fmt::appender(buf), "\tp{}: {}\n", i, m_parameters[i]);
	}

	fmt::format_to(fmt::appender(buf), "Unary: {}\n", m_unaries.size());
	for (size_t i = 0; i < m_unaries.size(); i++)
	{
		fmt::format_to(fmt::appender(buf), "\tu{}: {}({})\n", i,
		               unary_operator_to_string(m_unaries[i].op), m_unaries[i].operand.to_string());
	}

	fmt::format_to(fmt::appender(buf), "Binary: {}\n", m_binaries.size());
	for (size_t i = 0; i < m_binaries.size(); i++)
	{
		fmt::format_to(fmt::appender(buf), "\tb{}: {}({},{})\n", i,
		               binary_operator_to_string(m_binaries[i].op), m_binaries[i].left.to_string(),
		               m_binaries[i].right.to_string());
	}

	fmt::format_to(fmt::appender(buf), "Ternary: {}\n", m_ternaries.size());
	for (size_t i = 0; i < m_ternaries.size(); i++)
	{
		fmt::format_to(fmt::appender(buf), "\tt{}: {}({},{},{})\n", i,
		               ternary_operator_to_string(m_ternaries[i].op),
		               m_ternaries[i].left.to_string(), m_ternaries[i].middle.to_string(),
		               m_ternaries[i].right.to_string());
	}

	fmt::format_to(fmt::appender(buf), "Nary: {}\n", m_naries.size());
	for (size_t i = 0; i < m_naries.size(); i++)
	{
		fmt::format_to(fmt::appender(buf), "\tn{}: {}(", i,
		               nary_operator_to_string(m_naries[i].op));
		for (size_t j = 0; j < m_naries[i].operands.size(); j++)
		{
			fmt::format_to(fmt::appender(buf), "{}, ", m_naries[i].operands[j].to_string());
		}
		fmt::format_to(fmt::appender(buf), ")\n");
	}

	fmt::format_to(fmt::appender(buf), "Constraint outputs: {}\n", m_constraint_outputs.size());
	for (size_t i = 0; i < m_constraint_outputs.size(); i++)
	{
		fmt::format_to(fmt::appender(buf), "\tcon{}: {}\n", i, m_constraint_outputs[i].to_string());
	}

	fmt::format_to(fmt::appender(buf), "Objective outputs: {}\n", m_objective_outputs.size());
	for (size_t i = 0; i < m_objective_outputs.size(); i++)
	{
		fmt::format_to(fmt::appender(buf), "\tobj{}: {}\n", i, m_objective_outputs[i].to_string());
	}

	return fmt::to_string(buf);
}

size_t ExpressionGraph::n_variables() const
{
	return m_variables.size();
}

size_t ExpressionGraph::n_constants() const
{
	return m_constants.size();
}

size_t ExpressionGraph::n_parameters() const
{
	return m_parameters.size();
}

ExpressionHandle ExpressionGraph::add_variable(EntityId id)
{
	auto iter = m_variable_index_map.find(id);
	if (iter != m_variable_index_map.end())
	{
		return {ArrayType::Variable, static_cast<NodeId>(iter->second)};
	}
	else
	{
		auto index = m_variables.size();
		m_variables.emplace_back(id);
		m_variable_index_map.emplace(id, index);
		return {ArrayType::Variable, static_cast<NodeId>(index)};
	}
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

void ExpressionGraph::add_constraint_output(const ExpressionHandle &expression)
{
	m_constraint_outputs.push_back(expression);
}

void ExpressionGraph::add_objective_output(const ExpressionHandle &expression)
{
	m_objective_outputs.push_back(expression);
}

bool ExpressionGraph::has_constraint_output() const
{
	return !m_constraint_outputs.empty();
}

bool ExpressionGraph::has_objective_output() const
{
	return !m_objective_outputs.empty();
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
		auto coef = f.coefficients[i];
		if (coef == 1.0)
		{
			terms.push_back(x);
		}
		else if (coef == -1.0)
		{
			auto neg = add_unary(UnaryOperator::Neg, x);
			terms.push_back(neg);
		}
		else
		{
			auto c = add_constant(coef);
			terms.push_back(add_nary(NaryOperator::Mul, {c, x}));
		}
	}
	if (f.constant)
	{
		terms.push_back(add_constant(f.constant.value()));
	}
	if (terms.size() == 1)
	{
		return terms[0];
	}
	else
	{
		return add_nary(NaryOperator::Add, terms);
	}
}

ExpressionHandle ExpressionGraph::merge_scalarquadraticfunction(const ScalarQuadraticFunction &f)
{
	auto N = f.size();
	std::vector<ExpressionHandle> terms;
	terms.reserve(N + 1);
	for (size_t i = 0; i < N; i++)
	{
		auto x1 = f.variable_1s[i];
		auto x2 = f.variable_2s[i];
		auto coef = f.coefficients[i];

		ExpressionHandle x1_var = add_variable(x1);
		ExpressionHandle x2_var;

		if (x1 == x2)
		{
			x2_var = x1_var;
		}
		else
		{
			x2_var = add_variable(x2);
		}

		if (coef == 1.0)
		{
			terms.push_back(add_nary(NaryOperator::Mul, {x1_var, x2_var}));
		}
		else if (coef == -1.0)
		{
			auto neg = add_unary(UnaryOperator::Neg, add_nary(NaryOperator::Mul, {x1_var, x2_var}));
			terms.push_back(neg);
		}
		else
		{
			auto c = add_constant(coef);
			terms.push_back(add_nary(NaryOperator::Mul, {c, x1_var, x2_var}));
		}
	}
	if (f.affine_part)
	{
		terms.push_back(merge_scalaraffinefunction(f.affine_part.value()));
	}
	if (terms.size() == 1)
	{
		return terms[0];
	}
	else
	{
		return add_nary(NaryOperator::Add, terms);
	}
}

ExpressionHandle ExpressionGraph::merge_exprbuilder(const ExprBuilder &expr)
{
	std::vector<ExpressionHandle> terms;
	terms.reserve(expr.quadratic_terms.size() + expr.affine_terms.size() + 1);
	for (const auto &[varpair, coef] : expr.quadratic_terms)
	{
		auto x1 = varpair.var_1;
		auto x2 = varpair.var_2;

		ExpressionHandle x1_var = add_variable(x1);
		ExpressionHandle x2_var;

		if (x1 == x2)
		{
			x2_var = x1_var;
		}
		else
		{
			x2_var = add_variable(x2);
		}

		if (coef == 1.0)
		{
			terms.push_back(add_nary(NaryOperator::Mul, {x1_var, x2_var}));
		}
		else if (coef == -1.0)
		{
			auto neg = add_unary(UnaryOperator::Neg, add_nary(NaryOperator::Mul, {x1_var, x2_var}));
			terms.push_back(neg);
		}
		else
		{
			auto c = add_constant(coef);
			terms.push_back(add_nary(NaryOperator::Mul, {c, x1_var, x2_var}));
		}
	}
	for (const auto &[var, coef] : expr.affine_terms)
	{
		auto x = add_variable(var);
		if (coef == 1.0)
		{
			terms.push_back(x);
		}
		else if (coef == -1.0)
		{
			auto neg = add_unary(UnaryOperator::Neg, x);
			terms.push_back(neg);
		}
		else
		{
			auto c = add_constant(coef);
			terms.push_back(add_nary(NaryOperator::Mul, {c, x}));
		}
	}
	if (expr.constant_term)
	{
		terms.push_back(add_constant(expr.constant_term.value()));
	}
	if (terms.size() == 1)
	{
		return terms[0];
	}
	else
	{
		return add_nary(NaryOperator::Add, terms);
	}
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
	case BinaryOperator::Add2:
		return "Add2";
	case BinaryOperator::Mul2:
		return "Mul2";
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

inline static void hash_combine(uint64_t &hash, const uint64_t subhash)
{
	hash ^= subhash + 0x9e3779b9 + (hash << 6) + (hash >> 2);
}

inline static uint64_t to_i64(const ExpressionHandle &expr)
{
	uint32_t array = static_cast<uint32_t>(expr.array);
	uint32_t id = static_cast<uint32_t>(expr.id);
	return (static_cast<uint64_t>(array) << 32) | id;
}

uint64_t ExpressionGraph::main_structure_hash() const
{
	uint64_t hash = 0;
	hash_combine(hash, m_variables.size());
	hash_combine(hash, m_constants.size());
	hash_combine(hash, m_parameters.size());

	for (const auto &unary : m_unaries)
	{
		hash_combine(hash, (uint64_t)unary.op);
		hash_combine(hash, to_i64(unary.operand));
	}
	for (const auto &binary : m_binaries)
	{
		hash_combine(hash, (uint64_t)binary.op);
		hash_combine(hash, to_i64(binary.left));
		hash_combine(hash, to_i64(binary.right));
	}
	for (const auto &ternary : m_ternaries)
	{
		hash_combine(hash, (uint64_t)ternary.op);
		hash_combine(hash, to_i64(ternary.left));
		hash_combine(hash, to_i64(ternary.middle));
		hash_combine(hash, to_i64(ternary.right));
	}
	for (const auto &nary : m_naries)
	{
		hash_combine(hash, (uint64_t)nary.op);
		for (const auto &operand : nary.operands)
		{
			hash_combine(hash, to_i64(operand));
		}
	}
	return hash;
}

uint64_t ExpressionGraph::constraint_structure_hash(uint64_t hash) const
{
	for (const auto &output : m_constraint_outputs)
	{
		hash_combine(hash, to_i64(output));
	}
	return hash;
}

uint64_t ExpressionGraph::objective_structure_hash(uint64_t hash) const
{
	for (const auto &output : m_objective_outputs)
	{
		hash_combine(hash, to_i64(output));
	}
	return hash;
}

void unpack_comparison_expression(ExpressionGraph &graph, const ExpressionHandle &expr,
                                  ExpressionHandle &real_expr, double &lb, double &ub)
{
	// Only handles constraint in the form of f <= g or f >= g or f == g
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
	if (op != BinaryOperator::LessEqual && op != BinaryOperator::GreaterEqual &&
	    op != BinaryOperator::Equal)
	{
		throw std::runtime_error("Only <= or >= or == is supported for comparison constraint");
	}

	auto f = binary.left;
	auto g = binary.right;

	if (op == BinaryOperator::GreaterEqual)
	{
		// swap f and g
		auto temp = f;
		f = g;
		g = temp;

		op = BinaryOperator::LessEqual;
	}

	// Now we only handle f <= g or f == g

	// test if f or g is constant
	bool f_is_constant = f.array == ArrayType::Constant;
	bool g_is_constant = g.array == ArrayType::Constant;

	if (op == BinaryOperator::LessEqual)
	{
		if (f_is_constant)
		{
			lb = graph.m_constants[f.id];
			real_expr = g;
		}
		else if (g_is_constant)
		{
			ub = graph.m_constants[g.id];
			real_expr = f;
		}
		else
		{
			// f - g <= 0
			real_expr = graph.add_binary(BinaryOperator::Sub, f, g);
			ub = 0.0;
		}
	}
	else
	{
		if (f_is_constant)
		{
			lb = ub = graph.m_constants[f.id];
			real_expr = g;
		}
		else if (g_is_constant)
		{
			lb = ub = graph.m_constants[g.id];
			real_expr = f;
		}
		else
		{
			// f - g == 0
			real_expr = graph.add_binary(BinaryOperator::Sub, f, g);
			lb = ub = 0.0;
		}
	}
}
