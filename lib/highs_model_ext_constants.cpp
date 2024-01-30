#include <nanobind/nanobind.h>
#include "interfaces/highs_c_api.h"

namespace nb = nanobind;

void bind_highs_constants(nb::module_ &m)
{
	nb::module_ Enum = m.def_submodule("Enum");

#define BIND_C(x) Enum.attr(#x) = x

	BIND_C(kHighsOptionTypeBool);
	BIND_C(kHighsOptionTypeInt);
	BIND_C(kHighsOptionTypeDouble);
	BIND_C(kHighsOptionTypeString);

	BIND_C(kHighsInfoTypeInt64);
	BIND_C(kHighsInfoTypeInt);
	BIND_C(kHighsInfoTypeDouble);

	BIND_C(kHighsSolutionStatusNone);
	BIND_C(kHighsSolutionStatusInfeasible);
	BIND_C(kHighsSolutionStatusFeasible);

	BIND_C(kHighsModelStatusNotset);
	BIND_C(kHighsModelStatusLoadError);
	BIND_C(kHighsModelStatusModelError);
	BIND_C(kHighsModelStatusPresolveError);
	BIND_C(kHighsModelStatusSolveError);
	BIND_C(kHighsModelStatusPostsolveError);
	BIND_C(kHighsModelStatusModelEmpty);
	BIND_C(kHighsModelStatusOptimal);
	BIND_C(kHighsModelStatusInfeasible);
	BIND_C(kHighsModelStatusUnboundedOrInfeasible);
	BIND_C(kHighsModelStatusUnbounded);
	BIND_C(kHighsModelStatusObjectiveBound);
	BIND_C(kHighsModelStatusObjectiveTarget);
	BIND_C(kHighsModelStatusTimeLimit);
	BIND_C(kHighsModelStatusIterationLimit);
	BIND_C(kHighsModelStatusUnknown);
	BIND_C(kHighsModelStatusSolutionLimit);
	BIND_C(kHighsModelStatusInterrupt);
}