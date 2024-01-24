#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>

#include "pyoptinterface/copt_model.hpp"

namespace nb = nanobind;

extern void bind_copt_constants(nb::module_ &m);

NB_MODULE(copt_model_ext, m)
{
	bind_copt_constants(m);

	nb::class_<COPTEnvConfig>(m, "EnvConfig").def(nb::init<>()).def("set", &COPTEnvConfig::set);

	nb::class_<COPTEnv>(m, "Env").def(nb::init<>()).def(nb::init<COPTEnvConfig &>());

	nb::class_<COPTModel>(m, "_RawModelBase");

#define BIND_F(f) .def(#f, &COPTModelMixin::f)
	nb::class_<COPTModelMixin, COPTModel>(m, "RawModel")
	    .def(nb::init<>())
	    .def(nb::init<const COPTEnv &>())
	    // clang-format off
	    BIND_F(init)
	    // clang-format on

	    .def("add_variable", &COPTModelMixin::add_variable,
	         nb::arg("domain") = VariableDomain::Continuous, nb::arg("lb") = -COPT_INFINITY,
	         nb::arg("ub") = COPT_INFINITY, nb::arg("name") = "")
	    // clang-format off
	    BIND_F(delete_variable)
	    BIND_F(is_variable_active)
	    // clang-format on

	    .def("get_value",
	         nb::overload_cast<const VariableIndex &>(&COPTModelMixin::get_variable_value))
	    .def("get_value",
	         nb::overload_cast<const ScalarAffineFunction &>(&COPTModelMixin::get_expression_value))
	    .def("get_value", nb::overload_cast<const ScalarQuadraticFunction &>(
	                          &COPTModelMixin::get_expression_value))
	    .def("get_value",
	         nb::overload_cast<const ExprBuilder &>(&COPTModelMixin::get_expression_value))

	    .def("pprint", &COPTModelMixin::pprint_variable)
	    .def("pprint",
	         nb::overload_cast<const ScalarAffineFunction &, int>(
	             &COPTModelMixin::pprint_expression),
	         nb::arg("expr"), nb::arg("precision") = 4)
	    .def("pprint",
	         nb::overload_cast<const ScalarQuadraticFunction &, int>(
	             &COPTModelMixin::pprint_expression),
	         nb::arg("expr"), nb::arg("precision") = 4)
	    .def("pprint",
	         nb::overload_cast<const ExprBuilder &, int>(&COPTModelMixin::pprint_expression),
	         nb::arg("expr"), nb::arg("precision") = 4)

	    .def("add_linear_constraint",
	         nb::overload_cast<const ScalarAffineFunction &, ConstraintSense, CoeffT>(
	             &COPTModelMixin::add_linear_constraint))
	    .def("add_linear_constraint",
	         nb::overload_cast<const ExprBuilder &, ConstraintSense, CoeffT>(
	             &COPTModelMixin::add_linear_constraint_from_expr))
	    .def("add_quadratic_constraint",
	         nb::overload_cast<const ScalarQuadraticFunction &, ConstraintSense, CoeffT>(
	             &COPTModelMixin::add_quadratic_constraint))
	    .def("add_quadratic_constraint",
	         nb::overload_cast<const ExprBuilder &, ConstraintSense, CoeffT>(
	             &COPTModelMixin::add_quadratic_constraint_from_expr))
	    // clang-format off
		BIND_F(add_sos1_constraint)
		BIND_F(add_sos2_constraint)
		BIND_F(delete_constraint)
		BIND_F(is_constraint_active)
	    // clang-format on

	    .def("set_objective", nb::overload_cast<const ScalarQuadraticFunction &, ObjectiveSense>(
	                              &COPTModelMixin::set_objective))
	    .def("set_objective", nb::overload_cast<const ScalarAffineFunction &, ObjectiveSense>(
	                              &COPTModelMixin::set_objective))
	    .def("set_objective",
	         nb::overload_cast<const ExprBuilder &, ObjectiveSense>(&COPTModelMixin::set_objective))

	    // clang-format off
	    BIND_F(optimize)
	    BIND_F(version_string)
	    BIND_F(get_raw_model)

	    BIND_F(raw_parameter_attribute_type)

	    BIND_F(set_raw_parameter_int)
	    BIND_F(set_raw_parameter_double)
	    BIND_F(get_raw_parameter_int)
	    BIND_F(get_raw_parameter_double)

	    BIND_F(get_raw_attribute_int)
	    BIND_F(get_raw_attribute_double)

		BIND_F(get_variable_info)
	    BIND_F(set_variable_name)
	    BIND_F(get_variable_name)
	    BIND_F(set_variable_type)
	    BIND_F(get_variable_type)
	    BIND_F(set_variable_lower_bound)
	    BIND_F(set_variable_upper_bound)

		BIND_F(get_constraint_info)
	    BIND_F(set_constraint_name)
	    BIND_F(get_constraint_name)

	    BIND_F(set_obj_sense)

	    BIND_F(add_mip_start)

		BIND_F(get_normalized_rhs)
		BIND_F(set_normalized_rhs)
		BIND_F(get_normalized_coefficient)
		BIND_F(set_normalized_coefficient)
		BIND_F(get_objective_coefficient)
		BIND_F(set_objective_coefficient)
	    // clang-format on
	    ;
}