#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>

#include "pyoptinterface/mosek_model.hpp"

namespace nb = nanobind;

extern void bind_mosek_constants(nb::module_ &m);

NB_MODULE(mosek_model_ext, m)
{
	bind_mosek_constants(m);

	nb::class_<MOSEKEnv>(m, "Env")
	    .def(nb::init<>())
	    .def("putlicensecode", &MOSEKEnv::putlicensecode);

	nb::class_<MOSEKModel>(m, "_RawModelBase");

#define BIND_F(f) .def(#f, &MOSEKModelMixin::f)
	nb::class_<MOSEKModelMixin, MOSEKModel>(m, "RawModel")
	    .def(nb::init<>())
	    .def(nb::init<const MOSEKEnv &>())
	    // clang-format off
	    BIND_F(init)
	    // clang-format on

	    .def("add_variable", &MOSEKModelMixin::add_variable,
	         nb::arg("domain") = VariableDomain::Continuous, nb::arg("lb") = -MSK_INFINITY,
	         nb::arg("ub") = MSK_INFINITY, nb::arg("name") = "")
	    /*.def("add_variables", &MOSEKModelMixin::add_variables, nb::arg("N"),
	         nb::arg("domain") = VariableDomain::Continuous, nb::arg("lb") = -MSK_INFINITY,
	         nb::arg("ub") = MSK_INFINITY)*/
	    // clang-format off
	    BIND_F(delete_variable)
	    BIND_F(is_variable_active)
	    // clang-format on

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

	    .def("add_linear_constraint",
	         nb::overload_cast<const ScalarAffineFunction &, ConstraintSense, CoeffT, const char *>(
	             &MOSEKModelMixin::add_linear_constraint),
	         nb::arg("expr"), nb::arg("sense"), nb::arg("rhs"), nb::arg("name") = "")
	    .def("add_linear_constraint",
	         nb::overload_cast<const ExprBuilder &, ConstraintSense, CoeffT, const char *>(
	             &MOSEKModelMixin::add_linear_constraint_from_expr),
	         nb::arg("expr"), nb::arg("sense"), nb::arg("rhs"), nb::arg("name") = "")
	    .def("add_quadratic_constraint",
	         nb::overload_cast<const ScalarQuadraticFunction &, ConstraintSense, CoeffT,
	                           const char *>(&MOSEKModelMixin::add_quadratic_constraint),
	         nb::arg("expr"), nb::arg("sense"), nb::arg("rhs"), nb::arg("name") = "")
	    .def("add_quadratic_constraint",
	         nb::overload_cast<const ExprBuilder &, ConstraintSense, CoeffT, const char *>(
	             &MOSEKModelMixin::add_quadratic_constraint_from_expr),
	         nb::arg("expr"), nb::arg("sense"), nb::arg("rhs"), nb::arg("name") = "")
	    .def("delete_constraint", &MOSEKModelMixin::delete_constraint)
	    .def("is_constraint_active", &MOSEKModelMixin::is_constraint_active)

	    .def("set_objective", nb::overload_cast<const ScalarQuadraticFunction &, ObjectiveSense>(
	                              &MOSEKModelMixin::set_objective))
	    .def("set_objective", nb::overload_cast<const ScalarAffineFunction &, ObjectiveSense>(
	                              &MOSEKModelMixin::set_objective))
	    .def("set_objective", nb::overload_cast<const ExprBuilder &, ObjectiveSense>(
	                              &MOSEKModelMixin::set_objective))

	    // clang-format off
	    BIND_F(optimize)
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