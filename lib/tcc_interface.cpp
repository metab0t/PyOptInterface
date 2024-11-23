#include "pyoptinterface/tcc_interface.hpp"
#include <stdexcept>
#include <math.h>
#include "fmt/core.h"

namespace tcc
{
#define B DYLIB_DECLARE
APILIST
#undef B

static DynamicLibrary lib;
static bool is_loaded = false;

bool is_library_loaded()
{
	return is_loaded;
}

bool load_library(const std::string &path)
{
	bool success = lib.try_load(path.c_str());
	if (!success)
	{
		return false;
	}

	DYLIB_LOAD_INIT;

#define B DYLIB_LOAD_FUNCTION
	APILIST
#undef B

	if (IS_DYLIB_LOAD_SUCCESS)
	{
#define B DYLIB_SAVE_FUNCTION
		APILIST
#undef B
		is_loaded = true;
		return true;
	}
	else
	{
		return false;
	}
}
} // namespace tcc

void TCCInstance::init()
{
	TCCState *state = tcc::tcc_new();

	if (state == nullptr)
	{
		throw std::runtime_error("Failed to create TCC state");
	}

	m_state.reset(state);

	int ret = tcc::tcc_set_output_type(m_state.get(), TCC_OUTPUT_MEMORY);
	if (ret == -1)
	{
		throw std::runtime_error("Failed to set output type");
	}
}

void TCCInstance::add_include_path(const std::string &path)
{
	int ret = tcc::tcc_add_include_path(m_state.get(), path.c_str());
	if (ret == -1)
	{
		throw std::runtime_error(fmt::format("Failed to add include path {}", path));
	}
}

void TCCInstance::add_sysinclude_path(const std::string &path)
{
	int ret = tcc::tcc_add_sysinclude_path(m_state.get(), path.c_str());
	if (ret == -1)
	{
		throw std::runtime_error(fmt::format("Failed to add sysinclude path {}", path));
	}
}

void TCCInstance::add_library_path(const std::string &path)
{
	int ret = tcc::tcc_add_library_path(m_state.get(), path.c_str());
	if (ret == -1)
	{
		throw std::runtime_error(fmt::format("Failed to add library path {}", path));
	}
}

void TCCInstance::add_library(const std::string &name)
{
	int ret = tcc::tcc_add_library(m_state.get(), name.c_str());
	if (ret == -1)
	{
		throw std::runtime_error(fmt::format("Failed to add library {}", name));
	}
}

void TCCInstance::import_math_symbols()
{
	int ret;

#define UNARY_MATH_SYMBOL(name)                                                         \
	{                                                                                   \
		auto ptr = static_cast<double (*)(double)>(name);                               \
		ret = tcc::tcc_add_symbol(m_state.get(), #name, reinterpret_cast<void *>(ptr)); \
		if (ret == -1)                                                                  \
		{                                                                               \
			throw std::runtime_error(fmt::format("Failed to add symbol {}", #name));    \
		}                                                                               \
	}

	UNARY_MATH_SYMBOL(sin);
	UNARY_MATH_SYMBOL(cos);
	UNARY_MATH_SYMBOL(tan);
	UNARY_MATH_SYMBOL(asin);
	UNARY_MATH_SYMBOL(acos);
	UNARY_MATH_SYMBOL(atan);
	UNARY_MATH_SYMBOL(fabs);
	UNARY_MATH_SYMBOL(sqrt);
	UNARY_MATH_SYMBOL(exp);
	UNARY_MATH_SYMBOL(log);
	UNARY_MATH_SYMBOL(log10);

#define BINARY_MATH_SYMBOL(name)                                                        \
	{                                                                                   \
		auto ptr = static_cast<double (*)(double, double)>(name);                       \
		ret = tcc::tcc_add_symbol(m_state.get(), #name, reinterpret_cast<void *>(ptr)); \
		if (ret == -1)                                                                  \
		{                                                                               \
			throw std::runtime_error(fmt::format("Failed to add symbol {}", #name));    \
		}                                                                               \
	}

	BINARY_MATH_SYMBOL(pow);

#undef UNARY_MATH_SYMBOL
#undef BINARY_MATH_SYMBOL
}

void TCCInstance::compile_string(const std::string &code)
{
	int ret = tcc::tcc_compile_string(m_state.get(), code.c_str());
	if (ret == -1)
	{
		throw std::runtime_error("Failed to compile code");
	}

	ret = tcc::tcc_relocate(m_state.get());
	if (ret == -1)
	{
		throw std::runtime_error("Failed to relocate code");
	}
}

uintptr_t TCCInstance::get_symbol(const std::string &name)
{
	void *ptr = tcc::tcc_get_symbol(m_state.get(), name.c_str());
	if (ptr == nullptr)
	{
		throw std::runtime_error(fmt::format("Failed to get symbol {}", name));
	}
	return reinterpret_cast<uintptr_t>(ptr);
}
