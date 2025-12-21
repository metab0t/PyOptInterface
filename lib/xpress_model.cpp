#include "pyoptinterface/xpress_model.hpp"
#include "fmt/core.h"
#include "pyoptinterface/core.hpp"

#include <concepts>
#include <cstdio>
#include <exception>
#include <mutex>
#include <stack>
#include <stdexcept>
#ifndef _WIN32
#include <unistd.h>
#endif

namespace xpress
{

// Simple helper to have a Go-like defer functionality.
// Mainly used to move resource release closer to its acquisition.
template <typename LmdT>
struct Defer : LmdT
{
	Defer(LmdT l) : LmdT(l) {};
	~Defer() noexcept
	{
		(*this)();
	}
};

// Mimics what is done to load the other solvers.
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
		int major = {};
		int minor = {};
		int build = {};
		XPRSgetversionnumbers(&major, &minor, &build);
		// Use tuple comparison operator
		if (major < XPRS_VER_MAJOR)
		{
			fmt::print(
			    stderr,
			    "Warning: loaded Xpress version is older than the officially supported one.\n");
		}
	}
	return is_loaded;
}

static void check_license(int error)
{
	if (error == 0)
	{
		return;
	}

	char buffer[POI_XPRS_MAXMESSAGELENGTH];
	if (XPRSgetlicerrmsg(buffer, sizeof buffer) != 0)
	{
		throw std::runtime_error("Error while getting the Xpress license error message");
	}
	throw std::runtime_error(
	    fmt::format("Error while initializing Xpress Environment: {}", buffer));
}

Env::Env(const char *path)
{
	if (!xpress::is_library_loaded())
	{
		throw std::runtime_error("Xpress library is not loaded");
	}

	auto lg = std::lock_guard(mtx);
	assert(init_count >= 0);

	if (init_count <= 0)
	{
		check_license(XPRSinit(path));
	}
	++init_count;
	initialized = true;
}

Env::~Env()
{
	try
	{
		close();
	}
	catch (std::exception e)
	{
		fmt::print(stderr, "{}\n", e.what());
		fflush(stderr);
	}
}

void Env::close()
{
	if (!initialized)
	{
		return;
	}
	initialized = false;

	auto lg = std::lock_guard(mtx);
	--init_count;
	assert(init_count >= 0);

	if (init_count <= 0 && XPRSfree() != 0)
	{
		throw std::runtime_error("Error while freeing Xpress environment");
	}
}

std::pair<int, std::string> license(int p_i, const char *p_c)
{
	int i = p_i;
	std::string c(p_c);
	check_license(XPRSlicense(&i, c.data()));
	c.resize(strlen(c.data()));
	return std::make_pair(i, c);
}

bool beginlicensing()
{
	int notyet = {};
	check_license(XPRSbeginlicensing(&notyet));
	return notyet != 0;
}

void endlicensing()
{
	check_license(XPRSendlicensing());
}

static char poi_to_xprs_cons_sense(ConstraintSense sense)
{
	switch (sense)
	{
	case ConstraintSense::LessEqual:
		return 'L';
	case ConstraintSense::Equal:
		return 'E';
	case ConstraintSense::GreaterEqual:
		return 'G';
	default:
		throw std::runtime_error("Unknown constraint sense");
	}
}

static ConstraintSense xprs_to_poi_cons_sense(int ctype)
{
	switch (ctype)
	{
	case 'L':
		return ConstraintSense::LessEqual;
	case 'E':
		return ConstraintSense::Equal;
	case 'G':
		return ConstraintSense::GreaterEqual;
	case 'R': // Range constraints
	case 'N': // Free constraints
	default:
		throw std::runtime_error("Unsupported constraint sense");
	}
}
static int poi_to_xprs_obj_sense(ObjectiveSense sense)
{
	switch (sense)
	{
	case ObjectiveSense::Minimize:
		return POI_XPRS_OBJ_MINIMIZE;
	case ObjectiveSense::Maximize:
		return POI_XPRS_OBJ_MAXIMIZE;
	default:
		throw std::runtime_error("Unknown objective function sense");
	}
}

static char poi_to_xprs_var_type(VariableDomain domain)
{
	switch (domain)
	{
	case VariableDomain::Continuous:
		return 'C';
	case VariableDomain::Integer:
		return 'I';
	case VariableDomain::Binary:
		return 'B';
	case VariableDomain::SemiContinuous:
		return 'S';
	default:
		throw std::runtime_error("Unknown variable domain");
	}
}

static VariableDomain xprs_to_poi_var_type(char vtype)
{
	switch (vtype)
	{
	case 'C':
		return VariableDomain::Continuous;
	case 'I':
		return VariableDomain::Integer;
	case 'B':
		return VariableDomain::Binary;
	case 'S':
		return VariableDomain::SemiContinuous;
	default:
		throw std::runtime_error("Unknown variable domain");
	}
}

static char poi_to_xprs_sos_type(SOSType type)
{
	switch (type)
	{
	case SOSType::SOS1:
		return '1';
	case SOSType::SOS2:
		return '2';
	default:
		throw std::runtime_error("Unknown SOS type");
	}
}

// Check Xpress APIs return value for error and throws in case it is non-zero
void Model::_check(int error)
{
	// We allow users to define custom message callbacks which may throw exceptions. Since
	// Xpress APIs can trigger messages at any point, exceptions might be thrown even from
	// apparently "safe" operations. We always check for captured exceptions before returning.
	if (m_mode == XPRESS_MODEL_MODE::MAIN && !m_captured_exceptions.empty())
	{
		Defer exceptions_clear = [&] { m_captured_exceptions.clear(); };

		// Single exception - rethrow directly
		if (m_captured_exceptions.size() == 1)
		{
			std::rethrow_exception(m_captured_exceptions[0]);
		}

		// Multiple exceptions - aggregate into single message
		std::string new_what = "Multiple exceptions raised:\n";
		for (int i = 0; const auto &exc : m_captured_exceptions)
		{
			try
			{
				std::rethrow_exception(exc);
			}
			catch (const std::exception &e)
			{
				fmt::format_to(std::back_inserter(new_what), "{}. {}\n", ++i, e.what());
			}
			catch (...)
			{
				fmt::format_to(std::back_inserter(new_what), "{}. Unknown exception\n", ++i);
			}
		}
		throw std::runtime_error(new_what);
	}

	if (error == 0)
	{
		return;
	}

	char error_buffer[POI_XPRS_MAXMESSAGELENGTH];
	if (XPRSgetlasterror(m_model.get(), error_buffer) != 0)
	{
		throw std::runtime_error("Error while getting Xpress message error");
	}
	throw std::runtime_error(error_buffer);
}

// The default behavior of Xpress C APIs is to don't print anything unless a message CB is
// registered. Thus, this is the default print callback that redirect to standard streams.
static void default_print(XPRSprob prob, void *, char const *msg, int msgsize, int msgtype)
{
	if (msgtype < 0)
	{
		// Negative values are used to signal output end, and can be use as flush trigger
		// But we flush at every message, so no problem need to flush again.
		return;
	}

	FILE *out = (msgtype == 1 ? stdout : stderr);
	fmt::print(out, "{}\n", msgsize > 0 ? msg : "");
	fflush(out);
}

Model::Model(const Env &env)
{
	init(env);

	// The default behavior expected by POI differ a bit from Xpress default behavior. Here we
	// adjust some controls:

	// Verbose by default, the user can silence if needed
	set_raw_control_int_by_id(POI_XPRS_OUTPUTLOG, 1);

	// Register a message callback (can be overridden)
	_check(XPRSaddcbmessage(m_model.get(), &default_print, nullptr, 0));
	is_default_message_cb_set = true;

	// We do not support concurrent CBs invocation since each callback have to acquire Python GIL
	_check(XPRSsetintcontrol64(m_model.get(), POI_XPRS_MUTEXCALLBACKS, 1));

	// Use global solver if the model contains non linear formulas
	set_raw_control_int_by_id(POI_XPRS_NLPSOLVER, POI_XPRS_NLPSOLVER_GLOBAL);
}

void Model::init(const Env &env)
{
	if (!xpress::is_library_loaded())
	{
		throw std::runtime_error("Xpress library is not loaded");
	}
	if (auto lg = std::lock_guard(Env::mtx); Env::init_count <= 0)
	{
		throw std::runtime_error("Xpress environment is not initialized");
	}
	XPRSprob prob = nullptr;
	_check(XPRScreateprob(&prob));
	m_model.reset(prob);
	_clear_caches();
}

XPRSprob Model::_toggle_model_mode(XPRSprob model)
{
	if (m_mode == XPRESS_MODEL_MODE::MAIN)
	{
		m_mode = XPRESS_MODEL_MODE::CALLBACK_;
	}
	else
	{
		m_mode = XPRESS_MODEL_MODE::MAIN;
	}
	XPRSprob old = m_model.release();
	m_model.reset(model);
	return old;
}

Model::~Model()
try
{
	close();
}
catch (std::exception e)
{
	fmt::print(stderr, "{}\n", e.what());
	fflush(stderr);
}

void Model::close()
{
	// In CALLBACK mode we cannot destroy the problem, we release the unique_ptr instead
	if (m_mode == XPRESS_MODEL_MODE::CALLBACK_)
	{
		[[maybe_unused]] auto _ = m_model.release();
	}
	else
	{
		m_model.reset();
	}
}

void Model::_clear_caches()
{
	m_primal_ray.clear();
	m_dual_ray.clear();
	m_iis_cols.clear();
	m_iss_rows.clear();
	m_iis_bound_types.clear();
}

double Model::get_infinity()
{
	return POI_XPRS_PLUSINFINITY;
}

void Model::write(const std::string &filename)
{
	// Detect if the file should be compressed by looking at the last file
	// extension. We exploit short-circuiting and fold expressions to avoid long
	// else if branches.
	auto find_compress_ext_len = [&](auto &...extensions) {
		size_t ext_len = 0;
		((filename.ends_with(extensions) && (ext_len = sizeof extensions - 1)) || ...);
		return ext_len;
	};
	size_t compress_ext_len = find_compress_ext_len(".gz", ".zip", ".tar", ".tgz", ".bz2", ".bzip",
	                                                ".7z", ".xz", ".lz4", ".Z");
	std::string_view fname = filename;
	fname.remove_suffix(compress_ext_len);

	// Based on the second last extension, we deduce what the user wants.
	if (fname.ends_with(".mps"))
	{
		_check(XPRSwriteprob(m_model.get(), filename.c_str(), "v"));
	}
	else if (fname.ends_with(".lp"))
	{
		_check(XPRSwriteprob(m_model.get(), filename.c_str(), "lv"));
	}
	else if (fname.ends_with(".bss"))
	{
		_check(XPRSwritebasis(m_model.get(), filename.c_str(), "v"));
	}
	else if (fname.ends_with(".hdr") || fname.ends_with(".asc"))
	{
		_check(XPRSwritesol(m_model.get(), filename.c_str(), "v"));
	}
	else if (fname.ends_with(".sol"))
	{
		_check(XPRSwritebinsol(m_model.get(), filename.c_str(), "v"));
	}
	else if (fname.ends_with(".prt"))
	{
		_check(XPRSwriteprtsol(m_model.get(), filename.c_str(), "v"));
	}
	else if (fname.ends_with(".slx"))
	{
		_check(XPRSwriteslxsol(m_model.get(), filename.c_str(), "v"));
	}
	else if (fname.ends_with(".svf"))
	{
		_check(XPRSsaveas(m_model.get(), filename.c_str()));
	}
	else
	{
		throw std::runtime_error("Unknow file extension");
	}
}

std::string Model::get_problem_name()
{
	int size = get_raw_attribute_int_by_id(POI_XPRS_MAXPROBNAMELENGTH) + 1;
	std::string probname;
	probname.resize(size);
	_check(XPRSgetprobname(m_model.get(), probname.data()));
	// Align string size with string length
	probname.resize(strlen(probname.c_str()));
	return probname;
}

void Model::set_problem_name(const std::string &probname)
{
	_check(XPRSsetprobname(m_model.get(), probname.c_str()));
}

VariableIndex Model::add_variable(VariableDomain domain, double lb, double ub, const char *name)
{
	_check_expected_mode(XPRESS_MODEL_MODE::MAIN);
	_ensure_postsolved();
	_clear_caches();

	IndexT index = m_variable_index.add_index();
	VariableIndex variable(index);

	double zero[] = {0.0};
	int colidx = get_raw_attribute_int_by_id(POI_XPRS_COLS);
	_check(XPRSaddcols64(m_model.get(), 1, 0, zero, nullptr, nullptr, nullptr, &lb, &ub));
	_set_entity_name(POI_XPRS_NAMES_COLUMN, colidx, name);
	char vtype = poi_to_xprs_var_type(domain);
	if (domain != VariableDomain::Continuous)
	{
		char vtype = poi_to_xprs_var_type(domain);
		int icol = colidx;
		_check(XPRSchgcoltype(m_model.get(), 1, &icol, &vtype));
	}
	return variable;
}

void Model::delete_variable(VariableIndex variable)
{
	_check_expected_mode(XPRESS_MODEL_MODE::MAIN);
	_ensure_postsolved();
	_clear_caches();

	if (!is_variable_active(variable))
	{
		throw std::runtime_error("Variable does not exist");
	}
	int colidx = _variable_index(variable);
	_check(XPRSdelcols(m_model.get(), 1, &colidx));
	m_variable_index.delete_index(variable.index);
}

void Model::delete_variables(const Vector<VariableIndex> &variables)
{
	_check_expected_mode(XPRESS_MODEL_MODE::MAIN);
	_ensure_postsolved();
	_clear_caches();

	int n_variables = variables.size();
	if (n_variables == 0)
		return;

	std::vector<int> columns;
	columns.reserve(n_variables);
	for (int i = {}; i < n_variables; i++)
	{
		if (!is_variable_active(variables[i]))
		{
			continue;
		}
		auto column = _variable_index(variables[i]);
		columns.push_back(column);
	}
	_check(XPRSdelcols(m_model.get(), columns.size(), columns.data()));

	for (int i = {}; i < n_variables; i++)
	{
		m_variable_index.delete_index(variables[i].index);
	}
}

bool Model::is_variable_active(VariableIndex variable)
{
	return m_variable_index.has_index(variable.index);
}

std::string Model::pprint_variable(VariableIndex variable)
{
	_check_expected_mode(XPRESS_MODEL_MODE::MAIN);
	_ensure_postsolved();

	return get_variable_name(variable);
}

std::string Model::_get_entity_name(int etype, int eidx)
{
	_check_expected_mode(XPRESS_MODEL_MODE::MAIN);
	_ensure_postsolved();

	int req_size = {};
	_check(XPRSgetnamelist(m_model.get(), etype, nullptr, 0, &req_size, eidx, eidx));
	std::string value = {};
	value.resize(req_size);
	_check(XPRSgetnamelist(m_model.get(), etype, value.data(), req_size, &req_size, eidx, eidx));
	while (value.back() == '\0')
	{
		value.pop_back();
	}
	return value;
}

std::string Model::get_variable_name(VariableIndex variable)
{
	int colidx = _checked_variable_index(variable);
	return _get_entity_name(POI_XPRS_NAMES_COLUMN, colidx);
}

std::string Model::get_constraint_name(ConstraintIndex constraint)
{
	int rowidx = _checked_constraint_index(constraint);
	return _get_entity_name(POI_XPRS_NAMES_ROW, rowidx);
}

void Model::set_variable_bounds(VariableIndex variable, double lb, double ub)
{
	_check_expected_mode(XPRESS_MODEL_MODE::MAIN);
	_ensure_postsolved();
	_clear_caches();

	int column = _checked_variable_index(variable);
	int columns[] = {column, column};
	char btypes[] = "LU";
	double bounds[] = {lb, ub};
	_check(XPRSchgbounds(m_model.get(), 2, columns, btypes, bounds));
}

void Model::set_variable_lowerbound(VariableIndex variable, double lb)
{
	_check_expected_mode(XPRESS_MODEL_MODE::MAIN);
	_ensure_postsolved();
	_clear_caches();

	int colidx = _checked_variable_index(variable);
	_check(XPRSchgbounds(m_model.get(), 1, &colidx, "L", &lb));
}

void Model::set_variable_upperbound(VariableIndex variable, double ub)
{
	_check_expected_mode(XPRESS_MODEL_MODE::MAIN);
	_ensure_postsolved();
	_clear_caches();

	int colidx = _checked_variable_index(variable);
	_check(XPRSchgbounds(m_model.get(), 1, &colidx, "U", &ub));
}

void Model::set_variable_type(VariableIndex variable, VariableDomain vdomain)
{
	_check_expected_mode(XPRESS_MODEL_MODE::MAIN);
	_ensure_postsolved();
	_clear_caches();

	int colidx = _checked_variable_index(variable);
	char vtype = poi_to_xprs_var_type(vdomain);
	_check(XPRSchgcoltype(m_model.get(), 1, &colidx, &vtype));
}

void Model::_set_entity_name(int etype, int index, const char *name)
{
	if (name == nullptr || name[0] == '\0')
	{
		return;
	}
	_check(XPRSaddnames(m_model.get(), etype, name, index, index));
}

void Model::add_mip_start(const std::vector<VariableIndex> &variables,
                          const std::vector<double> &values)
{
	_check_expected_mode(XPRESS_MODEL_MODE::MAIN);
	if (variables.size() != values.size())
	{
		throw std::runtime_error("Number of variables and values do not match");
	}
	int numnz = variables.size();
	if (numnz == 0)
	{
		return;
	}

	std::vector<int> ind_v(numnz);
	for (int i = {}; i < numnz; i++)
	{
		ind_v[i] = _checked_variable_index(variables[i].index);
	}
	_check(XPRSaddmipsol(m_model.get(), numnz, values.data(), ind_v.data(), nullptr));
}

void Model::set_variable_name(VariableIndex variable, const char *name)
{
	_check_expected_mode(XPRESS_MODEL_MODE::MAIN);
	_ensure_postsolved();

	int column = _checked_variable_index(variable);
	_set_entity_name(POI_XPRS_NAMES_COLUMN, column, name);
}

void Model::set_constraint_name(ConstraintIndex constraint, const char *name)
{
	_check_expected_mode(XPRESS_MODEL_MODE::MAIN);
	_ensure_postsolved();

	int row = _checked_constraint_index(constraint);
	_set_entity_name(POI_XPRS_NAMES_ROW, row, name);
}

ConstraintIndex Model::add_linear_constraint(const ScalarAffineFunction &function,
                                             const std::tuple<double, double> &interval,
                                             const char *name)
{
	_check_expected_mode(XPRESS_MODEL_MODE::MAIN);
	_ensure_postsolved();
	_clear_caches();

	auto [lb, ub] = interval;
	double constant = static_cast<double>(function.constant.value_or(CoeffT{}));
	lb = std::clamp(lb - constant, POI_XPRS_MINUSINFINITY, POI_XPRS_PLUSINFINITY);
	ub = std::clamp(ub - constant, POI_XPRS_MINUSINFINITY, POI_XPRS_PLUSINFINITY);
	if (lb > ub - 1e-10)
	{
		throw std::runtime_error("LB > UB in the provieded interval.");
	}

	// Handle infinity bounds
	bool lb_inf = lb <= POI_XPRS_MINUSINFINITY;
	bool ub_inf = ub >= POI_XPRS_PLUSINFINITY;

	// Determine constraint type and parameters
	char g_sense = {};
	double g_rhs = {};
	double range_val = {};
	const double *g_range = {};

	if (lb_inf && ub_inf)
	{
		g_sense = 'N'; // Free row
		g_rhs = 0.0;
	}
	else if (lb_inf)
	{
		g_sense = 'L';
		g_rhs = ub;
	}
	else if (ub_inf)
	{
		g_sense = 'G';
		g_rhs = lb;
	}
	else if (std::abs(ub - lb) < 1e-10)
	{
		g_sense = 'E';
		g_rhs = ub;
	}
	else
	{
		g_sense = 'R';
		g_rhs = ub;
		range_val = ub - lb;
		g_range = &range_val;
	}

	IndexT index = m_constraint_index.add_index();
	ConstraintIndex constraint_index(ConstraintType::Linear, index);
	int rowidx = get_raw_attribute_int_by_id(POI_XPRS_ROWS);

	AffineFunctionPtrForm<int, int, double> ptr_form;
	ptr_form.make(this, function);
	int numnz = ptr_form.numnz;
	XPRSint64 beg[] = {0};
	const int *cind = ptr_form.index;
	const double *cval = ptr_form.value;

	_check(XPRSaddrows64(m_model.get(), 1, numnz, &g_sense, &g_rhs, g_range, beg, cind, cval));
	_set_entity_name(POI_XPRS_NAMES_ROW, rowidx, name);

	return constraint_index;
}

ConstraintIndex Model::add_linear_constraint(const ScalarAffineFunction &function,
                                             ConstraintSense sense, CoeffT rhs, const char *name)
{
	_check_expected_mode(XPRESS_MODEL_MODE::MAIN);
	_ensure_postsolved();
	_clear_caches();

	IndexT index = m_constraint_index.add_index();
	ConstraintIndex constraint_index(ConstraintType::Linear, index);
	int rowidx = get_raw_attribute_int_by_id(POI_XPRS_ROWS);

	AffineFunctionPtrForm<int, int, double> ptr_form;
	ptr_form.make(this, function);
	int numnz = ptr_form.numnz;

	double g_rhs = static_cast<double>(rhs - function.constant.value_or(CoeffT{}));
	g_rhs = std::clamp(g_rhs, POI_XPRS_MINUSINFINITY, POI_XPRS_PLUSINFINITY);

	// Map expr >= -inf and expr <= +inf to free rows
	char g_sense = poi_to_xprs_cons_sense(sense);
	if ((g_sense == 'G' && g_rhs <= POI_XPRS_MINUSINFINITY) ||
	    (g_sense == 'L' && g_rhs >= POI_XPRS_PLUSINFINITY))
	{
		g_sense = 'N'; // Free row
		g_rhs = 0.0;
	}

	XPRSint64 beg[] = {0};
	const int *cind = ptr_form.index;
	const double *cval = ptr_form.value;
	_check(XPRSaddrows64(m_model.get(), 1, numnz, &g_sense, &g_rhs, nullptr, beg, cind, cval));
	_set_entity_name(POI_XPRS_NAMES_ROW, rowidx, name);

	return constraint_index;
}

static QuadraticFunctionPtrForm<int, int, double> poi_to_xprs_quad_formula(
    Model &model, const ScalarQuadraticFunction &function, bool is_objective)
{
	QuadraticFunctionPtrForm<int, int, double> ptr_form;
	ptr_form.make(&model, function);
	int numqnz = ptr_form.numnz;

	// Xpress uses different quadratic representations for objectives vs constraints:
	//
	// OBJECTIVES: Use 0.5*x'Qx form with automatic symmetry.
	//   - Diagonal terms (i,i): Need 2× multiplication to compensate for the 0.5 factor
	//   - Off-diagonal terms (i,j): Used as-is; Xpress automatically mirrors to (j,i)
	//
	// CONSTRAINTS: Use x'Qx form with upper-triangular specification.
	//   - Diagonal terms (i,i): Used as-is
	//   - Off-diagonal terms (i,j): Need 0.5× division; Xpress expects coefficients
	//     for the upper triangular part only, which will be mirrored
	//
	// PyOptInterface provides coefficients in the standard x'Qx form through
	// QuadraticFunctionPtrForm, so we adjust based on whether this is for an objective function or
	// a constraint.

	// Copy coefficients (ptr_form.value may reference function.coefficients directly)
	if (ptr_form.value_storage.empty())
	{
		ptr_form.value_storage.reserve(numqnz);
		for (CoeffT c : function.coefficients)
		{
			ptr_form.value_storage.push_back(static_cast<double>(c));
		}
	}

	// Apply Xpress-specific coefficient adjustments
	for (int i = 0; i < numqnz; ++i)
	{
		if (is_objective && (ptr_form.row[i] == ptr_form.col[i]))
		{
			// Objective diagonal terms: multiply by 2 for 0.5*x'Qx convention
			ptr_form.value_storage[i] *= 2.0;
		}
		if (!is_objective && (ptr_form.row[i] != ptr_form.col[i]))
		{
			// Constraint off-diagonal terms: divide by 2 for upper-triangular specification
			ptr_form.value_storage[i] /= 2.0;
		}
	}

	ptr_form.value = ptr_form.value_storage.data();
	return ptr_form;
}

// Quadratic constraints are regular rows with a quadratic term.
ConstraintIndex Model::add_quadratic_constraint(const ScalarQuadraticFunction &function,
                                                ConstraintSense sense, CoeffT rhs, const char *name)
{
	_check_expected_mode(XPRESS_MODEL_MODE::MAIN);
	_ensure_postsolved();
	_clear_caches();

	int rowidx = get_raw_attribute_int_by_id(POI_XPRS_ROWS);

	const auto &affine_part = function.affine_part.value_or(ScalarAffineFunction{});
	ConstraintIndex constraint_index = add_linear_constraint(affine_part, sense, rhs, name);
	constraint_index.type = ConstraintType::Quadratic; // Fix constraint type

	// Add quadratic term
	QuadraticFunctionPtrForm<int, int, double> ptr_form =
	    poi_to_xprs_quad_formula(*this, function, false);
	int numqnz = ptr_form.numnz;
	const int *qrow = ptr_form.row;
	const int *qcol = ptr_form.col;
	const double *qval = ptr_form.value;

	_check(XPRSaddqmatrix64(m_model.get(), rowidx, numqnz, qrow, qcol, qval));
	++m_quad_nl_constr_num;
	return constraint_index;
}

ConstraintIndex Model::add_second_order_cone_constraint(const Vector<VariableIndex> &variables,
                                                        const char *name, bool rotated)
{
	_check_expected_mode(XPRESS_MODEL_MODE::MAIN);
	_ensure_postsolved();
	_clear_caches();

	int rot_offset = rotated ? 1 : 0;
	if (variables.size() <= rot_offset)
	{
		throw std::runtime_error("Not enough variables in SOC constraint.");
	}

	// SOC:           1 x_0 x_0 >= \sum_{i : [1,N)} x_i^2
	// Rotated SOC:   2 x_0 x_1 >= \sum_{i : [2,N)} x_i^2
	ScalarQuadraticFunction quadconstr;
	quadconstr.add_quadratic_term(variables[0], variables[rot_offset], 1.0 + rot_offset);
	for (int i = 1 + rot_offset; i < variables.size(); ++i)
	{
		quadconstr.add_quadratic_term(variables[i], variables[i], -1.0);
	}
	ConstraintIndex constraint_index =
	    add_quadratic_constraint(quadconstr, ConstraintSense::GreaterEqual, 0.0, name);
	constraint_index.type = ConstraintType::Cone;
	return constraint_index;
}

namespace
{
template <size_t N>
struct NlpArrays
{
	int types[N];
	double values[N];
};
} // namespace

ConstraintIndex Model::add_exp_cone_constraint(const Vector<VariableIndex> &variables,
                                               const char *name, bool dual)
{
	_check_expected_mode(XPRESS_MODEL_MODE::MAIN);
	_ensure_postsolved();
	_clear_caches();

	if (variables.size() != 3)
	{
		throw std::runtime_error("Exponential cone constraint must have 3 variables");
	}

	// Add affine part
	auto constraint_index =
	    add_linear_constraint_from_var(variables[0], ConstraintSense::GreaterEqual, 0.0);
	constraint_index.type = ConstraintType::Xpress_Nlp; // Fix constraint type

	int nnz = 0;
	const int *types = {};
	const double *values = {};
	double var1_idx = static_cast<double>(_checked_variable_index(variables[1]));
	double var2_idx = static_cast<double>(_checked_variable_index(variables[2]));

	int rowidx = _constraint_index(constraint_index);

	// Syntactic sugar to make hand written NLP formulas more readable
	auto make_type_value_arrays = [](auto... terms) {
		return NlpArrays<sizeof...(terms)>{{terms.type...}, {terms.value...}};
	};
	if (dual)
	{
		// linear_term + x_2 * exp(x_1 / x_2 - 1) >= 0
		auto [types, values] = make_type_value_arrays(  //
		    Tvp{POI_XPRS_TOK_COL, var2_idx},            // x_2
		    Tvp{POI_XPRS_TOK_RB, {}},                   // )
		    Tvp{POI_XPRS_TOK_COL, var1_idx},            // x_1
		    Tvp{POI_XPRS_TOK_COL, var2_idx},            // x_2
		    Tvp{POI_XPRS_TOK_OP, POI_XPRS_OP_DIVIDE},   // /
		    Tvp{POI_XPRS_TOK_CON, 1.0},                 // 1.0
		    Tvp{POI_XPRS_TOK_OP, POI_XPRS_OP_MINUS},    // -
		    Tvp{POI_XPRS_TOK_IFUN, POI_XPRS_IFUN_EXP},  // exp(
		    Tvp{POI_XPRS_TOK_OP, POI_XPRS_OP_MULTIPLY}, // *
		    Tvp{POI_XPRS_TOK_EOF, {}});                 // EOF

		int begs[] = {0, std::ssize(types)};
		_check(XPRSnlpaddformulas(m_model.get(), 1, &rowidx, begs, 1, types, values));
	}
	else
	{
		// linear_term - x_1 * exp(x_2 / x_1) >= 0
		auto [types, values] = make_type_value_arrays(  //
		    Tvp{POI_XPRS_TOK_COL, var1_idx},            // x_1
		    Tvp{POI_XPRS_TOK_RB, {}},                   // )
		    Tvp{POI_XPRS_TOK_COL, var2_idx},            // x_2
		    Tvp{POI_XPRS_TOK_COL, var1_idx},            // x_1
		    Tvp{POI_XPRS_TOK_OP, POI_XPRS_OP_DIVIDE},   // /
		    Tvp{POI_XPRS_TOK_IFUN, POI_XPRS_IFUN_EXP},  // exp(
		    Tvp{POI_XPRS_TOK_OP, POI_XPRS_OP_MULTIPLY}, // *
		    Tvp{POI_XPRS_TOK_OP, POI_XPRS_OP_UMINUS},   // -
		    Tvp{POI_XPRS_TOK_EOF, {}});                 // EOF

		int begs[] = {0, std::ssize(types)};
		_check(XPRSnlpaddformulas(m_model.get(), 1, &rowidx, begs, 1, types, values));
	}

	++m_quad_nl_constr_num;
	return constraint_index;
}

ConstraintIndex Model::_add_sos_constraint(const Vector<VariableIndex> &variables, SOSType sos_type,
                                           const Vector<double> &weights)
{
	IndexT index = m_sos_constraint_index.add_index();
	ConstraintIndex constraint_index(ConstraintType::SOS, index);

	const int nnz = variables.size();
	const char type[] = {poi_to_xprs_sos_type(sos_type)};
	const XPRSint64 beg[] = {0};
	std::vector<int> ind_v(nnz);
	for (int i = 0; i < nnz; i++)
	{
		ind_v[i] = _checked_variable_index(variables[i]);
	}
	_check(XPRSaddsets64(m_model.get(), 1, nnz, type, beg, ind_v.data(), weights.data()));
	return constraint_index;
}

ConstraintIndex Model::add_sos_constraint(const Vector<VariableIndex> &variables, SOSType sos_type)
{
	_check_expected_mode(XPRESS_MODEL_MODE::MAIN);
	_ensure_postsolved();
	_clear_caches();

	Vector<double> weights(variables.size());
	std::iota(weights.begin(), weights.end(), 1.0);
	return _add_sos_constraint(variables, sos_type, weights);
}

ConstraintIndex Model::add_sos_constraint(const Vector<VariableIndex> &variables, SOSType sos_type,
                                          const Vector<CoeffT> &weights)
{
	_check_expected_mode(XPRESS_MODEL_MODE::MAIN);
	_ensure_postsolved();
	_clear_caches();

	// If CoeffT will ever be not a double, we need to convert weights
	if constexpr (std::is_same_v<CoeffT, double>)
	{
		std::vector<double> double_weights;
		double_weights.reserve(weights.size());
		for (CoeffT w : weights)
		{
			double_weights.push_back(static_cast<double>(w));
		}
		return _add_sos_constraint(variables, sos_type, double_weights);
	}
	else
	{
		return _add_sos_constraint(variables, sos_type, weights);
	}
}

Tvp to_xprs_opcode(UnaryOperator opcode_enum)
{
	switch (opcode_enum)
	{
	case UnaryOperator::Neg:
		return {POI_XPRS_TOK_OP, POI_XPRS_OP_UMINUS};
	case UnaryOperator::Sin:
		return {POI_XPRS_TOK_IFUN, POI_XPRS_IFUN_SIN};
	case UnaryOperator::Cos:
		return {POI_XPRS_TOK_IFUN, POI_XPRS_IFUN_COS};
	case UnaryOperator::Tan:
		return {POI_XPRS_TOK_IFUN, POI_XPRS_IFUN_TAN};
	case UnaryOperator::Asin:
		return {POI_XPRS_TOK_IFUN, POI_XPRS_IFUN_ARCSIN};
	case UnaryOperator::Acos:
		return {POI_XPRS_TOK_IFUN, POI_XPRS_IFUN_ARCCOS};
	case UnaryOperator::Atan:
		return {POI_XPRS_TOK_IFUN, POI_XPRS_IFUN_ARCTAN};
	case UnaryOperator::Abs:
		return {POI_XPRS_TOK_IFUN, POI_XPRS_IFUN_ABS};
	case UnaryOperator::Sqrt:
		return {POI_XPRS_TOK_IFUN, POI_XPRS_IFUN_SQRT};
	case UnaryOperator::Exp:
		return {POI_XPRS_TOK_IFUN, POI_XPRS_IFUN_EXP};
	case UnaryOperator::Log:
		return {POI_XPRS_TOK_IFUN, POI_XPRS_IFUN_LN};
	case UnaryOperator::Log10:
		return {POI_XPRS_TOK_IFUN, POI_XPRS_IFUN_LOG10};
	default: {
		auto opname = unary_operator_to_string(opcode_enum);
		auto msg = fmt::format("Unknown unary operator for Xpress: {}", opname);
		throw std::runtime_error(msg);
	}
	}
}

Tvp to_xprs_opcode(BinaryOperator opcode_enum)
{
	switch (opcode_enum)
	{
	case BinaryOperator::Sub:
		return {POI_XPRS_TOK_OP, POI_XPRS_OP_MINUS};
	case BinaryOperator::Div:
		return {POI_XPRS_TOK_OP, POI_XPRS_OP_DIVIDE};
	case BinaryOperator::Pow:
		return {POI_XPRS_TOK_OP, POI_XPRS_OP_EXPONENT};
	case BinaryOperator::Add2:
		return {POI_XPRS_TOK_OP, POI_XPRS_OP_PLUS};
	case BinaryOperator::Mul2:
		return {POI_XPRS_TOK_OP, POI_XPRS_OP_MULTIPLY};
	default:
		auto opname = binary_operator_to_string(opcode_enum);
		auto msg = fmt::format("Unknown unary operator for Xpress: {}", opname);
		throw std::runtime_error(msg);
	}
}

Tvp to_xprs_opcode(TernaryOperator opcode_enum)
{
	auto opname = ternary_operator_to_string(opcode_enum);
	auto msg = fmt::format("Unknown unary operator for Xpress: {}", opname);
	throw std::runtime_error(msg);
}

Tvp to_xprs_opcode(NaryOperator opcode_enum)
{
	switch (opcode_enum)
	{
	case NaryOperator::Add:
		return {POI_XPRS_TOK_OP, POI_XPRS_OP_PLUS};
	case NaryOperator::Mul:
		return {POI_XPRS_TOK_OP, POI_XPRS_OP_MULTIPLY};
	default:
		auto opname = nary_operator_to_string(opcode_enum);
		auto msg = fmt::format("Unknown nary operator for Xpress: {}", opname);
		throw std::runtime_error(msg);
	}
}

BinaryOperator nary_to_binary_op(NaryOperator opcode_enum)
{
	switch (opcode_enum)
	{
	case NaryOperator::Add:
		return BinaryOperator::Add2;
	case NaryOperator::Mul:
		return BinaryOperator::Mul2;
	default: {
		auto opname = nary_operator_to_string(opcode_enum);
		auto msg = fmt::format("Unknown nary operator for Xpress: {}", opname);
		throw std::runtime_error(msg);
	}
	}
}

Tvp Model::_decode_expr(const ExpressionGraph &graph, const ExpressionHandle &expr)
{
	auto array_type = expr.array;
	auto index = expr.id;

	switch (array_type)
	{
	case ArrayType::Constant:
		return {POI_XPRS_TOK_CON, static_cast<double>(graph.m_constants[index])};
	case ArrayType::Variable:
		return {POI_XPRS_TOK_COL,
		        static_cast<double>(_checked_variable_index(graph.m_variables[index]))};
	case ArrayType::Parameter:
		break;
	case ArrayType::Unary:
		return to_xprs_opcode(graph.m_unaries[index].op);
	case ArrayType::Binary:
		return to_xprs_opcode(graph.m_binaries[index].op);
	case ArrayType::Ternary:
		return to_xprs_opcode(graph.m_ternaries[index].op);
	case ArrayType::Nary:
		return to_xprs_opcode(graph.m_naries[index].op);
	defaut:
		break;
	}
	throw std::runtime_error("Not supported expression.");
}

ExpressionHandle nary_to_binary(ExpressionGraph &graph, const ExpressionHandle &expr)
{
	auto &nary = graph.m_naries[expr.id];
	NaryOperator n_op = nary.op;
	BinaryOperator bin_opcode = nary_to_binary_op(n_op);
	int n_operands = nary.operands.size();
	if (n_operands == 0 || (n_op != NaryOperator::Add && n_op != NaryOperator::Mul))
	{
		return expr;
	}

	auto new_expr = nary.operands[0];
	for (int i = 1; i < n_operands; ++i)
	{
		new_expr = graph.add_binary(bin_opcode, new_expr, nary.operands[i]);
	}
	return new_expr;
}

std::pair<std::vector<int>, std::vector<double>> Model::_decode_graph_postfix_order(
    ExpressionGraph &graph, const ExpressionHandle &result)
{
	std::vector<int> types;
	std::vector<double> values;

	// Xpress uses a reversed Polish notation (post fix). So we need to visit the expression tree
	// in post-order depth first. We keep a stack to go depth first and visit each element
	// twice. First time process its children, second time decode it.
	std::stack<std::pair<ExpressionHandle, bool>> expr_stack;
	expr_stack.emplace(result, true);

	while (!expr_stack.empty())
	{
		auto &[expr, visit_children] = expr_stack.top();
		auto [type, value] = _decode_expr(graph, expr);

		// If its children have already been processed and we can add it to the expression
		if (!visit_children)
		{
			types.push_back(type);
			values.push_back(value);
			expr_stack.pop();
			continue;
		}

		// Xpress requires a parenthesis to start an internal or user function
		if (type == POI_XPRS_TOK_IFUN || type == POI_XPRS_TOK_FUN)
		{
			types.push_back(POI_XPRS_TOK_RB);
			values.push_back({});
		}

		switch (expr.array)
		{
		case ArrayType::Constant:
		case ArrayType::Variable:
			break;
		case ArrayType::Unary:
			expr_stack.emplace(graph.m_unaries[expr.id].operand, true);
			break;
		case ArrayType::Nary:
			// Xpress does not have nary operators out of the box, we have to translate them into a
			// sequence of binary operators
			expr = nary_to_binary(graph, expr);
			[[fallthrough]];
		case ArrayType::Binary:
			expr_stack.emplace(graph.m_binaries[expr.id].right, true);
			expr_stack.emplace(graph.m_binaries[expr.id].left, true);
			break;
		default:
			throw std::runtime_error("Unrecognized token.");
		}

		// Children has been processed and added to the stack, next time we'll add it to the
		// expression.
		visit_children = false;
	}

	types.push_back(POI_XPRS_TOK_EOF);
	values.push_back({});
	return {std::move(types), std::move(values)};
}

ConstraintIndex Model::add_single_nl_constraint(ExpressionGraph &graph,
                                                const ExpressionHandle &result,
                                                const std::tuple<double, double> &interval,
                                                const char *name)
{
	_check_expected_mode(XPRESS_MODEL_MODE::MAIN);
	_ensure_postsolved();
	_clear_caches();

	int rowidx = get_raw_attribute_int_by_id(POI_XPRS_ROWS);
	ConstraintIndex constraint = add_linear_constraint(ScalarAffineFunction{}, interval, name);
	constraint.type = ConstraintType::Xpress_Nlp;

	auto [types, values] = _decode_graph_postfix_order(graph, result);

	int nnz = values.size();
	int begs[] = {0, nnz};
	_check(XPRSnlpaddformulas(m_model.get(), 1, &rowidx, begs, 1, types.data(), values.data()));
	++m_quad_nl_constr_num;
	return constraint;
}

void Model::delete_constraint(ConstraintIndex constraint)
{
	_check_expected_mode(XPRESS_MODEL_MODE::MAIN);
	_ensure_postsolved();
	_clear_caches();

	if (!is_constraint_active(constraint))
	{
		throw std::runtime_error("Constraint does not exist");
	}

	int constraint_row = _checked_constraint_index(constraint);
	if (constraint_row >= 0)
	{
		switch (constraint.type)
		{
		case ConstraintType::Quadratic:
		case ConstraintType::Cone:
		case ConstraintType::Xpress_Nlp:
			--m_quad_nl_constr_num;
			[[fallthrough]];
		case ConstraintType::Linear:
			m_constraint_index.delete_index(constraint.index);
			_check(XPRSdelrows(m_model.get(), 1, &constraint_row));
			break;
		case ConstraintType::SOS:
			m_sos_constraint_index.delete_index(constraint.index);
			_check(XPRSdelsets(m_model.get(), 1, &constraint_row));
			break;
		default:
			throw std::runtime_error("Unknown constraint type");
		}
	}
}

bool Model::is_constraint_active(ConstraintIndex constraint)
{
	_check_expected_mode(XPRESS_MODEL_MODE::MAIN);
	switch (constraint.type)
	{
	case ConstraintType::Linear:
	case ConstraintType::Quadratic:
	case ConstraintType::Cone:
	case ConstraintType::Xpress_Nlp:
		return m_constraint_index.has_index(constraint.index);
	case ConstraintType::SOS:
		return m_sos_constraint_index.has_index(constraint.index);
	default:
		throw std::runtime_error("Unknown constraint type");
	}
	return false;
}

void Model::set_objective(const ScalarAffineFunction &function, ObjectiveSense sense)
{
	_check_expected_mode(XPRESS_MODEL_MODE::MAIN);
	_ensure_postsolved();
	_clear_caches();

	// Delete linear and quadratic term of the objective function
	_check(XPRSdelobj(m_model.get(), 0));
	_check(XPRSdelqmatrix(m_model.get(), -1));
	has_quad_objective = false;
	if (has_nlp_objective)
	{
		delete_constraint(m_nlp_obj_constraint);
		delete_variable(m_nlp_obj_variable);
		has_nlp_objective = false;
	}

	if (function.size() > 0)
	{
		AffineFunctionPtrForm<int, int, double> ptr_form;
		ptr_form.make(this, function);
		int numnz = ptr_form.numnz;
		const int *cind = ptr_form.index;
		const double *cval = ptr_form.value;
		_check(XPRSchgobj(m_model.get(), numnz, cind, cval));
	}
	if (function.constant.has_value())
	{
		int obj_constant_magic_index = -1;
		double obj_constant = -static_cast<double>(function.constant.value());
		_check(XPRSchgobj(m_model.get(), 1, &obj_constant_magic_index, &obj_constant));
	}
	int stype = poi_to_xprs_obj_sense(sense);
	_check(XPRSchgobjsense(m_model.get(), stype));
}

// Set quadratic objective function, replacing any previous objective.
// Handles Xpress's symmetric matrix convention where off-diagonal terms are doubled.
void Model::set_objective(const ScalarQuadraticFunction &function, ObjectiveSense sense)
{
	// Set affine part (also clears any previous quadratic terms)
	const auto &affine_part = function.affine_part.value_or(ScalarAffineFunction{});
	set_objective(affine_part, sense);

	// Add quadratic terms if present
	if (function.size() > 0)
	{
		QuadraticFunctionPtrForm<int, int, double> ptr_form =
		    poi_to_xprs_quad_formula(*this, function, true);
		int numqnz = ptr_form.numnz;
		const int *qrow = ptr_form.row;
		const int *qcol = ptr_form.col;
		const double *qval = ptr_form.value;
		_check(XPRSchgmqobj64(m_model.get(), numqnz, qrow, qcol, qval));
		has_quad_objective = true;
	}
}

void Model::set_objective(const ExprBuilder &function, ObjectiveSense sense)
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

void Model::add_single_nl_objective(ExpressionGraph &graph, const ExpressionHandle &result)
{
	_check_expected_mode(XPRESS_MODEL_MODE::MAIN);
	_ensure_postsolved();
	_clear_caches();

	if (!has_nlp_objective)
	{
		has_nlp_objective = true;
		m_nlp_obj_variable = add_variable();
		m_nlp_obj_constraint = add_linear_constraint(ScalarAffineFunction(m_nlp_obj_variable, -1.0),
		                                             ConstraintSense::Equal, 0.0, NULL);
		set_objective_coefficient(m_nlp_obj_variable, 1.0);
	}

	auto [types, values] = _decode_graph_postfix_order(graph, result);
	int nnz = values.size();
	int begs[] = {0, nnz};
	int rowidx = _constraint_index(m_nlp_obj_constraint);
	_check(XPRSnlpaddformulas(m_model.get(), 1, &rowidx, begs, 1, types.data(), values.data()));
}

double Model::get_normalized_rhs(ConstraintIndex constraint)
{
	_check_expected_mode(XPRESS_MODEL_MODE::MAIN);
	_ensure_postsolved();

	double rhs = {};
	int rowidx = _checked_constraint_index(constraint);
	_check(XPRSgetrhs(m_model.get(), &rhs, rowidx, rowidx));
	return rhs;
}

void Model::set_normalized_rhs(ConstraintIndex constraint, double value)
{
	_check_expected_mode(XPRESS_MODEL_MODE::MAIN);
	_ensure_postsolved();
	_clear_caches();

	int rowidx = _checked_constraint_index(constraint);
	_check(XPRSchgrhs(m_model.get(), 1, &rowidx, &value));
}

double Model::get_normalized_coefficient(ConstraintIndex constraint, VariableIndex variable)
{
	_check_expected_mode(XPRESS_MODEL_MODE::MAIN);
	_ensure_postsolved();

	int rowidx = _checked_constraint_index(constraint);
	int colidx = _checked_variable_index(variable);
	double coeff = {};
	_check(XPRSgetcoef(m_model.get(), rowidx, colidx, &coeff));
	return coeff;
}
void Model::set_normalized_coefficient(ConstraintIndex constraint, VariableIndex variable,
                                       double value)
{
	_check_expected_mode(XPRESS_MODEL_MODE::MAIN);
	_ensure_postsolved();
	_clear_caches();

	int rowidx = _checked_constraint_index(constraint);
	int colidx = _checked_variable_index(variable);
	_check(XPRSchgcoef(m_model.get(), rowidx, colidx, value));
}

double Model::get_objective_coefficient(VariableIndex variable)
{
	_check_expected_mode(XPRESS_MODEL_MODE::MAIN);
	_ensure_postsolved();

	int colidx = _checked_variable_index(variable);
	double coeff = {};
	_check(XPRSgetobj(m_model.get(), &coeff, colidx, colidx));
	return coeff;
}
void Model::set_objective_coefficient(VariableIndex variable, double value)
{
	_check_expected_mode(XPRESS_MODEL_MODE::MAIN);
	_ensure_postsolved();
	_clear_caches();

	int colidx = _checked_variable_index(variable);
	_check(XPRSchgobj(m_model.get(), 1, &colidx, &value));
}

int Model::_constraint_index(ConstraintIndex constraint)
{
	switch (constraint.type)
	{
	case ConstraintType::Linear:
	case ConstraintType::Quadratic:
	case ConstraintType::Cone:
	case ConstraintType::Xpress_Nlp:
		return m_constraint_index.get_index(constraint.index);
	case ConstraintType::SOS:
		return m_sos_constraint_index.get_index(constraint.index);
	default:
		throw std::runtime_error("Unknown constraint type");
	}
}

int Model::_variable_index(VariableIndex variable)
{
	return m_variable_index.get_index(variable.index);
}

int Model::_checked_constraint_index(ConstraintIndex constraint)
{
	int rowidx = _constraint_index(constraint);
	if (rowidx < 0)
	{
		throw std::runtime_error("Constraint does not exists");
	}
	return rowidx;
}

int Model::_checked_variable_index(VariableIndex variable)
{
	int colidx = _variable_index(variable);
	if (colidx < 0)
	{
		throw std::runtime_error("Variable does not exists");
	}
	return colidx;
}

bool Model::_is_mip()
{
	return get_raw_attribute_int_by_id(POI_XPRS_MIPENTS) > 0 ||
	       get_raw_attribute_int_by_id(POI_XPRS_SETS) > 0;
}

void Model::optimize()
{
	_check_expected_mode(XPRESS_MODEL_MODE::MAIN);
	_clear_caches();

	int stop_status = 0;
	_check(XPRSoptimize(m_model.get(), "", &stop_status, nullptr));
	m_need_postsolve = (stop_status == POI_XPRS_SOLVESTATUS_STOPPED);
}

double Model::get_variable_value(VariableIndex variable)
{
	_check_expected_mode(XPRESS_MODEL_MODE::MAIN);
	int colidx = _checked_variable_index(variable);
	int status = POI_XPRS_SOLAVAILABLE_NOTFOUND;
	double value = {};
	_check(XPRSgetsolution(m_model.get(), &status, &value, colidx, colidx));
	if (status == POI_XPRS_SOLAVAILABLE_NOTFOUND)
	{
		throw std::runtime_error("No solution found");
	}
	return value;
}

double Model::get_variable_rc(VariableIndex variable)
{
	_check_expected_mode(XPRESS_MODEL_MODE::MAIN);
	int colidx = _checked_variable_index(variable);
	int status = POI_XPRS_SOLAVAILABLE_NOTFOUND;
	double value = {};
	_check(XPRSgetredcosts(m_model.get(), &status, &value, colidx, colidx));
	if (status == POI_XPRS_SOLAVAILABLE_NOTFOUND)
	{
		throw std::runtime_error("No solution found");
	}
	return value;
}

double Model::get_variable_primal_ray(VariableIndex variable)
{
	_check_expected_mode(XPRESS_MODEL_MODE::MAIN);
	_ensure_postsolved();

	if (m_primal_ray.empty())
	{
		int has_ray = 0;
		_check(XPRSgetprimalray(m_model.get(), nullptr, &has_ray));
		if (has_ray == 0)
		{
			throw std::runtime_error("Primal ray not available");
		}
		m_primal_ray.resize(get_raw_attribute_int_by_id(POI_XPRS_COLS));
		_check(XPRSgetprimalray(m_model.get(), m_primal_ray.data(), &has_ray));
		assert(has_ray != 0);
	}

	int colidx = _checked_variable_index(variable);
	return m_primal_ray[colidx];
}

int Model::_get_basis_stat(int entity_idx, bool is_row)
{
	_check_expected_mode(XPRESS_MODEL_MODE::MAIN);
	_ensure_postsolved();

	int entity_stat = {};
	if (is_row)
	{
		_check(XPRSgetbasisval(m_model.get(), entity_idx, 0, &entity_stat, nullptr));
	}
	else
	{
		_check(XPRSgetbasisval(m_model.get(), 0, entity_idx, nullptr, &entity_stat));
	}
	return entity_stat;
}

bool Model::is_variable_basic(VariableIndex variable)
{
	return _get_basis_stat(_checked_variable_index(variable), false) == POI_XPRS_BASISSTATUS_BASIC;
}

bool Model::is_variable_nonbasic_lb(VariableIndex variable)
{
	return _get_basis_stat(_checked_variable_index(variable), false) ==
	       POI_XPRS_BASISSTATUS_NONBASIC_LOWER;
}

bool Model::is_variable_nonbasic_ub(VariableIndex variable)
{
	return _get_basis_stat(_checked_variable_index(variable), false) ==
	       POI_XPRS_BASISSTATUS_NONBASIC_UPPER;
}
bool Model::is_variable_superbasic(VariableIndex variable)
{
	return _get_basis_stat(_checked_variable_index(variable), false) ==
	       POI_XPRS_BASISSTATUS_SUPERBASIC;
}

double Model::get_variable_lowerbound(VariableIndex variable)
{
	_check_expected_mode(XPRESS_MODEL_MODE::MAIN);
	_ensure_postsolved();

	double lb = {};
	int colidx = _checked_variable_index(variable);
	_check(XPRSgetlb(m_model.get(), &lb, colidx, colidx));
	return lb;
}

double Model::get_variable_upperbound(VariableIndex variable)
{
	_check_expected_mode(XPRESS_MODEL_MODE::MAIN);
	_ensure_postsolved();

	double ub = {};
	int colidx = _checked_variable_index(variable);
	_check(XPRSgetub(m_model.get(), &ub, colidx, colidx));
	return ub;
}

VariableDomain Model::get_variable_type(VariableIndex variable)
{
	_check_expected_mode(XPRESS_MODEL_MODE::MAIN);
	_ensure_postsolved();

	int colidex = _checked_variable_index(variable);
	char vtype = {};
	_check(XPRSgetcoltype(m_model.get(), &vtype, colidex, colidex));
	return xprs_to_poi_var_type(vtype);
}

char Model::_get_variable_bound_IIS(VariableIndex variable)
{
	_check_expected_mode(XPRESS_MODEL_MODE::MAIN);
	_ensure_postsolved();

	if (m_iis_cols.empty() || m_iis_bound_types.empty())
	{
		int m_nrows = {};
		int m_ncols = {};
		_check(XPRSgetiisdata(m_model.get(), 1, &m_nrows, &m_ncols, nullptr, nullptr, nullptr,
		                      nullptr, nullptr, nullptr, nullptr, nullptr));

		m_iis_cols.resize(m_ncols);
		m_iis_bound_types.resize(m_ncols);
		_check(XPRSgetiisdata(m_model.get(), 1, &m_nrows, &m_ncols, nullptr, m_iis_cols.data(),
		                      nullptr, m_iis_bound_types.data(), nullptr, nullptr, nullptr,
		                      nullptr));
	}

	int colidx = _checked_variable_index(variable);
	for (int j = 0; j < m_iis_cols.size(); ++j)
	{
		if (m_iis_cols[j] == colidx)
		{
			return m_iis_bound_types[j];
		}
	}
	return '\0';
}

bool Model::is_variable_lowerbound_IIS(VariableIndex variable)
{
	return _get_variable_bound_IIS(variable) == 'L';
}

bool Model::is_variable_upperbound_IIS(VariableIndex variable)
{
	return _get_variable_bound_IIS(variable) == 'U';
}

void Model::set_constraint_sense(ConstraintIndex constraint, ConstraintSense sense)
{
	const int rowidx = _checked_constraint_index(constraint);
	const char rowtype = poi_to_xprs_cons_sense(sense);
	_check(XPRSchgrowtype(m_model.get(), 1, &rowidx, &rowtype));
}

ConstraintSense Model::get_constraint_sense(ConstraintIndex constraint)
{
	const int rowidx = _checked_constraint_index(constraint);
	char rowtype = {};
	_check(XPRSgetrowtype(m_model.get(), &rowtype, rowidx, rowidx));
	return xprs_to_poi_cons_sense(rowtype);
}

void Model::set_constraint_rhs(ConstraintIndex constraint, CoeffT rhs)
{
	const int rowidx = _checked_constraint_index(constraint);
	const double g_rhs[] = {static_cast<double>(rhs)};
	_check(XPRSchgrhs(m_model.get(), 1, &rowidx, g_rhs));
}

CoeffT Model::get_constraint_rhs(ConstraintIndex constraint)
{
	const int rowidx = _checked_constraint_index(constraint);
	double rhs = {};
	_check(XPRSgetrhs(m_model.get(), &rhs, rowidx, rowidx));
	return static_cast<CoeffT>(rhs);
}

double Model::get_constraint_slack(ConstraintIndex constraint)
{
	_check_expected_mode(XPRESS_MODEL_MODE::MAIN);
	_ensure_postsolved();

	int rowidx = _checked_constraint_index(constraint);
	int status = POI_XPRS_SOLAVAILABLE_NOTFOUND;
	double value = {};
	_check(XPRSgetslacks(m_model.get(), &status, &value, rowidx, rowidx));
	if (status == POI_XPRS_SOLAVAILABLE_NOTFOUND)
	{
		throw std::runtime_error("No solution found");
	}
	return value;
}

double Model::get_constraint_dual(ConstraintIndex constraint)
{
	_check_expected_mode(XPRESS_MODEL_MODE::MAIN);
	_ensure_postsolved();

	int rowidx = _checked_constraint_index(constraint);
	int status = POI_XPRS_SOLAVAILABLE_NOTFOUND;
	double value = {};
	_check(XPRSgetduals(m_model.get(), &status, &value, rowidx, rowidx));
	if (status == POI_XPRS_SOLAVAILABLE_NOTFOUND)
	{
		throw std::runtime_error("No solution found");
	}
	return value;
}

double Model::get_constraint_dual_ray(ConstraintIndex constraint)
{

	_check_expected_mode(XPRESS_MODEL_MODE::MAIN);
	_ensure_postsolved();

	if (m_dual_ray.empty())
	{
		int has_ray = 0;
		_check(XPRSgetdualray(m_model.get(), nullptr, &has_ray));
		if (has_ray == 0)
		{
			throw std::runtime_error("Dual ray not available");
		}
		m_dual_ray.resize(get_raw_attribute_int_by_id(POI_XPRS_ROWS));
		_check(XPRSgetdualray(m_model.get(), m_dual_ray.data(), &has_ray));
		assert(has_ray != 0);
	}

	int rowidx = _checked_constraint_index(constraint);
	return m_dual_ray[rowidx];
}

bool Model::is_constraint_basic(ConstraintIndex constraint)
{
	return _get_basis_stat(_checked_constraint_index(constraint), true) ==
	       POI_XPRS_BASISSTATUS_BASIC;
}

bool Model::is_constraint_nonbasic_lb(ConstraintIndex constraint)
{
	return _get_basis_stat(_checked_constraint_index(constraint), true) ==
	       POI_XPRS_BASISSTATUS_NONBASIC_LOWER;
}

bool Model::is_constraint_nonbasic_ub(ConstraintIndex constraint)
{
	return _get_basis_stat(_checked_constraint_index(constraint), true) ==
	       POI_XPRS_BASISSTATUS_NONBASIC_UPPER;
}

bool Model::is_constraint_superbasic(ConstraintIndex constraint)
{
	return _get_basis_stat(_checked_constraint_index(constraint), true) ==
	       POI_XPRS_BASISSTATUS_SUPERBASIC;
}

bool Model::is_constraint_in_IIS(ConstraintIndex constraint)
{
	_check_expected_mode(XPRESS_MODEL_MODE::MAIN);
	_ensure_postsolved();

	if (m_iss_rows.empty())
	{
		int nrow = {};
		int ncol = {};
		_check(XPRSgetiisdata(m_model.get(), 1, &nrow, &ncol, nullptr, nullptr, nullptr, nullptr,
		                      nullptr, nullptr, nullptr, nullptr));

		m_iss_rows.resize(nrow);
		_check(XPRSgetiisdata(m_model.get(), 1, &nrow, &ncol, m_iss_rows.data(), nullptr, nullptr,
		                      nullptr, nullptr, nullptr, nullptr, nullptr));
	}

	int rowidx = _constraint_index(constraint);
	for (int ridx : m_iss_rows)
	{
		if (ridx == rowidx)
		{
			return true;
		}
	}
	return false;
}

void *Model::get_raw_model()
{
	return m_model.get();
}

std::string Model::version_string()
{
	char buffer[32];
	XPRSgetversion(buffer);
	return buffer;
}

void Model::computeIIS()
{
	_check_expected_mode(XPRESS_MODEL_MODE::MAIN);
	int status = 0;

	// passing 1 emphasizes simplicity of the IIS
	// passing 2 emphasizes a quick result
	_check(XPRSiisfirst(m_model.get(), 2, &status));
	switch (status)
	{
	case 0:
		return;
	case 1:
		throw std::runtime_error("IIS: problem is feasible");
	case 2:
		throw std::runtime_error("IIS: generic error");
	case 3:
		throw std::runtime_error("IIS: timeout or interruption");
	default:
		throw std::runtime_error(fmt::format("IIS: unknown exit status {}", status));
	}
}

////////////////////////////////////////////////////////////////////////////////
//                       ATTRIBUTES AND CONTROLS ACCESS                       //
////////////////////////////////////////////////////////////////////////////////

static const char *xpress_type_to_string(CATypes type)
{
	switch (type)
	{
	case CATypes::INT:
		return "int";
	case CATypes::INT64:
		return "int64";
	case CATypes::DOUBLE:
		return "double";
	case CATypes::STRING:
		return "string";
	default:
		return "unknown";
	}
}

std::pair<int, CATypes> Model::_get_control_info(const char *control)
{
	int control_id = {};
	int control_type = {};
	_check(XPRSgetcontrolinfo(m_model.get(), control, &control_id, &control_type));
	return std::make_pair(control_id, static_cast<CATypes>(control_type));
}

std::pair<int, CATypes> Model::_get_attribute_info(const char *attrib)
{
	int attrib_id = {};
	int attrib_type = {};
	_check(XPRSgetattribinfo(m_model.get(), attrib, &attrib_id, &attrib_type));
	return std::make_pair(attrib_id, static_cast<CATypes>(attrib_type));
}

int Model::_get_checked_control_id(const char *control, CATypes expected, CATypes backup)
{
	auto [id, type] = _get_control_info(control);
	if (type == CATypes::NOTDEFINED)
	{
		throw std::runtime_error("Error, unknown control type.");
	}
	if (type != expected && type != backup)
	{
		auto error_msg = fmt::format(
		    "Error retriving control '{}'. Control type is '{}', but '{}' was expected.", control,
		    xpress_type_to_string(type), xpress_type_to_string(expected));
		throw std::runtime_error(error_msg);
	}
	return id;
}

int Model::_get_checked_attribute_id(const char *attrib, CATypes expected, CATypes backup)
{
	auto [id, type] = _get_attribute_info(attrib);
	if (type == CATypes::NOTDEFINED)
	{
		throw std::runtime_error("Error, unknown attribute type.");
	}
	if (type != expected)
	{
		auto error_msg = fmt::format(
		    "Error retriving attribute '{}'. Attribute type is '{}', but '{}' was expected.",
		    attrib, xpress_type_to_string(type), xpress_type_to_string(expected));
		throw std::runtime_error(error_msg);
	}
	return id;
}

// Generic access to attributes and controls
Model::xprs_type_variant_t Model::get_raw_attribute(const char *attrib)
{
	auto [id, type] = _get_attribute_info(attrib);
	switch (type)
	{
	case CATypes::INT:
	case CATypes::INT64:
		return get_raw_attribute_int_by_id(id);
	case CATypes::DOUBLE:
		return get_raw_attribute_dbl_by_id(id);
	case CATypes::STRING:
		return get_raw_attribute_str_by_id(id);
	default:
		throw std::runtime_error("Unknown attribute type");
	}
}

Model::xprs_type_variant_t Model::get_raw_control(const char *control)
{
	auto [id, type] = _get_control_info(control);
	switch (type)
	{
	case CATypes::INT:
	case CATypes::INT64:
		return get_raw_control_int_by_id(id);
	case CATypes::DOUBLE:
		return get_raw_control_dbl_by_id(id);
	case CATypes::STRING:
		return get_raw_control_str_by_id(id);
	default:
		throw std::runtime_error("Unknown attribute type");
	}
}

// Helper struct to achieve a sort of pattern matching with the visitor pattern.
// It basically exploit the overload resolution to get the equivalent of a series of
// if constexpr(std::is_same_v<ArgT,...>)
template <typename... Ts>
struct OverloadSet : public Ts...
{
	using Ts::operator()...;
};

// Deduction guide for OverloadSet
template <typename... Ts>
OverloadSet(Ts...) -> OverloadSet<Ts...>;

void Model::set_raw_control(const char *control, Model::xprs_type_variant_t &value)
{
	std::visit(OverloadSet{[&](double d) { set_raw_control_dbl(control, d); },
	                       [&](std::integral auto i) { set_raw_control_int(control, i); },
	                       [&](const std::string &s) { set_raw_control_str(control, s.c_str()); }},
	           value);
}

void Model::set_raw_control_int(const char *control, XPRSint64 value)
{
	int id = _get_checked_control_id(control, CATypes::INT64, CATypes::INT);
	return set_raw_control_int_by_id(id, value);
}

void Model::set_raw_control_dbl(const char *control, double value)
{
	int id = _get_checked_control_id(control, CATypes::DOUBLE);
	return set_raw_control_dbl_by_id(id, value);
}

void Model::set_raw_control_str(const char *control, const char *value)
{
	int id = _get_checked_control_id(control, CATypes::STRING);
	return set_raw_control_str_by_id(id, value);
}

XPRSint64 Model::get_raw_control_int(const char *control)
{
	int id = _get_checked_control_id(control, CATypes::INT64, CATypes::INT);
	return get_raw_control_int_by_id(id);
}

double Model::get_raw_control_dbl(const char *control)
{
	int id = _get_checked_control_id(control, CATypes::DOUBLE);
	return get_raw_control_dbl_by_id(id);
}

std::string Model::get_raw_control_str(const char *control)
{
	int id = _get_checked_control_id(control, CATypes::STRING);
	return get_raw_control_str_by_id(id);
}

XPRSint64 Model::get_raw_attribute_int(const char *attrib)
{
	int id = _get_checked_attribute_id(attrib, CATypes::INT64, CATypes::INT);
	return get_raw_attribute_int_by_id(id);
}

double Model::get_raw_attribute_dbl(const char *attrib)
{
	int id = _get_checked_attribute_id(attrib, CATypes::DOUBLE);
	return get_raw_attribute_dbl_by_id(id);
}

std::string Model::get_raw_attribute_str(const char *attrib)
{
	int id = _get_checked_attribute_id(attrib, CATypes::STRING);
	return get_raw_attribute_str_by_id(id);
}

void Model::set_raw_control_int_by_id(int control, XPRSint64 value)
{
	// Disabling Xpress internal callback mutex is forbidden since this could easily create race
	// condition and deadlocks since it's used in conjunction with Python GIL.
	if (control == POI_XPRS_MUTEXCALLBACKS)
	{
		throw std::runtime_error(
		    "Changing Xpress callback mutex setting is currently not supported.");
	}
	_check(XPRSsetintcontrol64(m_model.get(), control, value));
}

void Model::set_raw_control_dbl_by_id(int control, double value)
{
	_check(XPRSsetdblcontrol(m_model.get(), control, value));
}

void Model::set_raw_control_str_by_id(int control, const char *value)
{
	_check(XPRSsetstrcontrol(m_model.get(), control, value));
}

XPRSint64 Model::get_raw_control_int_by_id(int control)
{
	XPRSint64 value = {};
	_check(XPRSgetintcontrol64(m_model.get(), control, &value));
	return value;
}

double Model::get_raw_control_dbl_by_id(int control)
{
	double value = {};
	_check(XPRSgetdblcontrol(m_model.get(), control, &value));
	return value;
}

std::string Model::get_raw_control_str_by_id(int control)
{
	int req_size = {};
	_check(XPRSgetstringcontrol(m_model.get(), control, nullptr, 0, &req_size));
	std::string value = {};
	value.resize(req_size);
	_check(XPRSgetstringcontrol(m_model.get(), control, value.data(), req_size, &req_size));
	if (value.size() != req_size)
	{
		throw std::runtime_error("Error while getting control string");
	}
	// Align string size with string length
	value.resize(strlen(value.c_str()));
	return value;
}

XPRSint64 Model::get_raw_attribute_int_by_id(int attrib)
{
	XPRSint64 value = {};
	_check(XPRSgetintattrib64(m_model.get(), attrib, &value));
	return value;
}

double Model::get_raw_attribute_dbl_by_id(int attrib)
{
	double value = {};
	_check(XPRSgetdblattrib(m_model.get(), attrib, &value));
	return value;
}

std::string Model::get_raw_attribute_str_by_id(int attrib)
{
	int req_size = {};
	_check(XPRSgetstringattrib(m_model.get(), attrib, nullptr, 0, &req_size));
	std::string value = {};
	value.resize(req_size);
	_check(XPRSgetstringattrib(m_model.get(), attrib, value.data(), req_size, &req_size));
	if (value.size() != req_size)
	{
		throw std::runtime_error("Error while getting control string");
	}
	return value;
}

LPSTATUS Model::get_lp_status()
{
	return static_cast<LPSTATUS>(get_raw_attribute_int_by_id(POI_XPRS_LPSTATUS));
}

MIPSTATUS Model::get_mip_status()
{
	return static_cast<MIPSTATUS>(get_raw_attribute_int_by_id(POI_XPRS_MIPSTATUS));
}

NLPSTATUS Model::get_nlp_status()
{
	return static_cast<NLPSTATUS>(get_raw_attribute_int_by_id(POI_XPRS_NLPSTATUS));
}

SOLVESTATUS Model::get_solve_status()
{
	return static_cast<SOLVESTATUS>(get_raw_attribute_int_by_id(POI_XPRS_SOLVESTATUS));
}

SOLSTATUS Model::get_sol_status()
{
	return static_cast<SOLSTATUS>(get_raw_attribute_int_by_id(POI_XPRS_SOLSTATUS));
}

IISSOLSTATUS Model::get_iis_sol_status()
{
	return static_cast<IISSOLSTATUS>(get_raw_attribute_int_by_id(POI_XPRS_IISSOLSTATUS));
}

OPTIMIZETYPE Model::get_optimize_type()
{
	return static_cast<OPTIMIZETYPE>(get_raw_attribute_int_by_id(POI_XPRS_OPTIMIZETYPEUSED));
}

void Model::_ensure_postsolved()
{
	if (m_need_postsolve)
	{
		_check(XPRSpostsolve(m_model.get()));

		// Non-convex quadratic constraint might be solved with the non linear solver, so we have to
		// make sure that the problem is nl-postsolved, even if it is not always strictly necessary
		// and could introduce minor overhead.
		if (m_quad_nl_constr_num >= 0 || has_quad_objective || has_nlp_objective)
		{
			_check(XPRSnlppostsolve(m_model.get()));
		}
		m_need_postsolve = false;
	}
}

void Model::_check_expected_mode(XPRESS_MODEL_MODE mode)
{
	if (mode == XPRESS_MODEL_MODE::MAIN && m_mode == XPRESS_MODEL_MODE::CALLBACK_)
	{
		throw std::runtime_error("Cannot call this function from within a callback. "
		                         "This operation is only available on the main model.");
	}
	if (mode == XPRESS_MODEL_MODE::CALLBACK_ && m_mode == XPRESS_MODEL_MODE::MAIN)
	{
		throw std::runtime_error("This function can only be called from within a callback. "
		                         "It is not available on the main model.");
	}
}

xpress_cbs_data Model::cb_get_arguments()
{
	_check_expected_mode(XPRESS_MODEL_MODE::CALLBACK_);
	return cb_args;
}

// NOTE: XPRSgetcallbacksolution return a context dependent solution
double Model::_cb_get_context_solution(VariableIndex variable)
{
	_check_expected_mode(XPRESS_MODEL_MODE::CALLBACK_);
	// Xpress already caches solutions internally
	int p_available = 0;
	int colidx = _checked_variable_index(variable);
	double value[] = {0.0};
	_check(XPRSgetcallbacksolution(m_model.get(), &p_available, value, colidx, colidx));
	if (p_available == 0)
	{
		throw std::runtime_error("No solution available");
	}
	return value[0];
}

// Get MIP solution value for a variable in callback context.
// Returns the callback's candidate integer solution if available (intsol, preintsol contexts),
// otherwise falls back to the current incumbent solution.
double Model::cb_get_solution(VariableIndex variable)
{
	_check_expected_mode(XPRESS_MODEL_MODE::CALLBACK_);
	if (cb_where == CB_CONTEXT::intsol || cb_where == CB_CONTEXT::preintsol)
	{
		// Context provides a candidate integer solution - return it directly
		return _cb_get_context_solution(variable);
	}

	// No integer solution in current context - return best known incumbent instead
	return cb_get_incumbent(variable);
}

// Get LP relaxation solution value for a variable in callback context. Returns the callback's LP
// relaxation solution when available in contexts that solve LPs (bariteration, cutround,
// chgbranchobject, nodelpsolved, optnode). It throws in other contexts.
double Model::cb_get_relaxation(VariableIndex variable)
{
	_check_expected_mode(XPRESS_MODEL_MODE::CALLBACK_);
	if (cb_where != CB_CONTEXT::bariteration && cb_where != CB_CONTEXT::cutround &&
	    cb_where != CB_CONTEXT::chgbranchobject && cb_where != CB_CONTEXT::nodelpsolved &&
	    cb_where != CB_CONTEXT::optnode)
	{
		throw std::runtime_error("LP relaxation solution not available.");
	}
	return _cb_get_context_solution(variable);
}

double Model::cb_get_incumbent(VariableIndex variable)
{
	return get_variable_value(variable);
}

void Model::cb_set_solution(VariableIndex variable, double value)
{
	_check_expected_mode(XPRESS_MODEL_MODE::CALLBACK_);
	cb_sol_cache.emplace_back(_checked_variable_index(variable), value);
}

void Model::cb_submit_solution()
{
	_check_expected_mode(XPRESS_MODEL_MODE::CALLBACK_);

	auto &sol = cb_sol_cache;
	if (sol.empty())
	{
		return;
	}

	// Merge together coefficient of duplicated indices
	std::ranges::sort(sol);
	std::vector<int> indices;
	std::vector<double> values;
	int curr_idx = std::numeric_limits<int>::lowest();
	for (auto [idx, val] : sol)
	{
		if (curr_idx != idx)
		{
			curr_idx = idx;
			indices.push_back(idx);
			values.emplace_back();
		}
		values.back() += val;
	}

	int ncol = static_cast<int>(indices.size());
	_check(XPRSaddmipsol(m_model.get(), ncol, values.data(), indices.data(), nullptr));
}

void Model::cb_exit()
{
	_check_expected_mode(XPRESS_MODEL_MODE::CALLBACK_);
	_check(XPRSinterrupt(m_model.get(), POI_XPRS_STOP_USER));
}

void Model::_cb_add_cut(const ScalarAffineFunction &function, ConstraintSense sense, CoeffT rhs)
{
	AffineFunctionPtrForm<int, int, double> ptr_form;
	ptr_form.make(this, function);
	int numnz = ptr_form.numnz;
	char g_sense = poi_to_xprs_cons_sense(sense);
	double g_rhs = static_cast<double>(rhs - function.constant.value_or(CoeffT{}));
	const int *cind = ptr_form.index;
	const double *cval = ptr_form.value;

	// Before adding the cut, we must translate it to the presolved model. If this translation fails
	// then we cannot continue. The translation can only fail if we have presolve operations enabled
	// that should be disabled in case of dynamically separated constraints.
	int ncols = get_raw_attribute_int_by_id(POI_XPRS_COLS);
	int ps_numnz = 0;
	std::vector<int> ps_cind(ncols);
	std::vector<double> ps_cval(ncols);
	double ps_rhs = 0.0;
	int ps_status = 0;
	_check(XPRSpresolverow(m_model.get(), g_sense, numnz, cind, cval, g_rhs, ncols, &ps_numnz,
	                       ps_cind.data(), ps_cval.data(), &ps_rhs, &ps_status));
	if (ps_status != 0)
	{
		throw std::runtime_error("Failed to presolve new cut.");
	}

	XPRSint64 start[] = {0, ps_numnz};
	int ctype = 1;
	if (cb_where == CB_CONTEXT::cutround)
	{
		// NOTE: we assume cuts to be global since other solvers only support those
		_check(XPRSaddmanagedcuts64(m_model.get(), 1, 1, &g_sense, &ps_rhs, start, ps_cind.data(),
		                            ps_cval.data()));
	}
	else
	{
		_check(XPRSaddcuts64(m_model.get(), 1, &ctype, &g_sense, &ps_rhs, start, ps_cind.data(),
		                     ps_cval.data()));
	}
}

// Tries to add a lazy constraints. If the context is right, but we are not in a node of the tree,
// it fallbacks to simply rejecting the solution, since no cut can be added at that moment.
void Model::cb_add_lazy_constraint(const ScalarAffineFunction &function, ConstraintSense sense,
                                   CoeffT rhs)
{
	_check_expected_mode(XPRESS_MODEL_MODE::CALLBACK_);
	if (cb_where != CB_CONTEXT::nodelpsolved && cb_where != CB_CONTEXT::optnode &&
	    cb_where != CB_CONTEXT::preintsol && cb_where != CB_CONTEXT::prenode)
	{
		throw std::runtime_error("New constraints can be added only in NODELPSOLVED, OPTNODE, "
		                         "PREINTSOL, and PRENODE callbacks.");
	}

	if (cb_where != CB_CONTEXT::preintsol)
	{
		_cb_add_cut(function, sense, rhs);
		return;
	}

	auto *args = std::get<preintsol_struct *>(cb_args);
	if (args->soltype == 0)
	{
		_cb_add_cut(function, sense, rhs);
		return;
	}

	// If the solution didn't originated from a node of the tree, we can't reject it with a cut.
	// However, if the user cut makes the solution infeasible, we have to reject it.
	double pos_activity = 0.0;
	double neg_anctivity = 0.0;
	const int nnz = function.size();
	for (int i = 0; i < nnz; ++i)
	{
		double col_val = _cb_get_context_solution(function.variables[i]);
		double term_val = col_val * function.coefficients[i];
		(term_val > 0.0 ? pos_activity : neg_anctivity) += term_val;
	}
	const double activity = pos_activity + neg_anctivity;
	const double real_rhs = static_cast<double>(rhs - function.constant.value_or(0.0));
	double infeas = 0.0; // > 0 if solution violates constraint
	if (sense == ConstraintSense::Equal || sense == ConstraintSense::LessEqual)
	{
		infeas = std::max(infeas, activity - real_rhs);
	}
	if (sense == ConstraintSense::Equal || sense == ConstraintSense::GreaterEqual)
	{
		infeas = std::max(infeas, real_rhs - activity);
	}
	const double feastol = get_raw_control_dbl_by_id(POI_XPRS_FEASTOL);
	if (infeas > feastol)
	{
		// The user added a cut, but we are not in a context where it can be added. So, the only
		// thing we can reasonably do is to reject the solution iff it is made infeasible by the
		// user provided cut.
		*args->p_reject = 1;
	}
}

void Model::cb_add_lazy_constraint(const ExprBuilder &function, ConstraintSense sense, CoeffT rhs)
{
	ScalarAffineFunction f(function);
	cb_add_lazy_constraint(f, sense, rhs);
}

void Model::cb_add_user_cut(const ScalarAffineFunction &function, ConstraintSense sense, CoeffT rhs)
{
	_cb_add_cut(function, sense, rhs);
}

void Model::cb_add_user_cut(const ExprBuilder &function, ConstraintSense sense, CoeffT rhs)
{
	ScalarAffineFunction f(function);
	cb_add_user_cut(f, sense, rhs);
}

// Helper struct that defines a static function when instantiated.
// The defined static function is the actual Xpress CB that will be registered to the context
// selected by the Where argument.
// It collects the due-diligence common to all CBs, to minimize code duplication.
template <unsigned long long Where, typename CbStruct, typename RetT, typename... ArgsT>
struct Model::CbWrap
{
	static RetT cb(XPRSprob cb_prob, void *user_data, ArgsT... args) noexcept
	{
		auto *model = reinterpret_cast<Model *>(user_data);
		assert(model->m_mode == XPRESS_MODEL_MODE::MAIN);

		auto cb_args = CbStruct{{/*ret_code*/}, args...}; // Store additional arguments

		// Temporarily swap the XpressModel's problem pointer with the callback's thread-local
		// clone. This allows reusing the model's indexers and helper data without copying.
		//
		// Current design assumes serialized callback invocation (enforced by Python GIL). The swap
		// is safe because only one callback executes at a time.
		//
		// NOTE: Free-threading compatibility will require redesign. The approach would be to split
		// Model state into:
		//   - Immutable shared state (indexers, problem structure) shared through a common pointer,
		//   read-only when in callbacks
		//   - Thread-local state (callback context, temporary buffers)
		//
		// Instead of swapping problem pointers, create lightweight callback problem objects that
		// reference the shared state. This is conceptually simple but requires refactoring all
		// Model methods to access shared state through indirection.
		XPRSprob main_prob = model->_toggle_model_mode(cb_prob);
		// Ensure restoration on all exit paths
		Defer main_prob_restore = [&] { model->_toggle_model_mode(main_prob); };

		try
		{
			model->_check_expected_mode(XPRESS_MODEL_MODE::CALLBACK_);
			model->cb_sol_cache.clear();
			model->cb_where = static_cast<CB_CONTEXT>(Where);
			model->cb_args = &cb_args;
			model->m_callback(model, static_cast<CB_CONTEXT>(Where));
			model->cb_submit_solution();
		}
		catch (...)
		{
			// We cannot let any exception slip through a callback, we have to catch it,
			// terminate Xpress gracefully and then we can throw it again.
			if (XPRSinterrupt(cb_prob, POI_XPRS_STOP_USER) != 0)
			{
				std::rethrow_exception(std::current_exception()); // We have to terminate somehow
			}
			model->m_captured_exceptions.push_back(std::current_exception());
		}

		return cb_args.get_return_value();
	}
};

static constexpr unsigned long long as_flag(int ID)
{
	assert("ID must be in the [0, 63] range" && ID >= 0 && ID < 63);
	return (1ULL << ID);
}

static constexpr bool test_ctx(CB_CONTEXT dest_ctx, unsigned long long curr_ctx)
{
	auto ctx = static_cast<unsigned long long>(dest_ctx);
	return (curr_ctx & ctx) != 0; // The context matches the ID
}

void Model::set_callback(const Callback &cb, unsigned long long new_contexts)
{
	_check_expected_mode(XPRESS_MODEL_MODE::MAIN);

	// Default message callback management - we always register a default message handler
	// unless the user explicitly registers their own. When the user callback is removed,
	// restore the default handler.
	if (is_default_message_cb_set && test_ctx(CB_CONTEXT::message, new_contexts))
	{
		_check(XPRSremovecbmessage(m_model.get(), &default_print, nullptr));
		is_default_message_cb_set = false;
	}
	if (!is_default_message_cb_set && !test_ctx(CB_CONTEXT::message, new_contexts))
	{
		_check(XPRSaddcbmessage(m_model.get(), &default_print, nullptr, 0));
		is_default_message_cb_set = true;
	}

	// Register/unregister Xpress callbacks based on context changes. For each callback type,
	// compare the old context set (m_curr_contexts) with the new one. If a context needs to be
	// added or removed, register/unregister the corresponding low-level wrapper function.
	//
	// Note: The wrapper functions are stateless - they just forward to the user callback pointer.
	// Updating the callback for an already-registered context only requires updating m_callback;
	// the wrapper stays registered.

#define XPRSCB_SET_CTX(ID, NAME, RET, ...)                                       \
	{                                                                            \
		bool has_cb = test_ctx(CB_CONTEXT::NAME, m_curr_contexts);               \
		bool needs_cb = test_ctx(CB_CONTEXT::NAME, new_contexts);                \
		if (has_cb != needs_cb)                                                  \
		{                                                                        \
			auto *cb = &CbWrap<as_flag(ID), NAME##_struct, RET __VA_ARGS__>::cb; \
			if (has_cb)                                                          \
			{                                                                    \
				_check(XPRSremovecb##NAME(m_model.get(), cb, this));             \
			}                                                                    \
			else /* needs_cb */                                                  \
			{                                                                    \
				_check(XPRSaddcb##NAME(m_model.get(), cb, this, 0));             \
			}                                                                    \
		}                                                                        \
	}
	XPRSCB_LIST(XPRSCB_SET_CTX, XPRSCB_ARG_TYPE)
#undef XPRSCB_SET_CTX

	m_curr_contexts = new_contexts;
	m_callback = cb;
}
} // namespace xpress
