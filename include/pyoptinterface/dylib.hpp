#pragma once

#if defined(_MSC_VER)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#else
#include <dlfcn.h>
#endif

class DynamicLibrary
{
  public:
	DynamicLibrary() : handle(nullptr)
	{
	}

	~DynamicLibrary()
	{
		if (handle != nullptr)
		{
#if defined(_MSC_VER)
			FreeLibrary(static_cast<HINSTANCE>(handle));
#else
			dlclose(handle);
#endif
		}
	}

	bool try_load(const char *library)
	{
#if defined(_MSC_VER)
		handle = static_cast<void *>(LoadLibraryA(library));

		if (handle == nullptr)
		{
			handle = static_cast<void *>(LoadLibraryExA(library, NULL,
			                                            LOAD_LIBRARY_SEARCH_DEFAULT_DIRS |
			                                                LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR));
		}

#else
		handle = dlopen(library, RTLD_NOW);
#endif
		return handle != nullptr;
	}

	bool LibraryIsLoaded() const
	{
		return handle != nullptr;
	}

	void *get_symbol(const char *name)
	{
#if defined(_MSC_VER)
		FARPROC function_address = GetProcAddress(static_cast<HINSTANCE>(handle), name);
#else
		void *function_address = dlsym(handle, name);
#endif

		return reinterpret_cast<void *>(function_address);
	}

  private:
	void *handle = nullptr;
};