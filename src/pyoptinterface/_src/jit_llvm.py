from llvmlite import ir, binding

from typing import List

# Initialize LLVM
binding.initialize()
binding.initialize_native_target()
binding.initialize_native_asmprinter()


class LLJITCompiler:
    def __init__(self):
        target = binding.Target.from_default_triple()
        target_machine = target.create_target_machine(jit=True, opt=3)
        self.lljit = binding.create_lljit_compiler(target_machine)

    def compile_module(self, module: ir.Module, export_functions: List[str] = []):
        ir_str = str(module)
        self.source_code = ir_str
        builder = binding.JITLibraryBuilder().add_ir(ir_str).add_current_process()
        for f in export_functions:
            builder.export_symbol(f)
        self.rt = builder.link(self.lljit, "lib")

    def get_symbol(self, symbol_name: str):
        return self.rt[symbol_name]
