#include <nanobind/nanobind.h>
#include <nanobind/stl/vector.h>

#include "pyoptinterface/nlexpr.hpp"

namespace nb = nanobind;

NB_MODULE(nlexpr_ext, m)
{
	nb::class_<VariableNode>(m, "VariableNode")
	    .def(nb::init<EntityId>())
	    .def_ro("id", &VariableNode::id);

	nb::class_<ConstantNode>(m, "ConstantNode")
	    .def(nb::init<double>())
	    .def_ro("value", &ConstantNode::value);

	nb::class_<ParameterNode>(m, "ParameterNode")
	    .def(nb::init<EntityId>())
	    .def_ro("id", &ParameterNode::id);

	nb::enum_<ArrayType>(m, "ArrayType")
	    .value("Constant", ArrayType::Constant)
	    .value("Variable", ArrayType::Variable)
	    .value("Parameter", ArrayType::Parameter)
	    .value("Unary", ArrayType::Unary)
	    .value("Binary", ArrayType::Binary)
	    .value("Ternary", ArrayType::Ternary)
	    .value("Nary", ArrayType::Nary);

	nb::enum_<UnaryOperator>(m, "UnaryOperator")
	    .value("Neg", UnaryOperator::Neg)
	    .value("Sin", UnaryOperator::Sin)
	    .value("Cos", UnaryOperator::Cos)
	    .value("Tan", UnaryOperator::Tan)
	    .value("Asin", UnaryOperator::Asin)
	    .value("Acos", UnaryOperator::Acos)
	    .value("Atan", UnaryOperator::Atan)
	    .value("Abs", UnaryOperator::Abs)
	    .value("Sqrt", UnaryOperator::Sqrt)
	    .value("Exp", UnaryOperator::Exp)
	    .value("Log", UnaryOperator::Log)
	    .value("Log10", UnaryOperator::Log10);

	nb::enum_<BinaryOperator>(m, "BinaryOperator")
	    .value("Sub", BinaryOperator::Sub)
	    .value("Div", BinaryOperator::Div)
	    .value("Pow", BinaryOperator::Pow)
	    // compare ops
	    .value("LessThan", BinaryOperator::LessThan)
	    .value("LessEqual", BinaryOperator::LessEqual)
	    .value("Equal", BinaryOperator::Equal)
	    .value("NotEqual", BinaryOperator::NotEqual)
	    .value("GreaterEqual", BinaryOperator::GreaterEqual)
	    .value("GreaterThan", BinaryOperator::GreaterThan);

	nb::enum_<TernaryOperator>(m, "TernaryOperator")
	    .value("IfThenElse", TernaryOperator::IfThenElse);

	nb::enum_<NaryOperator>(m, "NaryOperator")
	    .value("Add", NaryOperator::Add)
	    .value("Mul", NaryOperator::Mul);

	nb::class_<ExpressionHandle>(m, "ExpressionHandle")
	    .def(nb::init<ArrayType, NodeId>())
	    .def_ro("array", &ExpressionHandle::array)
	    .def_ro("id", &ExpressionHandle::id);

	nb::class_<UnaryNode>(m, "UnaryNode")
	    .def(nb::init<UnaryOperator, ExpressionHandle>())
	    .def_ro("op", &UnaryNode::op)
	    .def_ro("operand", &UnaryNode::operand);

	nb::class_<BinaryNode>(m, "BinaryNode")
	    .def(nb::init<BinaryOperator, ExpressionHandle, ExpressionHandle>())
	    .def_ro("op", &BinaryNode::op)
	    .def_ro("left", &BinaryNode::left)
	    .def_ro("right", &BinaryNode::right);

	nb::class_<NaryNode>(m, "NaryNode")
	    .def(nb::init<NaryOperator, const std::vector<ExpressionHandle> &>())
	    .def_ro("op", &NaryNode::op)
	    .def_ro("operands", &NaryNode::operands);

	nb::class_<ExpressionGraph>(m, "ExpressionGraph")
	    .def(nb::init<>())
	    .def("n_variables", &ExpressionGraph::n_variables)
	    .def("n_parameters", &ExpressionGraph::n_parameters)
	    .def("add_variable", &ExpressionGraph::add_variable, nb::arg("id") = 0)
	    .def("add_constant", &ExpressionGraph::add_constant, nb::arg("value"))
	    .def("add_parameter", &ExpressionGraph::add_parameter, nb::arg("id") = 0)
	    .def("add_unary", &ExpressionGraph::add_unary)
	    .def("add_binary", &ExpressionGraph::add_binary)
	    .def("add_ternary", &ExpressionGraph::add_ternary)
	    .def("add_nary", &ExpressionGraph::add_nary)
	    .def("add_repeat_nary", &ExpressionGraph::add_repeat_nary)
	    .def("append_nary", &ExpressionGraph::append_nary)
	    .def("get_nary_operator", &ExpressionGraph::get_nary_operator);
}
