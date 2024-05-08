from io import StringIO

from llvmlite import ir

from .ipopt_model_ext import RawModel
from .codegen_c import generate_csrc_prelude, generate_csrc_from_graph
from .jit_c import TCCJITCompiler
from .codegen_llvm import create_llvmir_basic_functions, generate_llvmir_from_graph
from .jit_llvm import LLJITCompiler
from .tracefun import trace_adfun


def compile_functions_c(backend: RawModel, jit_compiler: TCCJITCompiler):
    io = StringIO()

    generate_csrc_prelude(io)

    functions = backend.m_function_model.nl_functions

    for function in functions:
        name = function.name

        f_name = name
        generate_csrc_from_graph(
            io,
            function.f_graph,
            f_name,
            np=function.np,
            indirect_x=True,
            indirect_p=True,
        )
        jacobian_name = name + "_jacobian"
        generate_csrc_from_graph(
            io,
            function.jacobian_graph,
            jacobian_name,
            np=function.np,
            indirect_x=True,
            indirect_p=True,
        )
        gradient_name = name + "_gradient"
        generate_csrc_from_graph(
            io,
            function.jacobian_graph,
            gradient_name,
            np=function.np,
            indirect_x=True,
            indirect_p=True,
            indirect_y=True,
            add_y=True,
        )
        hessian_name = name + "_hessian"
        generate_csrc_from_graph(
            io,
            function.hessian_graph,
            hessian_name,
            np=function.np,
            hessian_lagrange=True,
            nw=function.ny,
            indirect_x=True,
            indirect_p=True,
            indirect_y=True,
            add_y=True,
        )

    csrc = io.getvalue()

    jit_compiler.compile_string(csrc.encode())

    for function in functions:
        name = function.name
        f_name = name
        jacobian_name = name + "_jacobian"
        gradient_name = name + "_gradient"
        hessian_name = name + "_hessian"

        f_ptr = jit_compiler.get_symbol(f_name.encode())
        jacobian_ptr = jit_compiler.get_symbol(jacobian_name.encode())
        gradient_ptr = jit_compiler.get_symbol(gradient_name.encode())
        hessian_ptr = jit_compiler.get_symbol(hessian_name.encode())

        function.assign_evaluators(f_ptr, jacobian_ptr, gradient_ptr, hessian_ptr)

def compile_functions_llvm(backend: RawModel, jit_compiler: LLJITCompiler):
    module = ir.Module(name="my_module")
    create_llvmir_basic_functions(module)

    functions = backend.m_function_model.nl_functions

    export_functions = []

    for function in functions:
        name = function.name

        f_name = name
        generate_llvmir_from_graph(
            module,
            function.f_graph,
            f_name,
            np=function.np,
            indirect_x=True,
            indirect_p=True,
        )
        jacobian_name = name + "_jacobian"
        generate_llvmir_from_graph(
            module,
            function.jacobian_graph,
            jacobian_name,
            np=function.np,
            indirect_x=True,
            indirect_p=True,
        )
        gradient_name = name + "_gradient"
        generate_llvmir_from_graph(
            module,
            function.jacobian_graph,
            gradient_name,
            np=function.np,
            indirect_x=True,
            indirect_p=True,
            indirect_y=True,
            add_y=True,
        )
        hessian_name = name + "_hessian"
        generate_llvmir_from_graph(
            module,
            function.hessian_graph,
            hessian_name,
            np=function.np,
            hessian_lagrange=True,
            nw=function.ny,
            indirect_x=True,
            indirect_p=True,
            indirect_y=True,
            add_y=True,
        )

        export_functions.extend([f_name, jacobian_name, gradient_name, hessian_name])

    jit_compiler.compile_module(module, export_functions)

    for function in functions:
        name = function.name
        f_name = name
        jacobian_name = name + "_jacobian"
        gradient_name = name + "_gradient"
        hessian_name = name + "_hessian"

        f_ptr = jit_compiler.get_symbol(f_name)
        jacobian_ptr = jit_compiler.get_symbol(jacobian_name)
        gradient_ptr = jit_compiler.get_symbol(gradient_name)
        hessian_ptr = jit_compiler.get_symbol(hessian_name)

        function.assign_evaluators(f_ptr, jacobian_ptr, gradient_ptr, hessian_ptr)

class Model(RawModel):
    def __init__(self):
        self.jit_compiler = None
        super().__init__()

    def optimize(self, jit_engine = "C"):
        if jit_engine == "C":
            self.jit_compiler = TCCJITCompiler()
            compile_functions_c(self, self.jit_compiler)
        elif jit_engine == "LLVM":
            self.jit_compiler = LLJITCompiler()
            compile_functions_llvm(self, self.jit_compiler)

        super().optimize()

    def register_function(self, f, /, x, name, p = ()):
        adfun = trace_adfun(f, x, p)
        return super().register_function(adfun, name)
