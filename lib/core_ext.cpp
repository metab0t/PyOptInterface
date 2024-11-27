#include <nanobind/nanobind.h>
#include <nanobind/operators.h>
#include <nanobind/stl/vector.h>
#include <nanobind/stl/pair.h>
#include <nanobind/stl/optional.h>

#include "pyoptinterface/core.hpp"
#include "pyoptinterface/container.hpp"

namespace nb = nanobind;

NB_MODULE(core_ext, m)
{
	// VariableDomain
	nb::enum_<VariableDomain>(m, "VariableDomain", nb::is_arithmetic())
	    .value("Continuous", VariableDomain::Continuous)
	    .value("Integer", VariableDomain::Integer)
	    .value("Binary", VariableDomain::Binary)
	    .value("SemiContinuous", VariableDomain::SemiContinuous);

	// ConstraintSense
	nb::enum_<ConstraintSense>(m, "ConstraintSense")
	    .value("LessEqual", ConstraintSense::LessEqual)
	    .value("Equal", ConstraintSense::Equal)
	    .value("GreaterEqual", ConstraintSense::GreaterEqual)
	    .value("Within", ConstraintSense::Within);

	// ConstraintType
	nb::enum_<ConstraintType>(m, "ConstraintType")
	    .value("Linear", ConstraintType::Linear)
	    .value("Quadratic", ConstraintType::Quadratic)
	    .value("SOS", ConstraintType::SOS)
	    .value("Cone", ConstraintType::Cone);

	nb::enum_<SOSType>(m, "SOSType").value("SOS1", SOSType::SOS1).value("SOS2", SOSType::SOS2);

	// ObjectiveSense
	nb::enum_<ObjectiveSense>(m, "ObjectiveSense")
	    .value("Minimize", ObjectiveSense::Minimize)
	    .value("Maximize", ObjectiveSense::Maximize);

	nb::class_<VariableIndex>(m, "VariableIndex")
	    .def(nb::init<IndexT>())
	    .def_ro("index", &VariableIndex::index)
	    .def(-nb::self)
	    .def(nb::self + CoeffT())
	    .def(CoeffT() + nb::self)
	    .def(nb::self + VariableIndex())
	    .def(nb::self + ScalarAffineFunction())
	    .def(nb::self + ScalarQuadraticFunction())
	    .def(nb::self - CoeffT())
	    .def(CoeffT() - nb::self)
	    .def(nb::self - VariableIndex())
	    .def(nb::self - ScalarAffineFunction())
	    .def(nb::self - ScalarQuadraticFunction())
	    .def(nb::self * CoeffT())
	    .def(CoeffT() * nb::self)
	    .def(nb::self * VariableIndex())
	    .def(nb::self * ScalarAffineFunction())
	    .def(nb::self / CoeffT());

	nb::class_<ConstraintIndex>(m, "ConstraintIndex")
	    .def_ro("type", &ConstraintIndex::type)
	    .def_ro("index", &ConstraintIndex::index);

	nb::class_<ScalarAffineFunction>(m, "ScalarAffineFunction")
	    .def(nb::init<>())
	    .def(nb::init<CoeffT>())
	    .def(nb::init<const VariableIndex &>())
	    .def(nb::init<const VariableIndex &, CoeffT>())
	    .def(nb::init<const VariableIndex &, CoeffT, CoeffT>())
	    .def(nb::init<const Vector<CoeffT> &, const Vector<IndexT> &>(), nb::arg("coefficients"),
	         nb::arg("variables"))
	    .def(nb::init<const Vector<CoeffT> &, const Vector<IndexT> &, CoeffT>(),
	         nb::arg("coefficients"), nb::arg("variables"), nb::arg("constant"))
	    .def(nb::init<const ExprBuilder &>())
	    .def_ro("coefficients", &ScalarAffineFunction::coefficients)
	    .def_ro("variables", &ScalarAffineFunction::variables)
	    .def_ro("constant", &ScalarAffineFunction::constant)
	    .def("size", &ScalarAffineFunction::size)
	    .def("canonicalize", &ScalarAffineFunction::canonicalize,
	         nb::arg("threshold") = COEFTHRESHOLD)
	    .def("reserve", &ScalarAffineFunction::reserve)
	    .def("add_term", &ScalarAffineFunction::add_term)
	    .def("add_constant", &ScalarAffineFunction::add_constant)
	    .def(-nb::self)
	    .def(nb::self + CoeffT())
	    .def(CoeffT() + nb::self)
	    .def(nb::self + VariableIndex())
	    .def(nb::self + ScalarAffineFunction())
	    .def(nb::self + ScalarQuadraticFunction())
	    .def(nb::self - CoeffT())
	    .def(CoeffT() - nb::self)
	    .def(nb::self - VariableIndex())
	    .def(nb::self - ScalarAffineFunction())
	    .def(nb::self - ScalarQuadraticFunction())
	    .def(nb::self * CoeffT())
	    .def(CoeffT() * nb::self)
	    .def(nb::self * VariableIndex())
	    .def(nb::self * ScalarAffineFunction())
	    .def(nb::self / CoeffT());

	nb::class_<ScalarQuadraticFunction>(m, "ScalarQuadraticFunction")
	    .def(nb::init<>())
	    .def(nb::init<const Vector<CoeffT> &, const Vector<IndexT> &, const Vector<IndexT> &>(),
	         nb::arg("coefficients"), nb::arg("var1s"), nb::arg("var2s"))
	    .def(nb::init<const Vector<CoeffT> &, const Vector<IndexT> &, const Vector<IndexT> &,
	                  const ScalarAffineFunction &>(),
	         nb::arg("coefficients"), nb::arg("var1s"), nb::arg("var2s"), nb::arg("affine_part"))
	    .def(nb::init<const ExprBuilder &>())
	    .def_ro("coefficients", &ScalarQuadraticFunction::coefficients)
	    .def_ro("variable_1s", &ScalarQuadraticFunction::variable_1s)
	    .def_ro("variable_2s", &ScalarQuadraticFunction::variable_2s)
	    .def_ro("affine_part", &ScalarQuadraticFunction::affine_part)
	    .def("size", &ScalarQuadraticFunction::size)
	    .def("canonicalize", &ScalarQuadraticFunction::canonicalize,
	         nb::arg("threshold") = COEFTHRESHOLD)
	    .def("reserve_quadratic", &ScalarQuadraticFunction::reserve_quadratic)
	    .def("reserve_affine", &ScalarQuadraticFunction::reserve_affine)
	    .def("add_quadratic_term", &ScalarQuadraticFunction::add_quadratic_term)
	    .def("add_affine_term", &ScalarQuadraticFunction::add_affine_term)
	    .def("add_constant", &ScalarQuadraticFunction::add_constant)
	    .def(-nb::self)
	    .def(nb::self + CoeffT())
	    .def(CoeffT() + nb::self)
	    .def(nb::self + VariableIndex())
	    .def(nb::self + ScalarAffineFunction())
	    .def(nb::self + ScalarQuadraticFunction())
	    .def(nb::self - CoeffT())
	    .def(CoeffT() - nb::self)
	    .def(nb::self - VariableIndex())
	    .def(nb::self - ScalarAffineFunction())
	    .def(nb::self - ScalarQuadraticFunction())
	    .def(nb::self * CoeffT())
	    .def(CoeffT() * nb::self)
	    .def(nb::self / CoeffT());

	nb::class_<VariablePair>(m, "VariablePair").def(nb::init<IndexT, IndexT>());

	nb::class_<ExprBuilder>(m, "ExprBuilder")
	    .def(nb::init<>())
	    .def(nb::init<CoeffT>())
	    .def(nb::init<const VariableIndex &>())
	    .def(nb::init<const ScalarAffineFunction &>())
	    .def(nb::init<const ScalarQuadraticFunction &>())
	    .def("empty", &ExprBuilder::empty)
	    .def("degree", &ExprBuilder::degree)
	    .def("reserve_quadratic", &ExprBuilder::reserve_quadratic)
	    .def("reserve_affine", &ExprBuilder::reserve_affine)
	    .def("clear", &ExprBuilder::clear)
	    .def("clean_nearzero_terms", &ExprBuilder::clean_nearzero_terms,
	         nb::arg("threshold") = COEFTHRESHOLD)
	    .def("add_quadratic_term", &ExprBuilder::add_quadratic_term)
	    .def("set_quadratic_coef", &ExprBuilder::set_quadratic_coef)
	    .def("add_affine_term", &ExprBuilder::add_affine_term)
	    .def("set_affine_coef", &ExprBuilder::set_affine_coef)
	    .def(-nb::self)
	    .def(nb::self += CoeffT(), nb::rv_policy::none)
	    .def(nb::self += VariableIndex(), nb::rv_policy::none)
	    .def(nb::self += ScalarAffineFunction(), nb::rv_policy::none)
	    .def(nb::self += ScalarQuadraticFunction(), nb::rv_policy::none)
	    .def(nb::self += ExprBuilder(), nb::rv_policy::none)
	    .def(nb::self -= CoeffT(), nb::rv_policy::none)
	    .def(nb::self -= VariableIndex(), nb::rv_policy::none)
	    .def(nb::self -= ScalarAffineFunction(), nb::rv_policy::none)
	    .def(nb::self -= ScalarQuadraticFunction(), nb::rv_policy::none)
	    .def(nb::self -= ExprBuilder(), nb::rv_policy::none)
	    .def(nb::self *= CoeffT(), nb::rv_policy::none)
	    .def(nb::self *= VariableIndex(), nb::rv_policy::none)
	    .def(nb::self *= ScalarAffineFunction(), nb::rv_policy::none)
	    .def(nb::self *= ScalarQuadraticFunction(), nb::rv_policy::none)
	    .def(nb::self *= ExprBuilder(), nb::rv_policy::none)
	    .def(nb::self /= CoeffT(), nb::rv_policy::none)
	    .def(nb::self + CoeffT())
	    .def(CoeffT() + nb::self)
	    .def(nb::self + VariableIndex())
	    .def(nb::self + ScalarAffineFunction())
	    .def(nb::self + ScalarQuadraticFunction())
	    .def(nb::self + ExprBuilder())
	    .def(VariableIndex() + nb::self)
	    .def(ScalarAffineFunction() + nb::self)
	    .def(ScalarQuadraticFunction() + nb::self)
	    .def(ExprBuilder() + nb::self)
	    .def(nb::self - CoeffT())
	    .def(CoeffT() - nb::self)
	    .def(nb::self - VariableIndex())
	    .def(nb::self - ScalarAffineFunction())
	    .def(nb::self - ScalarQuadraticFunction())
	    .def(nb::self - ExprBuilder())
	    .def(VariableIndex() - nb::self)
	    .def(ScalarAffineFunction() - nb::self)
	    .def(ScalarQuadraticFunction() - nb::self)
	    .def(ExprBuilder() - nb::self)
	    .def(nb::self * CoeffT())
	    .def(CoeffT() * nb::self)
	    .def(nb::self * VariableIndex())
	    .def(nb::self * ScalarAffineFunction())
	    .def(nb::self * ScalarQuadraticFunction())
	    .def(nb::self * ExprBuilder())
	    .def(VariableIndex() * nb::self)
	    .def(ScalarAffineFunction() * nb::self)
	    .def(ScalarQuadraticFunction() * nb::self)
	    .def(ExprBuilder() * nb::self)
	    .def(nb::self / CoeffT());

	// We need to test the functionality of MonotoneIndexer
	using IntMonotoneIndexer = MonotoneIndexer<int>;
	nb::class_<IntMonotoneIndexer>(m, "IntMonotoneIndexer")
	    .def(nb::init<>())
	    .def("add_index", &IntMonotoneIndexer::add_index)
	    .def("get_index", &IntMonotoneIndexer::get_index)
	    .def("delete_index", &IntMonotoneIndexer::delete_index);
}
