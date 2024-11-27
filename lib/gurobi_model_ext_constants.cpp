#include <nanobind/nanobind.h>
#include "solvers/gurobi/gurobi_c.h"

namespace nb = nanobind;

void bind_gurobi_constants(nb::module_ &m)
{
	nb::module_ GRB = m.def_submodule("GRB");
	GRB.attr("BARHOMOGENEOUS_AUTO") = -1;
	GRB.attr("BARHOMOGENEOUS_OFF") = 0;
	GRB.attr("BARHOMOGENEOUS_ON") = 1;
	GRB.attr("BARORDER_AMD") = 0;
	GRB.attr("BARORDER_AUTOMATIC") = -1;
	GRB.attr("BARORDER_NESTEDDISSECTION") = 1;
	GRB.attr("BASIC") = 0;
	GRB.attr("BATCH_ABORTED") = 3;
	GRB.attr("BATCH_COMPLETED") = 5;
	GRB.attr("BATCH_CREATED") = 1;
	GRB.attr("BATCH_FAILED") = 4;
	GRB.attr("BATCH_SUBMITTED") = 2;
	GRB.attr("BINARY") = "B";
	GRB.attr("CONTINUOUS") = "C";
	GRB.attr("CUTOFF") = 6;
	GRB.attr("CUTS_AGGRESSIVE") = 2;
	GRB.attr("CUTS_AUTO") = -1;
	GRB.attr("CUTS_CONSERVATIVE") = 1;
	GRB.attr("CUTS_OFF") = 0;
	GRB.attr("CUTS_VERYAGGRESSIVE") = 3;
	GRB.attr("DEFAULT_CS_PORT") = 61000;
	GRB.attr("EQUAL") = "=";
	GRB.attr("ERROR_CALLBACK") = 10011;
	GRB.attr("ERROR_CLOUD") = 10028;
	GRB.attr("ERROR_CSWORKER") = 10030;
	GRB.attr("ERROR_DATA_NOT_AVAILABLE") = 10005;
	GRB.attr("ERROR_DUPLICATES") = 10018;
	GRB.attr("ERROR_EXCEED_2B_NONZEROS") = 10025;
	GRB.attr("ERROR_FAILED_TO_CREATE_MODEL") = 20002;
	GRB.attr("ERROR_FILE_READ") = 10012;
	GRB.attr("ERROR_FILE_WRITE") = 10013;
	GRB.attr("ERROR_IIS_NOT_INFEASIBLE") = 10015;
	GRB.attr("ERROR_INDEX_OUT_OF_RANGE") = 10006;
	GRB.attr("ERROR_INTERNAL") = 20003;
	GRB.attr("ERROR_INVALID_ARGUMENT") = 10003;
	GRB.attr("ERROR_INVALID_PIECEWISE_OBJ") = 10026;
	GRB.attr("ERROR_JOB_REJECTED") = 10023;
	GRB.attr("ERROR_MODEL_MODIFICATION") = 10029;
	GRB.attr("ERROR_NETWORK") = 10022;
	GRB.attr("ERROR_NODEFILE") = 10019;
	GRB.attr("ERROR_NOT_FOR_MIP") = 10016;
	GRB.attr("ERROR_NOT_IN_MODEL") = 20001;
	GRB.attr("ERROR_NOT_SUPPORTED") = 10024;
	GRB.attr("ERROR_NO_LICENSE") = 10009;
	GRB.attr("ERROR_NULL_ARGUMENT") = 10002;
	GRB.attr("ERROR_NUMERIC") = 10014;
	GRB.attr("ERROR_OPTIMIZATION_IN_PROGRESS") = 10017;
	GRB.attr("ERROR_OUT_OF_MEMORY") = 10001;
	GRB.attr("ERROR_QCP_EQUALITY_CONSTRAINT") = 10021;
	GRB.attr("ERROR_Q_NOT_PSD") = 10020;
	GRB.attr("ERROR_SECURITY") = 10032;
	GRB.attr("ERROR_SIZE_LIMIT_EXCEEDED") = 10010;
	GRB.attr("ERROR_TUNE_MODEL_TYPES") = 10031;
	GRB.attr("ERROR_UNKNOWN_ATTRIBUTE") = 10004;
	GRB.attr("ERROR_UNKNOWN_PARAMETER") = 10007;
	GRB.attr("ERROR_UPDATEMODE_CHANGE") = 10027;
	GRB.attr("ERROR_VALUE_OUT_OF_RANGE") = 10008;
	GRB.attr("FEASRELAX_CARDINALITY") = 2;
	GRB.attr("FEASRELAX_LINEAR") = 0;
	GRB.attr("FEASRELAX_QUADRATIC") = 1;
	GRB.attr("GENCONSTR_ABS") = 2;
	GRB.attr("GENCONSTR_AND") = 3;
	GRB.attr("GENCONSTR_COS") = 16;
	GRB.attr("GENCONSTR_EXP") = 10;
	GRB.attr("GENCONSTR_EXPA") = 11;
	GRB.attr("GENCONSTR_INDICATOR") = 7;
	GRB.attr("GENCONSTR_LOG") = 12;
	GRB.attr("GENCONSTR_LOGA") = 13;
	GRB.attr("GENCONSTR_LOGISTIC") = 18;
	GRB.attr("GENCONSTR_MAX") = 0;
	GRB.attr("GENCONSTR_MIN") = 1;
	GRB.attr("GENCONSTR_NL") = 6;
	GRB.attr("GENCONSTR_NORM") = 5;
	GRB.attr("GENCONSTR_OR") = 4;
	GRB.attr("GENCONSTR_POLY") = 9;
	GRB.attr("GENCONSTR_POW") = 14;
	GRB.attr("GENCONSTR_PWL") = 8;
	GRB.attr("GENCONSTR_SIN") = 15;
	GRB.attr("GENCONSTR_TAN") = 17;
	GRB.attr("GREATER_EQUAL") = ">";
	GRB.attr("INFEASIBLE") = 3;
	GRB.attr("INFINITY") = 1e+100;
	GRB.attr("INF_OR_UNBD") = 4;
	GRB.attr("INPROGRESS") = 14;
	GRB.attr("INTEGER") = "I";
	GRB.attr("INTERRUPTED") = 11;
	GRB.attr("ITERATION_LIMIT") = 7;
	GRB.attr("LESS_EQUAL") = "<";
	GRB.attr("LOADED") = 1;
	GRB.attr("MAXIMIZE") = -1;
	GRB.attr("MAXINT") = 2000000000;
	GRB.attr("MAX_CONCURRENT") = 64;
	GRB.attr("MAX_NAMELEN") = 255;
	GRB.attr("MAX_STRLEN") = 512;
	GRB.attr("MAX_TAGLEN") = 10240;
	GRB.attr("MEM_LIMIT") = 17;
	GRB.attr("METHOD_AUTO") = -1;
	GRB.attr("METHOD_BARRIER") = 2;
	GRB.attr("METHOD_CONCURRENT") = 3;
	GRB.attr("METHOD_DETERMINISTIC_CONCURRENT") = 4;
	GRB.attr("METHOD_DETERMINISTIC_CONCURRENT_SIMPLEX") = 5;
	GRB.attr("METHOD_DUAL") = 1;
	GRB.attr("METHOD_NONE") = -1;
	GRB.attr("METHOD_PRIMAL") = 0;
	GRB.attr("MINIMIZE") = 1;
	GRB.attr("MIPFOCUS_BALANCED") = 0;
	GRB.attr("MIPFOCUS_BESTBOUND") = 3;
	GRB.attr("MIPFOCUS_FEASIBILITY") = 1;
	GRB.attr("MIPFOCUS_OPTIMALITY") = 2;
	GRB.attr("NODE_LIMIT") = 8;
	GRB.attr("NONBASIC_LOWER") = -1;
	GRB.attr("NONBASIC_UPPER") = -2;
	GRB.attr("NUMERIC") = 12;
	GRB.attr("OPCODE_CONSTANT") = 0;
	GRB.attr("OPCODE_COS") = 10;
	GRB.attr("OPCODE_DIVIDE") = 5;
	GRB.attr("OPCODE_EXP") = 13;
	GRB.attr("OPCODE_LOG") = 14;
	GRB.attr("OPCODE_LOG10") = 16;
	GRB.attr("OPCODE_LOG2") = 15;
	GRB.attr("OPCODE_LOGISTIC") = 17;
	GRB.attr("OPCODE_MINUS") = 3;
	GRB.attr("OPCODE_MULTIPLY") = 4;
	GRB.attr("OPCODE_PLUS") = 2;
	GRB.attr("OPCODE_POW") = 12;
	GRB.attr("OPCODE_SIN") = 9;
	GRB.attr("OPCODE_SQRT") = 8;
	GRB.attr("OPCODE_SQUARE") = 7;
	GRB.attr("OPCODE_TAN") = 11;
	GRB.attr("OPCODE_UMINUS") = 6;
	GRB.attr("OPCODE_VARIABLE") = 1;
	GRB.attr("OPTIMAL") = 2;
	GRB.attr("PARTITION_CLEANUP") = 1;
	GRB.attr("PARTITION_EARLY") = 16;
	GRB.attr("PARTITION_NODES") = 2;
	GRB.attr("PARTITION_ROOTEND") = 4;
	GRB.attr("PARTITION_ROOTSTART") = 8;
	GRB.attr("PHASE_MIP_IMPROVE") = 2;
	GRB.attr("PHASE_MIP_NOREL") = 0;
	GRB.attr("PHASE_MIP_SEARCH") = 1;
	GRB.attr("PRESOLVE_AGGRESSIVE") = 2;
	GRB.attr("PRESOLVE_AUTO") = -1;
	GRB.attr("PRESOLVE_CONSERVATIVE") = 1;
	GRB.attr("PRESOLVE_OFF") = 0;
	GRB.attr("SEMICONT") = "S";
	GRB.attr("SEMIINT") = "N";
	GRB.attr("SIMPLEXPRICING_AUTO") = -1;
	GRB.attr("SIMPLEXPRICING_DEVEX") = 2;
	GRB.attr("SIMPLEXPRICING_PARTIAL") = 0;
	GRB.attr("SIMPLEXPRICING_STEEPEST_EDGE") = 1;
	GRB.attr("SIMPLEXPRICING_STEEPEST_QUICK") = 3;
	GRB.attr("SOLUTION_LIMIT") = 10;
	GRB.attr("SOS_TYPE1") = 1;
	GRB.attr("SOS_TYPE2") = 2;
	GRB.attr("SUBOPTIMAL") = 13;
	GRB.attr("SUPERBASIC") = -3;
	GRB.attr("TIME_LIMIT") = 9;
	GRB.attr("UNBOUNDED") = 5;
	GRB.attr("UNDEFINED") = 1e+101;
	GRB.attr("USER_OBJ_LIMIT") = 15;
	GRB.attr("VARBRANCH_AUTO") = -1;
	GRB.attr("VARBRANCH_MAX_INFEAS") = 2;
	GRB.attr("VARBRANCH_PSEUDO_REDUCED") = 0;
	GRB.attr("VARBRANCH_PSEUDO_SHADOW") = 1;
	GRB.attr("VARBRANCH_STRONG") = 3;
	GRB.attr("VERSION_MAJOR") = 12;
	GRB.attr("VERSION_MINOR") = 0;
	GRB.attr("VERSION_TECHNICAL") = 0;
	GRB.attr("WORK_LIMIT") = 16;

	nb::module_ Attr = GRB.def_submodule("Attr");
	Attr.attr("BarIterCount") = "BarIterCount";
	Attr.attr("BarPi") = "BarPi";
	Attr.attr("BarX") = "BarX";
	Attr.attr("BatchErrorCode") = "BatchErrorCode";
	Attr.attr("BatchErrorMessage") = "BatchErrorMessage";
	Attr.attr("BatchID") = "BatchID";
	Attr.attr("BatchStatus") = "BatchStatus";
	Attr.attr("BoundSVio") = "BoundSVio";
	Attr.attr("BoundSVioIndex") = "BoundSVioIndex";
	Attr.attr("BoundSVioSum") = "BoundSVioSum";
	Attr.attr("BoundVio") = "BoundVio";
	Attr.attr("BoundVioIndex") = "BoundVioIndex";
	Attr.attr("BoundVioSum") = "BoundVioSum";
	Attr.attr("BranchPriority") = "BranchPriority";
	Attr.attr("CBasis") = "CBasis";
	Attr.attr("CTag") = "CTag";
	Attr.attr("ComplVio") = "ComplVio";
	Attr.attr("ComplVioIndex") = "ComplVioIndex";
	Attr.attr("ComplVioSum") = "ComplVioSum";
	Attr.attr("ConcurrentWinMethod") = "ConcurrentWinMethod";
	Attr.attr("ConstrName") = "ConstrName";
	Attr.attr("ConstrResidual") = "ConstrResidual";
	Attr.attr("ConstrResidualIndex") = "ConstrResidualIndex";
	Attr.attr("ConstrResidualSum") = "ConstrResidualSum";
	Attr.attr("ConstrSResidual") = "ConstrSResidual";
	Attr.attr("ConstrSResidualIndex") = "ConstrSResidualIndex";
	Attr.attr("ConstrSResidualSum") = "ConstrSResidualSum";
	Attr.attr("ConstrSVio") = "ConstrSVio";
	Attr.attr("ConstrSVioIndex") = "ConstrSVioIndex";
	Attr.attr("ConstrSVioSum") = "ConstrSVioSum";
	Attr.attr("ConstrVio") = "ConstrVio";
	Attr.attr("ConstrVioIndex") = "ConstrVioIndex";
	Attr.attr("ConstrVioSum") = "ConstrVioSum";
	Attr.attr("DNumNZs") = "DNumNZs";
	Attr.attr("DStart") = "DStart";
	Attr.attr("DualResidual") = "DualResidual";
	Attr.attr("DualResidualIndex") = "DualResidualIndex";
	Attr.attr("DualResidualSum") = "DualResidualSum";
	Attr.attr("DualSResidual") = "DualSResidual";
	Attr.attr("DualSResidualIndex") = "DualSResidualIndex";
	Attr.attr("DualSResidualSum") = "DualSResidualSum";
	Attr.attr("DualSVio") = "DualSVio";
	Attr.attr("DualSVioIndex") = "DualSVioIndex";
	Attr.attr("DualSVioSum") = "DualSVioSum";
	Attr.attr("DualVio") = "DualVio";
	Attr.attr("DualVioIndex") = "DualVioIndex";
	Attr.attr("DualVioSum") = "DualVioSum";
	Attr.attr("FarkasDual") = "FarkasDual";
	Attr.attr("FarkasProof") = "FarkasProof";
	Attr.attr("Fingerprint") = "Fingerprint";
	Attr.attr("FuncNonlinear") = "FuncNonlinear";
	Attr.attr("FuncPieceError") = "FuncPieceError";
	Attr.attr("FuncPieceLength") = "FuncPieceLength";
	Attr.attr("FuncPieceRatio") = "FuncPieceRatio";
	Attr.attr("FuncPieces") = "FuncPieces";
	Attr.attr("GenConstrName") = "GenConstrName";
	Attr.attr("GenConstrType") = "GenConstrType";
	Attr.attr("IISConstr") = "IISConstr";
	Attr.attr("IISConstrForce") = "IISConstrForce";
	Attr.attr("IISGenConstr") = "IISGenConstr";
	Attr.attr("IISGenConstrForce") = "IISGenConstrForce";
	Attr.attr("IISLB") = "IISLB";
	Attr.attr("IISLBForce") = "IISLBForce";
	Attr.attr("IISMinimal") = "IISMinimal";
	Attr.attr("IISQConstr") = "IISQConstr";
	Attr.attr("IISQConstrForce") = "IISQConstrForce";
	Attr.attr("IISSOS") = "IISSOS";
	Attr.attr("IISSOSForce") = "IISSOSForce";
	Attr.attr("IISUB") = "IISUB";
	Attr.attr("IISUBForce") = "IISUBForce";
	Attr.attr("IntVio") = "IntVio";
	Attr.attr("IntVioIndex") = "IntVioIndex";
	Attr.attr("IntVioSum") = "IntVioSum";
	Attr.attr("IsMIP") = "IsMIP";
	Attr.attr("IsMultiObj") = "IsMultiObj";
	Attr.attr("IsQCP") = "IsQCP";
	Attr.attr("IsQP") = "IsQP";
	Attr.attr("IterCount") = "IterCount";
	Attr.attr("Kappa") = "Kappa";
	Attr.attr("KappaExact") = "KappaExact";
	Attr.attr("LB") = "LB";
	Attr.attr("Lazy") = "Lazy";
	Attr.attr("LicenseExpiration") = "LicenseExpiration";
	Attr.attr("MIPGap") = "MIPGap";
	Attr.attr("MaxBound") = "MaxBound";
	Attr.attr("MaxCoeff") = "MaxCoeff";
	Attr.attr("MaxMemUsed") = "MaxMemUsed";
	Attr.attr("MaxObjCoeff") = "MaxObjCoeff";
	Attr.attr("MaxQCCoeff") = "MaxQCCoeff";
	Attr.attr("MaxQCLCoeff") = "MaxQCLCoeff";
	Attr.attr("MaxQCRHS") = "MaxQCRHS";
	Attr.attr("MaxQObjCoeff") = "MaxQObjCoeff";
	Attr.attr("MaxRHS") = "MaxRHS";
	Attr.attr("MaxVio") = "MaxVio";
	Attr.attr("MemUsed") = "MemUsed";
	Attr.attr("MinBound") = "MinBound";
	Attr.attr("MinCoeff") = "MinCoeff";
	Attr.attr("MinObjCoeff") = "MinObjCoeff";
	Attr.attr("MinQCCoeff") = "MinQCCoeff";
	Attr.attr("MinQCLCoeff") = "MinQCLCoeff";
	Attr.attr("MinQCRHS") = "MinQCRHS";
	Attr.attr("MinQObjCoeff") = "MinQObjCoeff";
	Attr.attr("MinRHS") = "MinRHS";
	Attr.attr("ModelName") = "ModelName";
	Attr.attr("ModelSense") = "ModelSense";
	Attr.attr("NodeCount") = "NodeCount";
	Attr.attr("NumBinVars") = "NumBinVars";
	Attr.attr("NumConstrs") = "NumConstrs";
	Attr.attr("NumGenConstrs") = "NumGenConstrs";
	Attr.attr("NumIntVars") = "NumIntVars";
	Attr.attr("NumNZs") = "NumNZs";
	Attr.attr("NumObj") = "NumObj";
	Attr.attr("NumPWLObjVars") = "NumPWLObjVars";
	Attr.attr("NumQCNZs") = "NumQCNZs";
	Attr.attr("NumQConstrs") = "NumQConstrs";
	Attr.attr("NumQNZs") = "NumQNZs";
	Attr.attr("NumSOS") = "NumSOS";
	Attr.attr("NumScenarios") = "NumScenarios";
	Attr.attr("NumStart") = "NumStart";
	Attr.attr("NumVars") = "NumVars";
	Attr.attr("Obj") = "Obj";
	Attr.attr("ObjBound") = "ObjBound";
	Attr.attr("ObjBoundC") = "ObjBoundC";
	Attr.attr("ObjCon") = "ObjCon";
	Attr.attr("ObjN") = "ObjN";
	Attr.attr("ObjNAbsTol") = "ObjNAbsTol";
	Attr.attr("ObjNCon") = "ObjNCon";
	Attr.attr("ObjNName") = "ObjNName";
	Attr.attr("ObjNPriority") = "ObjNPriority";
	Attr.attr("ObjNRelTol") = "ObjNRelTol";
	Attr.attr("ObjNVal") = "ObjNVal";
	Attr.attr("ObjNWeight") = "ObjNWeight";
	Attr.attr("ObjVal") = "ObjVal";
	Attr.attr("PStart") = "PStart";
	Attr.attr("PWLObjCvx") = "PWLObjCvx";
	Attr.attr("Partition") = "Partition";
	Attr.attr("Pi") = "Pi";
	Attr.attr("PoolIgnore") = "PoolIgnore";
	Attr.attr("PoolObjBound") = "PoolObjBound";
	Attr.attr("PoolObjVal") = "PoolObjVal";
	Attr.attr("PreFixVal") = "PreFixVal";
	Attr.attr("QCName") = "QCName";
	Attr.attr("QCPi") = "QCPi";
	Attr.attr("QCRHS") = "QCRHS";
	Attr.attr("QCSense") = "QCSense";
	Attr.attr("QCSlack") = "QCSlack";
	Attr.attr("QCTag") = "QCTag";
	Attr.attr("RC") = "RC";
	Attr.attr("RHS") = "RHS";
	Attr.attr("Runtime") = "Runtime";
	Attr.attr("SALBLow") = "SALBLow";
	Attr.attr("SALBUp") = "SALBUp";
	Attr.attr("SAObjLow") = "SAObjLow";
	Attr.attr("SAObjUp") = "SAObjUp";
	Attr.attr("SARHSLow") = "SARHSLow";
	Attr.attr("SARHSUp") = "SARHSUp";
	Attr.attr("SAUBLow") = "SAUBLow";
	Attr.attr("SAUBUp") = "SAUBUp";
	Attr.attr("ScenNLB") = "ScenNLB";
	Attr.attr("ScenNName") = "ScenNName";
	Attr.attr("ScenNObj") = "ScenNObj";
	Attr.attr("ScenNObjBound") = "ScenNObjBound";
	Attr.attr("ScenNObjVal") = "ScenNObjVal";
	Attr.attr("ScenNRHS") = "ScenNRHS";
	Attr.attr("ScenNUB") = "ScenNUB";
	Attr.attr("ScenNX") = "ScenNX";
	Attr.attr("Sense") = "Sense";
	Attr.attr("Slack") = "Slack";
	Attr.attr("SolCount") = "SolCount";
	Attr.attr("Start") = "Start";
	Attr.attr("Status") = "Status";
	Attr.attr("TuneResultCount") = "TuneResultCount";
	Attr.attr("UB") = "UB";
	Attr.attr("UnbdRay") = "UnbdRay";
	Attr.attr("VBasis") = "VBasis";
	Attr.attr("VTag") = "VTag";
	Attr.attr("VType") = "VType";
	Attr.attr("VarHintPri") = "VarHintPri";
	Attr.attr("VarHintVal") = "VarHintVal";
	Attr.attr("VarName") = "VarName";
	Attr.attr("VarPreStat") = "VarPreStat";
	Attr.attr("Work") = "Work";
	Attr.attr("X") = "X";
	Attr.attr("Xn") = "Xn";

	nb::module_ Param = GRB.def_submodule("Param");
	Param.attr("AggFill") = "AggFill";
	Param.attr("Aggregate") = "Aggregate";
	Param.attr("BQPCuts") = "BQPCuts";
	Param.attr("BarConvTol") = "BarConvTol";
	Param.attr("BarCorrectors") = "BarCorrectors";
	Param.attr("BarHomogeneous") = "BarHomogeneous";
	Param.attr("BarIterLimit") = "BarIterLimit";
	Param.attr("BarOrder") = "BarOrder";
	Param.attr("BarQCPConvTol") = "BarQCPConvTol";
	Param.attr("BestBdStop") = "BestBdStop";
	Param.attr("BestObjStop") = "BestObjStop";
	Param.attr("BranchDir") = "BranchDir";
	Param.attr("CSAPIAccessID") = "CSAPIAccessID";
	Param.attr("CSAPISecret") = "CSAPISecret";
	Param.attr("CSAppName") = "CSAppName";
	Param.attr("CSAuthToken") = "CSAuthToken";
	Param.attr("CSBatchMode") = "CSBatchMode";
	Param.attr("CSClientLog") = "CSClientLog";
	Param.attr("CSGroup") = "CSGroup";
	Param.attr("CSIdleTimeout") = "CSIdleTimeout";
	Param.attr("CSManager") = "CSManager";
	Param.attr("CSPriority") = "CSPriority";
	Param.attr("CSQueueTimeout") = "CSQueueTimeout";
	Param.attr("CSRouter") = "CSRouter";
	Param.attr("CSTLSInsecure") = "CSTLSInsecure";
	Param.attr("CliqueCuts") = "CliqueCuts";
	Param.attr("CloudAccessID") = "CloudAccessID";
	Param.attr("CloudHost") = "CloudHost";
	Param.attr("CloudPool") = "CloudPool";
	Param.attr("CloudSecretKey") = "CloudSecretKey";
	Param.attr("ComputeServer") = "ComputeServer";
	Param.attr("ConcurrentJobs") = "ConcurrentJobs";
	Param.attr("ConcurrentMIP") = "ConcurrentMIP";
	Param.attr("ConcurrentMethod") = "ConcurrentMethod";
	Param.attr("CoverCuts") = "CoverCuts";
	Param.attr("Crossover") = "Crossover";
	Param.attr("CrossoverBasis") = "CrossoverBasis";
	Param.attr("CutAggPasses") = "CutAggPasses";
	Param.attr("CutPasses") = "CutPasses";
	Param.attr("Cutoff") = "Cutoff";
	Param.attr("Cuts") = "Cuts";
	Param.attr("DegenMoves") = "DegenMoves";
	Param.attr("Disconnected") = "Disconnected";
	Param.attr("DisplayInterval") = "DisplayInterval";
	Param.attr("DistributedMIPJobs") = "DistributedMIPJobs";
	Param.attr("DualImpliedCuts") = "DualImpliedCuts";
	Param.attr("DualReductions") = "DualReductions";
	Param.attr("FeasRelaxBigM") = "FeasRelaxBigM";
	Param.attr("FeasibilityTol") = "FeasibilityTol";
	Param.attr("FlowCoverCuts") = "FlowCoverCuts";
	Param.attr("FlowPathCuts") = "FlowPathCuts";
	Param.attr("FuncMaxVal") = "FuncMaxVal";
	Param.attr("FuncNonlinear") = "FuncNonlinear";
	Param.attr("FuncPieceError") = "FuncPieceError";
	Param.attr("FuncPieceLength") = "FuncPieceLength";
	Param.attr("FuncPieceRatio") = "FuncPieceRatio";
	Param.attr("FuncPieces") = "FuncPieces";
	Param.attr("GUBCoverCuts") = "GUBCoverCuts";
	Param.attr("GomoryPasses") = "GomoryPasses";
	Param.attr("Heuristics") = "Heuristics";
	Param.attr("IISMethod") = "IISMethod";
	Param.attr("IgnoreNames") = "IgnoreNames";
	Param.attr("ImpliedCuts") = "ImpliedCuts";
	Param.attr("ImproveStartGap") = "ImproveStartGap";
	Param.attr("ImproveStartNodes") = "ImproveStartNodes";
	Param.attr("ImproveStartTime") = "ImproveStartTime";
	Param.attr("InfProofCuts") = "InfProofCuts";
	Param.attr("InfUnbdInfo") = "InfUnbdInfo";
	Param.attr("IntFeasTol") = "IntFeasTol";
	Param.attr("IntegralityFocus") = "IntegralityFocus";
	Param.attr("IterationLimit") = "IterationLimit";
	Param.attr("JSONSolDetail") = "JSONSolDetail";
	Param.attr("JobID") = "JobID";
	Param.attr("LPWarmStart") = "LPWarmStart";
	Param.attr("LazyConstraints") = "LazyConstraints";
	Param.attr("LicenseID") = "LicenseID";
	Param.attr("LiftProjectCuts") = "LiftProjectCuts";
	Param.attr("LogFile") = "LogFile";
	Param.attr("LogToConsole") = "LogToConsole";
	Param.attr("MIPFocus") = "MIPFocus";
	Param.attr("MIPGap") = "MIPGap";
	Param.attr("MIPGapAbs") = "MIPGapAbs";
	Param.attr("MIPSepCuts") = "MIPSepCuts";
	Param.attr("MIQCPMethod") = "MIQCPMethod";
	Param.attr("MIRCuts") = "MIRCuts";
	Param.attr("MarkowitzTol") = "MarkowitzTol";
	Param.attr("MemLimit") = "MemLimit";
	Param.attr("Method") = "Method";
	Param.attr("MinRelNodes") = "MinRelNodes";
	Param.attr("MixingCuts") = "MixingCuts";
	Param.attr("ModKCuts") = "ModKCuts";
	Param.attr("MultiObjMethod") = "MultiObjMethod";
	Param.attr("MultiObjPre") = "MultiObjPre";
	Param.attr("NLPHeur") = "NLPHeur";
	Param.attr("NetworkAlg") = "NetworkAlg";
	Param.attr("NetworkCuts") = "NetworkCuts";
	Param.attr("NoRelHeurTime") = "NoRelHeurTime";
	Param.attr("NoRelHeurWork") = "NoRelHeurWork";
	Param.attr("NodeLimit") = "NodeLimit";
	Param.attr("NodeMethod") = "NodeMethod";
	Param.attr("NodefileDir") = "NodefileDir";
	Param.attr("NodefileStart") = "NodefileStart";
	Param.attr("NonConvex") = "NonConvex";
	Param.attr("NormAdjust") = "NormAdjust";
	Param.attr("NumericFocus") = "NumericFocus";
	Param.attr("OBBT") = "OBBT";
	Param.attr("ObjNumber") = "ObjNumber";
	Param.attr("ObjScale") = "ObjScale";
	Param.attr("OptimalityTol") = "OptimalityTol";
	Param.attr("OutputFlag") = "OutputFlag";
	Param.attr("PSDCuts") = "PSDCuts";
	Param.attr("PSDTol") = "PSDTol";
	Param.attr("PartitionPlace") = "PartitionPlace";
	Param.attr("PerturbValue") = "PerturbValue";
	Param.attr("PoolGap") = "PoolGap";
	Param.attr("PoolGapAbs") = "PoolGapAbs";
	Param.attr("PoolSearchMode") = "PoolSearchMode";
	Param.attr("PoolSolutions") = "PoolSolutions";
	Param.attr("PreCrush") = "PreCrush";
	Param.attr("PreDepRow") = "PreDepRow";
	Param.attr("PreDual") = "PreDual";
	Param.attr("PreMIQCPForm") = "PreMIQCPForm";
	Param.attr("PrePasses") = "PrePasses";
	Param.attr("PreQLinearize") = "PreQLinearize";
	Param.attr("PreSOS1BigM") = "PreSOS1BigM";
	Param.attr("PreSOS1Encoding") = "PreSOS1Encoding";
	Param.attr("PreSOS2BigM") = "PreSOS2BigM";
	Param.attr("PreSOS2Encoding") = "PreSOS2Encoding";
	Param.attr("PreSparsify") = "PreSparsify";
	Param.attr("Presolve") = "Presolve";
	Param.attr("ProjImpliedCuts") = "ProjImpliedCuts";
	Param.attr("PumpPasses") = "PumpPasses";
	Param.attr("QCPDual") = "QCPDual";
	Param.attr("Quad") = "Quad";
	Param.attr("RINS") = "RINS";
	Param.attr("RLTCuts") = "RLTCuts";
	Param.attr("Record") = "Record";
	Param.attr("RelaxLiftCuts") = "RelaxLiftCuts";
	Param.attr("ResultFile") = "ResultFile";
	Param.attr("ScaleFlag") = "ScaleFlag";
	Param.attr("ScenarioNumber") = "ScenarioNumber";
	Param.attr("Seed") = "Seed";
	Param.attr("ServerPassword") = "ServerPassword";
	Param.attr("ServerTimeout") = "ServerTimeout";
	Param.attr("SiftMethod") = "SiftMethod";
	Param.attr("Sifting") = "Sifting";
	Param.attr("SimplexPricing") = "SimplexPricing";
	Param.attr("SoftMemLimit") = "SoftMemLimit";
	Param.attr("SolFiles") = "SolFiles";
	Param.attr("SolutionLimit") = "SolutionLimit";
	Param.attr("SolutionNumber") = "SolutionNumber";
	Param.attr("SolutionTarget") = "SolutionTarget";
	Param.attr("StartNodeLimit") = "StartNodeLimit";
	Param.attr("StartNumber") = "StartNumber";
	Param.attr("StrongCGCuts") = "StrongCGCuts";
	Param.attr("SubMIPCuts") = "SubMIPCuts";
	Param.attr("SubMIPNodes") = "SubMIPNodes";
	Param.attr("Symmetry") = "Symmetry";
	Param.attr("TSPort") = "TSPort";
	Param.attr("ThreadLimit") = "ThreadLimit";
	Param.attr("Threads") = "Threads";
	Param.attr("TimeLimit") = "TimeLimit";
	Param.attr("TokenServer") = "TokenServer";
	Param.attr("TuneCleanup") = "TuneCleanup";
	Param.attr("TuneCriterion") = "TuneCriterion";
	Param.attr("TuneDynamicJobs") = "TuneDynamicJobs";
	Param.attr("TuneJobs") = "TuneJobs";
	Param.attr("TuneMetric") = "TuneMetric";
	Param.attr("TuneOutput") = "TuneOutput";
	Param.attr("TuneResults") = "TuneResults";
	Param.attr("TuneTargetMIPGap") = "TuneTargetMIPGap";
	Param.attr("TuneTargetTime") = "TuneTargetTime";
	Param.attr("TuneTimeLimit") = "TuneTimeLimit";
	Param.attr("TuneTrials") = "TuneTrials";
	Param.attr("UpdateMode") = "UpdateMode";
	Param.attr("UserName") = "UserName";
	Param.attr("VarBranch") = "VarBranch";
	Param.attr("WLSAccessID") = "WLSAccessID";
	Param.attr("WLSConfig") = "WLSConfig";
	Param.attr("WLSProxy") = "WLSProxy";
	Param.attr("WLSSecret") = "WLSSecret";
	Param.attr("WLSToken") = "WLSToken";
	Param.attr("WLSTokenDuration") = "WLSTokenDuration";
	Param.attr("WLSTokenRefresh") = "WLSTokenRefresh";
	Param.attr("WorkLimit") = "WorkLimit";
	Param.attr("WorkerPassword") = "WorkerPassword";
	Param.attr("WorkerPool") = "WorkerPool";
	Param.attr("ZeroHalfCuts") = "ZeroHalfCuts";
	Param.attr("ZeroObjNodes") = "ZeroObjNodes";

	nb::module_ Callback = GRB.def_submodule("Callback");
	Callback.attr("BARRIER") = 7;
	Callback.attr("BARRIER_COMPL") = 7006;
	Callback.attr("BARRIER_DUALINF") = 7005;
	Callback.attr("BARRIER_DUALOBJ") = 7003;
	Callback.attr("BARRIER_ITRCNT") = 7001;
	Callback.attr("BARRIER_PRIMINF") = 7004;
	Callback.attr("BARRIER_PRIMOBJ") = 7002;
	Callback.attr("IIS") = 9;
	Callback.attr("IIS_BOUNDGUESS") = 9006;
	Callback.attr("IIS_BOUNDMAX") = 9005;
	Callback.attr("IIS_BOUNDMIN") = 9004;
	Callback.attr("IIS_CONSTRGUESS") = 9003;
	Callback.attr("IIS_CONSTRMAX") = 9002;
	Callback.attr("IIS_CONSTRMIN") = 9001;
	Callback.attr("MAXMEMUSED") = 6005;
	Callback.attr("MEMUSED") = 6004;
	Callback.attr("MESSAGE") = 6;
	Callback.attr("MIP") = 3;
	Callback.attr("MIPNODE") = 5;
	Callback.attr("MIPNODE_BRVAR") = 5007;
	Callback.attr("MIPNODE_NODCNT") = 5005;
	Callback.attr("MIPNODE_OBJBND") = 5004;
	Callback.attr("MIPNODE_OBJBST") = 5003;
	Callback.attr("MIPNODE_OPENSCENARIOS") = 5008;
	Callback.attr("MIPNODE_PHASE") = 5009;
	Callback.attr("MIPNODE_REL") = 5002;
	Callback.attr("MIPNODE_SOLCNT") = 5006;
	Callback.attr("MIPNODE_STATUS") = 5001;
	Callback.attr("MIPSOL") = 4;
	Callback.attr("MIPSOL_NODCNT") = 4005;
	Callback.attr("MIPSOL_OBJ") = 4002;
	Callback.attr("MIPSOL_OBJBND") = 4004;
	Callback.attr("MIPSOL_OBJBST") = 4003;
	Callback.attr("MIPSOL_OPENSCENARIOS") = 4007;
	Callback.attr("MIPSOL_PHASE") = 4008;
	Callback.attr("MIPSOL_SOL") = 4001;
	Callback.attr("MIPSOL_SOLCNT") = 4006;
	Callback.attr("MIP_CUTCNT") = 3004;
	Callback.attr("MIP_ITRCNT") = 3006;
	Callback.attr("MIP_NODCNT") = 3002;
	Callback.attr("MIP_NODLFT") = 3005;
	Callback.attr("MIP_OBJBND") = 3001;
	Callback.attr("MIP_OBJBST") = 3000;
	Callback.attr("MIP_OPENSCENARIOS") = 3007;
	Callback.attr("MIP_PHASE") = 3008;
	Callback.attr("MIP_SOLCNT") = 3003;
	Callback.attr("MSG_STRING") = 6001;
	Callback.attr("MULTIOBJ") = 8;
	Callback.attr("MULTIOBJ_ITRCNT") = 8004;
	Callback.attr("MULTIOBJ_MIPGAP") = 8008;
	Callback.attr("MULTIOBJ_NODCNT") = 8009;
	Callback.attr("MULTIOBJ_NODLFT") = 8010;
	Callback.attr("MULTIOBJ_OBJBND") = 8006;
	Callback.attr("MULTIOBJ_OBJBST") = 8005;
	Callback.attr("MULTIOBJ_OBJCNT") = 8001;
	Callback.attr("MULTIOBJ_RUNTIME") = 8011;
	Callback.attr("MULTIOBJ_SOL") = 8003;
	Callback.attr("MULTIOBJ_SOLCNT") = 8002;
	Callback.attr("MULTIOBJ_STATUS") = 8007;
	Callback.attr("MULTIOBJ_WORK") = 8012;
	Callback.attr("POLLING") = 0;
	Callback.attr("PRESOLVE") = 1;
	Callback.attr("PRE_BNDCHG") = 1003;
	Callback.attr("PRE_COECHG") = 1004;
	Callback.attr("PRE_COLDEL") = 1000;
	Callback.attr("PRE_ROWDEL") = 1001;
	Callback.attr("PRE_SENCHG") = 1002;
	Callback.attr("RUNTIME") = 6002;
	Callback.attr("SIMPLEX") = 2;
	Callback.attr("SPX_DUALINF") = 2003;
	Callback.attr("SPX_ISPERT") = 2004;
	Callback.attr("SPX_ITRCNT") = 2000;
	Callback.attr("SPX_OBJVAL") = 2001;
	Callback.attr("SPX_PRIMINF") = 2002;
	Callback.attr("WORK") = 6003;
}
