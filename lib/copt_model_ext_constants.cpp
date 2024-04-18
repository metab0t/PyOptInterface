#include <nanobind/nanobind.h>
#include "solvers/copt/copt.h"

namespace nb = nanobind;

void bind_copt_constants(nb::module_ &m)
{
	nb::module_ COPT = m.def_submodule("COPT");

	COPT.attr("VERSION_MAJOR") = COPT_VERSION_MAJOR;
	COPT.attr("VERSION_MINOR") = COPT_VERSION_MINOR;
	COPT.attr("VERSION_TECHNICAL") = COPT_VERSION_TECHNICAL;

	COPT.attr("MINIMIZE") = COPT_MINIMIZE;
	COPT.attr("MAXIMIZE") = COPT_MAXIMIZE;

	COPT.attr("INFINITY") = COPT_INFINITY;
	COPT.attr("UNDEFINED") = COPT_UNDEFINED;

	COPT.attr("LESS_EQUAL") = COPT_LESS_EQUAL;
	COPT.attr("GREATER_EQUAL") = COPT_GREATER_EQUAL;
	COPT.attr("EQUAL") = COPT_EQUAL;
	COPT.attr("FREE") = COPT_FREE;
	COPT.attr("RANGE") = COPT_RANGE;

	COPT.attr("CONTINUOUS") = COPT_CONTINUOUS;
	COPT.attr("BINARY") = COPT_BINARY;
	COPT.attr("INTEGER") = COPT_INTEGER;

	COPT.attr("SOS_TYPE1") = COPT_SOS_TYPE1;
	COPT.attr("SOS_TYPE2") = COPT_SOS_TYPE2;

	COPT.attr("CONE_QUAD") = COPT_CONE_QUAD;
	COPT.attr("CONE_RQUAD") = COPT_CONE_RQUAD;

	COPT.attr("BASIS_LOWER") = COPT_BASIS_LOWER;
	COPT.attr("BASIS_BASIC") = COPT_BASIS_BASIC;
	COPT.attr("BASIS_UPPER") = COPT_BASIS_UPPER;
	COPT.attr("BASIS_SUPERBASIC") = COPT_BASIS_SUPERBASIC;
	COPT.attr("BASIS_FIXED") = COPT_BASIS_FIXED;

	COPT.attr("UNSTARTED") = COPT_MIPSTATUS_UNSTARTED;
	COPT.attr("OPTIMAL") = COPT_MIPSTATUS_OPTIMAL;
	COPT.attr("INFEASIBLE") = COPT_MIPSTATUS_INFEASIBLE;
	COPT.attr("UNBOUNDED") = COPT_MIPSTATUS_UNBOUNDED;
	COPT.attr("INF_OR_UNB") = COPT_MIPSTATUS_INF_OR_UNB;
	COPT.attr("NUMERICAL") = COPT_LPSTATUS_NUMERICAL;
	COPT.attr("NODELIMIT") = COPT_MIPSTATUS_NODELIMIT;
	COPT.attr("TIMEOUT") = COPT_MIPSTATUS_TIMEOUT;
	COPT.attr("UNFINISHED") = COPT_MIPSTATUS_UNFINISHED;
	COPT.attr("IMPRECISE") = COPT_LPSTATUS_IMPRECISE;
	COPT.attr("INTERRUPTED") = COPT_MIPSTATUS_INTERRUPTED;

	COPT.attr("CBCONTEXT_INCUMBENT") = COPT_CBCONTEXT_INCUMBENT;
	COPT.attr("CBCONTEXT_MIPRELAX") = COPT_CBCONTEXT_MIPRELAX;
	COPT.attr("CBCONTEXT_MIPSOL") = COPT_CBCONTEXT_MIPSOL;
	COPT.attr("CBCONTEXT_MIPNODE") = COPT_CBCONTEXT_MIPNODE;

	COPT.attr("CBINFO_BESTOBJ") = COPT_CBINFO_BESTOBJ;
	COPT.attr("CBINFO_BESTBND") = COPT_CBINFO_BESTBND;
	COPT.attr("CBINFO_HASINCUMBENT") = COPT_CBINFO_HASINCUMBENT;
	COPT.attr("CBINFO_INCUMBENT") = COPT_CBINFO_INCUMBENT;
	COPT.attr("CBINFO_MIPCANDIDATE") = COPT_CBINFO_MIPCANDIDATE;
	COPT.attr("CBINFO_MIPCANDOBJ") = COPT_CBINFO_MIPCANDOBJ;
	COPT.attr("CBINFO_RELAXSOLUTION") = COPT_CBINFO_RELAXSOLUTION;
	COPT.attr("CBINFO_RELAXSOLOBJ") = COPT_CBINFO_RELAXSOLOBJ;
	COPT.attr("CBINFO_NODESTATUS") = COPT_CBINFO_NODESTATUS;
}
