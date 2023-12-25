// #include <optional>
//
// #include "pyoptinterface/core.hpp"
// #include "pyoptinterface/container.hpp"
//
// struct VariableInfo
//{
//	VariableDomain domain;
// };
//
// class CacheModel
//{
//   public:
//	VariableIndex add_variable(VariableDomain domain = VariableDomain::Continuous);
//	void delete_variable(const VariableIndex &variable);
//	bool is_variable_active(const VariableIndex &variable) const;
//
//	ConstraintIndex add_linear_constraint(const LinearConstraint &constraint);
//	ConstraintIndex add_quadratic_constraint(const QuadraticConstraint &constraint);
//	ConstraintIndex add_sos1_constraint(const SOSConstraint &constraint);
//	ConstraintIndex add_sos2_constraint(const SOSConstraint &constraint);
//
//	void delete_constraint(const ConstraintIndex &constraint);
//
//	bool is_constraint_active(const ConstraintIndex &constraint) const;
//
//   private:
//	int num_variable = 0;
//	MonotoneVector<int> m_variable_index;
//	Vector<std::optional<VariableInfo>> m_variables;
//
//	int num_linear_constraint = 0;
//	MonotoneVector<int> m_linear_constraint_index;
//	Vector<std::optional<LinearConstraint>> m_linear_constraints;
//
//	int num_quadratic_constraint = 0;
//	MonotoneVector<int> m_quadratic_constraint_index;
//	Vector<std::optional<QuadraticConstraint>> m_quadratic_constraints;
//
//	int num_sos1_constraint = 0;
//	MonotoneVector<int> m_sos1_constraint_index;
//	Vector<std::optional<SOSConstraint>> m_sos1_constraints;
//
//	int num_sos2_constraint = 0;
//	MonotoneVector<int> m_sos2_constraint_index;
//	Vector<std::optional<SOSConstraint>> m_sos2_constraints;
// };
//
// VariableIndex CacheModel::add_variable(VariableDomain domain)
//{
//	IndexT index = m_variable_index.add_index();
//	m_variables.push_back(VariableInfo{domain});
//	num_variable++;
//	return VariableIndex(index);
// }
//
// void CacheModel::delete_variable(const VariableIndex &variable)
//{
//	m_variable_index.delete_index(variable.index);
//	m_variables[variable.index] = std::nullopt;
//	num_variable--;
// }
//
// bool CacheModel::is_variable_active(const VariableIndex &variable) const
//{
//	return m_variables[variable.index].has_value();
// }
//
// ConstraintIndex CacheModel::add_linear_constraint(const LinearConstraint &constraint)
//{
//	IndexT index = m_linear_constraint_index.add_index();
//	m_linear_constraints.push_back(constraint);
//	num_linear_constraint++;
//	return ConstraintIndex{ConstraintType::Linear, index};
// }
//
// ConstraintIndex CacheModel::add_quadratic_constraint(const QuadraticConstraint &constraint)
//{
//	IndexT index = m_quadratic_constraint_index.add_index();
//	m_quadratic_constraints.push_back(constraint);
//	num_quadratic_constraint++;
//	return ConstraintIndex{ConstraintType::Quadratic, index};
// }
//
// ConstraintIndex CacheModel::add_sos1_constraint(const SOSConstraint &constraint)
//{
//	IndexT index = m_sos1_constraint_index.add_index();
//	m_sos1_constraints.push_back(constraint);
//	num_sos1_constraint++;
//	return ConstraintIndex{ConstraintType::SOS1, index};
// }
//
// ConstraintIndex CacheModel::add_sos2_constraint(const SOSConstraint &constraint)
//{
//	IndexT index = m_sos2_constraint_index.add_index();
//	m_sos2_constraints.push_back(constraint);
//	num_sos2_constraint++;
//	return ConstraintIndex{ConstraintType::SOS2, index};
// }
//
// void CacheModel::delete_constraint(const ConstraintIndex &constraint)
//{
//	auto index = constraint.index;
//	switch (constraint.type)
//	{
//	case ConstraintType::Linear:
//		m_linear_constraint_index.delete_index(index);
//		m_linear_constraints[index] = std::nullopt;
//		num_linear_constraint--;
//		break;
//	case ConstraintType::Quadratic:
//		m_quadratic_constraint_index.delete_index(index);
//		m_quadratic_constraints[index] = std::nullopt;
//		num_quadratic_constraint--;
//		break;
//	case ConstraintType::SOS1:
//		m_sos1_constraint_index.delete_index(index);
//		m_sos1_constraints[index] = std::nullopt;
//		num_sos1_constraint--;
//		break;
//	case ConstraintType::SOS2:
//		m_sos2_constraint_index.delete_index(index);
//		m_sos2_constraints[index] = std::nullopt;
//		num_sos2_constraint--;
//		break;
//	}
// }
//
// bool CacheModel::is_constraint_active(const ConstraintIndex &constraint) const
//{
//	auto index = constraint.index;
//	switch (constraint.type)
//	{
//	case ConstraintType::Linear:
//		return m_linear_constraint_index.has_index(index);
//	case ConstraintType::Quadratic:
//		return m_quadratic_constraint_index.has_index(index);
//	case ConstraintType::SOS1:
//		return m_sos1_constraint_index.has_index(index);
//	case ConstraintType::SOS2:
//		return m_sos2_constraint_index.has_index(index);
//	}
// }
