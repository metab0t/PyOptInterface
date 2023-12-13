#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>

#include "pyoptinterface/gurobi_model.hpp"

namespace nb = nanobind;

extern void bind_gurobi_constants(nb::module_ &m);

NB_MODULE(gurobi_model_ext, m)
{
	bind_gurobi_constants(m);

	nb::class_<GurobiEnv>(m, "Env").def(nb::init<>());

	nb::class_<GurobiModel>(m, "RawModel")
	    .def(nb::init<>())
	    .def(nb::init<const GurobiEnv &>())
	    .def("init", &GurobiModel::init)
	    .def("add_variable", &GurobiModel::add_variable,
	         nb::arg("domain") = VariableDomain::Continuous)
	    .def("delete_variable", &GurobiModel::delete_variable)
	    .def("is_variable_active", &GurobiModel::is_variable_active)
	    .def("add_linear_constraint",
	         nb::overload_cast<const ScalarAffineFunction &, ConstraintSense, CoeffT>(
	             &GurobiModel::add_linear_constraint))
	    .def("add_linear_constraint",
	         nb::overload_cast<const ExprBuilder &, ConstraintSense, CoeffT>(
	             &GurobiModel::add_linear_constraint_from_expr))
	    .def("add_quadratic_constraint",
	         nb::overload_cast<const ScalarQuadraticFunction &, ConstraintSense, CoeffT>(
	             &GurobiModel::add_quadratic_constraint))
	    .def("add_quadratic_constraint",
	         nb::overload_cast<const ExprBuilder &, ConstraintSense, CoeffT>(
	             &GurobiModel::add_quadratic_constraint_from_expr))
	    .def("add_sos1_constraint", &GurobiModel::add_sos1_constraint)
	    .def("add_sos2_constraint", &GurobiModel::add_sos2_constraint)
	    .def("delete_constraint", &GurobiModel::delete_constraint)
	    .def("is_constraint_active", &GurobiModel::is_constraint_active)
	    .def("set_objective", nb::overload_cast<const ScalarQuadraticFunction &, ObjectiveSense>(
	                              &GurobiModel::set_objective))
	    .def("set_objective", nb::overload_cast<const ScalarAffineFunction &, ObjectiveSense>(
	                              &GurobiModel::set_objective))
	    .def("set_objective",
	         nb::overload_cast<const ExprBuilder &, ObjectiveSense>(&GurobiModel::set_objective))
	    .def("optimize", &GurobiModel::optimize)
	    .def("update", &GurobiModel::update)
	    .def("version_string", &GurobiModel::version_string)
	    .def("get_raw_model", &GurobiModel::get_raw_model)

	    .def("raw_parameter_type", &GurobiModel::raw_parameter_type)
	    .def("set_parameter_int", &GurobiModel::set_parameter_int)
	    .def("set_parameter_double", &GurobiModel::set_parameter_double)
	    .def("set_parameter_string", &GurobiModel::set_parameter_string)
	    .def("get_parameter_int", &GurobiModel::get_parameter_int)
	    .def("get_parameter_double", &GurobiModel::get_parameter_double)
	    .def("get_parameter_string", &GurobiModel::get_parameter_string)

	    .def("raw_attribute_type", &GurobiModel::raw_attribute_type)

	    .def("set_model_raw_attribute_int", &GurobiModel::set_model_raw_attribute_int)
	    .def("set_model_raw_attribute_double", &GurobiModel::set_model_raw_attribute_double)
	    .def("set_model_raw_attribute_string", &GurobiModel::set_model_raw_attribute_string)
	    .def("get_model_raw_attribute_int", &GurobiModel::get_model_raw_attribute_int)
	    .def("get_model_raw_attribute_double", &GurobiModel::get_model_raw_attribute_double)
	    .def("get_model_raw_attribute_string", &GurobiModel::get_model_raw_attribute_string)
	    .def("get_model_raw_attribute_vector_double",
	         &GurobiModel::get_model_raw_attribute_vector_double)
	    .def("get_model_raw_attribute_list_double",
	         &GurobiModel::get_model_raw_attribute_list_double)

	    .def("set_variable_raw_attribute_int", &GurobiModel::set_variable_raw_attribute_int)
	    .def("set_variable_raw_attribute_char", &GurobiModel::set_variable_raw_attribute_char)
	    .def("set_variable_raw_attribute_double", &GurobiModel::set_variable_raw_attribute_double)
	    .def("set_variable_raw_attribute_string", &GurobiModel::set_variable_raw_attribute_string)
	    .def("get_variable_raw_attribute_int", &GurobiModel::get_variable_raw_attribute_int)
	    .def("get_variable_raw_attribute_char", &GurobiModel::get_variable_raw_attribute_char)
	    .def("get_variable_raw_attribute_double", &GurobiModel::get_variable_raw_attribute_double)
	    .def("get_variable_raw_attribute_string", &GurobiModel::get_variable_raw_attribute_string)

	    .def("set_constraint_raw_attribute_int", &GurobiModel::set_constraint_raw_attribute_int)
	    .def("set_constraint_raw_attribute_char", &GurobiModel::set_constraint_raw_attribute_char)
	    .def("set_constraint_raw_attribute_double",
	         &GurobiModel::set_constraint_raw_attribute_double)
	    .def("set_constraint_raw_attribute_string",
	         &GurobiModel::set_constraint_raw_attribute_string)
	    .def("get_constraint_raw_attribute_int", &GurobiModel::get_constraint_raw_attribute_int)
	    .def("get_constraint_raw_attribute_char", &GurobiModel::get_constraint_raw_attribute_char)
	    .def("get_constraint_raw_attribute_double",
	         &GurobiModel::get_constraint_raw_attribute_double)
	    .def("get_constraint_raw_attribute_string",
	         &GurobiModel::get_constraint_raw_attribute_string);
}