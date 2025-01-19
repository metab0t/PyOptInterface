#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>

#include "pyoptinterface/highs_model.hpp"

namespace nb = nanobind;

extern void bind_highs_constants(nb::module_ &m);

NB_MODULE(highs_model_ext, m)
{
	m.import_("pyoptinterface._src.core_ext");

	m.def("is_library_loaded", &highs::is_library_loaded);
	m.def("load_library", &highs::load_library);

	bind_highs_constants(m);

	nb::enum_<HighsSolutionStatus>(m, "HighsSolutionStatus")
	    .value("OPTIMIZE_NOT_CALLED", HighsSolutionStatus::OPTIMIZE_NOT_CALLED)
	    .value("OPTIMIZE_OK", HighsSolutionStatus::OPTIMIZE_OK)
	    .value("OPTIMIZE_ERROR", HighsSolutionStatus::OPTIMIZE_ERROR);

	nb::class_<POIHighsSolution>(m, "HighsSolution")
	    .def_ro("status", &POIHighsSolution::status)
	    .def_ro("model_status", &POIHighsSolution::model_status)
	    .def_ro("primal_solution_status", &POIHighsSolution::primal_solution_status)
	    .def_ro("dual_solution_status", &POIHighsSolution::dual_solution_status)
	    .def_ro("has_primal_ray", &POIHighsSolution::has_primal_ray)
	    .def_ro("has_dual_ray", &POIHighsSolution::has_dual_ray);

	nb::class_<POIHighsModel>(m, "_RawModelBase");

#define BIND_F(f) .def(#f, &HighsModelMixin::f)
	nb::class_<HighsModelMixin, POIHighsModel>(m, "RawModel")
	    .def(nb::init<>())
	    .def_ro("m_n_variables", &HighsModelMixin::m_n_variables)
	    .def_ro("m_n_constraints", &HighsModelMixin::m_n_constraints)
	    // clang-format off
	    BIND_F(init)
	    BIND_F(write)
	    // clang-format on

	    .def_ro("solution", &HighsModelMixin::m_solution)

	    .def("add_variable", &HighsModelMixin::add_variable,
	         nb::arg("domain") = VariableDomain::Continuous, nb::arg("lb") = -kHighsInf,
	         nb::arg("ub") = kHighsInf, nb::arg("name") = "")
	    // clang-format off
	    BIND_F(delete_variable)
	    BIND_F(delete_variables)
	    BIND_F(is_variable_active)
	    // clang-format on
	    .def("set_variable_bounds", &HighsModelMixin::set_variable_bounds, nb::arg("variable"),
	         nb::arg("lb"), nb::arg("ub"))

	    .def("get_value",
	         nb::overload_cast<const VariableIndex &>(&HighsModelMixin::get_variable_value))
	    .def("get_value", nb::overload_cast<const ScalarAffineFunction &>(
	                          &HighsModelMixin::get_expression_value))
	    .def("get_value", nb::overload_cast<const ScalarQuadraticFunction &>(
	                          &HighsModelMixin::get_expression_value))
	    .def("get_value",
	         nb::overload_cast<const ExprBuilder &>(&HighsModelMixin::get_expression_value))

	    .def("pprint", &HighsModelMixin::pprint_variable)
	    .def("pprint",
	         nb::overload_cast<const ScalarAffineFunction &, int>(
	             &HighsModelMixin::pprint_expression),
	         nb::arg("expr"), nb::arg("precision") = 4)
	    .def("pprint",
	         nb::overload_cast<const ScalarQuadraticFunction &, int>(
	             &HighsModelMixin::pprint_expression),
	         nb::arg("expr"), nb::arg("precision") = 4)
	    .def("pprint",
	         nb::overload_cast<const ExprBuilder &, int>(&HighsModelMixin::pprint_expression),
	         nb::arg("expr"), nb::arg("precision") = 4)

	    .def("add_linear_constraint", &HighsModelMixin::add_linear_constraint, nb::arg("expr"),
	         nb::arg("sense"), nb::arg("rhs"), nb::arg("name") = "")
	    .def("add_linear_constraint", &HighsModelMixin::add_linear_constraint_from_var,
	         nb::arg("expr"), nb::arg("sense"), nb::arg("rhs"), nb::arg("name") = "")
	    .def("add_linear_constraint", &HighsModelMixin::add_linear_constraint_from_expr,
	         nb::arg("expr"), nb::arg("sense"), nb::arg("rhs"), nb::arg("name") = "")
	    .def("add_linear_constraint", &HighsModelMixin::add_linear_constraint_from_comparison,
	         nb::arg("con"), nb::arg("name") = "")
	    .def("delete_constraint", &HighsModelMixin::delete_constraint)
	    .def("is_constraint_active", &HighsModelMixin::is_constraint_active)

	    .def("set_objective",
	         nb::overload_cast<const ScalarQuadraticFunction &, ObjectiveSense>(
	             &HighsModelMixin::set_objective),
	         nb::arg("expr"), nb::arg("sense") = ObjectiveSense::Minimize)
	    .def("set_objective",
	         nb::overload_cast<const ScalarAffineFunction &, ObjectiveSense>(
	             &HighsModelMixin::set_objective),
	         nb::arg("expr"), nb::arg("sense") = ObjectiveSense::Minimize)
	    .def(
	        "set_objective",
	        nb::overload_cast<const ExprBuilder &, ObjectiveSense>(&HighsModelMixin::set_objective),
	        nb::arg("expr"), nb::arg("sense") = ObjectiveSense::Minimize)
	    .def("set_objective",
	         nb::overload_cast<const VariableIndex &, ObjectiveSense>(
	             &HighsModelMixin::set_objective_as_variable),
	         nb::arg("expr"), nb::arg("sense") = ObjectiveSense::Minimize)
	    .def("set_objective",
	         nb::overload_cast<CoeffT, ObjectiveSense>(&HighsModelMixin::set_objective_as_constant),
	         nb::arg("expr"), nb::arg("sense") = ObjectiveSense::Minimize)

	    .def("optimize", &HighsModelMixin::optimize, nb::call_guard<nb::gil_scoped_release>())

	    // clang-format off
	    BIND_F(version_string)
	    BIND_F(get_raw_model)

		BIND_F(getruntime)
		BIND_F(getnumrow)
		BIND_F(getnumcol)

	    BIND_F(raw_option_type)

	    BIND_F(set_raw_option_bool)
	    BIND_F(set_raw_option_int)
	    BIND_F(set_raw_option_double)
	    BIND_F(set_raw_option_string)
	    BIND_F(get_raw_option_bool)
	    BIND_F(get_raw_option_int)
	    BIND_F(get_raw_option_double)
	    BIND_F(get_raw_option_string)

		BIND_F(raw_info_type)
	    BIND_F(get_raw_info_int)
	    BIND_F(get_raw_info_int64)
	    BIND_F(get_raw_info_double)

	    BIND_F(set_variable_name)
	    BIND_F(get_variable_name)
	    BIND_F(set_variable_type)
	    BIND_F(get_variable_type)
	    BIND_F(set_variable_lower_bound)
	    BIND_F(set_variable_upper_bound)
		BIND_F(get_variable_lower_bound)
	    BIND_F(get_variable_upper_bound)

	    BIND_F(get_constraint_primal)
	    BIND_F(get_constraint_dual)
	    BIND_F(get_constraint_name)
	    BIND_F(set_constraint_name)

	    BIND_F(set_obj_sense)
	    BIND_F(get_obj_sense)
	    BIND_F(get_obj_value)

		BIND_F(set_primal_start)

		BIND_F(get_normalized_rhs)
		BIND_F(set_normalized_rhs)
		BIND_F(get_normalized_coefficient)
		BIND_F(set_normalized_coefficient)
		BIND_F(get_objective_coefficient)
		BIND_F(set_objective_coefficient)
	    // clang-format on
	    ;
}