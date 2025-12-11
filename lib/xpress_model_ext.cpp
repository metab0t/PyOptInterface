#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/pair.h>
#include <nanobind/stl/tuple.h>
#include <nanobind/stl/vector.h>
#include <nanobind/stl/variant.h>
#include <nanobind/stl/function.h>
#include <nanobind/trampoline.h>
#include "pyoptinterface/core.hpp"
#include "pyoptinterface/xpress_model.hpp"

namespace nb = nanobind;

using namespace nb::literals;
using namespace xpress;

extern void bind_xpress_constants(nb::module_ &m);

NB_MODULE(xpress_model_ext, m)
{
	m.import_("pyoptinterface._src.core_ext");

	m.def("is_library_loaded", &xpress::is_library_loaded);
	m.def("load_library", &xpress::load_library);
	m.def("license", &xpress::license, "i"_a, "c"_a);
	m.def("beginlicensing", &xpress::beginlicensing);
	m.def("endlicensing", &xpress::endlicensing);

	bind_xpress_constants(m);

	nb::class_<Env>(m, "Env")
	    .def(nb::init<>())
	    .def(nb::init<const char *>(), "path"_a = nullptr)
	    .def("close", &Env::close);

	nb::class_<Model>(m, "RawModel")
	    .def(nb::init<>())
	    .def(nb::init<const Env &>(), nb::keep_alive<1, 2>())

	    // Model management
	    .def("init", &Model::init, "env"_a)
	    .def("close", &Model::close)
	    .def("optimize", &Model::optimize, nb::call_guard<nb::gil_scoped_release>())
	    .def("computeIIS", &Model::computeIIS, nb::call_guard<nb::gil_scoped_release>())
	    .def("write", &Model::write, "filename"_a, nb::call_guard<nb::gil_scoped_release>())
	    .def("_is_mip", &Model::_is_mip)
	    .def_static("get_infinity", &Model::get_infinity)
	    .def("get_problem_name", &Model::get_problem_name)
	    .def("set_problem_name", &Model::set_problem_name, "probname"_a)
	    .def("add_mip_start", &Model::add_mip_start, "variables"_a, "values"_a)
	    .def("get_raw_model", &Model::get_raw_model)
	    .def("version_string", &Model::version_string)

	    // Index mappings
	    .def("_constraint_index", &Model::_constraint_index, "constraint"_a)
	    .def("_variable_index", &Model::_variable_index, "variable"_a)
	    .def("_checked_constraint_index", &Model::_checked_constraint_index, "constraint"_a)
	    .def("_checked_variable_index", &Model::_checked_variable_index, "variable"_a)

	    // Variables
	    .def("add_variable", &Model::add_variable, "domain"_a = VariableDomain::Continuous,
	         "lb"_a = XPRS_MINUSINFINITY, "ub"_a = XPRS_PLUSINFINITY, "name"_a = "")
	    .def("delete_variable", &Model::delete_variable, "variable"_a)
	    .def("delete_variables", &Model::delete_variables, "variables"_a)
	    .def("set_objective_coefficient", &Model::set_objective_coefficient, "variable"_a,
	         "value"_a)
	    .def("set_variable_bounds", &Model::set_variable_bounds, "variable"_a, "lb"_a, "ub"_a)
	    .def("set_variable_lowerbound", &Model::set_variable_lowerbound, "variable"_a, "lb"_a)
	    .def("set_variable_name", &Model::set_variable_name, "variable"_a, "name"_a)
	    .def("set_variable_type", &Model::set_variable_type, "variable"_a, "vtype"_a)
	    .def("set_variable_upperbound", &Model::set_variable_upperbound, "variable"_a, "ub"_a)
	    .def("is_variable_active", &Model::is_variable_active, "variable"_a)
	    .def("is_variable_basic", &Model::is_variable_basic, "variable"_a)
	    .def("get_variable_lowerbound_IIS", &Model::is_variable_lowerbound_IIS, "variable"_a)
	    .def("is_variable_nonbasic_lb", &Model::is_variable_nonbasic_lb, "variable"_a)
	    .def("is_variable_nonbasic_ub", &Model::is_variable_nonbasic_ub, "variable"_a)
	    .def("is_variable_superbasic", &Model::is_variable_superbasic, "variable"_a)
	    .def("get_variable_upperbound_IIS", &Model::is_variable_upperbound_IIS, "variable"_a)
	    .def("get_objective_coefficient", &Model::get_objective_coefficient, "variable"_a)
	    .def("get_variable_lowerbound", &Model::get_variable_lowerbound, "variable"_a)
	    .def("get_variable_primal_ray", &Model::get_variable_primal_ray, "variable"_a)
	    .def("get_variable_rc", &Model::get_variable_rc, "variable"_a)
	    .def("get_variable_upperbound", &Model::get_variable_upperbound, "variable"_a)
	    .def("get_variable_value", &Model::get_variable_value, "variable"_a)
	    .def("get_variable_name", &Model::get_variable_name, "variable"_a)
	    .def("pprint", &Model::pprint_variable, "variable"_a)
	    .def("get_variable_type", &Model::get_variable_type, "variable"_a)

	    // Constraints
	    .def("add_exp_cone_constraint", &Model::add_exp_cone_constraint, "variables"_a,
	         "name"_a = "", "dual"_a = false)
	    .def("_add_linear_constraint",
	         nb::overload_cast<const ScalarAffineFunction &, const std::tuple<double, double> &,
	                           const char *>(&Model::add_linear_constraint),
	         "function"_a, "interval"_a, "name"_a = "")
	    .def("_add_linear_constraint",
	         nb::overload_cast<const ScalarAffineFunction &, ConstraintSense, CoeffT, const char *>(
	             &Model::add_linear_constraint),
	         "function"_a, "sense"_a, "rhs"_a, "name"_a = "")
	    .def("add_quadratic_constraint", &Model::add_quadratic_constraint, "function"_a, "sense"_a,
	         "rhs"_a, "name"_a = "")
	    .def("_add_quadratic_constraint", &Model::add_quadratic_constraint, "function"_a, "sense"_a,
	         "rhs"_a, "name"_a = "")
	    .def("add_second_order_cone_constraint", &Model::add_second_order_cone_constraint,
	         "variables"_a, "name"_a = "", "rotated"_a = false)
	    .def("_add_single_nl_constraint", &Model::add_single_nl_constraint, "graph"_a, "result"_a,

	         "interval"_a, "name"_a = "")
	    .def("add_sos_constraint",
	         nb::overload_cast<const Vector<VariableIndex> &, SOSType, const Vector<CoeffT> &>(
	             &Model::add_sos_constraint),
	         "variables"_a, "sos_type"_a, "weights"_a)
	    .def("add_sos_constraint",
	         nb::overload_cast<const Vector<VariableIndex> &, SOSType>(&Model::add_sos_constraint),
	         "variables"_a, "sos_type"_a)
	    .def("delete_constraint", &Model::delete_constraint, "constraint"_a)
	    .def("set_constraint_name", &Model::set_constraint_name, "constraint"_a, "name"_a)
	    .def("set_constraint_rhs", &Model::set_constraint_rhs, "constraint"_a, "rhs"_a)
	    .def("set_constraint_sense", &Model::set_constraint_sense, "constraint"_a, "sense"_a)
	    .def("set_normalized_coefficient", &Model::set_normalized_coefficient, "constraint"_a,
	         "variable"_a, "value"_a)
	    .def("set_normalized_rhs", &Model::set_normalized_rhs, "constraint"_a, "value"_a)
	    .def("is_constraint_active", &Model::is_constraint_active, "constraint"_a)
	    .def("is_constraint_basic", &Model::is_constraint_basic, "constraint"_a)
	    .def("is_constraint_in_IIS", &Model::is_constraint_in_IIS, "constraint"_a)
	    .def("is_constraint_nonbasic_lb", &Model::is_constraint_nonbasic_lb, "constraint"_a)
	    .def("is_constraint_nonbasic_ub", &Model::is_constraint_nonbasic_ub, "constraint"_a)
	    .def("is_constraint_superbasic", &Model::is_constraint_superbasic, "constraint"_a)
	    .def("get_constraint_dual_ray", &Model::get_constraint_dual_ray, "constraint"_a)
	    .def("get_constraint_dual", &Model::get_constraint_dual, "constraint"_a)
	    .def("get_constraint_slack", &Model::get_constraint_slack, "constraint"_a)
	    .def("get_normalized_coefficient", &Model::get_normalized_coefficient, "constraint"_a,
	         "variable"_a)
	    .def("get_normalized_rhs", &Model::get_normalized_rhs, "constraint"_a)
	    .def("get_constraint_rhs", &Model::get_constraint_rhs, "constraint"_a)
	    .def("get_constraint_name", &Model::get_constraint_name, "constraint"_a)
	    .def("get_constraint_sense", &Model::get_constraint_sense, "constraint"_a)

	    // Objective function
	    .def("set_objective",
	         nb::overload_cast<const ScalarAffineFunction &, ObjectiveSense>(&Model::set_objective),
	         "function"_a, "sense"_a = ObjectiveSense::Minimize)
	    .def("set_objective",
	         nb::overload_cast<const ScalarQuadraticFunction &, ObjectiveSense>(
	             &Model::set_objective),
	         "function"_a, "sense"_a = ObjectiveSense::Minimize)
	    .def("set_objective",
	         nb::overload_cast<const ExprBuilder &, ObjectiveSense>(&Model::set_objective),
	         "function"_a, "sense"_a = ObjectiveSense::Minimize)
	    .def("_add_single_nl_objective", &Model::add_single_nl_objective, "graph"_a, "result"_a)

	    // Status queries
	    .def("get_lp_status", &Model::get_lp_status)
	    .def("get_mip_status", &Model::get_mip_status)
	    .def("get_nlp_status", &Model::get_nlp_status)
	    .def("get_sol_status", &Model::get_sol_status)
	    .def("get_solve_status", &Model::get_solve_status)
	    .def("get_optimize_type", &Model::get_optimize_type)
	    .def("get_iis_sol_status", &Model::get_iis_sol_status)

	    // Raw control/attribute access by ID
	    .def("set_raw_control_dbl_by_id", &Model::set_raw_control_dbl_by_id, "control"_a, "value"_a)
	    .def("set_raw_control_int_by_id", &Model::set_raw_control_int_by_id, "control"_a, "value"_a)
	    .def("set_raw_control_str_by_id", &Model::set_raw_control_str_by_id, "control"_a, "value"_a)
	    .def("get_raw_control_int_by_id", &Model::get_raw_control_int_by_id, "control"_a)
	    .def("get_raw_control_dbl_by_id", &Model::get_raw_control_dbl_by_id, "control"_a)
	    .def("get_raw_control_str_by_id", &Model::get_raw_control_str_by_id, "control"_a)
	    .def("get_raw_attribute_int_by_id", &Model::get_raw_attribute_int_by_id, "attrib"_a)
	    .def("get_raw_attribute_dbl_by_id", &Model::get_raw_attribute_dbl_by_id, "attrib"_a)
	    .def("get_raw_attribute_str_by_id", &Model::get_raw_attribute_str_by_id, "attrib"_a)

	    // Raw control/attribute access by string
	    .def("set_raw_control_int", &Model::set_raw_control_int, "control"_a, "value"_a)
	    .def("set_raw_control_dbl", &Model::set_raw_control_dbl, "control"_a, "value"_a)
	    .def("set_raw_control_str", &Model::set_raw_control_str, "control"_a, "value"_a)
	    .def("get_raw_control_int", &Model::get_raw_control_int, "control"_a)
	    .def("get_raw_control_dbl", &Model::get_raw_control_dbl, "control"_a)
	    .def("get_raw_control_str", &Model::get_raw_control_str, "control"_a)
	    .def("get_raw_attribute_int", &Model::get_raw_attribute_int, "attrib"_a)
	    .def("get_raw_attribute_dbl", &Model::get_raw_attribute_dbl, "attrib"_a)
	    .def("get_raw_attribute_str", &Model::get_raw_attribute_str, "attrib"_a)

	    // Generic variant access
	    .def("set_raw_control", &Model::set_raw_control, "control"_a, "value"_a)
	    .def("get_raw_attribute", &Model::get_raw_attribute, "attrib"_a)
	    .def("get_raw_control", &Model::get_raw_control, "control"_a)

	    // Callback methods
	    .def("set_callback", &Model::set_callback, "callback"_a, "cbctx"_a)
	    .def("cb_get_arguments", &Model::cb_get_arguments, nb::rv_policy::reference)
	    .def("cb_get_solution", &Model::cb_get_solution, "variable"_a)
	    .def("cb_get_relaxation", &Model::cb_get_relaxation, "variable"_a)
	    .def("cb_get_incumbent", &Model::cb_get_incumbent, "variable"_a)
	    .def("cb_set_solution", &Model::cb_set_solution, "variable"_a, "value"_a)
	    .def("cb_submit_solution", &Model::cb_submit_solution)
	    .def("cb_exit", &Model::cb_exit)
	    .def("cb_add_lazy_constraint",
	         nb::overload_cast<const ScalarAffineFunction &, ConstraintSense, CoeffT>(
	             &Model::cb_add_lazy_constraint),
	         "function"_a, "sense"_a, "rhs"_a)
	    .def("cb_add_lazy_constraint",
	         nb::overload_cast<const ExprBuilder &, ConstraintSense, CoeffT>(
	             &Model::cb_add_lazy_constraint),
	         "function"_a, "sense"_a, "rhs"_a)
	    .def("cb_add_user_cut",
	         nb::overload_cast<const ScalarAffineFunction &, ConstraintSense, CoeffT>(
	             &Model::cb_add_user_cut),
	         "function"_a, "sense"_a, "rhs"_a)
	    .def("cb_add_user_cut",
	         nb::overload_cast<const ExprBuilder &, ConstraintSense, CoeffT>(
	             &Model::cb_add_user_cut),
	         "function"_a, "sense"_a, "rhs"_a)

	    // Functions defined in CRTP Mixins
	    .def("pprint",
	         nb::overload_cast<const ScalarAffineFunction &, int>(&Model::pprint_expression),
	         "expr"_a, "precision"_a = 4)
	    .def("pprint",
	         nb::overload_cast<const ScalarQuadraticFunction &, int>(&Model::pprint_expression),
	         "expr"_a, "precision"_a = 4)
	    .def("pprint", nb::overload_cast<const ExprBuilder &, int>(&Model::pprint_expression),
	         "expr"_a, "precision"_a = 4)

	    .def("get_value", nb::overload_cast<VariableIndex>(&Model::get_variable_value))
	    .def("get_value",
	         nb::overload_cast<const ScalarAffineFunction &>(&Model::get_expression_value))
	    .def("get_value",
	         nb::overload_cast<const ScalarQuadraticFunction &>(&Model::get_expression_value))
	    .def("get_value", nb::overload_cast<const ExprBuilder &>(&Model::get_expression_value))

	    .def("_add_linear_constraint", &Model::add_linear_constraint_from_var, "expr"_a, "sense"_a,
	         "rhs"_a, "name"_a = "")
	    .def("_add_linear_constraint", &Model::add_linear_interval_constraint_from_var, "expr"_a,
	         "interval"_a, "name"_a = "")
	    .def("_add_linear_constraint", &Model::add_linear_constraint_from_expr, "expr"_a, "sense"_a,
	         "rhs"_a, "name"_a = "")
	    .def("_add_linear_constraint", &Model::add_linear_interval_constraint_from_expr, "expr"_a,
	         "interval"_a, "name"_a = "")
	    .def("_add_linear_constraint", &Model::add_linear_constraint_from_var, "expr"_a, "sense"_a,
	         "rhs"_a, "name"_a = "")
	    .def("_add_linear_constraint", &Model::add_linear_interval_constraint_from_var, "expr"_a,
	         "interval"_a, "name"_a = "")
	    .def("_add_linear_constraint", &Model::add_linear_constraint_from_expr, "expr"_a, "sense"_a,
	         "rhs"_a, "name"_a = "")
	    .def("_add_linear_constraint", &Model::add_linear_interval_constraint_from_expr, "expr"_a,
	         "interval"_a, "name"_a = "")

	    .def("_add_single_nl_constraint", &Model::add_single_nl_constraint_sense_rhs, "graph"_a,
	         "result"_a, "sense"_a, "rhs"_a, "name"_a = "")
	    .def("_add_single_nl_constraint", &Model::add_single_nl_constraint_from_comparison,
	         "graph"_a, "expr"_a, "name"_a = "")

	    .def("set_objective", &Model::set_objective_as_variable, "expr"_a,
	         "sense"_a = ObjectiveSense::Minimize)
	    .def("set_objective", &Model::set_objective_as_constant, "expr"_a,
	         "sense"_a = ObjectiveSense::Minimize);

	// Bind the return value only it the CB has one
	auto bind_ret_code = []<class S>(nb::class_<S> s) {
		// "if constexpr + templates" conditionally instantiates only the true branch.
		if constexpr (requires { &S::ret_code; })
			s.def_rw("ret_code", &S::ret_code);
	};

// When callbacks provide pointer arguments, those are usually meant as output arguments.
// An exception is with pointer to structs, which usually are just opaque objects passed around
// between API calls.
// We define pointer arguments as read-write, while all the other arguments stays read-only
#define XPRSCB_ARG_NB_FIELD(TYPE, NAME)                        \
	if constexpr (std::is_pointer_v<decltype(struct_t::NAME)>) \
		s.def_rw(#NAME, &struct_t::NAME);                      \
	else                                                       \
		s.def_ro(#NAME, &struct_t::NAME);

// Define the binding for the argument struct of each CB. In this way, Nanobind will be able to
// translate our std::variant of CB-struct pointers into the proper Python union object
#define XPRSCB_NB_STRUCTS(ID, NAME, RET, ...)              \
	{                                                      \
		using struct_t = NAME##_struct;                    \
		auto s = nb::class_<struct_t>(m, #NAME "_struct"); \
		__VA_ARGS__                                        \
		bind_ret_code(s);                                  \
	}
	XPRSCB_LIST(XPRSCB_NB_STRUCTS, XPRSCB_ARG_NB_FIELD)
#undef XPRSCB_NB_STRUCTS
}
