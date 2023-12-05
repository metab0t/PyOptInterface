#include <nanobind/nanobind.h>
#include "gurobi_c.h"

namespace nb = nanobind;

void bind_gurobi_constants(nb::module_ &m)
{
	nb::module_ GRB = m.def_submodule("GRB");

	// Status codes
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

	// Batch status codes
	GRB.attr("BATCH_CREATED") = GRB_BATCH_CREATED;
	GRB.attr("BATCH_SUBMITTED") = GRB_BATCH_SUBMITTED;
	GRB.attr("BATCH_ABORTED") = GRB_BATCH_ABORTED;
	GRB.attr("BATCH_FAILED") = GRB_BATCH_FAILED;
	GRB.attr("BATCH_COMPLETED") = GRB_BATCH_COMPLETED;

	// Constraint senses
	GRB.attr("LESS_EQUAL") = GRB_LESS_EQUAL;
	GRB.attr("GREATER_EQUAL") = GRB_GREATER_EQUAL;
	GRB.attr("EQUAL") = GRB_EQUAL;

	// Variable types
	GRB.attr("CONTINUOUS") = GRB_CONTINUOUS;
	GRB.attr("BINARY") = GRB_BINARY;
	GRB.attr("INTEGER") = GRB_INTEGER;
	GRB.attr("SEMICONT") = GRB_SEMICONT;
	GRB.attr("SEMIINT") = GRB_SEMIINT;

	// Objective sense
	GRB.attr("MINIMIZE") = GRB_MINIMIZE;
	GRB.attr("MAXIMIZE") = GRB_MAXIMIZE;

	// SOS types
	GRB.attr("SOS_TYPE1") = GRB_SOS_TYPE1;
	GRB.attr("SOS_TYPE2") = GRB_SOS_TYPE2;

	// General constraint types
	GRB.attr("GENCONSTR_MAX") = GRB_GENCONSTR_MAX;
	// ... continue with all other general constraint types ...

	// Basis status
	GRB.attr("BASIC") = GRB_BASIC;
	GRB.attr("NONBASIC_LOWER") = GRB_NONBASIC_LOWER;
	GRB.attr("NONBASIC_UPPER") = GRB_NONBASIC_UPPER;
	GRB.attr("SUPERBASIC") = GRB_SUPERBASIC;

	// Numeric constants
	GRB.attr("INFINITY") = GRB_INFINITY;
	GRB.attr("UNDEFINED") = GRB_UNDEFINED;
	GRB.attr("MAXINT") = GRB_MAXINT;

	// Limits
	GRB.attr("MAX_NAMELEN") = GRB_MAX_NAMELEN;
	GRB.attr("MAX_STRLEN") = GRB_MAX_STRLEN;
	GRB.attr("MAX_TAGLEN") = GRB_MAX_TAGLEN;
	GRB.attr("MAX_CONCURRENT") = GRB_MAX_CONCURRENT;
}
