#include "pyoptinterface/solver_common.hpp"

ConstraintIndex CommercialSolverBase::add_linear_constraint_from_expr(const ExprBuilder &function,
                                                            ConstraintSense sense, CoeffT rhs)
{
	ScalarAffineFunction f(function);
	return add_linear_constraint(f, sense, rhs);
}

ConstraintIndex CommercialSolverBase::add_quadratic_constraint_from_expr(const ExprBuilder &function,
                                                               ConstraintSense sense, CoeffT rhs)
{
	ScalarQuadraticFunction f(function);
	return add_quadratic_constraint(f, sense, rhs);
}
