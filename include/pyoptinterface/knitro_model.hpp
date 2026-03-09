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
	B(KN_checkout_license);             \
	B(KN_release_license);              \
	B(KN_new_lm);                       \
	B(KN_new);                          \
	B(KN_free);                         \
	B(KN_update);                       \
	B(KN_solve);                        \
	B(KN_get_param_id);                 \
	B(KN_get_param_type);               \
	B(KN_set_int_param);                \
	B(KN_set_char_param);               \
	B(KN_set_double_param);             \
	B(KN_get_int_param);                \
	B(KN_get_double_param);             \
	B(KN_add_var);                      \
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
	B(KN_get_con_lobnd);                \
	B(KN_get_con_upbnd);                \
	B(KN_set_con_name);                 \
	B(KN_get_con_name);                 \
	B(KN_set_obj_goal);                 \
	B(KN_get_obj_goal);                 \
	B(KN_set_var_primal_init_value);    \
	B(KN_add_obj_constant);             \
	B(KN_del_obj_constant);             \
	B(KN_add_obj_linear_struct);        \
	B(KN_del_obj_linear_struct_all);    \
	B(KN_add_obj_quadratic_struct);     \
	B(KN_del_obj_quadratic_struct_all); \
	B(KN_chg_obj_linear_term);          \
	B(KN_add_con_constant);             \
	B(KN_add_con_linear_struct);        \
	B(KN_add_con_linear_term);          \
	B(KN_add_con_quadratic_struct);     \
	B(KN_add_con_quadratic_term);       \
	B(KN_chg_con_linear_term);          \
	B(KN_add_eval_callback);            \
	B(KN_set_cb_user_params);           \
	B(KN_set_cb_grad);                  \
	B(KN_set_cb_hess);                  \
	B(KN_del_obj_eval_callback_all);    \
	B(KN_get_var_primal_value);         \
	B(KN_get_var_dual_value);           \
	B(KN_get_con_value);                \
	B(KN_get_con_dual_value);           \
	B(KN_get_obj_value);                \
	B(KN_get_number_iters);             \
	B(KN_get_mip_number_nodes);         \
	B(KN_get_mip_relaxation_bnd);       \
	B(KN_get_mip_rel_gap);              \
	B(KN_get_solve_time_real);          \
	B(KN_get_release);

namespace knitro
{
#define B DYLIB_EXTERN_DECLARE
APILIST
#undef B

bool is_library_loaded();

bool load_library(const std::string &path);

bool has_valid_license();
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

struct KNITROFreeLicenseT
{
	void operator()(LM_context *lmc) const
	{
		if (lmc != nullptr)
		{
			knitro::KN_release_license(&lmc);
		}
	}
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
	static inline constexpr const char *JAC_CLRNG = "cppad";
	static inline constexpr const char *HES_CLRNG = "cppad.symmetric";
	std::vector<KNINT> indexVars;
	std::vector<KNINT> indexCons;

	CppAD::ADFun<V> fun; /// < CppAD tape.

	/// Sparsity patterns
	CppAD::sparse_rc<std::vector<size_t>> jp;
	CppAD::sparse_rc<std::vector<size_t>> hp;

	/// Workspaces for sparse Jacobian and Hessian calculations
	CppAD::sparse_jac_work jw;
	CppAD::sparse_hes_work hw;

	/// Temporary vectors for evaluations
	std::vector<V> x;
	std::vector<V> w;
	CppAD::sparse_rcv<std::vector<size_t>, std::vector<V>> jac;
	CppAD::sparse_rcv<std::vector<size_t>, std::vector<V>> hes;

	void setup()
	{
		fun.optimize();
		size_t nx = fun.Domain();
		size_t ny = fun.Range();
		CppAD::sparse_rc<std::vector<size_t>> jp_in(nx, nx, nx);
		for (size_t i = 0; i < nx; i++)
		{
			jp_in.set(i, i, i);
		}
		fun.for_jac_sparsity(jp_in, false, false, false, jp);
		std::vector<bool> select_rows(ny, true);
		CppAD::sparse_rc<std::vector<size_t>> hp_out;
		fun.rev_hes_sparsity(select_rows, false, false, hp_out);
		auto &hrow = hp_out.row();
		auto &hcol = hp_out.col();
		for (size_t k = 0; k < hp_out.nnz(); k++)
		{
			size_t row = hrow[k];
			size_t col = hcol[k];
			if (row <= col)
			{
				hp.push_back(row, col);
			}
		}
		x.resize(nx, 0.0);
		w.resize(ny, 0.0);
		jac = CppAD::sparse_rcv<std::vector<size_t>, std::vector<V>>(jp);
		hes = CppAD::sparse_rcv<std::vector<size_t>, std::vector<V>>(hp);
	}

	bool is_objective() const
	{
		return indexCons.empty();
	}

	void eval_fun(const V *req_x, V *res_y)
	{
		copy_ptr(req_x, indexVars.data(), x);
		auto y = fun.Forward(0, x);
		copy_vec(y, res_y, is_objective());
	}

	void eval_jac(const V *req_x, V *res_jac)
	{
		copy_ptr(req_x, indexVars.data(), x);
		fun.sparse_jac_rev(x, jac, jp, JAC_CLRNG, jw);
		copy_vec(jac.val(), res_jac);
	}

	void eval_hess(const V *req_x, const V *req_w, V *res_hess)
	{
		copy_ptr(req_x, indexVars.data(), x);
		copy_ptr(req_w, indexCons.data(), w, is_objective());
		fun.sparse_hes(x, w, hes, hp, HES_CLRNG, hw);
		copy_vec(hes.val(), res_hess);
	}

	CallbackPattern get_callback_pattern() const
	{
		CallbackPattern p;
		p.indexCons = indexCons;

		auto &jrow = jp.row();
		auto &jcol = jp.col();
		if (indexCons.empty())
		{
			for (size_t k = 0; k < jp.nnz(); k++)
			{
				p.objGradIndexVars.push_back(indexVars[jcol[k]]);
			}
		}
		else
		{
			for (size_t k = 0; k < jp.nnz(); k++)
			{
				p.jacIndexCons.push_back(indexCons[jrow[k]]);
				p.jacIndexVars.push_back(indexVars[jcol[k]]);
			}
		}

		auto &hrow = hp.row();
		auto &hcol = hp.col();
		for (size_t k = 0; k < hp.nnz(); k++)
		{
			p.hessIndexVars1.push_back(indexVars[hrow[k]]);
			p.hessIndexVars2.push_back(indexVars[hcol[k]]);
		}

		return p;
	}

  private:
	template <typename T, typename I>
	static void copy_ptr(const T *src, const I *idx, std::vector<V> &dst, bool duplicate = false)
	{
		for (size_t i = 0; i < dst.size(); i++)
		{
			if (duplicate)
			{
				dst[i] = src[0];
			}
			else
			{
				dst[i] = src[idx[i]];
			}
		}
	}

	template <typename T>
	static void copy_vec(const std::vector<T> &src, T *dst, bool aggregate = false)
	{
		if (aggregate)
		{
			dst[0] = 0.0;
		}
		for (size_t i = 0; i < src.size(); i++)
		{
			if (aggregate)
			{
				dst[0] += src[i];
			}
			else
			{
				dst[i] = src[i];
			}
		}
	}
};

struct Outputs
{
	std::vector<size_t> objective_outputs;
	std::vector<size_t> constraint_outputs;
	std::vector<ConstraintIndex> constraints;
};

inline bool is_name_empty(const char *name)
{
	return name == nullptr || name[0] == '\0';
}

inline int knitro_var_type(VariableDomain domain)
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

inline VariableDomain knitro_variable_domain(int var_type)
{
	switch (var_type)
	{
	case KN_VARTYPE_CONTINUOUS:
		return VariableDomain::Continuous;
	case KN_VARTYPE_INTEGER:
		return VariableDomain::Integer;
	case KN_VARTYPE_BINARY:
		return VariableDomain::Binary;
	default:
		throw std::runtime_error("Unknown variable type");
	}
}

inline int knitro_obj_goal(ObjectiveSense sense)
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

inline ObjectiveSense knitro_obj_sense(int goal)
{
	switch (goal)
	{
	case KN_OBJGOAL_MINIMIZE:
		return ObjectiveSense::Minimize;
	case KN_OBJGOAL_MAXIMIZE:
		return ObjectiveSense::Maximize;
	default:
		throw std::runtime_error("Unknown objective goal");
	}
}

inline void knitro_throw(int error)
{
	if (error != 0)
	{
		throw std::runtime_error(fmt::format("KNITRO error code: {}", error));
	}
}

class KNITROEnv
{
  public:
	KNITROEnv(bool empty = false);

	KNITROEnv(const KNITROEnv &) = delete;
	KNITROEnv &operator=(const KNITROEnv &) = delete;

	KNITROEnv(KNITROEnv &&) = default;
	KNITROEnv &operator=(KNITROEnv &&) = default;

	void start();
	bool empty() const;
	std::shared_ptr<LM_context> get_lm() const;
	void close();

  private:
	void _check_error(int code) const;
	std::shared_ptr<LM_context> m_lm = nullptr;
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
	// Constructor/Init/Close
	KNITROModel();
	KNITROModel(const KNITROEnv &env);

	KNITROModel(const KNITROModel &) = delete;
	KNITROModel &operator=(const KNITROModel &) = delete;

	KNITROModel(KNITROModel &&) = default;
	KNITROModel &operator=(KNITROModel &&) = default;

	void init();
	void init(const KNITROEnv &env);
	void close();

	// Model information
	double get_infinity() const;
	std::string get_solver_name() const;
	std::string get_release() const;

	// Variable functions
	VariableIndex add_variable(VariableDomain domain = VariableDomain::Continuous,
	                           double lb = -KN_INFINITY, double ub = KN_INFINITY,
	                           const char *name = nullptr);
	void delete_variable(const VariableIndex &variable);
	double get_variable_lb(const VariableIndex &variable) const;
	double get_variable_ub(const VariableIndex &variable) const;
	void set_variable_lb(const VariableIndex &variable, double lb);
	void set_variable_ub(const VariableIndex &variable, double ub);
	void set_variable_bounds(const VariableIndex &variable, double lb, double ub);
	double get_variable_value(const VariableIndex &variable) const;
	void set_variable_start(const VariableIndex &variable, double start);
	std::string get_variable_name(const VariableIndex &variable) const;
	void set_variable_name(const VariableIndex &variable, const std::string &name);
	void set_variable_domain(const VariableIndex &variable, VariableDomain domain);
	VariableDomain get_variable_domain(const VariableIndex &variable) const;
	double get_variable_rc(const VariableIndex &variable) const;
	std::string pprint_variable(const VariableIndex &variable) const;

	// Constraint functions
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
	void delete_constraint(const ConstraintIndex &constraint);
	void set_constraint_name(const ConstraintIndex &constraint, const std::string &name);
	std::string get_constraint_name(const ConstraintIndex &constraint) const;
	double get_constraint_primal(const ConstraintIndex &constraint) const;
	double get_constraint_dual(const ConstraintIndex &constraint) const;
	void set_normalized_rhs(const ConstraintIndex &constraint, double rhs);
	double get_normalized_rhs(const ConstraintIndex &constraint) const;
	void set_normalized_coefficient(const ConstraintIndex &constraint,
	                                const VariableIndex &variable, double coefficient);

	// Objective functions
	void set_objective(const ScalarAffineFunction &f, ObjectiveSense sense);
	void set_objective(const ScalarQuadraticFunction &f, ObjectiveSense sense);
	void set_objective(const ExprBuilder &expr, ObjectiveSense sense);
	void add_single_nl_objective(ExpressionGraph &graph, const ExpressionHandle &result);
	double get_obj_value() const;
	void set_obj_sense(ObjectiveSense sense);
	ObjectiveSense get_obj_sense() const;
	void set_objective_coefficient(const VariableIndex &variable, double coefficient);

	// Solve functions
	void optimize();

	// Solve information
	size_t get_number_iterations() const;
	size_t get_mip_node_count() const;
	double get_obj_bound() const;
	double get_mip_relative_gap() const;
	double get_solve_time() const;

	// Model state
	bool dirty() const;
	bool empty() const;

	// Solve status
	int get_solve_status() const;

	// Parameter management
	template <typename T>
	void set_raw_parameter(const std::string &name, T value)
	{
		int param_id = _get_value<const char *, int>(knitro::KN_get_param_id, name.c_str());
		set_raw_parameter<T>(param_id, value);
	}

	template <typename T>
	void set_raw_parameter(int param_id, T value)
	{
		if constexpr (std::is_same_v<T, int>)
		{
			_set_value<int, T>(knitro::KN_set_int_param, param_id, value);
		}
		else if constexpr (std::is_same_v<T, double>)
		{
			_set_value<int, T>(knitro::KN_set_double_param, param_id, value);
		}
		else if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, const char *>)
		{
			if constexpr (std::is_same_v<T, std::string>)
			{
				_set_value<int, const char *>(knitro::KN_set_char_param, param_id, value.c_str());
			}
			else
			{
				_set_value<int, const char *>(knitro::KN_set_char_param, param_id, value);
			}
		}
		else
		{
			static_assert(std::is_same_v<T, int> || std::is_same_v<T, double> ||
			                  std::is_same_v<T, std::string> || std::is_same_v<T, const char *>,
			              "T must be int, double, std::string, or const char*");
		}
	}

	template <typename T>
	T get_raw_parameter(const std::string &name)
	{
		int param_id = _get_value<const char *, int>(knitro::KN_get_param_id, name.c_str());
		return get_raw_parameter<T>(param_id);
	}

	template <typename T>
	T get_raw_parameter(int param_id)
	{
		if constexpr (std::is_same_v<T, int>)
		{
			return _get_value<int, T>(knitro::KN_get_int_param, param_id);
		}
		else if constexpr (std::is_same_v<T, double>)
		{
			return _get_value<int, T>(knitro::KN_get_double_param, param_id);
		}
		else
		{
			static_assert(std::is_same_v<T, int> || std::is_same_v<T, double>,
			              "T must be int or double for get_raw_parameter");
		}
	}

	// Internal helpers
	void _check_error(int error) const;
	void _mark_dirty();
	void _unmark_dirty();
	void _check_dirty() const;
	KNINT _variable_index(const VariableIndex &variable) const;
	KNINT _constraint_index(const ConstraintIndex &constraint) const;

	size_t get_num_vars() const;
	size_t get_num_cons(std::optional<ConstraintType> type = std::nullopt) const;

  private:
	// Member variables
	size_t m_n_vars = 0;
	std::unordered_map<ConstraintType, size_t> m_n_cons_map;
	std::shared_ptr<LM_context> m_lm = nullptr;
	std::unique_ptr<KN_context, KNITROFreeProblemT> m_kc = nullptr;

	std::unordered_map<KNINT, std::variant<KNINT, std::pair<KNINT, KNINT>>> m_soc_aux_cons;
	std::unordered_map<KNINT, uint8_t> m_con_sense_flags;
	uint8_t m_obj_flag = 0;

	std::unordered_map<ExpressionGraph *, Outputs> m_pending_outputs;
	std::vector<std::unique_ptr<CallbackEvaluator<double>>> m_evaluators;
	bool m_has_pending_callbacks = false;
	int m_solve_status = 0;
	bool m_is_dirty = true;

  private:
	void _init();
	void _reset_state();
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
	void _add_pending_callbacks();
	void _add_callbacks(const ExpressionGraph &graph, const Outputs &outputs);
	void _add_callback(const ExpressionGraph &graph, const std::vector<size_t> &outputs,
	                   const std::vector<ConstraintIndex> &constraints);
	void _register_callback(CallbackEvaluator<double> *evaluator);
	void _update();
	void _pre_solve();
	void _solve();
	void _post_solve();

	template <typename F>
	ConstraintIndex _add_constraint_impl(ConstraintType type,
	                                     const std::tuple<double, double> &interval,
	                                     const char *name, const F &setter)
	{
		KNINT indexCon;
		int error = knitro::KN_add_con(m_kc.get(), &indexCon);
		_check_error(error);

		IndexT index = indexCon;
		ConstraintIndex constraint(type, index);

		double lb = std::get<0>(interval);
		double ub = std::get<1>(interval);

		error = knitro::KN_set_con_lobnd(m_kc.get(), indexCon, lb);
		_check_error(error);
		error = knitro::KN_set_con_upbnd(m_kc.get(), indexCon, ub);
		_check_error(error);

		setter(constraint);

		if (!is_name_empty(name))
		{
			error = knitro::KN_set_con_name(m_kc.get(), indexCon, name);
			_check_error(error);
		}

		m_con_sense_flags[indexCon] = CON_UPBND;

		auto it = m_n_cons_map.find(type);
		if (it != m_n_cons_map.end())
		{
			it->second++;
		}
		else
		{
			m_n_cons_map[type] = 1;
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
		set_obj_sense(sense);
		m_is_dirty = true;
	}

	template <typename V>
	using Getter = std::function<int(KN_context *, V *)>;
	template <typename V>
	using Setter = std::function<int(KN_context *, V)>;

	template <typename V>
	V _get_value(Getter<V> get) const
	{
		V value;
		int error = get(m_kc.get(), &value);
		_check_error(error);
		return value;
	}

	template <typename V>
	void _set_value(Setter<V> set, V value)
	{
		int error = set(m_kc.get(), value);
		_check_error(error);
	}

	template <typename K, typename V>
	using GetterMap = std::function<int(KN_context *, K, V *)>;
	template <typename K, typename V>
	using SetterMap = std::function<int(KN_context *, K, V)>;

	template <typename K, typename V>
	V _get_value(GetterMap<K, V> get, K key) const
	{
		V value;
		int error = get(m_kc.get(), key, &value);
		_check_error(error);
		return value;
	}

	template <typename K, typename V>
	void _set_value(SetterMap<K, V> set, K key, V value)
	{
		int error = set(m_kc.get(), key, value);
		_check_error(error);
	}

	template <typename F, typename K>
	std::string _get_name(F get, K key, const char *prefix) const
	{
		char name[1024];
		name[0] = '\0';
		int error = get(m_kc.get(), key, 1024, name);
		_check_error(error);

		if (name[0] != '\0')
		{
			return std::string(name);
		}
		else
		{
			return fmt::format("{}{}", prefix, key);
		}
	}

	template <typename I>
	KNINT _get_index(const I &index) const
	{
		return index.index;
	}
};
