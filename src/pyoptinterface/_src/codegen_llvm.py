from .nlcore_ext import (
    cpp_graph,
    graph_op,
)
from llvmlite import ir

D = ir.DoubleType()
D_PTR = ir.PointerType(D)
SZ = ir.IntType(64)
SZ_PTR = ir.PointerType(SZ)
I = ir.IntType(32)


def create_azmul(module: ir.Module):
    # Define and create the azmul function
    azmul_func_type = ir.FunctionType(D, [D, D])
    azmul_func = ir.Function(module, azmul_func_type, name="azmul")
    azmul_func.attributes.add("alwaysinline")
    x, y = azmul_func.args
    x.name = "x"
    y.name = "y"

    block = azmul_func.append_basic_block(name="entry")
    builder = ir.IRBuilder(block)

    # Implement the azmul logic
    with builder.if_else(builder.fcmp_unordered("==", x, D(0.0))) as (
        then,
        otherwise,
    ):
        with then:
            builder.ret(D(0.0))
        with otherwise:
            result = builder.fmul(x, y)
            builder.ret(result)
    builder.unreachable()


def create_sign(module: ir.Module):
    # Define and create the sign function
    sign_func_type = ir.FunctionType(D, [D])
    sign_func = ir.Function(module, sign_func_type, name="sign")
    sign_func.attributes.add("alwaysinline")
    x = sign_func.args[0]
    x.name = "x"

    block = sign_func.append_basic_block(name="entry")
    builder = ir.IRBuilder(block)

    # Implement the sign logic
    with builder.if_else(builder.fcmp_ordered(">", x, D(0.0))) as (
        then_positive,
        otherwise,
    ):
        with then_positive:
            builder.ret(D(1.0))
        with otherwise:
            with builder.if_else(builder.fcmp_ordered("==", x, D(0.0))) as (
                then_zero,
                otherwise_negative,
            ):
                with then_zero:
                    builder.ret(D(0.0))
                with otherwise_negative:
                    builder.ret(D(-1.0))
    builder.unreachable()


def create_direct_load_store(module: ir.Module):
    # Define and create the load_direct function
    # D load_directly(D_PTR, SZ)
    # returns ptr[idx_ptr[idx]]
    load_direct_func_type = ir.FunctionType(D, [D_PTR, SZ])
    load_direct_func = ir.Function(module, load_direct_func_type, name="load_direct")
    load_direct_func.attributes.add("alwaysinline")
    ptr, idx = load_direct_func.args
    ptr.name = "ptr"
    idx.name = "idx"

    block = load_direct_func.append_basic_block(name="entry")
    builder = ir.IRBuilder(block)

    # Implement the load_direct logic
    ptr = builder.gep(ptr, [idx], name="gep_ptr")
    val = builder.load(ptr, name="val")
    builder.ret(val)

    # Define and create the store_direct function
    # void store_directly(D_PTR, SZ, D)
    # ptr[idx] = val
    store_direct_func_type = ir.FunctionType(ir.VoidType(), [D_PTR, SZ, D])
    store_direct_func = ir.Function(module, store_direct_func_type, name="store_direct")
    store_direct_func.attributes.add("alwaysinline")
    ptr, idx, val = store_direct_func.args
    ptr.name = "ptr"
    idx.name = "idx"
    val.name = "val"

    block = store_direct_func.append_basic_block(name="entry")
    builder = ir.IRBuilder(block)

    # Implement the store_direct logic
    ptr = builder.gep(ptr, [idx], name="gep_ptr")
    builder.store(val, ptr)
    builder.ret_void()

    # Define and create the add_store_direct function
    # void add_store_directly(D_PTR, SZ, D)
    # ptr[idx] += val
    add_store_direct_func_type = ir.FunctionType(ir.VoidType(), [D_PTR, SZ, D])
    add_store_direct_func = ir.Function(
        module, add_store_direct_func_type, name="add_store_direct"
    )
    add_store_direct_func.attributes.add("alwaysinline")
    ptr, idx, val = add_store_direct_func.args
    ptr.name = "ptr"
    idx.name = "idx"
    val.name = "val"

    block = add_store_direct_func.append_basic_block(name="entry")
    builder = ir.IRBuilder(block)

    # Implement the add_store_direct logic
    ptr = builder.gep(ptr, [idx], name="gep_ptr")
    old_val = builder.load(ptr, name="old_val")
    new_val = builder.fadd(old_val, val, name="new_val")
    builder.store(new_val, ptr)
    builder.ret_void()


def create_indirect_load_store(module: ir.Module):
    # Define and create the load_indirect function
    # D load_indirectly(D_PTR, SZ_PTR, SZ)
    # returns ptr[idx_ptr[idx]]
    load_indirect_func_type = ir.FunctionType(D, [D_PTR, SZ_PTR, SZ])
    load_indirect_func = ir.Function(
        module, load_indirect_func_type, name="load_indirect"
    )
    load_indirect_func.attributes.add("alwaysinline")
    ptr, idx_ptr, idx = load_indirect_func.args
    ptr.name = "ptr"
    idx_ptr.name = "idx_ptr"
    idx.name = "idx"

    block = load_indirect_func.append_basic_block(name="entry")
    builder = ir.IRBuilder(block)

    # Implement the load_indirect logic
    idx = builder.gep(idx_ptr, [idx], name="gep_idx")
    idx = builder.load(idx, name="real_idx")
    ptr = builder.gep(ptr, [idx], name="gep_ptr")
    val = builder.load(ptr, name="val")
    builder.ret(val)

    # Define and create the store_indirect function
    # void store_indirectly(D_PTR, SZ_PTR, SZ, D)
    # ptr[idx_ptr[idx]] = val
    store_indirect_func_type = ir.FunctionType(ir.VoidType(), [D_PTR, SZ_PTR, SZ, D])
    store_indirect_func = ir.Function(
        module, store_indirect_func_type, name="store_indirect"
    )
    store_indirect_func.attributes.add("alwaysinline")
    ptr, idx_ptr, idx, val = store_indirect_func.args
    ptr.name = "ptr"
    idx_ptr.name = "idx_ptr"
    idx.name = "idx"
    val.name = "val"

    block = store_indirect_func.append_basic_block(name="entry")
    builder = ir.IRBuilder(block)

    # Implement the store_indirect logic
    idx = builder.gep(idx_ptr, [idx], name="gep_idx")
    idx = builder.load(idx, name="real_idx")
    ptr = builder.gep(ptr, [idx], name="gep_ptr")
    builder.store(val, ptr)
    builder.ret_void()

    # Define and create the add_store_indirect function
    # void add_store_indirectly(D_PTR, SZ_PTR, SZ, D)
    # ptr[idx_ptr[idx]] += val
    add_store_indirect_func_type = ir.FunctionType(
        ir.VoidType(), [D_PTR, SZ_PTR, SZ, D]
    )
    add_store_indirect_func = ir.Function(
        module, add_store_indirect_func_type, name="add_store_indirect"
    )
    add_store_indirect_func.attributes.add("alwaysinline")
    ptr, idx_ptr, idx, val = add_store_indirect_func.args
    ptr.name = "ptr"
    idx_ptr.name = "idx_ptr"
    idx.name = "idx"
    val.name = "val"

    block = add_store_indirect_func.append_basic_block(name="entry")
    builder = ir.IRBuilder(block)

    # Implement the add_store_indirect logic
    idx = builder.gep(idx_ptr, [idx], name="gep_idx")
    idx = builder.load(idx, name="real_idx")
    ptr = builder.gep(ptr, [idx], name="gep_ptr")
    old_val = builder.load(ptr, name="old_val")
    new_val = builder.fadd(old_val, val, name="new_val")
    builder.store(new_val, ptr)
    builder.ret_void()


def create_llvmir_basic_functions(module: ir.Module):
    create_azmul(module)
    create_sign(module)
    create_direct_load_store(module)
    create_indirect_load_store(module)

    sqrt = ir.Function(module, ir.FunctionType(D, [D]), name="sqrt")
    sin = ir.Function(module, ir.FunctionType(D, [D]), name="sin")
    cos = ir.Function(module, ir.FunctionType(D, [D]), name="cos")
    exp = ir.Function(module, ir.FunctionType(D, [D]), name="exp")
    log = ir.Function(module, ir.FunctionType(D, [D]), name="log")

    # sin = module.declare_intrinsic('llvm.sin', [D])
    # cos = module.declare_intrinsic('llvm.cos', [D])


# Define graph to LLVM IR translation function
def generate_llvmir_from_graph(
    module: ir.Module,
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

    n_node = 0
    for graph_iter in graph_obj:
        n_node += graph_iter.n_result

    has_parameter = np > 0

    # Define function signature
    func_args = [D_PTR]
    arg_names = ["x"]
    if has_parameter:
        func_args.append(D_PTR)
        arg_names.append("p")
    if hessian_lagrange:
        func_args.append(D_PTR)
        arg_names.append("w")
    func_args.append(D_PTR)
    arg_names.append("y")
    if indirect_x:
        func_args.append(SZ_PTR)
        arg_names.append("xi")
    if has_parameter and indirect_p:
        func_args.append(SZ_PTR)
        arg_names.append("pi")
    if hessian_lagrange and indirect_w:
        func_args.append(SZ_PTR)
        arg_names.append("wi")
    if indirect_y:
        func_args.append(SZ_PTR)
        arg_names.append("yi")

    func_type = ir.FunctionType(ir.VoidType(), func_args)
    func = ir.Function(module, func_type, name=name)

    # set argument names
    args_dict = {}
    args = func.args
    for i, arg in enumerate(args):
        arg.name = arg_names[i]
        args_dict[arg.name] = arg
    # Destructure the args
    x = args_dict.get("x", None)
    p = args_dict.get("p", None)
    w = args_dict.get("w", None)
    y = args_dict.get("y", None)
    xi = args_dict.get("xi", None)
    pi = args_dict.get("pi", None)
    wi = args_dict.get("wi", None)
    yi = args_dict.get("yi", None)
    # add noalias attribute
    for arg in [x, p, w, y, xi, pi, wi, yi]:
        if arg is not None:
            arg.add_attribute("noalias")

    # Define entry block
    block = func.append_basic_block(name="entry")
    builder = ir.IRBuilder(block)

    comment = ""
    if not hessian_lagrange:
        nx = n_dynamic_ind + n_variable_ind - np
        comment += f"nx = {nx}, np = {np}, "
    else:
        nx = n_dynamic_ind + n_variable_ind - np - nw
        comment += f"nx = {nx}, np = {np}, nw = {nw}, "
    ny = n_dependent
    comment += f"ny = {ny}"
    builder.comment(comment)

    # Constants and initial setup (if any)
    nc = n_constant
    constants = [graph_obj.constant_vec_get(i) for i in range(nc)]
    c_array = ir.Constant(ir.ArrayType(D, len(constants)), constants)
    c_global = ir.GlobalVariable(module, c_array.type, name=f"{name}_constants")
    c_global.linkage = "internal"
    c_global.initializer = c_array
    c_global_ptr = builder.bitcast(c_global, D_PTR)

    # sin = module.get_global("llvm.sin.f64")
    # cos = module.get_global("llvm.cos.f64")
    sqrt = module.get_global("sqrt")
    sin = module.get_global("sin")
    cos = module.get_global("cos")
    exp = module.get_global("exp")
    log = module.get_global("log")
    azmul = module.get_global("azmul")
    sign = module.get_global("sign")
    load_direct = module.get_global("load_direct")
    store_direct = module.get_global("store_direct")
    add_store_direct = module.get_global("add_store_direct")
    load_indirect = module.get_global("load_indirect")
    store_indirect = module.get_global("store_indirect")
    add_store_indirect = module.get_global("add_store_indirect")

    result_node = 0

    p_dict = {}
    w_dict = {}
    x_dict = {}
    c_dict = {}
    v_dict = {}

    def get_node_value(node: int):
        if node < 1:
            raise ValueError(f"Invalid node: {node}")
        if node < 1 + np:
            p_index = node - 1
            val = p_dict.get(p_index, None)
            if val is None:
                if indirect_p:
                    val = builder.call(load_indirect, [p, pi, SZ(p_index)])
                else:
                    val = builder.call(load_direct, [p, SZ(p_index)])
                val.name = f"p[{p_index}]"
                p_dict[p_index] = val
        elif node < 1 + n_dynamic_ind + n_variable_ind:
            if hessian_lagrange:
                if node < 1 + np + nw:
                    w_index = node - 1 - np
                    val = w_dict.get(w_index, None)
                    if val is None:
                        if indirect_w:
                            val = builder.call(load_indirect, [w, wi, SZ(w_index)])
                        else:
                            val = builder.call(load_direct, [w, SZ(w_index)])
                        val.name = f"w[{w_index}]"
                        w_dict[w_index] = val
                else:
                    x_index = node - 1 - np - nw
                    val = x_dict.get(x_index, None)
                    if val is None:
                        if indirect_x:
                            val = builder.call(load_indirect, [x, xi, SZ(x_index)])
                        else:
                            val = builder.call(load_direct, [x, SZ(x_index)])
                        val.name = f"x[{x_index}]"
                        x_dict[x_index] = val
            else:
                x_index = node - 1 - np
                val = x_dict.get(x_index, None)
                if val is None:
                    if indirect_x:
                        val = builder.call(load_indirect, [x, xi, SZ(x_index)])
                    else:
                        val = builder.call(load_direct, [x, SZ(x_index)])
                    val.name = f"x[{x_index}]"
                    x_dict[x_index] = val
        elif node < 1 + n_dynamic_ind + n_variable_ind + n_constant:
            c_index = node - 1 - n_dynamic_ind - n_variable_ind
            val = c_dict.get(c_index, None)
            if val is None:
                val = builder.call(load_direct, [c_global_ptr, SZ(c_index)])
                val.name = f"c[{c_index}]"
                c_dict[c_index] = val
        else:
            v_index = node - 1 - n_dynamic_ind - n_variable_ind - n_constant
            assert v_index < n_node
            v_val = v_dict.get(v_index, None)
            if v_val is None:
                raise ValueError(f"Node value not found: {v_index}")
            val = v_val
        return val

    for iter in graph_obj:
        op_enum = iter.op_enum
        n_result = iter.n_result
        arg_node = iter.arg_node

        assert n_result == 1
        assert len(arg_node) <= 2

        arg1 = get_node_value(arg_node[0])
        if len(arg_node) == 2:
            arg2 = get_node_value(arg_node[1])

        if op_enum == graph_op.add:
            ret_val = builder.fadd(arg1, arg2)
        elif op_enum == graph_op.sub:
            ret_val = builder.fsub(arg1, arg2)
        elif op_enum == graph_op.mul:
            ret_val = builder.fmul(arg1, arg2)
        elif op_enum == graph_op.div:
            ret_val = builder.fdiv(arg1, arg2)
        elif op_enum == graph_op.sqrt:
            ret_val = builder.call(sqrt, [arg1])
        elif op_enum == graph_op.sin:
            ret_val = builder.call(sin, [arg1])
        elif op_enum == graph_op.cos:
            ret_val = builder.call(cos, [arg1])
        elif op_enum == graph_op.exp:
            ret_val = builder.call(exp, [arg1])
        elif op_enum == graph_op.log:
            ret_val = builder.call(log, [arg1])
        elif op_enum == graph_op.azmul:
            ret_val = builder.fmul(arg1, arg2)
            # ret_val = builder.call(azmul, [arg1, arg2])
        elif op_enum == graph_op.neg:
            ret_val = builder.fneg(arg1)
        else:
            raise ValueError(f"Unknown op_enum: {op_enum}")

        ret_val.name = f"v[{result_node}]"
        v_dict[result_node] = ret_val

        result_node += n_result

    store_f_dict = {
        (True, True): add_store_indirect,
        (True, False): store_indirect,
        (False, True): add_store_direct,
        (False, False): store_direct,
    }
    store_f = store_f_dict[(indirect_y, add_y)]
    for i in range(ny):
        node = graph_obj.dependent_vec_get(i)
        val = get_node_value(node)
        builder.comment(f"write y[{i}]")
        if indirect_y:
            builder.call(store_f, [y, yi, SZ(i), val])
        else:
            builder.call(store_f, [y, SZ(i), val])

    # Return from the function
    builder.ret_void()
