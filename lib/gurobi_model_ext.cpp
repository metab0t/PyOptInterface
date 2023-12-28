#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>

#include "pyoptinterface/gurobi_model.hpp"

namespace nb = nanobind;

extern void bind_gurobi_constants(nb::module_ &m);

NB_MODULE(gurobi_model_ext, m)
{
	bind_gurobi_constants(m);

	nb::class_<GurobiEnv>(m, "Env").def(nb::init<>());

	nb::class_<GurobiModel, CommercialSolverBase>(m, "RawModel")
	    .def(nb::init<>())
	    .def(nb::init<const GurobiEnv &>())
	    .def("init", &GurobiModel::init)

	    .def("add_variable", &GurobiModel::add_variable,
	         nb::arg("domain") = VariableDomain::Continuous, nb::arg("lb") = -GRB_INFINITY,
	         nb::arg("ub") = GRB_INFINITY, nb::arg("name") = nullptr)
	    .def("delete_variable", &GurobiModel::delete_variable)
	    .def("is_variable_active", &GurobiModel::is_variable_active)

	    .def("get_value",
	         nb::overload_cast<const VariableIndex &>(&GurobiModel::get_variable_value))
	    .def("get_value",
	         nb::overload_cast<const ScalarAffineFunction &>(&GurobiModel::get_expression_value))
	    .def("get_value",
	         nb::overload_cast<const ScalarQuadraticFunction &>(&GurobiModel::get_expression_value))
	    .def("get_value",
	         nb::overload_cast<const ExprBuilder &>(&GurobiModel::get_expression_value))
	    .def("add_linear_constraint",
	         nb::overload_cast<const ScalarAffineFunction &, ConstraintSense, CoeffT>(
	             &GurobiModel::add_linear_constraint))

	    .def("pprint", &GurobiModel::pprint_variable)
	    .def("pprint",
	         nb::overload_cast<const ScalarAffineFunction &, int>(&GurobiModel::pprint_expression),
	         nb::arg("expr"), nb::arg("precision") = 4)
	    .def("pprint",
	         nb::overload_cast<const ScalarQuadraticFunction &, int>(
	             &GurobiModel::pprint_expression),
	         nb::arg("expr"), nb::arg("precision") = 4)
	    .def("pprint", nb::overload_cast<const ExprBuilder &, int>(&GurobiModel::pprint_expression),
	         nb::arg("expr"), nb::arg("precision") = 4)

	    .def("set_variable_name", &GurobiModel::set_variable_name)

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
	    .def("set_raw_parameter_int", &GurobiModel::set_raw_parameter_int)
	    .def("set_raw_parameter_double", &GurobiModel::set_raw_parameter_double)
	    .def("set_raw_parameter_string", &GurobiModel::set_raw_parameter_string)
	    .def("get_raw_parameter_int", &GurobiModel::get_raw_parameter_int)
	    .def("get_raw_parameter_double", &GurobiModel::get_raw_parameter_double)
	    .def("get_raw_parameter_string", &GurobiModel::get_raw_parameter_string)

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