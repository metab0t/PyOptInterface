#pragma once

#include "pyoptinterface/core.hpp"

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