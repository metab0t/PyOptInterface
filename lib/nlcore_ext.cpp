#include <nanobind/nanobind.h>
#include <nanobind/operators.h>
#include <nanobind/make_iterator.h>
#include <nanobind/stl/bind_vector.h>
#include <nanobind/stl/vector.h>
#include <nanobind/stl/string.h>

namespace nb = nanobind;

#include "pyoptinterface/nlcore.hpp"
#include "cppad/utility/pow_int.hpp"

using a_double = CppAD::AD<double>;
using advec = std::vector<a_double>;
using ADFun = CppAD::ADFun<double>;

using graph_op_enum = CppAD::graph::graph_op_enum;

NB_MAKE_OPAQUE(advec);
NB_MAKE_OPAQUE(std::vector<NonlinearFunction>);

NB_MODULE(nlcore_ext, m)
{
	nb::class_<a_double>(m, "a_double")
	    .def(nb::init<>())
	    .def(nb::init<double>())
	    .def(-nb::self)
	    .def(nb::self + a_double())
	    .def(nb::self - a_double())
	    .def(nb::self * a_double())
	    .def(nb::self / a_double())
	    .def(nb::self += a_double())
	    .def(nb::self -= a_double())
	    .def(nb::self *= a_double())
	    .def(nb::self /= a_double())
	    .def(nb::self + double())
	    .def(double() + nb::self)
	    .def(nb::self - double())
	    .def(double() - nb::self)
	    .def(nb::self * double())
	    .def(double() * nb::self)
	    .def(nb::self / double())
	    .def(double() / nb::self)
	    .def(nb::self += double())
	    .def(nb::self -= double())
	    .def(nb::self *= double())
	    .def(nb::self /= double());

	nb::bind_vector<advec, nb::rv_policy::reference_internal>(m, "advec");

	m.def("Independent", nb::overload_cast<advec &>(&CppAD::Independent<advec>), nb::arg("x"));
	m.def("Independent", nb::overload_cast<advec &, advec &>(&CppAD::Independent<advec>),
	      nb::arg("x"), nb::arg("dynamic"));
	m.def("Independent", nb::overload_cast<advec &, size_t, bool>(&CppAD::Independent<advec>),
	      nb::arg("x"), nb::arg("abort_op_index"), nb::arg("record_compare"));
	m.def("Independent",
	      nb::overload_cast<advec &, size_t, bool, advec &>(&CppAD::Independent<advec>),
	      nb::arg("x"), nb::arg("abort_op_index"), nb::arg("record_compare"), nb::arg("dynamic"));

	using cpp_graph_iter_value = cpp_graph::const_iterator::value_type;
	nb::class_<cpp_graph_iter_value>(m, "cpp_graph_iter")
	    .def_ro("op_enum", &cpp_graph_iter_value::op_enum)
	    .def_ro("n_result", &cpp_graph_iter_value::n_result)
	    .def_prop_ro("arg_node", [](const cpp_graph_iter_value &value) {
		    nb::list result;
		    for (auto i : (*value.arg_node_ptr))
			    result.append(nb::int_(i));
		    return result;
	    });

	nb::class_<cpp_graph>(m, "cpp_graph")
	    .def(nb::init<>())
	    .def(
	        "__iter__",
	        [](cpp_graph &g) {
		        return nb::make_iterator<nb::rv_policy::reference_internal>(
		            nb::type<cpp_graph>(), "item_iterator", g.begin(), g.end());
	        },
	        nb::keep_alive<0, 1>())
	    .def_prop_ro("n_dynamic_ind", [](cpp_graph &g) { return g.n_dynamic_ind_get(); })
	    .def_prop_ro("n_variable_ind", [](cpp_graph &g) { return g.n_variable_ind_get(); })
	    .def_prop_ro("n_constant", [](cpp_graph &g) { return g.constant_vec_size(); })
	    .def_prop_ro("n_dependent", [](cpp_graph &g) { return g.dependent_vec_size(); })
	    .def_prop_ro("n_operator", [](cpp_graph &g) { return g.operator_vec_size(); })
	    .def("constant_vec_get", &cpp_graph::constant_vec_get)
	    .def("dependent_vec_get", &cpp_graph::dependent_vec_get);

	nb::class_<ADFun>(m, "ADFun")
	    .def(nb::init<>())
	    .def(nb::init<advec &, advec &>())
	    .def("Dependent", nb::overload_cast<const advec &, const advec &>(&ADFun::Dependent<advec>))
	    .def("to_graph", &ADFun::to_graph)
	    .def("Domain", &ADFun::Domain)
	    .def("Range", &ADFun::Range)
	    .def("size_dyn_ind", &ADFun::size_dyn_ind)
	    .def("optimize", &ADFun::optimize)
	    .def("function_name_set", &ADFun::function_name_set);

	nb::enum_<HessianSparsityType>(m, "HessianSparsityType")
	    .value("Full", HessianSparsityType::Full)
	    .value("Upper", HessianSparsityType::Upper)
	    .value("Lower", HessianSparsityType::Lower);

	nb::class_<sparsity_pattern_t>(m, "sparsity_pattern_t")
	    .def("nnz", &sparsity_pattern_t::nnz)
	    .def("to_list", [](const sparsity_pattern_t &pattern) {
		    nb::list row, col;
		    for (int i = 0; i < pattern.nnz(); i++)
		    {
			    auto r = pattern.row()[i];
			    auto c = pattern.col()[i];
			    row.append(nb::int_(r));
			    col.append(nb::int_(c));
		    }
		    nb::int_ nnz(pattern.nnz());
		    nb::tuple result = nb::make_tuple(row, col, nnz);
		    return result;
	    });
	nb::class_<JacobianHessianSparsityPattern>(m, "JacobianHessianSparsityPattern")
	    .def_ro("jacobian", &JacobianHessianSparsityPattern::jacobian)
	    .def_ro("hessian", &JacobianHessianSparsityPattern::hessian)
	    .def_ro("reduced_hessian", &JacobianHessianSparsityPattern::reduced_hessian);
	m.def("jacobian_hessian_sparsity", &jacobian_hessian_sparsity<double>);
	m.def("sparse_jacobian", &sparse_jacobian<double>);
	m.def("sparse_hessian", &sparse_hessian<double>);

	m.def("abs", [](const a_double &x) { return CppAD::abs(x); });
	m.def("acos", [](const a_double &x) { return CppAD::acos(x); });
	m.def("acosh", [](const a_double &x) { return CppAD::acosh(x); });
	m.def("asin", [](const a_double &x) { return CppAD::asin(x); });
	m.def("asinh", [](const a_double &x) { return CppAD::asinh(x); });
	m.def("atan", [](const a_double &x) { return CppAD::atan(x); });
	m.def("atanh", [](const a_double &x) { return CppAD::atanh(x); });
	m.def("cos", [](const a_double &x) { return CppAD::cos(x); });
	m.def("cosh", [](const a_double &x) { return CppAD::cosh(x); });
	m.def("erf", [](const a_double &x) { return CppAD::erf(x); });
	m.def("erfc", [](const a_double &x) { return CppAD::erfc(x); });
	m.def("exp", [](const a_double &x) { return CppAD::exp(x); });
	m.def("expm1", [](const a_double &x) { return CppAD::expm1(x); });
	m.def("log1p", [](const a_double &x) { return CppAD::log1p(x); });
	m.def("log", [](const a_double &x) { return CppAD::log(x); });
	m.def("pow", [](const a_double &x, const int &y) { return CppAD::pow(x, y); });
	m.def("pow", [](const a_double &x, const double &y) { return CppAD::pow(x, y); });
	m.def("pow", [](const double &x, const a_double &y) { return CppAD::pow(x, y); });
	m.def("pow", [](const a_double &x, const a_double &y) { return CppAD::pow(x, y); });
	m.def("sin", [](const a_double &x) { return CppAD::sin(x); });
	m.def("sinh", [](const a_double &x) { return CppAD::sinh(x); });
	m.def("sqrt", [](const a_double &x) { return CppAD::sqrt(x); });
	m.def("tan", [](const a_double &x) { return CppAD::tan(x); });
	m.def("tanh", [](const a_double &x) { return CppAD::tanh(x); });

	nb::enum_<graph_op_enum>(m, "graph_op")
	    .value("abs", graph_op_enum::abs_graph_op)
	    .value("acos", graph_op_enum::acos_graph_op)
	    .value("acosh", graph_op_enum::acosh_graph_op)
	    .value("add", graph_op_enum::add_graph_op)
	    .value("asin", graph_op_enum::asin_graph_op)
	    .value("asinh", graph_op_enum::asinh_graph_op)
	    .value("atan", graph_op_enum::atan_graph_op)
	    .value("atanh", graph_op_enum::atanh_graph_op)
	    .value("atom4", graph_op_enum::atom4_graph_op)
	    .value("atom", graph_op_enum::atom_graph_op)
	    .value("azmul", graph_op_enum::azmul_graph_op)
	    .value("cexp_eq", graph_op_enum::cexp_eq_graph_op)
	    .value("cexp_le", graph_op_enum::cexp_le_graph_op)
	    .value("cexp_lt", graph_op_enum::cexp_lt_graph_op)
	    .value("comp_eq", graph_op_enum::comp_eq_graph_op)
	    .value("comp_le", graph_op_enum::comp_le_graph_op)
	    .value("comp_lt", graph_op_enum::comp_lt_graph_op)
	    .value("comp_ne", graph_op_enum::comp_ne_graph_op)
	    .value("cos", graph_op_enum::cos_graph_op)
	    .value("cosh", graph_op_enum::cosh_graph_op)
	    .value("discrete", graph_op_enum::discrete_graph_op)
	    .value("div", graph_op_enum::div_graph_op)
	    .value("erf", graph_op_enum::erf_graph_op)
	    .value("erfc", graph_op_enum::erfc_graph_op)
	    .value("exp", graph_op_enum::exp_graph_op)
	    .value("expm1", graph_op_enum::expm1_graph_op)
	    .value("log1p", graph_op_enum::log1p_graph_op)
	    .value("log", graph_op_enum::log_graph_op)
	    .value("mul", graph_op_enum::mul_graph_op)
	    .value("neg", graph_op_enum::neg_graph_op)
	    .value("pow", graph_op_enum::pow_graph_op)
	    .value("print", graph_op_enum::print_graph_op)
	    .value("sign", graph_op_enum::sign_graph_op)
	    .value("sin", graph_op_enum::sin_graph_op)
	    .value("sinh", graph_op_enum::sinh_graph_op)
	    .value("sqrt", graph_op_enum::sqrt_graph_op)
	    .value("sub", graph_op_enum::sub_graph_op)
	    .value("sum", graph_op_enum::sum_graph_op)
	    .value("tan", graph_op_enum::tan_graph_op)
	    .value("tanh", graph_op_enum::tanh_graph_op);

	// Bind nonlinear core part
	nb::class_<NonlinearFunction>(m, "NonlinearFunction")
	    .def(nb::init<ADFunD &, const std::string &>())
	    .def_ro("name", &NonlinearFunction::name)
	    .def_ro("nx", &NonlinearFunction::nx)
	    .def_ro("np", &NonlinearFunction::np)
	    .def_ro("ny", &NonlinearFunction::ny)
	    .def_ro("f_graph", &NonlinearFunction::f_graph)
	    .def_ro("jacobian_graph", &NonlinearFunction::jacobian_graph)
	    .def_ro("hessian_graph", &NonlinearFunction::hessian_graph)
	    .def_ro("m_jacobian_nnz", &NonlinearFunction::m_jacobian_nnz)
	    .def_ro("m_hessian_nnz", &NonlinearFunction::m_hessian_nnz)
	    .def_ro("m_jacobian_rows", &NonlinearFunction::m_jacobian_rows)
	    .def_ro("m_jacobian_cols", &NonlinearFunction::m_jacobian_cols)
	    .def_ro("m_hessian_rows", &NonlinearFunction::m_hessian_rows)
	    .def_ro("m_hessian_cols", &NonlinearFunction::m_hessian_cols)
	    .def("assign_evaluators", &NonlinearFunction::assign_evaluators);

	nb::class_<ParameterIndex>(m, "ParameterIndex")
	    .def(nb::init<IndexT>())
	    .def_ro("index", &ParameterIndex::index);

	nb::class_<FunctionIndex>(m, "FunctionIndex")
	    .def(nb::init<IndexT>())
	    .def_ro("index", &FunctionIndex::index);

	nb::class_<NLConstraintIndex>(m, "NLConstraintIndex")
	    .def_ro("index", &NLConstraintIndex::index)
	    .def_ro("dim", &NLConstraintIndex::dim);

	nb::bind_vector<std::vector<NonlinearFunction>, nb::rv_policy::reference_internal>(
	    m, "nlfunctionvec");

	nb::class_<NonlinearFunctionModel>(m, "NonlinearFunctionModel")
	    .def(nb::init<>())
	    .def_ro("nl_functions", &NonlinearFunctionModel::nl_functions);
}
