#pragma once

#include <deque>
#include <memory>
#include <optional>
#include <variant>

#include "solvers/knitro/knitro.h"

#include "pyoptinterface/core.hpp"
#include "pyoptinterface/container.hpp"
#include "pyoptinterface/cppad_interface.hpp"
#define USE_NLMIXIN
#include "pyoptinterface/solver_common.hpp"
#include "pyoptinterface/dylib.hpp"

// Define Knitro C APIs to be dynamically loaded
#define APILIST                         \
	B(KN_new);                          \
	B(KN_free);                         \
	B(KN_update);                       \
	B(KN_reset_params_to_defaults);     \
	B(KN_load_param_file);              \
	B(KN_save_param_file);              \
	B(KN_get_param_id);                 \
	B(KN_get_param_type);               \
	B(KN_set_int_param);                \
	B(KN_set_char_param);               \
	B(KN_set_double_param);             \
	B(KN_get_int_param);                \
	B(KN_get_double_param);             \
	B(KN_add_vars);                     \
	B(KN_add_var);                      \
	B(KN_add_cons);                     \
	B(KN_add_con);                      \
	B(KN_set_var_lobnd);                \
	B(KN_set_var_upbnd);                \
	B(KN_get_var_lobnd);                \
	B(KN_get_var_upbnd);                \
	B(KN_set_var_type);                 \
	B(KN_get_var_type);                 \
	B(KN_set_var_name);                 \
	B(KN_get_var_name);                 \
	B(KN_set_con_lobnd);                \
	B(KN_set_con_upbnd);                \
	B(KN_set_con_eqbnd);                \
	B(KN_get_con_lobnd);                \
	B(KN_get_con_upbnd);                \
	B(KN_get_con_eqbnd);                \
	B(KN_set_con_name);                 \
	B(KN_get_con_name);                 \
	B(KN_set_obj_goal);                 \
	B(KN_get_obj_goal);                 \
	B(KN_set_var_primal_init_value);    \
	B(KN_add_obj_constant);             \
	B(KN_del_obj_constant);             \
	B(KN_add_obj_linear_struct);        \
	B(KN_del_obj_linear_struct);        \
	B(KN_del_obj_linear_struct_all);    \
	B(KN_add_obj_quadratic_struct);     \
	B(KN_del_obj_quadratic_struct);     \
	B(KN_del_obj_quadratic_struct_all); \
	B(KN_add_con_constant);             \
	B(KN_add_con_linear_struct);        \
	B(KN_add_con_linear_struct_one);    \
	B(KN_add_con_linear_term);          \
	B(KN_add_con_quadratic_struct);     \
	B(KN_add_con_quadratic_struct_one); \
	B(KN_add_con_quadratic_term);       \
	B(KN_add_con_L2norm);               \
	B(KN_del_con_linear_struct);        \
	B(KN_del_con_quadratic_struct);     \
	B(KN_chg_con_linear_term);          \
	B(KN_add_eval_callback);            \
	B(KN_del_obj_eval_callback_all);    \
	B(KN_del_eval_callbacks);           \
	B(KN_set_cb_user_params);           \
	B(KN_set_cb_grad);                  \
	B(KN_set_cb_hess);                  \
	B(KN_solve);                        \
	B(KN_get_solution);                 \
	B(KN_get_obj_value);                \
	B(KN_get_number_cons);              \
	B(KN_get_number_vars);              \
	B(KN_get_var_primal_values);        \
	B(KN_get_var_primal_values_all);    \
	B(KN_get_var_dual_values);          \
	B(KN_get_var_dual_values_all);      \
	B(KN_get_con_values);               \
	B(KN_get_con_values_all);           \
	B(KN_get_con_dual_values);          \
	B(KN_get_con_dual_values_all);

namespace knitro
{
#define B DYLIB_EXTERN_DECLARE
APILIST
#undef B

bool is_library_loaded();

bool load_library(const std::string &path);
} // namespace knitro

struct KNITROFreeProblemT
{
	void operator()(KN_context *kc) const
	{
		if (kc != nullptr)
		{
			knitro::KN_free(&kc);
		}
	}
};

struct KNITROResult
{
	bool is_valid = false;
	int status = 0;
	double obj_val = 0.0;
	std::vector<double> x;
	std::vector<double> lambda;
	std::vector<double> con_values;
	std::vector<double> con_duals;
};

enum ObjectiveFlags
{
	OBJ_CONSTANT = 1 << 0,  // 0x01
	OBJ_LINEAR = 1 << 1,    // 0x02
	OBJ_QUADRATIC = 1 << 2, // 0x04
	OBJ_NONLINEAR = 1 << 3  // 0x08
};

enum ConstraintSenseFlags
{
	CON_LOBND = 1 << 0, // 0x01
	CON_UPBND = 1 << 1, // 0x02
};

struct CallbackPattern
{
	std::vector<KNINT> indexCons;
	std::vector<KNINT> objGradIndexVars;
	std::vector<KNINT> jacIndexCons;
	std::vector<KNINT> jacIndexVars;
	std::vector<KNINT> hessIndexVars1;
	std::vector<KNINT> hessIndexVars2;
};

template <typename V>
struct CallbackEvaluator
{
	std::vector<KNINT> indexVars;
	std::vector<KNINT> indexCons;

	CppAD::ADFun<V> fun;

	std::vector<size_t> fun_rows;
	std::vector<std::set<size_t>> jac_pattern;
	std::vector<size_t> jac_rows;
	std::vector<size_t> jac_cols;
	CppAD::sparse_jacobian_work jac_work;
	std::vector<std::set<size_t>> hess_pattern;
	std::vector<size_t> hess_rows;
	std::vector<size_t> hess_cols;
	CppAD::sparse_hessian_work hess_work;

	std::vector<V> x;
	std::vector<V> jac;
	std::vector<V> w;
	std::vector<V> hess;

	void setup()
	{
		fun.optimize();
		size_t m = fun_rows.size();
		std::vector<std::set<size_t>> jac_sparsity(m);
		for (size_t k = 0; k < m; k++)
		{
			jac_sparsity[k].insert(fun_rows[k]);
		}
		jac_pattern = fun.RevSparseJac(jac_sparsity.size(), jac_sparsity);
		for (size_t k = 0; k < jac_pattern.size(); k++)
		{
			for (size_t i : jac_pattern[k])
			{
				jac_rows.push_back(fun_rows[k]);
				jac_cols.push_back(i);
			}
		}
		std::vector<std::set<size_t>> r_hess_sparsity(fun.Domain());
		for (size_t i = 0; i < fun.Domain(); i++)
		{
			r_hess_sparsity[i].insert(i);
		}
		fun.ForSparseJac(fun.Domain(), r_hess_sparsity);
		std::vector<std::set<size_t>> hess_sparsity(1);
		for (size_t k = 0; k < m; k++)
		{
			hess_sparsity[0].insert(fun_rows[k]);
		}
		hess_pattern = fun.RevSparseHes(r_hess_sparsity.size(), hess_sparsity);
		for (size_t k = 0; k < hess_pattern.size(); k++)
		{
			for (size_t i : hess_pattern[k])
			{
				hess_rows.push_back(k);
				hess_cols.push_back(i);
			}
		}
		x.resize(indexVars.size());
		jac.resize(jac_rows.size());
		w.resize(fun.Range(), 0.0);
		hess.resize(hess_rows.size());
	}

	void eval_fun(const V *req_x, V *res_y, bool aggregate = false)
	{
		for (size_t i = 0; i < indexVars.size(); i++)
		{
			x[i] = req_x[indexVars[i]];
		}
		auto y = fun.Forward(0, x);
		for (size_t k = 0; k < fun_rows.size(); k++)
		{
			if (aggregate)
			{
				res_y[0] += y[fun_rows[k]];
			}
			else
			{
				res_y[k] = y[fun_rows[k]];
			}
		}
	}

	void eval_jac(const V *req_x, V *res_jac)
	{
		for (size_t i = 0; i < indexVars.size(); i++)
		{
			x[i] = req_x[indexVars[i]];
		}
		fun.SparseJacobianReverse(x, jac_pattern, jac_rows, jac_cols, jac, jac_work);
		for (size_t i = 0; i < jac.size(); i++)
		{
			res_jac[i] = jac[i];
		}
	}

	void eval_hess(const V *req_x, const V *req_w, V *res_hess,
	               bool aggregate = false)
	{
		for (size_t i = 0; i < indexVars.size(); i++)
		{
			x[i] = req_x[indexVars[i]];
		}
		for (size_t k = 0; k < fun_rows.size(); k++)
		{
			if (aggregate)
			{
				w[fun_rows[k]] = req_w[0];
			}
			else
			{
				w[fun_rows[k]] = req_w[indexCons[k]];
			}
		}
		fun.SparseHessian(x, w, hess_pattern, hess_rows, hess_cols, hess, hess_work);
		for (size_t i = 0; i < hess.size(); i++)
		{
			res_hess[i] = hess[i];
		}
	}

	CallbackPattern get_callback_pattern() const
	{
		CallbackPattern pattern;
		pattern.indexCons = indexCons;

		if (indexCons.empty())
		{
			for (size_t k = 0; k < jac_pattern.size(); k++)
			{
				for (size_t i : jac_pattern[k])
				{
					pattern.objGradIndexVars.push_back(indexVars[i]);
				}
			}
		}
		else
		{
			for (size_t k = 0; k < jac_pattern.size(); k++)
			{
				for (size_t i : jac_pattern[k])
				{
					pattern.jacIndexCons.push_back(indexCons[k]);
					pattern.jacIndexVars.push_back(indexVars[i]);
				}
			}
		}

		for (size_t k = 0; k < hess_pattern.size(); k++)
		{
			for (size_t i : hess_pattern[k])
			{
				pattern.hessIndexVars1.push_back(indexVars[k]);
				pattern.hessIndexVars2.push_back(indexVars[i]);
			}
		}

		return pattern;
	}
};

struct Outputs
{
	std::vector<size_t> obj_idxs;
	std::vector<size_t> con_idxs;
	std::vector<ConstraintIndex> cons;
};

class KNITROModel : public OnesideLinearConstraintMixin<KNITROModel>,
                    public TwosideLinearConstraintMixin<KNITROModel>,
                    public OnesideQuadraticConstraintMixin<KNITROModel>,
                    public TwosideQuadraticConstraintMixin<KNITROModel>,
                    public TwosideNLConstraintMixin<KNITROModel>,
                    public LinearObjectiveMixin<KNITROModel>,
                    public PPrintMixin<KNITROModel>,
                    public GetValueMixin<KNITROModel>
{
  public:
	KNITROModel();
	void init();
	void close();

	double get_infinity() const;
	KNINT _variable_index(const VariableIndex &variable);
	KNINT _constraint_index(const ConstraintIndex &constraint);

	VariableIndex add_variable(VariableDomain domain = VariableDomain::Continuous,
	                           double lb = -KN_INFINITY, double ub = KN_INFINITY,
	                           const char *name = nullptr);
	void delete_variable(const VariableIndex &variable);

	double get_variable_lb(const VariableIndex &variable);
	double get_variable_ub(const VariableIndex &variable);
	void set_variable_lb(const VariableIndex &variable, double lb);
	void set_variable_ub(const VariableIndex &variable, double ub);
	void set_variable_bounds(const VariableIndex &variable, double lb, double ub);
	double get_variable_value(const VariableIndex &variable);
	void set_variable_start(const VariableIndex &variable, double start);
	std::string get_variable_name(const VariableIndex &variable);
	void set_variable_name(const VariableIndex &variable, const std::string &name);
	void set_variable_domain(const VariableIndex &variable, VariableDomain domain);
	double get_variable_rc(const VariableIndex &variable);

	std::string pprint_variable(const VariableIndex &variable);

	ConstraintIndex add_linear_constraint(const ScalarAffineFunction &f, ConstraintSense sense,
	                                      double rhs, const char *name = nullptr);
	ConstraintIndex add_linear_constraint(const ScalarAffineFunction &f,
	                                      const std::tuple<double, double> &interval,
	                                      const char *name = nullptr);
	ConstraintIndex add_quadratic_constraint(const ScalarQuadraticFunction &f,
	                                         ConstraintSense sense, double rhs,
	                                         const char *name = nullptr);
	ConstraintIndex add_quadratic_constraint(const ScalarQuadraticFunction &f,
	                                         const std::tuple<double, double> &interval,
	                                         const char *name = nullptr);
	ConstraintIndex add_second_order_cone_constraint(const Vector<VariableIndex> &variables,
	                                                 const char *name, bool rotated);
	ConstraintIndex add_single_nl_constraint(ExpressionGraph &graph, const ExpressionHandle &result,
	                                         const std::tuple<double, double> &interval,
	                                         const char *name = nullptr);
	ConstraintIndex add_single_nl_constraint_sense_rhs(ExpressionGraph &graph,
	                                                   const ExpressionHandle &result,
	                                                   ConstraintSense sense, double rhs,
	                                                   const char *name = nullptr);

	void delete_constraint(ConstraintIndex constraint);
	void set_constraint_name(const ConstraintIndex &constraint, const std::string &name);
	std::string get_constraint_name(const ConstraintIndex &constraint);

	double get_constraint_primal(const ConstraintIndex &constraint);
	double get_constraint_dual(const ConstraintIndex &constraint);

	void set_normalized_rhs(const ConstraintIndex &constraint, double rhs);
	double get_normalized_rhs(const ConstraintIndex &constraint);
	void set_normalized_coefficient(const ConstraintIndex &constraint,
	                                const VariableIndex &variable, double coefficient);

	void set_objective(const ScalarAffineFunction &f, ObjectiveSense sense);
	void set_objective(const ScalarQuadraticFunction &f, ObjectiveSense sense);
	void set_objective(const ExprBuilder &expr, ObjectiveSense sense);
	void add_single_nl_objective(ExpressionGraph &graph, const ExpressionHandle &result);
	double get_obj_value();

	void optimize();

	template <typename T>
	void set_raw_parameter(const std::string &name, T value)
	{
		int error;
		int param_id;
		error = knitro::KN_get_param_id(m_kc.get(), name.c_str(), &param_id);
		check_error(error);
		set_raw_parameter<T>(param_id, value);
	}

	template <typename T>
	void set_raw_parameter(int param_id, T value)
	{
		int error;
		if constexpr (std::is_same_v<T, int>)
		{
			error = knitro::KN_set_int_param(m_kc.get(), param_id, value);
		}
		else if constexpr (std::is_same_v<T, double>)
		{
			error = knitro::KN_set_double_param(m_kc.get(), param_id, value);
		}
		else if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, const char *>)
		{
			if constexpr (std::is_same_v<T, std::string>)
			{
				error = knitro::KN_set_char_param(m_kc.get(), param_id, value.c_str());
			}
			else
			{
				error = knitro::KN_set_char_param(m_kc.get(), param_id, value);
			}
		}
		else
		{
			static_assert(std::is_same_v<T, int> || std::is_same_v<T, double> ||
			                  std::is_same_v<T, std::string> || std::is_same_v<T, const char *>,
			              "T must be int, double, std::string, or const char*");
		}
		check_error(error);
	}

	template <typename T>
	T get_raw_parameter(const std::string &name)
	{
		int error;
		int param_id;
		error = knitro::KN_get_param_id(m_kc.get(), name.c_str(), &param_id);
		check_error(error);
		return get_raw_parameter<T>(param_id);
	}

	template <typename T>
	T get_raw_parameter(int param_id)
	{
		int error;
		T value;
		if constexpr (std::is_same_v<T, int>)
		{
			error = knitro::KN_get_int_param(m_kc.get(), param_id, &value);
		}
		else if constexpr (std::is_same_v<T, double>)
		{
			error = knitro::KN_get_double_param(m_kc.get(), param_id, &value);
		}
		else
		{
			static_assert(std::is_same_v<T, int> || std::is_same_v<T, double>,
			              "T must be int or double for get_raw_parameter");
		}
		check_error(error);
		return value;
	}

	void check_error(int error) const;

	std::unique_ptr<KN_context, KNITROFreeProblemT> m_kc = nullptr;

	size_t n_vars = 0;
	size_t n_cons = 0;
	size_t n_lincons = 0;
	size_t n_quadcons = 0;
	size_t n_coniccons = 0;
	size_t n_nlcons = 0;

	std::unordered_map<KNINT, std::variant<KNINT, std::pair<KNINT, KNINT>>> m_soc_aux_cons;
	std::unordered_map<KNINT, uint8_t> m_con_sense_flags;
	uint8_t m_obj_flag = 0;

	std::unordered_map<ExpressionGraph *, Outputs> m_pending_outputs;
	std::vector<std::unique_ptr<CallbackEvaluator<double>>> m_evaluators;
	bool m_need_to_add_callbacks = false;

	KNITROResult m_result;
	bool m_is_dirty = true;
	int m_solve_status = 0;

	static bool is_name_empty(const char *name)
	{
		return name == nullptr || name[0] == '\0';
	}

	static int knitro_var_type(VariableDomain domain)
	{
		switch (domain)
		{
		case VariableDomain::Continuous:
			return KN_VARTYPE_CONTINUOUS;
		case VariableDomain::Integer:
			return KN_VARTYPE_INTEGER;
		case VariableDomain::Binary:
			return KN_VARTYPE_BINARY;
		default:
			throw std::runtime_error("Unknown variable domain");
		}
	}

	static int knitro_obj_goal(ObjectiveSense sense)
	{
		switch (sense)
		{
		case ObjectiveSense::Minimize:
			return KN_OBJGOAL_MINIMIZE;
		case ObjectiveSense::Maximize:
			return KN_OBJGOAL_MAXIMIZE;
		default:
			throw std::runtime_error("Unknown objective sense");
		}
	}

  private:
	std::tuple<double, double> _sense_to_interval(ConstraintSense sense, double rhs);
	void _update_con_sense_flags(const ConstraintIndex &constraint, ConstraintSense sense);

	void _set_linear_constraint(const ConstraintIndex &constraint, const ScalarAffineFunction &f);
	void _set_quadratic_constraint(const ConstraintIndex &constraint,
	                               const ScalarQuadraticFunction &f);
	void _set_second_order_cone_constraint(const ConstraintIndex &constraint,
	                                       const Vector<VariableIndex> &variables);
	void _set_second_order_cone_constraint_rotated(const ConstraintIndex &constraint,
	                                               const Vector<VariableIndex> &variables);
	void _set_linear_objective(const ScalarAffineFunction &f);
	void _set_quadratic_objective(const ScalarQuadraticFunction &f);
	void _reset_objective();
	void _add_graph(ExpressionGraph &graph);
	void _add_callbacks();
	void _add_constraint_callback(ExpressionGraph *graph, const Outputs &outputs);
	void _add_objective_callback(ExpressionGraph *graph, const Outputs &outputs);
	void _update();
	void _pre_solve();
	void _solve();
	void _post_solve();

	template <typename F>
	ConstraintIndex _add_constraint_impl(ConstraintType type,
	                                     const std::tuple<double, double> &interval,
	                                     const char *name, size_t *np, const F &setter)
	{
		KNINT indexCon;
		int error = knitro::KN_add_con(m_kc.get(), &indexCon);
		check_error(error);

		IndexT index = indexCon;
		ConstraintIndex constraint(type, index);

		double lb = std::get<0>(interval);
		double ub = std::get<1>(interval);

		error = knitro::KN_set_con_lobnd(m_kc.get(), indexCon, lb);
		check_error(error);
		error = knitro::KN_set_con_upbnd(m_kc.get(), indexCon, ub);
		check_error(error);

		setter(constraint);

		if (!is_name_empty(name))
		{
			error = knitro::KN_set_con_name(m_kc.get(), indexCon, name);
			check_error(error);
		}

		m_con_sense_flags[indexCon] = CON_UPBND;

		n_cons++;
		if (np != nullptr)
		{
			(*np)++;
		}
		m_is_dirty = true;

		return constraint;
	}

	template <typename S, typename F>
	ConstraintIndex _add_constraint_with_sense(const S &function, ConstraintSense sense, double rhs,
	                                           const char *name, const F &add)
	{
		auto interval = _sense_to_interval(sense, rhs);
		auto constraint = add(function, interval, name);
		_update_con_sense_flags(constraint, sense);
		return constraint;
	}

	template <typename F>
	void _set_objective_impl(ObjectiveSense sense, const F &setter)
	{
		_reset_objective();
		setter();
		int goal = knitro_obj_goal(sense);
		int error = knitro::KN_set_obj_goal(m_kc.get(), goal);
		check_error(error);
		m_is_dirty = true;
		m_result.is_valid = false;
	}

	template <typename F, typename G, typename H>
	void _register_callback(CallbackEvaluator<double> *evaluator, const F f, const G g, const H h)
	{
		CB_context *cb = nullptr;
		auto p = evaluator->get_callback_pattern();
		int error;
		error = knitro::KN_add_eval_callback(m_kc.get(), p.indexCons.empty(), p.indexCons.size(),
		                                     p.indexCons.data(), f, &cb);
		check_error(error);
		error = knitro::KN_set_cb_user_params(m_kc.get(), cb, evaluator);
		check_error(error);
		error = knitro::KN_set_cb_grad(m_kc.get(), cb, p.objGradIndexVars.size(),
		                               p.objGradIndexVars.data(), p.jacIndexCons.size(),
		                               p.jacIndexCons.data(), p.jacIndexVars.data(), g);
		check_error(error);
		error = knitro::KN_set_cb_hess(m_kc.get(), cb, p.hessIndexVars1.size(),
		                               p.hessIndexVars1.data(), p.hessIndexVars2.data(), h);
		check_error(error);
	}

	template <typename T, typename F, typename G, typename H>
	void _add_callback_impl(const ExpressionGraph &graph, const std::vector<size_t> &rows,
	                        const std::vector<ConstraintIndex> cons, const T &trace, const F f,
	                        const G g, const H h)
	{
		auto evaluator_ptr = std::make_unique<CallbackEvaluator<double>>();
		auto *evaluator = evaluator_ptr.get();
		evaluator->fun = trace(graph);
		evaluator->fun_rows = rows;
		evaluator->indexVars.resize(graph.n_variables());
		for (size_t i = 0; i < graph.n_variables(); i++)
		{
			evaluator->indexVars[i] = _variable_index(graph.m_variables[i]);
		}
		evaluator->indexCons.resize(cons.size());
		for (size_t i = 0; i < cons.size(); i++)
		{
			evaluator->indexCons[i] = _constraint_index(cons[i]);
		}
		evaluator->setup();
		_register_callback(evaluator, f, g, h);
		m_evaluators.push_back(std::move(evaluator_ptr));
	}

	template <typename F>
	std::string _get_name(KNINT index, F get, const char *prefix) const
	{
		char name[1024];
		name[0] = '\0';
		int error = get(m_kc.get(), index, 1024, name);
		check_error(error);

		if (name[0] != '\0')
		{
			return std::string(name);
		}
		else
		{
			return fmt::format("{}{}", prefix, index);
		}
	}

	template <typename F>
	void _set_name(KNINT index, const std::string &name, F set)
	{
		int error = set(m_kc.get(), index, name.c_str());
		check_error(error);
		m_is_dirty = true;
	}
};
