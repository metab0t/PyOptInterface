#include <nanobind/nanobind.h>
#include <nanobind/stl/vector.h>
#include <nanobind/stl/string.h>

namespace nb = nanobind;

#include "pyoptinterface/ipopt_model.hpp"

NB_MODULE(ipopt_model_ext, m)
{
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
	    .def_ro("m_function_model", &IpoptModel::m_function_model)
	    .def_ro("m_status", &IpoptModel::m_status)
	    .def("add_variable", &IpoptModel::add_variable, nb::arg("lb") = -INFINITY,
	         nb::arg("ub") = INFINITY, nb::arg("start") = 0.0)
	    .def("change_variable_lb", &IpoptModel::change_variable_lb)
	    .def("change_variable_ub", &IpoptModel::change_variable_ub)
	    .def("change_variable_bounds", &IpoptModel::change_variable_bounds, nb::arg("variable"),
	         nb::arg("lb"), nb::arg("ub"))
	    .def("get_variable_value", &IpoptModel::get_variable_value)
	    .def("add_parameter", &IpoptModel::add_parameter, nb::arg("value") = 0.0)
	    .def("set_parameter", &IpoptModel::set_parameter)

	    .def("add_linear_constraint",
	         nb::overload_cast<const ScalarAffineFunction &, ConstraintSense, CoeffT>(
	             &IpoptModel::add_linear_constraint),
	         nb::arg("expr"), nb::arg("sense"), nb::arg("rhs"))
	    .def("add_linear_constraint",
	         nb::overload_cast<const ScalarAffineFunction &, ConstraintSense, CoeffT, CoeffT>(
	             &IpoptModel::add_linear_constraint),
	         nb::arg("expr"), nb::arg("sense"), nb::arg("lb"), nb::arg("ub"))
	    .def("add_linear_constraint",
	         nb::overload_cast<const VariableIndex &, ConstraintSense, CoeffT>(
	             &IpoptModel::add_linear_constraint),
	         nb::arg("expr"), nb::arg("sense"), nb::arg("rhs"))
	    .def("add_linear_constraint",
	         nb::overload_cast<const VariableIndex &, ConstraintSense, CoeffT, CoeffT>(
	             &IpoptModel::add_linear_constraint),
	         nb::arg("expr"), nb::arg("sense"), nb::arg("lb"), nb::arg("ub"))
	    .def("add_linear_constraint",
	         nb::overload_cast<const ExprBuilder &, ConstraintSense, CoeffT>(
	             &IpoptModel::add_linear_constraint),
	         nb::arg("expr"), nb::arg("sense"), nb::arg("rhs"))
	    .def("add_linear_constraint",
	         nb::overload_cast<const ExprBuilder &, ConstraintSense, CoeffT, CoeffT>(
	             &IpoptModel::add_linear_constraint),
	         nb::arg("expr"), nb::arg("sense"), nb::arg("lb"), nb::arg("ub"))

	    .def("add_quadratic_constraint",
	         nb::overload_cast<const ScalarQuadraticFunction &, ConstraintSense, CoeffT>(
	             &IpoptModel::add_quadratic_constraint),
	         nb::arg("expr"), nb::arg("sense"), nb::arg("rhs"))
	    .def("add_linear_constraint",
	         nb::overload_cast<const ScalarQuadraticFunction &, ConstraintSense, CoeffT, CoeffT>(
	             &IpoptModel::add_quadratic_constraint),
	         nb::arg("expr"), nb::arg("sense"), nb::arg("lb"), nb::arg("ub"))
	    .def("add_quadratic_constraint",
	         nb::overload_cast<const ExprBuilder &, ConstraintSense, CoeffT>(
	             &IpoptModel::add_quadratic_constraint),
	         nb::arg("expr"), nb::arg("sense"), nb::arg("rhs"))
	    .def("add_quadratic_constraint",
	         nb::overload_cast<const ExprBuilder &, ConstraintSense, CoeffT, CoeffT>(
	             &IpoptModel::add_quadratic_constraint),
	         nb::arg("expr"), nb::arg("sense"), nb::arg("lb"), nb::arg("ub"))

	    .def("add_objective",
	         nb::overload_cast<const ExprBuilder &>(&IpoptModel::add_objective<ExprBuilder>))
	    .def("add_objective", nb::overload_cast<const ScalarQuadraticFunction &>(
	                              &IpoptModel::add_objective<ScalarQuadraticFunction>))
	    .def("add_objective", nb::overload_cast<const ScalarAffineFunction &>(
	                              &IpoptModel::add_objective<ScalarAffineFunction>))
	    .def("add_objective",
	         nb::overload_cast<const VariableIndex &>(&IpoptModel::add_objective<VariableIndex>))
	    .def("add_objective", nb::overload_cast<const double &>(&IpoptModel::add_objective<double>))
	    .def("add_objective",
	         nb::overload_cast<const FunctionIndex &, const std::vector<VariableIndex> &,
	                           const std::vector<ParameterIndex> &>(&IpoptModel::add_nl_objective))

	    .def("register_function", &IpoptModel::register_function)

	    .def("add_empty_nl_constraint",
	         nb::overload_cast<int, ConstraintSense, const std::vector<double> &>(
	             &IpoptModel::add_empty_nl_constraint),
	         nb::arg("dim"), nb::arg("sense"), nb::arg("rhs"))
	    .def("add_empty_nl_constraint",
	         nb::overload_cast<int, ConstraintSense, const std::vector<double> &,
	                           const std::vector<double> &>(&IpoptModel::add_empty_nl_constraint),
	         nb::arg("dim"), nb::arg("sense"), nb::arg("lb"), nb::arg("ub"))

	    .def("add_nl_constraint",
	         nb::overload_cast<const FunctionIndex &, const std::vector<VariableIndex> &,
	                           ConstraintSense, const std::vector<double> &>(
	             &IpoptModel::add_nl_constraint),
	         nb::arg("f"), nb::arg("x"), nb::arg("sense"), nb::arg("rhs"))
	    .def("add_nl_constraint",
	         nb::overload_cast<const FunctionIndex &, const std::vector<VariableIndex> &,
	                           const std::vector<ParameterIndex> &, ConstraintSense,
	                           const std::vector<double> &>(&IpoptModel::add_nl_constraint),
	         nb::arg("f"), nb::arg("x"), nb::arg("p"), nb::arg("sense"), nb::arg("rhs"))
	    .def("add_nl_constraint",
	         nb::overload_cast<const FunctionIndex &, const std::vector<VariableIndex> &,
	                           ConstraintSense, const std::vector<double> &,
	                           const std::vector<double> &>(&IpoptModel::add_nl_constraint),
	         nb::arg("f"), nb::arg("x"), nb::arg("sense"), nb::arg("lb"), nb::arg("ub"))
	    .def("add_nl_constraint",
	         nb::overload_cast<const FunctionIndex &, const std::vector<VariableIndex> &,
	                           const std::vector<ParameterIndex> &, ConstraintSense,
	                           const std::vector<double> &, const std::vector<double> &>(
	             &IpoptModel::add_nl_constraint),
	         nb::arg("f"), nb::arg("x"), nb::arg("p"), nb::arg("sense"), nb::arg("lb"),
	         nb::arg("ub"))

	    .def("add_nl_expression",
	         nb::overload_cast<const NLConstraintIndex &, const FunctionIndex &,
	                           const std::vector<VariableIndex> &,
	                           const std::vector<ParameterIndex> &>(&IpoptModel::add_nl_expression),
	         nb::arg("constraint"), nb::arg("f"), nb::arg("x"), nb::arg("p"))
	    .def("add_nl_expression",
	         nb::overload_cast<const NLConstraintIndex &, const FunctionIndex &,
	                           const std::vector<VariableIndex> &>(&IpoptModel::add_nl_expression),
	         nb::arg("constraint"), nb::arg("f"), nb::arg("x"))

	    .def("optimize", &IpoptModel::optimize)
	    .def("set_option_int", &IpoptModel::set_option_int)
	    .def("set_option_num", &IpoptModel::set_option_num)
	    .def("set_option_str", &IpoptModel::set_option_str);
}
