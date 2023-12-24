#pragma once

#include <string>
#include "pyoptinterface/core.hpp"

class CommercialSolverBase
{
  public:
	virtual ConstraintIndex add_linear_constraint(const ScalarAffineFunction &function,
	                                              ConstraintSense sense, CoeffT rhs) = 0;
	ConstraintIndex add_linear_constraint_from_expr(const ExprBuilder &function,
	                                                ConstraintSense sense, CoeffT rhs);
	virtual ConstraintIndex add_quadratic_constraint(const ScalarQuadraticFunction &function,
	                                                 ConstraintSense sense, CoeffT rhs) = 0;
	ConstraintIndex add_quadratic_constraint_from_expr(const ExprBuilder &function,
	                                                   ConstraintSense sense, CoeffT rhs);

	virtual double get_variable_value(const VariableIndex &variable) = 0;
	double get_expression_value(const ScalarAffineFunction &function);
	double get_expression_value(const ScalarQuadraticFunction &function);
	double get_expression_value(const ExprBuilder &function);

	virtual std::string pprint_variable(const VariableIndex &variable) = 0;
	std::string pprint_expression(const ScalarAffineFunction &function, int precision = 4);
	std::string pprint_expression(const ScalarQuadraticFunction &function, int precision = 4);
	std::string pprint_expression(const ExprBuilder &function, int precision = 4);
};

struct AffineFunctionPtrForm
{
	int numnz;
	int *index;
	double *value;
	std::vector<int> index_storage;
};

struct QuadraticFunctionPtrForm
{
	int numnz;
	int *row;
	int *col;
	double *value;
	std::vector<int> row_storage;
	std::vector<int> col_storage;
};

template <typename T>
void make_affine_ptr_form(T *model, const ScalarAffineFunction &function,
                          AffineFunctionPtrForm &ptr_form)
{
	int numnz = function.size();
	ptr_form.numnz = numnz;
	ptr_form.index_storage.resize(numnz);
	for (int i = 0; i < numnz; ++i)
	{
		ptr_form.index_storage[i] = model->_variable_index(function.variables[i]);
	}
	ptr_form.index = ptr_form.index_storage.data();
	ptr_form.value = (double *)function.coefficients.data();
}

template <typename T>
void make_quadratic_ptr_form(T *model, const ScalarQuadraticFunction &function,
                             QuadraticFunctionPtrForm &ptr_form)
{
	int numnz = function.size();
	ptr_form.numnz = numnz;
	ptr_form.row_storage.resize(numnz);
	ptr_form.col_storage.resize(numnz);
	for (int i = 0; i < numnz; ++i)
	{
		ptr_form.row_storage[i] = model->_variable_index(function.variable_1s[i]);
		ptr_form.col_storage[i] = model->_variable_index(function.variable_2s[i]);
	}
	ptr_form.row = ptr_form.row_storage.data();
	ptr_form.col = ptr_form.col_storage.data();
	ptr_form.value = (double *)function.coefficients.data();
}