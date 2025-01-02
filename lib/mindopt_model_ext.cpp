#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>
#include <nanobind/stl/function.h>

#include "pyoptinterface/mindopt_model.hpp"

namespace nb = nanobind;

NB_MODULE(mindopt_model_ext, m)
{
	m.import_("pyoptinterface._src.core_ext");

	m.def("is_library_loaded", &mindopt::is_library_loaded);
	m.def("load_library", &mindopt::load_library);

#define BIND_F(f) .def(#f, &MindoptEnv::f)
	nb::class_<MindoptEnv>(m, "RawEnv")
	    .def(nb::init<bool>(), nb::arg("empty") = false)
	    // clang-format off
		BIND_F(start)
	    // clang-format on
	    ;

	nb::class_<MindoptModel>(m, "_RawModelBase");

#undef BIND_F
#define BIND_F(f) .def(#f, &MindoptModelMixin::f)
	nb::class_<MindoptModelMixin, MindoptModel>(m, "RawModel")
	    .def(nb::init<>())
	    .def(nb::init<const MindoptEnv &>())
	    // clang-format off
		BIND_F(init)
		BIND_F(write)
	    // clang-format on

	    .def("add_variable", &MindoptModelMixin::add_variable,
	         nb::arg("domain") = VariableDomain::Continuous, nb::arg("lb") = -MDO_INFINITY,
	         nb::arg("ub") = MDO_INFINITY, nb::arg("name") = "")
	    // clang-format off
		BIND_F(delete_variable)
		BIND_F(delete_variables)
		BIND_F(is_variable_active)
	    // clang-format on
	    .def("set_variable_bounds", &MindoptModelMixin::set_variable_bounds, nb::arg("variable"),
	         nb::arg("lb"), nb::arg("ub"))

	    .def("get_value",
	         nb::overload_cast<const VariableIndex &>(&MindoptModelMixin::get_variable_value))
	    .def("get_value", nb::overload_cast<const ScalarAffineFunction &>(
	                          &MindoptModelMixin::get_expression_value))
	    .def("get_value", nb::overload_cast<const ScalarQuadraticFunction &>(
	                          &MindoptModelMixin::get_expression_value))
	    .def("get_value",
	         nb::overload_cast<const ExprBuilder &>(&MindoptModelMixin::get_expression_value))

	    .def("pprint", &MindoptModelMixin::pprint_variable)
	    .def("pprint",
	         nb::overload_cast<const ScalarAffineFunction &, int>(
	             &MindoptModelMixin::pprint_expression),
	         nb::arg("expr"), nb::arg("precision") = 4)
	    .def("pprint",
	         nb::overload_cast<const ScalarQuadraticFunction &, int>(
	             &MindoptModelMixin::pprint_expression),
	         nb::arg("expr"), nb::arg("precision") = 4)
	    .def("pprint",
	         nb::overload_cast<const ExprBuilder &, int>(&MindoptModelMixin::pprint_expression),
	         nb::arg("expr"), nb::arg("precision") = 4)

	    // clang-format off
		BIND_F(set_variable_name)
		BIND_F(set_constraint_name)
	    // clang-format on

	    .def("add_linear_constraint",
	         nb::overload_cast<const ScalarAffineFunction &, ConstraintSense, CoeffT, const char *>(
	             &MindoptModelMixin::add_linear_constraint),
	         nb::arg("expr"), nb::arg("sense"), nb::arg("rhs"), nb::arg("name") = "")
	    .def("add_linear_constraint",
	         nb::overload_cast<const VariableIndex &, ConstraintSense, CoeffT, const char *>(
	             &MindoptModelMixin::add_linear_constraint_from_var),
	         nb::arg("expr"), nb::arg("sense"), nb::arg("rhs"), nb::arg("name") = "")
	    .def("add_linear_constraint",
	         nb::overload_cast<const ExprBuilder &, ConstraintSense, CoeffT, const char *>(
	             &MindoptModelMixin::add_linear_constraint_from_expr),
	         nb::arg("expr"), nb::arg("sense"), nb::arg("rhs"), nb::arg("name") = "")
	    .def("add_quadratic_constraint",
	         nb::overload_cast<const ScalarQuadraticFunction &, ConstraintSense, CoeffT,
	                           const char *>(&MindoptModelMixin::add_quadratic_constraint),
	         nb::arg("expr"), nb::arg("sense"), nb::arg("rhs"), nb::arg("name") = "")
	    .def("add_quadratic_constraint",
	         nb::overload_cast<const ExprBuilder &, ConstraintSense, CoeffT, const char *>(
	             &MindoptModelMixin::add_quadratic_constraint_from_expr),
	         nb::arg("expr"), nb::arg("sense"), nb::arg("rhs"), nb::arg("name") = "")
	    .def("add_sos_constraint", nb::overload_cast<const Vector<VariableIndex> &, SOSType>(
	                                   &MindoptModelMixin::add_sos_constraint))
	    .def("add_sos_constraint",
	         nb::overload_cast<const Vector<VariableIndex> &, SOSType, const Vector<CoeffT> &>(
	             &MindoptModelMixin::add_sos_constraint))
	    // clang-format off
		BIND_F(delete_constraint)
		BIND_F(is_constraint_active)
	    // clang-format on

	    .def("set_objective",
	         nb::overload_cast<const ScalarQuadraticFunction &, ObjectiveSense>(
	             &MindoptModelMixin::set_objective),
	         nb::arg("expr"), nb::arg("sense") = ObjectiveSense::Minimize)
	    .def("set_objective",
	         nb::overload_cast<const ScalarAffineFunction &, ObjectiveSense>(
	             &MindoptModelMixin::set_objective),
	         nb::arg("expr"), nb::arg("sense") = ObjectiveSense::Minimize)
	    .def("set_objective",
	         nb::overload_cast<const ExprBuilder &, ObjectiveSense>(
	             &MindoptModelMixin::set_objective),
	         nb::arg("expr"), nb::arg("sense") = ObjectiveSense::Minimize)
	    .def("set_objective",
	         nb::overload_cast<const VariableIndex &, ObjectiveSense>(
	             &MindoptModelMixin::set_objective_as_variable),
	         nb::arg("expr"), nb::arg("sense") = ObjectiveSense::Minimize)
	    .def(
	        "set_objective",
	        nb::overload_cast<CoeffT, ObjectiveSense>(&MindoptModelMixin::set_objective_as_constant),
	        nb::arg("expr"), nb::arg("sense") = ObjectiveSense::Minimize)

	    .def("optimize", &MindoptModelMixin::optimize, nb::call_guard<nb::gil_scoped_release>())

	    // clang-format off
		BIND_F(version_string)
		BIND_F(get_raw_model)

		BIND_F(set_raw_parameter_int)
		BIND_F(set_raw_parameter_double)
		BIND_F(set_raw_parameter_string)
		BIND_F(get_raw_parameter_int)
		BIND_F(get_raw_parameter_double)
		BIND_F(get_raw_parameter_string)

		BIND_F(raw_attribute_type)

		BIND_F(set_model_raw_attribute_int)
		BIND_F(set_model_raw_attribute_double)
		BIND_F(set_model_raw_attribute_string)
		BIND_F(get_model_raw_attribute_int)
		BIND_F(get_model_raw_attribute_double)
		BIND_F(get_model_raw_attribute_string)
		BIND_F(get_model_raw_attribute_vector_double)
		BIND_F(get_model_raw_attribute_list_double)

		BIND_F(set_variable_raw_attribute_int)
		BIND_F(set_variable_raw_attribute_char)
		BIND_F(set_variable_raw_attribute_double)
		BIND_F(set_variable_raw_attribute_string)
		BIND_F(get_variable_raw_attribute_int)
		BIND_F(get_variable_raw_attribute_char)
		BIND_F(get_variable_raw_attribute_double)
		BIND_F(get_variable_raw_attribute_string)

		BIND_F(set_constraint_raw_attribute_int)
		BIND_F(set_constraint_raw_attribute_char)
		BIND_F(set_constraint_raw_attribute_double)
		BIND_F(set_constraint_raw_attribute_string)
		BIND_F(get_constraint_raw_attribute_int)
		BIND_F(get_constraint_raw_attribute_char)
		BIND_F(get_constraint_raw_attribute_double)
		BIND_F(get_constraint_raw_attribute_string)

		BIND_F(get_normalized_rhs)
		BIND_F(set_normalized_rhs)
		BIND_F(get_normalized_coefficient)
		BIND_F(set_normalized_coefficient)
		BIND_F(get_objective_coefficient)
		BIND_F(set_objective_coefficient)

		BIND_F(computeIIS)
	    // clang-format on
	    ;
}