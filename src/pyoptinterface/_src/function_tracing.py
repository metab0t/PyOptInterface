from .nlexpr_ext import (
    ExpressionGraph,
    ExpressionHandle,
    UnaryOperator,
    BinaryOperator,
    TernaryOperator,
    NaryOperator,
    ArrayType,
)
import inspect
from dataclasses import dataclass
import functools
import math
import types

# Trace an ordinary Python function to get its expression graph

Vars = types.SimpleNamespace
Params = types.SimpleNamespace


def pow_int(graph, expr, N):
    if N == 0:
        return 1
    if N < 0:
        return graph.add_binary(BinaryOperator.Div, 1, pow_int(graph, expr, -N))
    if N == 1:
        return expr

    M, r = divmod(N, 2)

    pow_2 = graph.add_nary(NaryOperator.Mul, [expr, expr])
    pow_2M = pow_int(graph, pow_2, M)

    if r == 0:
        return pow_2M
    else:
        return graph.add_nary(NaryOperator.Mul, [pow_2M, expr])


@dataclass
class TracingExpressionHandle:
    graph: ExpressionGraph
    expression: ExpressionHandle

    def _binary_operator(self, other, op: BinaryOperator, swap=False):
        expression = self.expression
        graph = self.graph
        if isinstance(other, (int, float)):
            other = graph.add_constant(other)
        elif isinstance(other, TracingExpressionHandle):
            other = other.expression
        else:
            return NotImplemented

        if swap:
            new_expression = graph.add_binary(op, other, expression)
        else:
            new_expression = graph.add_binary(op, expression, other)
        return TracingExpressionHandle(graph, new_expression)

    def _nary_operator(self, other, op: NaryOperator):
        expression = self.expression
        graph = self.graph
        if isinstance(other, (int, float)):
            other = graph.add_constant(other)
        elif isinstance(other, TracingExpressionHandle):
            other = other.expression
        else:
            return NotImplemented

        if (
            expression.array == ArrayType.Nary
            and graph.get_nary_operator(expression) == op
        ):
            graph.append_nary(expression, other)
            return self
        else:
            new_expression = graph.add_nary(op, [expression, other])
            return TracingExpressionHandle(graph, new_expression)

    def __add__(self, other):
        return self._nary_operator(other, NaryOperator.Add)

    def __radd__(self, other):
        return self._nary_operator(other, NaryOperator.Add)

    def __sub__(self, other):
        return self._binary_operator(other, BinaryOperator.Sub)

    def __rsub__(self, other):
        return self._binary_operator(other, BinaryOperator.Sub, swap=True)

    def __mul__(self, other):
        return self._nary_operator(other, NaryOperator.Mul)

    def __rmul__(self, other):
        return self._nary_operator(other, NaryOperator.Mul)

    def __truediv__(self, other):
        return self._binary_operator(other, BinaryOperator.Div)

    def __rtruediv__(self, other):
        return self._binary_operator(other, BinaryOperator.Div, swap=True)

    def __neg__(self):
        new_expression = self.graph.add_unary(UnaryOperator.Neg, self.expression)
        return TracingExpressionHandle(self.graph, new_expression)

    def __pow__(self, other):
        # special case for integer powers, which can be implemented as a repeated multiplication
        if isinstance(other, int):
            expression = self.expression
            graph = self.graph
            new_expression = pow_int(graph, expression, other)
            return TracingExpressionHandle(graph, new_expression)
        return self._binary_operator(other, BinaryOperator.Pow)

    def __rpow__(self, other):
        return self._binary_operator(other, BinaryOperator.Pow, swap=True)

    # compare operators
    def __eq__(self, other):
        return self._binary_operator(other, BinaryOperator.Equal)

    def __ne__(self, other):
        return self._binary_operator(other, BinaryOperator.NotEqual)

    def __lt__(self, other):
        return self._binary_operator(other, BinaryOperator.LessThan)

    def __le__(self, other):
        return self._binary_operator(other, BinaryOperator.LessEqual)

    def __gt__(self, other):
        return self._binary_operator(other, BinaryOperator.GreaterThan)

    def __ge__(self, other):
        return self._binary_operator(other, BinaryOperator.GreaterEqual)


# Implement unary mathematical functions
def unary_mathematical_function(math_function, op: UnaryOperator):
    @functools.wraps(math_function)
    def f(expr):
        if isinstance(expr, (int, float)):
            return math_function(expr)
        elif isinstance(expr, TracingExpressionHandle):
            expression = expr.expression
            graph = expr.graph
            new_expression = graph.add_unary(op, expression)
            return TracingExpressionHandle(graph, new_expression)
        else:
            return NotImplemented

    return f


sin = unary_mathematical_function(math.sin, UnaryOperator.Sin)
cos = unary_mathematical_function(math.cos, UnaryOperator.Cos)
tan = unary_mathematical_function(math.tan, UnaryOperator.Tan)
asin = unary_mathematical_function(math.asin, UnaryOperator.Asin)
acos = unary_mathematical_function(math.acos, UnaryOperator.Acos)
atan = unary_mathematical_function(math.atan, UnaryOperator.Atan)
abs = unary_mathematical_function(math.fabs, UnaryOperator.Abs)
sqrt = unary_mathematical_function(math.sqrt, UnaryOperator.Sqrt)
exp = unary_mathematical_function(math.exp, UnaryOperator.Exp)
log = unary_mathematical_function(math.log, UnaryOperator.Log)
log10 = unary_mathematical_function(math.log10, UnaryOperator.Log10)


# Implement binary mathematical functions
def binary_mathematical_function(math_function, op: BinaryOperator):
    @functools.wraps(math_function)
    def f(expr1, expr2):
        is_expr1 = isinstance(expr1, TracingExpressionHandle)
        is_expr2 = isinstance(expr2, TracingExpressionHandle)
        if not is_expr1 and not is_expr2:
            return math_function(expr1, expr2)

        if is_expr1:
            graph = expr1.graph
        else:
            graph = expr2.graph

        is_number1 = isinstance(expr1, (int, float))
        is_number2 = isinstance(expr2, (int, float))

        if is_number1:
            expr1 = graph.add_constant(expr1)
            is_expr1_legal = True
        elif is_expr1:
            expr1 = expr1.expression
            is_expr1_legal = True
        else:
            is_expr1_legal = False

        if is_number2:
            expr2 = graph.add_constant(expr2)
            is_expr2_legal = True
        elif is_expr2:
            expr2 = expr2.expression
            is_expr2_legal = True
        else:
            is_expr2_legal = False

        if is_expr1_legal and is_expr2_legal:
            new_expression = graph.add_binary(op, expr1, expr2)
            return TracingExpressionHandle(graph, new_expression)
        else:
            return NotImplemented

    return f


pow = binary_mathematical_function(math.pow, BinaryOperator.Pow)


def ifelse(condition, true_expr, false_expr):
    graph = None
    if isinstance(condition, TracingExpressionHandle):
        graph = condition.graph
        condition = condition.expression
    elif isinstance(condition, bool):
        if condition:
            return true_expr
        else:
            return false_expr
    if isinstance(true_expr, TracingExpressionHandle):
        graph = true_expr.graph
        true_expr = true_expr.expression
    if isinstance(false_expr, TracingExpressionHandle):
        graph = false_expr.graph
        false_expr = false_expr.expression

    if graph is None:
        if condition:
            return true_expr
        else:
            return false_expr
    else:
        if isinstance(true_expr, (int, float)):
            true_expr = graph.add_constant(true_expr)
        if isinstance(false_expr, (int, float)):
            false_expr = graph.add_constant(false_expr)

    new_expression = graph.add_ternary(
        TernaryOperator.IfThenElse, condition, true_expr, false_expr
    )
    return TracingExpressionHandle(graph, new_expression)


class DynamicTable:
    def __init__(self):
        self._data = {}

    def __getattr__(self, name):
        if name not in self._data:
            self._data[name] = self._create()
        return self._data[name]

    def __len__(self):
        return len(self._data)

    def _names(self):
        return list(self._data.keys())

    def _create(self):
        pass


class VariableTable(DynamicTable):
    def __init__(self, graph):
        super().__init__()
        self.graph = graph

    def _create(self):
        graph = self.graph
        n_data = len(self)
        variable = graph.add_variable(n_data)
        variable = TracingExpressionHandle(graph, variable)
        return variable


class ParameterTable(DynamicTable):
    def __init__(self, graph):
        super().__init__()
        self.graph = graph

    def _create(self):
        graph = self.graph
        n_data = len(self)
        parameter = graph.add_parameter(n_data)
        parameter = TracingExpressionHandle(graph, parameter)
        return parameter


class FunctionTracingResult:
    def __init__(self, graph, results, variable_names, parameter_names):
        self.graph = graph
        self.results = results
        self.variable_names = variable_names
        self.parameter_names = parameter_names

        # record the inverse mapping from name to index
        self.variable_indices_map = {name: i for i, name in enumerate(variable_names)}
        self.parameter_indices_map = {name: i for i, name in enumerate(parameter_names)}

    def n_variables(self):
        return len(self.variable_names)

    def n_parameters(self):
        return len(self.parameter_names)

    def n_outputs(self):
        return len(self.results)


def trace_function(f):
    graph = ExpressionGraph()
    vars = VariableTable(graph)
    params = ParameterTable(graph)

    # detect the number of arguments f receives
    # 1: f(vars)
    # 2: f(vars, params)
    # >2: error
    n_args = len(inspect.signature(f).parameters)
    if n_args == 1:
        results = f(vars)
    elif n_args == 2:
        results = f(vars, params)
    else:
        raise ValueError("Function to trace must receive either 1 or 2 arguments")

    # The results might be a scalar expressions or a list of scalar expressions
    if isinstance(results, TracingExpressionHandle):
        results = [results]

    results = [result.expression for result in results]

    return FunctionTracingResult(graph, results, vars._names(), params._names())
