#include "pyoptinterface/core.hpp"
#include <ranges>
#include <algorithm>
#include <stdexcept>
#include <format>
#include <assert.h>

VariableIndex::VariableIndex(IndexT v) : index(v)
{
}

ScalarAffineFunction::ScalarAffineFunction(CoeffT c) : constant(c)
{
}
ScalarAffineFunction::ScalarAffineFunction(const VariableIndex &v)
    : coefficients({1.0}), variables({v.index})
{
}
ScalarAffineFunction::ScalarAffineFunction(const VariableIndex &v, CoeffT c)
    : coefficients({c}), variables({v.index})
{
}
ScalarAffineFunction::ScalarAffineFunction(const VariableIndex &v, CoeffT c1, CoeffT c2)
    : coefficients({c1}), variables({v.index}), constant(c2)
{
}
ScalarAffineFunction::ScalarAffineFunction(const Vector<CoeffT> &a, const Vector<IndexT> &b)
    : coefficients(a), variables(b)
{
}
ScalarAffineFunction::ScalarAffineFunction(const Vector<CoeffT> &a, const Vector<IndexT> &b,
                                           const std::optional<CoeffT> &c)
    : coefficients(a), variables(b), constant(c)
{
}
ScalarAffineFunction::ScalarAffineFunction(const ExprBuilder &t)
{
	const auto &affine_terms = t.affine_terms;

	auto N = affine_terms.size();
	coefficients.reserve(N);
	variables.reserve(N);

	for (auto &[v, c] : affine_terms)
	{
		coefficients.push_back(c);
		variables.push_back(v);
	}
}

size_t ScalarAffineFunction::size() const
{
	return coefficients.size();
}
void ScalarAffineFunction::canonicalize(CoeffT threshold)
{
	ExprBuilder t(*this);
	t.clean_nearzero_terms(threshold);

	*this = ScalarAffineFunction(t);
	// sort coefficients and variable lock by lock with std::views::zip
	std::ranges::sort(std::views::zip(variables, coefficients));
}

ScalarQuadraticFunction::ScalarQuadraticFunction(const Vector<CoeffT> &c, const Vector<IndexT> &v1,
                                                 const Vector<IndexT> &v2)
    : coefficients(c), variable_1s(v1), variable_2s(v2)
{
}
ScalarQuadraticFunction::ScalarQuadraticFunction(const Vector<CoeffT> &c, const Vector<IndexT> &v1,
                                                 const Vector<IndexT> &v2,
                                                 const std::optional<ScalarAffineFunction> &a)
    : coefficients(c), variable_1s(v1), variable_2s(v2), affine_part(a)
{
}
ScalarQuadraticFunction::ScalarQuadraticFunction(const ExprBuilder &t)
{
	const auto &quadratic_terms = t.quadratic_terms;
	auto N = quadratic_terms.size();
	coefficients.reserve(N);
	variable_1s.reserve(N);
	variable_2s.reserve(N);

	for (auto &[vp, c] : quadratic_terms)
	{
		coefficients.push_back(c);
		variable_1s.push_back(vp.var_1);
		variable_2s.push_back(vp.var_2);
	}
	std::optional<ScalarAffineFunction> affine_part;
	if (!t.affine_terms.empty())
	{
		affine_part = ScalarAffineFunction(t);
	}
}
size_t ScalarQuadraticFunction::size() const
{
	return coefficients.size();
}
void ScalarQuadraticFunction::canonicalize(CoeffT threshold)
{
	ExprBuilder t(*this);
	t.clean_nearzero_terms(threshold);

	*this = ScalarQuadraticFunction(t);
	std::ranges::sort(std::views::zip(variable_1s, variable_2s, coefficients));
}

bool VariablePair::operator==(const VariablePair &x) const
{
	return var_1 == x.var_1 && var_2 == x.var_2;
}
bool VariablePair::operator<(const VariablePair &x) const
{
	if (var_1 != x.var_1)
		return var_1 < x.var_1;
	else
		return var_1 < var_2;
}

ExprBuilder::ExprBuilder(const VariableIndex &v)
{
	this->add(v);
}

ExprBuilder::ExprBuilder(const ScalarAffineFunction &a)
{
	auto N = a.coefficients.size();
	affine_terms.reserve(N);

	this->add(a);
}

ExprBuilder::ExprBuilder(const ScalarQuadraticFunction &q)
{
	if (q.affine_part)
	{
		affine_terms.reserve(q.affine_part.value().coefficients.size());
	}
	auto N = q.coefficients.size();
	quadratic_terms.reserve(N);

	this->add(q);
}

bool ExprBuilder::empty() const
{
	return quadratic_terms.empty() && affine_terms.empty() && !constant_term;
}

int ExprBuilder::degree() const
{
	if (!quadratic_terms.empty())
	{
		return 2;
	}
	if (!affine_terms.empty())
	{
		return 1;
	}
	return 0;
}

void ExprBuilder::clear()
{
	quadratic_terms.clear();
	affine_terms.clear();
	constant_term.reset();
}

void ExprBuilder::clean_nearzero_terms(CoeffT threshold)
{
	// clear all coeficients if abs(coefficient) < threshold
	for (auto it = quadratic_terms.begin(); it != quadratic_terms.end();)
	{
		if (std::abs(it->second) < threshold)
		{
			it = quadratic_terms.erase(it);
		}
		else
		{
			++it;
		}
	}
	for (auto it = affine_terms.begin(); it != affine_terms.end();)
	{
		if (std::abs(it->second) < threshold)
		{
			it = affine_terms.erase(it);
		}
		else
		{
			++it;
		}
	}
	if (constant_term && std::abs(*constant_term) < threshold)
	{
		constant_term.reset();
	}
}

void ExprBuilder::_add_quadratic_term(IndexT i, IndexT j, CoeffT coeff)
{
	if (i > j)
	{
		std::swap(i, j);
	}
	auto it = quadratic_terms.find({i, j});
	if (it == quadratic_terms.end())
	{
		quadratic_terms.insert({{i, j}, coeff});
	}
	else
	{
		it->second += coeff;
	}
}
void ExprBuilder::_set_quadratic_coef(IndexT i, IndexT j, CoeffT coeff)
{
	if (i > j)
	{
		std::swap(i, j);
	}
	auto it = quadratic_terms.find({i, j});
	if (it == quadratic_terms.end())
	{
		quadratic_terms.insert({{i, j}, coeff});
	}
	else
	{
		it->second = coeff;
	}
}
void ExprBuilder::add_quadratic_term(const VariableIndex &i, const VariableIndex &j, CoeffT coeff)
{
	_add_quadratic_term(i.index, j.index, coeff);
}
void ExprBuilder::set_quadratic_coef(const VariableIndex &i, const VariableIndex &j, CoeffT coeff)
{
	_set_quadratic_coef(i.index, j.index, coeff);
}

void ExprBuilder::_add_affine_term(IndexT i, CoeffT coeff)
{
	auto it = affine_terms.find(i);
	if (it == affine_terms.end())
	{
		affine_terms.insert({i, coeff});
	}
	else
	{
		it->second += coeff;
	}
}
void ExprBuilder::_set_affine_coef(IndexT i, CoeffT coeff)
{
	auto it = affine_terms.find(i);
	if (it == affine_terms.end())
	{
		affine_terms.insert({i, coeff});
	}
	else
	{
		it->second = coeff;
	}
}
void ExprBuilder::add_affine_term(const VariableIndex &i, CoeffT coeff)
{
	_add_affine_term(i.index, coeff);
}
void ExprBuilder::set_affine_coef(const VariableIndex &i, CoeffT coeff)
{
	_set_affine_coef(i.index, coeff);
}

void ExprBuilder::add(CoeffT c)
{
	constant_term = constant_term.value_or(0.0) + c;
}
void ExprBuilder::add(const VariableIndex &v)
{
	_add_affine_term(v.index, 1.0);
}
void ExprBuilder::add(const ScalarAffineFunction &a)
{
	auto N = a.coefficients.size();
	for (auto i = 0; i < N; i++)
	{
		_add_affine_term(a.variables[i], a.coefficients[i]);
	}

	if (a.constant)
	{
		constant_term = constant_term.value_or(0.0) + a.constant.value();
	}
}
void ExprBuilder::add(const ScalarQuadraticFunction &q)
{
	if (q.affine_part)
	{
		this->add(q.affine_part.value());
	}

	auto N = q.coefficients.size();
	for (auto i = 0; i < N; i++)
	{
		_add_quadratic_term(q.variable_1s[i], q.variable_2s[i], q.coefficients[i]);
	}
}
void ExprBuilder::add(const ExprBuilder &t)
{
	for (const auto &[varpair, c] : t.quadratic_terms)
	{
		_add_quadratic_term(varpair.var_1, varpair.var_2, c);
	}
	for (const auto &[v, c] : t.affine_terms)
	{
		_add_affine_term(v, c);
	}
	if (t.constant_term)
	{
		constant_term = constant_term.value_or(0.0) + t.constant_term.value();
	}
}

void ExprBuilder::sub(CoeffT c)
{
	constant_term = constant_term.value_or(0.0) - c;
}
void ExprBuilder::sub(const VariableIndex &v)
{
	_add_affine_term(v.index, -1.0);
}
void ExprBuilder::sub(const ScalarAffineFunction &a)
{
	auto N = a.coefficients.size();
	for (auto i = 0; i < N; i++)
	{
		_add_affine_term(a.variables[i], -a.coefficients[i]);
	}

	if (a.constant)
	{
		constant_term = constant_term.value_or(0.0) - a.constant.value();
	}
}
void ExprBuilder::sub(const ScalarQuadraticFunction &q)
{
	if (q.affine_part)
	{
		this->add(q.affine_part.value());
	}

	auto N = q.coefficients.size();
	for (auto i = 0; i < N; i++)
	{
		_add_quadratic_term(q.variable_1s[i], q.variable_2s[i], -q.coefficients[i]);
	}
}
void ExprBuilder::sub(const ExprBuilder &t)
{
	for (const auto &[varpair, c] : t.quadratic_terms)
	{
		_add_quadratic_term(varpair.var_1, varpair.var_2, -c);
	}
	for (const auto &[v, c] : t.affine_terms)
	{
		_add_affine_term(v, -c);
	}
	if (t.constant_term)
	{
		constant_term = constant_term.value_or(0.0) - t.constant_term.value();
	}
}

void ExprBuilder::mul(CoeffT c)
{
	if (constant_term)
	{
		constant_term = constant_term.value() * c;
	}
}
void ExprBuilder::mul(const VariableIndex &v)
{
	auto deg = degree();
	if (deg > 1)
	{
		throw std::logic_error(
		    std::format("ExprBuilder with degree {} cannot multiply with VariableIndex", deg));
	}

	auto N = affine_terms.size();
	quadratic_terms.reserve(N);
	for (const auto &[var2, c] : affine_terms)
	{
		_add_quadratic_term(v.index, var2, c);
	}

	if (constant_term)
	{
		affine_terms = {{v.index, constant_term.value()}};
		constant_term.reset();
	}
	else
	{
		affine_terms.clear();
	}
}
void ExprBuilder::mul(const ScalarAffineFunction &a)
{
	auto deg = degree();
	if (deg > 1)
	{
		throw std::logic_error(std::format(
		    "ExprBuilder with degree {} cannot multiply with ScalarAffineFunction", deg));
	}

	auto N1 = affine_terms.size();
	auto N2 = a.size();
	quadratic_terms.reserve(N1 * N2 / 2);
	for (const auto &[xi, ci] : affine_terms)
	{
		for (int j = 0; j < a.size(); j++)
		{
			auto dj = a.coefficients[j];
			auto xj = a.variables[j];
			_add_quadratic_term(xi, xj, ci * dj);
		}
	}

	if (a.constant)
	{
		auto d0 = a.constant.value();
		for (auto &[_, ci] : affine_terms)
		{
			ci *= d0;
		}
	}

	if (constant_term)
	{
		auto c0 = constant_term.value();
		for (int j = 0; j < a.size(); j++)
		{
			auto dj = a.coefficients[j];
			auto xj = a.variables[j];
			_add_affine_term(xj, c0 * dj);
		}
	}

	if (a.constant && constant_term)
	{
		constant_term = a.constant.value() * constant_term.value();
	}
}
void ExprBuilder::mul(const ScalarQuadraticFunction &q)
{
	auto deg = degree();
	if (deg > 0)
	{
		throw std::logic_error(std::format(
		    "ExprBuilder with degree {} cannot multiply with ScalarQuadraticFunction", deg));
	}

	auto N = q.coefficients.size();
	if (constant_term)
	{
		quadratic_terms.reserve(N);
		auto c = constant_term.value();
		for (auto i = 0; i < N; i++)
		{
			_add_quadratic_term(q.variable_1s[i], q.variable_2s[i], c * q.coefficients[i]);
		}
	}
}
void ExprBuilder::mul(const ExprBuilder &t)
{
	auto deg1 = degree();
	auto deg2 = t.degree();
	if (deg1 + deg2 > 2)
	{
		throw std::logic_error(std::format(
		    "ExprBuilder with degree {} cannot multiply with ExprBuilder with degree {}", deg1,
		    deg2));
	}

	if (deg1 == 0)
	{
		if (constant_term)
		{
			auto c = constant_term.value();

			auto N = t.quadratic_terms.size();
			quadratic_terms.reserve(N);
			for (const auto &[varpair, c2] : t.quadratic_terms)
			{
				_add_quadratic_term(varpair.var_1, varpair.var_2, c * c2);
			}

			N = t.affine_terms.size();
			affine_terms.reserve(N);
			for (const auto &[v, c1] : t.affine_terms)
			{
				_add_affine_term(v, c * c1);
			}

			if (t.constant_term)
			{
				constant_term = c * t.constant_term.value();
			}
			else
			{
				constant_term.reset();
			}
		}
	}
	else if (deg1 == 1)
	{
		if (deg2 == 1)
		{
			auto N1 = affine_terms.size();
			auto N2 = t.affine_terms.size();
			quadratic_terms.reserve(N1 * N2 / 2);
			for (const auto &[xi, ci] : affine_terms)
			{
				for (const auto &[xj, dj] : t.affine_terms)
				{
					_add_quadratic_term(xi, xj, ci * dj);
				}
			}
		}

		if (t.constant_term)
		{
			auto d0 = t.constant_term.value();
			for (auto &[_, ci] : affine_terms)
			{
				ci *= d0;
			}
		}

		if (constant_term)
		{
			auto c0 = constant_term.value();
			for (const auto &[xj, dj] : t.affine_terms)
			{
				_add_affine_term(xj, c0 * dj);
			}
		}

		if (t.constant_term && constant_term)
		{
			constant_term = t.constant_term.value() * constant_term.value();
		}
	}
	else if (deg1 == 2)
	{
		if (t.constant_term)
		{
			auto c = t.constant_term.value();

			for (auto &[_, c2] : quadratic_terms)
			{
				c2 *= c;
			}

			for (auto &[_, c1] : affine_terms)
			{
				c1 *= c;
			}

			if (constant_term)
			{
				constant_term = c * constant_term.value();
			}
		}
		else
		{
			clear();
		}
	}
}

// Operator overloading functions

auto operator+(const VariableIndex &a, CoeffT b) -> ScalarAffineFunction
{
	return ScalarAffineFunction(a, 1.0, b);
}
auto operator+(CoeffT b, const VariableIndex &a) -> ScalarAffineFunction
{
	return a + b;
}

auto operator+(const VariableIndex &a, const VariableIndex &b) -> ScalarAffineFunction
{
	return ScalarAffineFunction({1.0, 1.0}, {a.index, b.index});
}

auto operator+(const ScalarAffineFunction &a, CoeffT b) -> ScalarAffineFunction
{
	CoeffT new_constant = a.constant.value_or(0.0) + b;
	return ScalarAffineFunction(a.coefficients, a.variables, new_constant);
}
auto operator+(CoeffT b, const ScalarAffineFunction &a) -> ScalarAffineFunction
{
	return a + b;
}

auto operator+(const ScalarAffineFunction &a, const VariableIndex &b) -> ScalarAffineFunction
{
	auto coefficients = a.coefficients;
	auto variables = a.variables;
	coefficients.push_back(1.0);
	variables.push_back(b.index);
	return ScalarAffineFunction(coefficients, variables, a.constant);
}
auto operator+(const VariableIndex &b, const ScalarAffineFunction &a) -> ScalarAffineFunction
{
	return a + b;
}

auto operator+(const ScalarAffineFunction &a, const ScalarAffineFunction &b) -> ScalarAffineFunction
{
	ExprBuilder t(a);
	t.add(b);
	return ScalarAffineFunction(t);
}

auto operator+(const ScalarQuadraticFunction &a, CoeffT b) -> ScalarQuadraticFunction
{
	ScalarAffineFunction affine_part(b);
	if (a.affine_part)
	{
		affine_part = operator+(a.affine_part.value(), b);
	}
	return ScalarQuadraticFunction(a.coefficients, a.variable_1s, a.variable_2s, affine_part);
}
auto operator+(CoeffT b, const ScalarQuadraticFunction &a) -> ScalarQuadraticFunction
{
	return a + b;
}

auto operator+(const ScalarQuadraticFunction &a, const VariableIndex &b) -> ScalarQuadraticFunction
{
	ScalarAffineFunction affine_part(b);
	if (a.affine_part)
	{
		affine_part = operator+(a.affine_part.value(), b);
	}
	return ScalarQuadraticFunction(a.coefficients, a.variable_1s, a.variable_2s, affine_part);
}
auto operator+(const VariableIndex &b, const ScalarQuadraticFunction &a) -> ScalarQuadraticFunction
{
	return a + b;
}

auto operator+(const ScalarQuadraticFunction &a, const ScalarAffineFunction &b)
    -> ScalarQuadraticFunction
{
	ScalarAffineFunction affine_part(b);
	if (a.affine_part)
	{
		affine_part = operator+(a.affine_part.value(), b);
	}
	return ScalarQuadraticFunction(a.coefficients, a.variable_1s, a.variable_2s, affine_part);
}
auto operator+(const ScalarAffineFunction &b, const ScalarQuadraticFunction &a)
    -> ScalarQuadraticFunction
{
	return a + b;
}

auto operator+(const ScalarQuadraticFunction &a, const ScalarQuadraticFunction &b)
    -> ScalarQuadraticFunction
{
	ExprBuilder t(a);
	t.add(b);
	return ScalarQuadraticFunction(t);
}

auto operator-(const VariableIndex &a, CoeffT b) -> ScalarAffineFunction
{
	return ScalarAffineFunction(a, 1.0, -b);
}

auto operator-(CoeffT b, const VariableIndex &a) -> ScalarAffineFunction
{
	return ScalarAffineFunction(a, -1.0, b);
}

auto operator-(const VariableIndex &a, const VariableIndex &b) -> ScalarAffineFunction
{
	return ScalarAffineFunction({1.0, -1.0}, {a.index, b.index});
}

auto operator-(const ScalarAffineFunction &a, CoeffT b) -> ScalarAffineFunction
{
	return operator+(a, -b);
}

auto operator-(CoeffT b, const ScalarAffineFunction &a) -> ScalarAffineFunction
{
	ScalarAffineFunction aa = a;
	for (auto &c : aa.coefficients)
	{
		c *= -1.0;
	}
	if (aa.constant)
	{
		aa.constant = -aa.constant.value();
	}
	return operator+(aa, b);
}

auto operator-(const ScalarAffineFunction &a, const VariableIndex &b) -> ScalarAffineFunction
{
	auto coefficients = a.coefficients;
	auto variables = a.variables;
	coefficients.push_back(-1.0);
	variables.push_back(b.index);
	return ScalarAffineFunction(coefficients, variables, a.constant);
}

auto operator-(const VariableIndex &b, const ScalarAffineFunction &a) -> ScalarAffineFunction
{
	return operator+(operator*(a, -1.0), b);
}

auto operator-(const ScalarAffineFunction &a, const ScalarAffineFunction &b) -> ScalarAffineFunction
{
	return operator+(operator*(a, -1.0), b);
}

auto operator-(const ScalarQuadraticFunction &a, CoeffT b) -> ScalarQuadraticFunction
{
	return operator+(a, -b);
}

auto operator-(CoeffT b, const ScalarQuadraticFunction &a) -> ScalarQuadraticFunction
{
	return operator+(operator*(a, -1.0), b);
}

auto operator-(const ScalarQuadraticFunction &a, const VariableIndex &b) -> ScalarQuadraticFunction
{
	return operator+(a, ScalarAffineFunction(b, -1.0));
}

auto operator-(const VariableIndex &b, const ScalarQuadraticFunction &a) -> ScalarQuadraticFunction
{
	return operator+(operator*(a, -1.0), b);
}

auto operator-(const ScalarQuadraticFunction &a, const ScalarAffineFunction &b)
    -> ScalarQuadraticFunction
{
	return operator+(a, operator*(b, -1.0));
}

auto operator-(const ScalarAffineFunction &b, const ScalarQuadraticFunction &a)
    -> ScalarQuadraticFunction
{
	return operator+(operator*(a, -1.0), b);
}

auto operator-(const ScalarQuadraticFunction &a, const ScalarQuadraticFunction &b)
    -> ScalarQuadraticFunction
{
	return operator+(operator*(a, -1.0), b);
}

auto operator*(const VariableIndex &a, CoeffT b) -> ScalarAffineFunction
{
	return ScalarAffineFunction(a, b);
}
auto operator*(CoeffT b, const VariableIndex &a) -> ScalarAffineFunction
{
	return a * b;
}

auto operator*(const VariableIndex &a, const VariableIndex &b) -> ScalarQuadraticFunction
{
	return ScalarQuadraticFunction({1.0}, {a.index}, {b.index});
}

auto operator*(const ScalarAffineFunction &a, CoeffT b) -> ScalarAffineFunction
{
	ScalarAffineFunction aa = a;
	for (auto &c : aa.coefficients)
	{
		c *= b;
	}
	if (aa.constant)
	{
		aa.constant = aa.constant.value() * b;
	}
	return aa;
}
auto operator*(CoeffT b, const ScalarAffineFunction &a) -> ScalarAffineFunction
{
	return b * a;
}

auto operator*(const ScalarAffineFunction &a, const VariableIndex &b) -> ScalarQuadraticFunction
{
	auto &coefficients = a.coefficients;
	auto &variable_1s = a.variables;
	auto variable_2s = Vector<IndexT>(variable_1s.size(), b.index);

	std::optional<ScalarAffineFunction> affine_part;
	if (a.constant)
	{
		affine_part = operator*(b, a.constant.value());
	}

	return ScalarQuadraticFunction(coefficients, variable_1s, variable_2s, affine_part);
}
auto operator*(const VariableIndex &b, const ScalarAffineFunction &a) -> ScalarQuadraticFunction
{
	return a * b;
}

auto operator*(const ScalarAffineFunction &a, const ScalarAffineFunction &b)
    -> ScalarQuadraticFunction
{
	auto t = ExprBuilder();

	for (int i = 0; i < a.coefficients.size(); i++)
	{
		auto ci = a.coefficients[i];
		auto xi = a.variables[i];
		for (int j = 0; j < b.coefficients.size(); j++)
		{
			auto dj = b.coefficients[j];
			auto xj = b.variables[j];
			t._add_quadratic_term(xi, xj, ci * dj);
		}
	}
	if (b.constant)
	{
		auto d0 = b.constant.value();
		for (int i = 0; i < a.coefficients.size(); i++)
		{
			auto ci = b.coefficients[i];
			auto xi = b.variables[i];
			t._add_affine_term(xi, ci * d0);
		}
	}
	if (a.constant)
	{
		auto c0 = a.constant.value();
		for (int j = 0; j < b.coefficients.size(); j++)
		{
			auto dj = b.coefficients[j];
			auto xj = b.variables[j];
			t._add_affine_term(xj, c0 * dj);
		}
	}
	if (a.constant && b.constant)
	{
		t.constant_term = a.constant.value() * b.constant.value();
	}
	return ScalarQuadraticFunction(t);
}

auto operator*(const ScalarQuadraticFunction &a, CoeffT b) -> ScalarQuadraticFunction
{
	ScalarQuadraticFunction aa = a;
	for (auto &c : aa.coefficients)
	{
		c *= b;
	}
	if (aa.affine_part)
	{
		aa.affine_part = operator*(aa.affine_part.value(), b);
	}
	return aa;
}
auto operator*(CoeffT b, const ScalarQuadraticFunction &a) -> ScalarQuadraticFunction
{
	return a * b;
}
