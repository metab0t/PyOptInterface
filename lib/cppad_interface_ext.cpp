#include <nanobind/nanobind.h>
#include <nanobind/make_iterator.h>
#include <nanobind/stl/vector.h>
#include <nanobind/stl/string.h>

namespace nb = nanobind;

#include "pyoptinterface/cppad_interface.hpp"

using graph_op_enum = CppAD::graph::graph_op_enum;

struct cpp_graph_cursor
{
	size_t op_index = 0;
	size_t arg_index = 0;
};

graph_op_enum cursor_op(CppAD::cpp_graph &graph, cpp_graph_cursor &cursor)
{
	return graph.operator_vec_get(cursor.op_index);
}

size_t cursor_n_arg(CppAD::cpp_graph &graph, cpp_graph_cursor &cursor)
{
	auto op = cursor_op(graph, cursor);

	size_t n_arg = 0;

	switch (op)
	{
	// unary operators
	case graph_op_enum::abs_graph_op:
	case graph_op_enum::acos_graph_op:
	case graph_op_enum::acosh_graph_op:
	case graph_op_enum::asin_graph_op:
	case graph_op_enum::asinh_graph_op:
	case graph_op_enum::atan_graph_op:
	case graph_op_enum::atanh_graph_op:
	case graph_op_enum::cos_graph_op:
	case graph_op_enum::cosh_graph_op:
	case graph_op_enum::erf_graph_op:
	case graph_op_enum::erfc_graph_op:
	case graph_op_enum::exp_graph_op:
	case graph_op_enum::expm1_graph_op:
	case graph_op_enum::log1p_graph_op:
	case graph_op_enum::log_graph_op:
	case graph_op_enum::neg_graph_op:
	case graph_op_enum::sign_graph_op:
	case graph_op_enum::sin_graph_op:
	case graph_op_enum::sinh_graph_op:
	case graph_op_enum::sqrt_graph_op:
	case graph_op_enum::tan_graph_op:
	case graph_op_enum::tanh_graph_op:
		n_arg = 1;
		break;

	// binary operators
	case graph_op_enum::add_graph_op:
	case graph_op_enum::azmul_graph_op:
	case graph_op_enum::div_graph_op:
	case graph_op_enum::mul_graph_op:
	case graph_op_enum::pow_graph_op:
	case graph_op_enum::sub_graph_op:
		n_arg = 2;
		break;

	// conditional operators
	case graph_op_enum::cexp_eq_graph_op:
	case graph_op_enum::cexp_le_graph_op:
	case graph_op_enum::cexp_lt_graph_op:
		n_arg = 4;
		break;

	default: {
		std::string op_name = CppAD::local::graph::op_enum2name[op];
		auto message = "Unknown graph_op: " + op_name;
		throw std::runtime_error(message);
	}
	break;
	}

	return n_arg;
}

void advance_graph_cursor(CppAD::cpp_graph &graph, cpp_graph_cursor &cursor)
{
	auto n_arg = cursor_n_arg(graph, cursor);
	cursor.arg_index += n_arg;
	cursor.op_index++;
}

NB_MODULE(cppad_interface_ext, m)
{
	nb::enum_<graph_op_enum>(m, "graph_op")
	    .value("abs", graph_op_enum::abs_graph_op)
	    .value("acos", graph_op_enum::acos_graph_op)
	    .value("acosh", graph_op_enum::acosh_graph_op)
	    .value("add", graph_op_enum::add_graph_op)
	    .value("asin", graph_op_enum::asin_graph_op)
	    .value("asinh", graph_op_enum::asinh_graph_op)
	    .value("atan", graph_op_enum::atan_graph_op)
	    .value("atanh", graph_op_enum::atanh_graph_op)
	    .value("atom4", graph_op_enum::atom4_graph_op)
	    .value("atom", graph_op_enum::atom_graph_op)
	    .value("azmul", graph_op_enum::azmul_graph_op)
	    .value("cexp_eq", graph_op_enum::cexp_eq_graph_op)
	    .value("cexp_le", graph_op_enum::cexp_le_graph_op)
	    .value("cexp_lt", graph_op_enum::cexp_lt_graph_op)
	    .value("comp_eq", graph_op_enum::comp_eq_graph_op)
	    .value("comp_le", graph_op_enum::comp_le_graph_op)
	    .value("comp_lt", graph_op_enum::comp_lt_graph_op)
	    .value("comp_ne", graph_op_enum::comp_ne_graph_op)
	    .value("cos", graph_op_enum::cos_graph_op)
	    .value("cosh", graph_op_enum::cosh_graph_op)
	    .value("discrete", graph_op_enum::discrete_graph_op)
	    .value("div", graph_op_enum::div_graph_op)
	    .value("erf", graph_op_enum::erf_graph_op)
	    .value("erfc", graph_op_enum::erfc_graph_op)
	    .value("exp", graph_op_enum::exp_graph_op)
	    .value("expm1", graph_op_enum::expm1_graph_op)
	    .value("log1p", graph_op_enum::log1p_graph_op)
	    .value("log", graph_op_enum::log_graph_op)
	    .value("mul", graph_op_enum::mul_graph_op)
	    .value("neg", graph_op_enum::neg_graph_op)
	    .value("pow", graph_op_enum::pow_graph_op)
	    .value("print", graph_op_enum::print_graph_op)
	    .value("sign", graph_op_enum::sign_graph_op)
	    .value("sin", graph_op_enum::sin_graph_op)
	    .value("sinh", graph_op_enum::sinh_graph_op)
	    .value("sqrt", graph_op_enum::sqrt_graph_op)
	    .value("sub", graph_op_enum::sub_graph_op)
	    .value("sum", graph_op_enum::sum_graph_op)
	    .value("tan", graph_op_enum::tan_graph_op)
	    .value("tanh", graph_op_enum::tanh_graph_op);

	nb::class_<cpp_graph_cursor>(m, "cpp_graph_cursor")
	    .def(nb::init<>())
	    .def_ro("op_index", &cpp_graph_cursor::op_index)
	    .def_ro("arg_index", &cpp_graph_cursor::arg_index);

	nb::class_<CppAD::cpp_graph>(m, "cpp_graph")
	    .def(nb::init<>())
	    .def_prop_ro("n_dynamic_ind", [](CppAD::cpp_graph &g) { return g.n_dynamic_ind_get(); })
	    .def_prop_ro("n_variable_ind", [](CppAD::cpp_graph &g) { return g.n_variable_ind_get(); })
	    .def_prop_ro("n_constant", [](CppAD::cpp_graph &g) { return g.constant_vec_size(); })
	    .def_prop_ro("n_dependent", [](CppAD::cpp_graph &g) { return g.dependent_vec_size(); })
	    .def_prop_ro("n_operator", [](CppAD::cpp_graph &g) { return g.operator_vec_size(); })
	    .def_prop_ro("n_operator_arg", [](CppAD::cpp_graph &g) { return g.operator_arg_size(); })
	    .def("constant_vec_get", &CppAD::cpp_graph::constant_vec_get)
	    .def("dependent_vec_get", &CppAD::cpp_graph::dependent_vec_get)
	    .def("__str__",
	         [](CppAD::cpp_graph &g) {
		         std::ostringstream oss;
		         g.print(oss);
		         return oss.str();
	         })
	    .def("get_cursor_op", &cursor_op)
	    .def("get_cursor_n_arg", &cursor_n_arg)
	    .def("get_cursor_args",
	         [](CppAD::cpp_graph &graph, cpp_graph_cursor &cursor) {
		         auto n_arg = cursor_n_arg(graph, cursor);
		         nb::list args;
		         for (size_t i = 0; i < n_arg; i++)
		         {
			         auto arg = graph.operator_arg_get(cursor.arg_index + i);
			         args.append(nb::int_(arg));
		         }
		         return args;
	         })
	    .def("next_cursor", &advance_graph_cursor);

	nb::class_<ADFunDouble>(m, "ADFunDouble")
	    .def(nb::init<>())
	    .def_prop_ro("nx", [](const ADFunDouble &f) { return f.Domain(); })
	    .def_prop_ro("ny", [](const ADFunDouble &f) { return f.Range(); })
	    .def_prop_ro("np", [](const ADFunDouble &f) { return f.size_dyn_ind(); })
	    .def("to_graph", &ADFunDouble::to_graph);

	nb::class_<CppADAutodiffGraph>(m, "CppADAutodiffGraph")
	    .def(nb::init<>())
	    .def_ro("f", &CppADAutodiffGraph::f_graph)
	    .def_ro("jacobian", &CppADAutodiffGraph::jacobian_graph)
	    .def_ro("hessian", &CppADAutodiffGraph::hessian_graph);

	m.def("cppad_trace_graph_constraints", cppad_trace_graph_constraints);
	m.def("cppad_trace_graph_objective", cppad_trace_graph_objective, nb::arg("graph"),
	      nb::arg("aggregate") = true);
	m.def("cppad_autodiff", &cppad_autodiff);
}
