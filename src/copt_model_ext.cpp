#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>

#include "pyoptinterface/copt_model.hpp"

namespace nb = nanobind;

extern void bind_copt_constants(nb::module_ &m);

NB_MODULE(copt_model_ext, m)
{
	bind_copt_constants(m);

	nb::class_<COPTEnv>(m, "Env").def(nb::init<>());

	nb::class_<COPTModel>(m, "RawModel")
	    .def(nb::init<>())
	    .def(nb::init<const COPTEnv &>())
	    .def("init", &COPTModel::init)
	    .def("add_variable", &COPTModel::add_variable,
	         nb::arg("domain") = VariableDomain::Continuous)
	    .def("delete_variable", &COPTModel::delete_variable)
	    .def("is_variable_active", &COPTModel::is_variable_active)
	    .def("add_linear_constraint",
	         nb::overload_cast<const ScalarAffineFunction &, ConstraintSense, CoeffT>(
	             &COPTModel::add_linear_constraint))
	    .def("add_linear_constraint",
	         nb::overload_cast<const ExprBuilder &, ConstraintSense, CoeffT>(
	             &COPTModel::add_linear_constraint_from_expr))
	    .def("add_quadratic_constraint",
	         nb::overload_cast<const ScalarQuadraticFunction &, ConstraintSense, CoeffT>(
	             &COPTModel::add_quadratic_constraint))
	    .def("add_quadratic_constraint",
	         nb::overload_cast<const ExprBuilder &, ConstraintSense, CoeffT>(
	             &COPTModel::add_quadratic_constraint_from_expr))
	    .def("add_sos1_constraint", &COPTModel::add_sos1_constraint)
	    .def("add_sos2_constraint", &COPTModel::add_sos2_constraint)
	    .def("delete_constraint", &COPTModel::delete_constraint)
	    .def("is_constraint_active", &COPTModel::is_constraint_active)
	    .def("set_objective", nb::overload_cast<const ScalarQuadraticFunction &, ObjectiveSense>(
	                              &COPTModel::set_objective))
	    .def("set_objective", nb::overload_cast<const ScalarAffineFunction &, ObjectiveSense>(
	                              &COPTModel::set_objective))
	    .def("set_objective",
	         nb::overload_cast<const ExprBuilder &, ObjectiveSense>(&COPTModel::set_objective))
	    .def("optimize", &COPTModel::optimize)
	    .def("version_string", &COPTModel::version_string)
	    .def("get_raw_model", &COPTModel::get_raw_model)

	    .def("set_parameter_int", &COPTModel::set_parameter_int)
	    .def("set_parameter_double", &COPTModel::set_parameter_double)
	    .def("get_parameter_int", &COPTModel::get_parameter_int)
	    .def("get_parameter_double", &COPTModel::get_parameter_double)

	    .def("get_raw_attribute_int", &COPTModel::get_raw_attribute_int)
	    .def("get_raw_attribute_double", &COPTModel::get_raw_attribute_double)

	    .def("set_variable_name", &COPTModel::set_variable_name)
	    .def("get_variable_name", &COPTModel::get_variable_name)
	    .def("set_variable_type", &COPTModel::set_variable_type)
	    .def("get_variable_type", &COPTModel::get_variable_type)
	    .def("set_variable_lower_bound", &COPTModel::set_variable_lower_bound)
	    .def("set_variable_upper_bound", &COPTModel::set_variable_upper_bound)
	    .def("set_constraint_name", &COPTModel::set_constraint_name)
	    .def("get_constraint_name", &COPTModel::get_constraint_name)
	    .def("get_variable_info", &COPTModel::get_variable_info)
	    .def("get_constraint_info", &COPTModel::get_constraint_info)

	    .def("add_mip_start", &COPTModel::add_mip_start);
}