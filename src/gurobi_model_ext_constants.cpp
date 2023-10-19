#include <nanobind/nanobind.h>

#include "gurobi_c.h"

namespace nb = nanobind;

void bind_gurobi_constants(nb::module_ &m)
{
	nb::module_ GRB = m.def_submodule("GRB");

	GRB.attr("LOADED") = GRB_LOADED;
	GRB.attr("OPTIMAL") = GRB_OPTIMAL;
	GRB.attr("INFEASIBLE") = GRB_INFEASIBLE;
	GRB.attr("INF_OR_UNBD") = GRB_INF_OR_UNBD;
	GRB.attr("UNBOUNDED") = GRB_UNBOUNDED;
	GRB.attr("CUTOFF") = GRB_CUTOFF;
	GRB.attr("ITERATION_LIMIT") = GRB_ITERATION_LIMIT;
	GRB.attr("NODE_LIMIT") = GRB_NODE_LIMIT;
	GRB.attr("TIME_LIMIT") = GRB_TIME_LIMIT;
	GRB.attr("SOLUTION_LIMIT") = GRB_SOLUTION_LIMIT;
	GRB.attr("INTERRUPTED") = GRB_INTERRUPTED;
	GRB.attr("NUMERIC") = GRB_NUMERIC;
	GRB.attr("SUBOPTIMAL") = GRB_SUBOPTIMAL;
	GRB.attr("INPROGRESS") = GRB_INPROGRESS;
	GRB.attr("USER_OBJ_LIMIT") = GRB_USER_OBJ_LIMIT;
	GRB.attr("WORK_LIMIT") = GRB_WORK_LIMIT;
	GRB.attr("MEM_LIMIT") = GRB_MEM_LIMIT;

	GRB.attr("CONTINUOUS") = GRB_CONTINUOUS;
	GRB.attr("BINARY") = GRB_BINARY;
	GRB.attr("INTEGER") = GRB_INTEGER;
	GRB.attr("SEMICONT") = GRB_SEMICONT;
}