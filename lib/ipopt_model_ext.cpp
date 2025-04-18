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

	    .def("set_objective",
	         nb::overload_cast<const VariableIndex &, ObjectiveSense>(
	             &IpoptModelMixin::set_objective),
	         nb::arg("expr"), nb::arg("sense") = ObjectiveSense::Minimize)
	    .def("set_objective",
	         nb::overload_cast<const ScalarAffineFunction &, ObjectiveSense>(
	             &IpoptModelMixin::set_objective),
	         nb::arg("expr"), nb::arg("sense") = ObjectiveSense::Minimize)
	    .def("set_objective",
	         nb::overload_cast<const ScalarQuadraticFunction &, ObjectiveSense>(
	             &IpoptModelMixin::set_objective),
	         nb::arg("expr"), nb::arg("sense") = ObjectiveSense::Minimize)
	    .def(
	        "set_objective",
	        nb::overload_cast<const ExprBuilder &, ObjectiveSense>(&IpoptModelMixin::set_objective),
	        nb::arg("expr"), nb::arg("sense") = ObjectiveSense::Minimize)

	    // New API
	    .def("_add_graph_index", &IpoptModelMixin::add_graph_index)
	    .def("_record_graph_hash", &IpoptModelMixin::record_graph_hash)
	    .def("_aggregate_graph_constraint_groups",
	         &IpoptModelMixin::aggregate_graph_constraint_groups)
	    .def("_get_graph_constraint_group_representative",
	         &IpoptModelMixin::get_graph_constraint_group_representative)
	    .def("_aggregate_graph_objective_groups",
	         &IpoptModelMixin::aggregate_graph_objective_groups)
	    .def("_get_graph_objective_group_representative",
	         &IpoptModelMixin::get_graph_objective_group_representative)
	    .def("_assign_constraint_group_autodiff_structure",
	         &IpoptModelMixin::assign_constraint_group_autodiff_structure)
	    .def("_assign_constraint_group_autodiff_evaluator",
	         &IpoptModelMixin::assign_constraint_group_autodiff_evaluator)
	    .def("_assign_objective_group_autodiff_structure",
	         &IpoptModelMixin::assign_objective_group_autodiff_structure)
	    .def("_assign_objective_group_autodiff_evaluator",
	         &IpoptModelMixin::assign_objective_group_autodiff_evaluator)

	    .def("_add_single_nl_constraint", &IpoptModelMixin::add_single_nl_constraint)

	    .def("_optimize", &IpoptModelMixin::optimize, nb::call_guard<nb::gil_scoped_release>())

	    .def("load_current_solution", &IpoptModelMixin::load_current_solution)

	    .def("set_raw_option_int", &IpoptModelMixin::set_raw_option_int)
	    .def("set_raw_option_double", &IpoptModelMixin::set_raw_option_double)
	    .def("set_raw_option_string", &IpoptModelMixin::set_raw_option_string);
}
