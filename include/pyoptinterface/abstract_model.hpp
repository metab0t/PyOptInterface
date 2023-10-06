#pragma once

#include "pyoptinterface/core.hpp"

class AbstractModel
{
  public:
	virtual VariableIndex add_variable(VariableDomain domain) = 0;
	virtual void delete_variable(const VariableIndex &variable) = 0;
	virtual bool is_variable_active(const VariableIndex &variable) const = 0;

	virtual ConstraintIndex add_linear_constraint(const ScalarAffineFunction function,
	                                              ConstraintSense sense, CoeffT rhs) = 0;
	virtual ConstraintIndex add_quadratic_constraint(const ScalarQuadraticFunction function,
	                                                 ConstraintSense sense, CoeffT rhs) = 0;
	virtual ConstraintIndex add_sos1_constraint(const Vector<VariableIndex> &variables,
	                                            const Vector<CoeffT> &weights) = 0;
	virtual ConstraintIndex add_sos2_constraint(const Vector<VariableIndex> &variables,
	                                            const Vector<CoeffT> &weights) = 0;

	virtual void delete_constraint(const ConstraintIndex &constraint) = 0;
	virtual bool is_constraint_active(const ConstraintIndex &constraint) const = 0;

	virtual void set_objective(const ScalarAffineFunction &function, ObjectiveSense sense) = 0;
	virtual void set_objective(const ScalarQuadraticFunction &function, ObjectiveSense sense) = 0;

	virtual void optimize() = 0;

	virtual void set_variable_raw_attribute_int(const VariableIndex &variable,
	                                            const char *attr_name, int value) = 0;
	virtual void set_variable_raw_attribute_char(const VariableIndex &variable,
	                                             const char *attr_name, char value) = 0;
	virtual void set_variable_raw_attribute_double(const VariableIndex &variable,
	                                               const char *attr_name, double value) = 0;
	virtual void set_variable_raw_attribute_string(const VariableIndex &variable,
	                                               const char *attr_name, const char *value) = 0;
	virtual int get_variable_raw_attribute_int(const VariableIndex &variable,
	                                           const char *attr_name) = 0;
	virtual char get_variable_raw_attribute_char(const VariableIndex &variable,
	                                             const char *attr_name) = 0;
	virtual double get_variable_raw_attribute_double(const VariableIndex &variable,
	                                                 const char *attr_name) = 0;
	virtual char *get_variable_raw_attribute_string(const VariableIndex &variable,
	                                                const char *attr_name) = 0;
};
