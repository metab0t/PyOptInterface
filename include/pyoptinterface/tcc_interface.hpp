#pragma once

#include <memory>
#include <string>

#include "tcc/libtcc.h"

#include "ankerl/unordered_dense.h"
#include "pyoptinterface/dylib.hpp"

#define APILIST                 \
	B(tcc_new);                 \
	B(tcc_delete);              \
	B(tcc_add_include_path);    \
	B(tcc_add_sysinclude_path); \
	B(tcc_define_symbol);       \
	B(tcc_undefine_symbol);     \
	B(tcc_compile_string);      \
	B(tcc_set_output_type);     \
	B(tcc_add_library_path);    \
	B(tcc_add_library);         \
	B(tcc_add_symbol);          \
	B(tcc_relocate);            \
	B(tcc_get_symbol);

namespace tcc
{
#define B DYLIB_EXTERN_DECLARE
APILIST
#undef B

bool is_library_loaded();

bool load_library(const std::string &path);
} // namespace tcc

struct TCCFreeT
{
	void operator()(TCCState *state) const
	{
		tcc::tcc_delete(state);
	};
};

struct TCCInstance
{
	TCCInstance() = default;
	void init();

	void add_include_path(const std::string &path);
	void add_sysinclude_path(const std::string &path);

	void add_library_path(const std::string &path);
	void add_library(const std::string &name);

	void import_math_symbols();
	void compile_string(const std::string &code);

	uintptr_t get_symbol(const std::string &name);

	std::unique_ptr<TCCState, TCCFreeT> m_state;
};