#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>

#include "pyoptinterface/tcc_interface.hpp"

namespace nb = nanobind;

NB_MODULE(tcc_interface_ext, m)
{
	m.def("is_library_loaded", &tcc::is_library_loaded);
	m.def("load_library", &tcc::load_library);

	nb::class_<TCCInstance>(m, "TCCInstance")
	    .def(nb::init<>())
	    .def("init", &TCCInstance::init)
	    .def("add_include_path", &TCCInstance::add_include_path)
	    .def("add_sysinclude_path", &TCCInstance::add_sysinclude_path)
	    .def("add_library_path", &TCCInstance::add_library_path)
	    .def("add_library", &TCCInstance::add_library)
	    .def("import_math_symbols", &TCCInstance::import_math_symbols)
	    .def("compile_string", &TCCInstance::compile_string)
	    .def("get_symbol", &TCCInstance::get_symbol);
}