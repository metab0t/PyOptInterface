#include "pyoptinterface/copt_model.hpp"
#include "fmt/core.h"

#include <stack>

namespace copt
{
#define B DYLIB_DECLARE
APILIST
#undef B

static DynamicLibrary lib;
static bool is_loaded = false;

bool is_library_loaded()
{
	return is_loaded;
}

bool load_library(const std::string &path)
{
	bool success = lib.try_load(path.c_str());
	if (!success)
	{
		return false;
	}

	DYLIB_LOAD_INIT;

#define B DYLIB_LOAD_FUNCTION
	APILIST
#undef B

	if (IS_DYLIB_LOAD_SUCCESS)
	{
#define B DYLIB_SAVE_FUNCTION
		APILIST
#undef B
		is_loaded = true;
		return true;
	}
	else
	{
		return false;
	}
}
} // namespace copt

static void check_error(int error)
{
	if (error)
	{
		char errmsg[COPT_BUFFSIZE];

		copt::COPT_GetRetcodeMsg(error, errmsg, COPT_BUFFSIZE);
		throw std::runtime_error(errmsg);
	}
}

static char copt_con_sense(ConstraintSense sense)
{
	switch (sense)
	{
	case ConstraintSense::LessEqual:
		return COPT_LESS_EQUAL;
	case ConstraintSense::Equal:
		return COPT_EQUAL;
	case ConstraintSense::GreaterEqual:
		return COPT_GREATER_EQUAL;
	default:
		throw std::runtime_error("Unknown constraint sense");
	}
}

static int copt_obj_sense(ObjectiveSense sense)
{
	switch (sense)
	{
	case ObjectiveSense::Minimize:
		return COPT_MINIMIZE;
	case ObjectiveSense::Maximize:
		return COPT_MAXIMIZE;
	default:
		throw std::runtime_error("Unknown objective sense");
	}
}

static char copt_vtype(VariableDomain domain)
{
	switch (domain)
	{
	case VariableDomain::Continuous:
		return COPT_CONTINUOUS;
	case VariableDomain::Integer:
		return COPT_INTEGER;
	case VariableDomain::Binary:
		return COPT_BINARY;
	default:
		throw std::runtime_error("Unknown variable domain");
	}
}

static VariableDomain copt_vtype_to_domain(char vtype)
{
	switch (vtype)
	{
	case COPT_CONTINUOUS:
		return VariableDomain::Continuous;
	case COPT_INTEGER:
		return VariableDomain::Integer;
	case COPT_BINARY:
		return VariableDomain::Binary;
	default:
		throw std::runtime_error("Unknown variable domain");
	}
}

static int copt_sostype(SOSType type)
{
	switch (type)
	{
	case SOSType::SOS1:
		return COPT_SOS_TYPE1;
	case SOSType::SOS2:
		return COPT_SOS_TYPE2;
	default:
		throw std::runtime_error("Unknown SOS type");
	}
}

COPTModel::COPTModel(const COPTEnv &env)
{
	init(env);
}

void COPTModel::init(const COPTEnv &env)
{
	if (!copt::is_library_loaded())
	{
		throw std::runtime_error("COPT library is not loaded");
	}
	copt_prob *model;
	int error = copt::COPT_CreateProb(env.m_env, &model);
	check_error(error);
	m_model = std::unique_ptr<copt_prob, COPTfreemodelT>(model);
}

void COPTModel::close()
{
	m_model.reset();
}

void COPTModel::write(const std::string &filename)
{
	int error;
	if (filename.ends_with(".mps"))
	{
		error = copt::COPT_WriteMps(m_model.get(), filename.c_str());
	}
	else if (filename.ends_with(".lp"))
	{
		error = copt::COPT_WriteLp(m_model.get(), filename.c_str());
	}
	else if (filename.ends_with(".cbf"))
	{
		error = copt::COPT_WriteCbf(m_model.get(), filename.c_str());
	}
	else if (filename.ends_with(".bin"))
	{
		error = copt::COPT_WriteBin(m_model.get(), filename.c_str());
	}
	else if (filename.ends_with(".bas"))
	{
		error = copt::COPT_WriteBasis(m_model.get(), filename.c_str());
	}
	else if (filename.ends_with(".sol"))
	{
		error = copt::COPT_WriteSol(m_model.get(), filename.c_str());
	}
	else if (filename.ends_with(".mst"))
	{
		error = copt::COPT_WriteMst(m_model.get(), filename.c_str());
	}
	else if (filename.ends_with(".par"))
	{
		error = copt::COPT_WriteParam(m_model.get(), filename.c_str());
	}
	else
	{
		throw std::runtime_error("Unknown file extension");
	}
	check_error(error);
}

VariableIndex COPTModel::add_variable(VariableDomain domain, double lb, double ub, const char *name)
{
	if (name != nullptr && name[0] == '\0')
	{
		name = nullptr;
	}
	IndexT index = m_variable_index.add_index();
	VariableIndex variable(index);

	char vtype = copt_vtype(domain);
	int error = copt::COPT_AddCol(m_model.get(), 0.0, 0, NULL, NULL, vtype, lb, ub, name);
	check_error(error);

	return variable;
}

void COPTModel::delete_variable(const VariableIndex &variable)
{
	if (!is_variable_active(variable))
	{
		throw std::runtime_error("Variable does not exist");
	}

	int variable_column = _variable_index(variable);
	int error = copt::COPT_DelCols(m_model.get(), 1, &variable_column);
	check_error(error);

	m_variable_index.delete_index(variable.index);
}

void COPTModel::delete_variables(const Vector<VariableIndex> &variables)
{
	int n_variables = variables.size();
	if (n_variables == 0)
		return;

	std::vector<int> columns;
	columns.reserve(n_variables);
	for (int i = 0; i < n_variables; i++)
	{
		if (!is_variable_active(variables[i]))
		{
			continue;
		}
		auto column = _variable_index(variables[i]);
		columns.push_back(column);
	}

	int error = copt::COPT_DelCols(m_model.get(), columns.size(), columns.data());
	check_error(error);

	for (int i = 0; i < n_variables; i++)
	{
		m_variable_index.delete_index(variables[i].index);
	}
}

bool COPTModel::is_variable_active(const VariableIndex &variable)
{
	return m_variable_index.has_index(variable.index);
}

double COPTModel::get_variable_value(const VariableIndex &variable)
{
	return get_variable_info(variable, COPT_DBLINFO_VALUE);
}

std::string COPTModel::pprint_variable(const VariableIndex &variable)
{
	return get_variable_name(variable);
}

void COPTModel::set_variable_bounds(const VariableIndex &variable, double lb, double ub)
{
	auto column = _checked_variable_index(variable);
	int error;
	error = copt::COPT_SetColLower(m_model.get(), 1, &column, &lb);
	check_error(error);
	error = copt::COPT_SetColUpper(m_model.get(), 1, &column, &ub);
	check_error(error);
}

ConstraintIndex COPTModel::add_linear_constraint(const ScalarAffineFunction &function,
                                                 ConstraintSense sense, CoeffT rhs,
                                                 const char *name)
{
	IndexT index = m_linear_constraint_index.add_index();
	ConstraintIndex constraint_index(ConstraintType::Linear, index);

	AffineFunctionPtrForm<int, int, double> ptr_form;
	ptr_form.make(this, function);

	int numnz = ptr_form.numnz;
	int *cind = ptr_form.index;
	double *cval = ptr_form.value;
	char g_sense = copt_con_sense(sense);
	double g_rhs = rhs - function.constant.value_or(0.0);

	if (name != nullptr && name[0] == '\0')
	{
		name = nullptr;
	}

	int error = copt::COPT_AddRow(m_model.get(), numnz, cind, cval, g_sense, g_rhs, g_rhs, name);
	check_error(error);
	return constraint_index;
}

ConstraintIndex COPTModel::add_linear_constraint(const ScalarAffineFunction &function,
                                                 const std::tuple<double, double> &interval,
                                                 const char *name)
{
	auto lb = std::get<0>(interval);
	auto ub = std::get<1>(interval);

	if (function.constant.has_value())
	{
		lb -= function.constant.value_or(0.0);
		ub -= function.constant.value_or(0.0);
	}

	IndexT index = m_linear_constraint_index.add_index();
	ConstraintIndex constraint_index(ConstraintType::Linear, index);

	AffineFunctionPtrForm<int, int, double> ptr_form;
	ptr_form.make(this, function);

	int numnz = ptr_form.numnz;
	int *cind = ptr_form.index;
	double *cval = ptr_form.value;

	if (name != nullptr && name[0] == '\0')
	{
		name = nullptr;
	}

	int error = copt::COPT_AddRow(m_model.get(), numnz, cind, cval, 0, lb, ub, name);
	check_error(error);
	return constraint_index;

	return ConstraintIndex();
}

ConstraintIndex COPTModel::add_quadratic_constraint(const ScalarQuadraticFunction &function,
                                                    ConstraintSense sense, CoeffT rhs,
                                                    const char *name)
{
	IndexT index = m_quadratic_constraint_index.add_index();
	ConstraintIndex constraint_index(ConstraintType::Quadratic, index);

	const auto &affine_part = function.affine_part;

	int numlnz = 0;
	int *lind = NULL;
	double *lval = NULL;
	AffineFunctionPtrForm<int, int, double> affine_ptr_form;
	if (affine_part.has_value())
	{
		const auto &affine_function = affine_part.value();
		affine_ptr_form.make(this, affine_function);
		numlnz = affine_ptr_form.numnz;
		lind = affine_ptr_form.index;
		lval = affine_ptr_form.value;
	}

	QuadraticFunctionPtrForm<int, int, double> ptr_form;
	ptr_form.make(this, function);
	int numqnz = ptr_form.numnz;
	int *qrow = ptr_form.row;
	int *qcol = ptr_form.col;
	double *qval = ptr_form.value;

	char g_sense = copt_con_sense(sense);
	double g_rhs = rhs;
	if (affine_part)
		g_rhs -= affine_part->constant.value_or(0.0);

	if (name != nullptr && name[0] == '\0')
	{
		name = nullptr;
	}

	int error = copt::COPT_AddQConstr(m_model.get(), numlnz, lind, lval, numqnz, qrow, qcol, qval,
	                                  g_sense, g_rhs, name);
	check_error(error);
	return constraint_index;
}

ConstraintIndex COPTModel::add_sos_constraint(const Vector<VariableIndex> &variables,
                                              SOSType sos_type)
{
	Vector<CoeffT> weights(variables.size(), 1.0);
	return add_sos_constraint(variables, sos_type, weights);
}

ConstraintIndex COPTModel::add_sos_constraint(const Vector<VariableIndex> &variables,
                                              SOSType sos_type, const Vector<CoeffT> &weights)
{
	IndexT index = m_sos_constraint_index.add_index();
	ConstraintIndex constraint_index(ConstraintType::SOS, index);

	int numsos = 1;
	int nummembers = variables.size();
	int types = copt_sostype(sos_type);
	int beg[] = {0};
	int cnt[] = {nummembers};
	std::vector<int> ind_v(nummembers);
	for (int i = 0; i < nummembers; i++)
	{
		ind_v[i] = _variable_index(variables[i].index);
	}
	int *ind = ind_v.data();
	double *weight = (double *)weights.data();

	int error = copt::COPT_AddSOSs(m_model.get(), numsos, &types, beg, cnt, ind, weight);
	check_error(error);
	return constraint_index;
}

ConstraintIndex COPTModel::add_second_order_cone_constraint(const Vector<VariableIndex> &variables,
                                                            const char *name, bool rotated)
{
	IndexT index = m_cone_constraint_index.add_index();
	ConstraintIndex constraint_index(ConstraintType::Cone, index);

	int N = variables.size();
	std::vector<int> ind_v(N);
	for (int i = 0; i < N; i++)
	{
		ind_v[i] = _checked_variable_index(variables[i]);
	}

	int cone = COPT_CONE_QUAD;
	if (rotated)
	{
		cone = COPT_CONE_RQUAD;
	}

	int coneType[] = {cone};
	int coneBeg[] = {0};
	int coneCnt[] = {N};
	int *coneIdx = ind_v.data();

	int error = copt::COPT_AddCones(m_model.get(), 1, coneType, coneBeg, coneCnt, coneIdx);
	check_error(error);

	// COPT does not support name for cone constraints

	return constraint_index;
}

ConstraintIndex COPTModel::add_exp_cone_constraint(const Vector<VariableIndex> &variables,
                                                   const char *name, bool dual)
{
	IndexT index = m_exp_cone_constraint_index.add_index();
	ConstraintIndex constraint_index(ConstraintType::COPT_ExpCone, index);

	int N = variables.size();
	if (N != 3)
	{
		throw std::runtime_error("Exponential cone constraint must have 3 variables");
	}

	std::vector<int> ind_v(N);
	for (int i = 0; i < N; i++)
	{
		ind_v[i] = _checked_variable_index(variables[i]);
	}

	int cone = COPT_EXPCONE_PRIMAL;
	if (dual)
	{
		cone = COPT_EXPCONE_DUAL;
	}

	int coneType[] = {cone};
	int coneBeg[] = {0};
	int coneCnt[] = {N};
	int *coneIdx = ind_v.data();

	int error = copt::COPT_AddExpCones(m_model.get(), 1, coneType, coneIdx);
	check_error(error);

	// COPT does not support name for cone constraints

	return constraint_index;
}

int unary_opcode(const UnaryOperator &op)
{
	switch (op)
	{
	case UnaryOperator::Neg:
		return COPT_NL_NEG;
	case UnaryOperator::Sin:
		return COPT_NL_SIN;
	case UnaryOperator::Cos:
		return COPT_NL_COS;
	case UnaryOperator::Tan:
		return COPT_NL_TAN;
	case UnaryOperator::Asin:
		return COPT_NL_ASIN;
	case UnaryOperator::Acos:
		return COPT_NL_ACOS;
	case UnaryOperator::Atan:
		return COPT_NL_ATAN;
	case UnaryOperator::Abs:
		return COPT_NL_ABS;
	case UnaryOperator::Sqrt:
		return COPT_NL_SQRT;
	case UnaryOperator::Exp:
		return COPT_NL_EXP;
	case UnaryOperator::Log:
		return COPT_NL_LOG;
	case UnaryOperator::Log10:
		return COPT_NL_LOG10;
	default: {
		auto opname = unary_operator_to_string(op);
		auto msg = fmt::format("Unknown unary operator for COPT: {}", opname);
		throw std::runtime_error(msg);
	}
	}
}

int binary_opcode(const BinaryOperator &op)
{
	switch (op)
	{
	case BinaryOperator::Sub:
		return COPT_NL_MINUS;
	case BinaryOperator::Div:
		return COPT_NL_DIV;
	case BinaryOperator::Pow:
		return COPT_NL_POW;
	case BinaryOperator::Mul2:
		return COPT_NL_MULT;
	default: {
		auto opname = binary_operator_to_string(op);
		auto msg = fmt::format("Unknown binary operator for COPT: {}", opname);
		throw std::runtime_error(msg);
	}
	}
}

int nary_opcode(const NaryOperator &op)
{
	switch (op)
	{
	case NaryOperator::Add:
		return COPT_NL_SUM;
	default: {
		auto opname = nary_operator_to_string(op);
		auto msg = fmt::format("Unknown n-ary operator for COPT: {}", opname);
		throw std::runtime_error(msg);
	}
	}
}

void COPTModel::decode_expr(const ExpressionGraph &graph, const ExpressionHandle &expr,
                            std::vector<int> &opcodes, std::vector<double> &constants)
{
	auto array_type = expr.array;
	auto index = expr.id;
	switch (array_type)
	{
	case ArrayType::Constant: {
		opcodes.push_back(COPT_NL_GET);
		constants.push_back(graph.m_constants[index]);
		break;
	}
	case ArrayType::Variable: {
		auto column = _checked_variable_index(graph.m_variables[index]);
		opcodes.push_back(column);
		break;
	}
	case ArrayType::Parameter: {
		throw std::runtime_error("Parameter is not supported in COPT");
		break;
	}
	case ArrayType::Unary: {
		auto &unary = graph.m_unaries[index];
		int opcode = unary_opcode(unary.op);
		opcodes.push_back(opcode);
		break;
	}
	case ArrayType::Binary: {
		auto &binary = graph.m_binaries[index];
		int opcode = binary_opcode(binary.op);
		opcodes.push_back(opcode);
		break;
	}
	case ArrayType::Ternary: {
		throw std::runtime_error("Ternary operator is not supported in COPT");
		break;
	}
	case ArrayType::Nary: {
		auto &nary = graph.m_naries[index];
		int opcode = nary_opcode(nary.op);
		int n_operands = nary.operands.size();

		if (opcode == COPT_NL_SUM && n_operands == 1)
		{
			// COPT errors when sum 1 operand
		}
		else
		{
			opcodes.push_back(opcode);
			opcodes.push_back(n_operands);
		}
	}
	break;
	}
}

ConstraintIndex COPTModel::add_single_nl_constraint(ExpressionGraph &graph,
                                                    const ExpressionHandle &result, double lb,
                                                    double ub, const char *name)
{
	std::vector<int> opcodes;
	std::vector<double> constants;

	std::stack<ExpressionHandle> expr_stack;

	// init stack
	expr_stack.push(result);

	while (!expr_stack.empty())
	{
		auto expr = expr_stack.top();
		expr_stack.pop();

		// We need to convert the n-arg multiplication to 2-arg multiplication
		if (expr.array == ArrayType::Nary && graph.m_naries[expr.id].op == NaryOperator::Mul)
		{
			auto &nary = graph.m_naries[expr.id];
			int n_operands = nary.operands.size();

			if (n_operands == 1)
			{
				expr = nary.operands[0];
			}
			else if (n_operands >= 2)
			{
				ExpressionHandle left = nary.operands[0];
				ExpressionHandle right = nary.operands[1];
				ExpressionHandle new_expr = graph.add_binary(BinaryOperator::Mul2, left, right);
				for (int i = 2; i < n_operands; i++)
				{
					new_expr = graph.add_binary(BinaryOperator::Mul2, new_expr, nary.operands[i]);
				}
				expr = new_expr;
			}
		}

		decode_expr(graph, expr, opcodes, constants);

		auto array_type = expr.array;
		auto index = expr.id;
		switch (array_type)
		{
		case ArrayType::Unary: {
			auto &unary = graph.m_unaries[index];
			expr_stack.push(unary.operand);
			break;
		}
		case ArrayType::Binary: {
			auto &binary = graph.m_binaries[index];
			expr_stack.push(binary.right);
			expr_stack.push(binary.left);
			break;
		}
		case ArrayType::Nary: {
			auto &nary = graph.m_naries[index];
			for (int i = nary.operands.size() - 1; i >= 0; i--)
			{
				expr_stack.push(nary.operands[i]);
			}
			break;
		}
		default:
			break;
		}
	}

	if (name != nullptr && name[0] == '\0')
	{
		name = nullptr;
	}

	// add NL constraint
	int error =
	    copt::COPT_AddNLConstr(m_model.get(), opcodes.size(), constants.size(), opcodes.data(),
	                           constants.data(), 0, nullptr, nullptr, 0, lb, ub, name);
	check_error(error);

	IndexT constraint_index = m_nl_constraint_index.add_index();

	ConstraintIndex constraint(ConstraintType::COPT_NL, constraint_index);

	return constraint;
}

ConstraintIndex COPTModel::add_single_nl_constraint_from_comparison(ExpressionGraph &graph,
                                                                    const ExpressionHandle &expr,
                                                                    const char *name)
{
	ExpressionHandle real_expr;
	double lb = -COPT_INFINITY, ub = COPT_INFINITY;

	unpack_comparison_expression(graph, expr, real_expr, lb, ub);

	auto constraint = add_single_nl_constraint(graph, real_expr, lb, ub, name);
	return constraint;
}

void COPTModel::delete_constraint(const ConstraintIndex &constraint)
{
	int error = 0;
	int constraint_row = _constraint_index(constraint);
	if (constraint_row >= 0)
	{
		switch (constraint.type)
		{
		case ConstraintType::Linear:
			m_linear_constraint_index.delete_index(constraint.index);
			error = copt::COPT_DelRows(m_model.get(), 1, &constraint_row);
			break;
		case ConstraintType::Quadratic:
			m_quadratic_constraint_index.delete_index(constraint.index);
			error = copt::COPT_DelQConstrs(m_model.get(), 1, &constraint_row);
			break;
		case ConstraintType::SOS:
			m_sos_constraint_index.delete_index(constraint.index);
			error = copt::COPT_DelSOSs(m_model.get(), 1, &constraint_row);
			break;
		case ConstraintType::Cone:
			m_cone_constraint_index.delete_index(constraint.index);
			error = copt::COPT_DelCones(m_model.get(), 1, &constraint_row);
			break;
		case ConstraintType::COPT_ExpCone:
			m_exp_cone_constraint_index.delete_index(constraint.index);
			error = copt::COPT_DelExpCones(m_model.get(), 1, &constraint_row);
			break;
		default:
			throw std::runtime_error("Unknown constraint type");
		}
	}
	check_error(error);
}

bool COPTModel::is_constraint_active(const ConstraintIndex &constraint)
{
	switch (constraint.type)
	{
	case ConstraintType::Linear:
		return m_linear_constraint_index.has_index(constraint.index);
	case ConstraintType::Quadratic:
		return m_quadratic_constraint_index.has_index(constraint.index);
	case ConstraintType::SOS:
		return m_sos_constraint_index.has_index(constraint.index);
	case ConstraintType::Cone:
		return m_cone_constraint_index.has_index(constraint.index);
	case ConstraintType::COPT_ExpCone:
		return m_exp_cone_constraint_index.has_index(constraint.index);
	default:
		throw std::runtime_error("Unknown constraint type");
	}
}

void COPTModel::_set_affine_objective(const ScalarAffineFunction &function, ObjectiveSense sense,
                                      bool clear_quadratic)
{
	int error = 0;
	if (clear_quadratic)
	{
		// First delete all quadratic terms
		error = copt::COPT_DelQuadObj(m_model.get());
		check_error(error);
	}

	// Set Obj attribute of each variable
	int n_variables = get_raw_attribute_int(COPT_INTATTR_COLS);
	std::vector<int> ind_v(n_variables);
	for (int i = 0; i < n_variables; i++)
	{
		ind_v[i] = i;
	}
	std::vector<double> obj_v(n_variables, 0.0);

	int numnz = function.size();
	for (int i = 0; i < numnz; i++)
	{
		auto column = _variable_index(function.variables[i]);
		if (column < 0)
		{
			throw std::runtime_error("Variable does not exist");
		}
		obj_v[column] = function.coefficients[i];
	}

	error = copt::COPT_ReplaceColObj(m_model.get(), n_variables, ind_v.data(), obj_v.data());
	check_error(error);
	error = copt::COPT_SetObjConst(m_model.get(), function.constant.value_or(0.0));
	check_error(error);

	int obj_sense = copt_obj_sense(sense);
	error = copt::COPT_SetObjSense(m_model.get(), obj_sense);
	check_error(error);
}

void COPTModel::set_objective(const ScalarAffineFunction &function, ObjectiveSense sense)
{
	_set_affine_objective(function, sense, true);
}

void COPTModel::set_objective(const ScalarQuadraticFunction &function, ObjectiveSense sense)
{
	int error = 0;
	// First delete all quadratic terms
	error = copt::COPT_DelQuadObj(m_model.get());
	check_error(error);

	// Add quadratic term
	int numqnz = function.size();
	if (numqnz > 0)
	{
		QuadraticFunctionPtrForm<int, int, double> ptr_form;
		ptr_form.make(this, function);
		int numqnz = ptr_form.numnz;
		int *qrow = ptr_form.row;
		int *qcol = ptr_form.col;
		double *qval = ptr_form.value;

		error = copt::COPT_SetQuadObj(m_model.get(), numqnz, qrow, qcol, qval);
		check_error(error);
	}

	// Affine part
	const auto &affine_part = function.affine_part;
	if (affine_part)
	{
		const auto &affine_function = affine_part.value();
		_set_affine_objective(affine_function, sense, false);
	}
	else
	{
		ScalarAffineFunction zero;
		_set_affine_objective(zero, sense, false);
	}
}

void COPTModel::set_objective(const ExprBuilder &function, ObjectiveSense sense)
{
	auto deg = function.degree();
	if (deg <= 1)
	{
		ScalarAffineFunction f(function);
		set_objective(f, sense);
	}
	else if (deg == 2)
	{
		ScalarQuadraticFunction f(function);
		set_objective(f, sense);
	}
	else
	{
		throw std::runtime_error("Objective must be linear or quadratic");
	}
}

void COPTModel::optimize()
{
	if (has_callback)
	{
		// Store the number of variables for the callback
		m_callback_userdata.n_variables = get_raw_attribute_int("Cols");
	}
	int error = copt::COPT_Solve(m_model.get());
	check_error(error);
}

int COPTModel::raw_parameter_attribute_type(const char *name)
{
	int retval;
	int error = copt::COPT_SearchParamAttr(m_model.get(), name, &retval);
	check_error(error);
	return retval;
}

void COPTModel::set_raw_parameter_int(const char *param_name, int value)
{
	int error = copt::COPT_SetIntParam(m_model.get(), param_name, value);
	check_error(error);
}

void COPTModel::set_raw_parameter_double(const char *param_name, double value)
{
	int error = copt::COPT_SetDblParam(m_model.get(), param_name, value);
	check_error(error);
}

int COPTModel::get_raw_parameter_int(const char *param_name)
{
	int retval;
	int error = copt::COPT_GetIntParam(m_model.get(), param_name, &retval);
	check_error(error);
	return retval;
}

double COPTModel::get_raw_parameter_double(const char *param_name)
{
	double retval;
	int error = copt::COPT_GetDblParam(m_model.get(), param_name, &retval);
	check_error(error);
	return retval;
}

int COPTModel::get_raw_attribute_int(const char *attr_name)
{
	int retval;
	int error = copt::COPT_GetIntAttr(m_model.get(), attr_name, &retval);
	check_error(error);
	return retval;
}

double COPTModel::get_raw_attribute_double(const char *attr_name)
{
	double retval;
	int error = copt::COPT_GetDblAttr(m_model.get(), attr_name, &retval);
	check_error(error);
	return retval;
}

double COPTModel::get_variable_info(const VariableIndex &variable, const char *info_name)
{
	auto column = _checked_variable_index(variable);
	double retval;
	int error = copt::COPT_GetColInfo(m_model.get(), info_name, 1, &column, &retval);
	check_error(error);
	return retval;
}

std::string COPTModel::get_variable_name(const VariableIndex &variable)
{
	auto column = _checked_variable_index(variable);
	int error;
	int reqsize;
	error = copt::COPT_GetColName(m_model.get(), column, NULL, 0, &reqsize);
	check_error(error);
	std::string retval(reqsize - 1, '\0');
	error = copt::COPT_GetColName(m_model.get(), column, retval.data(), reqsize, &reqsize);
	check_error(error);
	return retval;
}

void COPTModel::set_variable_name(const VariableIndex &variable, const char *name)
{
	auto column = _checked_variable_index(variable);
	const char *names[] = {name};
	int error = copt::COPT_SetColNames(m_model.get(), 1, &column, names);
	check_error(error);
}

VariableDomain COPTModel::get_variable_type(const VariableIndex &variable)
{
	auto column = _checked_variable_index(variable);
	char vtype;
	int error = copt::COPT_GetColType(m_model.get(), 1, &column, &vtype);
	check_error(error);
	return copt_vtype_to_domain(vtype);
}

void COPTModel::set_variable_type(const VariableIndex &variable, VariableDomain domain)
{
	char vtype = copt_vtype(domain);
	auto column = _checked_variable_index(variable);
	int error = copt::COPT_SetColType(m_model.get(), 1, &column, &vtype);
	check_error(error);
}

void COPTModel::set_variable_lower_bound(const VariableIndex &variable, double lb)
{
	auto column = _checked_variable_index(variable);
	int error = copt::COPT_SetColLower(m_model.get(), 1, &column, &lb);
	check_error(error);
}

void COPTModel::set_variable_upper_bound(const VariableIndex &variable, double ub)
{
	auto column = _checked_variable_index(variable);
	int error = copt::COPT_SetColUpper(m_model.get(), 1, &column, &ub);
	check_error(error);
}

double COPTModel::get_constraint_info(const ConstraintIndex &constraint, const char *info_name)
{
	int row = _checked_constraint_index(constraint);
	double retval;
	int num = 1;
	int error;
	switch (constraint.type)
	{
	case ConstraintType::Linear:
		error = copt::COPT_GetRowInfo(m_model.get(), info_name, num, &row, &retval);
		break;
	case ConstraintType::Quadratic:
		error = copt::COPT_GetQConstrInfo(m_model.get(), info_name, num, &row, &retval);
		break;
	default:
		throw std::runtime_error("Unknown constraint type");
	}
	check_error(error);
	return retval;
}

std::string COPTModel::get_constraint_name(const ConstraintIndex &constraint)
{
	int row = _checked_constraint_index(constraint);
	int error;
	int reqsize;
	switch (constraint.type)
	{
	case ConstraintType::Linear:
		error = copt::COPT_GetRowName(m_model.get(), row, NULL, 0, &reqsize);
		break;
	case ConstraintType::Quadratic:
		error = copt::COPT_GetQConstrName(m_model.get(), row, NULL, 0, &reqsize);
		break;
	default:
		throw std::runtime_error("Unknown constraint type");
	}
	check_error(error);
	std::string retval(reqsize - 1, '\0');
	switch (constraint.type)
	{
	case ConstraintType::Linear:
		error = copt::COPT_GetRowName(m_model.get(), row, retval.data(), reqsize, &reqsize);
		break;
	case ConstraintType::Quadratic:
		error = copt::COPT_GetQConstrName(m_model.get(), row, retval.data(), reqsize, &reqsize);
		break;
	}
	check_error(error);
	return retval;
}

void COPTModel::set_constraint_name(const ConstraintIndex &constraint, const char *name)
{
	int row = _checked_constraint_index(constraint);
	const char *names[] = {name};
	int error;
	switch (constraint.type)
	{
	case ConstraintType::Linear:
		error = copt::COPT_SetRowNames(m_model.get(), 1, &row, names);
		break;
	case ConstraintType::Quadratic:
		error = copt::COPT_SetQConstrNames(m_model.get(), 1, &row, names);
		break;
	default:
		throw std::runtime_error("Unknown constraint type");
	}
	check_error(error);
}

void COPTModel::set_obj_sense(ObjectiveSense sense)
{
	int obj_sense = copt_obj_sense(sense);
	int error = copt::COPT_SetObjSense(m_model.get(), obj_sense);
	check_error(error);
}

void COPTModel::add_mip_start(const Vector<VariableIndex> &variables, const Vector<double> &values)
{
	if (variables.size() != values.size())
	{
		throw std::runtime_error("Number of variables and values do not match");
	}
	int numnz = variables.size();
	if (numnz == 0)
		return;

	std::vector<int> ind_v(numnz);
	for (int i = 0; i < numnz; i++)
	{
		ind_v[i] = _variable_index(variables[i].index);
	}
	int *ind = ind_v.data();
	double *val = (double *)values.data();

	int error = copt::COPT_AddMipStart(m_model.get(), numnz, ind, val);
	check_error(error);
}

double COPTModel::get_normalized_rhs(const ConstraintIndex &constraint)
{
	int row = _checked_constraint_index(constraint);
	int num = 1;
	int error;
	switch (constraint.type)
	{
	case ConstraintType::Linear: {
		double lb, ub;
		error = copt::COPT_GetRowInfo(m_model.get(), COPT_DBLINFO_LB, num, &row, &lb);
		check_error(error);
		error = copt::COPT_GetRowInfo(m_model.get(), COPT_DBLINFO_UB, num, &row, &ub);
		check_error(error);

		bool lb_inf = lb < -COPT_INFINITY + 1.0;
		bool ub_inf = ub > COPT_INFINITY - 1.0;

		if (!lb_inf)
			return lb;
		if (!ub_inf)
			return ub;

		throw std::runtime_error("Constraint has no finite bound");
	}
	break;
	case ConstraintType::Quadratic: {
		double rhs;
		error = copt::COPT_GetQConstrRhs(m_model.get(), num, &row, &rhs);
		check_error(error);
		return rhs;
	}
	break;
	default:
		throw std::runtime_error("Unknown constraint type to get_normalized_rhs");
	}
}

void COPTModel::set_normalized_rhs(const ConstraintIndex &constraint, double value)
{
	int row = _checked_constraint_index(constraint);
	int num = 1;
	int error;
	switch (constraint.type)
	{
	case ConstraintType::Linear: {
		double lb, ub;

		error = copt::COPT_GetRowInfo(m_model.get(), COPT_DBLINFO_LB, num, &row, &lb);
		check_error(error);
		error = copt::COPT_GetRowInfo(m_model.get(), COPT_DBLINFO_UB, num, &row, &ub);
		check_error(error);

		bool lb_inf = lb < -COPT_INFINITY + 1.0;
		bool ub_inf = ub > COPT_INFINITY - 1.0;

		if (!lb_inf)
		{
			error = copt::COPT_SetRowLower(m_model.get(), num, &row, &value);
			check_error(error);
		}
		if (!ub_inf)
		{
			error = copt::COPT_SetRowUpper(m_model.get(), num, &row, &value);
			check_error(error);
		}

		if (lb_inf && ub_inf)
		{
			throw std::runtime_error("Constraint has no finite bound");
		}
	}
	break;
	case ConstraintType::Quadratic: {
		error = copt::COPT_SetQConstrRhs(m_model.get(), num, &row, &value);
		check_error(error);
	}
	break;
	default:
		throw std::runtime_error("Unknown constraint type to set_normalized_rhs");
	}
}

double COPTModel::get_normalized_coefficient(const ConstraintIndex &constraint,
                                             const VariableIndex &variable)
{
	if (constraint.type != ConstraintType::Linear)
	{
		throw std::runtime_error("Only linear constraint supports get_normalized_coefficient");
	}
	int row = _checked_constraint_index(constraint);
	int col = _checked_variable_index(variable);
	double retval;
	int error = copt::COPT_GetElem(m_model.get(), col, row, &retval);
	check_error(error);
	return retval;
}

void COPTModel::set_normalized_coefficient(const ConstraintIndex &constraint,
                                           const VariableIndex &variable, double value)
{
	if (constraint.type != ConstraintType::Linear)
	{
		throw std::runtime_error("Only linear constraint supports set_normalized_coefficient");
	}
	int row = _checked_constraint_index(constraint);
	int col = _checked_variable_index(variable);
	int error = copt::COPT_SetElem(m_model.get(), col, row, value);
	check_error(error);
}

double COPTModel::get_objective_coefficient(const VariableIndex &variable)
{
	return get_variable_info(variable, COPT_DBLINFO_OBJ);
}

void COPTModel::set_objective_coefficient(const VariableIndex &variable, double value)
{
	auto column = _checked_variable_index(variable);
	int error = copt::COPT_SetColObj(m_model.get(), 1, &column, &value);
	check_error(error);
}

int COPTModel::_variable_index(const VariableIndex &variable)
{
	return m_variable_index.get_index(variable.index);
}

int COPTModel::_checked_variable_index(const VariableIndex &variable)
{
	int column = _variable_index(variable);
	if (column < 0)
	{
		throw std::runtime_error("Variable does not exist");
	}
	return column;
}

int COPTModel::_constraint_index(const ConstraintIndex &constraint)
{
	switch (constraint.type)
	{
	case ConstraintType::Linear:
		return m_linear_constraint_index.get_index(constraint.index);
	case ConstraintType::Quadratic:
		return m_quadratic_constraint_index.get_index(constraint.index);
	case ConstraintType::SOS:
		return m_sos_constraint_index.get_index(constraint.index);
	case ConstraintType::Cone:
		return m_cone_constraint_index.get_index(constraint.index);
	case ConstraintType::COPT_ExpCone:
		return m_exp_cone_constraint_index.get_index(constraint.index);
	default:
		throw std::runtime_error("Unknown constraint type");
	}
}

int COPTModel::_checked_constraint_index(const ConstraintIndex &constraint)
{
	int row = _constraint_index(constraint);
	if (row < 0)
	{
		throw std::runtime_error("Constraint does not exist");
	}
	return row;
}

void *COPTModel::get_raw_model()
{
	return m_model.get();
}

std::string COPTModel::version_string()
{
	char buffer[COPT_BUFFSIZE];
	copt::COPT_GetBanner(buffer, COPT_BUFFSIZE);
	return buffer;
}

COPTEnv::COPTEnv()
{
	if (!copt::is_library_loaded())
	{
		throw std::runtime_error("COPT library is not loaded");
	}
	int error = copt::COPT_CreateEnv(&m_env);
	check_error(error);
}

COPTEnv::COPTEnv(COPTEnvConfig &config)
{
	if (!copt::is_library_loaded())
	{
		throw std::runtime_error("COPT library is not loaded");
	}
	int error = copt::COPT_CreateEnvWithConfig(config.m_config, &m_env);
	check_error(error);
}

COPTEnv::~COPTEnv()
{
	int error = copt::COPT_DeleteEnv(&m_env);
	check_error(error);
}

void COPTEnv::close()
{
	if (m_env != nullptr)
	{
		copt::COPT_DeleteEnv(&m_env);
	}
	m_env = nullptr;
}

COPTEnvConfig::COPTEnvConfig()
{
	if (!copt::is_library_loaded())
	{
		throw std::runtime_error("COPT library is not loaded");
	}
	int error = copt::COPT_CreateEnvConfig(&m_config);
	check_error(error);
}

COPTEnvConfig::~COPTEnvConfig()
{
	int error = copt::COPT_DeleteEnvConfig(&m_config);
	check_error(error);
}

void COPTEnvConfig::set(const char *param_name, const char *value)
{
	int error = copt::COPT_SetEnvConfig(m_config, param_name, value);
	check_error(error);
}

// Callback
int RealCOPTCallbackFunction(copt_prob *prob, void *cbdata, int cbctx, void *userdata)
{
	auto real_userdata = static_cast<COPTCallbackUserdata *>(userdata);
	auto model = static_cast<COPTModel *>(real_userdata->model);
	auto &callback = real_userdata->callback;

	model->m_cbdata = cbdata;
	model->m_callback_userdata.where = cbctx;
	model->m_callback_userdata.cb_get_mipsol_called = false;
	model->m_callback_userdata.cb_get_mipnoderel_called = false;
	model->m_callback_userdata.cb_get_mipincumbent_called = false;
	model->m_callback_userdata.cb_set_solution_called = false;
	model->m_callback_userdata.cb_requires_submit_solution = false;
	callback(model, cbctx);

	if (model->m_callback_userdata.cb_requires_submit_solution)
	{
		model->cb_submit_solution();
	}

	return COPT_RETCODE_OK;
}

void COPTModel::set_callback(const COPTCallback &callback, int cbctx)
{
	m_callback_userdata.model = this;
	m_callback_userdata.callback = callback;

	int error = copt::COPT_SetCallback(m_model.get(), RealCOPTCallbackFunction, cbctx,
	                                   &m_callback_userdata);
	check_error(error);

	has_callback = true;
}

int COPTModel::cb_get_info_int(const std::string &what)
{
	int retval;
	int error = copt::COPT_GetCallbackInfo(m_cbdata, what.c_str(), &retval);
	check_error(error);
	return retval;
}

double COPTModel::cb_get_info_double(const std::string &what)
{
	double retval;
	int error = copt::COPT_GetCallbackInfo(m_cbdata, what.c_str(), &retval);
	check_error(error);
	return retval;
}

void COPTModel::cb_get_info_doublearray(const std::string &what)
{
	int n_vars = m_callback_userdata.n_variables;
	double *val = nullptr;
	if (what == COPT_CBINFO_MIPCANDIDATE)
	{
		m_callback_userdata.mipsol.resize(n_vars);
		val = m_callback_userdata.mipsol.data();
	}
	else if (what == COPT_CBINFO_RELAXSOLUTION)
	{
		m_callback_userdata.mipnoderel.resize(n_vars);
		val = m_callback_userdata.mipnoderel.data();
	}
	else if (what == COPT_CBINFO_INCUMBENT)
	{
		m_callback_userdata.mipincumbent.resize(n_vars);
		val = m_callback_userdata.mipincumbent.data();
	}
	else
	{
		throw std::runtime_error("Invalid what for cb_get_info_doublearray");
	}
	int error = copt::COPT_GetCallbackInfo(m_cbdata, what.c_str(), val);
	check_error(error);
}

double COPTModel::cb_get_solution(const VariableIndex &variable)
{
	auto &userdata = m_callback_userdata;
	if (!userdata.cb_get_mipsol_called)
	{
		cb_get_info_doublearray(COPT_CBINFO_MIPCANDIDATE);
		userdata.cb_get_mipsol_called = true;
	}
	auto index = _variable_index(variable);
	return userdata.mipsol[index];
}

double COPTModel::cb_get_relaxation(const VariableIndex &variable)
{
	auto &userdata = m_callback_userdata;
	if (!userdata.cb_get_mipnoderel_called)
	{
		cb_get_info_doublearray(COPT_CBINFO_RELAXSOLUTION);
		userdata.cb_get_mipnoderel_called = true;
	}
	auto index = _variable_index(variable);
	return userdata.mipnoderel[index];
}

double COPTModel::cb_get_incumbent(const VariableIndex &variable)
{
	auto &userdata = m_callback_userdata;
	if (!userdata.cb_get_mipincumbent_called)
	{
		cb_get_info_doublearray(COPT_CBINFO_INCUMBENT);
		userdata.cb_get_mipincumbent_called = true;
	}
	auto index = _variable_index(variable);
	return userdata.mipincumbent[index];
}

void COPTModel::cb_set_solution(const VariableIndex &variable, double value)
{
	auto &userdata = m_callback_userdata;
	if (!userdata.cb_set_solution_called)
	{
		userdata.heuristic_solution.resize(userdata.n_variables, COPT_UNDEFINED);
		userdata.cb_set_solution_called = true;
	}
	userdata.heuristic_solution[_variable_index(variable)] = value;
	m_callback_userdata.cb_requires_submit_solution = true;
}

double COPTModel::cb_submit_solution()
{
	if (!m_callback_userdata.cb_set_solution_called)
	{
		throw std::runtime_error("No solution is set in the callback!");
	}
	double obj;
	int error = copt::COPT_AddCallbackSolution(m_cbdata,
	                                           m_callback_userdata.heuristic_solution.data(), &obj);
	check_error(error);
	m_callback_userdata.cb_requires_submit_solution = false;
	return obj;
}

void COPTModel::cb_exit()
{
	int error = copt::COPT_Interrupt(m_model.get());
	check_error(error);
}

void COPTModel::cb_add_lazy_constraint(const ScalarAffineFunction &function, ConstraintSense sense,
                                       CoeffT rhs)
{
	AffineFunctionPtrForm<int, int, double> ptr_form;
	ptr_form.make(this, function);

	int numnz = ptr_form.numnz;
	int *cind = ptr_form.index;
	double *cval = ptr_form.value;
	char g_sense = copt_con_sense(sense);
	double g_rhs = rhs - function.constant.value_or(0.0);

	int error = copt::COPT_AddCallbackLazyConstr(m_cbdata, numnz, cind, cval, g_sense, g_rhs);
	check_error(error);
}

void COPTModel::cb_add_lazy_constraint(const ExprBuilder &function, ConstraintSense sense,
                                       CoeffT rhs)
{
	ScalarAffineFunction f(function);
	cb_add_lazy_constraint(f, sense, rhs);
}

void COPTModel::cb_add_user_cut(const ScalarAffineFunction &function, ConstraintSense sense,
                                CoeffT rhs)
{
	AffineFunctionPtrForm<int, int, double> ptr_form;
	ptr_form.make(this, function);

	int numnz = ptr_form.numnz;
	int *cind = ptr_form.index;
	double *cval = ptr_form.value;
	char g_sense = copt_con_sense(sense);
	double g_rhs = rhs - function.constant.value_or(0.0);

	int error = copt::COPT_AddCallbackUserCut(m_cbdata, numnz, cind, cval, g_sense, g_rhs);
	check_error(error);
}

void COPTModel::cb_add_user_cut(const ExprBuilder &function, ConstraintSense sense, CoeffT rhs)
{
	ScalarAffineFunction f(function);
	cb_add_user_cut(f, sense, rhs);
}

void COPTModel::computeIIS()
{
	int error = copt::COPT_ComputeIIS(m_model.get());
	check_error(error);
}

int COPTModel::_get_variable_upperbound_IIS(const VariableIndex &variable)
{
	auto column = _checked_variable_index(variable);
	int retval;
	int error = copt::COPT_GetColUpperIIS(m_model.get(), 1, &column, &retval);
	check_error(error);
	return retval;
}

int COPTModel::_get_variable_lowerbound_IIS(const VariableIndex &variable)
{
	auto column = _checked_variable_index(variable);
	int retval;
	int error = copt::COPT_GetColLowerIIS(m_model.get(), 1, &column, &retval);
	check_error(error);
	return retval;
}

int COPTModel::_get_constraint_IIS(const ConstraintIndex &constraint)
{
	int row = _checked_constraint_index(constraint);
	int num = 1;
	int error;
	switch (constraint.type)
	{
	case ConstraintType::Linear: {
		int lb_iis, ub_iis;

		error = copt::COPT_GetRowLowerIIS(m_model.get(), num, &row, &lb_iis);
		check_error(error);

		error = copt::COPT_GetRowUpperIIS(m_model.get(), num, &row, &ub_iis);
		check_error(error);

		return lb_iis + ub_iis;
	}
	break;
	case ConstraintType::SOS: {
		int iis;
		error = copt::COPT_GetSOSIIS(m_model.get(), num, &row, &iis);
		check_error(error);
		return iis;
	}
	break;
	default:
		throw std::runtime_error("Unknown constraint type to get IIS state");
	}
}
