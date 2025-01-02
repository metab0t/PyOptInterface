#ifndef _MINDOPTCPP_DEF_
#define _MINDOPTCPP_DEF_

#define ERROR_INVALID_ARGUMENT                  -3
#define ERROR_INVALID_COL_INDEX                 -2003
#define ERROR_INVALID_ROW_INDEX                 -2002
#define ERROR_ASSOCIATED_MODEL_RELEASED         -100000
#define ERROR_MSG_ASSOCIATED_MODEL_RELEASED     "Environment associated model has been released."
#define ERROR_NOT_IMPLEMENTED                   -100002
#define ERROR_MSG_NOT_IMPLEMENTED               "Not implemented yet."


enum MDO_IntParam
{
    MDO_IntParam_Method,
    MDO_IntParam_PostScaling,
    MDO_IntParam_Presolve,
    MDO_IntParam_Dualization,
    MDO_IntParam_OutputFlag,
    MDO_IntParam_LogToConsole,
    MDO_IntParam_NumThreads,
    MDO_IntParam_EnableNetworkFlow,
    MDO_IntParam_EnableStochasticLP,
    MDO_IntParam_NumericFocus,
    MDO_IntParam_SpxColumnGeneration,
    MDO_IntParam_SpxCrashStart,
    MDO_IntParam_SpxMaxIterations,
    MDO_IntParam_SpxPrimalPricing,
    MDO_IntParam_SpxDualPricing,
    MDO_IntParam_IpmMaxIterations,
    MDO_IntParam_MipLevelCuts,
    MDO_IntParam_MipLevelHeurs,
    MDO_IntParam_MipLevelProbing,
    MDO_IntParam_MipLevelStrong,
    MDO_IntParam_MipLevelSubmip,
    MDO_IntParam_MipMaxNodes,
    MDO_IntParam_MipRootParallelism,
    MDO_IntParam_MipAutoConfiguration,
    MDO_IntParam_MipMaxSols,
    MDO_IntParam_MipAllowDualPresolve,
    MDO_IntParam_MipEnableLazyConstr,
    MDO_IntParam_MipDetectDisconnectedComponents,
    MDO_IntParam_MipSolutionPoolSize,
    MDO_IntParam_MipMaxStallingNodes,
    MDO_IntParam_MipSolutionNumber,
    MDO_IntParam_SolutionTarget
};

enum MDO_DoubleParam
{
    MDO_DoubleParam_MaxTime,
    MDO_DoubleParam_SpxPrimalTolerance,
    MDO_DoubleParam_SpxDualTolerance,
    MDO_DoubleParam_IpmPrimalTolerance,
    MDO_DoubleParam_IpmDualTolerance,
    MDO_DoubleParam_IpmGapTolerance,
    MDO_DoubleParam_MipIntegerTolerance,
    MDO_DoubleParam_MipObjectiveTolerance,
    MDO_DoubleParam_MipGapAbs,
    MDO_DoubleParam_MipGapRel,
    MDO_DoubleParam_MipLinearizationBigM,
    MDO_DoubleParam_MipCutoff
};

enum MDO_StringParam
{
    MDO_StringParam_LogFile
};

enum MDO_IntAttr
{
    MDO_IntAttr_MinSense,
    MDO_IntAttr_ModelSense,
    MDO_IntAttr_HasSolution,
    MDO_IntAttr_HasPrimalRay,
    MDO_IntAttr_HasDualRay,
    MDO_IntAttr_NumVars,
    MDO_IntAttr_NumConss,
    MDO_IntAttr_NumConstrs,
    MDO_IntAttr_NumQConstrs,
    MDO_IntAttr_NumSOS,
    MDO_IntAttr_NumGenConstrs,
    MDO_IntAttr_NumEnts,
    MDO_IntAttr_NumNZs,
    MDO_IntAttr_SpxNumIters,
    MDO_IntAttr_IpmNumIters,
    MDO_IntAttr_IsInteger,
    MDO_IntAttr_IISVar,
    MDO_IntAttr_IISConstr,
    MDO_IntAttr_IISCol,
    MDO_IntAttr_IISRow,
    MDO_IntAttr_ColBasis,
    MDO_IntAttr_RowBasis,
    MDO_IntAttr_GenConstrType,
    MDO_IntAttr_Status,
    MDO_IntAttr_Dim,
    MDO_IntAttr_NumPsdVars,
    MDO_IntAttr_NumPsdConstrs,
    MDO_IntAttr_SolCount
};

enum MDO_CharAttr
{
    MDO_CharAttr_VType
};

enum MDO_DoubleAttr
{
    MDO_DoubleAttr_ObjConst,
    MDO_DoubleAttr_ObjCon,
    MDO_DoubleAttr_PrimalObjVal,
    MDO_DoubleAttr_ObjVal,
    MDO_DoubleAttr_DualObjVal,
    MDO_DoubleAttr_PresolverTime,
    MDO_DoubleAttr_SolverTime,
    MDO_DoubleAttr_SolutionTime,
    MDO_DoubleAttr_MipGapAbs,
    MDO_DoubleAttr_MipGapRel,
    MDO_DoubleAttr_LB,
    MDO_DoubleAttr_UB,
    MDO_DoubleAttr_Obj,
    MDO_DoubleAttr_LHS,
    MDO_DoubleAttr_RHS,
    MDO_DoubleAttr_QCLHS,
    MDO_DoubleAttr_QCRHS,
    MDO_DoubleAttr_PrimalSoln,
    MDO_DoubleAttr_X,
    MDO_DoubleAttr_Xn,
    MDO_DoubleAttr_Start,
    MDO_DoubleAttr_DualSoln,
    MDO_DoubleAttr_Activity,
    MDO_DoubleAttr_ReducedCost,
    MDO_DoubleAttr_RC,
    MDO_DoubleAttr_PsdCLHS,
    MDO_DoubleAttr_PsdCRHS
};

enum MDO_StringAttr
{
    MDO_StringAttr_ProbName,
    MDO_StringAttr_ModelName,
    MDO_StringAttr_RowName,
    MDO_StringAttr_ConstrName,
    MDO_StringAttr_QCName,
    MDO_StringAttr_ColName,
    MDO_StringAttr_VarName,
    MDO_StringAttr_PsdVarName,
    MDO_StringAttr_PsdCName,
    MDO_StringAttr_GenConstrName
};

enum MDO_MatAttr
{
    MDO_MatAttr_PsdX,
    MDO_MatAttr_PsdObj
};

#endif
