#include <nanobind/nanobind.h>
#include <nanobind/operators.h>
#include <nanobind/stl/vector.h>
#include <nanobind/stl/optional.h>

#include "pyoptinterface/core.hpp"
#include "pyoptinterface/solver_common.hpp"

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
	    .value("GreaterEqual", ConstraintSense::GreaterEqual);

	// ConstraintType
	nb::enum_<ConstraintType>(m, "ConstraintType")
	    .value("Linear", ConstraintType::Linear)
	    .value("Quadratic", ConstraintType::Quadratic)
	    .value("SOS1", ConstraintType::SOS1)
	    .value("SOS2", ConstraintType::SOS2);

	// ObjectiveSense
	nb::enum_<ObjectiveSense>(m, "ObjectiveSense")
	    .value("Minimize", ObjectiveSense::Minimize)
	    .value("Maximize", ObjectiveSense::Maximize);

	nb::class_<VariableIndex>(m, "VariableIndex")
	    .def(nb::init<IndexT>())
	    .def_ro("index", &VariableIndex::index)
	    .def(nb::self + CoeffT())
	    .def(CoeffT() + nb::self)
	    .def(nb::self + VariableIndex())
	    .def(nb::self + ScalarAffineFunction())
	    .def(nb::self + ScalarQuadraticFunction())
	    .def(nb::self - CoeffT())
	    .def(nb::self - VariableIndex())
	    .def(nb::self - ScalarAffineFunction())
	    .def(nb::self - ScalarQuadraticFunction())
	    .def(nb::self * CoeffT())
	    .def(CoeffT() * nb::self)
	    .def(nb::self * VariableIndex())
	    .def(nb::self * ScalarAffineFunction());

	nb::class_<ConstraintIndex>(m, "ConstraintIndex")
	    .def_ro("type", &ConstraintIndex::type)
	    .def_ro("index", &ConstraintIndex::index);

	nb::class_<ScalarAffineFunction>(m, "ScalarAffineFunction")
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
	    .def("canonicalize", &ScalarAffineFunction::canonicalize,
	         nb::arg("threshold") = COEFTHRESHOLD)
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
	    .def(nb::self * ScalarAffineFunction());

	nb::class_<ScalarQuadraticFunction>(m, "ScalarQuadraticFunction")
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
	    .def("canonicalize", &ScalarQuadraticFunction::canonicalize,
	         nb::arg("threshold") = COEFTHRESHOLD)
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
	    .def(CoeffT() * nb::self);

	nb::class_<VariablePair>(m, "VariablePair").def(nb::init<IndexT, IndexT>());

	nb::class_<ExprBuilder>(m, "ExprBuilder")
	    .def(nb::init<>())
	    .def(nb::init<const VariableIndex &>())
	    .def(nb::init<const ScalarAffineFunction &>())
	    .def(nb::init<const ScalarQuadraticFunction &>())
	    .def("empty", &ExprBuilder::empty)
	    .def("degree", &ExprBuilder::degree)
	    .def("clear", &ExprBuilder::clear)
	    .def("clean_nearzero_terms", &ExprBuilder::clean_nearzero_terms,
	         nb::arg("threshold") = COEFTHRESHOLD)
	    .def("add_quadratic_term", &ExprBuilder::add_quadratic_term)
	    .def("set_quadratic_coef", &ExprBuilder::set_quadratic_coef)
	    .def("add_affine_term", &ExprBuilder::add_affine_term)
	    .def("set_affine_coef", &ExprBuilder::set_affine_coef)
	    .def("add", nb::overload_cast<CoeffT>(&ExprBuilder::add))
	    .def("add", nb::overload_cast<const VariableIndex &>(&ExprBuilder::add))
	    .def("add", nb::overload_cast<const ScalarAffineFunction &>(&ExprBuilder::add))
	    .def("add", nb::overload_cast<const ScalarQuadraticFunction &>(&ExprBuilder::add))
	    .def("add", nb::overload_cast<const ExprBuilder &>(&ExprBuilder::add))
	    .def("sub", nb::overload_cast<CoeffT>(&ExprBuilder::sub))
	    .def("sub", nb::overload_cast<const VariableIndex &>(&ExprBuilder::sub))
	    .def("sub", nb::overload_cast<const ScalarAffineFunction &>(&ExprBuilder::sub))
	    .def("sub", nb::overload_cast<const ScalarQuadraticFunction &>(&ExprBuilder::sub))
	    .def("sub", nb::overload_cast<const ExprBuilder &>(&ExprBuilder::sub))
	    .def("mul", nb::overload_cast<CoeffT>(&ExprBuilder::mul))
	    .def("mul", nb::overload_cast<const VariableIndex &>(&ExprBuilder::mul))
	    .def("mul", nb::overload_cast<const ScalarAffineFunction &>(&ExprBuilder::mul))
	    .def("mul", nb::overload_cast<const ScalarQuadraticFunction &>(&ExprBuilder::mul))
	    .def("mul", nb::overload_cast<const ExprBuilder &>(&ExprBuilder::mul));
}
