#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>
#include <nanobind/stl/tuple.h>
#include <nanobind/stl/function.h>

#include "pyoptinterface/gurobi_model.hpp"

namespace nb = nanobind;

extern void bind_gurobi_constants(nb::module_ &m);

NB_MODULE(gurobi_model_ext, m)
{
	m.import_("pyoptinterface._src.core_ext");

	m.def("is_library_loaded", &gurobi::is_library_loaded);
	m.def("load_library", &gurobi::load_library);

	bind_gurobi_constants(m);

#define BIND_F(f) .def(#f, &GurobiEnv::f)
	nb::class_<GurobiEnv>(m, "RawEnv")
	    .def(nb::init<bool>(), nb::arg("empty") = false)
	    // clang-format off
		BIND_F(start)
		BIND_F(close)
		BIND_F(raw_parameter_type)
		BIND_F(set_raw_parameter_int)
		BIND_F(set_raw_parameter_double)
		BIND_F(set_raw_parameter_string)
	    // clang-format on
	    ;

#undef BIND_F
#define BIND_F(f) .def(#f, &GurobiModel::f)
	nb::class_<GurobiModel>(m, "RawModel")
	    .def(nb::init<>())
	    .def(nb::init<const GurobiEnv &>())
	    // clang-format off
		BIND_F(init)
		BIND_F(close)
		BIND_F(write)
	    // clang-format on
	    .def("_reset", &GurobiModel::_reset, nb::arg("clearall") = 0)

	    .def("add_variable", &GurobiModel::add_variable,
	         nb::arg("domain") = VariableDomain::Continuous, nb::arg("lb") = -GRB_INFINITY,
	         nb::arg("ub") = GRB_INFINITY, nb::arg("name") = "")
	    // clang-format off
		BIND_F(delete_variable)
		BIND_F(delete_variables)
		BIND_F(is_variable_active)
	    // clang-format on
	    .def("set_variable_bounds", &GurobiModel::set_variable_bounds, nb::arg("variable"),
	         nb::arg("lb"), nb::arg("ub"))

	    .def("get_value", &GurobiModel::get_variable_value)
	    .def("get_value",
	         nb::overload_cast<const ScalarAffineFunction &>(&GurobiModel::get_expression_value))
	    .def("get_value",
	         nb::overload_cast<const ScalarQuadraticFunction &>(&GurobiModel::get_expression_value))
	    .def("get_value",
	         nb::overload_cast<const ExprBuilder &>(&GurobiModel::get_expression_value))

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

	    // clang-format off
		BIND_F(set_variable_name)
		BIND_F(set_constraint_name)
	    // clang-format on

	    .def("_add_linear_constraint", &GurobiModel::add_linear_constraint, nb::arg("expr"),
	         nb::arg("sense"), nb::arg("rhs"), nb::arg("name") = "")
	    .def("_add_linear_constraint", &GurobiModel::add_linear_constraint_from_var,
	         nb::arg("expr"), nb::arg("sense"), nb::arg("rhs"), nb::arg("name") = "")
	    .def("_add_linear_constraint", &GurobiModel::add_linear_constraint_from_expr,
	         nb::arg("expr"), nb::arg("sense"), nb::arg("rhs"), nb::arg("name") = "")
	    .def("_add_quadratic_constraint", &GurobiModel::add_quadratic_constraint, nb::arg("expr"),
	         nb::arg("sense"), nb::arg("rhs"), nb::arg("name") = "")
	    .def("_add_quadratic_constraint", &GurobiModel::add_quadratic_constraint_from_expr,
	         nb::arg("expr"), nb::arg("sense"), nb::arg("rhs"), nb::arg("name") = "")
	    .def("add_sos_constraint", nb::overload_cast<const Vector<VariableIndex> &, SOSType>(
	                                   &GurobiModel::add_sos_constraint))
	    .def("add_sos_constraint",
	         nb::overload_cast<const Vector<VariableIndex> &, SOSType, const Vector<CoeffT> &>(
	             &GurobiModel::add_sos_constraint))

	    .def("_add_single_nl_constraint", &GurobiModel::add_single_nl_constraint, nb::arg("graph"),
	         nb::arg("result"), nb::arg("interval"), nb::arg("name") = "")
	    .def("_add_single_nl_constraint", &GurobiModel::add_single_nl_constraint_sense_rhs,
	         nb::arg("graph"), nb::arg("result"), nb::arg("sense"), nb::arg("rhs"),
	         nb::arg("name") = "")
	    .def("_add_single_nl_constraint", &GurobiModel::add_single_nl_constraint_from_comparison,
	         nb::arg("graph"), nb::arg("expr"), nb::arg("name") = "")

	    // clang-format off
		BIND_F(delete_constraint)
		BIND_F(is_constraint_active)
	    // clang-format on

	    .def("set_objective",
	         nb::overload_cast<const ScalarQuadraticFunction &, ObjectiveSense>(
	             &GurobiModel::set_objective),
	         nb::arg("expr"), nb::arg("sense") = ObjectiveSense::Minimize)
	    .def("set_objective",
	         nb::overload_cast<const ScalarAffineFunction &, ObjectiveSense>(
	             &GurobiModel::set_objective),
	         nb::arg("expr"), nb::arg("sense") = ObjectiveSense::Minimize)
	    .def("set_objective",
	         nb::overload_cast<const ExprBuilder &, ObjectiveSense>(&GurobiModel::set_objective),
	         nb::arg("expr"), nb::arg("sense") = ObjectiveSense::Minimize)
	    .def("set_objective",
	         nb::overload_cast<const VariableIndex &, ObjectiveSense>(
	             &GurobiModel::set_objective_as_variable),
	         nb::arg("expr"), nb::arg("sense") = ObjectiveSense::Minimize)
	    .def("set_objective",
	         nb::overload_cast<CoeffT, ObjectiveSense>(&GurobiModel::set_objective_as_constant),
	         nb::arg("expr"), nb::arg("sense") = ObjectiveSense::Minimize)

	    .def("_add_single_nl_objective", &GurobiModel::add_single_nl_objective)

	    .def("cb_add_lazy_constraint",
	         nb::overload_cast<const ScalarAffineFunction &, ConstraintSense, CoeffT>(
	             &GurobiModel::cb_add_lazy_constraint),
	         nb::arg("expr"), nb::arg("sense"), nb::arg("rhs"))
	    .def("cb_add_lazy_constraint",
	         nb::overload_cast<const ExprBuilder &, ConstraintSense, CoeffT>(
	             &GurobiModel::cb_add_lazy_constraint),
	         nb::arg("expr"), nb::arg("sense"), nb::arg("rhs"))
	    .def("cb_add_user_cut",
	         nb::overload_cast<const ScalarAffineFunction &, ConstraintSense, CoeffT>(
	             &GurobiModel::cb_add_user_cut),
	         nb::arg("expr"), nb::arg("sense"), nb::arg("rhs"))
	    .def("cb_add_user_cut",
	         nb::overload_cast<const ExprBuilder &, ConstraintSense, CoeffT>(
	             &GurobiModel::cb_add_user_cut),
	         nb::arg("expr"), nb::arg("sense"), nb::arg("rhs"))

	    .def("optimize", &GurobiModel::optimize, nb::call_guard<nb::gil_scoped_release>())

	    // clang-format off
		BIND_F(update)
		BIND_F(version_string)
		BIND_F(get_raw_model)

		BIND_F(set_callback)
		BIND_F(cb_get_info_int)
		BIND_F(cb_get_info_double)
		BIND_F(cb_get_solution)
		BIND_F(cb_get_relaxation)
		BIND_F(cb_set_solution)
		BIND_F(cb_submit_solution)
		BIND_F(cb_exit)

		BIND_F(raw_parameter_type)
		BIND_F(set_raw_parameter_int)
		BIND_F(set_raw_parameter_double)
		BIND_F(set_raw_parameter_string)
		BIND_F(get_raw_parameter_int)
		BIND_F(get_raw_parameter_double)
		BIND_F(get_raw_parameter_string)

		BIND_F(raw_attribute_type)

		BIND_F(set_model_raw_attribute_int)
		BIND_F(set_model_raw_attribute_double)
		BIND_F(set_model_raw_attribute_string)
		BIND_F(get_model_raw_attribute_int)
		BIND_F(get_model_raw_attribute_double)
		BIND_F(get_model_raw_attribute_string)
		BIND_F(get_model_raw_attribute_vector_double)
		BIND_F(get_model_raw_attribute_list_double)

		BIND_F(set_variable_raw_attribute_int)
		BIND_F(set_variable_raw_attribute_char)
		BIND_F(set_variable_raw_attribute_double)
		BIND_F(set_variable_raw_attribute_string)
		BIND_F(get_variable_raw_attribute_int)
		BIND_F(get_variable_raw_attribute_char)
		BIND_F(get_variable_raw_attribute_double)
		BIND_F(get_variable_raw_attribute_string)

		BIND_F(set_constraint_raw_attribute_int)
		BIND_F(set_constraint_raw_attribute_char)
		BIND_F(set_constraint_raw_attribute_double)
		BIND_F(set_constraint_raw_attribute_string)
		BIND_F(get_constraint_raw_attribute_int)
		BIND_F(get_constraint_raw_attribute_char)
		BIND_F(get_constraint_raw_attribute_double)
		BIND_F(get_constraint_raw_attribute_string)

		BIND_F(get_normalized_rhs)
		BIND_F(set_normalized_rhs)
		BIND_F(get_normalized_coefficient)
		BIND_F(set_normalized_coefficient)
		BIND_F(get_objective_coefficient)
		BIND_F(set_objective_coefficient)

		BIND_F(_converttofixed)

		BIND_F(computeIIS)
	    // clang-format on
	    ;
}