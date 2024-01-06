#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>

#include "pyoptinterface/copt_model.hpp"

namespace nb = nanobind;

extern void bind_copt_constants(nb::module_ &m);

NB_MODULE(copt_model_ext, m)
{
	bind_copt_constants(m);

	nb::class_<COPTEnvConfig>(m, "EnvConfig")
	    .def(nb::init<>())
	    .def("config", &COPTEnvConfig::config);

	nb::class_<COPTEnv>(m, "Env").def(nb::init<>()).def(nb::init<COPTEnvConfig &>());

	nb::class_<COPTModel>(m, "_RawModelBase");

	nb::class_<COPTModelMixin, COPTModel>(m, "RawModel")
	    .def(nb::init<>())
	    .def(nb::init<const COPTEnv &>())
	    .def("init", &COPTModelMixin::init)

	    .def("add_variable", &COPTModelMixin::add_variable,
	         nb::arg("domain") = VariableDomain::Continuous, nb::arg("lb") = -COPT_INFINITY,
	         nb::arg("ub") = COPT_INFINITY, nb::arg("name") = "")
	    .def("delete_variable", &COPTModelMixin::delete_variable)
	    .def("is_variable_active", &COPTModelMixin::is_variable_active)

	    .def("get_value", nb::overload_cast<const VariableIndex &>(&COPTModelMixin::get_variable_value))
	    .def("get_value",
	         nb::overload_cast<const ScalarAffineFunction &>(&COPTModelMixin::get_expression_value))
	    .def("get_value",
	         nb::overload_cast<const ScalarQuadraticFunction &>(&COPTModelMixin::get_expression_value))
	    .def("get_value", nb::overload_cast<const ExprBuilder &>(&COPTModelMixin::get_expression_value))

	    .def("pprint", &COPTModelMixin::pprint_variable)
	    .def("pprint",
	         nb::overload_cast<const ScalarAffineFunction &, int>(&COPTModelMixin::pprint_expression),
	         nb::arg("expr"), nb::arg("precision") = 4)
	    .def("pprint",
	         nb::overload_cast<const ScalarQuadraticFunction &, int>(&COPTModelMixin::pprint_expression),
	         nb::arg("expr"), nb::arg("precision") = 4)
	    .def("pprint", nb::overload_cast<const ExprBuilder &, int>(&COPTModelMixin::pprint_expression),
	         nb::arg("expr"), nb::arg("precision") = 4)

	    .def("set_variable_name", &COPTModelMixin::set_variable_name)

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
	    .def("add_sos1_constraint", &COPTModelMixin::add_sos1_constraint)
	    .def("add_sos2_constraint", &COPTModelMixin::add_sos2_constraint)
	    .def("delete_constraint", &COPTModelMixin::delete_constraint)
	    .def("is_constraint_active", &COPTModelMixin::is_constraint_active)

	    .def("set_objective", nb::overload_cast<const ScalarQuadraticFunction &, ObjectiveSense>(
	                              &COPTModelMixin::set_objective))
	    .def("set_objective", nb::overload_cast<const ScalarAffineFunction &, ObjectiveSense>(
	                              &COPTModelMixin::set_objective))
	    .def("set_objective",
	         nb::overload_cast<const ExprBuilder &, ObjectiveSense>(&COPTModelMixin::set_objective))
	    .def("optimize", &COPTModelMixin::optimize)
	    .def("version_string", &COPTModelMixin::version_string)
	    .def("get_raw_model", &COPTModelMixin::get_raw_model)

	    .def("set_raw_parameter_int", &COPTModelMixin::set_raw_parameter_int)
	    .def("set_raw_parameter_double", &COPTModelMixin::set_raw_parameter_double)
	    .def("get_raw_parameter_int", &COPTModelMixin::get_raw_parameter_int)
	    .def("get_raw_parameter_double", &COPTModelMixin::get_raw_parameter_double)

	    .def("get_raw_attribute_int", &COPTModelMixin::get_raw_attribute_int)
	    .def("get_raw_attribute_double", &COPTModelMixin::get_raw_attribute_double)

	    .def("set_variable_name", &COPTModelMixin::set_variable_name)
	    .def("get_variable_name", &COPTModelMixin::get_variable_name)
	    .def("set_variable_type", &COPTModelMixin::set_variable_type)
	    .def("get_variable_type", &COPTModelMixin::get_variable_type)
	    .def("set_variable_lower_bound", &COPTModelMixin::set_variable_lower_bound)
	    .def("set_variable_upper_bound", &COPTModelMixin::set_variable_upper_bound)
	    .def("set_constraint_name", &COPTModelMixin::set_constraint_name)
	    .def("get_constraint_name", &COPTModelMixin::get_constraint_name)
	    .def("get_variable_info", &COPTModelMixin::get_variable_info)
	    .def("get_constraint_info", &COPTModelMixin::get_constraint_info)

	    .def("add_mip_start", &COPTModelMixin::add_mip_start);
}