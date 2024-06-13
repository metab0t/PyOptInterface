#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>
#include <nanobind/stl/function.h>

#include "pyoptinterface/copt_model.hpp"

namespace nb = nanobind;

extern void bind_copt_constants(nb::module_ &m);

NB_MODULE(copt_model_ext, m)
{
	m.import_("core_ext");

	m.def("is_library_loaded", &copt::is_library_loaded);
	m.def("load_library", &copt::load_library);

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
	    BIND_F(write)
	    // clang-format on

	    .def("add_variable", &COPTModelMixin::add_variable,
	         nb::arg("domain") = VariableDomain::Continuous, nb::arg("lb") = -COPT_INFINITY,
	         nb::arg("ub") = COPT_INFINITY, nb::arg("name") = "")
	    // clang-format off
	    BIND_F(delete_variable)
	    BIND_F(delete_variables)
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
	         nb::overload_cast<const ScalarAffineFunction &, ConstraintSense, CoeffT, const char *>(
	             &COPTModelMixin::add_linear_constraint),
	         nb::arg("expr"), nb::arg("sense"), nb::arg("rhs"), nb::arg("name") = "")
	    .def("add_linear_constraint",
	         nb::overload_cast<const VariableIndex &, ConstraintSense, CoeffT, const char *>(
	             &COPTModelMixin::add_linear_constraint_from_var),
	         nb::arg("expr"), nb::arg("sense"), nb::arg("rhs"), nb::arg("name") = "")
	    .def("add_linear_constraint",
	         nb::overload_cast<const ExprBuilder &, ConstraintSense, CoeffT, const char *>(
	             &COPTModelMixin::add_linear_constraint_from_expr),
	         nb::arg("expr"), nb::arg("sense"), nb::arg("rhs"), nb::arg("name") = "")
	    .def("add_quadratic_constraint",
	         nb::overload_cast<const ScalarQuadraticFunction &, ConstraintSense, CoeffT,
	                           const char *>(&COPTModelMixin::add_quadratic_constraint),
	         nb::arg("expr"), nb::arg("sense"), nb::arg("rhs"), nb::arg("name") = "")
	    .def("add_quadratic_constraint",
	         nb::overload_cast<const ExprBuilder &, ConstraintSense, CoeffT, const char *>(
	             &COPTModelMixin::add_quadratic_constraint_from_expr),
	         nb::arg("expr"), nb::arg("sense"), nb::arg("rhs"), nb::arg("name") = "")
	    .def("add_second_order_cone_constraint", &COPTModelMixin::add_second_order_cone_constraint,
	         nb::arg("variables"), nb::arg("name") = "")
	    .def("add_sos_constraint", nb::overload_cast<const Vector<VariableIndex> &, SOSType>(
	                                   &COPTModelMixin::add_sos_constraint))
	    .def("add_sos_constraint",
	         nb::overload_cast<const Vector<VariableIndex> &, SOSType, const Vector<CoeffT> &>(
	             &COPTModelMixin::add_sos_constraint))
	    // clang-format off
		BIND_F(delete_constraint)
		BIND_F(is_constraint_active)
	    // clang-format on

	    .def("set_objective",
	         nb::overload_cast<const ScalarQuadraticFunction &, ObjectiveSense>(
	             &COPTModelMixin::set_objective),
	         nb::arg("expr"), nb::arg("sense") = ObjectiveSense::Minimize)
	    .def("set_objective",
	         nb::overload_cast<const ScalarAffineFunction &, ObjectiveSense>(
	             &COPTModelMixin::set_objective),
	         nb::arg("expr"), nb::arg("sense") = ObjectiveSense::Minimize)
	    .def("set_objective",
	         nb::overload_cast<const ExprBuilder &, ObjectiveSense>(&COPTModelMixin::set_objective),
	         nb::arg("expr"), nb::arg("sense") = ObjectiveSense::Minimize)
	    .def("set_objective",
	         nb::overload_cast<CoeffT, ObjectiveSense>(&COPTModelMixin::set_objective_as_constant),
	         nb::arg("expr"), nb::arg("sense") = ObjectiveSense::Minimize)

		.def("cb_add_lazy_constraint",
	         nb::overload_cast<const ScalarAffineFunction &, ConstraintSense, CoeffT>(
	             &COPTModelMixin::cb_add_lazy_constraint),
	         nb::arg("expr"), nb::arg("sense"), nb::arg("rhs"))
	    .def("cb_add_lazy_constraint",
	         nb::overload_cast<const ExprBuilder &, ConstraintSense, CoeffT>(
	             &COPTModelMixin::cb_add_lazy_constraint),
	         nb::arg("expr"), nb::arg("sense"), nb::arg("rhs"))
	    .def("cb_add_user_cut",
	         nb::overload_cast<const ScalarAffineFunction &, ConstraintSense, CoeffT>(
	             &COPTModelMixin::cb_add_user_cut),
	         nb::arg("expr"), nb::arg("sense"), nb::arg("rhs"))
	    .def("cb_add_user_cut",
	         nb::overload_cast<const ExprBuilder &, ConstraintSense, CoeffT>(
	             &COPTModelMixin::cb_add_user_cut),
	         nb::arg("expr"), nb::arg("sense"), nb::arg("rhs"))

		.def("optimize", &COPTModelMixin::optimize, nb::call_guard<nb::gil_scoped_release>())

	    // clang-format off
	    BIND_F(version_string)
	    BIND_F(get_raw_model)

		BIND_F(set_callback)
		BIND_F(cb_get_info_int)
		BIND_F(cb_get_info_double)
		BIND_F(cb_get_solution)
		BIND_F(cb_get_relaxation)
		BIND_F(cb_get_incumbent)
		BIND_F(cb_set_solution)
		BIND_F(cb_submit_solution)
		BIND_F(cb_exit)

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