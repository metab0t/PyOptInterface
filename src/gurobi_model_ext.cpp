#include <nanobind/nanobind.h>
#include <nanobind/stl/bind_vector.h>
#include <nanobind/stl/bind_map.h>
#include <nanobind/stl/optional.h>

#include "pyoptinterface/gurobi_model.hpp"

namespace nb = nanobind;

NB_MODULE(gurobi_model_ext, m)
{
	nb::class_<GurobiEnv>(m, "Env").def(nb::init<>());

	nb::class_<GurobiModel>(m, "Model")
	    .def(nb::init<>())
	    .def(nb::init<const GurobiEnv &>())
	    .def("init", &GurobiModel::init)
	    .def("add_variable", &GurobiModel::add_variable,
	         nb::arg("domain") = VariableDomain::Continuous)
	    .def("delete_variable", &GurobiModel::delete_variable)
	    .def("is_variable_active", &GurobiModel::is_variable_active)
	    .def("add_linear_constraint", &GurobiModel::add_linear_constraint)
	    .def("add_quadratic_constraint", &GurobiModel::add_quadratic_constraint)
	    .def("add_sos1_constraint", &GurobiModel::add_sos1_constraint)
	    .def("add_sos2_constraint", &GurobiModel::add_sos2_constraint)
	    .def("delete_constraint", &GurobiModel::delete_constraint)
	    .def("is_constraint_active", &GurobiModel::is_constraint_active)
	    .def("set_objective", nb::overload_cast<const ScalarQuadraticFunction &, ObjectiveSense>(
	                              &GurobiModel::set_objective))
	    .def("set_objective", nb::overload_cast<const ScalarAffineFunction &, ObjectiveSense>(
	                              &GurobiModel::set_objective))
	    .def("optimize", &GurobiModel::optimize)
	    .def("update", &GurobiModel::update)
	    .def("set_variable_raw_attribute_int", &GurobiModel::set_variable_raw_attribute_int)
	    .def("set_variable_raw_attribute_char", &GurobiModel::set_variable_raw_attribute_char)
	    .def("set_variable_raw_attribute_double", &GurobiModel::set_variable_raw_attribute_double)
	    .def("set_variable_raw_attribute_string", &GurobiModel::set_variable_raw_attribute_string)
	    .def("get_variable_raw_attribute_int", &GurobiModel::get_variable_raw_attribute_int)
	    .def("get_variable_raw_attribute_char", &GurobiModel::get_variable_raw_attribute_char)
	    .def("get_variable_raw_attribute_double", &GurobiModel::get_variable_raw_attribute_double)
	    .def("get_variable_raw_attribute_string", &GurobiModel::get_variable_raw_attribute_string);
}