#include <nanobind/nanobind.h>
#include <nanobind/stl/vector.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/tuple.h>

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

	nb::class_<IpoptModel>(m, "_RawModelBase");

	nb::class_<IpoptModelMixin>(m, "RawModel")
	    .def(nb::init<>())
	    .def("close", &IpoptModelMixin::close)
	    .def_ro("m_status", &IpoptModelMixin::m_status)
	    .def("add_variable", &IpoptModelMixin::add_variable, nb::arg("lb") = -INFINITY,
	         nb::arg("ub") = INFINITY, nb::arg("start") = 0.0, nb::arg("name") = "")
	    .def("get_variable_lb", &IpoptModelMixin::get_variable_lb)
	    .def("get_variable_ub", &IpoptModelMixin::get_variable_ub)
	    .def("set_variable_lb", &IpoptModelMixin::set_variable_lb)
	    .def("set_variable_ub", &IpoptModelMixin::set_variable_ub)
	    .def("set_variable_bounds", &IpoptModelMixin::set_variable_bounds, nb::arg("variable"),
	         nb::arg("lb"), nb::arg("ub"))

	    .def("get_variable_start", &IpoptModelMixin::get_variable_start)
	    .def("set_variable_start", &IpoptModelMixin::set_variable_start)

	    .def("get_variable_name", &IpoptModelMixin::get_variable_name)
	    .def("set_variable_name", &IpoptModelMixin::set_variable_name)

	    .def("get_value",
	         nb::overload_cast<const VariableIndex &>(&IpoptModelMixin::get_variable_value))
	    .def("get_value", nb::overload_cast<const ScalarAffineFunction &>(
	                          &IpoptModelMixin::get_expression_value))
	    .def("get_value", nb::overload_cast<const ScalarQuadraticFunction &>(
	                          &IpoptModelMixin::get_expression_value))
	    .def("get_value",
	         nb::overload_cast<const ExprBuilder &>(&IpoptModelMixin::get_expression_value))

	    .def("pprint", &IpoptModelMixin::pprint_variable)
	    .def("pprint",
	         nb::overload_cast<const ScalarAffineFunction &, int>(
	             &IpoptModelMixin::pprint_expression),
	         nb::arg("expr"), nb::arg("precision") = 4)
	    .def("pprint",
	         nb::overload_cast<const ScalarQuadraticFunction &, int>(
	             &IpoptModelMixin::pprint_expression),
	         nb::arg("expr"), nb::arg("precision") = 4)
	    .def("pprint",
	         nb::overload_cast<const ExprBuilder &, int>(&IpoptModelMixin::pprint_expression),
	         nb::arg("expr"), nb::arg("precision") = 4)

	    .def("add_parameter", &IpoptModelMixin::add_parameter, nb::arg("value") = 0.0)
	    .def("set_parameter", &IpoptModelMixin::set_parameter)

	    .def("get_obj_value", &IpoptModelMixin::get_obj_value)
	    .def("get_constraint_primal", &IpoptModelMixin::get_constraint_primal)
	    .def("get_constraint_dual", &IpoptModelMixin::get_constraint_dual)

	    .def("_add_linear_constraint",
	         nb::overload_cast<const ScalarAffineFunction &, ConstraintSense, CoeffT, const char *>(
	             &IpoptModelMixin::add_linear_constraint),
	         nb::arg("expr"), nb::arg("sense"), nb::arg("rhs"), nb::arg("name") = "")
	    .def("_add_linear_constraint",
	         nb::overload_cast<const ScalarAffineFunction &, ConstraintSense,
	                           const std::tuple<double, double> &, const char *>(
	             &IpoptModelMixin::add_linear_constraint),
	         nb::arg("expr"), nb::arg("sense"), nb::arg("interval"), nb::arg("name") = "")
	    .def("_add_linear_constraint",
	         nb::overload_cast<const VariableIndex &, ConstraintSense, CoeffT, const char *>(
	             &IpoptModelMixin::add_linear_constraint),
	         nb::arg("expr"), nb::arg("sense"), nb::arg("rhs"), nb::arg("name") = "")
	    .def("_add_linear_constraint",
	         nb::overload_cast<const VariableIndex &, ConstraintSense,
	                           const std::tuple<double, double> &, const char *>(
	             &IpoptModelMixin::add_linear_constraint),
	         nb::arg("expr"), nb::arg("sense"), nb::arg("interval"), nb::arg("name") = "")
	    .def("_add_linear_constraint",
	         nb::overload_cast<const ExprBuilder &, ConstraintSense, CoeffT, const char *>(
	             &IpoptModelMixin::add_linear_constraint),
	         nb::arg("expr"), nb::arg("sense"), nb::arg("rhs"), nb::arg("name") = "")
	    .def("_add_linear_constraint",
	         nb::overload_cast<const ExprBuilder &, ConstraintSense,
	                           const std::tuple<double, double> &, const char *>(
	             &IpoptModelMixin::add_linear_constraint),
	         nb::arg("expr"), nb::arg("sense"), nb::arg("interval"), nb::arg("name") = "")

	    .def("_add_quadratic_constraint",
	         nb::overload_cast<const ScalarQuadraticFunction &, ConstraintSense, CoeffT,
	                           const char *>(&IpoptModelMixin::add_quadratic_constraint),
	         nb::arg("expr"), nb::arg("sense"), nb::arg("rhs"), nb::arg("name") = "")
	    .def("_add_quadratic_constraint",
	         nb::overload_cast<const ScalarQuadraticFunction &, ConstraintSense,
	                           const std::tuple<double, double> &, const char *>(
	             &IpoptModelMixin::add_quadratic_constraint),
	         nb::arg("expr"), nb::arg("sense"), nb::arg("interval"), nb::arg("name") = "")
	    .def("_add_quadratic_constraint",
	         nb::overload_cast<const ExprBuilder &, ConstraintSense, CoeffT, const char *>(
	             &IpoptModelMixin::add_quadratic_constraint),
	         nb::arg("expr"), nb::arg("sense"), nb::arg("rhs"), nb::arg("name") = "")
	    .def("_add_quadratic_constraint",
	         nb::overload_cast<const ExprBuilder &, ConstraintSense,
	                           const std::tuple<double, double> &, const char *>(
	             &IpoptModelMixin::add_quadratic_constraint),
	         nb::arg("expr"), nb::arg("sense"), nb::arg("interval"), nb::arg("name") = "")

	    .def("add_objective", &IpoptModelMixin::add_objective<ExprBuilder>)
	    .def("add_objective", &IpoptModelMixin::add_objective<ScalarQuadraticFunction>)
	    .def("add_objective", &IpoptModelMixin::add_objective<ScalarAffineFunction>)
	    .def("add_objective", &IpoptModelMixin::add_objective<VariableIndex>)
	    .def("add_objective", &IpoptModelMixin::add_objective<double>)

	    .def("set_objective", &IpoptModelMixin::set_objective<ExprBuilder>, nb::arg("expr"),
	         nb::arg("sense") = ObjectiveSense::Minimize, nb::arg("clear_nl") = false)
	    .def("set_objective", &IpoptModelMixin::set_objective<ScalarQuadraticFunction>,
	         nb::arg("expr"), nb::arg("sense") = ObjectiveSense::Minimize,
	         nb::arg("clear_nl") = false)
	    .def("set_objective", &IpoptModelMixin::set_objective<ScalarAffineFunction>,
	         nb::arg("expr"), nb::arg("sense") = ObjectiveSense::Minimize,
	         nb::arg("clear_nl") = false)
	    .def("set_objective", &IpoptModelMixin::set_objective<VariableIndex>, nb::arg("expr"),
	         nb::arg("sense") = ObjectiveSense::Minimize, nb::arg("clear_nl") = false)
	    .def("set_objective", &IpoptModelMixin::set_objective<double>, nb::arg("expr"),
	         nb::arg("sense") = ObjectiveSense::Minimize, nb::arg("clear_nl") = false)

	    .def("_add_fn_objective", &IpoptModelMixin::_add_fn_objective)

	    .def("clear_nl_objective", &IpoptModelMixin::clear_nl_objective)

	    .def("_register_function", &IpoptModelMixin::_register_function)
	    .def("_set_function_evaluator", &IpoptModelMixin::_set_function_evaluator)
	    .def("_has_function_evaluator", &IpoptModelMixin::_has_function_evaluator)

	    .def("_add_fn_constraint_bounds", &IpoptModelMixin::_add_fn_constraint_bounds)
	    .def("_add_fn_constraint_eq", &IpoptModelMixin::_add_fn_constraint_eq)

	    .def("_optimize", &IpoptModelMixin::optimize, nb::call_guard<nb::gil_scoped_release>())

	    .def("load_current_solution", &IpoptModelMixin::load_current_solution)

	    .def("set_raw_option_int", &IpoptModelMixin::set_raw_option_int)
	    .def("set_raw_option_double", &IpoptModelMixin::set_raw_option_double)
	    .def("set_raw_option_string", &IpoptModelMixin::set_raw_option_string);
}
