#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>

#include "pyoptinterface/gurobi_model.hpp"

namespace nb = nanobind;

extern void bind_gurobi_constants(nb::module_ &m);

NB_MODULE(gurobi_model_ext, m)
{
	bind_gurobi_constants(m);

#define BIND_F(f) .def(#f, &GurobiEnv::f)
	nb::class_<GurobiEnv>(m, "Env").def(nb::init<bool>(), nb::arg("empty") = false)
	    // clang-format off
		BIND_F(start)
		BIND_F(raw_parameter_type)
		BIND_F(set_raw_parameter_int)
		BIND_F(set_raw_parameter_double)
		BIND_F(set_raw_parameter_string)
	    // clang-format on
	    ;

	nb::class_<GurobiModel>(m, "_RawModelBase");

#undef BIND_F
#define BIND_F(f) .def(#f, &GurobiModelMixin::f)
	nb::class_<GurobiModelMixin, GurobiModel>(m, "RawModel")
	    .def(nb::init<>())
	    .def(nb::init<const GurobiEnv &>())
	    // clang-format off
	    BIND_F(init)
	    // clang-format on

	    .def("add_variable", &GurobiModelMixin::add_variable,
	         nb::arg("domain") = VariableDomain::Continuous, nb::arg("lb") = -GRB_INFINITY,
	         nb::arg("ub") = GRB_INFINITY, nb::arg("name") = "")
	    // clang-format off
	    BIND_F(delete_variable)
	    BIND_F(delete_variables)
	    BIND_F(is_variable_active)
	    // clang-format on

	    .def("get_value",
	         nb::overload_cast<const VariableIndex &>(&GurobiModelMixin::get_variable_value))
	    .def("get_value", nb::overload_cast<const ScalarAffineFunction &>(
	                          &GurobiModelMixin::get_expression_value))
	    .def("get_value", nb::overload_cast<const ScalarQuadraticFunction &>(
	                          &GurobiModelMixin::get_expression_value))
	    .def("get_value",
	         nb::overload_cast<const ExprBuilder &>(&GurobiModelMixin::get_expression_value))

	    .def("pprint", &GurobiModelMixin::pprint_variable)
	    .def("pprint",
	         nb::overload_cast<const ScalarAffineFunction &, int>(
	             &GurobiModelMixin::pprint_expression),
	         nb::arg("expr"), nb::arg("precision") = 4)
	    .def("pprint",
	         nb::overload_cast<const ScalarQuadraticFunction &, int>(
	             &GurobiModelMixin::pprint_expression),
	         nb::arg("expr"), nb::arg("precision") = 4)
	    .def("pprint",
	         nb::overload_cast<const ExprBuilder &, int>(&GurobiModelMixin::pprint_expression),
	         nb::arg("expr"), nb::arg("precision") = 4)

	    // clang-format off
		BIND_F(set_variable_name)
		BIND_F(set_constraint_name)
	    // clang-format on

	    .def("add_linear_constraint",
	         nb::overload_cast<const ScalarAffineFunction &, ConstraintSense, CoeffT, const char *>(
	             &GurobiModelMixin::add_linear_constraint),
	         nb::arg("expr"), nb::arg("sense"), nb::arg("rhs"), nb::arg("name") = "")
	    .def("add_linear_constraint",
	         nb::overload_cast<const VariableIndex &, ConstraintSense, CoeffT, const char *>(
	             &GurobiModelMixin::add_linear_constraint_from_var),
	         nb::arg("expr"), nb::arg("sense"), nb::arg("rhs"), nb::arg("name") = "")
	    .def("add_linear_constraint",
	         nb::overload_cast<const ExprBuilder &, ConstraintSense, CoeffT, const char *>(
	             &GurobiModelMixin::add_linear_constraint_from_expr),
	         nb::arg("expr"), nb::arg("sense"), nb::arg("rhs"), nb::arg("name") = "")
	    .def("add_quadratic_constraint",
	         nb::overload_cast<const ScalarQuadraticFunction &, ConstraintSense, CoeffT,
	                           const char *>(&GurobiModelMixin::add_quadratic_constraint),
	         nb::arg("expr"), nb::arg("sense"), nb::arg("rhs"), nb::arg("name") = "")
	    .def("add_quadratic_constraint",
	         nb::overload_cast<const ExprBuilder &, ConstraintSense, CoeffT, const char *>(
	             &GurobiModelMixin::add_quadratic_constraint_from_expr),
	         nb::arg("expr"), nb::arg("sense"), nb::arg("rhs"), nb::arg("name") = "")
	    .def("add_sos_constraint", nb::overload_cast<const Vector<VariableIndex> &, SOSType>(
	                                   &GurobiModelMixin::add_sos_constraint))
	    .def("add_sos_constraint",
	         nb::overload_cast<const Vector<VariableIndex> &, SOSType, const Vector<CoeffT> &>(
	             &GurobiModelMixin::add_sos_constraint))
	    // clang-format off
		BIND_F(delete_constraint)
		BIND_F(is_constraint_active)
	    // clang-format on

	    .def("set_objective", nb::overload_cast<const ScalarQuadraticFunction &, ObjectiveSense>(
	                              &GurobiModelMixin::set_objective))
	    .def("set_objective", nb::overload_cast<const ScalarAffineFunction &, ObjectiveSense>(
	                              &GurobiModelMixin::set_objective))
	    .def("set_objective", nb::overload_cast<const ExprBuilder &, ObjectiveSense>(
	                              &GurobiModelMixin::set_objective))

	    // clang-format off
	    BIND_F(optimize)
	    BIND_F(update)
	    BIND_F(version_string)
	    BIND_F(get_raw_model)

	    BIND_F(raw_parameter_type)
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
	    // clang-format on
	    ;
}