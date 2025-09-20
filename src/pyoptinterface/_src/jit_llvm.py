from llvmlite import ir, binding

from typing import List

# Initialize LLVM
try:
    # llvmlite older than 0.45.0 still requires this
    binding.initialize()
except Exception:
    pass

binding.initialize_native_target()
binding.initialize_native_asmprinter()


class LLJITCompiler:
    def __init__(self):
        target = binding.Target.from_default_triple()
        target_machine = target.create_target_machine(jit=True, opt=3)
        self.lljit = binding.create_lljit_compiler(target_machine)

        self.rts = []
        self.source_codes = []

    def compile_module(self, module: ir.Module, export_functions: List[str] = []):
        ir_str = str(module)
        self.source_codes.append(ir_str)
        builder = binding.JITLibraryBuilder().add_ir(ir_str).add_current_process()
        for f in export_functions:
            builder.export_symbol(f)
        n = len(self.rts)
        libname = f"lib{n}"
        rt = builder.link(self.lljit, libname)
        self.rts.append(rt)

        return rt
