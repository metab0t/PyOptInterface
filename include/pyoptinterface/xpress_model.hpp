#pragma

#include <exception>
#include <memory>
#include <variant>

// #include <xprs.h>
#include "../thirdparty/solvers/xpress/xpress_forward_decls.h"

#include "pyoptinterface/core.hpp"
#include "pyoptinterface/container.hpp"
#define USE_NLMIXIN
#include "pyoptinterface/solver_common.hpp"
#include "pyoptinterface/dylib.hpp"

// PyOptInterface has been compiled and tested with Xpress version 46.1.1
#define XPRS_VER_MAJOR 46
#define XPRS_VER_MINOR 1
#define XPRS_VER_BUILD 1

#if POI_XPVERSION_MAJOR < XPRS_VER_MAJOR
#warning "System Xpress library major version is older than the officially supported version. " \
    "Some features may not work correctly."
#endif

// Xpress Callback Abstraction Layer
//
// Xpress supports multiple callbacks that we want to expose to PyOptInterface as "contexts" to
// match the design of other solver interfaces. To bridge Xpress's multiple-callback system to
// PyOptInterface's single-callback-with-context model, we need to repeat the same setup for each
// callback: define the low-level C function, register it with Xpress, handle its arguments, and
// bind everything through Nanobind.
//
// Rather than copy-paste this boilerplate for each callback (error-prone and hard to maintain), we
// use macros to generate everything from a single list.
//
// How does it work:
// We define XPRSCB_LIST below with all supported callbacks and their signatures. This list is
// parameterized by two macros that let us generate different code from the same data:
//
//   MACRO - Receives callback metadata (ID, name, return type, arguments) and expands to whatever
//   we're generating (function declarations, enum values, registration code, etc.)
//
//   ARG - Formats each callback *optional* argument. MACRO receives arguments wrapped by ARG, so it
//   can extract type/name information as needed.
//
// When we need to do something with all callbacks (e.g., declare wrapper functions), we pass
// specific MACRO and ARG definitions to XPRSCB_LIST, and it generates the code.
//
// MACRO is invoked with:
//   1. ID - Unique number for enum bit-flags (0-63, used for callback identification)
//   2. NAME - Xpress callback name (e.g., "optnode" for XPRSaddcboptnode)
//   3. RET - Return type of the Xpress callback function
//   4. ...ARGS - Callback arguments (excluding XPRSprob and void* which are always first),
//                     each wrapped in ARG(TYPE, NAME)
//
// ARG is invoked with:
//   1. TYPE - Argument type from Xpress documentation
//   2. NAME - Argument name from Xpress documentation
//
// Example: To declare all callback wrappers, define MACRO to generate a function declaration and
// ARG to format parameters, then invoke XPRSCB_LIST(MACRO, ARG).

// clang-format off
#define XPRSCB_LIST(MACRO, ARG)                  \
	MACRO(0, bariteration, void,                 \
        ARG(int *, p_action))                    \
	MACRO(1, barlog, int)                        \
	MACRO(2, afterobjective, void)               \
	MACRO(3, beforeobjective, void)              \
	MACRO(5, presolve, void)                     \
	MACRO(6, checktime,int)                      \
	MACRO(7, chgbranchobject, void,              \
        ARG(XPRSbranchobject, obranch)           \
        ARG(XPRSbranchobject *, p_newobject))    \
	MACRO(8, cutlog, int)                        \
	MACRO(9, cutround, void,                     \
        ARG(int, ifxpresscuts)                   \
        ARG(int*, p_action))                     \
	MACRO(10, destroymt, void)                   \
	MACRO(11, gapnotify, void,                   \
        ARG(double*, p_relgapnotifytarget)       \
        ARG(double*, p_absgapnotifytarget)       \
        ARG(double*, p_absgapnotifyobjtarget)    \
        ARG(double*, p_absgapnotifyboundtarget)) \
	MACRO(12, miplog, int)                       \
	MACRO(13, infnode, void)                     \
	MACRO(14, intsol, void)                      \
	MACRO(15, lplog, int)                        \
	MACRO(16, message, void,                     \
        ARG(const char*, msg)                    \
        ARG(int, msglen)                         \
        ARG(int, msgtype))                       \
    /* This CB is currently incompatible with this wapper design 
    MACRO(17, mipthread, void,                   \
        ARG(XPRSprob, threadprob))  */           \
	MACRO(18, newnode, void,                     \
        ARG(int, parentnode)                     \
        ARG(int, node)                           \
        ARG(int, branch))                        \
	MACRO(19, nodecutoff, void,                  \
        ARG(int, node))                          \
	MACRO(20, nodelpsolved, void)                \
	MACRO(21, optnode, void,                     \
        ARG(int*, p_infeasible))                 \
	MACRO(22, preintsol, void,                   \
        ARG(int, soltype)                        \
        ARG(int*, p_reject)                      \
        ARG(double*, p_cutoff))                  \
	MACRO(23, prenode, void,                     \
        ARG(int*, p_infeasible))                 \
	MACRO(24, usersolnotify, void,               \
        ARG(const char*, solname)                \
        ARG(int, status))
// clang-format on

// Common callbacks optional arguments formatting macros:
#define XPRSCB_ARG_IGNORE(TYPE, NAME)         // Ingore arguments
#define XPRSCB_ARG_FN(TYPE, NAME) , TYPE NAME // As args of a function
#define XPRSCB_ARG_TYPE(TYPE, NAME) , TYPE    // As args of a template

// Expand to 2 elements of APILIST
#define XPRSCB_ADDREMOVE_FN(ID, NAME, ...) \
	B(XPRSaddcb##NAME);                    \
	B(XPRSremovecb##NAME);

// Define Xpress C APIs list.
// Similar idea to the CBs list, here the B macro is externally provided as in the other solvers
// interface implementation.
#define APILIST                 \
	B(XPRSaddcols64);           \
	B(XPRSaddcuts64);           \
	B(XPRSaddmanagedcuts64);    \
	B(XPRSaddmipsol);           \
	B(XPRSaddnames);            \
	B(XPRSaddqmatrix64);        \
	B(XPRSaddrows64);           \
	B(XPRSaddsets64);           \
	B(XPRSbeginlicensing);      \
	B(XPRSchgbounds);           \
	B(XPRSchgcoef);             \
	B(XPRSchgcoltype);          \
	B(XPRSchgmqobj64);          \
	B(XPRSchgobj);              \
	B(XPRSchgobjsense);         \
	B(XPRSchgrhs);              \
	B(XPRSchgrowtype);          \
	B(XPRScreateprob);          \
	B(XPRSdelcols);             \
	B(XPRSdelobj);              \
	B(XPRSdelqmatrix);          \
	B(XPRSdelrows);             \
	B(XPRSdelsets);             \
	B(XPRSdestroyprob);         \
	B(XPRSendlicensing);        \
	B(XPRSfree);                \
	B(XPRSgetattribinfo);       \
	B(XPRSgetbasisval);         \
	B(XPRSgetcallbacksolution); \
	B(XPRSgetcoef);             \
	B(XPRSgetcoltype);          \
	B(XPRSgetcontrolinfo);      \
	B(XPRSgetdblattrib);        \
	B(XPRSgetdblcontrol);       \
	B(XPRSgetdualray);          \
	B(XPRSgetduals);            \
	B(XPRSgetiisdata);          \
	B(XPRSgetintattrib64);      \
	B(XPRSgetintcontrol64);     \
	B(XPRSgetlasterror);        \
	B(XPRSgetlb);               \
	B(XPRSgetlicerrmsg);        \
	B(XPRSgetlpsol);            \
	B(XPRSgetnamelist);         \
	B(XPRSgetobj);              \
	B(XPRSgetprimalray);        \
	B(XPRSgetprobname);         \
	B(XPRSgetredcosts);         \
	B(XPRSgetrhs);              \
	B(XPRSgetrowtype);          \
	B(XPRSgetslacks);           \
	B(XPRSgetsolution);         \
	B(XPRSgetstrattrib);        \
	B(XPRSgetstrcontrol);       \
	B(XPRSgetstringattrib);     \
	B(XPRSgetstringcontrol);    \
	B(XPRSgetub);               \
	B(XPRSgetversion);          \
	B(XPRSgetversionnumbers);   \
	B(XPRSiisall);              \
	B(XPRSiisfirst);            \
	B(XPRSinit);                \
	B(XPRSinterrupt);           \
	B(XPRSlicense);             \
	B(XPRSnlpaddformulas);      \
	B(XPRSnlploadformulas);     \
	B(XPRSnlppostsolve);        \
	B(XPRSoptimize);            \
	B(XPRSpostsolve);           \
	B(XPRSpresolverow);         \
	B(XPRSsaveas);              \
	B(XPRSsetdblcontrol);       \
	B(XPRSsetintcontrol);       \
	B(XPRSsetintcontrol64);     \
	B(XPRSsetlogfile);          \
	B(XPRSsetprobname);         \
	B(XPRSsetstrcontrol);       \
	B(XPRSwritebasis);          \
	B(XPRSwritebinsol);         \
	B(XPRSwriteprob);           \
	B(XPRSwriteprtsol);         \
	B(XPRSwriteslxsol);         \
	B(XPRSwritesol);            \
	XPRSCB_LIST(XPRSCB_ADDREMOVE_FN, XPRSCB_ARG_IGNORE);

namespace xpress
{
// Define xpress function inside the xpress namespace
#define B DYLIB_EXTERN_DECLARE
APILIST
#undef B

// Libray loading functions
bool is_library_loaded();
bool load_library(const std::string &path);
std::pair<int, std::string> license(int p_i, const char *p_c);
bool beginlicensing();
void endlicensing();

// Xpress doesn't have an Env pointer, however, POI manages environment
// initialization with an OOP interface.
struct Env
{
	Env(const char *path = nullptr);
	~Env();
	void close();

	static inline std::mutex mtx;
	static inline int init_count = 0;
	bool initialized = false;
};

// Some of the Xpress enums are here re-defined to enforce type safety
// Types associated with Xpress attribute and controls
enum class CATypes : int
{
	NOTDEFINED = POI_XPRS_TYPE_NOTDEFINED,
	INT = POI_XPRS_TYPE_INT,
	INT64 = POI_XPRS_TYPE_INT64,
	DOUBLE = POI_XPRS_TYPE_DOUBLE,
	STRING = POI_XPRS_TYPE_STRING,
};

enum class SOLSTATUS : int
{
	NOTFOUND = POI_XPRS_SOLSTATUS_NOTFOUND,
	OPTIMAL = POI_XPRS_SOLSTATUS_OPTIMAL,
	FEASIBLE = POI_XPRS_SOLSTATUS_FEASIBLE,
	INFEASIBLE = POI_XPRS_SOLSTATUS_INFEASIBLE,
	UNBOUNDED = POI_XPRS_SOLSTATUS_UNBOUNDED
};

enum class SOLVESTATUS : int
{
	UNSTARTED = POI_XPRS_SOLVESTATUS_UNSTARTED,
	STOPPED = POI_XPRS_SOLVESTATUS_STOPPED,
	FAILED = POI_XPRS_SOLVESTATUS_FAILED,
	COMPLETED = POI_XPRS_SOLVESTATUS_COMPLETED
};

enum class LPSTATUS : int
{
	UNSTARTED = POI_XPRS_LP_UNSTARTED,
	OPTIMAL = POI_XPRS_LP_OPTIMAL,
	INFEAS = POI_XPRS_LP_INFEAS,
	CUTOFF = POI_XPRS_LP_CUTOFF,
	UNFINISHED = POI_XPRS_LP_UNFINISHED,
	UNBOUNDED = POI_XPRS_LP_UNBOUNDED,
	CUTOFF_IN_DUAL = POI_XPRS_LP_CUTOFF_IN_DUAL,
	UNSOLVED = POI_XPRS_LP_UNSOLVED,
	NONCONVEX = POI_XPRS_LP_NONCONVEX
};

enum class MIPSTATUS : int
{
	NOT_LOADED = POI_XPRS_MIP_NOT_LOADED,
	LP_NOT_OPTIMAL = POI_XPRS_MIP_LP_NOT_OPTIMAL,
	LP_OPTIMAL = POI_XPRS_MIP_LP_OPTIMAL,
	NO_SOL_FOUND = POI_XPRS_MIP_NO_SOL_FOUND,
	SOLUTION = POI_XPRS_MIP_SOLUTION,
	INFEAS = POI_XPRS_MIP_INFEAS,
	OPTIMAL = POI_XPRS_MIP_OPTIMAL,
	UNBOUNDED = POI_XPRS_MIP_UNBOUNDED
};

enum class NLPSTATUS : int
{
	UNSTARTED = POI_XPRS_NLPSTATUS_UNSTARTED,
	SOLUTION = POI_XPRS_NLPSTATUS_SOLUTION,
	LOCALLY_OPTIMAL = POI_XPRS_NLPSTATUS_LOCALLY_OPTIMAL,
	OPTIMAL = POI_XPRS_NLPSTATUS_OPTIMAL,
	NOSOLUTION = POI_XPRS_NLPSTATUS_NOSOLUTION,
	LOCALLY_INFEASIBLE = POI_XPRS_NLPSTATUS_LOCALLY_INFEASIBLE,
	INFEASIBLE = POI_XPRS_NLPSTATUS_INFEASIBLE,
	UNBOUNDED = POI_XPRS_NLPSTATUS_UNBOUNDED,
	UNFINISHED = POI_XPRS_NLPSTATUS_UNFINISHED,
	UNSOLVED = POI_XPRS_NLPSTATUS_UNSOLVED,
};

enum class IISSOLSTATUS : int
{
	UNSTARTED = POI_XPRS_IIS_UNSTARTED,
	FEASIBLE = POI_XPRS_IIS_FEASIBLE,
	COMPLETED = POI_XPRS_IIS_COMPLETED,
	UNFINISHED = POI_XPRS_IIS_UNFINISHED
};

enum class SOLAVAILABLE : int
{
	NOTFOUND = POI_XPRS_SOLAVAILABLE_NOTFOUND,
	OPTIMAL = POI_XPRS_SOLAVAILABLE_OPTIMAL,
	FEASIBLE = POI_XPRS_SOLAVAILABLE_FEASIBLE
};

enum class OPTIMIZETYPE : int
{
	NONE = POI_XPRS_OPTIMIZETYPE_NONE,
	LP = POI_XPRS_OPTIMIZETYPE_LP,
	MIP = POI_XPRS_OPTIMIZETYPE_MIP,
	LOCAL = POI_XPRS_OPTIMIZETYPE_LOCAL,
	GLOBAL = POI_XPRS_OPTIMIZETYPE_GLOBAL
};

////////////////////////////////////////////////////////////////////////////////
//                      CALLBACKS TYPES AND DEFINITIONS                       //
////////////////////////////////////////////////////////////////////////////////

// Callback contexts enum. Defines an unique id for each callback/context.
// The values of this enum class are defined as bitflags, to allow for multiple context manipulation
// using a single uint64 value.
enum class CB_CONTEXT : unsigned long long
{
// Define a enum element
#define XPRSCB_ENUM(ID, NAME, ...) NAME = (1ULL << ID),
	XPRSCB_LIST(XPRSCB_ENUM, XPRSCB_ARG_IGNORE)
#undef XPRSCB_ENUM
};

class Model;                                               // Define later in this file
using Callback = std::function<void(Model *, CB_CONTEXT)>; // Callback opaque container

// xpress::Model operates in two modes to handle callback local XPRSprob objects:
//
// MAIN mode - The model owns its XPRSprob and manages all optimization state normally.
// CALLBACK mode - A temporary, non-owning wrapper around Xpress's thread-local problem clone that
// gets passed to callbacks. Since Xpress creates separate problem clones for callbacks, we need to
// temporarily "borrow" this clone while preserving xpress::Model's internal state
// (variable/constraint indexers, etc.) without copies.
//
// During a callback, we swap the XPRSprob pointer from the main model into the callback pointer,
// execute the user's callback with a full xpress::Model object, then swap it back.
//
// Thread safety: This swap-based design assumes callbacks execute sequentially. Python's GIL
// enforces this anyway (only one callback can execute Python code at a time), and we configure
// Xpress to mutex callbacks invocations, so concurrent callback execution isn't an issue.
enum class XPRESS_MODEL_MODE
{
	MAIN,      // Owns XPRSprob; holds global callback state
	CALLBACK_, // Non-owning wrapper around Xpress's callback problem clone
};

// Simple conditional struct that define the ret_code field only if it is not void
template <typename T>
struct ReturnValue
{
	T ret_code;
	T get_return_value() const
	{
		return ret_code;
	}
};

template <>
struct ReturnValue<void>
{
	void get_return_value() const {};
};

// Since we use the CB macro list, we get the original types passed at the low-level CBs
// This helper struct is used to inject type-exceptions on the callback struct field types. In most
// cases this is not required, except when dealing with pointers to opaque objects, which Nanobind
// doesn't handle automatically.
template <typename T>
struct StructFieldType
{
	using type = T; // Default behavior: just use the original type
};

// (At the best of my knowledge) Nanobind does not support binding for opaque types (pointers to
// declared structs but whose definition is not available in that point). Some callbacks works with
// opaque objects that are passed around (e.g., branch object), we don't need to access them,
// just to pass them back to other Xpress APIs.

// XPRSbranchobject is a pointer to an opaque type, which nanobind struggle with. So we pass it
// as an opaque void*, which nanobind knows how to handle.
template <>
struct StructFieldType<XPRSbranchobject>
{
	using type = void *;
};

template <>
struct StructFieldType<XPRSbranchobject *>
{
	using type = void *;
};

// Callback Argument Structs
//
// PyOptInterface callbacks have a uniform signature: callback(model, context). On the other hand,
// Xpress callbacks have varying signatures with callback-specific arguments (e.g., node IDs,
// branching info, solution status). To bridge this gap, we generate a struct for each callback type
// that holds its specific arguments.
//
// During a callback, we populate the appropriate struct with the current arguments, and we provide
// it to the CB through the "Model.cb_get_arguments" function. The Python callback can then access
// these arguments as named fields.
//
// For example, the 'optnode' callback receives an int* p_infeasible parameter. We generate:
//   struct optnode_struct { int* p_infeasible; };
// The Python callback accesses this as: model.cb_get_arguments().p_infeasible

#define XPRSCB_ARG_STRUCT(TYPE, NAME) StructFieldType<TYPE>::type NAME;
#define XPRSCB_STRUCT(ID, NAME, RET, ...)   \
	struct NAME##_struct : ReturnValue<RET> \
	{                                       \
		__VA_ARGS__                         \
	};
XPRSCB_LIST(XPRSCB_STRUCT, XPRSCB_ARG_STRUCT)
#undef XPRSCB_STRUCT

// Callback Data Variant
//
// We use std::variant to store pointers to callback-specific argument structs, rather than
// a generic void*. This provides two benefits:
//
// 1. Type safety - The variant ensures we can only store pointers to known callback structs,
//    catching type errors at compile time.
//
// 2. Automatic Python binding - Nanobind automatically converts std::variant to Python union
//    types, giving Python callbacks properly-typed access to callback arguments without manual
//    type casting or wrapper code.
//
// The variant contains nullptr_t plus a pointer type for each callback struct (e.g.,
// optnode_struct*, intsol_struct*, etc.)
#define XPRSCB_STRUCT_NAME(ID, NAME, RET, ...) , NAME##_struct *
using xpress_cbs_data =
    std::variant<std::nullptr_t XPRSCB_LIST(XPRSCB_STRUCT_NAME, XPRSCB_ARG_IGNORE)>;
#undef XPRSCB_STRUCT_NAME

// Type-value pair mainly used for NLP formulas
struct Tvp
{
	int type;
	double value;
};

// xpress::Model - Main solver interface for building and solving Xpress optimization models.
// Inherits standard PyOptInterface modeling API through CRTP mixins for constraints, objectives,
// and solution queries.
class Model : public OnesideLinearConstraintMixin<Model>,
              public TwosideLinearConstraintMixin<Model>,
              public OnesideQuadraticConstraintMixin<Model>,
              public TwosideNLConstraintMixin<Model>,
              public LinearObjectiveMixin<Model>,
              public PPrintMixin<Model>,
              public GetValueMixin<Model>
{

  public:
	Model() = default;
	~Model();

	// Avoid involuntary copies
	Model(const Model &) = delete;
	Model &operator=(const Model &) = delete;

	// Move is fine, and we need it to wrap callback XPRSprob
	Model(Model &&) noexcept = default;
	Model &operator=(Model &&) noexcept = default;

	Model(const Env &env);
	void init(const Env &env);
	void close();

	void optimize();
	bool _is_mip();
	static double get_infinity();
	void write(const std::string &filename);
	std::string get_problem_name();
	void set_problem_name(const std::string &probname);
	void add_mip_start(const std::vector<VariableIndex> &variables,
	                   const std::vector<double> &values);
	void *get_raw_model();
	void computeIIS();
	std::string version_string();

	// Index mappings
	int _constraint_index(ConstraintIndex constraint);
	int _variable_index(VariableIndex variable);
	int _checked_constraint_index(ConstraintIndex constraint);
	int _checked_variable_index(VariableIndex variable);

	// Variables
	VariableIndex add_variable(VariableDomain domain = VariableDomain::Continuous,
	                           double lb = POI_XPRS_MINUSINFINITY,
	                           double ub = POI_XPRS_PLUSINFINITY, const char *name = nullptr);
	void delete_variable(VariableIndex variable);
	void delete_variables(const Vector<VariableIndex> &variables);
	void set_objective_coefficient(VariableIndex variable, double value);
	void set_variable_bounds(VariableIndex variable, double lb, double ub);
	void set_variable_lowerbound(VariableIndex variable, double lb);
	void set_variable_name(VariableIndex variable, const char *name);
	void set_variable_type(VariableIndex variable, VariableDomain vtype);
	void set_variable_upperbound(VariableIndex variable, double ub);
	bool is_variable_active(VariableIndex variable);
	bool is_variable_basic(VariableIndex variable);
	bool is_variable_lowerbound_IIS(VariableIndex variable);
	bool is_variable_nonbasic_lb(VariableIndex variable);
	bool is_variable_nonbasic_ub(VariableIndex variable);
	bool is_variable_superbasic(VariableIndex variable);
	bool is_variable_upperbound_IIS(VariableIndex variable);
	double get_objective_coefficient(VariableIndex variable);
	double get_variable_lowerbound(VariableIndex variable);
	double get_variable_primal_ray(VariableIndex variable);
	double get_variable_rc(VariableIndex variable);
	double get_variable_upperbound(VariableIndex variable);
	double get_variable_value(VariableIndex variable);
	std::string get_variable_name(VariableIndex variable);
	std::string pprint_variable(VariableIndex variable);
	VariableDomain get_variable_type(VariableIndex variable);

	// Constraints
	ConstraintIndex add_exp_cone_constraint(const Vector<VariableIndex> &variables,
	                                        const char *name, bool dual);
	ConstraintIndex add_linear_constraint(const ScalarAffineFunction &function,
	                                      const std::tuple<double, double> &interval,
	                                      const char *name);
	ConstraintIndex add_linear_constraint(const ScalarAffineFunction &function,
	                                      ConstraintSense sense, CoeffT rhs,
	                                      const char *name = nullptr);
	ConstraintIndex add_quadratic_constraint(const ScalarQuadraticFunction &function,
	                                         ConstraintSense sense, CoeffT rhs,
	                                         const char *name = nullptr);
	ConstraintIndex add_second_order_cone_constraint(const Vector<VariableIndex> &variables,
	                                                 const char *name, bool rotated);
	ConstraintIndex add_single_nl_constraint(ExpressionGraph &graph, const ExpressionHandle &result,
	                                         const std::tuple<double, double> &interval,
	                                         const char *name = nullptr);
	ConstraintIndex add_sos_constraint(const Vector<VariableIndex> &variables, SOSType sos_type,
	                                   const Vector<CoeffT> &weights);
	ConstraintIndex add_sos_constraint(const Vector<VariableIndex> &variables, SOSType sos_type);
	void delete_constraint(ConstraintIndex constraint);
	void set_constraint_name(ConstraintIndex constraint, const char *name);
	void set_constraint_rhs(ConstraintIndex constraint, CoeffT rhs);
	void set_constraint_sense(ConstraintIndex constraint, ConstraintSense sense);
	void set_normalized_coefficient(ConstraintIndex constraint, VariableIndex variable,
	                                double value);
	void set_normalized_rhs(ConstraintIndex constraint, double value);
	bool is_constraint_active(ConstraintIndex constraint);
	bool is_constraint_basic(ConstraintIndex constraint);
	bool is_constraint_in_IIS(ConstraintIndex constraint);
	bool is_constraint_nonbasic_lb(ConstraintIndex constraint);
	bool is_constraint_nonbasic_ub(ConstraintIndex constraint);
	bool is_constraint_superbasic(ConstraintIndex constraint);
	double get_constraint_dual_ray(ConstraintIndex constraint);
	double get_constraint_dual(ConstraintIndex constraint);
	double get_constraint_slack(ConstraintIndex constraint);
	double get_normalized_coefficient(ConstraintIndex constraint, VariableIndex variable);
	double get_normalized_rhs(ConstraintIndex constraint);
	CoeffT get_constraint_rhs(ConstraintIndex constraint);
	std::string get_constraint_name(ConstraintIndex constraint);
	ConstraintSense get_constraint_sense(ConstraintIndex constraint);

	// Objective function
	void set_objective(const ScalarAffineFunction &function, ObjectiveSense sense);
	void set_objective(const ScalarQuadraticFunction &function, ObjectiveSense sense);
	void set_objective(const ExprBuilder &function, ObjectiveSense sense);
	void add_single_nl_objective(ExpressionGraph &graph, const ExpressionHandle &result);

	// Xpress->POI exit status mappings
	LPSTATUS get_lp_status();
	MIPSTATUS get_mip_status();
	NLPSTATUS get_nlp_status();
	SOLSTATUS get_sol_status();
	SOLVESTATUS get_solve_status();
	OPTIMIZETYPE get_optimize_type();
	IISSOLSTATUS get_iis_sol_status();

	// Native Attribute/Control access using integer IDs
	void set_raw_control_dbl_by_id(int control, double value);
	void set_raw_control_int_by_id(int control, XPRSint64 value);
	void set_raw_control_str_by_id(int control, const char *value);
	XPRSint64 get_raw_control_int_by_id(int control);
	double get_raw_control_dbl_by_id(int control);
	std::string get_raw_control_str_by_id(int control);

	XPRSint64 get_raw_attribute_int_by_id(int attrib);
	double get_raw_attribute_dbl_by_id(int attrib);
	std::string get_raw_attribute_str_by_id(int attrib);

	// Attribute/Control access through string IDs (converted to integer IDs)
	void set_raw_control_int(const char *control, XPRSint64 value);
	void set_raw_control_dbl(const char *control, double value);
	void set_raw_control_str(const char *control, const char *value);
	XPRSint64 get_raw_control_int(const char *control);
	double get_raw_control_dbl(const char *control);
	std::string get_raw_control_str(const char *control);

	XPRSint64 get_raw_attribute_int(const char *attrib);
	double get_raw_attribute_dbl(const char *attrib);
	std::string get_raw_attribute_str(const char *attrib);

	// Type generic Attribute/Control access through string IDs
	using xprs_type_variant_t = std::variant<int, XPRSint64, double, std::string>;
	void set_raw_control(const char *control, xprs_type_variant_t &value);
	xprs_type_variant_t get_raw_attribute(const char *attrib);
	xprs_type_variant_t get_raw_control(const char *control);

	// Callback
	void set_callback(const Callback &callback, unsigned long long cbctx);
	xpress_cbs_data cb_get_arguments();

	double cb_get_solution(VariableIndex variable);
	double cb_get_relaxation(VariableIndex variable);
	double cb_get_incumbent(VariableIndex variable);
	void cb_set_solution(VariableIndex variable, double value);
	void cb_submit_solution();

	void cb_exit();

	// NOTE: Xpress only provide ways to add local cuts, so, all these functions map to the same
	// XPRSaddcuts operation
	void cb_add_lazy_constraint(const ScalarAffineFunction &function, ConstraintSense sense,
	                            CoeffT rhs);
	void cb_add_lazy_constraint(const ExprBuilder &function, ConstraintSense sense, CoeffT rhs);
	void cb_add_user_cut(const ScalarAffineFunction &function, ConstraintSense sense, CoeffT rhs);
	void cb_add_user_cut(const ExprBuilder &function, ConstraintSense sense, CoeffT rhs);

  private: // HELPER FUNCTIONS
	void _check(int error);
	void _clear_caches();

	// Modeling helpers
	void _set_entity_name(int etype, int index, const char *name);
	ConstraintIndex _add_sos_constraint(const Vector<VariableIndex> &variables, SOSType sos_type,
	                                    const Vector<double> &weights);
	std::string _get_entity_name(int etype, int eidx);
	char _get_variable_bound_IIS(VariableIndex variable);
	int _get_basis_stat(int entity_idx, bool is_row = false);

	// NLP helpers
	Tvp _decode_expr(const ExpressionGraph &graph, const ExpressionHandle &expr);
	std::pair<std::vector<int>, std::vector<double>> _decode_graph_postfix_order(
	    ExpressionGraph &graph, const ExpressionHandle &result);

	// Attributes/Controls helpers
	int _get_checked_attribute_id(const char *attrib, CATypes expected,
	                              CATypes backup = CATypes::NOTDEFINED);
	int _get_checked_control_id(const char *control, CATypes expected,
	                            CATypes backup = CATypes::NOTDEFINED);
	std::pair<int, CATypes> _get_attribute_info(const char *attrib);
	std::pair<int, CATypes> _get_control_info(const char *control);

	// Callback/Mode helpers
	void _check_expected_mode(XPRESS_MODEL_MODE mode);
	void _ensure_postsolved();
	double _cb_get_context_solution(VariableIndex variable);
	void _cb_add_cut(const ScalarAffineFunction &function, ConstraintSense sense, CoeffT rhs);
	XPRSprob _toggle_model_mode(XPRSprob model);

  private: // TYPES
	// Helper struct used to spawn lower level C callbacks with matching signature
	template <unsigned long long, typename, typename, typename...>
	struct CbWrap;

	// XPRSprob deleter to automatically RAII using unique_ptr
	struct ProbDeleter
	{
		void operator()(XPRSprob prob)
		{
			if (xpress::XPRSdestroyprob(prob) != 0)
			{
				throw std::runtime_error("Error while destroying Xpress problem object");
			}
		}
	};

  private: // MEMBER VARIABLES
	// Xpress problem pointer
	std::unique_ptr<xo_prob_struct, ProbDeleter> m_model;

	// Current operating mode (MAIN or CALLBACK) - enables runtime validation that
	// callback-only methods aren't called outside callbacks and vice versa
	XPRESS_MODEL_MODE m_mode;

	// Index management for variables and constraints
	// Note that Xpress uses the same index set to refer to almost all its constraint types. The
	// only exception are SOS which are handled in a separate pool.
	MonotoneIndexer<int> m_variable_index;
	MonotoneIndexer<int> m_constraint_index;
	MonotoneIndexer<int> m_sos_constraint_index;

	// Tracks whether the model requires postsolving before queries can be made.
	// When optimization is interrupted, the model remains in a presolved state where variable
	// and constraint indices don't match the original problem structure. Querying model data
	// in this state would return incorrect results. Postsolving restores the original index
	// mappings but discards all optimization progress, forcing any subsequent optimization
	// to restart from scratch.
	bool m_need_postsolve = false;

	// Any non-linear or non-convex quadratic constraint is handled with Xpress non linear solver.
	// We keep count of all the non-linear (strictly speaking) constraints.
	int m_quad_nl_constr_num = 0;
	bool has_quad_objective = false;

	bool has_nlp_objective = false;
	VariableIndex m_nlp_obj_variable;
	ConstraintIndex m_nlp_obj_constraint;

	// Cached vectors
	std::vector<double> m_primal_ray;
	std::vector<double> m_dual_ray;
	std::vector<int> m_iis_cols;
	std::vector<int> m_iss_rows;
	std::vector<char> m_iis_bound_types;

	// Message callback state - we register a default handler but allow user override
	bool is_default_message_cb_set;

	// User callback and active contexts
	Callback m_callback = nullptr;
	unsigned long long m_curr_contexts = 0; // Bitwise OR of enabled callback contexts

	// Exception propagation - if a callback throws, we capture it here to let Xpress
	// complete cleanup gracefully before rethrowing to Python
	std::vector<std::exception_ptr> m_captured_exceptions;

	// Current callback context being executed
	CB_CONTEXT cb_where;

	// Heuristic solution builder - accumulates (variable, value) pairs during callback;
	// submitted to Xpress when callback completes
	std::vector<std::pair<int, double>> cb_sol_cache;

	// Callback-specific arguments - variant holding pointers to context-specific structs
	// (e.g., optnode_struct*, intsol_struct*) based on current callback type
	xpress_cbs_data cb_args = nullptr;
};
} // namespace xpress
