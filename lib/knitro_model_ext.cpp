#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>
#include <nanobind/stl/tuple.h>

#include "pyoptinterface/knitro_model.hpp"

namespace nb = nanobind;

extern void bind_knitro_constants(nb::module_ &m);

NB_MODULE(knitro_model_ext, m)
{
	m.import_("pyoptinterface._src.core_ext");

	m.def("is_library_loaded", &knitro::is_library_loaded);
	m.def("load_library", &knitro::load_library);

	bind_knitro_constants(m);

#define BIND_F(f) .def(#f, &KNITROModel::f)
	nb::class_<KNITROModel>(m, "RawModel")
	    .def(nb::init<>())

	    // clang-format off
		BIND_F(init)
		BIND_F(close)
		BIND_F(get_infinity)
		BIND_F(get_number_iterations)
		BIND_F(get_mip_node_count)
		BIND_F(get_obj_bound)
		BIND_F(get_mip_relative_gap)
		BIND_F(get_solve_time)
		BIND_F(get_solver_name)
		BIND_F(get_release)
	    // clang-format on

	    .def_ro("n_vars", &KNITROModel::n_vars)
	    .def_ro("n_cons", &KNITROModel::n_cons)
	    .def_ro("n_lincons", &KNITROModel::n_lincons)
	    .def_ro("n_quadcons", &KNITROModel::n_quadcons)
	    .def_ro("n_coniccons", &KNITROModel::n_coniccons)
	    .def_ro("n_nlcons", &KNITROModel::n_nlcons)

	    .def("add_variable", &KNITROModel::add_variable,
	         nb::arg("domain") = VariableDomain::Continuous, nb::arg("lb") = -KN_INFINITY,
	         nb::arg("ub") = KN_INFINITY, nb::arg("name") = "")

	    // clang-format off
		BIND_F(get_variable_lb)
		BIND_F(get_variable_ub)
		BIND_F(set_variable_lb)
		BIND_F(set_variable_ub)
		BIND_F(set_variable_bounds)
	    BIND_F(set_variable_start)
		BIND_F(get_variable_name)
		BIND_F(set_variable_name)
		BIND_F(set_variable_domain)
		BIND_F(get_variable_rc)
		BIND_F(delete_variable)
	    // clang-format on

	    .def("get_value", &KNITROModel::get_variable_value)
	    .def("get_value",
	         nb::overload_cast<const ScalarAffineFunction &>(&KNITROModel::get_expression_value))
	    .def("get_value",
	         nb::overload_cast<const ScalarQuadraticFunction &>(&KNITROModel::get_expression_value))
	    .def("get_value",
	         nb::overload_cast<const ExprBuilder &>(&KNITROModel::get_expression_value))

	    .def("pprint", &KNITROModel::pprint_variable)
	    .def("pprint",
	         nb::overload_cast<const ScalarAffineFunction &, int>(&KNITROModel::pprint_expression),
	         nb::arg("expr"), nb::arg("precision") = 4)
	    .def("pprint",
	         nb::overload_cast<const ScalarQuadraticFunction &, int>(
	             &KNITROModel::pprint_expression),
	         nb::arg("expr"), nb::arg("precision") = 4)
	    .def("pprint", nb::overload_cast<const ExprBuilder &, int>(&KNITROModel::pprint_expression),
	         nb::arg("expr"), nb::arg("precision") = 4)

	    // clang-format off
		BIND_F(set_constraint_name)
		BIND_F(get_constraint_name)
		BIND_F(get_constraint_primal)
		BIND_F(get_constraint_dual)
		BIND_F(set_normalized_rhs)
		BIND_F(get_normalized_rhs)
		BIND_F(set_normalized_coefficient)
	    // clang-format on

	    .def("_add_linear_constraint",
	         nb::overload_cast<const ScalarAffineFunction &, ConstraintSense, CoeffT, const char *>(
	             &KNITROModel::add_linear_constraint),
	         nb::arg("expr"), nb::arg("sense"), nb::arg("rhs"), nb::arg("name") = "")
	    .def("_add_linear_constraint",
	         nb::overload_cast<const ScalarAffineFunction &, const std::tuple<double, double> &,
	                           const char *>(&KNITROModel::add_linear_constraint),
	         nb::arg("expr"), nb::arg("interval"), nb::arg("name") = "")
	    .def("_add_linear_constraint", &KNITROModel::add_linear_constraint_from_var,
	         nb::arg("expr"), nb::arg("sense"), nb::arg("rhs"), nb::arg("name") = "")
	    .def("_add_linear_constraint", &KNITROModel::add_linear_interval_constraint_from_var,
	         nb::arg("expr"), nb::arg("interval"), nb::arg("name") = "")
	    .def("_add_linear_constraint", &KNITROModel::add_linear_constraint_from_expr,
	         nb::arg("expr"), nb::arg("sense"), nb::arg("rhs"), nb::arg("name") = "")
	    .def("_add_linear_constraint", &KNITROModel::add_linear_interval_constraint_from_expr,
	         nb::arg("expr"), nb::arg("interval"), nb::arg("name") = "")

	    .def("_add_quadratic_constraint",
	         nb::overload_cast<const ScalarQuadraticFunction &, ConstraintSense, CoeffT,
	                           const char *>(&KNITROModel::add_quadratic_constraint),
	         nb::arg("expr"), nb::arg("sense"), nb::arg("rhs"), nb::arg("name") = "")
	    .def("_add_quadratic_constraint",
	         nb::overload_cast<const ScalarQuadraticFunction &, const std::tuple<double, double> &,
	                           const char *>(&KNITROModel::add_quadratic_constraint),
	         nb::arg("expr"), nb::arg("interval"), nb::arg("name") = "")
	    .def("_add_quadratic_constraint", &KNITROModel::add_quadratic_constraint_from_expr,
	         nb::arg("expr"), nb::arg("sense"), nb::arg("rhs"), nb::arg("name") = "")
	    .def("_add_quadratic_constraint", &KNITROModel::add_quadratic_interval_constraint_from_expr,
	         nb::arg("expr"), nb::arg("interval"), nb::arg("name") = "")

	    .def("add_second_order_cone_constraint", &KNITROModel::add_second_order_cone_constraint,
	         nb::arg("variables"), nb::arg("name") = "", nb::arg("rotated") = false)

	    .def("_add_single_nl_constraint", &KNITROModel::add_single_nl_constraint, nb::arg("graph"),
	         nb::arg("result"), nb::arg("interval"), nb::arg("name") = "")
	    .def("_add_single_nl_constraint", &KNITROModel::add_single_nl_constraint_sense_rhs,
	         nb::arg("graph"), nb::arg("result"), nb::arg("sense"), nb::arg("rhs"),
	         nb::arg("name") = "")
	    .def("_add_single_nl_constraint", &KNITROModel::add_single_nl_constraint_from_comparison,
	         nb::arg("graph"), nb::arg("expr"), nb::arg("name") = "")

	    // clang-format off
	    BIND_F(delete_constraint)
	    // clang-format on

	    .def("set_objective",
	         nb::overload_cast<const ScalarAffineFunction &, ObjectiveSense>(
	             &KNITROModel::set_objective),
	         nb::arg("expr"), nb::arg("sense") = ObjectiveSense::Minimize)
	    .def("set_objective",
	         nb::overload_cast<const ScalarQuadraticFunction &, ObjectiveSense>(
	             &KNITROModel::set_objective),
	         nb::arg("expr"), nb::arg("sense") = ObjectiveSense::Minimize)
	    .def("set_objective",
	         nb::overload_cast<const ExprBuilder &, ObjectiveSense>(&KNITROModel::set_objective),
	         nb::arg("expr"), nb::arg("sense") = ObjectiveSense::Minimize)
	    .def("set_objective",
	         nb::overload_cast<const VariableIndex &, ObjectiveSense>(
	             &KNITROModel::set_objective_as_variable),
	         nb::arg("expr"), nb::arg("sense") = ObjectiveSense::Minimize)
	    .def("set_objective",
	         nb::overload_cast<CoeffT, ObjectiveSense>(&KNITROModel::set_objective_as_constant),
	         nb::arg("expr"), nb::arg("sense") = ObjectiveSense::Minimize)
	    .def("_add_single_nl_objective", &KNITROModel::add_single_nl_objective, nb::arg("graph"),
	         nb::arg("result"))

	    // clang-format off
		BIND_F(get_obj_value)
		BIND_F(set_obj_sense)
		BIND_F(get_obj_sense)
	    // clang-format on

	    .def("optimize", &KNITROModel::optimize, nb::call_guard<nb::gil_scoped_release>())

	    .def(
	        "set_raw_parameter",
	        [](KNITROModel &m, const std::string &name, int value) {
		        m.set_raw_parameter<int>(name, value);
	        },
	        nb::arg("name"), nb::arg("value"))
	    .def(
	        "set_raw_parameter",
	        [](KNITROModel &m, const std::string &name, double value) {
		        m.set_raw_parameter<double>(name, value);
	        },
	        nb::arg("name"), nb::arg("value"))
	    .def(
	        "set_raw_parameter",
	        [](KNITROModel &m, const std::string &name, const std::string &value) {
		        m.set_raw_parameter<std::string>(name, value);
	        },
	        nb::arg("name"), nb::arg("value"))
	    .def(
	        "set_raw_parameter",
	        [](KNITROModel &m, int param_id, int value) {
		        m.set_raw_parameter<int>(param_id, value);
	        },
	        nb::arg("param_id"), nb::arg("value"))
	    .def(
	        "set_raw_parameter",
	        [](KNITROModel &m, int param_id, double value) {
		        m.set_raw_parameter<double>(param_id, value);
	        },
	        nb::arg("param_id"), nb::arg("value"))
	    .def(
	        "set_raw_parameter",
	        [](KNITROModel &m, int param_id, const std::string &value) {
		        m.set_raw_parameter<std::string>(param_id, value);
	        },
	        nb::arg("param_id"), nb::arg("value"))

	    .def(
	        "get_raw_parameter",
	        [](KNITROModel &m, const std::string &name) -> int {
		        return m.get_raw_parameter<int>(name);
	        },
	        nb::arg("name"))
	    .def(
	        "get_raw_parameter",
	        [](KNITROModel &m, const std::string &name) -> double {
		        return m.get_raw_parameter<double>(name);
	        },
	        nb::arg("name"))
	    .def(
	        "get_raw_parameter",
	        [](KNITROModel &m, int param_id) -> int { return m.get_raw_parameter<int>(param_id); },
	        nb::arg("param_id"))
	    .def(
	        "get_raw_parameter",
	        [](KNITROModel &m, int param_id) -> double {
		        return m.get_raw_parameter<double>(param_id);
	        },
	        nb::arg("param_id"))

	    .def_rw("m_is_dirty", &KNITROModel::m_is_dirty)
	    .def_ro("m_solve_status", &KNITROModel::m_solve_status);

#undef BIND_F
}
