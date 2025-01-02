#ifndef _MINDOPT_TYPE_H_
#define _MINDOPT_TYPE_H_


#if defined(_WIN32) || defined(_WIN64)
 
#ifdef MDO_BUILDING_LIB
#   define MDODLL
#   define MDOAPI               __stdcall
#   define MDOAPIVA             __cdecl
#else
#ifdef MDO_BUILDING_DLL
#   define MDODLL               __declspec(dllexport)
#else
#   define MDODLL               __declspec(dllimport)
#endif
#   define MDOAPI               __stdcall
#   define MDOAPIVA             __cdecl
#endif
 
#elif defined(__linux__)
 
#   define MDODLL
#   define MDOAPI
#   define MDOAPIVA
 
#elif defined(__APPLE__)
 
#   define MDODLL
#   define MDOAPI
#   define MDOAPIVA
 
#endif

#ifndef RESULT_TYPE_H
#define RESULT_TYPE_H

 
typedef enum MdoStatus
{
    MDO_UNKNOWN                                           =         0,  
    MDO_OPTIMAL                                           =         1,  
    MDO_INFEASIBLE                                        =         2,  
    MDO_UNBOUNDED                                         =         3,  
    MDO_INF_OR_UBD                                        =         4,  
    MDO_SUB_OPTIMAL                                       =         5,  
} MdoStatus;

 
typedef enum MdoResult
{
     
     
     

    MDO_OKAY                                        =         0,  
    MDO_ERROR                                       =        -1,  
    MDO_NOMEMORY                                    =        -2,  
    MDO_INVALID_ARGUMENT                            =        -3,  
    MDO_INVALID_LICENSE                             =       -10,  
    MDO_HOME_ENV_NOT_FOUND                          =       -11,  
    MDO_DLL_ERROR                                   =       -12,  

     
     
     

    MDO_IO_ERROR                                    =     -1000,  
    MDO_FILE_READ_ERROR                             =     -1001,  
    MDO_FILE_WRITE_ERROR                            =     -1002,  
    MDO_DIRECTORY_ERROR                             =     -1003,  
    MDO_FORMAT_ERROR                                =     -1100,  
    MDO_VERSION_ERROR                               =     -1101,  
    MDO_REMOTE_INVALID_TOKEN                        =     -1200,  
    MDO_REMOTE_CONNECTION_ERROR                     =     -1201,  

     
     
     

    MDO_MODEL_INPUT_ERROR                           =     -2000,  
    MDO_MODEL_EMPTY                                 =     -2001,  
    MDO_MODEL_INVALID_ROW_IDX                       =     -2002,  
    MDO_MODEL_INVALID_COL_IDX                       =     -2003,  
    MDO_MODEL_INVALID_ROW_NAME                      =     -2004,  
    MDO_MODEL_INVALID_COL_NAME                      =     -2005,  
    MDO_MODEL_INVALID_SYM_MAT_IDX                   =     -2006,  
    MDO_MODEL_INVALID_SYM_MAT_ROW_IDX               =     -2007,  
    MDO_MODEL_INVALID_SYM_MAT_COL_IDX               =     -2008,  
    MDO_MODEL_INVALID_STR_ATTR                      =     -2010,  
    MDO_MODEL_INVALID_INT_ATTR                      =     -2011,  
    MDO_MODEL_INVALID_REAL_ATTR                     =     -2012,  
    MDO_MODEL_INVALID_REAL_ATTR_SYM_MAT             =     -2013,  
    MDO_MODEL_INVALID_CHAR_ATTR                     =     -2014,  
    MDO_MODEL_INVALID_MAT_ATTR                      =     -2015,  
    MDO_MODEL_INVALID_ATTR_NAME                     =     -2019,  
    MDO_MODEL_INVALID_SOS_TYPE                      =     -2020,  
    MDO_MODEL_INVALID_SOS_IDX                       =     -2021,  
    MDO_MODEL_INVALID_INDICATOR_COL_IDX             =     -2022,  
    MDO_MODEL_INVALID_INDICATOR_ROW_IDX             =     -2023,  
    MDO_MODEL_INVALID_INT_RELAX                     =     -2024,  
    MDO_DATA_NOT_AVAILABLE                          =     -2025,  

     
     
     

    MDO_NO_SOLN                                     =     -3000,  
    MDO_NO_RAY                                      =     -3001,  
    MDO_NO_STATISTICS                               =     -3002,  
    MDO_INVALID_BASIS_STATUS                        =     -3003,  
    MDO_IIS_NO_SOLN                                 =     -3004,  
    MDO_IIS_FEASIBLE                                =     -3005,  
    MDO_INVALID_SOL_IDX                             =     -3006,  

     
     
     

    MDO_PARAM_SET_ERROR                             =     -4000,  
    MDO_PARAM_GET_ERROR                             =     -4001,  

     
     
     

    MDO_CB_INVALID_WHERE                            =     -5000,  
    MDO_CB_INVALID_WHAT                             =     -5001,  
    MDO_CB_INVALID_SUBMISSION                       =     -5002,  

     
     
     

    MDO_ABORT_ITERATION_LIMIT                       =     -9000,  
    MDO_ABORT_TIME_LIMIT                            =     -9001,  
    MDO_ABORT_INTERRUPTED                           =     -9002,  
    MDO_ABORT_NODE_LIMIT                            =     -9003,  
    MDO_ABORT_STALLING_NODE_LIMIT                   =     -9004,  
    MDO_ABORT_SOLUTION_LIMIT                        =     -9005,  
    MDO_ABORT_INVALID_METHOD                        =     -9011,  
    MDO_ABORT_SOLVER_NOT_AVAILABLE                  =     -9012,  

     
     
     

    MDO_SIMPLEX_NUMERIC                             =    -10000,  

     
     
     

    MDO_INTERIOR_NUMERIC                            =    -20000,  

     
     
     

    MDO_IIS_NUMERIC                                 =    -30000,  

     
     
     

    MDO_CE_VALIDATE_FAILED                          =    -40000,  

} MdoResult;

#endif  

 

 

#define MDO_INT_ATTR_MIN_SENSE                            "MinSense"
#define MDO_INT_ATTR_MODEL_SENSE                          "ModelSense"
#define MDO_INT_ATTR_HAS_SOLUTION                         "HasSolution"
#define MDO_INT_ATTR_HAS_PRIMAL_RAY                       "HasPrimalRay"
#define MDO_INT_ATTR_HAS_DUAL_RAY                         "HasDualRay"
#define MDO_INT_ATTR_NUM_VARS                             "NumVars"
#define MDO_INT_ATTR_NUM_CONSS                            "NumConss"
#define MDO_INT_ATTR_NUM_CONSTRS                          "NumConstrs"
#define MDO_INT_ATTR_NUM_Q_CONSTRS                        "NumQConstrs"
#define MDO_INT_ATTR_NUM_SOS                              "NumSOS"
#define MDO_INT_ATTR_NUM_GEN_CONSTRS                      "NumGenConstrs"
#define MDO_INT_ATTR_NUM_ENTS                             "NumEnts"
#define MDO_INT_ATTR_NUM_NZS                              "NumNZs"
#define MDO_INT_ATTR_SPX_NUM_ITERS                        "SPX/NumIters"
#define MDO_INT_ATTR_IPM_NUM_ITERS                        "IPM/NumIters"
#define MDO_INT_ATTR_IS_INTEGER                           "IsInteger"
#define MDO_INT_ATTR_IIS_VAR                              "IISVar"
#define MDO_INT_ATTR_IIS_CONSTR                           "IISConstr"
#define MDO_INT_ATTR_IIS_COL                              "IISCol"
#define MDO_INT_ATTR_IIS_ROW                              "IISRow"
#define MDO_INT_ATTR_COL_BASIS                            "ColBasis"
#define MDO_INT_ATTR_ROW_BASIS                            "RowBasis"
#define MDO_INT_ATTR_GEN_CONSTR_TYPE                      "GenConstrType"
#define MDO_INT_ATTR_STATUS                               "Status"
#define MDO_INT_ATTR_DIM                                  "Dim"
#define MDO_INT_ATTR_NUM_PSD_VARS                         "NumPsdVars"
#define MDO_INT_ATTR_NUM_PSD_CONSTRS                      "NumPsdConstrs"
#define MDO_INT_ATTR_SOL_COUNT                            "SolCount"

 

#define MDO_DBL_ATTR_OBJ_CONST                            "ObjConst"
#define MDO_DBL_ATTR_OBJ_CON                              "ObjCon"
#define MDO_DBL_ATTR_PRIMAL_OBJ_VAL                       "PrimalObjVal"
#define MDO_DBL_ATTR_OBJ_VAL                              "ObjVal"
#define MDO_DBL_ATTR_DUAL_OBJ_VAL                         "DualObjVal"
#define MDO_DBL_ATTR_PRESOLVER_TIME                       "PresolverTime"
#define MDO_DBL_ATTR_SOLVER_TIME                          "SolverTime"
#define MDO_DBL_ATTR_SOLUTION_TIME                        "SolutionTime"
#define MDO_DBL_ATTR_MIP_GAP_ABS                          "MIP/GapAbs"
#define MDO_DBL_ATTR_MIP_GAP_REL                          "MIP/GapRel"
#define MDO_DBL_ATTR_LB                                   "LB"
#define MDO_DBL_ATTR_UB                                   "UB"
#define MDO_DBL_ATTR_OBJ                                  "Obj"
#define MDO_DBL_ATTR_LHS                                  "LHS"
#define MDO_DBL_ATTR_RHS                                  "RHS"
#define MDO_DBL_ATTR_Q_C_LHS                              "QCLHS"
#define MDO_DBL_ATTR_Q_C_RHS                              "QCRHS"
#define MDO_DBL_ATTR_PRIMAL_SOLN                          "PrimalSoln"
#define MDO_DBL_ATTR_X                                    "X"
#define MDO_DBL_ATTR_XN                                   "Xn"
#define MDO_DBL_ATTR_START                                "Start"
#define MDO_DBL_ATTR_DUAL_SOLN                            "DualSoln"
#define MDO_DBL_ATTR_ACTIVITY                             "Activity"
#define MDO_DBL_ATTR_REDUCED_COST                         "ReducedCost"
#define MDO_DBL_ATTR_RC                                   "RC"
#define MDO_DBL_ATTR_PSD_C_LHS                            "PsdCLHS"
#define MDO_DBL_ATTR_PSD_C_RHS                            "PsdCRHS"

 

#define MDO_STR_ATTR_PROB_NAME                            "ProbName"
#define MDO_STR_ATTR_MODEL_NAME                           "ModelName"
#define MDO_STR_ATTR_ROW_NAME                             "RowName"
#define MDO_STR_ATTR_CONSTR_NAME                          "ConstrName"
#define MDO_STR_ATTR_Q_C_NAME                             "QCName"
#define MDO_STR_ATTR_COL_NAME                             "ColName"
#define MDO_STR_ATTR_VAR_NAME                             "VarName"
#define MDO_STR_ATTR_PSD_VAR_NAME                         "PsdVarName"
#define MDO_STR_ATTR_PSD_C_NAME                           "PsdCName"
#define MDO_STR_ATTR_GEN_CONSTR_NAME                      "GenConstrName"

 

#define MDO_CHAR_ATTR_VTYPE                               "VType"

 

#define MDO_MAT_ATTR_PSD_X                                "PsdX"
#define MDO_MAT_ATTR_PSD_OBJ                              "PsdObj"


 

 

#define MDO_INT_PAR_METHOD                                "Method"
#define MDO_INT_PAR_POST_SCALING                          "PostScaling"
#define MDO_INT_PAR_PRESOLVE                              "Presolve"
#define MDO_INT_PAR_DUALIZATION                           "Dualization"
#define MDO_INT_PAR_OUTPUT_FLAG                           "OutputFlag"
#define MDO_INT_PAR_LOG_TO_CONSOLE                        "LogToConsole"
#define MDO_INT_PAR_NUM_THREADS                           "NumThreads"
#define MDO_INT_PAR_ENABLE_NETWORK_FLOW                   "EnableNetworkFlow"
#define MDO_INT_PAR_ENABLE_STOCHASTIC_LP                  "EnableStochasticLP"
#define MDO_INT_PAR_NUMERIC_FOCUS                         "NumericFocus"
#define MDO_INT_PAR_SPX_COLUMN_GENERATION                 "SPX/ColumnGeneration"
#define MDO_INT_PAR_SPX_CRASH_START                       "SPX/CrashStart"
#define MDO_INT_PAR_SPX_MAX_ITERATIONS                    "SPX/MaxIterations"
#define MDO_INT_PAR_SPX_PRIMAL_PRICING                    "SPX/PrimalPricing"
#define MDO_INT_PAR_SPX_DUAL_PRICING                      "SPX/DualPricing"
#define MDO_INT_PAR_IPM_MAX_ITERATIONS                    "IPM/MaxIterations"
#define MDO_INT_PAR_MIP_LEVEL_CUTS                        "MIP/LevelCuts"
#define MDO_INT_PAR_MIP_LEVEL_HEURS                       "MIP/LevelHeurs"
#define MDO_INT_PAR_MIP_LEVEL_PROBING                     "MIP/LevelProbing"
#define MDO_INT_PAR_MIP_LEVEL_STRONG                      "MIP/LevelStrong"
#define MDO_INT_PAR_MIP_LEVEL_SUBMIP                      "MIP/LevelSubmip"
#define MDO_INT_PAR_MIP_MAX_NODES                         "MIP/MaxNodes"
#define MDO_INT_PAR_MIP_ROOT_PARALLELISM                  "MIP/RootParallelism"
#define MDO_INT_PAR_MIP_AUTO_CONFIGURATION                "MIP/AutoConfiguration"
#define MDO_INT_PAR_MIP_MAX_SOLS                          "MIP/MaxSols"
#define MDO_INT_PAR_MIP_ALLOW_DUAL_PRESOLVE               "MIP/AllowDualPresolve"
#define MDO_INT_PAR_MIP_ENABLE_LAZY_CONSTR                "MIP/EnableLazyConstr"
#define MDO_INT_PAR_MIP_DETECT_DISCONNECTED_COMPONENTS    "MIP/DetectDisconnectedComponents"
#define MDO_INT_PAR_MIP_SOLUTION_POOL_SIZE                "MIP/SolutionPoolSize"
#define MDO_INT_PAR_MIP_MAX_STALLING_NODES                "MIP/MaxStallingNodes"
#define MDO_INT_PAR_MIP_SOLUTION_NUMBER                   "MIP/SolutionNumber"
#define MDO_INT_PAR_SOLUTION_TARGET                       "SolutionTarget"

 

#define MDO_DBL_PAR_MAX_TIME                              "MaxTime"
#define MDO_DBL_PAR_SPX_PRIMAL_TOLERANCE                  "SPX/PrimalTolerance"
#define MDO_DBL_PAR_SPX_DUAL_TOLERANCE                    "SPX/DualTolerance"
#define MDO_DBL_PAR_IPM_PRIMAL_TOLERANCE                  "IPM/PrimalTolerance"
#define MDO_DBL_PAR_IPM_DUAL_TOLERANCE                    "IPM/DualTolerance"
#define MDO_DBL_PAR_IPM_GAP_TOLERANCE                     "IPM/GapTolerance"
#define MDO_DBL_PAR_MIP_INTEGER_TOLERANCE                 "MIP/IntegerTolerance"
#define MDO_DBL_PAR_MIP_OBJECTIVE_TOLERANCE               "MIP/ObjectiveTolerance"
#define MDO_DBL_PAR_MIP_GAP_ABS                           "MIP/GapAbs"
#define MDO_DBL_PAR_MIP_GAP_REL                           "MIP/GapRel"
#define MDO_DBL_PAR_MIP_LINEARIZATION_BIG_M               "MIP/LinearizationBigM"
#define MDO_DBL_PAR_MIP_CUTOFF                            "MIP/Cutoff"

 

#define MDO_STR_PAR_LOG_FILE                              "LogFile"

 

#define MDO_CB_MIPSTART                                   3       
#define MDO_CB_MIPSOL                                     4       
#define MDO_CB_MIPNODE                                    5       

 

 

#define MDO_CB_PRE_NUMVARS                                200     
#define MDO_CB_PRE_NUMCONSTRS                             201     
#define MDO_CB_PRE_NUMNZS                                 202     

 

#define MDO_CB_MIP_OBJBST                                 300     
#define MDO_CB_MIP_OBJBND                                 301     
#define MDO_CB_MIP_RELVAL                                 302     

 

#define MDO_CB_MIP_NODCNT                                 306     
#define MDO_CB_MIP_SOLCNT                                 307     
#define MDO_CB_MIP_CUTCNT                                 308     
#define MDO_CB_MIP_NODLFT                                 309     
#define MDO_CB_MIP_NODEID                                 310     
#define MDO_CB_MIP_DEPTH                                  311     
#define MDO_CB_MIP_PHASE                                  312     

 

#define MDO_CB_MIP_SOL                                    316     
#define MDO_CB_MIP_REL                                    317     


typedef void                    MDOenv;      
typedef void                    MDOmodel;    

#define MDO_MAX_STRLEN     512

 

#define MDO_UNDEFINED                            -1.23457E+88

 

#define MDO_INFINITY                                  1.0E+20

 

#define MDO_GENCONSTR_INDICATOR                             6

 

#define MDO_CONTINUOUS                                    'C'
#define MDO_BINARY                                        'B'
#define MDO_INTEGER                                       'I'
#define MDO_SEMICONT                                      'S'
#define MDO_SEMIINT                                       'N'

 

#define MDO_EQUAL                                         '='
#define MDO_LESS_EQUAL                                    '<'
#define MDO_GREATER_EQUAL                                 '>'

 

#define MDO_MINIMIZE                                        1
#define MDO_MAXIMIZE                                       -1

 

#define MDO_SOS_TYPE1                                       1
#define MDO_SOS_TYPE2                                       2


#endif

