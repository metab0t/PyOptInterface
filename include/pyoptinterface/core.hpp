#pragma once

#include <stdint.h>
#include <vector>
#include <optional>
#include "ankerl/unordered_dense.h"

using IndexT = std::int32_t;
using CoeffT = double;

constexpr CoeffT COEFTHRESHOLD = 1e-12;

template <typename K, typename V>
using Hashmap = ankerl::unordered_dense::map<K, V>;

template <typename K>
using Hashset = ankerl::unordered_dense::set<K>;

template <typename V>
using Vector = std::vector<V>;

struct VariableIndex;
struct ScalarAffineFunction;
struct ScalarQuadraticFunction;
struct ExprBuilder;

enum class VariableDomain
{
	Continuous,
	Integer,
	Binary,
	SemiContinuous,
};

struct VariableIndex
{
	IndexT index;

	VariableIndex() = default;
	VariableIndex(IndexT v);
};

struct ScalarAffineFunction
{
	Vector<CoeffT> coefficients;
	Vector<IndexT> variables;
	std::optional<CoeffT> constant;

	ScalarAffineFunction() = default;
	ScalarAffineFunction(CoeffT v);
	ScalarAffineFunction(const VariableIndex &v);
	// v * c
	ScalarAffineFunction(const VariableIndex &v, CoeffT c);
	// v * c1 + c2
	ScalarAffineFunction(const VariableIndex &v, CoeffT c1, CoeffT c2);
	ScalarAffineFunction(const Vector<CoeffT> &, const Vector<IndexT> &);
	ScalarAffineFunction(const Vector<CoeffT> &, const Vector<IndexT> &,
	                     const std::optional<CoeffT> &);

	ScalarAffineFunction(const ExprBuilder &t);

	size_t size() const;
	void canonicalize(CoeffT threshold = COEFTHRESHOLD);

	void reserve(size_t n);
	void add_term(const VariableIndex &v, CoeffT c);
	void add_constant(CoeffT c);
};

struct ScalarQuadraticFunction
{
	Vector<CoeffT> coefficients;
	Vector<IndexT> variable_1s;
	Vector<IndexT> variable_2s;
	std::optional<ScalarAffineFunction> affine_part;

	ScalarQuadraticFunction() = default;
	ScalarQuadraticFunction(const Vector<CoeffT> &, const Vector<IndexT> &, const Vector<IndexT> &);
	ScalarQuadraticFunction(const Vector<CoeffT> &, const Vector<IndexT> &, const Vector<IndexT> &,
	                        const std::optional<ScalarAffineFunction> &);
	ScalarQuadraticFunction(const ExprBuilder &t);

	size_t size() const;
	void canonicalize(CoeffT threshold = COEFTHRESHOLD);

	void reserve_quadratic(size_t n);
	void reserve_affine(size_t n);
	void add_quadratic_term(const VariableIndex &v1, const VariableIndex &v2, CoeffT c);
	void add_affine_term(const VariableIndex &v, CoeffT c);
	void add_constant(CoeffT c);
};

struct VariablePair
{
	IndexT var_1;
	IndexT var_2;

	bool operator==(const VariablePair &x) const;
	bool operator<(const VariablePair &x) const;

	VariablePair() = default;
	VariablePair(IndexT v1, IndexT v2) : var_1(v1), var_2(v2)
	{
	}
};

template <>
struct ankerl::unordered_dense::hash<VariablePair>
{
	using is_avalanching = void;

	[[nodiscard]] auto operator()(VariablePair const &x) const noexcept -> uint64_t
	{
		static_assert(std::has_unique_object_representations_v<VariablePair>);
		return detail::wyhash::hash(&x, sizeof(x));
	}
};

struct ExprBuilder
{
	Hashmap<VariablePair, CoeffT> quadratic_terms;
	Hashmap<IndexT, CoeffT> affine_terms;
	std::optional<CoeffT> constant_term;

	ExprBuilder() = default;
	ExprBuilder(CoeffT c);
	ExprBuilder(const VariableIndex &v);
	ExprBuilder(const ScalarAffineFunction &a);
	ExprBuilder(const ScalarQuadraticFunction &q);

	ExprBuilder &operator+=(CoeffT c);
	ExprBuilder &operator+=(const VariableIndex &v);
	ExprBuilder &operator+=(const ScalarAffineFunction &a);
	ExprBuilder &operator+=(const ScalarQuadraticFunction &q);
	ExprBuilder &operator+=(const ExprBuilder &t);

	ExprBuilder &operator-=(CoeffT c);
	ExprBuilder &operator-=(const VariableIndex &v);
	ExprBuilder &operator-=(const ScalarAffineFunction &a);
	ExprBuilder &operator-=(const ScalarQuadraticFunction &q);
	ExprBuilder &operator-=(const ExprBuilder &t);

	ExprBuilder &operator*=(CoeffT c);
	ExprBuilder &operator*=(const VariableIndex &v);
	ExprBuilder &operator*=(const ScalarAffineFunction &a);
	ExprBuilder &operator*=(const ScalarQuadraticFunction &q);
	ExprBuilder &operator*=(const ExprBuilder &t);

	ExprBuilder &operator/=(CoeffT c);

	bool empty() const;
	int degree() const;

	void reserve_quadratic(size_t n);
	void reserve_affine(size_t n);

	void clear();
	void clean_nearzero_terms(CoeffT threshold = COEFTHRESHOLD);
	void _add_quadratic_term(IndexT i, IndexT j, CoeffT coeff);
	void _set_quadratic_coef(IndexT i, IndexT j, CoeffT coeff);
	void add_quadratic_term(const VariableIndex &i, const VariableIndex &j, CoeffT coeff);
	void set_quadratic_coef(const VariableIndex &i, const VariableIndex &j, CoeffT coeff);
	void _add_affine_term(IndexT i, CoeffT coeff);
	void _set_affine_coef(IndexT i, CoeffT coeff);
	void add_affine_term(const VariableIndex &i, CoeffT coeff);
	void set_affine_coef(const VariableIndex &i, CoeffT coeff);
};

auto operator+(const VariableIndex &a, CoeffT b) -> ScalarAffineFunction;
auto operator+(CoeffT a, const VariableIndex &b) -> ScalarAffineFunction;
auto operator+(const VariableIndex &a, const VariableIndex &b) -> ScalarAffineFunction;
auto operator+(const ScalarAffineFunction &a, CoeffT b) -> ScalarAffineFunction;
auto operator+(CoeffT a, const ScalarAffineFunction &b) -> ScalarAffineFunction;
auto operator+(const ScalarAffineFunction &a, const VariableIndex &b) -> ScalarAffineFunction;
auto operator+(const VariableIndex &a, const ScalarAffineFunction &b) -> ScalarAffineFunction;
auto operator+(const ScalarAffineFunction &a, const ScalarAffineFunction &b)
    -> ScalarAffineFunction;
auto operator+(const ScalarQuadraticFunction &a, CoeffT b) -> ScalarQuadraticFunction;
auto operator+(CoeffT a, const ScalarQuadraticFunction &b) -> ScalarQuadraticFunction;
auto operator+(const ScalarQuadraticFunction &a, const VariableIndex &b) -> ScalarQuadraticFunction;
auto operator+(const VariableIndex &a, const ScalarQuadraticFunction &b) -> ScalarQuadraticFunction;
auto operator+(const ScalarQuadraticFunction &a, const ScalarAffineFunction &b)
    -> ScalarQuadraticFunction;
auto operator+(const ScalarAffineFunction &a, const ScalarQuadraticFunction &b)
    -> ScalarQuadraticFunction;
auto operator+(const ScalarQuadraticFunction &a, const ScalarQuadraticFunction &b)
    -> ScalarQuadraticFunction;

auto operator-(const VariableIndex &a, CoeffT b) -> ScalarAffineFunction;
auto operator-(CoeffT a, const VariableIndex &b) -> ScalarAffineFunction;
auto operator-(const VariableIndex &a, const VariableIndex &b) -> ScalarAffineFunction;
auto operator-(const ScalarAffineFunction &a, CoeffT b) -> ScalarAffineFunction;
auto operator-(CoeffT a, const ScalarAffineFunction &b) -> ScalarAffineFunction;
auto operator-(const ScalarAffineFunction &a, const VariableIndex &b) -> ScalarAffineFunction;
auto operator-(const VariableIndex &a, const ScalarAffineFunction &b) -> ScalarAffineFunction;
auto operator-(const ScalarAffineFunction &a, const ScalarAffineFunction &b)
    -> ScalarAffineFunction;
auto operator-(const ScalarQuadraticFunction &a, CoeffT b) -> ScalarQuadraticFunction;
auto operator-(CoeffT a, const ScalarQuadraticFunction &b) -> ScalarQuadraticFunction;
auto operator-(const ScalarQuadraticFunction &a, const VariableIndex &b) -> ScalarQuadraticFunction;
auto operator-(const VariableIndex &a, const ScalarQuadraticFunction &b) -> ScalarQuadraticFunction;
auto operator-(const ScalarQuadraticFunction &a, const ScalarAffineFunction &b)
    -> ScalarQuadraticFunction;
auto operator-(const ScalarAffineFunction &a, const ScalarQuadraticFunction &b)
    -> ScalarQuadraticFunction;
auto operator-(const ScalarQuadraticFunction &a, const ScalarQuadraticFunction &b)
    -> ScalarQuadraticFunction;

auto operator*(const VariableIndex &a, CoeffT b) -> ScalarAffineFunction;
auto operator*(CoeffT a, const VariableIndex &b) -> ScalarAffineFunction;
auto operator*(const VariableIndex &a, const VariableIndex &b) -> ScalarQuadraticFunction;
auto operator*(const ScalarAffineFunction &a, CoeffT b) -> ScalarAffineFunction;
auto operator*(CoeffT a, const ScalarAffineFunction &b) -> ScalarAffineFunction;
auto operator*(const ScalarAffineFunction &a, const VariableIndex &b) -> ScalarQuadraticFunction;
auto operator*(const VariableIndex &a, const ScalarAffineFunction &b) -> ScalarQuadraticFunction;
auto operator*(const ScalarAffineFunction &a, const ScalarAffineFunction &b)
    -> ScalarQuadraticFunction;
auto operator*(const ScalarQuadraticFunction &a, CoeffT b) -> ScalarQuadraticFunction;
auto operator*(CoeffT a, const ScalarQuadraticFunction &b) -> ScalarQuadraticFunction;

auto operator/(const VariableIndex &a, CoeffT b) -> ScalarAffineFunction;
auto operator/(const ScalarAffineFunction &a, CoeffT b) -> ScalarAffineFunction;
auto operator/(const ScalarQuadraticFunction &a, CoeffT b) -> ScalarQuadraticFunction;

// unary minus operator
auto operator-(const VariableIndex &a) -> ScalarAffineFunction;
auto operator-(const ScalarAffineFunction &a) -> ScalarAffineFunction;
auto operator-(const ScalarQuadraticFunction &a) -> ScalarQuadraticFunction;
auto operator-(const ExprBuilder &a) -> ExprBuilder;

// Operator overloading for	ExprBuilder
// Sadly, they are inefficient than the +=,-=,*=,/= functions but they are important for a
// user-friendly interface
// The functions are like ScalarQuadraticFunction but returns a ExprBuilder
auto operator+(const ExprBuilder &a, CoeffT b) -> ExprBuilder;
auto operator+(CoeffT b, const ExprBuilder &a) -> ExprBuilder;
auto operator+(const ExprBuilder &a, const VariableIndex &b) -> ExprBuilder;
auto operator+(const VariableIndex &b, const ExprBuilder &a) -> ExprBuilder;
auto operator+(const ExprBuilder &a, const ScalarAffineFunction &b) -> ExprBuilder;
auto operator+(const ScalarAffineFunction &b, const ExprBuilder &a) -> ExprBuilder;
auto operator+(const ExprBuilder &a, const ScalarQuadraticFunction &b) -> ExprBuilder;
auto operator+(const ScalarQuadraticFunction &b, const ExprBuilder &a) -> ExprBuilder;
auto operator+(const ExprBuilder &a, const ExprBuilder &b) -> ExprBuilder;

auto operator-(const ExprBuilder &a, CoeffT b) -> ExprBuilder;
auto operator-(CoeffT b, const ExprBuilder &a) -> ExprBuilder;
auto operator-(const ExprBuilder &a, const VariableIndex &b) -> ExprBuilder;
auto operator-(const VariableIndex &b, const ExprBuilder &a) -> ExprBuilder;
auto operator-(const ExprBuilder &a, const ScalarAffineFunction &b) -> ExprBuilder;
auto operator-(const ScalarAffineFunction &b, const ExprBuilder &a) -> ExprBuilder;
auto operator-(const ExprBuilder &a, const ScalarQuadraticFunction &b) -> ExprBuilder;
auto operator-(const ScalarQuadraticFunction &b, const ExprBuilder &a) -> ExprBuilder;
auto operator-(const ExprBuilder &a, const ExprBuilder &b) -> ExprBuilder;

auto operator*(const ExprBuilder &a, CoeffT b) -> ExprBuilder;
auto operator*(CoeffT b, const ExprBuilder &a) -> ExprBuilder;
auto operator*(const ExprBuilder &a, const VariableIndex &b) -> ExprBuilder;
auto operator*(const VariableIndex &b, const ExprBuilder &a) -> ExprBuilder;
auto operator*(const ExprBuilder &a, const ScalarAffineFunction &b) -> ExprBuilder;
auto operator*(const ScalarAffineFunction &b, const ExprBuilder &a) -> ExprBuilder;
auto operator*(const ExprBuilder &a, const ScalarQuadraticFunction &b) -> ExprBuilder;
auto operator*(const ScalarQuadraticFunction &b, const ExprBuilder &a) -> ExprBuilder;
auto operator*(const ExprBuilder &a, const ExprBuilder &b) -> ExprBuilder;

auto operator/(const ExprBuilder &a, CoeffT b) -> ExprBuilder;

enum class ConstraintType
{
	Linear,
	Quadratic,
	SOS,
	SecondOrderCone,
	ExponentialCone,
	SolverDefinedOrNL,
	Gurobi_General,
	COPT_NL,
	IPOPT_NL,
	Xpress_Nlp,
	KNITRO_NL,
};

enum class SOSType
{
	SOS1,
	SOS2,
};

enum class ConstraintSense
{
	LessEqual,
	GreaterEqual,
	Equal
};

struct ConstraintIndex
{
	ConstraintType type;
	IndexT index;

	ConstraintIndex() = default;
	ConstraintIndex(ConstraintType t, IndexT i) : type(t), index(i)
	{
	}
};

// struct LinearConstraint
//{
//	ScalarAffineFunction function;
//	ConstraintSense sense;
//	CoeffT rhs;
// };
//
// struct QuadraticConstraint
//{
//	ScalarQuadraticFunction function;
//	ConstraintSense sense;
//	CoeffT rhs;
// };
//
// struct SOSConstraint
//{
//	Vector<VariableIndex> variables;
//	Vector<CoeffT> weights;
// };

enum class ObjectiveSense
{
	Minimize,
	Maximize
};
