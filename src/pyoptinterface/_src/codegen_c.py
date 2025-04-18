from .cppad_interface_ext import (
    graph_op,
)
from .cpp_graph_iter import cpp_graph_iterator

from typing import IO

op2name = {
    graph_op.abs: "fabs",
    graph_op.acos: "acos",
    graph_op.asin: "asin",
    graph_op.atan: "atan",
    graph_op.cos: "cos",
    graph_op.exp: "exp",
    graph_op.log: "log",
    graph_op.pow: "pow",
    graph_op.sign: "sign",
    graph_op.sin: "sin",
    graph_op.sqrt: "sqrt",
    graph_op.tan: "tan",
    graph_op.add: "+",
    graph_op.sub: "-",
    graph_op.mul: "*",
    graph_op.div: "/",
    graph_op.azmul: "*",
    graph_op.neg: "-",
}

compare_ops = set([graph_op.cexp_eq, graph_op.cexp_le, graph_op.cexp_lt])

compare_ops_string = {
    graph_op.cexp_eq: "==",
    graph_op.cexp_le: "<=",
    graph_op.cexp_lt: "<",
}


def generate_csrc_prelude(io: IO[str]):
    io.write(
        """// includes
#include <stddef.h>

// typedefs
typedef double float_point_t;

// declare mathematical functions
#define UNARY(f) extern float_point_t f(float_point_t x)
#define BINARY(f) extern float_point_t f(float_point_t x, float_point_t y)

// unary functions
UNARY(fabs);
UNARY(acos);
UNARY(asin);
UNARY(atan);
UNARY(cos);
UNARY(exp);
UNARY(log);
UNARY(sin);
UNARY(sqrt);
UNARY(tan);

// binary functions
BINARY(pow);

// externals
// azmul
float_point_t azmul(float_point_t x, float_point_t y)
{
    if( x == 0.0 ) return 0.0;
    return x * y;
}

// sign
float_point_t sign(float_point_t x)
{	
    if( x > 0.0 ) return 1.0;
    if( x == 0.0 ) return 0.0;
    return -1.0;
}
"""
    )


def generate_csrc_from_graph(
    io: IO[str],
    graph_obj,
    name: str,
    np: int = 0,
    hessian_lagrange: bool = False,
    nw: int = 0,
    indirect_x: bool = False,
    indirect_p: bool = False,
    indirect_w: bool = False,
    indirect_y: bool = False,
    add_y: bool = False,
):
    n_dynamic_ind = graph_obj.n_dynamic_ind
    n_variable_ind = graph_obj.n_variable_ind
    n_constant = graph_obj.n_constant
    n_dependent = graph_obj.n_dependent

    # Simple case
    # 0 -> dummy
    # [1, 1 + np) -> *p
    # [1 + np, 1 + n_dynamic_ind + n_variable_ind) -> *x
    # [1 + n_dynamic_ind + n_variable_ind, 1 + n_dynamic_ind + n_variable_ind + n_constant) -> c[...]
    # [1 + n_dynamic_ind + n_variable_ind + n_constant, ...) -> v[...]

    # Hessian lagragian case
    # 0 -> dummy
    # [1, 1 + np) -> *p
    # [1 + np, 1 + np + nw) -> *w
    # [1 + np + nw, 1 + n_dynamic_ind + n_variable_ind) -> *x
    # [1 + n_dynamic_ind + n_variable_ind, 1 + n_dynamic_ind + n_variable_ind + n_constant) -> c[...]
    # [1 + n_dynamic_ind + n_variable_ind + n_constant, ...) -> v[...]

    n_node = 0
    for graph_iter in cpp_graph_iterator(graph_obj):
        n_node += graph_iter.n_result

    has_parameter = np > 0

    function_args_signature = ["const float_point_t* x"]
    if has_parameter:
        function_args_signature.append("const float_point_t* p")
    if hessian_lagrange:
        function_args_signature.append("const float_point_t* w")
    function_args_signature.append("float_point_t* y")
    if indirect_x:
        function_args_signature.append("const int* xi")
    if has_parameter and indirect_p:
        function_args_signature.append("const int* pi")
    if hessian_lagrange and indirect_w:
        function_args_signature.append("const int* wi")
    if indirect_y:
        function_args_signature.append("const int* yi")

    function_args = ", ".join(function_args_signature)

    function_prototype = f"""
void {name}(
    {function_args}
)
"""
    io.write(function_prototype)

    if not hessian_lagrange:
        nx = n_dynamic_ind + n_variable_ind - np
    else:
        nx = n_dynamic_ind + n_variable_ind - np - nw
    ny = n_dependent
    io.write(
        f"""{{
    // begin function body

    // size checks
    // const size_t nx = {nx};
    // const size_t np = {np};
    // const size_t ny = {ny};
"""
    )
    if hessian_lagrange:
        io.write(f"    // const size_t nw = {nw};\n")

    io.write(
        f"""
    // declare variables
    float_point_t v[{n_node}];
    """
    )

    nc = n_constant
    if nc > 0:
        cs = (graph_obj.constant_vec_get(i) for i in range(nc))
        cs_str = ", ".join(f"{c}" for c in cs)
        io.write(
            f"""
    // constants
    // set c[i] for i = 0, ..., nc-1
    // nc = {nc}
    static const float_point_t c[{nc}] = {{
        {cs_str}
    }};
"""
        )

    n_result_node = n_node
    io.write(
        f"""
    // result nodes
    // set v[i] for i = 0, ..., n_result_node-1
    // n_result_node = {n_result_node}
"""
    )

    def get_node_name(node: int) -> str:
        if node < 1:
            raise ValueError(f"Invalid node: {node}")
        if node < 1 + np:
            index = node - 1
            if indirect_p:
                return f"p[pi[{index}]]"
            else:
                return f"p[{index}]"
        elif node < 1 + n_dynamic_ind + n_variable_ind:
            if hessian_lagrange:
                if node < 1 + np + nw:
                    index = node - 1 - np
                    if indirect_w:
                        return f"w[wi[{index}]]"
                    else:
                        return f"w[{index}]"
                else:
                    index = node - 1 - np - nw
                    if indirect_x:
                        return f"x[xi[{index}]]"
                    else:
                        return f"x[{index}]"
            else:
                index = node - 1 - np
                if indirect_x:
                    return f"x[xi[{index}]]"
                else:
                    return f"x[{index}]"
        elif node < 1 + n_dynamic_ind + n_variable_ind + n_constant:
            index = node - 1 - n_dynamic_ind - n_variable_ind
            return f"c[{index}]"
        else:
            node = node - 1 - n_dynamic_ind - n_variable_ind - n_constant
            assert node < n_node
            return f"v[{node}]"

    result_node = 0

    infix_operators = set(["+", "-", "*", "/"])

    for iter in cpp_graph_iterator(graph_obj):
        op = iter.op
        n_result = iter.n_result
        args = iter.args

        assert n_result == 1

        n_arg = len(args)

        op_name = op2name.get(op, None)
        if op_name is not None:
            if n_arg == 1:
                arg1 = get_node_name(args[0])
                io.write(f"    v[{result_node}] = {op_name}({arg1});\n")
            elif n_arg == 2:
                arg1 = get_node_name(args[0])
                arg2 = get_node_name(args[1])
                if op_name in infix_operators:
                    io.write(f"    v[{result_node}] = {arg1} {op_name} {arg2};\n")
                else:
                    io.write(f"    v[{result_node}] = {op_name}({arg1}, {arg2});\n")
            else:
                message = f"Unknown n_arg: {n_arg} for op_enum: {op}\n"
                raise ValueError(message)
        elif op in compare_ops:
            cmp_op = compare_ops_string[op]
            assert n_arg == 4
            predicate = get_node_name(args[0])
            target = get_node_name(args[1])
            then_value = get_node_name(args[2])
            else_value = get_node_name(args[3])
            io.write(
                f"    v[{result_node}] = {predicate} {cmp_op} {target} ? {then_value} : {else_value};\n"
            )
        else:
            message = f"Unknown name for op_enum: {op}\n"
            debug_context = f"name: {name}\nfull graph:\n{str(graph_obj)}"
            raise ValueError(message + debug_context)

        result_node += n_result

    io.write(
        """
    // dependent variables
    // set y[i] for i = 0, ny-1
"""
    )

    op = "="
    if add_y:
        op = "+="
    for i in range(ny):
        node = graph_obj.dependent_vec_get(i)
        node_name = get_node_name(node)
        if indirect_y:
            assignment = f"    y[yi[{i}]] {op} {node_name};\n"
        else:
            assignment = f"    y[{i}] {op} {node_name};\n"
        io.write(assignment)

    io.write(
        """
    // end function body
}
"""
    )
