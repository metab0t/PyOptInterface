from .nlcore_ext import (
    cpp_graph,
    graph_op,
)

from typing import IO

op2name = {
    graph_op.add: "+",
    graph_op.sub: "-",
    graph_op.mul: "*",
    graph_op.div: "/",
    graph_op.sqrt: "sqrt",
    graph_op.sin: "sin",
    graph_op.cos: "cos",
    graph_op.exp: "exp",
    graph_op.log: "log",
    graph_op.azmul: "*",
    graph_op.neg: "-",
}


def generate_csrc_prelude(io: IO[str]):
    io.write(
        """// includes
# include <stddef.h>
# include <math.h>

// typedefs
typedef double float_point_t;

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


def generate_csrc_prelude_declaration(io: IO[str]):
    io.write(
        """// includes
# include <stddef.h>
# include <math.h>

// typedefs
typedef double float_point_t;

// externals
// azmul
extern float_point_t azmul(float_point_t x, float_point_t y);
// sign
extern float_point_t sign(float_point_t x);
"""
    )


def generate_csrc_from_graph(
    io: IO[str],
    graph_obj: cpp_graph,
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
    for graph_iter in graph_obj:
        n_node += graph_iter.n_result

    has_parameter = np > 0

    function_args_signature = ["const float_point_t* x"]
    if has_parameter:
        function_args_signature.append("const float_point_t* p")
    if hessian_lagrange:
        function_args_signature.append("const float_point_t* w")
    function_args_signature.append("float_point_t* y")
    if indirect_x:
        function_args_signature.append("const size_t* xi")
    if has_parameter and indirect_p:
        function_args_signature.append("const size_t* pi")
    if hessian_lagrange and indirect_w:
        function_args_signature.append("const size_t* wi")
    if indirect_y:
        function_args_signature.append("const size_t* yi")

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

    for iter in graph_obj:
        op_enum = iter.op_enum
        n_result = iter.n_result
        arg_node = iter.arg_node

        assert n_result == 1
        op_name = op2name.get(op_enum, None)
        if op_name is None:
            raise ValueError(f"Unknown op_enum: {op_enum}")

        n_arg = len(arg_node)
        if n_arg == 1:
            arg1 = get_node_name(arg_node[0])
            io.write(f"    v[{result_node}] = {op_name}({arg1});\n")
        elif n_arg == 2:
            arg1 = get_node_name(arg_node[0])
            arg2 = get_node_name(arg_node[1])
            if op_name in infix_operators:
                io.write(f"    v[{result_node}] = {arg1} {op_name} {arg2};\n")
            else:
                io.write(f"    v[{result_node}] = {op_name}({arg1}, {arg2});\n")
        else:
            raise ValueError(f"Unknown n_arg: {n_arg} for op_enum: {op_enum}")

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

    extern_function_declaration = "extern " + function_prototype

    return extern_function_declaration
