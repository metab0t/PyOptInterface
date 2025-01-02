#include <nanobind/nanobind.h>
#include <nanobind/stl/vector.h>
#include <nanobind/stl/string.h>

namespace nb = nanobind;

#include "pyoptinterface/ipopt_model.hpp"

NB_MODULE(ipopt_model_ext, m)
{
	m.import_("pyoptinterface._src.core_ext");
	m.import_("pyoptinterface._src.nleval_ext");

	m.def("is_library_loaded", &ipopt::is_library_loaded);
	m.def("load_library", &ipopt::load_library);

	nb::enum_<ApplicationReturnStatus>(m, "ApplicationReturnStatus")
	    .value("Solve_Succeeded", ApplicationReturnStatus::Solve_Succeeded)
	    .value("Solved_To_Acceptable_Level", ApplicationReturnStatus::Solved_To_Acceptable_Level)
	    .value("Infeasible_Problem_Detected", ApplicationReturnStatus::Infeasible_Problem_Detected)
	    .value("Search_Direction_Becomes_Too_Small",
	           ApplicationReturnStatus::Search_Direction_Becomes_Too_Small)
	    .value("Diverging_Iterates", ApplicationReturnStatus::Diverging_Iterates)
	    .value("User_Requested_Stop", ApplicationReturnStatus::User_Requested_Stop)
	    .value("Feasible_Point_Found", ApplicationReturnStatus::Feasible_Point_Found)
	    .value("Maximum_Iterations_Exceeded", ApplicationReturnStatus::Maximum_Iterations_Exceeded)
	    .value("Restoration_Failed", ApplicationReturnStatus::Restoration_Failed)
	    .value("Error_In_Step_Computation", ApplicationReturnStatus::Error_In_Step_Computation)
	    .value("Maximum_CpuTime_Exceeded", ApplicationReturnStatus::Maximum_CpuTime_Exceeded)
	    .value("Maximum_WallTime_Exceeded", ApplicationReturnStatus::Maximum_WallTime_Exceeded)
	    .value("Not_Enough_Degrees_Of_Freedom",
	           ApplicationReturnStatus::Not_Enough_Degrees_Of_Freedom)
	    .value("Invalid_Problem_Definition", ApplicationReturnStatus::Invalid_Problem_Definition)
	    .value("Invalid_Option", ApplicationReturnStatus::Invalid_Option)
	    .value("Invalid_Number_Detected", ApplicationReturnStatus::Invalid_Number_Detected)
	    .value("Unrecoverable_Exception", ApplicationReturnStatus::Unrecoverable_Exception)
	    .value("NonIpopt_Exception_Thrown", ApplicationReturnStatus::NonIpopt_Exception_Thrown)
	    .value("Insufficient_Memory", ApplicationReturnStatus::Insufficient_Memory)
	    .value("Internal_Error", ApplicationReturnStatus::Internal_Error);

	nb::class_<IpoptModel>(m, "RawModel")
	    .def(nb::init<>())
	    .def_ro("m_status", &IpoptModel::m_status)
	    .def("add_variable", &IpoptModel::add_variable, nb::arg("lb") = -INFINITY,
	         nb::arg("ub") = INFINITY, nb::arg("start") = 0.0, nb::arg("name") = "")
	    .def("get_variable_lb", &IpoptModel::get_variable_lb)
	    .def("get_variable_ub", &IpoptModel::get_variable_ub)
	    .def("set_variable_lb", &IpoptModel::set_variable_lb)
	    .def("set_variable_ub", &IpoptModel::set_variable_ub)
	    .def("set_variable_bounds", &IpoptModel::set_variable_bounds, nb::arg("variable"),
	         nb::arg("lb"), nb::arg("ub"))

	    .def("get_variable_start", &IpoptModel::get_variable_start)
	    .def("set_variable_start", &IpoptModel::set_variable_start)

	    .def("get_variable_name", &IpoptModel::get_variable_name)
	    .def("set_variable_name", &IpoptModel::set_variable_name)

	    .def("get_value", nb::overload_cast<const VariableIndex &>(&IpoptModel::get_variable_value))
	    .def("get_value",
	         nb::overload_cast<const ScalarAffineFunction &>(&IpoptModel::get_expression_value))
	    .def("get_value",
	         nb::overload_cast<const ScalarQuadraticFunction &>(&IpoptModel::get_expression_value))
	    .def("get_value", nb::overload_cast<const ExprBuilder &>(&IpoptModel::get_expression_value))

	    .def("pprint", &IpoptModel::pprint_variable)
	    .def("pprint",
	         nb::overload_cast<const ScalarAffineFunction &, int>(&IpoptModel::pprint_expression),
	         nb::arg("expr"), nb::arg("precision") = 4)
	    .def(
	        "pprint",
	        nb::overload_cast<const ScalarQuadraticFunction &, int>(&IpoptModel::pprint_expression),
	        nb::arg("expr"), nb::arg("precision") = 4)
	    .def("pprint", nb::overload_cast<const ExprBuilder &, int>(&IpoptModel::pprint_expression),
	         nb::arg("expr"), nb::arg("precision") = 4)

	    .def("add_parameter", &IpoptModel::add_parameter, nb::arg("value") = 0.0)
	    .def("set_parameter", &IpoptModel::set_parameter)

	    .def("get_obj_value", &IpoptModel::get_obj_value)
	    .def("get_constraint_primal", &IpoptModel::get_constraint_primal)
	    .def("get_constraint_dual", &IpoptModel::get_constraint_dual)

	    .def("add_linear_constraint",
	         nb::overload_cast<const ScalarAffineFunction &, ConstraintSense, CoeffT, const char *>(
	             &IpoptModel::add_linear_constraint),
	         nb::arg("expr"), nb::arg("sense"), nb::arg("rhs"), nb::arg("name") = "")
	    .def("add_linear_constraint",
	         nb::overload_cast<const ScalarAffineFunction &, ConstraintSense, CoeffT, CoeffT,
	                           const char *>(&IpoptModel::add_linear_constraint),
	         nb::arg("expr"), nb::arg("sense"), nb::arg("lb"), nb::arg("ub"), nb::arg("name") = "")
	    .def("add_linear_constraint",
	         nb::overload_cast<const VariableIndex &, ConstraintSense, CoeffT, const char *>(
	             &IpoptModel::add_linear_constraint),
	         nb::arg("expr"), nb::arg("sense"), nb::arg("rhs"), nb::arg("name") = "")
	    .def(
	        "add_linear_constraint",
	        nb::overload_cast<const VariableIndex &, ConstraintSense, CoeffT, CoeffT, const char *>(
	            &IpoptModel::add_linear_constraint),
	        nb::arg("expr"), nb::arg("sense"), nb::arg("lb"), nb::arg("ub"), nb::arg("name") = "")
	    .def("add_linear_constraint",
	         nb::overload_cast<const ExprBuilder &, ConstraintSense, CoeffT, const char *>(
	             &IpoptModel::add_linear_constraint),
	         nb::arg("expr"), nb::arg("sense"), nb::arg("rhs"), nb::arg("name") = "")
	    .def("add_linear_constraint",
	         nb::overload_cast<const ExprBuilder &, ConstraintSense, CoeffT, CoeffT, const char *>(
	             &IpoptModel::add_linear_constraint),
	         nb::arg("expr"), nb::arg("sense"), nb::arg("lb"), nb::arg("ub"), nb::arg("name") = "")

	    .def("add_quadratic_constraint",
	         nb::overload_cast<const ScalarQuadraticFunction &, ConstraintSense, CoeffT,
	                           const char *>(&IpoptModel::add_quadratic_constraint),
	         nb::arg("expr"), nb::arg("sense"), nb::arg("rhs"), nb::arg("name") = "")
	    .def("add_quadratic_constraint",
	         nb::overload_cast<const ScalarQuadraticFunction &, ConstraintSense, CoeffT, CoeffT,
	                           const char *>(&IpoptModel::add_quadratic_constraint),
	         nb::arg("expr"), nb::arg("sense"), nb::arg("lb"), nb::arg("ub"), nb::arg("name") = "")
	    .def("add_quadratic_constraint",
	         nb::overload_cast<const ExprBuilder &, ConstraintSense, CoeffT, const char *>(
	             &IpoptModel::add_quadratic_constraint),
	         nb::arg("expr"), nb::arg("sense"), nb::arg("rhs"), nb::arg("name") = "")
	    .def("add_quadratic_constraint",
	         nb::overload_cast<const ExprBuilder &, ConstraintSense, CoeffT, CoeffT, const char *>(
	             &IpoptModel::add_quadratic_constraint),
	         nb::arg("expr"), nb::arg("sense"), nb::arg("lb"), nb::arg("ub"), nb::arg("name") = "")

	    .def("add_objective", &IpoptModel::add_objective<ExprBuilder>)
	    .def("add_objective", &IpoptModel::add_objective<ScalarQuadraticFunction>)
	    .def("add_objective", &IpoptModel::add_objective<ScalarAffineFunction>)
	    .def("add_objective", &IpoptModel::add_objective<VariableIndex>)
	    .def("add_objective", &IpoptModel::add_objective<double>)

	    .def("set_objective", &IpoptModel::set_objective<ExprBuilder>, nb::arg("expr"),
	         nb::arg("sense") = ObjectiveSense::Minimize, nb::arg("clear_nl") = false)
	    .def("set_objective", &IpoptModel::set_objective<ScalarQuadraticFunction>, nb::arg("expr"),
	         nb::arg("sense") = ObjectiveSense::Minimize, nb::arg("clear_nl") = false)
	    .def("set_objective", &IpoptModel::set_objective<ScalarAffineFunction>, nb::arg("expr"),
	         nb::arg("sense") = ObjectiveSense::Minimize, nb::arg("clear_nl") = false)
	    .def("set_objective", &IpoptModel::set_objective<VariableIndex>, nb::arg("expr"),
	         nb::arg("sense") = ObjectiveSense::Minimize, nb::arg("clear_nl") = false)
	    .def("set_objective", &IpoptModel::set_objective<double>, nb::arg("expr"),
	         nb::arg("sense") = ObjectiveSense::Minimize, nb::arg("clear_nl") = false)

	    .def("_add_nl_objective", &IpoptModel::_add_nl_objective)

	    .def("clear_nl_objective", &IpoptModel::clear_nl_objective)

	    .def("_register_function", &IpoptModel::_register_function)
	    .def("_set_function_evaluator", &IpoptModel::_set_function_evaluator)
	    .def("_has_function_evaluator", &IpoptModel::_has_function_evaluator)

	    .def("_add_nl_constraint_bounds", &IpoptModel::_add_nl_constraint_bounds)
	    .def("_add_nl_constraint_eq", &IpoptModel::_add_nl_constraint_eq)

	    .def("_optimize", &IpoptModel::optimize, nb::call_guard<nb::gil_scoped_release>())

	    .def("load_current_solution", &IpoptModel::load_current_solution)

	    .def("set_raw_option_int", &IpoptModel::set_raw_option_int)
	    .def("set_raw_option_double", &IpoptModel::set_raw_option_double)
	    .def("set_raw_option_string", &IpoptModel::set_raw_option_string);
}
