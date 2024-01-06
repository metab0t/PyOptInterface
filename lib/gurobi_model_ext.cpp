#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>

#include "pyoptinterface/gurobi_model.hpp"

namespace nb = nanobind;

extern void bind_gurobi_constants(nb::module_ &m);

NB_MODULE(gurobi_model_ext, m)
{
	bind_gurobi_constants(m);

	nb::class_<GurobiEnv>(m, "Env")
	    .def(nb::init<bool>(), nb::arg("empty") = false)
	    .def("start", &GurobiEnv::start)
	    .def("raw_parameter_type", &GurobiEnv::raw_parameter_type)
	    .def("set_raw_parameter_int", &GurobiEnv::set_raw_parameter_int)
	    .def("set_raw_parameter_double", &GurobiEnv::set_raw_parameter_double)
	    .def("set_raw_parameter_string", &GurobiEnv::set_raw_parameter_string);

	nb::class_<GurobiModel>(m, "_RawModelBase");

	nb::class_<GurobiModelMixin, GurobiModel>(m, "RawModel")
	    .def(nb::init<>())
	    .def(nb::init<const GurobiEnv &>())
	    .def("init", &GurobiModelMixin::init)

	    .def("add_variable", &GurobiModelMixin::add_variable,
	         nb::arg("domain") = VariableDomain::Continuous, nb::arg("lb") = -GRB_INFINITY,
	         nb::arg("ub") = GRB_INFINITY, nb::arg("name") = "")
	    .def("delete_variable", &GurobiModelMixin::delete_variable)
	    .def("is_variable_active", &GurobiModelMixin::is_variable_active)

	    .def("get_value",
	         nb::overload_cast<const VariableIndex &>(&GurobiModelMixin::get_variable_value))
	    .def("get_value",
	         nb::overload_cast<const ScalarAffineFunction &>(&GurobiModelMixin::get_expression_value))
	    .def("get_value",
	         nb::overload_cast<const ScalarQuadraticFunction &>(&GurobiModelMixin::get_expression_value))
	    .def("get_value",
	         nb::overload_cast<const ExprBuilder &>(&GurobiModelMixin::get_expression_value))
	    .def("add_linear_constraint",
	         nb::overload_cast<const ScalarAffineFunction &, ConstraintSense, CoeffT>(
	             &GurobiModelMixin::add_linear_constraint))

	    .def("pprint", &GurobiModelMixin::pprint_variable)
	    .def("pprint",
	         nb::overload_cast<const ScalarAffineFunction &, int>(&GurobiModelMixin::pprint_expression),
	         nb::arg("expr"), nb::arg("precision") = 4)
	    .def("pprint",
	         nb::overload_cast<const ScalarQuadraticFunction &, int>(
	             &GurobiModelMixin::pprint_expression),
	         nb::arg("expr"), nb::arg("precision") = 4)
	    .def("pprint", nb::overload_cast<const ExprBuilder &, int>(&GurobiModelMixin::pprint_expression),
	         nb::arg("expr"), nb::arg("precision") = 4)

	    .def("set_variable_name", &GurobiModelMixin::set_variable_name)
	    .def("set_constraint_name", &GurobiModelMixin::set_constraint_name)

	    .def("add_linear_constraint",
	         nb::overload_cast<const ExprBuilder &, ConstraintSense, CoeffT>(
	             &GurobiModelMixin::add_linear_constraint_from_expr))
	    .def("add_quadratic_constraint",
	         nb::overload_cast<const ScalarQuadraticFunction &, ConstraintSense, CoeffT>(
	             &GurobiModelMixin::add_quadratic_constraint))
	    .def("add_quadratic_constraint",
	         nb::overload_cast<const ExprBuilder &, ConstraintSense, CoeffT>(
	             &GurobiModelMixin::add_quadratic_constraint_from_expr))
	    .def("add_sos1_constraint", &GurobiModelMixin::add_sos1_constraint)
	    .def("add_sos2_constraint", &GurobiModelMixin::add_sos2_constraint)
	    .def("delete_constraint", &GurobiModelMixin::delete_constraint)
	    .def("is_constraint_active", &GurobiModelMixin::is_constraint_active)

	    .def("set_objective", nb::overload_cast<const ScalarQuadraticFunction &, ObjectiveSense>(
	                              &GurobiModelMixin::set_objective))
	    .def("set_objective", nb::overload_cast<const ScalarAffineFunction &, ObjectiveSense>(
	                              &GurobiModelMixin::set_objective))
	    .def("set_objective",
	         nb::overload_cast<const ExprBuilder &, ObjectiveSense>(&GurobiModelMixin::set_objective))
	    .def("optimize", &GurobiModelMixin::optimize)
	    .def("update", &GurobiModelMixin::update)
	    .def("version_string", &GurobiModelMixin::version_string)
	    .def("get_raw_model", &GurobiModelMixin::get_raw_model)

	    .def("raw_parameter_type", &GurobiModelMixin::raw_parameter_type)
	    .def("set_raw_parameter_int", &GurobiModelMixin::set_raw_parameter_int)
	    .def("set_raw_parameter_double", &GurobiModelMixin::set_raw_parameter_double)
	    .def("set_raw_parameter_string", &GurobiModelMixin::set_raw_parameter_string)
	    .def("get_raw_parameter_int", &GurobiModelMixin::get_raw_parameter_int)
	    .def("get_raw_parameter_double", &GurobiModelMixin::get_raw_parameter_double)
	    .def("get_raw_parameter_string", &GurobiModelMixin::get_raw_parameter_string)

	    .def("raw_attribute_type", &GurobiModelMixin::raw_attribute_type)

	    .def("set_model_raw_attribute_int", &GurobiModelMixin::set_model_raw_attribute_int)
	    .def("set_model_raw_attribute_double", &GurobiModelMixin::set_model_raw_attribute_double)
	    .def("set_model_raw_attribute_string", &GurobiModelMixin::set_model_raw_attribute_string)
	    .def("get_model_raw_attribute_int", &GurobiModelMixin::get_model_raw_attribute_int)
	    .def("get_model_raw_attribute_double", &GurobiModelMixin::get_model_raw_attribute_double)
	    .def("get_model_raw_attribute_string", &GurobiModelMixin::get_model_raw_attribute_string)
	    .def("get_model_raw_attribute_vector_double",
	         &GurobiModelMixin::get_model_raw_attribute_vector_double)
	    .def("get_model_raw_attribute_list_double",
	         &GurobiModelMixin::get_model_raw_attribute_list_double)

	    .def("set_variable_raw_attribute_int", &GurobiModelMixin::set_variable_raw_attribute_int)
	    .def("set_variable_raw_attribute_char", &GurobiModelMixin::set_variable_raw_attribute_char)
	    .def("set_variable_raw_attribute_double", &GurobiModelMixin::set_variable_raw_attribute_double)
	    .def("set_variable_raw_attribute_string", &GurobiModelMixin::set_variable_raw_attribute_string)
	    .def("get_variable_raw_attribute_int", &GurobiModelMixin::get_variable_raw_attribute_int)
	    .def("get_variable_raw_attribute_char", &GurobiModelMixin::get_variable_raw_attribute_char)
	    .def("get_variable_raw_attribute_double", &GurobiModelMixin::get_variable_raw_attribute_double)
	    .def("get_variable_raw_attribute_string", &GurobiModelMixin::get_variable_raw_attribute_string)

	    .def("set_constraint_raw_attribute_int", &GurobiModelMixin::set_constraint_raw_attribute_int)
	    .def("set_constraint_raw_attribute_char", &GurobiModelMixin::set_constraint_raw_attribute_char)
	    .def("set_constraint_raw_attribute_double",
	         &GurobiModelMixin::set_constraint_raw_attribute_double)
	    .def("set_constraint_raw_attribute_string",
	         &GurobiModelMixin::set_constraint_raw_attribute_string)
	    .def("get_constraint_raw_attribute_int", &GurobiModelMixin::get_constraint_raw_attribute_int)
	    .def("get_constraint_raw_attribute_char", &GurobiModelMixin::get_constraint_raw_attribute_char)
	    .def("get_constraint_raw_attribute_double",
	         &GurobiModelMixin::get_constraint_raw_attribute_double)
	    .def("get_constraint_raw_attribute_string",
	         &GurobiModelMixin::get_constraint_raw_attribute_string);
}