#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>

#include "pyoptinterface/mosek_model.hpp"

namespace nb = nanobind;

extern void bind_mosek_constants(nb::module_ &m);

NB_MODULE(mosek_model_ext, m)
{
	m.import_("pyoptinterface._src.core_ext");

	m.def("is_library_loaded", &mosek::is_library_loaded);
	m.def("load_library", &mosek::load_library);

	bind_mosek_constants(m);

	nb::class_<MOSEKEnv>(m, "Env")
	    .def(nb::init<>())
	    .def("close", &MOSEKEnv::close)
	    .def("putlicensecode", &MOSEKEnv::putlicensecode);

	nb::class_<MOSEKModel>(m, "_RawModelBase");

#define BIND_F(f) .def(#f, &MOSEKModelMixin::f)
	nb::class_<MOSEKModelMixin, MOSEKModel>(m, "RawModel")
	    .def(nb::init<>())
	    .def(nb::init<const MOSEKEnv &>())
	    // clang-format off
	    BIND_F(init)
	    BIND_F(write)
	    BIND_F(close)
	    // clang-format on

	    .def("add_variable", &MOSEKModelMixin::add_variable,
	         nb::arg("domain") = VariableDomain::Continuous, nb::arg("lb") = -MSK_INFINITY,
	         nb::arg("ub") = MSK_INFINITY, nb::arg("name") = "")
	    // clang-format off
	    BIND_F(delete_variable)
	    BIND_F(delete_variables)
	    BIND_F(is_variable_active)
	    // clang-format on
	    .def("set_variable_bounds", &MOSEKModelMixin::set_variable_bounds, nb::arg("variable"),
	         nb::arg("lb"), nb::arg("ub"))

	    .def("get_value",
	         nb::overload_cast<const VariableIndex &>(&MOSEKModelMixin::get_variable_value))
	    .def("get_value", nb::overload_cast<const ScalarAffineFunction &>(
	                          &MOSEKModelMixin::get_expression_value))
	    .def("get_value", nb::overload_cast<const ScalarQuadraticFunction &>(
	                          &MOSEKModelMixin::get_expression_value))
	    .def("get_value",
	         nb::overload_cast<const ExprBuilder &>(&MOSEKModelMixin::get_expression_value))

	    .def("pprint", &MOSEKModelMixin::pprint_variable)
	    .def("pprint",
	         nb::overload_cast<const ScalarAffineFunction &, int>(
	             &MOSEKModelMixin::pprint_expression),
	         nb::arg("expr"), nb::arg("precision") = 4)
	    .def("pprint",
	         nb::overload_cast<const ScalarQuadraticFunction &, int>(
	             &MOSEKModelMixin::pprint_expression),
	         nb::arg("expr"), nb::arg("precision") = 4)
	    .def("pprint",
	         nb::overload_cast<const ExprBuilder &, int>(&MOSEKModelMixin::pprint_expression),
	         nb::arg("expr"), nb::arg("precision") = 4)

	    .def("add_linear_constraint", &MOSEKModelMixin::add_linear_constraint, nb::arg("expr"),
	         nb::arg("sense"), nb::arg("rhs"), nb::arg("name") = "")
	    .def("add_linear_constraint", &MOSEKModelMixin::add_linear_constraint_from_var,
	         nb::arg("expr"), nb::arg("sense"), nb::arg("rhs"), nb::arg("name") = "")
	    .def("add_linear_constraint", &MOSEKModelMixin::add_linear_constraint_from_expr,
	         nb::arg("expr"), nb::arg("sense"), nb::arg("rhs"), nb::arg("name") = "")
	    .def("add_linear_constraint", &MOSEKModelMixin::add_linear_constraint_from_comparison,
	         nb::arg("con"), nb::arg("name") = "")
	    .def("add_quadratic_constraint", &MOSEKModelMixin::add_quadratic_constraint,
	         nb::arg("expr"), nb::arg("sense"), nb::arg("rhs"), nb::arg("name") = "")
	    .def("add_quadratic_constraint", &MOSEKModelMixin::add_quadratic_constraint_from_expr,
	         nb::arg("expr"), nb::arg("sense"), nb::arg("rhs"), nb::arg("name") = "")
	    .def("add_quadratic_constraint", &MOSEKModelMixin::add_quadratic_constraint_from_comparison,
	         nb::arg("con"), nb::arg("name") = "")
	    .def("add_second_order_cone_constraint", &MOSEKModelMixin::add_second_order_cone_constraint,
	         nb::arg("variables"), nb::arg("name") = "", nb::arg("rotated") = false)
	    .def("add_exp_cone_constraint", &MOSEKModelMixin::add_exp_cone_constraint,
	         nb::arg("variables"), nb::arg("name") = "", nb::arg("dual") = false)
	    // clang-format off
		BIND_F(delete_constraint)
		BIND_F(is_constraint_active)
	    // clang-format on

	    .def("set_objective",
	         nb::overload_cast<const ScalarQuadraticFunction &, ObjectiveSense>(
	             &MOSEKModelMixin::set_objective),
	         nb::arg("expr"), nb::arg("sense") = ObjectiveSense::Minimize)
	    .def("set_objective",
	         nb::overload_cast<const ScalarAffineFunction &, ObjectiveSense>(
	             &MOSEKModelMixin::set_objective),
	         nb::arg("expr"), nb::arg("sense") = ObjectiveSense::Minimize)
	    .def(
	        "set_objective",
	        nb::overload_cast<const ExprBuilder &, ObjectiveSense>(&MOSEKModelMixin::set_objective),
	        nb::arg("expr"), nb::arg("sense") = ObjectiveSense::Minimize)
	    .def("set_objective",
	         nb::overload_cast<const VariableIndex &, ObjectiveSense>(
	             &MOSEKModelMixin::set_objective_as_variable),
	         nb::arg("expr"), nb::arg("sense") = ObjectiveSense::Minimize)
	    .def("set_objective",
	         nb::overload_cast<CoeffT, ObjectiveSense>(&MOSEKModelMixin::set_objective_as_constant),
	         nb::arg("expr"), nb::arg("sense") = ObjectiveSense::Minimize)

	    .def("optimize", &MOSEKModelMixin::optimize, nb::call_guard<nb::gil_scoped_release>())

	    // clang-format off
	    BIND_F(version_string)
	    BIND_F(get_raw_model)

	    BIND_F(raw_parameter_type)

	    BIND_F(set_raw_parameter_int)
	    BIND_F(set_raw_parameter_double)
	    BIND_F(set_raw_parameter_string)
	    BIND_F(get_raw_parameter_int)
	    BIND_F(get_raw_parameter_double)
	    BIND_F(get_raw_parameter_string)

	    BIND_F(get_raw_information_int)
	    BIND_F(get_raw_information_double)

	    BIND_F(getnumvar)
	    BIND_F(getnumcon)
	    BIND_F(getprosta)
	    BIND_F(getsolsta)
		BIND_F(getprimalobj)
		BIND_F(getdualobj)

	    BIND_F(enable_log)
	    BIND_F(disable_log)

	    BIND_F(set_variable_name)
	    BIND_F(get_variable_name)
	    BIND_F(set_variable_type)
	    BIND_F(get_variable_type)
	    BIND_F(set_variable_lower_bound)
	    BIND_F(set_variable_upper_bound)
		BIND_F(get_variable_lower_bound)
	    BIND_F(get_variable_upper_bound)
	    BIND_F(set_variable_primal)

	    BIND_F(get_constraint_primal)
	    BIND_F(get_constraint_dual)
	    BIND_F(get_constraint_name)
	    BIND_F(set_constraint_name)

	    BIND_F(set_obj_sense)
	    BIND_F(get_obj_sense)

		BIND_F(get_normalized_rhs)
		BIND_F(set_normalized_rhs)
		BIND_F(get_normalized_coefficient)
		BIND_F(set_normalized_coefficient)
		BIND_F(get_objective_coefficient)
		BIND_F(set_objective_coefficient)
	    // clang-format on
	    ;
}