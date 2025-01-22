#ifndef MOSEK_H
#define MOSEK_H

/******************************************************************************
 ** Module : mosek.h
 **
 ** Generated 2025
 **
 ** Copyright (c) MOSEK ApS, Denmark.
 **
 ** All rights reserved
 **
 ******************************************************************************/
/*
 The content of this file is subject to copyright. However, it may free
 of charge be redistributed in identical form --- i.e. with no changes of
 the wording --- for any legitimate purpose.
*/


#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#define MSK_VERSION_MAJOR    11
#define MSK_VERSION_MINOR    0
#define MSK_VERSION_REVISION 4
#define MSK_VERSION_STATE    ""

#define MSK_INFINITY 1.0e30

/* BEGIN PLATFORM SPECIFIC DEFINITIONS (linux64x86) */
#define MSKAPI   
#define MSKAPIVA 
/* END   PLATFORM SPECIFIC DEFINITIONS (linux64x86) */


/* Enums and constants */
/* namespace mosek { */
enum MSKbasindtype_enum {
  MSK_BI_NEVER       = 0,
  MSK_BI_ALWAYS      = 1,
  MSK_BI_NO_ERROR    = 2,
  MSK_BI_IF_FEASIBLE = 3,
  MSK_BI_RESERVERED  = 4
};
#define MSK_BI_BEGIN MSK_BI_NEVER
#define MSK_BI_END   (1+MSK_BI_RESERVERED)


enum MSKboundkey_enum {
  MSK_BK_LO = 0,
  MSK_BK_UP = 1,
  MSK_BK_FX = 2,
  MSK_BK_FR = 3,
  MSK_BK_RA = 4
};
#define MSK_BK_BEGIN MSK_BK_LO
#define MSK_BK_END   (1+MSK_BK_RA)


enum MSKmark_enum {
  MSK_MARK_LO = 0,
  MSK_MARK_UP = 1
};
#define MSK_MARK_BEGIN MSK_MARK_LO
#define MSK_MARK_END   (1+MSK_MARK_UP)


enum MSKsimprecision_enum {
  MSK_SIM_PRECISION_NORMAL   = 0,
  MSK_SIM_PRECISION_EXTENDED = 1
};
#define MSK_SIM_PRECISION_BEGIN MSK_SIM_PRECISION_NORMAL
#define MSK_SIM_PRECISION_END   (1+MSK_SIM_PRECISION_EXTENDED)


enum MSKsimdegen_enum {
  MSK_SIM_DEGEN_NONE       = 0,
  MSK_SIM_DEGEN_FREE       = 1,
  MSK_SIM_DEGEN_AGGRESSIVE = 2,
  MSK_SIM_DEGEN_MODERATE   = 3,
  MSK_SIM_DEGEN_MINIMUM    = 4
};
#define MSK_SIM_DEGEN_BEGIN MSK_SIM_DEGEN_NONE
#define MSK_SIM_DEGEN_END   (1+MSK_SIM_DEGEN_MINIMUM)


enum MSKtranspose_enum {
  MSK_TRANSPOSE_NO  = 0,
  MSK_TRANSPOSE_YES = 1
};
#define MSK_TRANSPOSE_BEGIN MSK_TRANSPOSE_NO
#define MSK_TRANSPOSE_END   (1+MSK_TRANSPOSE_YES)


enum MSKuplo_enum {
  MSK_UPLO_LO = 0,
  MSK_UPLO_UP = 1
};
#define MSK_UPLO_BEGIN MSK_UPLO_LO
#define MSK_UPLO_END   (1+MSK_UPLO_UP)


enum MSKsimreform_enum {
  MSK_SIM_REFORMULATION_OFF        = 0,
  MSK_SIM_REFORMULATION_ON         = 1,
  MSK_SIM_REFORMULATION_FREE       = 2,
  MSK_SIM_REFORMULATION_AGGRESSIVE = 3
};
#define MSK_SIM_REFORMULATION_BEGIN MSK_SIM_REFORMULATION_OFF
#define MSK_SIM_REFORMULATION_END   (1+MSK_SIM_REFORMULATION_AGGRESSIVE)


enum MSKsimdupvec_enum {
  MSK_SIM_EXPLOIT_DUPVEC_OFF  = 0,
  MSK_SIM_EXPLOIT_DUPVEC_ON   = 1,
  MSK_SIM_EXPLOIT_DUPVEC_FREE = 2
};
#define MSK_SIM_EXPLOIT_DUPVEC_BEGIN MSK_SIM_EXPLOIT_DUPVEC_OFF
#define MSK_SIM_EXPLOIT_DUPVEC_END   (1+MSK_SIM_EXPLOIT_DUPVEC_FREE)


enum MSKsimhotstart_enum {
  MSK_SIM_HOTSTART_NONE        = 0,
  MSK_SIM_HOTSTART_FREE        = 1,
  MSK_SIM_HOTSTART_STATUS_KEYS = 2
};
#define MSK_SIM_HOTSTART_BEGIN MSK_SIM_HOTSTART_NONE
#define MSK_SIM_HOTSTART_END   (1+MSK_SIM_HOTSTART_STATUS_KEYS)


enum MSKintpnthotstart_enum {
  MSK_INTPNT_HOTSTART_NONE        = 0,
  MSK_INTPNT_HOTSTART_PRIMAL      = 1,
  MSK_INTPNT_HOTSTART_DUAL        = 2,
  MSK_INTPNT_HOTSTART_PRIMAL_DUAL = 3
};
#define MSK_INTPNT_HOTSTART_BEGIN MSK_INTPNT_HOTSTART_NONE
#define MSK_INTPNT_HOTSTART_END   (1+MSK_INTPNT_HOTSTART_PRIMAL_DUAL)


enum MSKcallbackcode_enum {
  MSK_CALLBACK_BEGIN_BI                    = 0,
  MSK_CALLBACK_BEGIN_CONIC                 = 1,
  MSK_CALLBACK_BEGIN_DUAL_BI               = 2,
  MSK_CALLBACK_BEGIN_DUAL_SENSITIVITY      = 3,
  MSK_CALLBACK_BEGIN_DUAL_SETUP_BI         = 4,
  MSK_CALLBACK_BEGIN_DUAL_SIMPLEX          = 5,
  MSK_CALLBACK_BEGIN_DUAL_SIMPLEX_BI       = 6,
  MSK_CALLBACK_BEGIN_FOLDING               = 7,
  MSK_CALLBACK_BEGIN_FOLDING_BI            = 8,
  MSK_CALLBACK_BEGIN_FOLDING_BI_DUAL       = 9,
  MSK_CALLBACK_BEGIN_FOLDING_BI_INITIALIZE = 10,
  MSK_CALLBACK_BEGIN_FOLDING_BI_OPTIMIZER  = 11,
  MSK_CALLBACK_BEGIN_FOLDING_BI_PRIMAL     = 12,
  MSK_CALLBACK_BEGIN_INFEAS_ANA            = 13,
  MSK_CALLBACK_BEGIN_INITIALIZE_BI         = 14,
  MSK_CALLBACK_BEGIN_INTPNT                = 15,
  MSK_CALLBACK_BEGIN_LICENSE_WAIT          = 16,
  MSK_CALLBACK_BEGIN_MIO                   = 17,
  MSK_CALLBACK_BEGIN_OPTIMIZE_BI           = 18,
  MSK_CALLBACK_BEGIN_OPTIMIZER             = 19,
  MSK_CALLBACK_BEGIN_PRESOLVE              = 20,
  MSK_CALLBACK_BEGIN_PRIMAL_BI             = 21,
  MSK_CALLBACK_BEGIN_PRIMAL_REPAIR         = 22,
  MSK_CALLBACK_BEGIN_PRIMAL_SENSITIVITY    = 23,
  MSK_CALLBACK_BEGIN_PRIMAL_SETUP_BI       = 24,
  MSK_CALLBACK_BEGIN_PRIMAL_SIMPLEX        = 25,
  MSK_CALLBACK_BEGIN_PRIMAL_SIMPLEX_BI     = 26,
  MSK_CALLBACK_BEGIN_QCQO_REFORMULATE      = 27,
  MSK_CALLBACK_BEGIN_READ                  = 28,
  MSK_CALLBACK_BEGIN_ROOT_CUTGEN           = 29,
  MSK_CALLBACK_BEGIN_SIMPLEX               = 30,
  MSK_CALLBACK_BEGIN_SOLVE_ROOT_RELAX      = 31,
  MSK_CALLBACK_BEGIN_TO_CONIC              = 32,
  MSK_CALLBACK_BEGIN_WRITE                 = 33,
  MSK_CALLBACK_CONIC                       = 34,
  MSK_CALLBACK_DECOMP_MIO                  = 35,
  MSK_CALLBACK_DUAL_SIMPLEX                = 36,
  MSK_CALLBACK_END_BI                      = 37,
  MSK_CALLBACK_END_CONIC                   = 38,
  MSK_CALLBACK_END_DUAL_BI                 = 39,
  MSK_CALLBACK_END_DUAL_SENSITIVITY        = 40,
  MSK_CALLBACK_END_DUAL_SETUP_BI           = 41,
  MSK_CALLBACK_END_DUAL_SIMPLEX            = 42,
  MSK_CALLBACK_END_DUAL_SIMPLEX_BI         = 43,
  MSK_CALLBACK_END_FOLDING                 = 44,
  MSK_CALLBACK_END_FOLDING_BI              = 45,
  MSK_CALLBACK_END_FOLDING_BI_DUAL         = 46,
  MSK_CALLBACK_END_FOLDING_BI_INITIALIZE   = 47,
  MSK_CALLBACK_END_FOLDING_BI_OPTIMIZER    = 48,
  MSK_CALLBACK_END_FOLDING_BI_PRIMAL       = 49,
  MSK_CALLBACK_END_INFEAS_ANA              = 50,
  MSK_CALLBACK_END_INITIALIZE_BI           = 51,
  MSK_CALLBACK_END_INTPNT                  = 52,
  MSK_CALLBACK_END_LICENSE_WAIT            = 53,
  MSK_CALLBACK_END_MIO                     = 54,
  MSK_CALLBACK_END_OPTIMIZE_BI             = 55,
  MSK_CALLBACK_END_OPTIMIZER               = 56,
  MSK_CALLBACK_END_PRESOLVE                = 57,
  MSK_CALLBACK_END_PRIMAL_BI               = 58,
  MSK_CALLBACK_END_PRIMAL_REPAIR           = 59,
  MSK_CALLBACK_END_PRIMAL_SENSITIVITY      = 60,
  MSK_CALLBACK_END_PRIMAL_SETUP_BI         = 61,
  MSK_CALLBACK_END_PRIMAL_SIMPLEX          = 62,
  MSK_CALLBACK_END_PRIMAL_SIMPLEX_BI       = 63,
  MSK_CALLBACK_END_QCQO_REFORMULATE        = 64,
  MSK_CALLBACK_END_READ                    = 65,
  MSK_CALLBACK_END_ROOT_CUTGEN             = 66,
  MSK_CALLBACK_END_SIMPLEX                 = 67,
  MSK_CALLBACK_END_SIMPLEX_BI              = 68,
  MSK_CALLBACK_END_SOLVE_ROOT_RELAX        = 69,
  MSK_CALLBACK_END_TO_CONIC                = 70,
  MSK_CALLBACK_END_WRITE                   = 71,
  MSK_CALLBACK_FOLDING_BI_DUAL             = 72,
  MSK_CALLBACK_FOLDING_BI_OPTIMIZER        = 73,
  MSK_CALLBACK_FOLDING_BI_PRIMAL           = 74,
  MSK_CALLBACK_HEARTBEAT                   = 75,
  MSK_CALLBACK_IM_DUAL_SENSIVITY           = 76,
  MSK_CALLBACK_IM_DUAL_SIMPLEX             = 77,
  MSK_CALLBACK_IM_LICENSE_WAIT             = 78,
  MSK_CALLBACK_IM_LU                       = 79,
  MSK_CALLBACK_IM_MIO                      = 80,
  MSK_CALLBACK_IM_MIO_DUAL_SIMPLEX         = 81,
  MSK_CALLBACK_IM_MIO_INTPNT               = 82,
  MSK_CALLBACK_IM_MIO_PRIMAL_SIMPLEX       = 83,
  MSK_CALLBACK_IM_ORDER                    = 84,
  MSK_CALLBACK_IM_PRIMAL_SENSIVITY         = 85,
  MSK_CALLBACK_IM_PRIMAL_SIMPLEX           = 86,
  MSK_CALLBACK_IM_READ                     = 87,
  MSK_CALLBACK_IM_ROOT_CUTGEN              = 88,
  MSK_CALLBACK_IM_SIMPLEX                  = 89,
  MSK_CALLBACK_INTPNT                      = 90,
  MSK_CALLBACK_NEW_INT_MIO                 = 91,
  MSK_CALLBACK_OPTIMIZE_BI                 = 92,
  MSK_CALLBACK_PRIMAL_SIMPLEX              = 93,
  MSK_CALLBACK_QO_REFORMULATE              = 94,
  MSK_CALLBACK_READ_OPF                    = 95,
  MSK_CALLBACK_READ_OPF_SECTION            = 96,
  MSK_CALLBACK_RESTART_MIO                 = 97,
  MSK_CALLBACK_SOLVING_REMOTE              = 98,
  MSK_CALLBACK_UPDATE_DUAL_BI              = 99,
  MSK_CALLBACK_UPDATE_DUAL_SIMPLEX         = 100,
  MSK_CALLBACK_UPDATE_DUAL_SIMPLEX_BI      = 101,
  MSK_CALLBACK_UPDATE_PRESOLVE             = 102,
  MSK_CALLBACK_UPDATE_PRIMAL_BI            = 103,
  MSK_CALLBACK_UPDATE_PRIMAL_SIMPLEX       = 104,
  MSK_CALLBACK_UPDATE_PRIMAL_SIMPLEX_BI    = 105,
  MSK_CALLBACK_UPDATE_SIMPLEX              = 106,
  MSK_CALLBACK_WRITE_OPF                   = 107
};
#define MSK_CALLBACK_BEGIN MSK_CALLBACK_BEGIN_BI
#define MSK_CALLBACK_END   (1+MSK_CALLBACK_WRITE_OPF)


enum MSKcompresstype_enum {
  MSK_COMPRESS_NONE = 0,
  MSK_COMPRESS_FREE = 1,
  MSK_COMPRESS_GZIP = 2,
  MSK_COMPRESS_ZSTD = 3
};
#define MSK_COMPRESS_BEGIN MSK_COMPRESS_NONE
#define MSK_COMPRESS_END   (1+MSK_COMPRESS_ZSTD)


enum MSKconetype_enum {
  MSK_CT_QUAD  = 0,
  MSK_CT_RQUAD = 1,
  MSK_CT_PEXP  = 2,
  MSK_CT_DEXP  = 3,
  MSK_CT_PPOW  = 4,
  MSK_CT_DPOW  = 5,
  MSK_CT_ZERO  = 6
};
#define MSK_CT_BEGIN MSK_CT_QUAD
#define MSK_CT_END   (1+MSK_CT_ZERO)


enum MSKdomaintype_enum {
  MSK_DOMAIN_R                    = 0,
  MSK_DOMAIN_RZERO                = 1,
  MSK_DOMAIN_RPLUS                = 2,
  MSK_DOMAIN_RMINUS               = 3,
  MSK_DOMAIN_QUADRATIC_CONE       = 4,
  MSK_DOMAIN_RQUADRATIC_CONE      = 5,
  MSK_DOMAIN_PRIMAL_EXP_CONE      = 6,
  MSK_DOMAIN_DUAL_EXP_CONE        = 7,
  MSK_DOMAIN_PRIMAL_POWER_CONE    = 8,
  MSK_DOMAIN_DUAL_POWER_CONE      = 9,
  MSK_DOMAIN_PRIMAL_GEO_MEAN_CONE = 10,
  MSK_DOMAIN_DUAL_GEO_MEAN_CONE   = 11,
  MSK_DOMAIN_SVEC_PSD_CONE        = 12
};
#define MSK_DOMAIN_BEGIN MSK_DOMAIN_R
#define MSK_DOMAIN_END   (1+MSK_DOMAIN_SVEC_PSD_CONE)


enum MSKnametype_enum {
  MSK_NAME_TYPE_GEN = 0,
  MSK_NAME_TYPE_MPS = 1,
  MSK_NAME_TYPE_LP  = 2
};
#define MSK_NAME_TYPE_BEGIN MSK_NAME_TYPE_GEN
#define MSK_NAME_TYPE_END   (1+MSK_NAME_TYPE_LP)


enum MSKsymmattype_enum {
  MSK_SYMMAT_TYPE_SPARSE = 0
};
#define MSK_SYMMAT_TYPE_BEGIN MSK_SYMMAT_TYPE_SPARSE
#define MSK_SYMMAT_TYPE_END   (1+MSK_SYMMAT_TYPE_SPARSE)


enum MSKdataformat_enum {
  MSK_DATA_FORMAT_EXTENSION = 0,
  MSK_DATA_FORMAT_MPS       = 1,
  MSK_DATA_FORMAT_LP        = 2,
  MSK_DATA_FORMAT_OP        = 3,
  MSK_DATA_FORMAT_FREE_MPS  = 4,
  MSK_DATA_FORMAT_TASK      = 5,
  MSK_DATA_FORMAT_PTF       = 6,
  MSK_DATA_FORMAT_CB        = 7,
  MSK_DATA_FORMAT_JSON_TASK = 8
};
#define MSK_DATA_FORMAT_BEGIN MSK_DATA_FORMAT_EXTENSION
#define MSK_DATA_FORMAT_END   (1+MSK_DATA_FORMAT_JSON_TASK)


enum MSKsolformat_enum {
  MSK_SOL_FORMAT_EXTENSION = 0,
  MSK_SOL_FORMAT_B         = 1,
  MSK_SOL_FORMAT_TASK      = 2,
  MSK_SOL_FORMAT_JSON_TASK = 3
};
#define MSK_SOL_FORMAT_BEGIN MSK_SOL_FORMAT_EXTENSION
#define MSK_SOL_FORMAT_END   (1+MSK_SOL_FORMAT_JSON_TASK)


enum MSKdinfitem_enum {
  MSK_DINF_ANA_PRO_SCALARIZED_CONSTRAINT_MATRIX_DENSITY   = 0,
  MSK_DINF_BI_CLEAN_TIME                                  = 1,
  MSK_DINF_BI_DUAL_TIME                                   = 2,
  MSK_DINF_BI_PRIMAL_TIME                                 = 3,
  MSK_DINF_BI_TIME                                        = 4,
  MSK_DINF_FOLDING_BI_OPTIMIZE_TIME                       = 5,
  MSK_DINF_FOLDING_BI_UNFOLD_DUAL_TIME                    = 6,
  MSK_DINF_FOLDING_BI_UNFOLD_INITIALIZE_TIME              = 7,
  MSK_DINF_FOLDING_BI_UNFOLD_PRIMAL_TIME                  = 8,
  MSK_DINF_FOLDING_BI_UNFOLD_TIME                         = 9,
  MSK_DINF_FOLDING_FACTOR                                 = 10,
  MSK_DINF_FOLDING_TIME                                   = 11,
  MSK_DINF_INTPNT_DUAL_FEAS                               = 12,
  MSK_DINF_INTPNT_DUAL_OBJ                                = 13,
  MSK_DINF_INTPNT_FACTOR_NUM_FLOPS                        = 14,
  MSK_DINF_INTPNT_OPT_STATUS                              = 15,
  MSK_DINF_INTPNT_ORDER_TIME                              = 16,
  MSK_DINF_INTPNT_PRIMAL_FEAS                             = 17,
  MSK_DINF_INTPNT_PRIMAL_OBJ                              = 18,
  MSK_DINF_INTPNT_TIME                                    = 19,
  MSK_DINF_MIO_CLIQUE_SELECTION_TIME                      = 20,
  MSK_DINF_MIO_CLIQUE_SEPARATION_TIME                     = 21,
  MSK_DINF_MIO_CMIR_SELECTION_TIME                        = 22,
  MSK_DINF_MIO_CMIR_SEPARATION_TIME                       = 23,
  MSK_DINF_MIO_CONSTRUCT_SOLUTION_OBJ                     = 24,
  MSK_DINF_MIO_DUAL_BOUND_AFTER_PRESOLVE                  = 25,
  MSK_DINF_MIO_GMI_SELECTION_TIME                         = 26,
  MSK_DINF_MIO_GMI_SEPARATION_TIME                        = 27,
  MSK_DINF_MIO_IMPLIED_BOUND_SELECTION_TIME               = 28,
  MSK_DINF_MIO_IMPLIED_BOUND_SEPARATION_TIME              = 29,
  MSK_DINF_MIO_INITIAL_FEASIBLE_SOLUTION_OBJ              = 30,
  MSK_DINF_MIO_KNAPSACK_COVER_SELECTION_TIME              = 31,
  MSK_DINF_MIO_KNAPSACK_COVER_SEPARATION_TIME             = 32,
  MSK_DINF_MIO_LIPRO_SELECTION_TIME                       = 33,
  MSK_DINF_MIO_LIPRO_SEPARATION_TIME                      = 34,
  MSK_DINF_MIO_OBJ_ABS_GAP                                = 35,
  MSK_DINF_MIO_OBJ_BOUND                                  = 36,
  MSK_DINF_MIO_OBJ_INT                                    = 37,
  MSK_DINF_MIO_OBJ_REL_GAP                                = 38,
  MSK_DINF_MIO_PROBING_TIME                               = 39,
  MSK_DINF_MIO_ROOT_CUT_SELECTION_TIME                    = 40,
  MSK_DINF_MIO_ROOT_CUT_SEPARATION_TIME                   = 41,
  MSK_DINF_MIO_ROOT_OPTIMIZER_TIME                        = 42,
  MSK_DINF_MIO_ROOT_PRESOLVE_TIME                         = 43,
  MSK_DINF_MIO_ROOT_TIME                                  = 44,
  MSK_DINF_MIO_SYMMETRY_DETECTION_TIME                    = 45,
  MSK_DINF_MIO_SYMMETRY_FACTOR                            = 46,
  MSK_DINF_MIO_TIME                                       = 47,
  MSK_DINF_MIO_USER_OBJ_CUT                               = 48,
  MSK_DINF_OPTIMIZER_TICKS                                = 49,
  MSK_DINF_OPTIMIZER_TIME                                 = 50,
  MSK_DINF_PRESOLVE_ELI_TIME                              = 51,
  MSK_DINF_PRESOLVE_LINDEP_TIME                           = 52,
  MSK_DINF_PRESOLVE_TIME                                  = 53,
  MSK_DINF_PRESOLVE_TOTAL_PRIMAL_PERTURBATION             = 54,
  MSK_DINF_PRIMAL_REPAIR_PENALTY_OBJ                      = 55,
  MSK_DINF_QCQO_REFORMULATE_MAX_PERTURBATION              = 56,
  MSK_DINF_QCQO_REFORMULATE_TIME                          = 57,
  MSK_DINF_QCQO_REFORMULATE_WORST_CHOLESKY_COLUMN_SCALING = 58,
  MSK_DINF_QCQO_REFORMULATE_WORST_CHOLESKY_DIAG_SCALING   = 59,
  MSK_DINF_READ_DATA_TIME                                 = 60,
  MSK_DINF_REMOTE_TIME                                    = 61,
  MSK_DINF_SIM_DUAL_TIME                                  = 62,
  MSK_DINF_SIM_FEAS                                       = 63,
  MSK_DINF_SIM_OBJ                                        = 64,
  MSK_DINF_SIM_PRIMAL_TIME                                = 65,
  MSK_DINF_SIM_TIME                                       = 66,
  MSK_DINF_SOL_BAS_DUAL_OBJ                               = 67,
  MSK_DINF_SOL_BAS_DVIOLCON                               = 68,
  MSK_DINF_SOL_BAS_DVIOLVAR                               = 69,
  MSK_DINF_SOL_BAS_NRM_BARX                               = 70,
  MSK_DINF_SOL_BAS_NRM_SLC                                = 71,
  MSK_DINF_SOL_BAS_NRM_SLX                                = 72,
  MSK_DINF_SOL_BAS_NRM_SUC                                = 73,
  MSK_DINF_SOL_BAS_NRM_SUX                                = 74,
  MSK_DINF_SOL_BAS_NRM_XC                                 = 75,
  MSK_DINF_SOL_BAS_NRM_XX                                 = 76,
  MSK_DINF_SOL_BAS_NRM_Y                                  = 77,
  MSK_DINF_SOL_BAS_PRIMAL_OBJ                             = 78,
  MSK_DINF_SOL_BAS_PVIOLCON                               = 79,
  MSK_DINF_SOL_BAS_PVIOLVAR                               = 80,
  MSK_DINF_SOL_ITG_NRM_BARX                               = 81,
  MSK_DINF_SOL_ITG_NRM_XC                                 = 82,
  MSK_DINF_SOL_ITG_NRM_XX                                 = 83,
  MSK_DINF_SOL_ITG_PRIMAL_OBJ                             = 84,
  MSK_DINF_SOL_ITG_PVIOLACC                               = 85,
  MSK_DINF_SOL_ITG_PVIOLBARVAR                            = 86,
  MSK_DINF_SOL_ITG_PVIOLCON                               = 87,
  MSK_DINF_SOL_ITG_PVIOLCONES                             = 88,
  MSK_DINF_SOL_ITG_PVIOLDJC                               = 89,
  MSK_DINF_SOL_ITG_PVIOLITG                               = 90,
  MSK_DINF_SOL_ITG_PVIOLVAR                               = 91,
  MSK_DINF_SOL_ITR_DUAL_OBJ                               = 92,
  MSK_DINF_SOL_ITR_DVIOLACC                               = 93,
  MSK_DINF_SOL_ITR_DVIOLBARVAR                            = 94,
  MSK_DINF_SOL_ITR_DVIOLCON                               = 95,
  MSK_DINF_SOL_ITR_DVIOLCONES                             = 96,
  MSK_DINF_SOL_ITR_DVIOLVAR                               = 97,
  MSK_DINF_SOL_ITR_NRM_BARS                               = 98,
  MSK_DINF_SOL_ITR_NRM_BARX                               = 99,
  MSK_DINF_SOL_ITR_NRM_SLC                                = 100,
  MSK_DINF_SOL_ITR_NRM_SLX                                = 101,
  MSK_DINF_SOL_ITR_NRM_SNX                                = 102,
  MSK_DINF_SOL_ITR_NRM_SUC                                = 103,
  MSK_DINF_SOL_ITR_NRM_SUX                                = 104,
  MSK_DINF_SOL_ITR_NRM_XC                                 = 105,
  MSK_DINF_SOL_ITR_NRM_XX                                 = 106,
  MSK_DINF_SOL_ITR_NRM_Y                                  = 107,
  MSK_DINF_SOL_ITR_PRIMAL_OBJ                             = 108,
  MSK_DINF_SOL_ITR_PVIOLACC                               = 109,
  MSK_DINF_SOL_ITR_PVIOLBARVAR                            = 110,
  MSK_DINF_SOL_ITR_PVIOLCON                               = 111,
  MSK_DINF_SOL_ITR_PVIOLCONES                             = 112,
  MSK_DINF_SOL_ITR_PVIOLVAR                               = 113,
  MSK_DINF_TO_CONIC_TIME                                  = 114,
  MSK_DINF_WRITE_DATA_TIME                                = 115
};
#define MSK_DINF_BEGIN MSK_DINF_ANA_PRO_SCALARIZED_CONSTRAINT_MATRIX_DENSITY
#define MSK_DINF_END   (1+MSK_DINF_WRITE_DATA_TIME)


enum MSKfeature_enum {
  MSK_FEATURE_PTS  = 0,
  MSK_FEATURE_PTON = 1
};
#define MSK_FEATURE_BEGIN MSK_FEATURE_PTS
#define MSK_FEATURE_END   (1+MSK_FEATURE_PTON)


enum MSKdparam_enum {
  MSK_DPAR_ANA_SOL_INFEAS_TOL                      = 0,
  MSK_DPAR_BASIS_REL_TOL_S                         = 1,
  MSK_DPAR_BASIS_TOL_S                             = 2,
  MSK_DPAR_BASIS_TOL_X                             = 3,
  MSK_DPAR_DATA_SYM_MAT_TOL                        = 4,
  MSK_DPAR_DATA_SYM_MAT_TOL_HUGE                   = 5,
  MSK_DPAR_DATA_SYM_MAT_TOL_LARGE                  = 6,
  MSK_DPAR_DATA_TOL_AIJ_HUGE                       = 7,
  MSK_DPAR_DATA_TOL_AIJ_LARGE                      = 8,
  MSK_DPAR_DATA_TOL_BOUND_INF                      = 9,
  MSK_DPAR_DATA_TOL_BOUND_WRN                      = 10,
  MSK_DPAR_DATA_TOL_C_HUGE                         = 11,
  MSK_DPAR_DATA_TOL_CJ_LARGE                       = 12,
  MSK_DPAR_DATA_TOL_QIJ                            = 13,
  MSK_DPAR_DATA_TOL_X                              = 14,
  MSK_DPAR_FOLDING_TOL_EQ                          = 15,
  MSK_DPAR_INTPNT_CO_TOL_DFEAS                     = 16,
  MSK_DPAR_INTPNT_CO_TOL_INFEAS                    = 17,
  MSK_DPAR_INTPNT_CO_TOL_MU_RED                    = 18,
  MSK_DPAR_INTPNT_CO_TOL_NEAR_REL                  = 19,
  MSK_DPAR_INTPNT_CO_TOL_PFEAS                     = 20,
  MSK_DPAR_INTPNT_CO_TOL_REL_GAP                   = 21,
  MSK_DPAR_INTPNT_QO_TOL_DFEAS                     = 22,
  MSK_DPAR_INTPNT_QO_TOL_INFEAS                    = 23,
  MSK_DPAR_INTPNT_QO_TOL_MU_RED                    = 24,
  MSK_DPAR_INTPNT_QO_TOL_NEAR_REL                  = 25,
  MSK_DPAR_INTPNT_QO_TOL_PFEAS                     = 26,
  MSK_DPAR_INTPNT_QO_TOL_REL_GAP                   = 27,
  MSK_DPAR_INTPNT_TOL_DFEAS                        = 28,
  MSK_DPAR_INTPNT_TOL_DSAFE                        = 29,
  MSK_DPAR_INTPNT_TOL_INFEAS                       = 30,
  MSK_DPAR_INTPNT_TOL_MU_RED                       = 31,
  MSK_DPAR_INTPNT_TOL_PATH                         = 32,
  MSK_DPAR_INTPNT_TOL_PFEAS                        = 33,
  MSK_DPAR_INTPNT_TOL_PSAFE                        = 34,
  MSK_DPAR_INTPNT_TOL_REL_GAP                      = 35,
  MSK_DPAR_INTPNT_TOL_REL_STEP                     = 36,
  MSK_DPAR_INTPNT_TOL_STEP_SIZE                    = 37,
  MSK_DPAR_LOWER_OBJ_CUT                           = 38,
  MSK_DPAR_LOWER_OBJ_CUT_FINITE_TRH                = 39,
  MSK_DPAR_MIO_CLIQUE_TABLE_SIZE_FACTOR            = 40,
  MSK_DPAR_MIO_DJC_MAX_BIGM                        = 41,
  MSK_DPAR_MIO_MAX_TIME                            = 42,
  MSK_DPAR_MIO_REL_GAP_CONST                       = 43,
  MSK_DPAR_MIO_TOL_ABS_GAP                         = 44,
  MSK_DPAR_MIO_TOL_ABS_RELAX_INT                   = 45,
  MSK_DPAR_MIO_TOL_FEAS                            = 46,
  MSK_DPAR_MIO_TOL_REL_DUAL_BOUND_IMPROVEMENT      = 47,
  MSK_DPAR_MIO_TOL_REL_GAP                         = 48,
  MSK_DPAR_OPTIMIZER_MAX_TICKS                     = 49,
  MSK_DPAR_OPTIMIZER_MAX_TIME                      = 50,
  MSK_DPAR_PRESOLVE_TOL_ABS_LINDEP                 = 51,
  MSK_DPAR_PRESOLVE_TOL_PRIMAL_INFEAS_PERTURBATION = 52,
  MSK_DPAR_PRESOLVE_TOL_REL_LINDEP                 = 53,
  MSK_DPAR_PRESOLVE_TOL_S                          = 54,
  MSK_DPAR_PRESOLVE_TOL_X                          = 55,
  MSK_DPAR_QCQO_REFORMULATE_REL_DROP_TOL           = 56,
  MSK_DPAR_SEMIDEFINITE_TOL_APPROX                 = 57,
  MSK_DPAR_SIM_LU_TOL_REL_PIV                      = 58,
  MSK_DPAR_SIM_PRECISION_SCALING_EXTENDED          = 59,
  MSK_DPAR_SIM_PRECISION_SCALING_NORMAL            = 60,
  MSK_DPAR_SIMPLEX_ABS_TOL_PIV                     = 61,
  MSK_DPAR_UPPER_OBJ_CUT                           = 62,
  MSK_DPAR_UPPER_OBJ_CUT_FINITE_TRH                = 63
};
#define MSK_DPAR_BEGIN MSK_DPAR_ANA_SOL_INFEAS_TOL
#define MSK_DPAR_END   (1+MSK_DPAR_UPPER_OBJ_CUT_FINITE_TRH)


enum MSKliinfitem_enum {
  MSK_LIINF_ANA_PRO_SCALARIZED_CONSTRAINT_MATRIX_NUM_COLUMNS = 0,
  MSK_LIINF_ANA_PRO_SCALARIZED_CONSTRAINT_MATRIX_NUM_NZ      = 1,
  MSK_LIINF_ANA_PRO_SCALARIZED_CONSTRAINT_MATRIX_NUM_ROWS    = 2,
  MSK_LIINF_BI_CLEAN_ITER                                    = 3,
  MSK_LIINF_BI_DUAL_ITER                                     = 4,
  MSK_LIINF_BI_PRIMAL_ITER                                   = 5,
  MSK_LIINF_FOLDING_BI_DUAL_ITER                             = 6,
  MSK_LIINF_FOLDING_BI_OPTIMIZER_ITER                        = 7,
  MSK_LIINF_FOLDING_BI_PRIMAL_ITER                           = 8,
  MSK_LIINF_INTPNT_FACTOR_NUM_NZ                             = 9,
  MSK_LIINF_MIO_ANZ                                          = 10,
  MSK_LIINF_MIO_FINAL_ANZ                                    = 11,
  MSK_LIINF_MIO_INTPNT_ITER                                  = 12,
  MSK_LIINF_MIO_NUM_DUAL_ILLPOSED_CER                        = 13,
  MSK_LIINF_MIO_NUM_PRIM_ILLPOSED_CER                        = 14,
  MSK_LIINF_MIO_PRESOLVED_ANZ                                = 15,
  MSK_LIINF_MIO_SIMPLEX_ITER                                 = 16,
  MSK_LIINF_RD_NUMACC                                        = 17,
  MSK_LIINF_RD_NUMANZ                                        = 18,
  MSK_LIINF_RD_NUMDJC                                        = 19,
  MSK_LIINF_RD_NUMQNZ                                        = 20,
  MSK_LIINF_SIMPLEX_ITER                                     = 21
};
#define MSK_LIINF_BEGIN MSK_LIINF_ANA_PRO_SCALARIZED_CONSTRAINT_MATRIX_NUM_COLUMNS
#define MSK_LIINF_END   (1+MSK_LIINF_SIMPLEX_ITER)


enum MSKiinfitem_enum {
  MSK_IINF_ANA_PRO_NUM_CON                       = 0,
  MSK_IINF_ANA_PRO_NUM_CON_EQ                    = 1,
  MSK_IINF_ANA_PRO_NUM_CON_FR                    = 2,
  MSK_IINF_ANA_PRO_NUM_CON_LO                    = 3,
  MSK_IINF_ANA_PRO_NUM_CON_RA                    = 4,
  MSK_IINF_ANA_PRO_NUM_CON_UP                    = 5,
  MSK_IINF_ANA_PRO_NUM_VAR                       = 6,
  MSK_IINF_ANA_PRO_NUM_VAR_BIN                   = 7,
  MSK_IINF_ANA_PRO_NUM_VAR_CONT                  = 8,
  MSK_IINF_ANA_PRO_NUM_VAR_EQ                    = 9,
  MSK_IINF_ANA_PRO_NUM_VAR_FR                    = 10,
  MSK_IINF_ANA_PRO_NUM_VAR_INT                   = 11,
  MSK_IINF_ANA_PRO_NUM_VAR_LO                    = 12,
  MSK_IINF_ANA_PRO_NUM_VAR_RA                    = 13,
  MSK_IINF_ANA_PRO_NUM_VAR_UP                    = 14,
  MSK_IINF_FOLDING_APPLIED                       = 15,
  MSK_IINF_INTPNT_FACTOR_DIM_DENSE               = 16,
  MSK_IINF_INTPNT_ITER                           = 17,
  MSK_IINF_INTPNT_NUM_THREADS                    = 18,
  MSK_IINF_INTPNT_SOLVE_DUAL                     = 19,
  MSK_IINF_MIO_ABSGAP_SATISFIED                  = 20,
  MSK_IINF_MIO_CLIQUE_TABLE_SIZE                 = 21,
  MSK_IINF_MIO_CONSTRUCT_SOLUTION                = 22,
  MSK_IINF_MIO_FINAL_NUMBIN                      = 23,
  MSK_IINF_MIO_FINAL_NUMBINCONEVAR               = 24,
  MSK_IINF_MIO_FINAL_NUMCON                      = 25,
  MSK_IINF_MIO_FINAL_NUMCONE                     = 26,
  MSK_IINF_MIO_FINAL_NUMCONEVAR                  = 27,
  MSK_IINF_MIO_FINAL_NUMCONT                     = 28,
  MSK_IINF_MIO_FINAL_NUMCONTCONEVAR              = 29,
  MSK_IINF_MIO_FINAL_NUMDEXPCONES                = 30,
  MSK_IINF_MIO_FINAL_NUMDJC                      = 31,
  MSK_IINF_MIO_FINAL_NUMDPOWCONES                = 32,
  MSK_IINF_MIO_FINAL_NUMINT                      = 33,
  MSK_IINF_MIO_FINAL_NUMINTCONEVAR               = 34,
  MSK_IINF_MIO_FINAL_NUMPEXPCONES                = 35,
  MSK_IINF_MIO_FINAL_NUMPPOWCONES                = 36,
  MSK_IINF_MIO_FINAL_NUMQCONES                   = 37,
  MSK_IINF_MIO_FINAL_NUMRQCONES                  = 38,
  MSK_IINF_MIO_FINAL_NUMVAR                      = 39,
  MSK_IINF_MIO_INITIAL_FEASIBLE_SOLUTION         = 40,
  MSK_IINF_MIO_NODE_DEPTH                        = 41,
  MSK_IINF_MIO_NUM_ACTIVE_NODES                  = 42,
  MSK_IINF_MIO_NUM_ACTIVE_ROOT_CUTS              = 43,
  MSK_IINF_MIO_NUM_BLOCKS_SOLVED_IN_BB           = 44,
  MSK_IINF_MIO_NUM_BLOCKS_SOLVED_IN_PRESOLVE     = 45,
  MSK_IINF_MIO_NUM_BRANCH                        = 46,
  MSK_IINF_MIO_NUM_INT_SOLUTIONS                 = 47,
  MSK_IINF_MIO_NUM_RELAX                         = 48,
  MSK_IINF_MIO_NUM_REPEATED_PRESOLVE             = 49,
  MSK_IINF_MIO_NUM_RESTARTS                      = 50,
  MSK_IINF_MIO_NUM_ROOT_CUT_ROUNDS               = 51,
  MSK_IINF_MIO_NUM_SELECTED_CLIQUE_CUTS          = 52,
  MSK_IINF_MIO_NUM_SELECTED_CMIR_CUTS            = 53,
  MSK_IINF_MIO_NUM_SELECTED_GOMORY_CUTS          = 54,
  MSK_IINF_MIO_NUM_SELECTED_IMPLIED_BOUND_CUTS   = 55,
  MSK_IINF_MIO_NUM_SELECTED_KNAPSACK_COVER_CUTS  = 56,
  MSK_IINF_MIO_NUM_SELECTED_LIPRO_CUTS           = 57,
  MSK_IINF_MIO_NUM_SEPARATED_CLIQUE_CUTS         = 58,
  MSK_IINF_MIO_NUM_SEPARATED_CMIR_CUTS           = 59,
  MSK_IINF_MIO_NUM_SEPARATED_GOMORY_CUTS         = 60,
  MSK_IINF_MIO_NUM_SEPARATED_IMPLIED_BOUND_CUTS  = 61,
  MSK_IINF_MIO_NUM_SEPARATED_KNAPSACK_COVER_CUTS = 62,
  MSK_IINF_MIO_NUM_SEPARATED_LIPRO_CUTS          = 63,
  MSK_IINF_MIO_NUM_SOLVED_NODES                  = 64,
  MSK_IINF_MIO_NUMBIN                            = 65,
  MSK_IINF_MIO_NUMBINCONEVAR                     = 66,
  MSK_IINF_MIO_NUMCON                            = 67,
  MSK_IINF_MIO_NUMCONE                           = 68,
  MSK_IINF_MIO_NUMCONEVAR                        = 69,
  MSK_IINF_MIO_NUMCONT                           = 70,
  MSK_IINF_MIO_NUMCONTCONEVAR                    = 71,
  MSK_IINF_MIO_NUMDEXPCONES                      = 72,
  MSK_IINF_MIO_NUMDJC                            = 73,
  MSK_IINF_MIO_NUMDPOWCONES                      = 74,
  MSK_IINF_MIO_NUMINT                            = 75,
  MSK_IINF_MIO_NUMINTCONEVAR                     = 76,
  MSK_IINF_MIO_NUMPEXPCONES                      = 77,
  MSK_IINF_MIO_NUMPPOWCONES                      = 78,
  MSK_IINF_MIO_NUMQCONES                         = 79,
  MSK_IINF_MIO_NUMRQCONES                        = 80,
  MSK_IINF_MIO_NUMVAR                            = 81,
  MSK_IINF_MIO_OBJ_BOUND_DEFINED                 = 82,
  MSK_IINF_MIO_PRESOLVED_NUMBIN                  = 83,
  MSK_IINF_MIO_PRESOLVED_NUMBINCONEVAR           = 84,
  MSK_IINF_MIO_PRESOLVED_NUMCON                  = 85,
  MSK_IINF_MIO_PRESOLVED_NUMCONE                 = 86,
  MSK_IINF_MIO_PRESOLVED_NUMCONEVAR              = 87,
  MSK_IINF_MIO_PRESOLVED_NUMCONT                 = 88,
  MSK_IINF_MIO_PRESOLVED_NUMCONTCONEVAR          = 89,
  MSK_IINF_MIO_PRESOLVED_NUMDEXPCONES            = 90,
  MSK_IINF_MIO_PRESOLVED_NUMDJC                  = 91,
  MSK_IINF_MIO_PRESOLVED_NUMDPOWCONES            = 92,
  MSK_IINF_MIO_PRESOLVED_NUMINT                  = 93,
  MSK_IINF_MIO_PRESOLVED_NUMINTCONEVAR           = 94,
  MSK_IINF_MIO_PRESOLVED_NUMPEXPCONES            = 95,
  MSK_IINF_MIO_PRESOLVED_NUMPPOWCONES            = 96,
  MSK_IINF_MIO_PRESOLVED_NUMQCONES               = 97,
  MSK_IINF_MIO_PRESOLVED_NUMRQCONES              = 98,
  MSK_IINF_MIO_PRESOLVED_NUMVAR                  = 99,
  MSK_IINF_MIO_RELGAP_SATISFIED                  = 100,
  MSK_IINF_MIO_TOTAL_NUM_SELECTED_CUTS           = 101,
  MSK_IINF_MIO_TOTAL_NUM_SEPARATED_CUTS          = 102,
  MSK_IINF_MIO_USER_OBJ_CUT                      = 103,
  MSK_IINF_OPT_NUMCON                            = 104,
  MSK_IINF_OPT_NUMVAR                            = 105,
  MSK_IINF_OPTIMIZE_RESPONSE                     = 106,
  MSK_IINF_PRESOLVE_NUM_PRIMAL_PERTURBATIONS     = 107,
  MSK_IINF_PURIFY_DUAL_SUCCESS                   = 108,
  MSK_IINF_PURIFY_PRIMAL_SUCCESS                 = 109,
  MSK_IINF_RD_NUMBARVAR                          = 110,
  MSK_IINF_RD_NUMCON                             = 111,
  MSK_IINF_RD_NUMCONE                            = 112,
  MSK_IINF_RD_NUMINTVAR                          = 113,
  MSK_IINF_RD_NUMQ                               = 114,
  MSK_IINF_RD_NUMVAR                             = 115,
  MSK_IINF_RD_PROTYPE                            = 116,
  MSK_IINF_SIM_DUAL_DEG_ITER                     = 117,
  MSK_IINF_SIM_DUAL_HOTSTART                     = 118,
  MSK_IINF_SIM_DUAL_HOTSTART_LU                  = 119,
  MSK_IINF_SIM_DUAL_INF_ITER                     = 120,
  MSK_IINF_SIM_DUAL_ITER                         = 121,
  MSK_IINF_SIM_NUMCON                            = 122,
  MSK_IINF_SIM_NUMVAR                            = 123,
  MSK_IINF_SIM_PRIMAL_DEG_ITER                   = 124,
  MSK_IINF_SIM_PRIMAL_HOTSTART                   = 125,
  MSK_IINF_SIM_PRIMAL_HOTSTART_LU                = 126,
  MSK_IINF_SIM_PRIMAL_INF_ITER                   = 127,
  MSK_IINF_SIM_PRIMAL_ITER                       = 128,
  MSK_IINF_SIM_SOLVE_DUAL                        = 129,
  MSK_IINF_SOL_BAS_PROSTA                        = 130,
  MSK_IINF_SOL_BAS_SOLSTA                        = 131,
  MSK_IINF_SOL_ITG_PROSTA                        = 132,
  MSK_IINF_SOL_ITG_SOLSTA                        = 133,
  MSK_IINF_SOL_ITR_PROSTA                        = 134,
  MSK_IINF_SOL_ITR_SOLSTA                        = 135,
  MSK_IINF_STO_NUM_A_REALLOC                     = 136
};
#define MSK_IINF_BEGIN MSK_IINF_ANA_PRO_NUM_CON
#define MSK_IINF_END   (1+MSK_IINF_STO_NUM_A_REALLOC)


enum MSKinftype_enum {
  MSK_INF_DOU_TYPE  = 0,
  MSK_INF_INT_TYPE  = 1,
  MSK_INF_LINT_TYPE = 2
};
#define MSK_INF_BEGIN MSK_INF_DOU_TYPE
#define MSK_INF_END   (1+MSK_INF_LINT_TYPE)


enum MSKiomode_enum {
  MSK_IOMODE_READ      = 0,
  MSK_IOMODE_WRITE     = 1,
  MSK_IOMODE_READWRITE = 2
};
#define MSK_IOMODE_BEGIN MSK_IOMODE_READ
#define MSK_IOMODE_END   (1+MSK_IOMODE_READWRITE)


enum MSKiparam_enum {
  MSK_IPAR_ANA_SOL_BASIS                      = 0,
  MSK_IPAR_ANA_SOL_PRINT_VIOLATED             = 1,
  MSK_IPAR_AUTO_SORT_A_BEFORE_OPT             = 2,
  MSK_IPAR_AUTO_UPDATE_SOL_INFO               = 3,
  MSK_IPAR_BASIS_SOLVE_USE_PLUS_ONE           = 4,
  MSK_IPAR_BI_CLEAN_OPTIMIZER                 = 5,
  MSK_IPAR_BI_IGNORE_MAX_ITER                 = 6,
  MSK_IPAR_BI_IGNORE_NUM_ERROR                = 7,
  MSK_IPAR_BI_MAX_ITERATIONS                  = 8,
  MSK_IPAR_CACHE_LICENSE                      = 9,
  MSK_IPAR_COMPRESS_STATFILE                  = 10,
  MSK_IPAR_FOLDING_USE                        = 11,
  MSK_IPAR_GETDUAL_CONVERT_LMIS               = 12,
  MSK_IPAR_HEARTBEAT_SIM_FREQ_TICKS           = 13,
  MSK_IPAR_INFEAS_GENERIC_NAMES               = 14,
  MSK_IPAR_INFEAS_REPORT_AUTO                 = 15,
  MSK_IPAR_INFEAS_REPORT_LEVEL                = 16,
  MSK_IPAR_INTPNT_BASIS                       = 17,
  MSK_IPAR_INTPNT_DIFF_STEP                   = 18,
  MSK_IPAR_INTPNT_HOTSTART                    = 19,
  MSK_IPAR_INTPNT_MAX_ITERATIONS              = 20,
  MSK_IPAR_INTPNT_MAX_NUM_COR                 = 21,
  MSK_IPAR_INTPNT_OFF_COL_TRH                 = 22,
  MSK_IPAR_INTPNT_ORDER_GP_NUM_SEEDS          = 23,
  MSK_IPAR_INTPNT_ORDER_METHOD                = 24,
  MSK_IPAR_INTPNT_REGULARIZATION_USE          = 25,
  MSK_IPAR_INTPNT_SCALING                     = 26,
  MSK_IPAR_INTPNT_SOLVE_FORM                  = 27,
  MSK_IPAR_INTPNT_STARTING_POINT              = 28,
  MSK_IPAR_LICENSE_DEBUG                      = 29,
  MSK_IPAR_LICENSE_PAUSE_TIME                 = 30,
  MSK_IPAR_LICENSE_SUPPRESS_EXPIRE_WRNS       = 31,
  MSK_IPAR_LICENSE_TRH_EXPIRY_WRN             = 32,
  MSK_IPAR_LICENSE_WAIT                       = 33,
  MSK_IPAR_LOG                                = 34,
  MSK_IPAR_LOG_ANA_PRO                        = 35,
  MSK_IPAR_LOG_BI                             = 36,
  MSK_IPAR_LOG_BI_FREQ                        = 37,
  MSK_IPAR_LOG_CUT_SECOND_OPT                 = 38,
  MSK_IPAR_LOG_EXPAND                         = 39,
  MSK_IPAR_LOG_FEAS_REPAIR                    = 40,
  MSK_IPAR_LOG_FILE                           = 41,
  MSK_IPAR_LOG_INCLUDE_SUMMARY                = 42,
  MSK_IPAR_LOG_INFEAS_ANA                     = 43,
  MSK_IPAR_LOG_INTPNT                         = 44,
  MSK_IPAR_LOG_LOCAL_INFO                     = 45,
  MSK_IPAR_LOG_MIO                            = 46,
  MSK_IPAR_LOG_MIO_FREQ                       = 47,
  MSK_IPAR_LOG_ORDER                          = 48,
  MSK_IPAR_LOG_PRESOLVE                       = 49,
  MSK_IPAR_LOG_SENSITIVITY                    = 50,
  MSK_IPAR_LOG_SENSITIVITY_OPT                = 51,
  MSK_IPAR_LOG_SIM                            = 52,
  MSK_IPAR_LOG_SIM_FREQ                       = 53,
  MSK_IPAR_LOG_SIM_FREQ_GIGA_TICKS            = 54,
  MSK_IPAR_LOG_STORAGE                        = 55,
  MSK_IPAR_MAX_NUM_WARNINGS                   = 56,
  MSK_IPAR_MIO_BRANCH_DIR                     = 57,
  MSK_IPAR_MIO_CONFLICT_ANALYSIS_LEVEL        = 58,
  MSK_IPAR_MIO_CONIC_OUTER_APPROXIMATION      = 59,
  MSK_IPAR_MIO_CONSTRUCT_SOL                  = 60,
  MSK_IPAR_MIO_CROSSOVER_MAX_NODES            = 61,
  MSK_IPAR_MIO_CUT_CLIQUE                     = 62,
  MSK_IPAR_MIO_CUT_CMIR                       = 63,
  MSK_IPAR_MIO_CUT_GMI                        = 64,
  MSK_IPAR_MIO_CUT_IMPLIED_BOUND              = 65,
  MSK_IPAR_MIO_CUT_KNAPSACK_COVER             = 66,
  MSK_IPAR_MIO_CUT_LIPRO                      = 67,
  MSK_IPAR_MIO_CUT_SELECTION_LEVEL            = 68,
  MSK_IPAR_MIO_DATA_PERMUTATION_METHOD        = 69,
  MSK_IPAR_MIO_DUAL_RAY_ANALYSIS_LEVEL        = 70,
  MSK_IPAR_MIO_FEASPUMP_LEVEL                 = 71,
  MSK_IPAR_MIO_HEURISTIC_LEVEL                = 72,
  MSK_IPAR_MIO_INDEPENDENT_BLOCK_LEVEL        = 73,
  MSK_IPAR_MIO_MAX_NUM_BRANCHES               = 74,
  MSK_IPAR_MIO_MAX_NUM_RELAXS                 = 75,
  MSK_IPAR_MIO_MAX_NUM_RESTARTS               = 76,
  MSK_IPAR_MIO_MAX_NUM_ROOT_CUT_ROUNDS        = 77,
  MSK_IPAR_MIO_MAX_NUM_SOLUTIONS              = 78,
  MSK_IPAR_MIO_MEMORY_EMPHASIS_LEVEL          = 79,
  MSK_IPAR_MIO_MIN_REL                        = 80,
  MSK_IPAR_MIO_MODE                           = 81,
  MSK_IPAR_MIO_NODE_OPTIMIZER                 = 82,
  MSK_IPAR_MIO_NODE_SELECTION                 = 83,
  MSK_IPAR_MIO_NUMERICAL_EMPHASIS_LEVEL       = 84,
  MSK_IPAR_MIO_OPT_FACE_MAX_NODES             = 85,
  MSK_IPAR_MIO_PERSPECTIVE_REFORMULATE        = 86,
  MSK_IPAR_MIO_PRESOLVE_AGGREGATOR_USE        = 87,
  MSK_IPAR_MIO_PROBING_LEVEL                  = 88,
  MSK_IPAR_MIO_PROPAGATE_OBJECTIVE_CONSTRAINT = 89,
  MSK_IPAR_MIO_QCQO_REFORMULATION_METHOD      = 90,
  MSK_IPAR_MIO_RENS_MAX_NODES                 = 91,
  MSK_IPAR_MIO_RINS_MAX_NODES                 = 92,
  MSK_IPAR_MIO_ROOT_OPTIMIZER                 = 93,
  MSK_IPAR_MIO_SEED                           = 94,
  MSK_IPAR_MIO_SYMMETRY_LEVEL                 = 95,
  MSK_IPAR_MIO_VAR_SELECTION                  = 96,
  MSK_IPAR_MIO_VB_DETECTION_LEVEL             = 97,
  MSK_IPAR_MT_SPINCOUNT                       = 98,
  MSK_IPAR_NG                                 = 99,
  MSK_IPAR_NUM_THREADS                        = 100,
  MSK_IPAR_OPF_WRITE_HEADER                   = 101,
  MSK_IPAR_OPF_WRITE_HINTS                    = 102,
  MSK_IPAR_OPF_WRITE_LINE_LENGTH              = 103,
  MSK_IPAR_OPF_WRITE_PARAMETERS               = 104,
  MSK_IPAR_OPF_WRITE_PROBLEM                  = 105,
  MSK_IPAR_OPF_WRITE_SOL_BAS                  = 106,
  MSK_IPAR_OPF_WRITE_SOL_ITG                  = 107,
  MSK_IPAR_OPF_WRITE_SOL_ITR                  = 108,
  MSK_IPAR_OPF_WRITE_SOLUTIONS                = 109,
  MSK_IPAR_OPTIMIZER                          = 110,
  MSK_IPAR_PARAM_READ_CASE_NAME               = 111,
  MSK_IPAR_PARAM_READ_IGN_ERROR               = 112,
  MSK_IPAR_PRESOLVE_ELIMINATOR_MAX_FILL       = 113,
  MSK_IPAR_PRESOLVE_ELIMINATOR_MAX_NUM_TRIES  = 114,
  MSK_IPAR_PRESOLVE_LINDEP_ABS_WORK_TRH       = 115,
  MSK_IPAR_PRESOLVE_LINDEP_NEW                = 116,
  MSK_IPAR_PRESOLVE_LINDEP_REL_WORK_TRH       = 117,
  MSK_IPAR_PRESOLVE_LINDEP_USE                = 118,
  MSK_IPAR_PRESOLVE_MAX_NUM_PASS              = 119,
  MSK_IPAR_PRESOLVE_MAX_NUM_REDUCTIONS        = 120,
  MSK_IPAR_PRESOLVE_USE                       = 121,
  MSK_IPAR_PRIMAL_REPAIR_OPTIMIZER            = 122,
  MSK_IPAR_PTF_WRITE_PARAMETERS               = 123,
  MSK_IPAR_PTF_WRITE_SINGLE_PSD_TERMS         = 124,
  MSK_IPAR_PTF_WRITE_SOLUTIONS                = 125,
  MSK_IPAR_PTF_WRITE_TRANSFORM                = 126,
  MSK_IPAR_READ_ASYNC                         = 127,
  MSK_IPAR_READ_DEBUG                         = 128,
  MSK_IPAR_READ_KEEP_FREE_CON                 = 129,
  MSK_IPAR_READ_MPS_FORMAT                    = 130,
  MSK_IPAR_READ_MPS_WIDTH                     = 131,
  MSK_IPAR_READ_TASK_IGNORE_PARAM             = 132,
  MSK_IPAR_REMOTE_USE_COMPRESSION             = 133,
  MSK_IPAR_REMOVE_UNUSED_SOLUTIONS            = 134,
  MSK_IPAR_SENSITIVITY_ALL                    = 135,
  MSK_IPAR_SENSITIVITY_TYPE                   = 136,
  MSK_IPAR_SIM_BASIS_FACTOR_USE               = 137,
  MSK_IPAR_SIM_DEGEN                          = 138,
  MSK_IPAR_SIM_DETECT_PWL                     = 139,
  MSK_IPAR_SIM_DUAL_CRASH                     = 140,
  MSK_IPAR_SIM_DUAL_PHASEONE_METHOD           = 141,
  MSK_IPAR_SIM_DUAL_RESTRICT_SELECTION        = 142,
  MSK_IPAR_SIM_DUAL_SELECTION                 = 143,
  MSK_IPAR_SIM_EXPLOIT_DUPVEC                 = 144,
  MSK_IPAR_SIM_HOTSTART                       = 145,
  MSK_IPAR_SIM_HOTSTART_LU                    = 146,
  MSK_IPAR_SIM_MAX_ITERATIONS                 = 147,
  MSK_IPAR_SIM_MAX_NUM_SETBACKS               = 148,
  MSK_IPAR_SIM_NON_SINGULAR                   = 149,
  MSK_IPAR_SIM_PRECISION                      = 150,
  MSK_IPAR_SIM_PRECISION_BOOST                = 151,
  MSK_IPAR_SIM_PRIMAL_CRASH                   = 152,
  MSK_IPAR_SIM_PRIMAL_PHASEONE_METHOD         = 153,
  MSK_IPAR_SIM_PRIMAL_RESTRICT_SELECTION      = 154,
  MSK_IPAR_SIM_PRIMAL_SELECTION               = 155,
  MSK_IPAR_SIM_REFACTOR_FREQ                  = 156,
  MSK_IPAR_SIM_REFORMULATION                  = 157,
  MSK_IPAR_SIM_SAVE_LU                        = 158,
  MSK_IPAR_SIM_SCALING                        = 159,
  MSK_IPAR_SIM_SCALING_METHOD                 = 160,
  MSK_IPAR_SIM_SEED                           = 161,
  MSK_IPAR_SIM_SOLVE_FORM                     = 162,
  MSK_IPAR_SIM_SWITCH_OPTIMIZER               = 163,
  MSK_IPAR_SOL_FILTER_KEEP_BASIC              = 164,
  MSK_IPAR_SOL_READ_NAME_WIDTH                = 165,
  MSK_IPAR_SOL_READ_WIDTH                     = 166,
  MSK_IPAR_TIMING_LEVEL                       = 167,
  MSK_IPAR_WRITE_ASYNC                        = 168,
  MSK_IPAR_WRITE_BAS_CONSTRAINTS              = 169,
  MSK_IPAR_WRITE_BAS_HEAD                     = 170,
  MSK_IPAR_WRITE_BAS_VARIABLES                = 171,
  MSK_IPAR_WRITE_COMPRESSION                  = 172,
  MSK_IPAR_WRITE_FREE_CON                     = 173,
  MSK_IPAR_WRITE_GENERIC_NAMES                = 174,
  MSK_IPAR_WRITE_IGNORE_INCOMPATIBLE_ITEMS    = 175,
  MSK_IPAR_WRITE_INT_CONSTRAINTS              = 176,
  MSK_IPAR_WRITE_INT_HEAD                     = 177,
  MSK_IPAR_WRITE_INT_VARIABLES                = 178,
  MSK_IPAR_WRITE_JSON_INDENTATION             = 179,
  MSK_IPAR_WRITE_LP_FULL_OBJ                  = 180,
  MSK_IPAR_WRITE_LP_LINE_WIDTH                = 181,
  MSK_IPAR_WRITE_MPS_FORMAT                   = 182,
  MSK_IPAR_WRITE_MPS_INT                      = 183,
  MSK_IPAR_WRITE_SOL_BARVARIABLES             = 184,
  MSK_IPAR_WRITE_SOL_CONSTRAINTS              = 185,
  MSK_IPAR_WRITE_SOL_HEAD                     = 186,
  MSK_IPAR_WRITE_SOL_IGNORE_INVALID_NAMES     = 187,
  MSK_IPAR_WRITE_SOL_VARIABLES                = 188
};
#define MSK_IPAR_BEGIN MSK_IPAR_ANA_SOL_BASIS
#define MSK_IPAR_END   (1+MSK_IPAR_WRITE_SOL_VARIABLES)


enum MSKbranchdir_enum {
  MSK_BRANCH_DIR_FREE       = 0,
  MSK_BRANCH_DIR_UP         = 1,
  MSK_BRANCH_DIR_DOWN       = 2,
  MSK_BRANCH_DIR_NEAR       = 3,
  MSK_BRANCH_DIR_FAR        = 4,
  MSK_BRANCH_DIR_ROOT_LP    = 5,
  MSK_BRANCH_DIR_GUIDED     = 6,
  MSK_BRANCH_DIR_PSEUDOCOST = 7
};
#define MSK_BRANCH_DIR_BEGIN MSK_BRANCH_DIR_FREE
#define MSK_BRANCH_DIR_END   (1+MSK_BRANCH_DIR_PSEUDOCOST)


enum MSKmiqcqoreformmethod_enum {
  MSK_MIO_QCQO_REFORMULATION_METHOD_FREE             = 0,
  MSK_MIO_QCQO_REFORMULATION_METHOD_NONE             = 1,
  MSK_MIO_QCQO_REFORMULATION_METHOD_LINEARIZATION    = 2,
  MSK_MIO_QCQO_REFORMULATION_METHOD_EIGEN_VAL_METHOD = 3,
  MSK_MIO_QCQO_REFORMULATION_METHOD_DIAG_SDP         = 4,
  MSK_MIO_QCQO_REFORMULATION_METHOD_RELAX_SDP        = 5
};
#define MSK_MIO_QCQO_REFORMULATION_METHOD_BEGIN MSK_MIO_QCQO_REFORMULATION_METHOD_FREE
#define MSK_MIO_QCQO_REFORMULATION_METHOD_END   (1+MSK_MIO_QCQO_REFORMULATION_METHOD_RELAX_SDP)


enum MSKmiodatapermmethod_enum {
  MSK_MIO_DATA_PERMUTATION_METHOD_NONE         = 0,
  MSK_MIO_DATA_PERMUTATION_METHOD_CYCLIC_SHIFT = 1,
  MSK_MIO_DATA_PERMUTATION_METHOD_RANDOM       = 2
};
#define MSK_MIO_DATA_PERMUTATION_METHOD_BEGIN MSK_MIO_DATA_PERMUTATION_METHOD_NONE
#define MSK_MIO_DATA_PERMUTATION_METHOD_END   (1+MSK_MIO_DATA_PERMUTATION_METHOD_RANDOM)


enum MSKmiocontsoltype_enum {
  MSK_MIO_CONT_SOL_NONE    = 0,
  MSK_MIO_CONT_SOL_ROOT    = 1,
  MSK_MIO_CONT_SOL_ITG     = 2,
  MSK_MIO_CONT_SOL_ITG_REL = 3
};
#define MSK_MIO_CONT_SOL_BEGIN MSK_MIO_CONT_SOL_NONE
#define MSK_MIO_CONT_SOL_END   (1+MSK_MIO_CONT_SOL_ITG_REL)


enum MSKmiomode_enum {
  MSK_MIO_MODE_IGNORED   = 0,
  MSK_MIO_MODE_SATISFIED = 1
};
#define MSK_MIO_MODE_BEGIN MSK_MIO_MODE_IGNORED
#define MSK_MIO_MODE_END   (1+MSK_MIO_MODE_SATISFIED)


enum MSKmionodeseltype_enum {
  MSK_MIO_NODE_SELECTION_FREE   = 0,
  MSK_MIO_NODE_SELECTION_FIRST  = 1,
  MSK_MIO_NODE_SELECTION_BEST   = 2,
  MSK_MIO_NODE_SELECTION_PSEUDO = 3
};
#define MSK_MIO_NODE_SELECTION_BEGIN MSK_MIO_NODE_SELECTION_FREE
#define MSK_MIO_NODE_SELECTION_END   (1+MSK_MIO_NODE_SELECTION_PSEUDO)


enum MSKmiovarseltype_enum {
  MSK_MIO_VAR_SELECTION_FREE       = 0,
  MSK_MIO_VAR_SELECTION_PSEUDOCOST = 1,
  MSK_MIO_VAR_SELECTION_STRONG     = 2
};
#define MSK_MIO_VAR_SELECTION_BEGIN MSK_MIO_VAR_SELECTION_FREE
#define MSK_MIO_VAR_SELECTION_END   (1+MSK_MIO_VAR_SELECTION_STRONG)


enum MSKmpsformat_enum {
  MSK_MPS_FORMAT_STRICT  = 0,
  MSK_MPS_FORMAT_RELAXED = 1,
  MSK_MPS_FORMAT_FREE    = 2,
  MSK_MPS_FORMAT_CPLEX   = 3
};
#define MSK_MPS_FORMAT_BEGIN MSK_MPS_FORMAT_STRICT
#define MSK_MPS_FORMAT_END   (1+MSK_MPS_FORMAT_CPLEX)


enum MSKobjsense_enum {
  MSK_OBJECTIVE_SENSE_MINIMIZE = 0,
  MSK_OBJECTIVE_SENSE_MAXIMIZE = 1
};
#define MSK_OBJECTIVE_SENSE_BEGIN MSK_OBJECTIVE_SENSE_MINIMIZE
#define MSK_OBJECTIVE_SENSE_END   (1+MSK_OBJECTIVE_SENSE_MAXIMIZE)


enum MSKonoffkey_enum {
  MSK_OFF = 0,
  MSK_ON  = 1
};
#define MSK_BEGIN MSK_OFF
#define MSK_END   (1+MSK_ON)


enum MSKoptimizertype_enum {
  MSK_OPTIMIZER_CONIC              = 0,
  MSK_OPTIMIZER_DUAL_SIMPLEX       = 1,
  MSK_OPTIMIZER_FREE               = 2,
  MSK_OPTIMIZER_FREE_SIMPLEX       = 3,
  MSK_OPTIMIZER_INTPNT             = 4,
  MSK_OPTIMIZER_MIXED_INT          = 5,
  MSK_OPTIMIZER_NEW_DUAL_SIMPLEX   = 6,
  MSK_OPTIMIZER_NEW_PRIMAL_SIMPLEX = 7,
  MSK_OPTIMIZER_PRIMAL_SIMPLEX     = 8
};
#define MSK_OPTIMIZER_BEGIN MSK_OPTIMIZER_CONIC
#define MSK_OPTIMIZER_END   (1+MSK_OPTIMIZER_PRIMAL_SIMPLEX)


enum MSKorderingtype_enum {
  MSK_ORDER_METHOD_FREE           = 0,
  MSK_ORDER_METHOD_APPMINLOC      = 1,
  MSK_ORDER_METHOD_EXPERIMENTAL   = 2,
  MSK_ORDER_METHOD_TRY_GRAPHPAR   = 3,
  MSK_ORDER_METHOD_FORCE_GRAPHPAR = 4,
  MSK_ORDER_METHOD_NONE           = 5
};
#define MSK_ORDER_METHOD_BEGIN MSK_ORDER_METHOD_FREE
#define MSK_ORDER_METHOD_END   (1+MSK_ORDER_METHOD_NONE)


enum MSKpresolvemode_enum {
  MSK_PRESOLVE_MODE_OFF  = 0,
  MSK_PRESOLVE_MODE_ON   = 1,
  MSK_PRESOLVE_MODE_FREE = 2
};
#define MSK_PRESOLVE_MODE_BEGIN MSK_PRESOLVE_MODE_OFF
#define MSK_PRESOLVE_MODE_END   (1+MSK_PRESOLVE_MODE_FREE)


enum MSKfoldingmode_enum {
  MSK_FOLDING_MODE_OFF               = 0,
  MSK_FOLDING_MODE_FREE              = 1,
  MSK_FOLDING_MODE_FREE_UNLESS_BASIC = 2,
  MSK_FOLDING_MODE_FORCE             = 3
};
#define MSK_FOLDING_MODE_BEGIN MSK_FOLDING_MODE_OFF
#define MSK_FOLDING_MODE_END   (1+MSK_FOLDING_MODE_FORCE)


enum MSKparametertype_enum {
  MSK_PAR_INVALID_TYPE = 0,
  MSK_PAR_DOU_TYPE     = 1,
  MSK_PAR_INT_TYPE     = 2,
  MSK_PAR_STR_TYPE     = 3
};
#define MSK_PAR_BEGIN MSK_PAR_INVALID_TYPE
#define MSK_PAR_END   (1+MSK_PAR_STR_TYPE)


enum MSKproblemitem_enum {
  MSK_PI_VAR  = 0,
  MSK_PI_CON  = 1,
  MSK_PI_CONE = 2
};
#define MSK_PI_BEGIN MSK_PI_VAR
#define MSK_PI_END   (1+MSK_PI_CONE)


enum MSKproblemtype_enum {
  MSK_PROBTYPE_LO    = 0,
  MSK_PROBTYPE_QO    = 1,
  MSK_PROBTYPE_QCQO  = 2,
  MSK_PROBTYPE_CONIC = 3,
  MSK_PROBTYPE_MIXED = 4
};
#define MSK_PROBTYPE_BEGIN MSK_PROBTYPE_LO
#define MSK_PROBTYPE_END   (1+MSK_PROBTYPE_MIXED)


enum MSKprosta_enum {
  MSK_PRO_STA_UNKNOWN                  = 0,
  MSK_PRO_STA_PRIM_AND_DUAL_FEAS       = 1,
  MSK_PRO_STA_PRIM_FEAS                = 2,
  MSK_PRO_STA_DUAL_FEAS                = 3,
  MSK_PRO_STA_PRIM_INFEAS              = 4,
  MSK_PRO_STA_DUAL_INFEAS              = 5,
  MSK_PRO_STA_PRIM_AND_DUAL_INFEAS     = 6,
  MSK_PRO_STA_ILL_POSED                = 7,
  MSK_PRO_STA_PRIM_INFEAS_OR_UNBOUNDED = 8
};
#define MSK_PRO_STA_BEGIN MSK_PRO_STA_UNKNOWN
#define MSK_PRO_STA_END   (1+MSK_PRO_STA_PRIM_INFEAS_OR_UNBOUNDED)


enum MSKrescode_enum {
  MSK_RES_OK                                                   = 0,
  MSK_RES_WRN_OPEN_PARAM_FILE                                  = 50,
  MSK_RES_WRN_LARGE_BOUND                                      = 51,
  MSK_RES_WRN_LARGE_LO_BOUND                                   = 52,
  MSK_RES_WRN_LARGE_UP_BOUND                                   = 53,
  MSK_RES_WRN_LARGE_CON_FX                                     = 54,
  MSK_RES_WRN_LARGE_CJ                                         = 57,
  MSK_RES_WRN_LARGE_AIJ                                        = 62,
  MSK_RES_WRN_ZERO_AIJ                                         = 63,
  MSK_RES_WRN_NAME_MAX_LEN                                     = 65,
  MSK_RES_WRN_SPAR_MAX_LEN                                     = 66,
  MSK_RES_WRN_MPS_SPLIT_RHS_VECTOR                             = 70,
  MSK_RES_WRN_MPS_SPLIT_RAN_VECTOR                             = 71,
  MSK_RES_WRN_MPS_SPLIT_BOU_VECTOR                             = 72,
  MSK_RES_WRN_LP_OLD_QUAD_FORMAT                               = 80,
  MSK_RES_WRN_LP_DROP_VARIABLE                                 = 85,
  MSK_RES_WRN_NZ_IN_UPR_TRI                                    = 200,
  MSK_RES_WRN_DROPPED_NZ_QOBJ                                  = 201,
  MSK_RES_WRN_IGNORE_INTEGER                                   = 250,
  MSK_RES_WRN_NO_GLOBAL_OPTIMIZER                              = 251,
  MSK_RES_WRN_MIO_INFEASIBLE_FINAL                             = 270,
  MSK_RES_WRN_SOL_FILTER                                       = 300,
  MSK_RES_WRN_UNDEF_SOL_FILE_NAME                              = 350,
  MSK_RES_WRN_SOL_FILE_IGNORED_CON                             = 351,
  MSK_RES_WRN_SOL_FILE_IGNORED_VAR                             = 352,
  MSK_RES_WRN_TOO_FEW_BASIS_VARS                               = 400,
  MSK_RES_WRN_TOO_MANY_BASIS_VARS                              = 405,
  MSK_RES_WRN_LICENSE_EXPIRE                                   = 500,
  MSK_RES_WRN_LICENSE_SERVER                                   = 501,
  MSK_RES_WRN_EMPTY_NAME                                       = 502,
  MSK_RES_WRN_USING_GENERIC_NAMES                              = 503,
  MSK_RES_WRN_INVALID_MPS_NAME                                 = 504,
  MSK_RES_WRN_INVALID_MPS_OBJ_NAME                             = 505,
  MSK_RES_WRN_LICENSE_FEATURE_EXPIRE                           = 509,
  MSK_RES_WRN_PARAM_NAME_DOU                                   = 510,
  MSK_RES_WRN_PARAM_NAME_INT                                   = 511,
  MSK_RES_WRN_PARAM_NAME_STR                                   = 512,
  MSK_RES_WRN_PARAM_STR_VALUE                                  = 515,
  MSK_RES_WRN_PARAM_IGNORED_CMIO                               = 516,
  MSK_RES_WRN_ZEROS_IN_SPARSE_ROW                              = 705,
  MSK_RES_WRN_ZEROS_IN_SPARSE_COL                              = 710,
  MSK_RES_WRN_INCOMPLETE_LINEAR_DEPENDENCY_CHECK               = 800,
  MSK_RES_WRN_ELIMINATOR_SPACE                                 = 801,
  MSK_RES_WRN_PRESOLVE_OUTOFSPACE                              = 802,
  MSK_RES_WRN_PRESOLVE_PRIMAL_PERTURBATIONS                    = 803,
  MSK_RES_WRN_WRITE_CHANGED_NAMES                              = 830,
  MSK_RES_WRN_WRITE_DISCARDED_CFIX                             = 831,
  MSK_RES_WRN_DUPLICATE_CONSTRAINT_NAMES                       = 850,
  MSK_RES_WRN_DUPLICATE_VARIABLE_NAMES                         = 851,
  MSK_RES_WRN_DUPLICATE_BARVARIABLE_NAMES                      = 852,
  MSK_RES_WRN_DUPLICATE_CONE_NAMES                             = 853,
  MSK_RES_WRN_ANA_LARGE_BOUNDS                                 = 900,
  MSK_RES_WRN_ANA_C_ZERO                                       = 901,
  MSK_RES_WRN_ANA_EMPTY_COLS                                   = 902,
  MSK_RES_WRN_ANA_CLOSE_BOUNDS                                 = 903,
  MSK_RES_WRN_ANA_ALMOST_INT_BOUNDS                            = 904,
  MSK_RES_WRN_NO_INFEASIBILITY_REPORT_WHEN_MATRIX_VARIABLES    = 930,
  MSK_RES_WRN_GETDUAL_IGNORES_INTEGRALITY                      = 940,
  MSK_RES_WRN_NO_DUALIZER                                      = 950,
  MSK_RES_WRN_SYM_MAT_LARGE                                    = 960,
  MSK_RES_WRN_MODIFIED_DOUBLE_PARAMETER                        = 970,
  MSK_RES_WRN_LARGE_FIJ                                        = 980,
  MSK_RES_WRN_PTF_UNKNOWN_SECTION                              = 981,
  MSK_RES_ERR_LICENSE                                          = 1000,
  MSK_RES_ERR_LICENSE_EXPIRED                                  = 1001,
  MSK_RES_ERR_LICENSE_VERSION                                  = 1002,
  MSK_RES_ERR_LICENSE_OLD_SERVER_VERSION                       = 1003,
  MSK_RES_ERR_SIZE_LICENSE                                     = 1005,
  MSK_RES_ERR_PROB_LICENSE                                     = 1006,
  MSK_RES_ERR_FILE_LICENSE                                     = 1007,
  MSK_RES_ERR_MISSING_LICENSE_FILE                             = 1008,
  MSK_RES_ERR_SIZE_LICENSE_CON                                 = 1010,
  MSK_RES_ERR_SIZE_LICENSE_VAR                                 = 1011,
  MSK_RES_ERR_SIZE_LICENSE_INTVAR                              = 1012,
  MSK_RES_ERR_OPTIMIZER_LICENSE                                = 1013,
  MSK_RES_ERR_FLEXLM                                           = 1014,
  MSK_RES_ERR_LICENSE_SERVER                                   = 1015,
  MSK_RES_ERR_LICENSE_MAX                                      = 1016,
  MSK_RES_ERR_LICENSE_MOSEKLM_DAEMON                           = 1017,
  MSK_RES_ERR_LICENSE_FEATURE                                  = 1018,
  MSK_RES_ERR_PLATFORM_NOT_LICENSED                            = 1019,
  MSK_RES_ERR_LICENSE_CANNOT_ALLOCATE                          = 1020,
  MSK_RES_ERR_LICENSE_CANNOT_CONNECT                           = 1021,
  MSK_RES_ERR_LICENSE_INVALID_HOSTID                           = 1025,
  MSK_RES_ERR_LICENSE_SERVER_VERSION                           = 1026,
  MSK_RES_ERR_LICENSE_NO_SERVER_SUPPORT                        = 1027,
  MSK_RES_ERR_LICENSE_NO_SERVER_LINE                           = 1028,
  MSK_RES_ERR_OLDER_DLL                                        = 1035,
  MSK_RES_ERR_NEWER_DLL                                        = 1036,
  MSK_RES_ERR_LINK_FILE_DLL                                    = 1040,
  MSK_RES_ERR_THREAD_MUTEX_INIT                                = 1045,
  MSK_RES_ERR_THREAD_MUTEX_LOCK                                = 1046,
  MSK_RES_ERR_THREAD_MUTEX_UNLOCK                              = 1047,
  MSK_RES_ERR_THREAD_CREATE                                    = 1048,
  MSK_RES_ERR_THREAD_COND_INIT                                 = 1049,
  MSK_RES_ERR_UNKNOWN                                          = 1050,
  MSK_RES_ERR_SPACE                                            = 1051,
  MSK_RES_ERR_FILE_OPEN                                        = 1052,
  MSK_RES_ERR_FILE_READ                                        = 1053,
  MSK_RES_ERR_FILE_WRITE                                       = 1054,
  MSK_RES_ERR_DATA_FILE_EXT                                    = 1055,
  MSK_RES_ERR_INVALID_FILE_NAME                                = 1056,
  MSK_RES_ERR_INVALID_SOL_FILE_NAME                            = 1057,
  MSK_RES_ERR_END_OF_FILE                                      = 1059,
  MSK_RES_ERR_NULL_ENV                                         = 1060,
  MSK_RES_ERR_NULL_TASK                                        = 1061,
  MSK_RES_ERR_INVALID_STREAM                                   = 1062,
  MSK_RES_ERR_NO_INIT_ENV                                      = 1063,
  MSK_RES_ERR_INVALID_TASK                                     = 1064,
  MSK_RES_ERR_NULL_POINTER                                     = 1065,
  MSK_RES_ERR_LIVING_TASKS                                     = 1066,
  MSK_RES_ERR_READ_GZIP                                        = 1067,
  MSK_RES_ERR_READ_ZSTD                                        = 1068,
  MSK_RES_ERR_READ_ASYNC                                       = 1069,
  MSK_RES_ERR_BLANK_NAME                                       = 1070,
  MSK_RES_ERR_DUP_NAME                                         = 1071,
  MSK_RES_ERR_FORMAT_STRING                                    = 1072,
  MSK_RES_ERR_SPARSITY_SPECIFICATION                           = 1073,
  MSK_RES_ERR_MISMATCHING_DIMENSION                            = 1074,
  MSK_RES_ERR_INVALID_OBJ_NAME                                 = 1075,
  MSK_RES_ERR_INVALID_CON_NAME                                 = 1076,
  MSK_RES_ERR_INVALID_VAR_NAME                                 = 1077,
  MSK_RES_ERR_INVALID_CONE_NAME                                = 1078,
  MSK_RES_ERR_INVALID_BARVAR_NAME                              = 1079,
  MSK_RES_ERR_SPACE_LEAKING                                    = 1080,
  MSK_RES_ERR_SPACE_NO_INFO                                    = 1081,
  MSK_RES_ERR_DIMENSION_SPECIFICATION                          = 1082,
  MSK_RES_ERR_AXIS_NAME_SPECIFICATION                          = 1083,
  MSK_RES_ERR_READ_PREMATURE_EOF                               = 1089,
  MSK_RES_ERR_READ_FORMAT                                      = 1090,
  MSK_RES_ERR_WRITE_LP_INVALID_VAR_NAMES                       = 1091,
  MSK_RES_ERR_WRITE_LP_DUPLICATE_VAR_NAMES                     = 1092,
  MSK_RES_ERR_WRITE_LP_INVALID_CON_NAMES                       = 1093,
  MSK_RES_ERR_WRITE_LP_DUPLICATE_CON_NAMES                     = 1094,
  MSK_RES_ERR_MPS_FILE                                         = 1100,
  MSK_RES_ERR_MPS_INV_FIELD                                    = 1101,
  MSK_RES_ERR_MPS_INV_MARKER                                   = 1102,
  MSK_RES_ERR_MPS_NULL_CON_NAME                                = 1103,
  MSK_RES_ERR_MPS_NULL_VAR_NAME                                = 1104,
  MSK_RES_ERR_MPS_UNDEF_CON_NAME                               = 1105,
  MSK_RES_ERR_MPS_UNDEF_VAR_NAME                               = 1106,
  MSK_RES_ERR_MPS_INVALID_CON_KEY                              = 1107,
  MSK_RES_ERR_MPS_INVALID_BOUND_KEY                            = 1108,
  MSK_RES_ERR_MPS_INVALID_SEC_NAME                             = 1109,
  MSK_RES_ERR_MPS_NO_OBJECTIVE                                 = 1110,
  MSK_RES_ERR_MPS_SPLITTED_VAR                                 = 1111,
  MSK_RES_ERR_MPS_MUL_CON_NAME                                 = 1112,
  MSK_RES_ERR_MPS_MUL_QSEC                                     = 1113,
  MSK_RES_ERR_MPS_MUL_QOBJ                                     = 1114,
  MSK_RES_ERR_MPS_INV_SEC_ORDER                                = 1115,
  MSK_RES_ERR_MPS_MUL_CSEC                                     = 1116,
  MSK_RES_ERR_MPS_CONE_TYPE                                    = 1117,
  MSK_RES_ERR_MPS_CONE_OVERLAP                                 = 1118,
  MSK_RES_ERR_MPS_CONE_REPEAT                                  = 1119,
  MSK_RES_ERR_MPS_NON_SYMMETRIC_Q                              = 1120,
  MSK_RES_ERR_MPS_DUPLICATE_Q_ELEMENT                          = 1121,
  MSK_RES_ERR_MPS_INVALID_OBJSENSE                             = 1122,
  MSK_RES_ERR_MPS_TAB_IN_FIELD2                                = 1125,
  MSK_RES_ERR_MPS_TAB_IN_FIELD3                                = 1126,
  MSK_RES_ERR_MPS_TAB_IN_FIELD5                                = 1127,
  MSK_RES_ERR_MPS_INVALID_OBJ_NAME                             = 1128,
  MSK_RES_ERR_MPS_INVALID_KEY                                  = 1129,
  MSK_RES_ERR_MPS_INVALID_INDICATOR_CONSTRAINT                 = 1130,
  MSK_RES_ERR_MPS_INVALID_INDICATOR_VARIABLE                   = 1131,
  MSK_RES_ERR_MPS_INVALID_INDICATOR_VALUE                      = 1132,
  MSK_RES_ERR_MPS_INVALID_INDICATOR_QUADRATIC_CONSTRAINT       = 1133,
  MSK_RES_ERR_OPF_SYNTAX                                       = 1134,
  MSK_RES_ERR_OPF_PREMATURE_EOF                                = 1136,
  MSK_RES_ERR_OPF_MISMATCHED_TAG                               = 1137,
  MSK_RES_ERR_OPF_DUPLICATE_BOUND                              = 1138,
  MSK_RES_ERR_OPF_DUPLICATE_CONSTRAINT_NAME                    = 1139,
  MSK_RES_ERR_OPF_INVALID_CONE_TYPE                            = 1140,
  MSK_RES_ERR_OPF_INCORRECT_TAG_PARAM                          = 1141,
  MSK_RES_ERR_OPF_INVALID_TAG                                  = 1142,
  MSK_RES_ERR_OPF_DUPLICATE_CONE_ENTRY                         = 1143,
  MSK_RES_ERR_OPF_TOO_LARGE                                    = 1144,
  MSK_RES_ERR_OPF_DUAL_INTEGER_SOLUTION                        = 1146,
  MSK_RES_ERR_LP_EMPTY                                         = 1151,
  MSK_RES_ERR_WRITE_MPS_INVALID_NAME                           = 1153,
  MSK_RES_ERR_LP_INVALID_VAR_NAME                              = 1154,
  MSK_RES_ERR_WRITE_OPF_INVALID_VAR_NAME                       = 1156,
  MSK_RES_ERR_LP_FILE_FORMAT                                   = 1157,
  MSK_RES_ERR_LP_EXPECTED_NUMBER                               = 1158,
  MSK_RES_ERR_READ_LP_MISSING_END_TAG                          = 1159,
  MSK_RES_ERR_LP_INDICATOR_VAR                                 = 1160,
  MSK_RES_ERR_LP_EXPECTED_OBJECTIVE                            = 1161,
  MSK_RES_ERR_LP_EXPECTED_CONSTRAINT_RELATION                  = 1162,
  MSK_RES_ERR_LP_AMBIGUOUS_CONSTRAINT_BOUND                    = 1163,
  MSK_RES_ERR_LP_DUPLICATE_SECTION                             = 1164,
  MSK_RES_ERR_READ_LP_DELAYED_ROWS_NOT_SUPPORTED               = 1165,
  MSK_RES_ERR_WRITING_FILE                                     = 1166,
  MSK_RES_ERR_WRITE_ASYNC                                      = 1167,
  MSK_RES_ERR_INVALID_NAME_IN_SOL_FILE                         = 1170,
  MSK_RES_ERR_JSON_SYNTAX                                      = 1175,
  MSK_RES_ERR_JSON_STRING                                      = 1176,
  MSK_RES_ERR_JSON_NUMBER_OVERFLOW                             = 1177,
  MSK_RES_ERR_JSON_FORMAT                                      = 1178,
  MSK_RES_ERR_JSON_DATA                                        = 1179,
  MSK_RES_ERR_JSON_MISSING_DATA                                = 1180,
  MSK_RES_ERR_PTF_INCOMPATIBILITY                              = 1181,
  MSK_RES_ERR_PTF_UNDEFINED_ITEM                               = 1182,
  MSK_RES_ERR_PTF_INCONSISTENCY                                = 1183,
  MSK_RES_ERR_PTF_FORMAT                                       = 1184,
  MSK_RES_ERR_ARGUMENT_LENNEQ                                  = 1197,
  MSK_RES_ERR_ARGUMENT_TYPE                                    = 1198,
  MSK_RES_ERR_NUM_ARGUMENTS                                    = 1199,
  MSK_RES_ERR_IN_ARGUMENT                                      = 1200,
  MSK_RES_ERR_ARGUMENT_DIMENSION                               = 1201,
  MSK_RES_ERR_SHAPE_IS_TOO_LARGE                               = 1202,
  MSK_RES_ERR_INDEX_IS_TOO_SMALL                               = 1203,
  MSK_RES_ERR_INDEX_IS_TOO_LARGE                               = 1204,
  MSK_RES_ERR_INDEX_IS_NOT_UNIQUE                              = 1205,
  MSK_RES_ERR_PARAM_NAME                                       = 1206,
  MSK_RES_ERR_PARAM_NAME_DOU                                   = 1207,
  MSK_RES_ERR_PARAM_NAME_INT                                   = 1208,
  MSK_RES_ERR_PARAM_NAME_STR                                   = 1209,
  MSK_RES_ERR_PARAM_INDEX                                      = 1210,
  MSK_RES_ERR_PARAM_IS_TOO_LARGE                               = 1215,
  MSK_RES_ERR_PARAM_IS_TOO_SMALL                               = 1216,
  MSK_RES_ERR_PARAM_VALUE_STR                                  = 1217,
  MSK_RES_ERR_PARAM_TYPE                                       = 1218,
  MSK_RES_ERR_INF_DOU_INDEX                                    = 1219,
  MSK_RES_ERR_INF_INT_INDEX                                    = 1220,
  MSK_RES_ERR_INDEX_ARR_IS_TOO_SMALL                           = 1221,
  MSK_RES_ERR_INDEX_ARR_IS_TOO_LARGE                           = 1222,
  MSK_RES_ERR_INF_LINT_INDEX                                   = 1225,
  MSK_RES_ERR_ARG_IS_TOO_SMALL                                 = 1226,
  MSK_RES_ERR_ARG_IS_TOO_LARGE                                 = 1227,
  MSK_RES_ERR_INVALID_WHICHSOL                                 = 1228,
  MSK_RES_ERR_INF_DOU_NAME                                     = 1230,
  MSK_RES_ERR_INF_INT_NAME                                     = 1231,
  MSK_RES_ERR_INF_TYPE                                         = 1232,
  MSK_RES_ERR_INF_LINT_NAME                                    = 1234,
  MSK_RES_ERR_INDEX                                            = 1235,
  MSK_RES_ERR_WHICHSOL                                         = 1236,
  MSK_RES_ERR_SOLITEM                                          = 1237,
  MSK_RES_ERR_WHICHITEM_NOT_ALLOWED                            = 1238,
  MSK_RES_ERR_MAXNUMCON                                        = 1240,
  MSK_RES_ERR_MAXNUMVAR                                        = 1241,
  MSK_RES_ERR_MAXNUMBARVAR                                     = 1242,
  MSK_RES_ERR_MAXNUMQNZ                                        = 1243,
  MSK_RES_ERR_TOO_SMALL_MAX_NUM_NZ                             = 1245,
  MSK_RES_ERR_INVALID_IDX                                      = 1246,
  MSK_RES_ERR_INVALID_MAX_NUM                                  = 1247,
  MSK_RES_ERR_UNALLOWED_WHICHSOL                               = 1248,
  MSK_RES_ERR_NUMCONLIM                                        = 1250,
  MSK_RES_ERR_NUMVARLIM                                        = 1251,
  MSK_RES_ERR_TOO_SMALL_MAXNUMANZ                              = 1252,
  MSK_RES_ERR_INV_APTRE                                        = 1253,
  MSK_RES_ERR_MUL_A_ELEMENT                                    = 1254,
  MSK_RES_ERR_INV_BK                                           = 1255,
  MSK_RES_ERR_INV_BKC                                          = 1256,
  MSK_RES_ERR_INV_BKX                                          = 1257,
  MSK_RES_ERR_INV_VAR_TYPE                                     = 1258,
  MSK_RES_ERR_SOLVER_PROBTYPE                                  = 1259,
  MSK_RES_ERR_OBJECTIVE_RANGE                                  = 1260,
  MSK_RES_ERR_INV_RESCODE                                      = 1261,
  MSK_RES_ERR_INV_IINF                                         = 1262,
  MSK_RES_ERR_INV_LIINF                                        = 1263,
  MSK_RES_ERR_INV_DINF                                         = 1264,
  MSK_RES_ERR_BASIS                                            = 1266,
  MSK_RES_ERR_INV_SKC                                          = 1267,
  MSK_RES_ERR_INV_SKX                                          = 1268,
  MSK_RES_ERR_INV_SK_STR                                       = 1269,
  MSK_RES_ERR_INV_SK                                           = 1270,
  MSK_RES_ERR_INV_CONE_TYPE_STR                                = 1271,
  MSK_RES_ERR_INV_CONE_TYPE                                    = 1272,
  MSK_RES_ERR_INV_SKN                                          = 1274,
  MSK_RES_ERR_INVALID_SURPLUS                                  = 1275,
  MSK_RES_ERR_INV_NAME_ITEM                                    = 1280,
  MSK_RES_ERR_PRO_ITEM                                         = 1281,
  MSK_RES_ERR_INVALID_FORMAT_TYPE                              = 1283,
  MSK_RES_ERR_FIRSTI                                           = 1285,
  MSK_RES_ERR_LASTI                                            = 1286,
  MSK_RES_ERR_FIRSTJ                                           = 1287,
  MSK_RES_ERR_LASTJ                                            = 1288,
  MSK_RES_ERR_MAX_LEN_IS_TOO_SMALL                             = 1289,
  MSK_RES_ERR_NONLINEAR_EQUALITY                               = 1290,
  MSK_RES_ERR_NONCONVEX                                        = 1291,
  MSK_RES_ERR_NONLINEAR_RANGED                                 = 1292,
  MSK_RES_ERR_CON_Q_NOT_PSD                                    = 1293,
  MSK_RES_ERR_CON_Q_NOT_NSD                                    = 1294,
  MSK_RES_ERR_OBJ_Q_NOT_PSD                                    = 1295,
  MSK_RES_ERR_OBJ_Q_NOT_NSD                                    = 1296,
  MSK_RES_ERR_ARGUMENT_PERM_ARRAY                              = 1299,
  MSK_RES_ERR_CONE_INDEX                                       = 1300,
  MSK_RES_ERR_CONE_SIZE                                        = 1301,
  MSK_RES_ERR_CONE_OVERLAP                                     = 1302,
  MSK_RES_ERR_CONE_REP_VAR                                     = 1303,
  MSK_RES_ERR_MAXNUMCONE                                       = 1304,
  MSK_RES_ERR_CONE_TYPE                                        = 1305,
  MSK_RES_ERR_CONE_TYPE_STR                                    = 1306,
  MSK_RES_ERR_CONE_OVERLAP_APPEND                              = 1307,
  MSK_RES_ERR_REMOVE_CONE_VARIABLE                             = 1310,
  MSK_RES_ERR_APPENDING_TOO_BIG_CONE                           = 1311,
  MSK_RES_ERR_CONE_PARAMETER                                   = 1320,
  MSK_RES_ERR_SOL_FILE_INVALID_NUMBER                          = 1350,
  MSK_RES_ERR_HUGE_C                                           = 1375,
  MSK_RES_ERR_HUGE_AIJ                                         = 1380,
  MSK_RES_ERR_DUPLICATE_AIJ                                    = 1385,
  MSK_RES_ERR_LOWER_BOUND_IS_A_NAN                             = 1390,
  MSK_RES_ERR_UPPER_BOUND_IS_A_NAN                             = 1391,
  MSK_RES_ERR_INFINITE_BOUND                                   = 1400,
  MSK_RES_ERR_INV_QOBJ_SUBI                                    = 1401,
  MSK_RES_ERR_INV_QOBJ_SUBJ                                    = 1402,
  MSK_RES_ERR_INV_QOBJ_VAL                                     = 1403,
  MSK_RES_ERR_INV_QCON_SUBK                                    = 1404,
  MSK_RES_ERR_INV_QCON_SUBI                                    = 1405,
  MSK_RES_ERR_INV_QCON_SUBJ                                    = 1406,
  MSK_RES_ERR_INV_QCON_VAL                                     = 1407,
  MSK_RES_ERR_QCON_SUBI_TOO_SMALL                              = 1408,
  MSK_RES_ERR_QCON_SUBI_TOO_LARGE                              = 1409,
  MSK_RES_ERR_QOBJ_UPPER_TRIANGLE                              = 1415,
  MSK_RES_ERR_QCON_UPPER_TRIANGLE                              = 1417,
  MSK_RES_ERR_FIXED_BOUND_VALUES                               = 1420,
  MSK_RES_ERR_TOO_SMALL_A_TRUNCATION_VALUE                     = 1421,
  MSK_RES_ERR_INVALID_OBJECTIVE_SENSE                          = 1445,
  MSK_RES_ERR_UNDEFINED_OBJECTIVE_SENSE                        = 1446,
  MSK_RES_ERR_Y_IS_UNDEFINED                                   = 1449,
  MSK_RES_ERR_NAN_IN_DOUBLE_DATA                               = 1450,
  MSK_RES_ERR_INF_IN_DOUBLE_DATA                               = 1451,
  MSK_RES_ERR_NAN_IN_BLC                                       = 1461,
  MSK_RES_ERR_NAN_IN_BUC                                       = 1462,
  MSK_RES_ERR_INVALID_CFIX                                     = 1469,
  MSK_RES_ERR_NAN_IN_C                                         = 1470,
  MSK_RES_ERR_NAN_IN_BLX                                       = 1471,
  MSK_RES_ERR_NAN_IN_BUX                                       = 1472,
  MSK_RES_ERR_INVALID_AIJ                                      = 1473,
  MSK_RES_ERR_INVALID_CJ                                       = 1474,
  MSK_RES_ERR_SYM_MAT_INVALID                                  = 1480,
  MSK_RES_ERR_SYM_MAT_HUGE                                     = 1482,
  MSK_RES_ERR_INV_PROBLEM                                      = 1500,
  MSK_RES_ERR_MIXED_CONIC_AND_NL                               = 1501,
  MSK_RES_ERR_GLOBAL_INV_CONIC_PROBLEM                         = 1503,
  MSK_RES_ERR_INV_OPTIMIZER                                    = 1550,
  MSK_RES_ERR_MIO_NO_OPTIMIZER                                 = 1551,
  MSK_RES_ERR_NO_OPTIMIZER_VAR_TYPE                            = 1552,
  MSK_RES_ERR_FINAL_SOLUTION                                   = 1560,
  MSK_RES_ERR_FIRST                                            = 1570,
  MSK_RES_ERR_LAST                                             = 1571,
  MSK_RES_ERR_SLICE_SIZE                                       = 1572,
  MSK_RES_ERR_NEGATIVE_SURPLUS                                 = 1573,
  MSK_RES_ERR_NEGATIVE_APPEND                                  = 1578,
  MSK_RES_ERR_POSTSOLVE                                        = 1580,
  MSK_RES_ERR_OVERFLOW                                         = 1590,
  MSK_RES_ERR_NO_BASIS_SOL                                     = 1600,
  MSK_RES_ERR_BASIS_FACTOR                                     = 1610,
  MSK_RES_ERR_BASIS_SINGULAR                                   = 1615,
  MSK_RES_ERR_FACTOR                                           = 1650,
  MSK_RES_ERR_FEASREPAIR_CANNOT_RELAX                          = 1700,
  MSK_RES_ERR_FEASREPAIR_SOLVING_RELAXED                       = 1701,
  MSK_RES_ERR_FEASREPAIR_INCONSISTENT_BOUND                    = 1702,
  MSK_RES_ERR_REPAIR_INVALID_PROBLEM                           = 1710,
  MSK_RES_ERR_REPAIR_OPTIMIZATION_FAILED                       = 1711,
  MSK_RES_ERR_NAME_MAX_LEN                                     = 1750,
  MSK_RES_ERR_NAME_IS_NULL                                     = 1760,
  MSK_RES_ERR_INVALID_COMPRESSION                              = 1800,
  MSK_RES_ERR_INVALID_IOMODE                                   = 1801,
  MSK_RES_ERR_NO_PRIMAL_INFEAS_CER                             = 2000,
  MSK_RES_ERR_NO_DUAL_INFEAS_CER                               = 2001,
  MSK_RES_ERR_NO_SOLUTION_IN_CALLBACK                          = 2500,
  MSK_RES_ERR_INV_MARKI                                        = 2501,
  MSK_RES_ERR_INV_MARKJ                                        = 2502,
  MSK_RES_ERR_INV_NUMI                                         = 2503,
  MSK_RES_ERR_INV_NUMJ                                         = 2504,
  MSK_RES_ERR_TASK_INCOMPATIBLE                                = 2560,
  MSK_RES_ERR_TASK_INVALID                                     = 2561,
  MSK_RES_ERR_TASK_WRITE                                       = 2562,
  MSK_RES_ERR_READ_WRITE                                       = 2563,
  MSK_RES_ERR_TASK_PREMATURE_EOF                               = 2564,
  MSK_RES_ERR_LU_MAX_NUM_TRIES                                 = 2800,
  MSK_RES_ERR_INVALID_UTF8                                     = 2900,
  MSK_RES_ERR_INVALID_WCHAR                                    = 2901,
  MSK_RES_ERR_NO_DUAL_FOR_ITG_SOL                              = 2950,
  MSK_RES_ERR_NO_SNX_FOR_BAS_SOL                               = 2953,
  MSK_RES_ERR_INTERNAL                                         = 3000,
  MSK_RES_ERR_API_ARRAY_TOO_SMALL                              = 3001,
  MSK_RES_ERR_API_CB_CONNECT                                   = 3002,
  MSK_RES_ERR_API_FATAL_ERROR                                  = 3005,
  MSK_RES_ERR_SEN_FORMAT                                       = 3050,
  MSK_RES_ERR_SEN_UNDEF_NAME                                   = 3051,
  MSK_RES_ERR_SEN_INDEX_RANGE                                  = 3052,
  MSK_RES_ERR_SEN_BOUND_INVALID_UP                             = 3053,
  MSK_RES_ERR_SEN_BOUND_INVALID_LO                             = 3054,
  MSK_RES_ERR_SEN_INDEX_INVALID                                = 3055,
  MSK_RES_ERR_SEN_INVALID_REGEXP                               = 3056,
  MSK_RES_ERR_SEN_SOLUTION_STATUS                              = 3057,
  MSK_RES_ERR_SEN_NUMERICAL                                    = 3058,
  MSK_RES_ERR_SEN_UNHANDLED_PROBLEM_TYPE                       = 3080,
  MSK_RES_ERR_UNB_STEP_SIZE                                    = 3100,
  MSK_RES_ERR_IDENTICAL_TASKS                                  = 3101,
  MSK_RES_ERR_AD_INVALID_CODELIST                              = 3102,
  MSK_RES_ERR_INTERNAL_TEST_FAILED                             = 3500,
  MSK_RES_ERR_INT64_TO_INT32_CAST                              = 3800,
  MSK_RES_ERR_INFEAS_UNDEFINED                                 = 3910,
  MSK_RES_ERR_NO_BARX_FOR_SOLUTION                             = 3915,
  MSK_RES_ERR_NO_BARS_FOR_SOLUTION                             = 3916,
  MSK_RES_ERR_BAR_VAR_DIM                                      = 3920,
  MSK_RES_ERR_SYM_MAT_INVALID_ROW_INDEX                        = 3940,
  MSK_RES_ERR_SYM_MAT_INVALID_COL_INDEX                        = 3941,
  MSK_RES_ERR_SYM_MAT_NOT_LOWER_TRINGULAR                      = 3942,
  MSK_RES_ERR_SYM_MAT_INVALID_VALUE                            = 3943,
  MSK_RES_ERR_SYM_MAT_DUPLICATE                                = 3944,
  MSK_RES_ERR_INVALID_SYM_MAT_DIM                              = 3950,
  MSK_RES_ERR_API_INTERNAL                                     = 3999,
  MSK_RES_ERR_INVALID_FILE_FORMAT_FOR_SYM_MAT                  = 4000,
  MSK_RES_ERR_INVALID_FILE_FORMAT_FOR_CFIX                     = 4001,
  MSK_RES_ERR_INVALID_FILE_FORMAT_FOR_RANGED_CONSTRAINTS       = 4002,
  MSK_RES_ERR_INVALID_FILE_FORMAT_FOR_FREE_CONSTRAINTS         = 4003,
  MSK_RES_ERR_INVALID_FILE_FORMAT_FOR_CONES                    = 4005,
  MSK_RES_ERR_INVALID_FILE_FORMAT_FOR_QUADRATIC_TERMS          = 4006,
  MSK_RES_ERR_INVALID_FILE_FORMAT_FOR_NONLINEAR                = 4010,
  MSK_RES_ERR_INVALID_FILE_FORMAT_FOR_DISJUNCTIVE_CONSTRAINTS  = 4011,
  MSK_RES_ERR_INVALID_FILE_FORMAT_FOR_AFFINE_CONIC_CONSTRAINTS = 4012,
  MSK_RES_ERR_DUPLICATE_CONSTRAINT_NAMES                       = 4500,
  MSK_RES_ERR_DUPLICATE_VARIABLE_NAMES                         = 4501,
  MSK_RES_ERR_DUPLICATE_BARVARIABLE_NAMES                      = 4502,
  MSK_RES_ERR_DUPLICATE_CONE_NAMES                             = 4503,
  MSK_RES_ERR_DUPLICATE_DOMAIN_NAMES                           = 4504,
  MSK_RES_ERR_DUPLICATE_DJC_NAMES                              = 4505,
  MSK_RES_ERR_NON_UNIQUE_ARRAY                                 = 5000,
  MSK_RES_ERR_ARGUMENT_IS_TOO_SMALL                            = 5004,
  MSK_RES_ERR_ARGUMENT_IS_TOO_LARGE                            = 5005,
  MSK_RES_ERR_MIO_INTERNAL                                     = 5010,
  MSK_RES_ERR_INVALID_PROBLEM_TYPE                             = 6000,
  MSK_RES_ERR_UNHANDLED_SOLUTION_STATUS                        = 6010,
  MSK_RES_ERR_UPPER_TRIANGLE                                   = 6020,
  MSK_RES_ERR_LAU_SINGULAR_MATRIX                              = 7000,
  MSK_RES_ERR_LAU_NOT_POSITIVE_DEFINITE                        = 7001,
  MSK_RES_ERR_LAU_INVALID_LOWER_TRIANGULAR_MATRIX              = 7002,
  MSK_RES_ERR_LAU_UNKNOWN                                      = 7005,
  MSK_RES_ERR_LAU_ARG_M                                        = 7010,
  MSK_RES_ERR_LAU_ARG_N                                        = 7011,
  MSK_RES_ERR_LAU_ARG_K                                        = 7012,
  MSK_RES_ERR_LAU_ARG_TRANSA                                   = 7015,
  MSK_RES_ERR_LAU_ARG_TRANSB                                   = 7016,
  MSK_RES_ERR_LAU_ARG_UPLO                                     = 7017,
  MSK_RES_ERR_LAU_ARG_TRANS                                    = 7018,
  MSK_RES_ERR_LAU_INVALID_SPARSE_SYMMETRIC_MATRIX              = 7019,
  MSK_RES_ERR_CBF_PARSE                                        = 7100,
  MSK_RES_ERR_CBF_OBJ_SENSE                                    = 7101,
  MSK_RES_ERR_CBF_NO_VARIABLES                                 = 7102,
  MSK_RES_ERR_CBF_TOO_MANY_CONSTRAINTS                         = 7103,
  MSK_RES_ERR_CBF_TOO_MANY_VARIABLES                           = 7104,
  MSK_RES_ERR_CBF_NO_VERSION_SPECIFIED                         = 7105,
  MSK_RES_ERR_CBF_SYNTAX                                       = 7106,
  MSK_RES_ERR_CBF_DUPLICATE_OBJ                                = 7107,
  MSK_RES_ERR_CBF_DUPLICATE_CON                                = 7108,
  MSK_RES_ERR_CBF_DUPLICATE_VAR                                = 7110,
  MSK_RES_ERR_CBF_DUPLICATE_INT                                = 7111,
  MSK_RES_ERR_CBF_INVALID_VAR_TYPE                             = 7112,
  MSK_RES_ERR_CBF_INVALID_CON_TYPE                             = 7113,
  MSK_RES_ERR_CBF_INVALID_DOMAIN_DIMENSION                     = 7114,
  MSK_RES_ERR_CBF_DUPLICATE_OBJACOORD                          = 7115,
  MSK_RES_ERR_CBF_DUPLICATE_BCOORD                             = 7116,
  MSK_RES_ERR_CBF_DUPLICATE_ACOORD                             = 7117,
  MSK_RES_ERR_CBF_TOO_FEW_VARIABLES                            = 7118,
  MSK_RES_ERR_CBF_TOO_FEW_CONSTRAINTS                          = 7119,
  MSK_RES_ERR_CBF_TOO_FEW_INTS                                 = 7120,
  MSK_RES_ERR_CBF_TOO_MANY_INTS                                = 7121,
  MSK_RES_ERR_CBF_INVALID_INT_INDEX                            = 7122,
  MSK_RES_ERR_CBF_UNSUPPORTED                                  = 7123,
  MSK_RES_ERR_CBF_DUPLICATE_PSDVAR                             = 7124,
  MSK_RES_ERR_CBF_INVALID_PSDVAR_DIMENSION                     = 7125,
  MSK_RES_ERR_CBF_TOO_FEW_PSDVAR                               = 7126,
  MSK_RES_ERR_CBF_INVALID_EXP_DIMENSION                        = 7127,
  MSK_RES_ERR_CBF_DUPLICATE_POW_CONES                          = 7130,
  MSK_RES_ERR_CBF_DUPLICATE_POW_STAR_CONES                     = 7131,
  MSK_RES_ERR_CBF_INVALID_POWER                                = 7132,
  MSK_RES_ERR_CBF_POWER_CONE_IS_TOO_LONG                       = 7133,
  MSK_RES_ERR_CBF_INVALID_POWER_CONE_INDEX                     = 7134,
  MSK_RES_ERR_CBF_INVALID_POWER_STAR_CONE_INDEX                = 7135,
  MSK_RES_ERR_CBF_UNHANDLED_POWER_CONE_TYPE                    = 7136,
  MSK_RES_ERR_CBF_UNHANDLED_POWER_STAR_CONE_TYPE               = 7137,
  MSK_RES_ERR_CBF_POWER_CONE_MISMATCH                          = 7138,
  MSK_RES_ERR_CBF_POWER_STAR_CONE_MISMATCH                     = 7139,
  MSK_RES_ERR_CBF_INVALID_NUMBER_OF_CONES                      = 7140,
  MSK_RES_ERR_CBF_INVALID_DIMENSION_OF_CONES                   = 7141,
  MSK_RES_ERR_CBF_INVALID_NUM_OBJACOORD                        = 7150,
  MSK_RES_ERR_CBF_INVALID_NUM_OBJFCOORD                        = 7151,
  MSK_RES_ERR_CBF_INVALID_NUM_ACOORD                           = 7152,
  MSK_RES_ERR_CBF_INVALID_NUM_BCOORD                           = 7153,
  MSK_RES_ERR_CBF_INVALID_NUM_FCOORD                           = 7155,
  MSK_RES_ERR_CBF_INVALID_NUM_HCOORD                           = 7156,
  MSK_RES_ERR_CBF_INVALID_NUM_DCOORD                           = 7157,
  MSK_RES_ERR_CBF_EXPECTED_A_KEYWORD                           = 7158,
  MSK_RES_ERR_CBF_INVALID_NUM_PSDCON                           = 7200,
  MSK_RES_ERR_CBF_DUPLICATE_PSDCON                             = 7201,
  MSK_RES_ERR_CBF_INVALID_DIMENSION_OF_PSDCON                  = 7202,
  MSK_RES_ERR_CBF_INVALID_PSDCON_INDEX                         = 7203,
  MSK_RES_ERR_CBF_INVALID_PSDCON_VARIABLE_INDEX                = 7204,
  MSK_RES_ERR_CBF_INVALID_PSDCON_BLOCK_INDEX                   = 7205,
  MSK_RES_ERR_CBF_UNSUPPORTED_CHANGE                           = 7210,
  MSK_RES_ERR_MIO_INVALID_ROOT_OPTIMIZER                       = 7700,
  MSK_RES_ERR_MIO_INVALID_NODE_OPTIMIZER                       = 7701,
  MSK_RES_ERR_MPS_WRITE_CPLEX_INVALID_CONE_TYPE                = 7750,
  MSK_RES_ERR_TOCONIC_CONSTR_Q_NOT_PSD                         = 7800,
  MSK_RES_ERR_TOCONIC_CONSTRAINT_FX                            = 7801,
  MSK_RES_ERR_TOCONIC_CONSTRAINT_RA                            = 7802,
  MSK_RES_ERR_TOCONIC_CONSTR_NOT_CONIC                         = 7803,
  MSK_RES_ERR_TOCONIC_OBJECTIVE_NOT_PSD                        = 7804,
  MSK_RES_ERR_GETDUAL_NOT_AVAILABLE                            = 7820,
  MSK_RES_ERR_SERVER_CONNECT                                   = 8000,
  MSK_RES_ERR_SERVER_PROTOCOL                                  = 8001,
  MSK_RES_ERR_SERVER_STATUS                                    = 8002,
  MSK_RES_ERR_SERVER_TOKEN                                     = 8003,
  MSK_RES_ERR_SERVER_ADDRESS                                   = 8004,
  MSK_RES_ERR_SERVER_CERTIFICATE                               = 8005,
  MSK_RES_ERR_SERVER_TLS_CLIENT                                = 8006,
  MSK_RES_ERR_SERVER_ACCESS_TOKEN                              = 8007,
  MSK_RES_ERR_SERVER_PROBLEM_SIZE                              = 8008,
  MSK_RES_ERR_SERVER_HARD_TIMEOUT                              = 8009,
  MSK_RES_ERR_DUPLICATE_INDEX_IN_A_SPARSE_MATRIX               = 20050,
  MSK_RES_ERR_DUPLICATE_INDEX_IN_AFEIDX_LIST                   = 20060,
  MSK_RES_ERR_DUPLICATE_FIJ                                    = 20100,
  MSK_RES_ERR_INVALID_FIJ                                      = 20101,
  MSK_RES_ERR_HUGE_FIJ                                         = 20102,
  MSK_RES_ERR_INVALID_G                                        = 20103,
  MSK_RES_ERR_INVALID_B                                        = 20150,
  MSK_RES_ERR_DOMAIN_INVALID_INDEX                             = 20400,
  MSK_RES_ERR_DOMAIN_DIMENSION                                 = 20401,
  MSK_RES_ERR_DOMAIN_DIMENSION_PSD                             = 20402,
  MSK_RES_ERR_NOT_POWER_DOMAIN                                 = 20403,
  MSK_RES_ERR_DOMAIN_POWER_INVALID_ALPHA                       = 20404,
  MSK_RES_ERR_DOMAIN_POWER_NEGATIVE_ALPHA                      = 20405,
  MSK_RES_ERR_DOMAIN_POWER_NLEFT                               = 20406,
  MSK_RES_ERR_AFE_INVALID_INDEX                                = 20500,
  MSK_RES_ERR_ACC_INVALID_INDEX                                = 20600,
  MSK_RES_ERR_ACC_INVALID_ENTRY_INDEX                          = 20601,
  MSK_RES_ERR_ACC_AFE_DOMAIN_MISMATCH                          = 20602,
  MSK_RES_ERR_DJC_INVALID_INDEX                                = 20700,
  MSK_RES_ERR_DJC_UNSUPPORTED_DOMAIN_TYPE                      = 20701,
  MSK_RES_ERR_DJC_AFE_DOMAIN_MISMATCH                          = 20702,
  MSK_RES_ERR_DJC_INVALID_TERM_SIZE                            = 20703,
  MSK_RES_ERR_DJC_DOMAIN_TERMSIZE_MISMATCH                     = 20704,
  MSK_RES_ERR_DJC_TOTAL_NUM_TERMS_MISMATCH                     = 20705,
  MSK_RES_ERR_UNDEF_SOLUTION                                   = 22000,
  MSK_RES_ERR_NO_DOTY                                          = 22010,
  MSK_RES_TRM_MAX_ITERATIONS                                   = 100000,
  MSK_RES_TRM_MAX_TIME                                         = 100001,
  MSK_RES_TRM_OBJECTIVE_RANGE                                  = 100002,
  MSK_RES_TRM_STALL                                            = 100006,
  MSK_RES_TRM_USER_CALLBACK                                    = 100007,
  MSK_RES_TRM_MIO_NUM_RELAXS                                   = 100008,
  MSK_RES_TRM_MIO_NUM_BRANCHES                                 = 100009,
  MSK_RES_TRM_NUM_MAX_NUM_INT_SOLUTIONS                        = 100015,
  MSK_RES_TRM_MAX_NUM_SETBACKS                                 = 100020,
  MSK_RES_TRM_NUMERICAL_PROBLEM                                = 100025,
  MSK_RES_TRM_LOST_RACE                                        = 100027,
  MSK_RES_TRM_INTERNAL                                         = 100030,
  MSK_RES_TRM_INTERNAL_STOP                                    = 100031,
  MSK_RES_TRM_SERVER_MAX_TIME                                  = 100032,
  MSK_RES_TRM_SERVER_MAX_MEMORY                                = 100033
};


enum MSKrescodetype_enum {
  MSK_RESPONSE_OK  = 0,
  MSK_RESPONSE_WRN = 1,
  MSK_RESPONSE_TRM = 2,
  MSK_RESPONSE_ERR = 3,
  MSK_RESPONSE_UNK = 4
};
#define MSK_RESPONSE_BEGIN MSK_RESPONSE_OK
#define MSK_RESPONSE_END   (1+MSK_RESPONSE_UNK)


enum MSKscalingtype_enum {
  MSK_SCALING_FREE = 0,
  MSK_SCALING_NONE = 1
};
#define MSK_SCALING_BEGIN MSK_SCALING_FREE
#define MSK_SCALING_END   (1+MSK_SCALING_NONE)


enum MSKscalingmethod_enum {
  MSK_SCALING_METHOD_POW2 = 0,
  MSK_SCALING_METHOD_FREE = 1
};
#define MSK_SCALING_METHOD_BEGIN MSK_SCALING_METHOD_POW2
#define MSK_SCALING_METHOD_END   (1+MSK_SCALING_METHOD_FREE)


enum MSKsensitivitytype_enum {
  MSK_SENSITIVITY_TYPE_BASIS = 0
};
#define MSK_SENSITIVITY_TYPE_BEGIN MSK_SENSITIVITY_TYPE_BASIS
#define MSK_SENSITIVITY_TYPE_END   (1+MSK_SENSITIVITY_TYPE_BASIS)


enum MSKsimseltype_enum {
  MSK_SIM_SELECTION_FREE    = 0,
  MSK_SIM_SELECTION_FULL    = 1,
  MSK_SIM_SELECTION_ASE     = 2,
  MSK_SIM_SELECTION_DEVEX   = 3,
  MSK_SIM_SELECTION_SE      = 4,
  MSK_SIM_SELECTION_PARTIAL = 5
};
#define MSK_SIM_SELECTION_BEGIN MSK_SIM_SELECTION_FREE
#define MSK_SIM_SELECTION_END   (1+MSK_SIM_SELECTION_PARTIAL)


enum MSKsolitem_enum {
  MSK_SOL_ITEM_XC  = 0,
  MSK_SOL_ITEM_XX  = 1,
  MSK_SOL_ITEM_Y   = 2,
  MSK_SOL_ITEM_SLC = 3,
  MSK_SOL_ITEM_SUC = 4,
  MSK_SOL_ITEM_SLX = 5,
  MSK_SOL_ITEM_SUX = 6,
  MSK_SOL_ITEM_SNX = 7
};
#define MSK_SOL_ITEM_BEGIN MSK_SOL_ITEM_XC
#define MSK_SOL_ITEM_END   (1+MSK_SOL_ITEM_SNX)


enum MSKsolsta_enum {
  MSK_SOL_STA_UNKNOWN            = 0,
  MSK_SOL_STA_OPTIMAL            = 1,
  MSK_SOL_STA_PRIM_FEAS          = 2,
  MSK_SOL_STA_DUAL_FEAS          = 3,
  MSK_SOL_STA_PRIM_AND_DUAL_FEAS = 4,
  MSK_SOL_STA_PRIM_INFEAS_CER    = 5,
  MSK_SOL_STA_DUAL_INFEAS_CER    = 6,
  MSK_SOL_STA_PRIM_ILLPOSED_CER  = 7,
  MSK_SOL_STA_DUAL_ILLPOSED_CER  = 8,
  MSK_SOL_STA_INTEGER_OPTIMAL    = 9
};
#define MSK_SOL_STA_BEGIN MSK_SOL_STA_UNKNOWN
#define MSK_SOL_STA_END   (1+MSK_SOL_STA_INTEGER_OPTIMAL)


enum MSKsoltype_enum {
  MSK_SOL_ITR = 0,
  MSK_SOL_BAS = 1,
  MSK_SOL_ITG = 2
};
#define MSK_SOL_BEGIN MSK_SOL_ITR
#define MSK_SOL_END   (1+MSK_SOL_ITG)


enum MSKsolveform_enum {
  MSK_SOLVE_FREE   = 0,
  MSK_SOLVE_PRIMAL = 1,
  MSK_SOLVE_DUAL   = 2
};
#define MSK_SOLVE_BEGIN MSK_SOLVE_FREE
#define MSK_SOLVE_END   (1+MSK_SOLVE_DUAL)


enum MSKsparam_enum {
  MSK_SPAR_BAS_SOL_FILE_NAME         = 0,
  MSK_SPAR_DATA_FILE_NAME            = 1,
  MSK_SPAR_DEBUG_FILE_NAME           = 2,
  MSK_SPAR_INT_SOL_FILE_NAME         = 3,
  MSK_SPAR_ITR_SOL_FILE_NAME         = 4,
  MSK_SPAR_MIO_DEBUG_STRING          = 5,
  MSK_SPAR_PARAM_COMMENT_SIGN        = 6,
  MSK_SPAR_PARAM_READ_FILE_NAME      = 7,
  MSK_SPAR_PARAM_WRITE_FILE_NAME     = 8,
  MSK_SPAR_READ_MPS_BOU_NAME         = 9,
  MSK_SPAR_READ_MPS_OBJ_NAME         = 10,
  MSK_SPAR_READ_MPS_RAN_NAME         = 11,
  MSK_SPAR_READ_MPS_RHS_NAME         = 12,
  MSK_SPAR_REMOTE_OPTSERVER_HOST     = 13,
  MSK_SPAR_REMOTE_TLS_CERT           = 14,
  MSK_SPAR_REMOTE_TLS_CERT_PATH      = 15,
  MSK_SPAR_SENSITIVITY_FILE_NAME     = 16,
  MSK_SPAR_SENSITIVITY_RES_FILE_NAME = 17,
  MSK_SPAR_SOL_FILTER_XC_LOW         = 18,
  MSK_SPAR_SOL_FILTER_XC_UPR         = 19,
  MSK_SPAR_SOL_FILTER_XX_LOW         = 20,
  MSK_SPAR_SOL_FILTER_XX_UPR         = 21,
  MSK_SPAR_STAT_KEY                  = 22,
  MSK_SPAR_STAT_NAME                 = 23
};
#define MSK_SPAR_BEGIN MSK_SPAR_BAS_SOL_FILE_NAME
#define MSK_SPAR_END   (1+MSK_SPAR_STAT_NAME)


enum MSKstakey_enum {
  MSK_SK_UNK    = 0,
  MSK_SK_BAS    = 1,
  MSK_SK_SUPBAS = 2,
  MSK_SK_LOW    = 3,
  MSK_SK_UPR    = 4,
  MSK_SK_FIX    = 5,
  MSK_SK_INF    = 6
};
#define MSK_SK_BEGIN MSK_SK_UNK
#define MSK_SK_END   (1+MSK_SK_INF)


enum MSKstartpointtype_enum {
  MSK_STARTING_POINT_FREE     = 0,
  MSK_STARTING_POINT_GUESS    = 1,
  MSK_STARTING_POINT_CONSTANT = 2
};
#define MSK_STARTING_POINT_BEGIN MSK_STARTING_POINT_FREE
#define MSK_STARTING_POINT_END   (1+MSK_STARTING_POINT_CONSTANT)


enum MSKstreamtype_enum {
  MSK_STREAM_LOG = 0,
  MSK_STREAM_MSG = 1,
  MSK_STREAM_ERR = 2,
  MSK_STREAM_WRN = 3
};
#define MSK_STREAM_BEGIN MSK_STREAM_LOG
#define MSK_STREAM_END   (1+MSK_STREAM_WRN)


enum MSKvalue_enum {
  MSK_LICENSE_BUFFER_LENGTH = 21,
  MSK_MAX_STR_LEN           = 1024
};


enum MSKvariabletype_enum {
  MSK_VAR_TYPE_CONT = 0,
  MSK_VAR_TYPE_INT  = 1
};
#define MSK_VAR_BEGIN MSK_VAR_TYPE_CONT
#define MSK_VAR_END   (1+MSK_VAR_TYPE_INT)


/* } namespace mosek; */
/**************************************************/
#define MSK_FIRST_ERR_CODE 1000 
#define MSK_LAST_ERR_CODE  9999 
/**************************************************/
typedef enum MSK_whichenum_enum {
  MSK_WHICHENUM_LANGUAGE,
  MSK_WHICHENUM_BASINDTYPE,
  MSK_WHICHENUM_BOUNDKEY,
  MSK_WHICHENUM_MARK,
  MSK_WHICHENUM_SIMPRECISION,
  MSK_WHICHENUM_SIMDEGEN,
  MSK_WHICHENUM_TRANSPOSE,
  MSK_WHICHENUM_UPLO,
  MSK_WHICHENUM_SIMREFORM,
  MSK_WHICHENUM_SIMDUPVEC,
  MSK_WHICHENUM_SIMHOTSTART,
  MSK_WHICHENUM_INTPNTHOTSTART,
  MSK_WHICHENUM_CALLBACKCODE,
  MSK_WHICHENUM_COMPRESSTYPE,
  MSK_WHICHENUM_CONETYPE,
  MSK_WHICHENUM_DOMAINTYPE,
  MSK_WHICHENUM_NAMETYPE,
  MSK_WHICHENUM_SYMMATTYPE,
  MSK_WHICHENUM_DATAFORMAT,
  MSK_WHICHENUM_SOLFORMAT,
  MSK_WHICHENUM_DINFITEM,
  MSK_WHICHENUM_FEATURE,
  MSK_WHICHENUM_DPARAM,
  MSK_WHICHENUM_LIINFITEM,
  MSK_WHICHENUM_INTERNAL_LIINF,
  MSK_WHICHENUM_IINFITEM,
  MSK_WHICHENUM_INFTYPE,
  MSK_WHICHENUM_INTERNAL_DINF,
  MSK_WHICHENUM_INTERNAL_IINF,
  MSK_WHICHENUM_IOMODE,
  MSK_WHICHENUM_IPARAM,
  MSK_WHICHENUM_BRANCHDIR,
  MSK_WHICHENUM_MIQCQOREFORMMETHOD,
  MSK_WHICHENUM_MIODATAPERMMETHOD,
  MSK_WHICHENUM_MIOCONTSOLTYPE,
  MSK_WHICHENUM_MIOMODE,
  MSK_WHICHENUM_MIONODESELTYPE,
  MSK_WHICHENUM_MIOVARSELTYPE,
  MSK_WHICHENUM_MPSFORMAT,
  MSK_WHICHENUM_OBJSENSE,
  MSK_WHICHENUM_ONOFFKEY,
  MSK_WHICHENUM_OPTIMIZERTYPE,
  MSK_WHICHENUM_ORDERINGTYPE,
  MSK_WHICHENUM_PRESOLVEMODE,
  MSK_WHICHENUM_FOLDINGMODE,
  MSK_WHICHENUM_PARAMETERTYPE,
  MSK_WHICHENUM_PROBLEMITEM,
  MSK_WHICHENUM_PROBLEMTYPE,
  MSK_WHICHENUM_PROSTA,
  MSK_WHICHENUM_RESCODE,
  MSK_WHICHENUM_RESCODETYPE,
  MSK_WHICHENUM_SCALINGTYPE,
  MSK_WHICHENUM_SCALINGMETHOD,
  MSK_WHICHENUM_SENSITIVITYTYPE,
  MSK_WHICHENUM_SIMSELTYPE,
  MSK_WHICHENUM_SOLITEM,
  MSK_WHICHENUM_SOLSTA,
  MSK_WHICHENUM_SOLTYPE,
  MSK_WHICHENUM_SOLVEFORM,
  MSK_WHICHENUM_SPARAM,
  MSK_WHICHENUM_STAKEY,
  MSK_WHICHENUM_STARTPOINTTYPE,
  MSK_WHICHENUM_STREAMTYPE,
  MSK_WHICHENUM_VARIABLETYPE,
  MSK_WHICHENUM_LAST
} /* MSKwhichenum_enum */
MSKwhichenume;



#define MSK_SPAR_BAS_SOL_FILE_NAME_                         "MSK_SPAR_BAS_SOL_FILE_NAME"
#define MSK_SPAR_DATA_FILE_NAME_                            "MSK_SPAR_DATA_FILE_NAME"
#define MSK_SPAR_DEBUG_FILE_NAME_                           "MSK_SPAR_DEBUG_FILE_NAME"
#define MSK_SPAR_INT_SOL_FILE_NAME_                         "MSK_SPAR_INT_SOL_FILE_NAME"
#define MSK_SPAR_ITR_SOL_FILE_NAME_                         "MSK_SPAR_ITR_SOL_FILE_NAME"
#define MSK_SPAR_MIO_DEBUG_STRING_                          "MSK_SPAR_MIO_DEBUG_STRING"
#define MSK_SPAR_PARAM_COMMENT_SIGN_                        "MSK_SPAR_PARAM_COMMENT_SIGN"
#define MSK_SPAR_PARAM_READ_FILE_NAME_                      "MSK_SPAR_PARAM_READ_FILE_NAME"
#define MSK_SPAR_PARAM_WRITE_FILE_NAME_                     "MSK_SPAR_PARAM_WRITE_FILE_NAME"
#define MSK_SPAR_READ_MPS_BOU_NAME_                         "MSK_SPAR_READ_MPS_BOU_NAME"
#define MSK_SPAR_READ_MPS_OBJ_NAME_                         "MSK_SPAR_READ_MPS_OBJ_NAME"
#define MSK_SPAR_READ_MPS_RAN_NAME_                         "MSK_SPAR_READ_MPS_RAN_NAME"
#define MSK_SPAR_READ_MPS_RHS_NAME_                         "MSK_SPAR_READ_MPS_RHS_NAME"
#define MSK_SPAR_REMOTE_OPTSERVER_HOST_                     "MSK_SPAR_REMOTE_OPTSERVER_HOST"
#define MSK_SPAR_REMOTE_TLS_CERT_                           "MSK_SPAR_REMOTE_TLS_CERT"
#define MSK_SPAR_REMOTE_TLS_CERT_PATH_                      "MSK_SPAR_REMOTE_TLS_CERT_PATH"
#define MSK_SPAR_SENSITIVITY_FILE_NAME_                     "MSK_SPAR_SENSITIVITY_FILE_NAME"
#define MSK_SPAR_SENSITIVITY_RES_FILE_NAME_                 "MSK_SPAR_SENSITIVITY_RES_FILE_NAME"
#define MSK_SPAR_SOL_FILTER_XC_LOW_                         "MSK_SPAR_SOL_FILTER_XC_LOW"
#define MSK_SPAR_SOL_FILTER_XC_UPR_                         "MSK_SPAR_SOL_FILTER_XC_UPR"
#define MSK_SPAR_SOL_FILTER_XX_LOW_                         "MSK_SPAR_SOL_FILTER_XX_LOW"
#define MSK_SPAR_SOL_FILTER_XX_UPR_                         "MSK_SPAR_SOL_FILTER_XX_UPR"
#define MSK_SPAR_STAT_KEY_                                  "MSK_SPAR_STAT_KEY"
#define MSK_SPAR_STAT_NAME_                                 "MSK_SPAR_STAT_NAME"

#define MSK_DPAR_ANA_SOL_INFEAS_TOL_                        "MSK_DPAR_ANA_SOL_INFEAS_TOL"
#define MSK_DPAR_BASIS_REL_TOL_S_                           "MSK_DPAR_BASIS_REL_TOL_S"
#define MSK_DPAR_BASIS_TOL_S_                               "MSK_DPAR_BASIS_TOL_S"
#define MSK_DPAR_BASIS_TOL_X_                               "MSK_DPAR_BASIS_TOL_X"
#define MSK_DPAR_DATA_SYM_MAT_TOL_                          "MSK_DPAR_DATA_SYM_MAT_TOL"
#define MSK_DPAR_DATA_SYM_MAT_TOL_HUGE_                     "MSK_DPAR_DATA_SYM_MAT_TOL_HUGE"
#define MSK_DPAR_DATA_SYM_MAT_TOL_LARGE_                    "MSK_DPAR_DATA_SYM_MAT_TOL_LARGE"
#define MSK_DPAR_DATA_TOL_AIJ_HUGE_                         "MSK_DPAR_DATA_TOL_AIJ_HUGE"
#define MSK_DPAR_DATA_TOL_AIJ_LARGE_                        "MSK_DPAR_DATA_TOL_AIJ_LARGE"
#define MSK_DPAR_DATA_TOL_BOUND_INF_                        "MSK_DPAR_DATA_TOL_BOUND_INF"
#define MSK_DPAR_DATA_TOL_BOUND_WRN_                        "MSK_DPAR_DATA_TOL_BOUND_WRN"
#define MSK_DPAR_DATA_TOL_C_HUGE_                           "MSK_DPAR_DATA_TOL_C_HUGE"
#define MSK_DPAR_DATA_TOL_CJ_LARGE_                         "MSK_DPAR_DATA_TOL_CJ_LARGE"
#define MSK_DPAR_DATA_TOL_QIJ_                              "MSK_DPAR_DATA_TOL_QIJ"
#define MSK_DPAR_DATA_TOL_X_                                "MSK_DPAR_DATA_TOL_X"
#define MSK_DPAR_FOLDING_TOL_EQ_                            "MSK_DPAR_FOLDING_TOL_EQ"
#define MSK_DPAR_INTPNT_CO_TOL_DFEAS_                       "MSK_DPAR_INTPNT_CO_TOL_DFEAS"
#define MSK_DPAR_INTPNT_CO_TOL_INFEAS_                      "MSK_DPAR_INTPNT_CO_TOL_INFEAS"
#define MSK_DPAR_INTPNT_CO_TOL_MU_RED_                      "MSK_DPAR_INTPNT_CO_TOL_MU_RED"
#define MSK_DPAR_INTPNT_CO_TOL_NEAR_REL_                    "MSK_DPAR_INTPNT_CO_TOL_NEAR_REL"
#define MSK_DPAR_INTPNT_CO_TOL_PFEAS_                       "MSK_DPAR_INTPNT_CO_TOL_PFEAS"
#define MSK_DPAR_INTPNT_CO_TOL_REL_GAP_                     "MSK_DPAR_INTPNT_CO_TOL_REL_GAP"
#define MSK_DPAR_INTPNT_QO_TOL_DFEAS_                       "MSK_DPAR_INTPNT_QO_TOL_DFEAS"
#define MSK_DPAR_INTPNT_QO_TOL_INFEAS_                      "MSK_DPAR_INTPNT_QO_TOL_INFEAS"
#define MSK_DPAR_INTPNT_QO_TOL_MU_RED_                      "MSK_DPAR_INTPNT_QO_TOL_MU_RED"
#define MSK_DPAR_INTPNT_QO_TOL_NEAR_REL_                    "MSK_DPAR_INTPNT_QO_TOL_NEAR_REL"
#define MSK_DPAR_INTPNT_QO_TOL_PFEAS_                       "MSK_DPAR_INTPNT_QO_TOL_PFEAS"
#define MSK_DPAR_INTPNT_QO_TOL_REL_GAP_                     "MSK_DPAR_INTPNT_QO_TOL_REL_GAP"
#define MSK_DPAR_INTPNT_TOL_DFEAS_                          "MSK_DPAR_INTPNT_TOL_DFEAS"
#define MSK_DPAR_INTPNT_TOL_DSAFE_                          "MSK_DPAR_INTPNT_TOL_DSAFE"
#define MSK_DPAR_INTPNT_TOL_INFEAS_                         "MSK_DPAR_INTPNT_TOL_INFEAS"
#define MSK_DPAR_INTPNT_TOL_MU_RED_                         "MSK_DPAR_INTPNT_TOL_MU_RED"
#define MSK_DPAR_INTPNT_TOL_PATH_                           "MSK_DPAR_INTPNT_TOL_PATH"
#define MSK_DPAR_INTPNT_TOL_PFEAS_                          "MSK_DPAR_INTPNT_TOL_PFEAS"
#define MSK_DPAR_INTPNT_TOL_PSAFE_                          "MSK_DPAR_INTPNT_TOL_PSAFE"
#define MSK_DPAR_INTPNT_TOL_REL_GAP_                        "MSK_DPAR_INTPNT_TOL_REL_GAP"
#define MSK_DPAR_INTPNT_TOL_REL_STEP_                       "MSK_DPAR_INTPNT_TOL_REL_STEP"
#define MSK_DPAR_INTPNT_TOL_STEP_SIZE_                      "MSK_DPAR_INTPNT_TOL_STEP_SIZE"
#define MSK_DPAR_LOWER_OBJ_CUT_                             "MSK_DPAR_LOWER_OBJ_CUT"
#define MSK_DPAR_LOWER_OBJ_CUT_FINITE_TRH_                  "MSK_DPAR_LOWER_OBJ_CUT_FINITE_TRH"
#define MSK_DPAR_MIO_CLIQUE_TABLE_SIZE_FACTOR_              "MSK_DPAR_MIO_CLIQUE_TABLE_SIZE_FACTOR"
#define MSK_DPAR_MIO_DJC_MAX_BIGM_                          "MSK_DPAR_MIO_DJC_MAX_BIGM"
#define MSK_DPAR_MIO_MAX_TIME_                              "MSK_DPAR_MIO_MAX_TIME"
#define MSK_DPAR_MIO_REL_GAP_CONST_                         "MSK_DPAR_MIO_REL_GAP_CONST"
#define MSK_DPAR_MIO_TOL_ABS_GAP_                           "MSK_DPAR_MIO_TOL_ABS_GAP"
#define MSK_DPAR_MIO_TOL_ABS_RELAX_INT_                     "MSK_DPAR_MIO_TOL_ABS_RELAX_INT"
#define MSK_DPAR_MIO_TOL_FEAS_                              "MSK_DPAR_MIO_TOL_FEAS"
#define MSK_DPAR_MIO_TOL_REL_DUAL_BOUND_IMPROVEMENT_        "MSK_DPAR_MIO_TOL_REL_DUAL_BOUND_IMPROVEMENT"
#define MSK_DPAR_MIO_TOL_REL_GAP_                           "MSK_DPAR_MIO_TOL_REL_GAP"
#define MSK_DPAR_OPTIMIZER_MAX_TICKS_                       "MSK_DPAR_OPTIMIZER_MAX_TICKS"
#define MSK_DPAR_OPTIMIZER_MAX_TIME_                        "MSK_DPAR_OPTIMIZER_MAX_TIME"
#define MSK_DPAR_PRESOLVE_TOL_ABS_LINDEP_                   "MSK_DPAR_PRESOLVE_TOL_ABS_LINDEP"
#define MSK_DPAR_PRESOLVE_TOL_PRIMAL_INFEAS_PERTURBATION_   "MSK_DPAR_PRESOLVE_TOL_PRIMAL_INFEAS_PERTURBATION"
#define MSK_DPAR_PRESOLVE_TOL_REL_LINDEP_                   "MSK_DPAR_PRESOLVE_TOL_REL_LINDEP"
#define MSK_DPAR_PRESOLVE_TOL_S_                            "MSK_DPAR_PRESOLVE_TOL_S"
#define MSK_DPAR_PRESOLVE_TOL_X_                            "MSK_DPAR_PRESOLVE_TOL_X"
#define MSK_DPAR_QCQO_REFORMULATE_REL_DROP_TOL_             "MSK_DPAR_QCQO_REFORMULATE_REL_DROP_TOL"
#define MSK_DPAR_SEMIDEFINITE_TOL_APPROX_                   "MSK_DPAR_SEMIDEFINITE_TOL_APPROX"
#define MSK_DPAR_SIM_LU_TOL_REL_PIV_                        "MSK_DPAR_SIM_LU_TOL_REL_PIV"
#define MSK_DPAR_SIM_PRECISION_SCALING_EXTENDED_            "MSK_DPAR_SIM_PRECISION_SCALING_EXTENDED"
#define MSK_DPAR_SIM_PRECISION_SCALING_NORMAL_              "MSK_DPAR_SIM_PRECISION_SCALING_NORMAL"
#define MSK_DPAR_SIMPLEX_ABS_TOL_PIV_                       "MSK_DPAR_SIMPLEX_ABS_TOL_PIV"
#define MSK_DPAR_UPPER_OBJ_CUT_                             "MSK_DPAR_UPPER_OBJ_CUT"
#define MSK_DPAR_UPPER_OBJ_CUT_FINITE_TRH_                  "MSK_DPAR_UPPER_OBJ_CUT_FINITE_TRH"

#define MSK_IPAR_ANA_SOL_BASIS_                             "MSK_IPAR_ANA_SOL_BASIS"
#define MSK_IPAR_ANA_SOL_PRINT_VIOLATED_                    "MSK_IPAR_ANA_SOL_PRINT_VIOLATED"
#define MSK_IPAR_AUTO_SORT_A_BEFORE_OPT_                    "MSK_IPAR_AUTO_SORT_A_BEFORE_OPT"
#define MSK_IPAR_AUTO_UPDATE_SOL_INFO_                      "MSK_IPAR_AUTO_UPDATE_SOL_INFO"
#define MSK_IPAR_BASIS_SOLVE_USE_PLUS_ONE_                  "MSK_IPAR_BASIS_SOLVE_USE_PLUS_ONE"
#define MSK_IPAR_BI_CLEAN_OPTIMIZER_                        "MSK_IPAR_BI_CLEAN_OPTIMIZER"
#define MSK_IPAR_BI_IGNORE_MAX_ITER_                        "MSK_IPAR_BI_IGNORE_MAX_ITER"
#define MSK_IPAR_BI_IGNORE_NUM_ERROR_                       "MSK_IPAR_BI_IGNORE_NUM_ERROR"
#define MSK_IPAR_BI_MAX_ITERATIONS_                         "MSK_IPAR_BI_MAX_ITERATIONS"
#define MSK_IPAR_CACHE_LICENSE_                             "MSK_IPAR_CACHE_LICENSE"
#define MSK_IPAR_COMPRESS_STATFILE_                         "MSK_IPAR_COMPRESS_STATFILE"
#define MSK_IPAR_FOLDING_USE_                               "MSK_IPAR_FOLDING_USE"
#define MSK_IPAR_GETDUAL_CONVERT_LMIS_                      "MSK_IPAR_GETDUAL_CONVERT_LMIS"
#define MSK_IPAR_HEARTBEAT_SIM_FREQ_TICKS_                  "MSK_IPAR_HEARTBEAT_SIM_FREQ_TICKS"
#define MSK_IPAR_INFEAS_GENERIC_NAMES_                      "MSK_IPAR_INFEAS_GENERIC_NAMES"
#define MSK_IPAR_INFEAS_REPORT_AUTO_                        "MSK_IPAR_INFEAS_REPORT_AUTO"
#define MSK_IPAR_INFEAS_REPORT_LEVEL_                       "MSK_IPAR_INFEAS_REPORT_LEVEL"
#define MSK_IPAR_INTPNT_BASIS_                              "MSK_IPAR_INTPNT_BASIS"
#define MSK_IPAR_INTPNT_DIFF_STEP_                          "MSK_IPAR_INTPNT_DIFF_STEP"
#define MSK_IPAR_INTPNT_HOTSTART_                           "MSK_IPAR_INTPNT_HOTSTART"
#define MSK_IPAR_INTPNT_MAX_ITERATIONS_                     "MSK_IPAR_INTPNT_MAX_ITERATIONS"
#define MSK_IPAR_INTPNT_MAX_NUM_COR_                        "MSK_IPAR_INTPNT_MAX_NUM_COR"
#define MSK_IPAR_INTPNT_OFF_COL_TRH_                        "MSK_IPAR_INTPNT_OFF_COL_TRH"
#define MSK_IPAR_INTPNT_ORDER_GP_NUM_SEEDS_                 "MSK_IPAR_INTPNT_ORDER_GP_NUM_SEEDS"
#define MSK_IPAR_INTPNT_ORDER_METHOD_                       "MSK_IPAR_INTPNT_ORDER_METHOD"
#define MSK_IPAR_INTPNT_REGULARIZATION_USE_                 "MSK_IPAR_INTPNT_REGULARIZATION_USE"
#define MSK_IPAR_INTPNT_SCALING_                            "MSK_IPAR_INTPNT_SCALING"
#define MSK_IPAR_INTPNT_SOLVE_FORM_                         "MSK_IPAR_INTPNT_SOLVE_FORM"
#define MSK_IPAR_INTPNT_STARTING_POINT_                     "MSK_IPAR_INTPNT_STARTING_POINT"
#define MSK_IPAR_LICENSE_DEBUG_                             "MSK_IPAR_LICENSE_DEBUG"
#define MSK_IPAR_LICENSE_PAUSE_TIME_                        "MSK_IPAR_LICENSE_PAUSE_TIME"
#define MSK_IPAR_LICENSE_SUPPRESS_EXPIRE_WRNS_              "MSK_IPAR_LICENSE_SUPPRESS_EXPIRE_WRNS"
#define MSK_IPAR_LICENSE_TRH_EXPIRY_WRN_                    "MSK_IPAR_LICENSE_TRH_EXPIRY_WRN"
#define MSK_IPAR_LICENSE_WAIT_                              "MSK_IPAR_LICENSE_WAIT"
#define MSK_IPAR_LOG_                                       "MSK_IPAR_LOG"
#define MSK_IPAR_LOG_ANA_PRO_                               "MSK_IPAR_LOG_ANA_PRO"
#define MSK_IPAR_LOG_BI_                                    "MSK_IPAR_LOG_BI"
#define MSK_IPAR_LOG_BI_FREQ_                               "MSK_IPAR_LOG_BI_FREQ"
#define MSK_IPAR_LOG_CUT_SECOND_OPT_                        "MSK_IPAR_LOG_CUT_SECOND_OPT"
#define MSK_IPAR_LOG_EXPAND_                                "MSK_IPAR_LOG_EXPAND"
#define MSK_IPAR_LOG_FEAS_REPAIR_                           "MSK_IPAR_LOG_FEAS_REPAIR"
#define MSK_IPAR_LOG_FILE_                                  "MSK_IPAR_LOG_FILE"
#define MSK_IPAR_LOG_INCLUDE_SUMMARY_                       "MSK_IPAR_LOG_INCLUDE_SUMMARY"
#define MSK_IPAR_LOG_INFEAS_ANA_                            "MSK_IPAR_LOG_INFEAS_ANA"
#define MSK_IPAR_LOG_INTPNT_                                "MSK_IPAR_LOG_INTPNT"
#define MSK_IPAR_LOG_LOCAL_INFO_                            "MSK_IPAR_LOG_LOCAL_INFO"
#define MSK_IPAR_LOG_MIO_                                   "MSK_IPAR_LOG_MIO"
#define MSK_IPAR_LOG_MIO_FREQ_                              "MSK_IPAR_LOG_MIO_FREQ"
#define MSK_IPAR_LOG_ORDER_                                 "MSK_IPAR_LOG_ORDER"
#define MSK_IPAR_LOG_PRESOLVE_                              "MSK_IPAR_LOG_PRESOLVE"
#define MSK_IPAR_LOG_SENSITIVITY_                           "MSK_IPAR_LOG_SENSITIVITY"
#define MSK_IPAR_LOG_SENSITIVITY_OPT_                       "MSK_IPAR_LOG_SENSITIVITY_OPT"
#define MSK_IPAR_LOG_SIM_                                   "MSK_IPAR_LOG_SIM"
#define MSK_IPAR_LOG_SIM_FREQ_                              "MSK_IPAR_LOG_SIM_FREQ"
#define MSK_IPAR_LOG_SIM_FREQ_GIGA_TICKS_                   "MSK_IPAR_LOG_SIM_FREQ_GIGA_TICKS"
#define MSK_IPAR_LOG_STORAGE_                               "MSK_IPAR_LOG_STORAGE"
#define MSK_IPAR_MAX_NUM_WARNINGS_                          "MSK_IPAR_MAX_NUM_WARNINGS"
#define MSK_IPAR_MIO_BRANCH_DIR_                            "MSK_IPAR_MIO_BRANCH_DIR"
#define MSK_IPAR_MIO_CONFLICT_ANALYSIS_LEVEL_               "MSK_IPAR_MIO_CONFLICT_ANALYSIS_LEVEL"
#define MSK_IPAR_MIO_CONIC_OUTER_APPROXIMATION_             "MSK_IPAR_MIO_CONIC_OUTER_APPROXIMATION"
#define MSK_IPAR_MIO_CONSTRUCT_SOL_                         "MSK_IPAR_MIO_CONSTRUCT_SOL"
#define MSK_IPAR_MIO_CROSSOVER_MAX_NODES_                   "MSK_IPAR_MIO_CROSSOVER_MAX_NODES"
#define MSK_IPAR_MIO_CUT_CLIQUE_                            "MSK_IPAR_MIO_CUT_CLIQUE"
#define MSK_IPAR_MIO_CUT_CMIR_                              "MSK_IPAR_MIO_CUT_CMIR"
#define MSK_IPAR_MIO_CUT_GMI_                               "MSK_IPAR_MIO_CUT_GMI"
#define MSK_IPAR_MIO_CUT_IMPLIED_BOUND_                     "MSK_IPAR_MIO_CUT_IMPLIED_BOUND"
#define MSK_IPAR_MIO_CUT_KNAPSACK_COVER_                    "MSK_IPAR_MIO_CUT_KNAPSACK_COVER"
#define MSK_IPAR_MIO_CUT_LIPRO_                             "MSK_IPAR_MIO_CUT_LIPRO"
#define MSK_IPAR_MIO_CUT_SELECTION_LEVEL_                   "MSK_IPAR_MIO_CUT_SELECTION_LEVEL"
#define MSK_IPAR_MIO_DATA_PERMUTATION_METHOD_               "MSK_IPAR_MIO_DATA_PERMUTATION_METHOD"
#define MSK_IPAR_MIO_DUAL_RAY_ANALYSIS_LEVEL_               "MSK_IPAR_MIO_DUAL_RAY_ANALYSIS_LEVEL"
#define MSK_IPAR_MIO_FEASPUMP_LEVEL_                        "MSK_IPAR_MIO_FEASPUMP_LEVEL"
#define MSK_IPAR_MIO_HEURISTIC_LEVEL_                       "MSK_IPAR_MIO_HEURISTIC_LEVEL"
#define MSK_IPAR_MIO_INDEPENDENT_BLOCK_LEVEL_               "MSK_IPAR_MIO_INDEPENDENT_BLOCK_LEVEL"
#define MSK_IPAR_MIO_MAX_NUM_BRANCHES_                      "MSK_IPAR_MIO_MAX_NUM_BRANCHES"
#define MSK_IPAR_MIO_MAX_NUM_RELAXS_                        "MSK_IPAR_MIO_MAX_NUM_RELAXS"
#define MSK_IPAR_MIO_MAX_NUM_RESTARTS_                      "MSK_IPAR_MIO_MAX_NUM_RESTARTS"
#define MSK_IPAR_MIO_MAX_NUM_ROOT_CUT_ROUNDS_               "MSK_IPAR_MIO_MAX_NUM_ROOT_CUT_ROUNDS"
#define MSK_IPAR_MIO_MAX_NUM_SOLUTIONS_                     "MSK_IPAR_MIO_MAX_NUM_SOLUTIONS"
#define MSK_IPAR_MIO_MEMORY_EMPHASIS_LEVEL_                 "MSK_IPAR_MIO_MEMORY_EMPHASIS_LEVEL"
#define MSK_IPAR_MIO_MIN_REL_                               "MSK_IPAR_MIO_MIN_REL"
#define MSK_IPAR_MIO_MODE_                                  "MSK_IPAR_MIO_MODE"
#define MSK_IPAR_MIO_NODE_OPTIMIZER_                        "MSK_IPAR_MIO_NODE_OPTIMIZER"
#define MSK_IPAR_MIO_NODE_SELECTION_                        "MSK_IPAR_MIO_NODE_SELECTION"
#define MSK_IPAR_MIO_NUMERICAL_EMPHASIS_LEVEL_              "MSK_IPAR_MIO_NUMERICAL_EMPHASIS_LEVEL"
#define MSK_IPAR_MIO_OPT_FACE_MAX_NODES_                    "MSK_IPAR_MIO_OPT_FACE_MAX_NODES"
#define MSK_IPAR_MIO_PERSPECTIVE_REFORMULATE_               "MSK_IPAR_MIO_PERSPECTIVE_REFORMULATE"
#define MSK_IPAR_MIO_PRESOLVE_AGGREGATOR_USE_               "MSK_IPAR_MIO_PRESOLVE_AGGREGATOR_USE"
#define MSK_IPAR_MIO_PROBING_LEVEL_                         "MSK_IPAR_MIO_PROBING_LEVEL"
#define MSK_IPAR_MIO_PROPAGATE_OBJECTIVE_CONSTRAINT_        "MSK_IPAR_MIO_PROPAGATE_OBJECTIVE_CONSTRAINT"
#define MSK_IPAR_MIO_QCQO_REFORMULATION_METHOD_             "MSK_IPAR_MIO_QCQO_REFORMULATION_METHOD"
#define MSK_IPAR_MIO_RENS_MAX_NODES_                        "MSK_IPAR_MIO_RENS_MAX_NODES"
#define MSK_IPAR_MIO_RINS_MAX_NODES_                        "MSK_IPAR_MIO_RINS_MAX_NODES"
#define MSK_IPAR_MIO_ROOT_OPTIMIZER_                        "MSK_IPAR_MIO_ROOT_OPTIMIZER"
#define MSK_IPAR_MIO_SEED_                                  "MSK_IPAR_MIO_SEED"
#define MSK_IPAR_MIO_SYMMETRY_LEVEL_                        "MSK_IPAR_MIO_SYMMETRY_LEVEL"
#define MSK_IPAR_MIO_VAR_SELECTION_                         "MSK_IPAR_MIO_VAR_SELECTION"
#define MSK_IPAR_MIO_VB_DETECTION_LEVEL_                    "MSK_IPAR_MIO_VB_DETECTION_LEVEL"
#define MSK_IPAR_MT_SPINCOUNT_                              "MSK_IPAR_MT_SPINCOUNT"
#define MSK_IPAR_NG_                                        "MSK_IPAR_NG"
#define MSK_IPAR_NUM_THREADS_                               "MSK_IPAR_NUM_THREADS"
#define MSK_IPAR_OPF_WRITE_HEADER_                          "MSK_IPAR_OPF_WRITE_HEADER"
#define MSK_IPAR_OPF_WRITE_HINTS_                           "MSK_IPAR_OPF_WRITE_HINTS"
#define MSK_IPAR_OPF_WRITE_LINE_LENGTH_                     "MSK_IPAR_OPF_WRITE_LINE_LENGTH"
#define MSK_IPAR_OPF_WRITE_PARAMETERS_                      "MSK_IPAR_OPF_WRITE_PARAMETERS"
#define MSK_IPAR_OPF_WRITE_PROBLEM_                         "MSK_IPAR_OPF_WRITE_PROBLEM"
#define MSK_IPAR_OPF_WRITE_SOL_BAS_                         "MSK_IPAR_OPF_WRITE_SOL_BAS"
#define MSK_IPAR_OPF_WRITE_SOL_ITG_                         "MSK_IPAR_OPF_WRITE_SOL_ITG"
#define MSK_IPAR_OPF_WRITE_SOL_ITR_                         "MSK_IPAR_OPF_WRITE_SOL_ITR"
#define MSK_IPAR_OPF_WRITE_SOLUTIONS_                       "MSK_IPAR_OPF_WRITE_SOLUTIONS"
#define MSK_IPAR_OPTIMIZER_                                 "MSK_IPAR_OPTIMIZER"
#define MSK_IPAR_PARAM_READ_CASE_NAME_                      "MSK_IPAR_PARAM_READ_CASE_NAME"
#define MSK_IPAR_PARAM_READ_IGN_ERROR_                      "MSK_IPAR_PARAM_READ_IGN_ERROR"
#define MSK_IPAR_PRESOLVE_ELIMINATOR_MAX_FILL_              "MSK_IPAR_PRESOLVE_ELIMINATOR_MAX_FILL"
#define MSK_IPAR_PRESOLVE_ELIMINATOR_MAX_NUM_TRIES_         "MSK_IPAR_PRESOLVE_ELIMINATOR_MAX_NUM_TRIES"
#define MSK_IPAR_PRESOLVE_LINDEP_ABS_WORK_TRH_              "MSK_IPAR_PRESOLVE_LINDEP_ABS_WORK_TRH"
#define MSK_IPAR_PRESOLVE_LINDEP_NEW_                       "MSK_IPAR_PRESOLVE_LINDEP_NEW"
#define MSK_IPAR_PRESOLVE_LINDEP_REL_WORK_TRH_              "MSK_IPAR_PRESOLVE_LINDEP_REL_WORK_TRH"
#define MSK_IPAR_PRESOLVE_LINDEP_USE_                       "MSK_IPAR_PRESOLVE_LINDEP_USE"
#define MSK_IPAR_PRESOLVE_MAX_NUM_PASS_                     "MSK_IPAR_PRESOLVE_MAX_NUM_PASS"
#define MSK_IPAR_PRESOLVE_MAX_NUM_REDUCTIONS_               "MSK_IPAR_PRESOLVE_MAX_NUM_REDUCTIONS"
#define MSK_IPAR_PRESOLVE_USE_                              "MSK_IPAR_PRESOLVE_USE"
#define MSK_IPAR_PRIMAL_REPAIR_OPTIMIZER_                   "MSK_IPAR_PRIMAL_REPAIR_OPTIMIZER"
#define MSK_IPAR_PTF_WRITE_PARAMETERS_                      "MSK_IPAR_PTF_WRITE_PARAMETERS"
#define MSK_IPAR_PTF_WRITE_SINGLE_PSD_TERMS_                "MSK_IPAR_PTF_WRITE_SINGLE_PSD_TERMS"
#define MSK_IPAR_PTF_WRITE_SOLUTIONS_                       "MSK_IPAR_PTF_WRITE_SOLUTIONS"
#define MSK_IPAR_PTF_WRITE_TRANSFORM_                       "MSK_IPAR_PTF_WRITE_TRANSFORM"
#define MSK_IPAR_READ_ASYNC_                                "MSK_IPAR_READ_ASYNC"
#define MSK_IPAR_READ_DEBUG_                                "MSK_IPAR_READ_DEBUG"
#define MSK_IPAR_READ_KEEP_FREE_CON_                        "MSK_IPAR_READ_KEEP_FREE_CON"
#define MSK_IPAR_READ_MPS_FORMAT_                           "MSK_IPAR_READ_MPS_FORMAT"
#define MSK_IPAR_READ_MPS_WIDTH_                            "MSK_IPAR_READ_MPS_WIDTH"
#define MSK_IPAR_READ_TASK_IGNORE_PARAM_                    "MSK_IPAR_READ_TASK_IGNORE_PARAM"
#define MSK_IPAR_REMOTE_USE_COMPRESSION_                    "MSK_IPAR_REMOTE_USE_COMPRESSION"
#define MSK_IPAR_REMOVE_UNUSED_SOLUTIONS_                   "MSK_IPAR_REMOVE_UNUSED_SOLUTIONS"
#define MSK_IPAR_SENSITIVITY_ALL_                           "MSK_IPAR_SENSITIVITY_ALL"
#define MSK_IPAR_SENSITIVITY_TYPE_                          "MSK_IPAR_SENSITIVITY_TYPE"
#define MSK_IPAR_SIM_BASIS_FACTOR_USE_                      "MSK_IPAR_SIM_BASIS_FACTOR_USE"
#define MSK_IPAR_SIM_DEGEN_                                 "MSK_IPAR_SIM_DEGEN"
#define MSK_IPAR_SIM_DETECT_PWL_                            "MSK_IPAR_SIM_DETECT_PWL"
#define MSK_IPAR_SIM_DUAL_CRASH_                            "MSK_IPAR_SIM_DUAL_CRASH"
#define MSK_IPAR_SIM_DUAL_PHASEONE_METHOD_                  "MSK_IPAR_SIM_DUAL_PHASEONE_METHOD"
#define MSK_IPAR_SIM_DUAL_RESTRICT_SELECTION_               "MSK_IPAR_SIM_DUAL_RESTRICT_SELECTION"
#define MSK_IPAR_SIM_DUAL_SELECTION_                        "MSK_IPAR_SIM_DUAL_SELECTION"
#define MSK_IPAR_SIM_EXPLOIT_DUPVEC_                        "MSK_IPAR_SIM_EXPLOIT_DUPVEC"
#define MSK_IPAR_SIM_HOTSTART_                              "MSK_IPAR_SIM_HOTSTART"
#define MSK_IPAR_SIM_HOTSTART_LU_                           "MSK_IPAR_SIM_HOTSTART_LU"
#define MSK_IPAR_SIM_MAX_ITERATIONS_                        "MSK_IPAR_SIM_MAX_ITERATIONS"
#define MSK_IPAR_SIM_MAX_NUM_SETBACKS_                      "MSK_IPAR_SIM_MAX_NUM_SETBACKS"
#define MSK_IPAR_SIM_NON_SINGULAR_                          "MSK_IPAR_SIM_NON_SINGULAR"
#define MSK_IPAR_SIM_PRECISION_                             "MSK_IPAR_SIM_PRECISION"
#define MSK_IPAR_SIM_PRECISION_BOOST_                       "MSK_IPAR_SIM_PRECISION_BOOST"
#define MSK_IPAR_SIM_PRIMAL_CRASH_                          "MSK_IPAR_SIM_PRIMAL_CRASH"
#define MSK_IPAR_SIM_PRIMAL_PHASEONE_METHOD_                "MSK_IPAR_SIM_PRIMAL_PHASEONE_METHOD"
#define MSK_IPAR_SIM_PRIMAL_RESTRICT_SELECTION_             "MSK_IPAR_SIM_PRIMAL_RESTRICT_SELECTION"
#define MSK_IPAR_SIM_PRIMAL_SELECTION_                      "MSK_IPAR_SIM_PRIMAL_SELECTION"
#define MSK_IPAR_SIM_REFACTOR_FREQ_                         "MSK_IPAR_SIM_REFACTOR_FREQ"
#define MSK_IPAR_SIM_REFORMULATION_                         "MSK_IPAR_SIM_REFORMULATION"
#define MSK_IPAR_SIM_SAVE_LU_                               "MSK_IPAR_SIM_SAVE_LU"
#define MSK_IPAR_SIM_SCALING_                               "MSK_IPAR_SIM_SCALING"
#define MSK_IPAR_SIM_SCALING_METHOD_                        "MSK_IPAR_SIM_SCALING_METHOD"
#define MSK_IPAR_SIM_SEED_                                  "MSK_IPAR_SIM_SEED"
#define MSK_IPAR_SIM_SOLVE_FORM_                            "MSK_IPAR_SIM_SOLVE_FORM"
#define MSK_IPAR_SIM_SWITCH_OPTIMIZER_                      "MSK_IPAR_SIM_SWITCH_OPTIMIZER"
#define MSK_IPAR_SOL_FILTER_KEEP_BASIC_                     "MSK_IPAR_SOL_FILTER_KEEP_BASIC"
#define MSK_IPAR_SOL_READ_NAME_WIDTH_                       "MSK_IPAR_SOL_READ_NAME_WIDTH"
#define MSK_IPAR_SOL_READ_WIDTH_                            "MSK_IPAR_SOL_READ_WIDTH"
#define MSK_IPAR_TIMING_LEVEL_                              "MSK_IPAR_TIMING_LEVEL"
#define MSK_IPAR_WRITE_ASYNC_                               "MSK_IPAR_WRITE_ASYNC"
#define MSK_IPAR_WRITE_BAS_CONSTRAINTS_                     "MSK_IPAR_WRITE_BAS_CONSTRAINTS"
#define MSK_IPAR_WRITE_BAS_HEAD_                            "MSK_IPAR_WRITE_BAS_HEAD"
#define MSK_IPAR_WRITE_BAS_VARIABLES_                       "MSK_IPAR_WRITE_BAS_VARIABLES"
#define MSK_IPAR_WRITE_COMPRESSION_                         "MSK_IPAR_WRITE_COMPRESSION"
#define MSK_IPAR_WRITE_FREE_CON_                            "MSK_IPAR_WRITE_FREE_CON"
#define MSK_IPAR_WRITE_GENERIC_NAMES_                       "MSK_IPAR_WRITE_GENERIC_NAMES"
#define MSK_IPAR_WRITE_IGNORE_INCOMPATIBLE_ITEMS_           "MSK_IPAR_WRITE_IGNORE_INCOMPATIBLE_ITEMS"
#define MSK_IPAR_WRITE_INT_CONSTRAINTS_                     "MSK_IPAR_WRITE_INT_CONSTRAINTS"
#define MSK_IPAR_WRITE_INT_HEAD_                            "MSK_IPAR_WRITE_INT_HEAD"
#define MSK_IPAR_WRITE_INT_VARIABLES_                       "MSK_IPAR_WRITE_INT_VARIABLES"
#define MSK_IPAR_WRITE_JSON_INDENTATION_                    "MSK_IPAR_WRITE_JSON_INDENTATION"
#define MSK_IPAR_WRITE_LP_FULL_OBJ_                         "MSK_IPAR_WRITE_LP_FULL_OBJ"
#define MSK_IPAR_WRITE_LP_LINE_WIDTH_                       "MSK_IPAR_WRITE_LP_LINE_WIDTH"
#define MSK_IPAR_WRITE_MPS_FORMAT_                          "MSK_IPAR_WRITE_MPS_FORMAT"
#define MSK_IPAR_WRITE_MPS_INT_                             "MSK_IPAR_WRITE_MPS_INT"
#define MSK_IPAR_WRITE_SOL_BARVARIABLES_                    "MSK_IPAR_WRITE_SOL_BARVARIABLES"
#define MSK_IPAR_WRITE_SOL_CONSTRAINTS_                     "MSK_IPAR_WRITE_SOL_CONSTRAINTS"
#define MSK_IPAR_WRITE_SOL_HEAD_                            "MSK_IPAR_WRITE_SOL_HEAD"
#define MSK_IPAR_WRITE_SOL_IGNORE_INVALID_NAMES_            "MSK_IPAR_WRITE_SOL_IGNORE_INVALID_NAMES"
#define MSK_IPAR_WRITE_SOL_VARIABLES_                       "MSK_IPAR_WRITE_SOL_VARIABLES"

#define MSK_IINF_ANA_PRO_NUM_CON_                           "MSK_IINF_ANA_PRO_NUM_CON"
#define MSK_IINF_ANA_PRO_NUM_CON_EQ_                        "MSK_IINF_ANA_PRO_NUM_CON_EQ"
#define MSK_IINF_ANA_PRO_NUM_CON_FR_                        "MSK_IINF_ANA_PRO_NUM_CON_FR"
#define MSK_IINF_ANA_PRO_NUM_CON_LO_                        "MSK_IINF_ANA_PRO_NUM_CON_LO"
#define MSK_IINF_ANA_PRO_NUM_CON_RA_                        "MSK_IINF_ANA_PRO_NUM_CON_RA"
#define MSK_IINF_ANA_PRO_NUM_CON_UP_                        "MSK_IINF_ANA_PRO_NUM_CON_UP"
#define MSK_IINF_ANA_PRO_NUM_VAR_                           "MSK_IINF_ANA_PRO_NUM_VAR"
#define MSK_IINF_ANA_PRO_NUM_VAR_BIN_                       "MSK_IINF_ANA_PRO_NUM_VAR_BIN"
#define MSK_IINF_ANA_PRO_NUM_VAR_CONT_                      "MSK_IINF_ANA_PRO_NUM_VAR_CONT"
#define MSK_IINF_ANA_PRO_NUM_VAR_EQ_                        "MSK_IINF_ANA_PRO_NUM_VAR_EQ"
#define MSK_IINF_ANA_PRO_NUM_VAR_FR_                        "MSK_IINF_ANA_PRO_NUM_VAR_FR"
#define MSK_IINF_ANA_PRO_NUM_VAR_INT_                       "MSK_IINF_ANA_PRO_NUM_VAR_INT"
#define MSK_IINF_ANA_PRO_NUM_VAR_LO_                        "MSK_IINF_ANA_PRO_NUM_VAR_LO"
#define MSK_IINF_ANA_PRO_NUM_VAR_RA_                        "MSK_IINF_ANA_PRO_NUM_VAR_RA"
#define MSK_IINF_ANA_PRO_NUM_VAR_UP_                        "MSK_IINF_ANA_PRO_NUM_VAR_UP"
#define MSK_IINF_FOLDING_APPLIED_                           "MSK_IINF_FOLDING_APPLIED"
#define MSK_IINF_INTPNT_FACTOR_DIM_DENSE_                   "MSK_IINF_INTPNT_FACTOR_DIM_DENSE"
#define MSK_IINF_INTPNT_ITER_                               "MSK_IINF_INTPNT_ITER"
#define MSK_IINF_INTPNT_NUM_THREADS_                        "MSK_IINF_INTPNT_NUM_THREADS"
#define MSK_IINF_INTPNT_SOLVE_DUAL_                         "MSK_IINF_INTPNT_SOLVE_DUAL"
#define MSK_IINF_MIO_ABSGAP_SATISFIED_                      "MSK_IINF_MIO_ABSGAP_SATISFIED"
#define MSK_IINF_MIO_CLIQUE_TABLE_SIZE_                     "MSK_IINF_MIO_CLIQUE_TABLE_SIZE"
#define MSK_IINF_MIO_CONSTRUCT_SOLUTION_                    "MSK_IINF_MIO_CONSTRUCT_SOLUTION"
#define MSK_IINF_MIO_FINAL_NUMBIN_                          "MSK_IINF_MIO_FINAL_NUMBIN"
#define MSK_IINF_MIO_FINAL_NUMBINCONEVAR_                   "MSK_IINF_MIO_FINAL_NUMBINCONEVAR"
#define MSK_IINF_MIO_FINAL_NUMCON_                          "MSK_IINF_MIO_FINAL_NUMCON"
#define MSK_IINF_MIO_FINAL_NUMCONE_                         "MSK_IINF_MIO_FINAL_NUMCONE"
#define MSK_IINF_MIO_FINAL_NUMCONEVAR_                      "MSK_IINF_MIO_FINAL_NUMCONEVAR"
#define MSK_IINF_MIO_FINAL_NUMCONT_                         "MSK_IINF_MIO_FINAL_NUMCONT"
#define MSK_IINF_MIO_FINAL_NUMCONTCONEVAR_                  "MSK_IINF_MIO_FINAL_NUMCONTCONEVAR"
#define MSK_IINF_MIO_FINAL_NUMDEXPCONES_                    "MSK_IINF_MIO_FINAL_NUMDEXPCONES"
#define MSK_IINF_MIO_FINAL_NUMDJC_                          "MSK_IINF_MIO_FINAL_NUMDJC"
#define MSK_IINF_MIO_FINAL_NUMDPOWCONES_                    "MSK_IINF_MIO_FINAL_NUMDPOWCONES"
#define MSK_IINF_MIO_FINAL_NUMINT_                          "MSK_IINF_MIO_FINAL_NUMINT"
#define MSK_IINF_MIO_FINAL_NUMINTCONEVAR_                   "MSK_IINF_MIO_FINAL_NUMINTCONEVAR"
#define MSK_IINF_MIO_FINAL_NUMPEXPCONES_                    "MSK_IINF_MIO_FINAL_NUMPEXPCONES"
#define MSK_IINF_MIO_FINAL_NUMPPOWCONES_                    "MSK_IINF_MIO_FINAL_NUMPPOWCONES"
#define MSK_IINF_MIO_FINAL_NUMQCONES_                       "MSK_IINF_MIO_FINAL_NUMQCONES"
#define MSK_IINF_MIO_FINAL_NUMRQCONES_                      "MSK_IINF_MIO_FINAL_NUMRQCONES"
#define MSK_IINF_MIO_FINAL_NUMVAR_                          "MSK_IINF_MIO_FINAL_NUMVAR"
#define MSK_IINF_MIO_INITIAL_FEASIBLE_SOLUTION_             "MSK_IINF_MIO_INITIAL_FEASIBLE_SOLUTION"
#define MSK_IINF_MIO_NODE_DEPTH_                            "MSK_IINF_MIO_NODE_DEPTH"
#define MSK_IINF_MIO_NUM_ACTIVE_NODES_                      "MSK_IINF_MIO_NUM_ACTIVE_NODES"
#define MSK_IINF_MIO_NUM_ACTIVE_ROOT_CUTS_                  "MSK_IINF_MIO_NUM_ACTIVE_ROOT_CUTS"
#define MSK_IINF_MIO_NUM_BLOCKS_SOLVED_IN_BB_               "MSK_IINF_MIO_NUM_BLOCKS_SOLVED_IN_BB"
#define MSK_IINF_MIO_NUM_BLOCKS_SOLVED_IN_PRESOLVE_         "MSK_IINF_MIO_NUM_BLOCKS_SOLVED_IN_PRESOLVE"
#define MSK_IINF_MIO_NUM_BRANCH_                            "MSK_IINF_MIO_NUM_BRANCH"
#define MSK_IINF_MIO_NUM_INT_SOLUTIONS_                     "MSK_IINF_MIO_NUM_INT_SOLUTIONS"
#define MSK_IINF_MIO_NUM_RELAX_                             "MSK_IINF_MIO_NUM_RELAX"
#define MSK_IINF_MIO_NUM_REPEATED_PRESOLVE_                 "MSK_IINF_MIO_NUM_REPEATED_PRESOLVE"
#define MSK_IINF_MIO_NUM_RESTARTS_                          "MSK_IINF_MIO_NUM_RESTARTS"
#define MSK_IINF_MIO_NUM_ROOT_CUT_ROUNDS_                   "MSK_IINF_MIO_NUM_ROOT_CUT_ROUNDS"
#define MSK_IINF_MIO_NUM_SELECTED_CLIQUE_CUTS_              "MSK_IINF_MIO_NUM_SELECTED_CLIQUE_CUTS"
#define MSK_IINF_MIO_NUM_SELECTED_CMIR_CUTS_                "MSK_IINF_MIO_NUM_SELECTED_CMIR_CUTS"
#define MSK_IINF_MIO_NUM_SELECTED_GOMORY_CUTS_              "MSK_IINF_MIO_NUM_SELECTED_GOMORY_CUTS"
#define MSK_IINF_MIO_NUM_SELECTED_IMPLIED_BOUND_CUTS_       "MSK_IINF_MIO_NUM_SELECTED_IMPLIED_BOUND_CUTS"
#define MSK_IINF_MIO_NUM_SELECTED_KNAPSACK_COVER_CUTS_      "MSK_IINF_MIO_NUM_SELECTED_KNAPSACK_COVER_CUTS"
#define MSK_IINF_MIO_NUM_SELECTED_LIPRO_CUTS_               "MSK_IINF_MIO_NUM_SELECTED_LIPRO_CUTS"
#define MSK_IINF_MIO_NUM_SEPARATED_CLIQUE_CUTS_             "MSK_IINF_MIO_NUM_SEPARATED_CLIQUE_CUTS"
#define MSK_IINF_MIO_NUM_SEPARATED_CMIR_CUTS_               "MSK_IINF_MIO_NUM_SEPARATED_CMIR_CUTS"
#define MSK_IINF_MIO_NUM_SEPARATED_GOMORY_CUTS_             "MSK_IINF_MIO_NUM_SEPARATED_GOMORY_CUTS"
#define MSK_IINF_MIO_NUM_SEPARATED_IMPLIED_BOUND_CUTS_      "MSK_IINF_MIO_NUM_SEPARATED_IMPLIED_BOUND_CUTS"
#define MSK_IINF_MIO_NUM_SEPARATED_KNAPSACK_COVER_CUTS_     "MSK_IINF_MIO_NUM_SEPARATED_KNAPSACK_COVER_CUTS"
#define MSK_IINF_MIO_NUM_SEPARATED_LIPRO_CUTS_              "MSK_IINF_MIO_NUM_SEPARATED_LIPRO_CUTS"
#define MSK_IINF_MIO_NUM_SOLVED_NODES_                      "MSK_IINF_MIO_NUM_SOLVED_NODES"
#define MSK_IINF_MIO_NUMBIN_                                "MSK_IINF_MIO_NUMBIN"
#define MSK_IINF_MIO_NUMBINCONEVAR_                         "MSK_IINF_MIO_NUMBINCONEVAR"
#define MSK_IINF_MIO_NUMCON_                                "MSK_IINF_MIO_NUMCON"
#define MSK_IINF_MIO_NUMCONE_                               "MSK_IINF_MIO_NUMCONE"
#define MSK_IINF_MIO_NUMCONEVAR_                            "MSK_IINF_MIO_NUMCONEVAR"
#define MSK_IINF_MIO_NUMCONT_                               "MSK_IINF_MIO_NUMCONT"
#define MSK_IINF_MIO_NUMCONTCONEVAR_                        "MSK_IINF_MIO_NUMCONTCONEVAR"
#define MSK_IINF_MIO_NUMDEXPCONES_                          "MSK_IINF_MIO_NUMDEXPCONES"
#define MSK_IINF_MIO_NUMDJC_                                "MSK_IINF_MIO_NUMDJC"
#define MSK_IINF_MIO_NUMDPOWCONES_                          "MSK_IINF_MIO_NUMDPOWCONES"
#define MSK_IINF_MIO_NUMINT_                                "MSK_IINF_MIO_NUMINT"
#define MSK_IINF_MIO_NUMINTCONEVAR_                         "MSK_IINF_MIO_NUMINTCONEVAR"
#define MSK_IINF_MIO_NUMPEXPCONES_                          "MSK_IINF_MIO_NUMPEXPCONES"
#define MSK_IINF_MIO_NUMPPOWCONES_                          "MSK_IINF_MIO_NUMPPOWCONES"
#define MSK_IINF_MIO_NUMQCONES_                             "MSK_IINF_MIO_NUMQCONES"
#define MSK_IINF_MIO_NUMRQCONES_                            "MSK_IINF_MIO_NUMRQCONES"
#define MSK_IINF_MIO_NUMVAR_                                "MSK_IINF_MIO_NUMVAR"
#define MSK_IINF_MIO_OBJ_BOUND_DEFINED_                     "MSK_IINF_MIO_OBJ_BOUND_DEFINED"
#define MSK_IINF_MIO_PRESOLVED_NUMBIN_                      "MSK_IINF_MIO_PRESOLVED_NUMBIN"
#define MSK_IINF_MIO_PRESOLVED_NUMBINCONEVAR_               "MSK_IINF_MIO_PRESOLVED_NUMBINCONEVAR"
#define MSK_IINF_MIO_PRESOLVED_NUMCON_                      "MSK_IINF_MIO_PRESOLVED_NUMCON"
#define MSK_IINF_MIO_PRESOLVED_NUMCONE_                     "MSK_IINF_MIO_PRESOLVED_NUMCONE"
#define MSK_IINF_MIO_PRESOLVED_NUMCONEVAR_                  "MSK_IINF_MIO_PRESOLVED_NUMCONEVAR"
#define MSK_IINF_MIO_PRESOLVED_NUMCONT_                     "MSK_IINF_MIO_PRESOLVED_NUMCONT"
#define MSK_IINF_MIO_PRESOLVED_NUMCONTCONEVAR_              "MSK_IINF_MIO_PRESOLVED_NUMCONTCONEVAR"
#define MSK_IINF_MIO_PRESOLVED_NUMDEXPCONES_                "MSK_IINF_MIO_PRESOLVED_NUMDEXPCONES"
#define MSK_IINF_MIO_PRESOLVED_NUMDJC_                      "MSK_IINF_MIO_PRESOLVED_NUMDJC"
#define MSK_IINF_MIO_PRESOLVED_NUMDPOWCONES_                "MSK_IINF_MIO_PRESOLVED_NUMDPOWCONES"
#define MSK_IINF_MIO_PRESOLVED_NUMINT_                      "MSK_IINF_MIO_PRESOLVED_NUMINT"
#define MSK_IINF_MIO_PRESOLVED_NUMINTCONEVAR_               "MSK_IINF_MIO_PRESOLVED_NUMINTCONEVAR"
#define MSK_IINF_MIO_PRESOLVED_NUMPEXPCONES_                "MSK_IINF_MIO_PRESOLVED_NUMPEXPCONES"
#define MSK_IINF_MIO_PRESOLVED_NUMPPOWCONES_                "MSK_IINF_MIO_PRESOLVED_NUMPPOWCONES"
#define MSK_IINF_MIO_PRESOLVED_NUMQCONES_                   "MSK_IINF_MIO_PRESOLVED_NUMQCONES"
#define MSK_IINF_MIO_PRESOLVED_NUMRQCONES_                  "MSK_IINF_MIO_PRESOLVED_NUMRQCONES"
#define MSK_IINF_MIO_PRESOLVED_NUMVAR_                      "MSK_IINF_MIO_PRESOLVED_NUMVAR"
#define MSK_IINF_MIO_RELGAP_SATISFIED_                      "MSK_IINF_MIO_RELGAP_SATISFIED"
#define MSK_IINF_MIO_TOTAL_NUM_SELECTED_CUTS_               "MSK_IINF_MIO_TOTAL_NUM_SELECTED_CUTS"
#define MSK_IINF_MIO_TOTAL_NUM_SEPARATED_CUTS_              "MSK_IINF_MIO_TOTAL_NUM_SEPARATED_CUTS"
#define MSK_IINF_MIO_USER_OBJ_CUT_                          "MSK_IINF_MIO_USER_OBJ_CUT"
#define MSK_IINF_OPT_NUMCON_                                "MSK_IINF_OPT_NUMCON"
#define MSK_IINF_OPT_NUMVAR_                                "MSK_IINF_OPT_NUMVAR"
#define MSK_IINF_OPTIMIZE_RESPONSE_                         "MSK_IINF_OPTIMIZE_RESPONSE"
#define MSK_IINF_PRESOLVE_NUM_PRIMAL_PERTURBATIONS_         "MSK_IINF_PRESOLVE_NUM_PRIMAL_PERTURBATIONS"
#define MSK_IINF_PURIFY_DUAL_SUCCESS_                       "MSK_IINF_PURIFY_DUAL_SUCCESS"
#define MSK_IINF_PURIFY_PRIMAL_SUCCESS_                     "MSK_IINF_PURIFY_PRIMAL_SUCCESS"
#define MSK_IINF_RD_NUMBARVAR_                              "MSK_IINF_RD_NUMBARVAR"
#define MSK_IINF_RD_NUMCON_                                 "MSK_IINF_RD_NUMCON"
#define MSK_IINF_RD_NUMCONE_                                "MSK_IINF_RD_NUMCONE"
#define MSK_IINF_RD_NUMINTVAR_                              "MSK_IINF_RD_NUMINTVAR"
#define MSK_IINF_RD_NUMQ_                                   "MSK_IINF_RD_NUMQ"
#define MSK_IINF_RD_NUMVAR_                                 "MSK_IINF_RD_NUMVAR"
#define MSK_IINF_RD_PROTYPE_                                "MSK_IINF_RD_PROTYPE"
#define MSK_IINF_SIM_DUAL_DEG_ITER_                         "MSK_IINF_SIM_DUAL_DEG_ITER"
#define MSK_IINF_SIM_DUAL_HOTSTART_                         "MSK_IINF_SIM_DUAL_HOTSTART"
#define MSK_IINF_SIM_DUAL_HOTSTART_LU_                      "MSK_IINF_SIM_DUAL_HOTSTART_LU"
#define MSK_IINF_SIM_DUAL_INF_ITER_                         "MSK_IINF_SIM_DUAL_INF_ITER"
#define MSK_IINF_SIM_DUAL_ITER_                             "MSK_IINF_SIM_DUAL_ITER"
#define MSK_IINF_SIM_NUMCON_                                "MSK_IINF_SIM_NUMCON"
#define MSK_IINF_SIM_NUMVAR_                                "MSK_IINF_SIM_NUMVAR"
#define MSK_IINF_SIM_PRIMAL_DEG_ITER_                       "MSK_IINF_SIM_PRIMAL_DEG_ITER"
#define MSK_IINF_SIM_PRIMAL_HOTSTART_                       "MSK_IINF_SIM_PRIMAL_HOTSTART"
#define MSK_IINF_SIM_PRIMAL_HOTSTART_LU_                    "MSK_IINF_SIM_PRIMAL_HOTSTART_LU"
#define MSK_IINF_SIM_PRIMAL_INF_ITER_                       "MSK_IINF_SIM_PRIMAL_INF_ITER"
#define MSK_IINF_SIM_PRIMAL_ITER_                           "MSK_IINF_SIM_PRIMAL_ITER"
#define MSK_IINF_SIM_SOLVE_DUAL_                            "MSK_IINF_SIM_SOLVE_DUAL"
#define MSK_IINF_SOL_BAS_PROSTA_                            "MSK_IINF_SOL_BAS_PROSTA"
#define MSK_IINF_SOL_BAS_SOLSTA_                            "MSK_IINF_SOL_BAS_SOLSTA"
#define MSK_IINF_SOL_ITG_PROSTA_                            "MSK_IINF_SOL_ITG_PROSTA"
#define MSK_IINF_SOL_ITG_SOLSTA_                            "MSK_IINF_SOL_ITG_SOLSTA"
#define MSK_IINF_SOL_ITR_PROSTA_                            "MSK_IINF_SOL_ITR_PROSTA"
#define MSK_IINF_SOL_ITR_SOLSTA_                            "MSK_IINF_SOL_ITR_SOLSTA"
#define MSK_IINF_STO_NUM_A_REALLOC_                         "MSK_IINF_STO_NUM_A_REALLOC"

#define MSK_DINF_ANA_PRO_SCALARIZED_CONSTRAINT_MATRIX_DENSITY_ "MSK_DINF_ANA_PRO_SCALARIZED_CONSTRAINT_MATRIX_DENSITY"
#define MSK_DINF_BI_CLEAN_TIME_                             "MSK_DINF_BI_CLEAN_TIME"
#define MSK_DINF_BI_DUAL_TIME_                              "MSK_DINF_BI_DUAL_TIME"
#define MSK_DINF_BI_PRIMAL_TIME_                            "MSK_DINF_BI_PRIMAL_TIME"
#define MSK_DINF_BI_TIME_                                   "MSK_DINF_BI_TIME"
#define MSK_DINF_FOLDING_BI_OPTIMIZE_TIME_                  "MSK_DINF_FOLDING_BI_OPTIMIZE_TIME"
#define MSK_DINF_FOLDING_BI_UNFOLD_DUAL_TIME_               "MSK_DINF_FOLDING_BI_UNFOLD_DUAL_TIME"
#define MSK_DINF_FOLDING_BI_UNFOLD_INITIALIZE_TIME_         "MSK_DINF_FOLDING_BI_UNFOLD_INITIALIZE_TIME"
#define MSK_DINF_FOLDING_BI_UNFOLD_PRIMAL_TIME_             "MSK_DINF_FOLDING_BI_UNFOLD_PRIMAL_TIME"
#define MSK_DINF_FOLDING_BI_UNFOLD_TIME_                    "MSK_DINF_FOLDING_BI_UNFOLD_TIME"
#define MSK_DINF_FOLDING_FACTOR_                            "MSK_DINF_FOLDING_FACTOR"
#define MSK_DINF_FOLDING_TIME_                              "MSK_DINF_FOLDING_TIME"
#define MSK_DINF_INTPNT_DUAL_FEAS_                          "MSK_DINF_INTPNT_DUAL_FEAS"
#define MSK_DINF_INTPNT_DUAL_OBJ_                           "MSK_DINF_INTPNT_DUAL_OBJ"
#define MSK_DINF_INTPNT_FACTOR_NUM_FLOPS_                   "MSK_DINF_INTPNT_FACTOR_NUM_FLOPS"
#define MSK_DINF_INTPNT_OPT_STATUS_                         "MSK_DINF_INTPNT_OPT_STATUS"
#define MSK_DINF_INTPNT_ORDER_TIME_                         "MSK_DINF_INTPNT_ORDER_TIME"
#define MSK_DINF_INTPNT_PRIMAL_FEAS_                        "MSK_DINF_INTPNT_PRIMAL_FEAS"
#define MSK_DINF_INTPNT_PRIMAL_OBJ_                         "MSK_DINF_INTPNT_PRIMAL_OBJ"
#define MSK_DINF_INTPNT_TIME_                               "MSK_DINF_INTPNT_TIME"
#define MSK_DINF_MIO_CLIQUE_SELECTION_TIME_                 "MSK_DINF_MIO_CLIQUE_SELECTION_TIME"
#define MSK_DINF_MIO_CLIQUE_SEPARATION_TIME_                "MSK_DINF_MIO_CLIQUE_SEPARATION_TIME"
#define MSK_DINF_MIO_CMIR_SELECTION_TIME_                   "MSK_DINF_MIO_CMIR_SELECTION_TIME"
#define MSK_DINF_MIO_CMIR_SEPARATION_TIME_                  "MSK_DINF_MIO_CMIR_SEPARATION_TIME"
#define MSK_DINF_MIO_CONSTRUCT_SOLUTION_OBJ_                "MSK_DINF_MIO_CONSTRUCT_SOLUTION_OBJ"
#define MSK_DINF_MIO_DUAL_BOUND_AFTER_PRESOLVE_             "MSK_DINF_MIO_DUAL_BOUND_AFTER_PRESOLVE"
#define MSK_DINF_MIO_GMI_SELECTION_TIME_                    "MSK_DINF_MIO_GMI_SELECTION_TIME"
#define MSK_DINF_MIO_GMI_SEPARATION_TIME_                   "MSK_DINF_MIO_GMI_SEPARATION_TIME"
#define MSK_DINF_MIO_IMPLIED_BOUND_SELECTION_TIME_          "MSK_DINF_MIO_IMPLIED_BOUND_SELECTION_TIME"
#define MSK_DINF_MIO_IMPLIED_BOUND_SEPARATION_TIME_         "MSK_DINF_MIO_IMPLIED_BOUND_SEPARATION_TIME"
#define MSK_DINF_MIO_INITIAL_FEASIBLE_SOLUTION_OBJ_         "MSK_DINF_MIO_INITIAL_FEASIBLE_SOLUTION_OBJ"
#define MSK_DINF_MIO_KNAPSACK_COVER_SELECTION_TIME_         "MSK_DINF_MIO_KNAPSACK_COVER_SELECTION_TIME"
#define MSK_DINF_MIO_KNAPSACK_COVER_SEPARATION_TIME_        "MSK_DINF_MIO_KNAPSACK_COVER_SEPARATION_TIME"
#define MSK_DINF_MIO_LIPRO_SELECTION_TIME_                  "MSK_DINF_MIO_LIPRO_SELECTION_TIME"
#define MSK_DINF_MIO_LIPRO_SEPARATION_TIME_                 "MSK_DINF_MIO_LIPRO_SEPARATION_TIME"
#define MSK_DINF_MIO_OBJ_ABS_GAP_                           "MSK_DINF_MIO_OBJ_ABS_GAP"
#define MSK_DINF_MIO_OBJ_BOUND_                             "MSK_DINF_MIO_OBJ_BOUND"
#define MSK_DINF_MIO_OBJ_INT_                               "MSK_DINF_MIO_OBJ_INT"
#define MSK_DINF_MIO_OBJ_REL_GAP_                           "MSK_DINF_MIO_OBJ_REL_GAP"
#define MSK_DINF_MIO_PROBING_TIME_                          "MSK_DINF_MIO_PROBING_TIME"
#define MSK_DINF_MIO_ROOT_CUT_SELECTION_TIME_               "MSK_DINF_MIO_ROOT_CUT_SELECTION_TIME"
#define MSK_DINF_MIO_ROOT_CUT_SEPARATION_TIME_              "MSK_DINF_MIO_ROOT_CUT_SEPARATION_TIME"
#define MSK_DINF_MIO_ROOT_OPTIMIZER_TIME_                   "MSK_DINF_MIO_ROOT_OPTIMIZER_TIME"
#define MSK_DINF_MIO_ROOT_PRESOLVE_TIME_                    "MSK_DINF_MIO_ROOT_PRESOLVE_TIME"
#define MSK_DINF_MIO_ROOT_TIME_                             "MSK_DINF_MIO_ROOT_TIME"
#define MSK_DINF_MIO_SYMMETRY_DETECTION_TIME_               "MSK_DINF_MIO_SYMMETRY_DETECTION_TIME"
#define MSK_DINF_MIO_SYMMETRY_FACTOR_                       "MSK_DINF_MIO_SYMMETRY_FACTOR"
#define MSK_DINF_MIO_TIME_                                  "MSK_DINF_MIO_TIME"
#define MSK_DINF_MIO_USER_OBJ_CUT_                          "MSK_DINF_MIO_USER_OBJ_CUT"
#define MSK_DINF_OPTIMIZER_TICKS_                           "MSK_DINF_OPTIMIZER_TICKS"
#define MSK_DINF_OPTIMIZER_TIME_                            "MSK_DINF_OPTIMIZER_TIME"
#define MSK_DINF_PRESOLVE_ELI_TIME_                         "MSK_DINF_PRESOLVE_ELI_TIME"
#define MSK_DINF_PRESOLVE_LINDEP_TIME_                      "MSK_DINF_PRESOLVE_LINDEP_TIME"
#define MSK_DINF_PRESOLVE_TIME_                             "MSK_DINF_PRESOLVE_TIME"
#define MSK_DINF_PRESOLVE_TOTAL_PRIMAL_PERTURBATION_        "MSK_DINF_PRESOLVE_TOTAL_PRIMAL_PERTURBATION"
#define MSK_DINF_PRIMAL_REPAIR_PENALTY_OBJ_                 "MSK_DINF_PRIMAL_REPAIR_PENALTY_OBJ"
#define MSK_DINF_QCQO_REFORMULATE_MAX_PERTURBATION_         "MSK_DINF_QCQO_REFORMULATE_MAX_PERTURBATION"
#define MSK_DINF_QCQO_REFORMULATE_TIME_                     "MSK_DINF_QCQO_REFORMULATE_TIME"
#define MSK_DINF_QCQO_REFORMULATE_WORST_CHOLESKY_COLUMN_SCALING_ "MSK_DINF_QCQO_REFORMULATE_WORST_CHOLESKY_COLUMN_SCALING"
#define MSK_DINF_QCQO_REFORMULATE_WORST_CHOLESKY_DIAG_SCALING_ "MSK_DINF_QCQO_REFORMULATE_WORST_CHOLESKY_DIAG_SCALING"
#define MSK_DINF_READ_DATA_TIME_                            "MSK_DINF_READ_DATA_TIME"
#define MSK_DINF_REMOTE_TIME_                               "MSK_DINF_REMOTE_TIME"
#define MSK_DINF_SIM_DUAL_TIME_                             "MSK_DINF_SIM_DUAL_TIME"
#define MSK_DINF_SIM_FEAS_                                  "MSK_DINF_SIM_FEAS"
#define MSK_DINF_SIM_OBJ_                                   "MSK_DINF_SIM_OBJ"
#define MSK_DINF_SIM_PRIMAL_TIME_                           "MSK_DINF_SIM_PRIMAL_TIME"
#define MSK_DINF_SIM_TIME_                                  "MSK_DINF_SIM_TIME"
#define MSK_DINF_SOL_BAS_DUAL_OBJ_                          "MSK_DINF_SOL_BAS_DUAL_OBJ"
#define MSK_DINF_SOL_BAS_DVIOLCON_                          "MSK_DINF_SOL_BAS_DVIOLCON"
#define MSK_DINF_SOL_BAS_DVIOLVAR_                          "MSK_DINF_SOL_BAS_DVIOLVAR"
#define MSK_DINF_SOL_BAS_NRM_BARX_                          "MSK_DINF_SOL_BAS_NRM_BARX"
#define MSK_DINF_SOL_BAS_NRM_SLC_                           "MSK_DINF_SOL_BAS_NRM_SLC"
#define MSK_DINF_SOL_BAS_NRM_SLX_                           "MSK_DINF_SOL_BAS_NRM_SLX"
#define MSK_DINF_SOL_BAS_NRM_SUC_                           "MSK_DINF_SOL_BAS_NRM_SUC"
#define MSK_DINF_SOL_BAS_NRM_SUX_                           "MSK_DINF_SOL_BAS_NRM_SUX"
#define MSK_DINF_SOL_BAS_NRM_XC_                            "MSK_DINF_SOL_BAS_NRM_XC"
#define MSK_DINF_SOL_BAS_NRM_XX_                            "MSK_DINF_SOL_BAS_NRM_XX"
#define MSK_DINF_SOL_BAS_NRM_Y_                             "MSK_DINF_SOL_BAS_NRM_Y"
#define MSK_DINF_SOL_BAS_PRIMAL_OBJ_                        "MSK_DINF_SOL_BAS_PRIMAL_OBJ"
#define MSK_DINF_SOL_BAS_PVIOLCON_                          "MSK_DINF_SOL_BAS_PVIOLCON"
#define MSK_DINF_SOL_BAS_PVIOLVAR_                          "MSK_DINF_SOL_BAS_PVIOLVAR"
#define MSK_DINF_SOL_ITG_NRM_BARX_                          "MSK_DINF_SOL_ITG_NRM_BARX"
#define MSK_DINF_SOL_ITG_NRM_XC_                            "MSK_DINF_SOL_ITG_NRM_XC"
#define MSK_DINF_SOL_ITG_NRM_XX_                            "MSK_DINF_SOL_ITG_NRM_XX"
#define MSK_DINF_SOL_ITG_PRIMAL_OBJ_                        "MSK_DINF_SOL_ITG_PRIMAL_OBJ"
#define MSK_DINF_SOL_ITG_PVIOLACC_                          "MSK_DINF_SOL_ITG_PVIOLACC"
#define MSK_DINF_SOL_ITG_PVIOLBARVAR_                       "MSK_DINF_SOL_ITG_PVIOLBARVAR"
#define MSK_DINF_SOL_ITG_PVIOLCON_                          "MSK_DINF_SOL_ITG_PVIOLCON"
#define MSK_DINF_SOL_ITG_PVIOLCONES_                        "MSK_DINF_SOL_ITG_PVIOLCONES"
#define MSK_DINF_SOL_ITG_PVIOLDJC_                          "MSK_DINF_SOL_ITG_PVIOLDJC"
#define MSK_DINF_SOL_ITG_PVIOLITG_                          "MSK_DINF_SOL_ITG_PVIOLITG"
#define MSK_DINF_SOL_ITG_PVIOLVAR_                          "MSK_DINF_SOL_ITG_PVIOLVAR"
#define MSK_DINF_SOL_ITR_DUAL_OBJ_                          "MSK_DINF_SOL_ITR_DUAL_OBJ"
#define MSK_DINF_SOL_ITR_DVIOLACC_                          "MSK_DINF_SOL_ITR_DVIOLACC"
#define MSK_DINF_SOL_ITR_DVIOLBARVAR_                       "MSK_DINF_SOL_ITR_DVIOLBARVAR"
#define MSK_DINF_SOL_ITR_DVIOLCON_                          "MSK_DINF_SOL_ITR_DVIOLCON"
#define MSK_DINF_SOL_ITR_DVIOLCONES_                        "MSK_DINF_SOL_ITR_DVIOLCONES"
#define MSK_DINF_SOL_ITR_DVIOLVAR_                          "MSK_DINF_SOL_ITR_DVIOLVAR"
#define MSK_DINF_SOL_ITR_NRM_BARS_                          "MSK_DINF_SOL_ITR_NRM_BARS"
#define MSK_DINF_SOL_ITR_NRM_BARX_                          "MSK_DINF_SOL_ITR_NRM_BARX"
#define MSK_DINF_SOL_ITR_NRM_SLC_                           "MSK_DINF_SOL_ITR_NRM_SLC"
#define MSK_DINF_SOL_ITR_NRM_SLX_                           "MSK_DINF_SOL_ITR_NRM_SLX"
#define MSK_DINF_SOL_ITR_NRM_SNX_                           "MSK_DINF_SOL_ITR_NRM_SNX"
#define MSK_DINF_SOL_ITR_NRM_SUC_                           "MSK_DINF_SOL_ITR_NRM_SUC"
#define MSK_DINF_SOL_ITR_NRM_SUX_                           "MSK_DINF_SOL_ITR_NRM_SUX"
#define MSK_DINF_SOL_ITR_NRM_XC_                            "MSK_DINF_SOL_ITR_NRM_XC"
#define MSK_DINF_SOL_ITR_NRM_XX_                            "MSK_DINF_SOL_ITR_NRM_XX"
#define MSK_DINF_SOL_ITR_NRM_Y_                             "MSK_DINF_SOL_ITR_NRM_Y"
#define MSK_DINF_SOL_ITR_PRIMAL_OBJ_                        "MSK_DINF_SOL_ITR_PRIMAL_OBJ"
#define MSK_DINF_SOL_ITR_PVIOLACC_                          "MSK_DINF_SOL_ITR_PVIOLACC"
#define MSK_DINF_SOL_ITR_PVIOLBARVAR_                       "MSK_DINF_SOL_ITR_PVIOLBARVAR"
#define MSK_DINF_SOL_ITR_PVIOLCON_                          "MSK_DINF_SOL_ITR_PVIOLCON"
#define MSK_DINF_SOL_ITR_PVIOLCONES_                        "MSK_DINF_SOL_ITR_PVIOLCONES"
#define MSK_DINF_SOL_ITR_PVIOLVAR_                          "MSK_DINF_SOL_ITR_PVIOLVAR"
#define MSK_DINF_TO_CONIC_TIME_                             "MSK_DINF_TO_CONIC_TIME"
#define MSK_DINF_WRITE_DATA_TIME_                           "MSK_DINF_WRITE_DATA_TIME"

#define MSK_LIINF_ANA_PRO_SCALARIZED_CONSTRAINT_MATRIX_NUM_COLUMNS_ "MSK_LIINF_ANA_PRO_SCALARIZED_CONSTRAINT_MATRIX_NUM_COLUMNS"
#define MSK_LIINF_ANA_PRO_SCALARIZED_CONSTRAINT_MATRIX_NUM_NZ_ "MSK_LIINF_ANA_PRO_SCALARIZED_CONSTRAINT_MATRIX_NUM_NZ"
#define MSK_LIINF_ANA_PRO_SCALARIZED_CONSTRAINT_MATRIX_NUM_ROWS_ "MSK_LIINF_ANA_PRO_SCALARIZED_CONSTRAINT_MATRIX_NUM_ROWS"
#define MSK_LIINF_BI_CLEAN_ITER_                            "MSK_LIINF_BI_CLEAN_ITER"
#define MSK_LIINF_BI_DUAL_ITER_                             "MSK_LIINF_BI_DUAL_ITER"
#define MSK_LIINF_BI_PRIMAL_ITER_                           "MSK_LIINF_BI_PRIMAL_ITER"
#define MSK_LIINF_FOLDING_BI_DUAL_ITER_                     "MSK_LIINF_FOLDING_BI_DUAL_ITER"
#define MSK_LIINF_FOLDING_BI_OPTIMIZER_ITER_                "MSK_LIINF_FOLDING_BI_OPTIMIZER_ITER"
#define MSK_LIINF_FOLDING_BI_PRIMAL_ITER_                   "MSK_LIINF_FOLDING_BI_PRIMAL_ITER"
#define MSK_LIINF_INTPNT_FACTOR_NUM_NZ_                     "MSK_LIINF_INTPNT_FACTOR_NUM_NZ"
#define MSK_LIINF_MIO_ANZ_                                  "MSK_LIINF_MIO_ANZ"
#define MSK_LIINF_MIO_FINAL_ANZ_                            "MSK_LIINF_MIO_FINAL_ANZ"
#define MSK_LIINF_MIO_INTPNT_ITER_                          "MSK_LIINF_MIO_INTPNT_ITER"
#define MSK_LIINF_MIO_NUM_DUAL_ILLPOSED_CER_                "MSK_LIINF_MIO_NUM_DUAL_ILLPOSED_CER"
#define MSK_LIINF_MIO_NUM_PRIM_ILLPOSED_CER_                "MSK_LIINF_MIO_NUM_PRIM_ILLPOSED_CER"
#define MSK_LIINF_MIO_PRESOLVED_ANZ_                        "MSK_LIINF_MIO_PRESOLVED_ANZ"
#define MSK_LIINF_MIO_SIMPLEX_ITER_                         "MSK_LIINF_MIO_SIMPLEX_ITER"
#define MSK_LIINF_RD_NUMACC_                                "MSK_LIINF_RD_NUMACC"
#define MSK_LIINF_RD_NUMANZ_                                "MSK_LIINF_RD_NUMANZ"
#define MSK_LIINF_RD_NUMDJC_                                "MSK_LIINF_RD_NUMDJC"
#define MSK_LIINF_RD_NUMQNZ_                                "MSK_LIINF_RD_NUMQNZ"
#define MSK_LIINF_SIMPLEX_ITER_                             "MSK_LIINF_SIMPLEX_ITER"



/* Typedefs */

typedef char       MSKchart;
typedef void     * MSKvoid_t;

#ifdef  MSKINT64
typedef MSKINT64 __mskint64;
#else
typedef long long __mskint64;
#endif

#if defined(LLONG_MAX) && LLONG_MAX <= INT_MAX
#warning "Expected (long long) to be a 64bit type. MOSEK API functions may not work."
#endif
typedef int          __mskint32;

/*
typedef unsigned int       __mskuint32;
typedef signed   int       __mskint32;
typedef unsigned long long __mskuint64;
typedef signed   long long __mskint64;
*/

/* Enumeration typedefs */
#ifndef MSK_NO_ENUMS
typedef int                     MSKbasindtypee;
typedef enum MSKboundkey_enum        MSKboundkeye;
typedef int                     MSKbranchdire;
typedef enum MSKcallbackcode_enum    MSKcallbackcodee;
typedef enum MSKcompresstype_enum    MSKcompresstypee;
typedef enum MSKconetype_enum        MSKconetypee;
typedef enum MSKdataformat_enum      MSKdataformate;
typedef enum MSKdinfitem_enum        MSKdinfiteme;
typedef enum MSKdomaintype_enum      MSKdomaintypee;
typedef enum MSKdparam_enum          MSKdparame;
typedef enum MSKfeature_enum         MSKfeaturee;
typedef int                     MSKfoldingmodee;
typedef enum MSKiinfitem_enum        MSKiinfiteme;
typedef enum MSKinftype_enum         MSKinftypee;
typedef enum MSKintpnthotstart_enum  MSKintpnthotstarte;
typedef int                     MSKiomodee;
typedef enum MSKiparam_enum          MSKiparame;
typedef enum MSKliinfitem_enum       MSKliinfiteme;
typedef enum MSKmark_enum            MSKmarke;
typedef int                     MSKmiocontsoltypee;
typedef int                     MSKmiodatapermmethode;
typedef int                     MSKmiomodee;
typedef int                     MSKmionodeseltypee;
typedef int                     MSKmiovarseltypee;
typedef int                     MSKmiqcqoreformmethode;
typedef int                     MSKmpsformate;
typedef enum MSKnametype_enum        MSKnametypee;
typedef enum MSKobjsense_enum        MSKobjsensee;
typedef int                     MSKonoffkeye;
typedef int                     MSKoptimizertypee;
typedef int                     MSKorderingtypee;
typedef enum MSKparametertype_enum   MSKparametertypee;
typedef int                     MSKpresolvemodee;
typedef enum MSKproblemitem_enum     MSKproblemiteme;
typedef enum MSKproblemtype_enum     MSKproblemtypee;
typedef enum MSKprosta_enum          MSKprostae;
typedef enum MSKrescode_enum         MSKrescodee;
typedef enum MSKrescodetype_enum     MSKrescodetypee;
typedef int                     MSKscalingmethode;
typedef int                     MSKscalingtypee;
typedef int                     MSKsensitivitytypee;
typedef enum MSKsimdegen_enum        MSKsimdegene;
typedef enum MSKsimdupvec_enum       MSKsimdupvece;
typedef enum MSKsimhotstart_enum     MSKsimhotstarte;
typedef enum MSKsimprecision_enum    MSKsimprecisione;
typedef enum MSKsimreform_enum       MSKsimreforme;
typedef int                     MSKsimseltypee;
typedef enum MSKsolformat_enum       MSKsolformate;
typedef enum MSKsolitem_enum         MSKsoliteme;
typedef enum MSKsolsta_enum          MSKsolstae;
typedef enum MSKsoltype_enum         MSKsoltypee;
typedef int                     MSKsolveforme;
typedef enum MSKsparam_enum          MSKsparame;
typedef enum MSKstakey_enum          MSKstakeye;
typedef int                     MSKstartpointtypee;
typedef enum MSKstreamtype_enum      MSKstreamtypee;
typedef enum MSKsymmattype_enum      MSKsymmattypee;
typedef enum MSKtranspose_enum       MSKtransposee;
typedef enum MSKuplo_enum            MSKuploe;
typedef int                     MSKvaluee;
typedef enum MSKvariabletype_enum    MSKvariabletypee;
#else
typedef int                     MSKbasindtypee;
typedef int                     MSKboundkeye;
typedef int                     MSKbranchdire;
typedef int                     MSKcallbackcodee;
typedef int                     MSKcompresstypee;
typedef int                     MSKconetypee;
typedef int                     MSKdataformate;
typedef int                     MSKdinfiteme;
typedef int                     MSKdomaintypee;
typedef int                     MSKdparame;
typedef int                     MSKfeaturee;
typedef int                     MSKfoldingmodee;
typedef int                     MSKiinfiteme;
typedef int                     MSKinftypee;
typedef int                     MSKintpnthotstarte;
typedef int                     MSKiomodee;
typedef int                     MSKiparame;
typedef int                     MSKliinfiteme;
typedef int                     MSKmarke;
typedef int                     MSKmiocontsoltypee;
typedef int                     MSKmiodatapermmethode;
typedef int                     MSKmiomodee;
typedef int                     MSKmionodeseltypee;
typedef int                     MSKmiovarseltypee;
typedef int                     MSKmiqcqoreformmethode;
typedef int                     MSKmpsformate;
typedef int                     MSKnametypee;
typedef int                     MSKobjsensee;
typedef int                     MSKonoffkeye;
typedef int                     MSKoptimizertypee;
typedef int                     MSKorderingtypee;
typedef int                     MSKparametertypee;
typedef int                     MSKpresolvemodee;
typedef int                     MSKproblemiteme;
typedef int                     MSKproblemtypee;
typedef int                     MSKprostae;
typedef int                     MSKrescodee;
typedef int                     MSKrescodetypee;
typedef int                     MSKscalingmethode;
typedef int                     MSKscalingtypee;
typedef int                     MSKsensitivitytypee;
typedef int                     MSKsimdegene;
typedef int                     MSKsimdupvece;
typedef int                     MSKsimhotstarte;
typedef int                     MSKsimprecisione;
typedef int                     MSKsimreforme;
typedef int                     MSKsimseltypee;
typedef int                     MSKsolformate;
typedef int                     MSKsoliteme;
typedef int                     MSKsolstae;
typedef int                     MSKsoltypee;
typedef int                     MSKsolveforme;
typedef int                     MSKsparame;
typedef int                     MSKstakeye;
typedef int                     MSKstartpointtypee;
typedef int                     MSKstreamtypee;
typedef int                     MSKsymmattypee;
typedef int                     MSKtransposee;
typedef int                     MSKuploe;
typedef int                     MSKvaluee;
typedef int                     MSKvariabletypee;
#endif

/* Simple typedefs */
struct mskenvt;
struct msktaskt;
typedef struct mskenvt MSKenv;
typedef struct msktaskt MSKtask;
typedef int MSKbooleant;

typedef MSKenv * MSKenv_t;

typedef int32_t MSKint32t;

typedef int64_t MSKint64t;

typedef double MSKrealt;

typedef char * MSKstring_t;

typedef MSKtask * MSKtask_t;

typedef void * MSKuserhandle_t;

typedef wchar_t MSKwchart;

/* Function typedefs */
typedef MSKint32t  (MSKAPI * MSKcallbackfunc) (
	MSKtask_t task,
	MSKuserhandle_t usrptr,
	MSKcallbackcodee caller,
	const MSKrealt * douinf,
	const MSKint32t * intinf,
	const MSKint64t * lintinf);

typedef void  (MSKAPI * MSKexitfunc) (
	MSKuserhandle_t usrptr,
	const char * file,
	MSKint32t line,
	const char * msg);

typedef size_t  (MSKAPI * MSKhreadfunc) (
	MSKuserhandle_t handle,
	void * dest,
	const size_t count);

typedef size_t  (MSKAPI * MSKhwritefunc) (
	MSKuserhandle_t handle,
	const void * src,
	const size_t count);

typedef MSKrescodee  (MSKAPI * MSKresponsefunc) (
	MSKuserhandle_t handle,
	MSKrescodee r,
	const char * msg);

typedef void  (MSKAPI * MSKstreamfunc) (
	MSKuserhandle_t handle,
	const char * str);




/* Functions */

/* using __cplusplus */
#ifdef __cplusplus
extern "C" {
#endif

/* MSK_analyzenames */
MSKrescodee (MSKAPI MSK_analyzenames) (
	MSKtask_t task,
	MSKstreamtypee whichstream,
	MSKnametypee nametype);

/* MSK_analyzeproblem */
MSKrescodee (MSKAPI MSK_analyzeproblem) (
	MSKtask_t task,
	MSKstreamtypee whichstream);

/* MSK_analyzesolution */
MSKrescodee (MSKAPI MSK_analyzesolution) (
	MSKtask_t task,
	MSKstreamtypee whichstream,
	MSKsoltypee whichsol);

/* MSK_appendacc */
MSKrescodee (MSKAPI MSK_appendacc) (
	MSKtask_t task,
	MSKint64t domidx,
	MSKint64t numafeidx,
	const MSKint64t * afeidxlist,
	const MSKrealt * b);

/* MSK_appendaccs */
MSKrescodee (MSKAPI MSK_appendaccs) (
	MSKtask_t task,
	MSKint64t numaccs,
	const MSKint64t * domidxs,
	MSKint64t numafeidx,
	const MSKint64t * afeidxlist,
	const MSKrealt * b);

/* MSK_appendaccseq */
MSKrescodee (MSKAPI MSK_appendaccseq) (
	MSKtask_t task,
	MSKint64t domidx,
	MSKint64t numafeidx,
	MSKint64t afeidxfirst,
	const MSKrealt * b);

/* MSK_appendaccsseq */
MSKrescodee (MSKAPI MSK_appendaccsseq) (
	MSKtask_t task,
	MSKint64t numaccs,
	const MSKint64t * domidxs,
	MSKint64t numafeidx,
	MSKint64t afeidxfirst,
	const MSKrealt * b);

/* MSK_appendafes */
MSKrescodee (MSKAPI MSK_appendafes) (
	MSKtask_t task,
	MSKint64t num);

/* MSK_appendbarvars */
MSKrescodee (MSKAPI MSK_appendbarvars) (
	MSKtask_t task,
	MSKint32t num,
	const MSKint32t * dim);

/* MSK_appendcone */
MSKrescodee (MSKAPI MSK_appendcone) (
	MSKtask_t task,
	MSKconetypee ct,
	MSKrealt conepar,
	MSKint32t nummem,
	const MSKint32t * submem);

/* MSK_appendconeseq */
MSKrescodee (MSKAPI MSK_appendconeseq) (
	MSKtask_t task,
	MSKconetypee ct,
	MSKrealt conepar,
	MSKint32t nummem,
	MSKint32t j);

/* MSK_appendconesseq */
MSKrescodee (MSKAPI MSK_appendconesseq) (
	MSKtask_t task,
	MSKint32t num,
	const MSKconetypee * ct,
	const MSKrealt * conepar,
	const MSKint32t * nummem,
	MSKint32t j);

/* MSK_appendcons */
MSKrescodee (MSKAPI MSK_appendcons) (
	MSKtask_t task,
	MSKint32t num);

/* MSK_appenddjcs */
MSKrescodee (MSKAPI MSK_appenddjcs) (
	MSKtask_t task,
	MSKint64t num);

/* MSK_appenddualexpconedomain */
MSKrescodee (MSKAPI MSK_appenddualexpconedomain) (
	MSKtask_t task,
	MSKint64t * domidx);

/* MSK_appenddualgeomeanconedomain */
MSKrescodee (MSKAPI MSK_appenddualgeomeanconedomain) (
	MSKtask_t task,
	MSKint64t n,
	MSKint64t * domidx);

/* MSK_appenddualpowerconedomain */
MSKrescodee (MSKAPI MSK_appenddualpowerconedomain) (
	MSKtask_t task,
	MSKint64t n,
	MSKint64t nleft,
	const MSKrealt * alpha,
	MSKint64t * domidx);

/* MSK_appendprimalexpconedomain */
MSKrescodee (MSKAPI MSK_appendprimalexpconedomain) (
	MSKtask_t task,
	MSKint64t * domidx);

/* MSK_appendprimalgeomeanconedomain */
MSKrescodee (MSKAPI MSK_appendprimalgeomeanconedomain) (
	MSKtask_t task,
	MSKint64t n,
	MSKint64t * domidx);

/* MSK_appendprimalpowerconedomain */
MSKrescodee (MSKAPI MSK_appendprimalpowerconedomain) (
	MSKtask_t task,
	MSKint64t n,
	MSKint64t nleft,
	const MSKrealt * alpha,
	MSKint64t * domidx);

/* MSK_appendquadraticconedomain */
MSKrescodee (MSKAPI MSK_appendquadraticconedomain) (
	MSKtask_t task,
	MSKint64t n,
	MSKint64t * domidx);

/* MSK_appendrdomain */
MSKrescodee (MSKAPI MSK_appendrdomain) (
	MSKtask_t task,
	MSKint64t n,
	MSKint64t * domidx);

/* MSK_appendrminusdomain */
MSKrescodee (MSKAPI MSK_appendrminusdomain) (
	MSKtask_t task,
	MSKint64t n,
	MSKint64t * domidx);

/* MSK_appendrplusdomain */
MSKrescodee (MSKAPI MSK_appendrplusdomain) (
	MSKtask_t task,
	MSKint64t n,
	MSKint64t * domidx);

/* MSK_appendrquadraticconedomain */
MSKrescodee (MSKAPI MSK_appendrquadraticconedomain) (
	MSKtask_t task,
	MSKint64t n,
	MSKint64t * domidx);

/* MSK_appendrzerodomain */
MSKrescodee (MSKAPI MSK_appendrzerodomain) (
	MSKtask_t task,
	MSKint64t n,
	MSKint64t * domidx);

/* MSK_appendsparsesymmat */
MSKrescodee (MSKAPI MSK_appendsparsesymmat) (
	MSKtask_t task,
	MSKint32t dim,
	MSKint64t nz,
	const MSKint32t * subi,
	const MSKint32t * subj,
	const MSKrealt * valij,
	MSKint64t * idx);

/* MSK_appendsparsesymmatlist */
MSKrescodee (MSKAPI MSK_appendsparsesymmatlist) (
	MSKtask_t task,
	MSKint32t num,
	const MSKint32t * dims,
	const MSKint64t * nz,
	const MSKint32t * subi,
	const MSKint32t * subj,
	const MSKrealt * valij,
	MSKint64t * idx);

/* MSK_appendsvecpsdconedomain */
MSKrescodee (MSKAPI MSK_appendsvecpsdconedomain) (
	MSKtask_t task,
	MSKint64t n,
	MSKint64t * domidx);

/* MSK_appendvars */
MSKrescodee (MSKAPI MSK_appendvars) (
	MSKtask_t task,
	MSKint32t num);

/* MSK_asyncgetlog */
MSKrescodee (MSKAPI MSK_asyncgetlog) (
	MSKtask_t task,
	const char * addr,
	const char * accesstoken,
	const char * token);

/* MSK_asyncgetresult */
MSKrescodee (MSKAPI MSK_asyncgetresult) (
	MSKtask_t task,
	const char * address,
	const char * accesstoken,
	const char * token,
	MSKbooleant * respavailable,
	MSKrescodee * resp,
	MSKrescodee * trm);

/* MSK_asyncoptimize */
MSKrescodee (MSKAPI MSK_asyncoptimize) (
	MSKtask_t task,
	const char * address,
	const char * accesstoken,
	char * token);

/* MSK_asyncpoll */
MSKrescodee (MSKAPI MSK_asyncpoll) (
	MSKtask_t task,
	const char * address,
	const char * accesstoken,
	const char * token,
	MSKbooleant * respavailable,
	MSKrescodee * resp,
	MSKrescodee * trm);

/* MSK_asyncstop */
MSKrescodee (MSKAPI MSK_asyncstop) (
	MSKtask_t task,
	const char * address,
	const char * accesstoken,
	const char * token);

/* MSK_basiscond */
MSKrescodee (MSKAPI MSK_basiscond) (
	MSKtask_t task,
	MSKrealt * nrmbasis,
	MSKrealt * nrminvbasis);

/* MSK_bktostr */
MSKrescodee (MSKAPI MSK_bktostr) (
	MSKtask_t task,
	MSKboundkeye bk,
	char * str);

/* MSK_callocdbgtask */
void * (MSKAPI MSK_callocdbgtask) (
	MSKtask_t task,
	const size_t number,
	const size_t size,
	const char * file,
	const unsigned line);

/* MSK_calloctask */
void * (MSKAPI MSK_calloctask) (
	MSKtask_t task,
	const size_t number,
	const size_t size);

/* MSK_checkmemtask */
MSKrescodee (MSKAPI MSK_checkmemtask) (
	MSKtask_t task,
	const char * file,
	MSKint32t line);

/* MSK_chgconbound */
MSKrescodee (MSKAPI MSK_chgconbound) (
	MSKtask_t task,
	MSKint32t i,
	MSKint32t lower,
	MSKint32t finite,
	MSKrealt value);

/* MSK_chgvarbound */
MSKrescodee (MSKAPI MSK_chgvarbound) (
	MSKtask_t task,
	MSKint32t j,
	MSKint32t lower,
	MSKint32t finite,
	MSKrealt value);

/* MSK_clonetask */
MSKrescodee (MSKAPI MSK_clonetask) (
	MSKtask_t task,
	MSKtask_t * clonedtask);

/* MSK_commitchanges */
MSKrescodee (MSKAPI MSK_commitchanges) (
	MSKtask_t task);

/* MSK_conetypetostr */
MSKrescodee (MSKAPI MSK_conetypetostr) (
	MSKtask_t task,
	MSKconetypee ct,
	char * str);

/* MSK_deletesolution */
MSKrescodee (MSKAPI MSK_deletesolution) (
	MSKtask_t task,
	MSKsoltypee whichsol);

/* MSK_deletetask */
MSKrescodee (MSKAPI MSK_deletetask) (
	MSKtask_t * task);

/* MSK_dualsensitivity */
MSKrescodee (MSKAPI MSK_dualsensitivity) (
	MSKtask_t task,
	MSKint32t numj,
	const MSKint32t * subj,
	MSKrealt * leftpricej,
	MSKrealt * rightpricej,
	MSKrealt * leftrangej,
	MSKrealt * rightrangej);

/* MSK_echotask */
MSKrescodee (MSKAPIVA MSK_echotask) (
	MSKtask_t task,
	MSKstreamtypee whichstream,
	const char * format,
	...);

/* MSK_emptyafebarfrow */
MSKrescodee (MSKAPI MSK_emptyafebarfrow) (
	MSKtask_t task,
	MSKint64t afeidx);

/* MSK_emptyafebarfrowlist */
MSKrescodee (MSKAPI MSK_emptyafebarfrowlist) (
	MSKtask_t task,
	MSKint64t numafeidx,
	const MSKint64t * afeidxlist);

/* MSK_emptyafefcol */
MSKrescodee (MSKAPI MSK_emptyafefcol) (
	MSKtask_t task,
	MSKint32t varidx);

/* MSK_emptyafefcollist */
MSKrescodee (MSKAPI MSK_emptyafefcollist) (
	MSKtask_t task,
	MSKint64t numvaridx,
	const MSKint32t * varidx);

/* MSK_emptyafefrow */
MSKrescodee (MSKAPI MSK_emptyafefrow) (
	MSKtask_t task,
	MSKint64t afeidx);

/* MSK_emptyafefrowlist */
MSKrescodee (MSKAPI MSK_emptyafefrowlist) (
	MSKtask_t task,
	MSKint64t numafeidx,
	const MSKint64t * afeidx);

/* MSK_evaluateacc */
MSKrescodee (MSKAPI MSK_evaluateacc) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint64t accidx,
	MSKrealt * activity);

/* MSK_evaluateaccs */
MSKrescodee (MSKAPI MSK_evaluateaccs) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKrealt * activity);

/* MSK_freedbgtask */
void (MSKAPI MSK_freedbgtask) (
	MSKtask_t task,
	void * buffer,
	const char * file,
	const unsigned line);

/* MSK_freetask */
void (MSKAPI MSK_freetask) (
	MSKtask_t task,
	void * buffer);

/* MSK_generateaccnames */
MSKrescodee (MSKAPI MSK_generateaccnames) (
	MSKtask_t task,
	MSKint64t num,
	const MSKint64t * sub,
	const char * fmt,
	MSKint32t ndims,
	const MSKint32t * dims,
	const MSKint64t * sp,
	MSKint32t numnamedaxis,
	const MSKint32t * namedaxisidxs,
	MSKint64t numnames,
	const char ** names);

/* MSK_generatebarvarnames */
MSKrescodee (MSKAPI MSK_generatebarvarnames) (
	MSKtask_t task,
	MSKint32t num,
	const MSKint32t * subj,
	const char * fmt,
	MSKint32t ndims,
	const MSKint32t * dims,
	const MSKint64t * sp,
	MSKint32t numnamedaxis,
	const MSKint32t * namedaxisidxs,
	MSKint64t numnames,
	const char ** names);

/* MSK_generateconenames */
MSKrescodee (MSKAPI MSK_generateconenames) (
	MSKtask_t task,
	MSKint32t num,
	const MSKint32t * subk,
	const char * fmt,
	MSKint32t ndims,
	const MSKint32t * dims,
	const MSKint64t * sp,
	MSKint32t numnamedaxis,
	const MSKint32t * namedaxisidxs,
	MSKint64t numnames,
	const char ** names);

/* MSK_generateconnames */
MSKrescodee (MSKAPI MSK_generateconnames) (
	MSKtask_t task,
	MSKint32t num,
	const MSKint32t * subi,
	const char * fmt,
	MSKint32t ndims,
	const MSKint32t * dims,
	const MSKint64t * sp,
	MSKint32t numnamedaxis,
	const MSKint32t * namedaxisidxs,
	MSKint64t numnames,
	const char ** names);

/* MSK_generatedjcnames */
MSKrescodee (MSKAPI MSK_generatedjcnames) (
	MSKtask_t task,
	MSKint64t num,
	const MSKint64t * sub,
	const char * fmt,
	MSKint32t ndims,
	const MSKint32t * dims,
	const MSKint64t * sp,
	MSKint32t numnamedaxis,
	const MSKint32t * namedaxisidxs,
	MSKint64t numnames,
	const char ** names);

/* MSK_generatevarnames */
MSKrescodee (MSKAPI MSK_generatevarnames) (
	MSKtask_t task,
	MSKint32t num,
	const MSKint32t * subj,
	const char * fmt,
	MSKint32t ndims,
	const MSKint32t * dims,
	const MSKint64t * sp,
	MSKint32t numnamedaxis,
	const MSKint32t * namedaxisidxs,
	MSKint64t numnames,
	const char ** names);

/* MSK_getaccafeidxlist */
MSKrescodee (MSKAPI MSK_getaccafeidxlist) (
	MSKtask_t task,
	MSKint64t accidx,
	MSKint64t * afeidxlist);

/* MSK_getaccb */
MSKrescodee (MSKAPI MSK_getaccb) (
	MSKtask_t task,
	MSKint64t accidx,
	MSKrealt * b);

/* MSK_getaccbarfblocktriplet */
MSKrescodee (MSKAPI MSK_getaccbarfblocktriplet) (
	MSKtask_t task,
	MSKint64t maxnumtrip,
	MSKint64t * numtrip,
	MSKint64t * acc_afe,
	MSKint32t * bar_var,
	MSKint32t * blk_row,
	MSKint32t * blk_col,
	MSKrealt * blk_val);

/* MSK_getaccbarfnumblocktriplets */
MSKrescodee (MSKAPI MSK_getaccbarfnumblocktriplets) (
	MSKtask_t task,
	MSKint64t * numtrip);

/* MSK_getaccdomain */
MSKrescodee (MSKAPI MSK_getaccdomain) (
	MSKtask_t task,
	MSKint64t accidx,
	MSKint64t * domidx);

/* MSK_getaccdoty */
MSKrescodee (MSKAPI MSK_getaccdoty) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint64t accidx,
	MSKrealt * doty);

/* MSK_getaccdotys */
MSKrescodee (MSKAPI MSK_getaccdotys) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKrealt * doty);

/* MSK_getaccfnumnz */
MSKrescodee (MSKAPI MSK_getaccfnumnz) (
	MSKtask_t task,
	MSKint64t * accfnnz);

/* MSK_getaccftrip */
MSKrescodee (MSKAPI MSK_getaccftrip) (
	MSKtask_t task,
	MSKint64t * frow,
	MSKint32t * fcol,
	MSKrealt * fval);

/* MSK_getaccgvector */
MSKrescodee (MSKAPI MSK_getaccgvector) (
	MSKtask_t task,
	MSKrealt * g);

/* MSK_getaccn */
MSKrescodee (MSKAPI MSK_getaccn) (
	MSKtask_t task,
	MSKint64t accidx,
	MSKint64t * n);

/* MSK_getaccname */
MSKrescodee (MSKAPI MSK_getaccname) (
	MSKtask_t task,
	MSKint64t accidx,
	MSKint32t sizename,
	char * name);

/* MSK_getaccnamelen */
MSKrescodee (MSKAPI MSK_getaccnamelen) (
	MSKtask_t task,
	MSKint64t accidx,
	MSKint32t * len);

/* MSK_getaccntot */
MSKrescodee (MSKAPI MSK_getaccntot) (
	MSKtask_t task,
	MSKint64t * n);

/* MSK_getaccs */
MSKrescodee (MSKAPI MSK_getaccs) (
	MSKtask_t task,
	MSKint64t * domidxlist,
	MSKint64t * afeidxlist,
	MSKrealt * b);

/* MSK_getacol */
MSKrescodee (MSKAPI MSK_getacol) (
	MSKtask_t task,
	MSKint32t j,
	MSKint32t * nzj,
	MSKint32t * subj,
	MSKrealt * valj);

/* MSK_getacolnumnz */
MSKrescodee (MSKAPI MSK_getacolnumnz) (
	MSKtask_t task,
	MSKint32t i,
	MSKint32t * nzj);

/* MSK_getacolslice */
MSKrescodee (MSKAPI MSK_getacolslice) (
	MSKtask_t task,
	MSKint32t first,
	MSKint32t last,
	MSKint32t maxnumnz,
	MSKint32t * ptrb,
	MSKint32t * ptre,
	MSKint32t * sub,
	MSKrealt * val);

/* MSK_getacolslice64 */
MSKrescodee (MSKAPI MSK_getacolslice64) (
	MSKtask_t task,
	MSKint32t first,
	MSKint32t last,
	MSKint64t maxnumnz,
	MSKint64t * ptrb,
	MSKint64t * ptre,
	MSKint32t * sub,
	MSKrealt * val);

/* MSK_getacolslicenumnz */
MSKrescodee (MSKAPI MSK_getacolslicenumnz) (
	MSKtask_t task,
	MSKint32t first,
	MSKint32t last,
	MSKint32t * numnz);

/* MSK_getacolslicenumnz64 */
MSKrescodee (MSKAPI MSK_getacolslicenumnz64) (
	MSKtask_t task,
	MSKint32t first,
	MSKint32t last,
	MSKint64t * numnz);

/* MSK_getacolslicetrip */
MSKrescodee (MSKAPI MSK_getacolslicetrip) (
	MSKtask_t task,
	MSKint32t first,
	MSKint32t last,
	MSKint64t maxnumnz,
	MSKint32t * subi,
	MSKint32t * subj,
	MSKrealt * val);

/* MSK_getafebarfblocktriplet */
MSKrescodee (MSKAPI MSK_getafebarfblocktriplet) (
	MSKtask_t task,
	MSKint64t maxnumtrip,
	MSKint64t * numtrip,
	MSKint64t * afeidx,
	MSKint32t * barvaridx,
	MSKint32t * subk,
	MSKint32t * subl,
	MSKrealt * valkl);

/* MSK_getafebarfnumblocktriplets */
MSKrescodee (MSKAPI MSK_getafebarfnumblocktriplets) (
	MSKtask_t task,
	MSKint64t * numtrip);

/* MSK_getafebarfnumrowentries */
MSKrescodee (MSKAPI MSK_getafebarfnumrowentries) (
	MSKtask_t task,
	MSKint64t afeidx,
	MSKint32t * numentr);

/* MSK_getafebarfrow */
MSKrescodee (MSKAPI MSK_getafebarfrow) (
	MSKtask_t task,
	MSKint64t afeidx,
	MSKint32t * barvaridx,
	MSKint64t * ptrterm,
	MSKint64t * numterm,
	MSKint64t * termidx,
	MSKrealt * termweight);

/* MSK_getafebarfrowinfo */
MSKrescodee (MSKAPI MSK_getafebarfrowinfo) (
	MSKtask_t task,
	MSKint64t afeidx,
	MSKint32t * numentr,
	MSKint64t * numterm);

/* MSK_getafefnumnz */
MSKrescodee (MSKAPI MSK_getafefnumnz) (
	MSKtask_t task,
	MSKint64t * numnz);

/* MSK_getafefrow */
MSKrescodee (MSKAPI MSK_getafefrow) (
	MSKtask_t task,
	MSKint64t afeidx,
	MSKint32t * numnz,
	MSKint32t * varidx,
	MSKrealt * val);

/* MSK_getafefrownumnz */
MSKrescodee (MSKAPI MSK_getafefrownumnz) (
	MSKtask_t task,
	MSKint64t afeidx,
	MSKint32t * numnz);

/* MSK_getafeftrip */
MSKrescodee (MSKAPI MSK_getafeftrip) (
	MSKtask_t task,
	MSKint64t * afeidx,
	MSKint32t * varidx,
	MSKrealt * val);

/* MSK_getafeg */
MSKrescodee (MSKAPI MSK_getafeg) (
	MSKtask_t task,
	MSKint64t afeidx,
	MSKrealt * g);

/* MSK_getafegslice */
MSKrescodee (MSKAPI MSK_getafegslice) (
	MSKtask_t task,
	MSKint64t first,
	MSKint64t last,
	MSKrealt * g);

/* MSK_getaij */
MSKrescodee (MSKAPI MSK_getaij) (
	MSKtask_t task,
	MSKint32t i,
	MSKint32t j,
	MSKrealt * aij);

/* MSK_getapiecenumnz */
MSKrescodee (MSKAPI MSK_getapiecenumnz) (
	MSKtask_t task,
	MSKint32t firsti,
	MSKint32t lasti,
	MSKint32t firstj,
	MSKint32t lastj,
	MSKint32t * numnz);

/* MSK_getarow */
MSKrescodee (MSKAPI MSK_getarow) (
	MSKtask_t task,
	MSKint32t i,
	MSKint32t * nzi,
	MSKint32t * subi,
	MSKrealt * vali);

/* MSK_getarownumnz */
MSKrescodee (MSKAPI MSK_getarownumnz) (
	MSKtask_t task,
	MSKint32t i,
	MSKint32t * nzi);

/* MSK_getarowslice */
MSKrescodee (MSKAPI MSK_getarowslice) (
	MSKtask_t task,
	MSKint32t first,
	MSKint32t last,
	MSKint32t maxnumnz,
	MSKint32t * ptrb,
	MSKint32t * ptre,
	MSKint32t * sub,
	MSKrealt * val);

/* MSK_getarowslice64 */
MSKrescodee (MSKAPI MSK_getarowslice64) (
	MSKtask_t task,
	MSKint32t first,
	MSKint32t last,
	MSKint64t maxnumnz,
	MSKint64t * ptrb,
	MSKint64t * ptre,
	MSKint32t * sub,
	MSKrealt * val);

/* MSK_getarowslicenumnz */
MSKrescodee (MSKAPI MSK_getarowslicenumnz) (
	MSKtask_t task,
	MSKint32t first,
	MSKint32t last,
	MSKint32t * numnz);

/* MSK_getarowslicenumnz64 */
MSKrescodee (MSKAPI MSK_getarowslicenumnz64) (
	MSKtask_t task,
	MSKint32t first,
	MSKint32t last,
	MSKint64t * numnz);

/* MSK_getarowslicetrip */
MSKrescodee (MSKAPI MSK_getarowslicetrip) (
	MSKtask_t task,
	MSKint32t first,
	MSKint32t last,
	MSKint64t maxnumnz,
	MSKint32t * subi,
	MSKint32t * subj,
	MSKrealt * val);

/* MSK_getatrip */
MSKrescodee (MSKAPI MSK_getatrip) (
	MSKtask_t task,
	MSKint64t maxnumnz,
	MSKint32t * subi,
	MSKint32t * subj,
	MSKrealt * val);

/* MSK_getatruncatetol */
MSKrescodee (MSKAPI MSK_getatruncatetol) (
	MSKtask_t task,
	MSKrealt * tolzero);

/* MSK_getbarablocktriplet */
MSKrescodee (MSKAPI MSK_getbarablocktriplet) (
	MSKtask_t task,
	MSKint64t maxnum,
	MSKint64t * num,
	MSKint32t * subi,
	MSKint32t * subj,
	MSKint32t * subk,
	MSKint32t * subl,
	MSKrealt * valijkl);

/* MSK_getbaraidx */
MSKrescodee (MSKAPI MSK_getbaraidx) (
	MSKtask_t task,
	MSKint64t idx,
	MSKint64t maxnum,
	MSKint32t * i,
	MSKint32t * j,
	MSKint64t * num,
	MSKint64t * sub,
	MSKrealt * weights);

/* MSK_getbaraidxij */
MSKrescodee (MSKAPI MSK_getbaraidxij) (
	MSKtask_t task,
	MSKint64t idx,
	MSKint32t * i,
	MSKint32t * j);

/* MSK_getbaraidxinfo */
MSKrescodee (MSKAPI MSK_getbaraidxinfo) (
	MSKtask_t task,
	MSKint64t idx,
	MSKint64t * num);

/* MSK_getbarasparsity */
MSKrescodee (MSKAPI MSK_getbarasparsity) (
	MSKtask_t task,
	MSKint64t maxnumnz,
	MSKint64t * numnz,
	MSKint64t * idxij);

/* MSK_getbarcblocktriplet */
MSKrescodee (MSKAPI MSK_getbarcblocktriplet) (
	MSKtask_t task,
	MSKint64t maxnum,
	MSKint64t * num,
	MSKint32t * subj,
	MSKint32t * subk,
	MSKint32t * subl,
	MSKrealt * valjkl);

/* MSK_getbarcidx */
MSKrescodee (MSKAPI MSK_getbarcidx) (
	MSKtask_t task,
	MSKint64t idx,
	MSKint64t maxnum,
	MSKint32t * j,
	MSKint64t * num,
	MSKint64t * sub,
	MSKrealt * weights);

/* MSK_getbarcidxinfo */
MSKrescodee (MSKAPI MSK_getbarcidxinfo) (
	MSKtask_t task,
	MSKint64t idx,
	MSKint64t * num);

/* MSK_getbarcidxj */
MSKrescodee (MSKAPI MSK_getbarcidxj) (
	MSKtask_t task,
	MSKint64t idx,
	MSKint32t * j);

/* MSK_getbarcsparsity */
MSKrescodee (MSKAPI MSK_getbarcsparsity) (
	MSKtask_t task,
	MSKint64t maxnumnz,
	MSKint64t * numnz,
	MSKint64t * idxj);

/* MSK_getbarsj */
MSKrescodee (MSKAPI MSK_getbarsj) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint32t j,
	MSKrealt * barsj);

/* MSK_getbarsslice */
MSKrescodee (MSKAPI MSK_getbarsslice) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint32t first,
	MSKint32t last,
	MSKint64t slicesize,
	MSKrealt * barsslice);

/* MSK_getbarvarname */
MSKrescodee (MSKAPI MSK_getbarvarname) (
	MSKtask_t task,
	MSKint32t i,
	MSKint32t sizename,
	char * name);

/* MSK_getbarvarnameindex */
MSKrescodee (MSKAPI MSK_getbarvarnameindex) (
	MSKtask_t task,
	const char * somename,
	MSKint32t * asgn,
	MSKint32t * index);

/* MSK_getbarvarnamelen */
MSKrescodee (MSKAPI MSK_getbarvarnamelen) (
	MSKtask_t task,
	MSKint32t i,
	MSKint32t * len);

/* MSK_getbarxj */
MSKrescodee (MSKAPI MSK_getbarxj) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint32t j,
	MSKrealt * barxj);

/* MSK_getbarxslice */
MSKrescodee (MSKAPI MSK_getbarxslice) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint32t first,
	MSKint32t last,
	MSKint64t slicesize,
	MSKrealt * barxslice);

/* MSK_getc */
MSKrescodee (MSKAPI MSK_getc) (
	MSKtask_t task,
	MSKrealt * c);

/* MSK_getcallbackfunc */
MSKrescodee (MSKAPI MSK_getcallbackfunc) (
	MSKtask_t task,
	MSKcallbackfunc * func,
	MSKuserhandle_t * handle);

/* MSK_getcfix */
MSKrescodee (MSKAPI MSK_getcfix) (
	MSKtask_t task,
	MSKrealt * cfix);

/* MSK_getcj */
MSKrescodee (MSKAPI MSK_getcj) (
	MSKtask_t task,
	MSKint32t j,
	MSKrealt * cj);

/* MSK_getclist */
MSKrescodee (MSKAPI MSK_getclist) (
	MSKtask_t task,
	MSKint32t num,
	const MSKint32t * subj,
	MSKrealt * c);

/* MSK_getconbound */
MSKrescodee (MSKAPI MSK_getconbound) (
	MSKtask_t task,
	MSKint32t i,
	MSKboundkeye * bk,
	MSKrealt * bl,
	MSKrealt * bu);

/* MSK_getconboundslice */
MSKrescodee (MSKAPI MSK_getconboundslice) (
	MSKtask_t task,
	MSKint32t first,
	MSKint32t last,
	MSKboundkeye * bk,
	MSKrealt * bl,
	MSKrealt * bu);

/* MSK_getcone */
MSKrescodee (MSKAPI MSK_getcone) (
	MSKtask_t task,
	MSKint32t k,
	MSKconetypee * ct,
	MSKrealt * conepar,
	MSKint32t * nummem,
	MSKint32t * submem);

/* MSK_getconeinfo */
MSKrescodee (MSKAPI MSK_getconeinfo) (
	MSKtask_t task,
	MSKint32t k,
	MSKconetypee * ct,
	MSKrealt * conepar,
	MSKint32t * nummem);

/* MSK_getconename */
MSKrescodee (MSKAPI MSK_getconename) (
	MSKtask_t task,
	MSKint32t i,
	MSKint32t sizename,
	char * name);

/* MSK_getconenameindex */
MSKrescodee (MSKAPI MSK_getconenameindex) (
	MSKtask_t task,
	const char * somename,
	MSKint32t * asgn,
	MSKint32t * index);

/* MSK_getconenamelen */
MSKrescodee (MSKAPI MSK_getconenamelen) (
	MSKtask_t task,
	MSKint32t i,
	MSKint32t * len);

/* MSK_getconname */
MSKrescodee (MSKAPI MSK_getconname) (
	MSKtask_t task,
	MSKint32t i,
	MSKint32t sizename,
	char * name);

/* MSK_getconnameindex */
MSKrescodee (MSKAPI MSK_getconnameindex) (
	MSKtask_t task,
	const char * somename,
	MSKint32t * asgn,
	MSKint32t * index);

/* MSK_getconnamelen */
MSKrescodee (MSKAPI MSK_getconnamelen) (
	MSKtask_t task,
	MSKint32t i,
	MSKint32t * len);

/* MSK_getcslice */
MSKrescodee (MSKAPI MSK_getcslice) (
	MSKtask_t task,
	MSKint32t first,
	MSKint32t last,
	MSKrealt * c);

/* MSK_getdimbarvarj */
MSKrescodee (MSKAPI MSK_getdimbarvarj) (
	MSKtask_t task,
	MSKint32t j,
	MSKint32t * dimbarvarj);

/* MSK_getdjcafeidxlist */
MSKrescodee (MSKAPI MSK_getdjcafeidxlist) (
	MSKtask_t task,
	MSKint64t djcidx,
	MSKint64t * afeidxlist);

/* MSK_getdjcb */
MSKrescodee (MSKAPI MSK_getdjcb) (
	MSKtask_t task,
	MSKint64t djcidx,
	MSKrealt * b);

/* MSK_getdjcdomainidxlist */
MSKrescodee (MSKAPI MSK_getdjcdomainidxlist) (
	MSKtask_t task,
	MSKint64t djcidx,
	MSKint64t * domidxlist);

/* MSK_getdjcname */
MSKrescodee (MSKAPI MSK_getdjcname) (
	MSKtask_t task,
	MSKint64t djcidx,
	MSKint32t sizename,
	char * name);

/* MSK_getdjcnamelen */
MSKrescodee (MSKAPI MSK_getdjcnamelen) (
	MSKtask_t task,
	MSKint64t djcidx,
	MSKint32t * len);

/* MSK_getdjcnumafe */
MSKrescodee (MSKAPI MSK_getdjcnumafe) (
	MSKtask_t task,
	MSKint64t djcidx,
	MSKint64t * numafe);

/* MSK_getdjcnumafetot */
MSKrescodee (MSKAPI MSK_getdjcnumafetot) (
	MSKtask_t task,
	MSKint64t * numafetot);

/* MSK_getdjcnumdomain */
MSKrescodee (MSKAPI MSK_getdjcnumdomain) (
	MSKtask_t task,
	MSKint64t djcidx,
	MSKint64t * numdomain);

/* MSK_getdjcnumdomaintot */
MSKrescodee (MSKAPI MSK_getdjcnumdomaintot) (
	MSKtask_t task,
	MSKint64t * numdomaintot);

/* MSK_getdjcnumterm */
MSKrescodee (MSKAPI MSK_getdjcnumterm) (
	MSKtask_t task,
	MSKint64t djcidx,
	MSKint64t * numterm);

/* MSK_getdjcnumtermtot */
MSKrescodee (MSKAPI MSK_getdjcnumtermtot) (
	MSKtask_t task,
	MSKint64t * numtermtot);

/* MSK_getdjcs */
MSKrescodee (MSKAPI MSK_getdjcs) (
	MSKtask_t task,
	MSKint64t * domidxlist,
	MSKint64t * afeidxlist,
	MSKrealt * b,
	MSKint64t * termsizelist,
	MSKint64t * numterms);

/* MSK_getdjctermsizelist */
MSKrescodee (MSKAPI MSK_getdjctermsizelist) (
	MSKtask_t task,
	MSKint64t djcidx,
	MSKint64t * termsizelist);

/* MSK_getdomainn */
MSKrescodee (MSKAPI MSK_getdomainn) (
	MSKtask_t task,
	MSKint64t domidx,
	MSKint64t * n);

/* MSK_getdomainname */
MSKrescodee (MSKAPI MSK_getdomainname) (
	MSKtask_t task,
	MSKint64t domidx,
	MSKint32t sizename,
	char * name);

/* MSK_getdomainnamelen */
MSKrescodee (MSKAPI MSK_getdomainnamelen) (
	MSKtask_t task,
	MSKint64t domidx,
	MSKint32t * len);

/* MSK_getdomaintype */
MSKrescodee (MSKAPI MSK_getdomaintype) (
	MSKtask_t task,
	MSKint64t domidx,
	MSKdomaintypee * domtype);

/* MSK_getdouinf */
MSKrescodee (MSKAPI MSK_getdouinf) (
	MSKtask_t task,
	MSKdinfiteme whichdinf,
	MSKrealt * dvalue);

/* MSK_getdouparam */
MSKrescodee (MSKAPI MSK_getdouparam) (
	MSKtask_t task,
	MSKdparame param,
	MSKrealt * parvalue);

/* MSK_getdualobj */
MSKrescodee (MSKAPI MSK_getdualobj) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKrealt * dualobj);

/* MSK_getdualproblem */
MSKrescodee (MSKAPI MSK_getdualproblem) (
	MSKtask_t task,
	MSKtask_t * dualtask);

/* MSK_getdualsolutionnorms */
MSKrescodee (MSKAPI MSK_getdualsolutionnorms) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKrealt * nrmy,
	MSKrealt * nrmslc,
	MSKrealt * nrmsuc,
	MSKrealt * nrmslx,
	MSKrealt * nrmsux,
	MSKrealt * nrmsnx,
	MSKrealt * nrmbars);

/* MSK_getdviolacc */
MSKrescodee (MSKAPI MSK_getdviolacc) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint64t numaccidx,
	const MSKint64t * accidxlist,
	MSKrealt * viol);

/* MSK_getdviolbarvar */
MSKrescodee (MSKAPI MSK_getdviolbarvar) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint32t num,
	const MSKint32t * sub,
	MSKrealt * viol);

/* MSK_getdviolcon */
MSKrescodee (MSKAPI MSK_getdviolcon) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint32t num,
	const MSKint32t * sub,
	MSKrealt * viol);

/* MSK_getdviolcones */
MSKrescodee (MSKAPI MSK_getdviolcones) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint32t num,
	const MSKint32t * sub,
	MSKrealt * viol);

/* MSK_getdviolvar */
MSKrescodee (MSKAPI MSK_getdviolvar) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint32t num,
	const MSKint32t * sub,
	MSKrealt * viol);

/* MSK_getenv */
MSKrescodee (MSKAPI MSK_getenv) (
	MSKtask_t task,
	MSKenv_t * env);

/* MSK_getinfeasiblesubproblem */
MSKrescodee (MSKAPI MSK_getinfeasiblesubproblem) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKtask_t * inftask);

/* MSK_getinfindex */
MSKrescodee (MSKAPI MSK_getinfindex) (
	MSKtask_t task,
	MSKinftypee inftype,
	const char * infname,
	MSKint32t * infindex);

/* MSK_getinfmax */
MSKrescodee (MSKAPI MSK_getinfmax) (
	MSKtask_t task,
	MSKinftypee inftype,
	MSKint32t * infmax);

/* MSK_getinfname */
MSKrescodee (MSKAPI MSK_getinfname) (
	MSKtask_t task,
	MSKinftypee inftype,
	MSKint32t whichinf,
	char * infname);

/* MSK_getintinf */
MSKrescodee (MSKAPI MSK_getintinf) (
	MSKtask_t task,
	MSKiinfiteme whichiinf,
	MSKint32t * ivalue);

/* MSK_getintparam */
MSKrescodee (MSKAPI MSK_getintparam) (
	MSKtask_t task,
	MSKiparame param,
	MSKint32t * parvalue);

/* MSK_getlasterror */
MSKrescodee (MSKAPI MSK_getlasterror) (
	MSKtask_t task,
	MSKrescodee * lastrescode,
	MSKint32t sizelastmsg,
	MSKint32t * lastmsglen,
	char * lastmsg);

/* MSK_getlasterror64 */
MSKrescodee (MSKAPI MSK_getlasterror64) (
	MSKtask_t task,
	MSKrescodee * lastrescode,
	MSKint64t sizelastmsg,
	MSKint64t * lastmsglen,
	char * lastmsg);

/* MSK_getlenbarvarj */
MSKrescodee (MSKAPI MSK_getlenbarvarj) (
	MSKtask_t task,
	MSKint32t j,
	MSKint64t * lenbarvarj);

/* MSK_getlintinf */
MSKrescodee (MSKAPI MSK_getlintinf) (
	MSKtask_t task,
	MSKliinfiteme whichliinf,
	MSKint64t * ivalue);

/* MSK_getlintparam */
MSKrescodee (MSKAPI MSK_getlintparam) (
	MSKtask_t task,
	MSKiparame param,
	MSKint64t * parvalue);

/* MSK_getmaxnamelen */
MSKrescodee (MSKAPI MSK_getmaxnamelen) (
	MSKtask_t task,
	MSKint32t * maxlen);

/* MSK_getmaxnumanz */
MSKrescodee (MSKAPI MSK_getmaxnumanz) (
	MSKtask_t task,
	MSKint32t * maxnumanz);

/* MSK_getmaxnumanz64 */
MSKrescodee (MSKAPI MSK_getmaxnumanz64) (
	MSKtask_t task,
	MSKint64t * maxnumanz);

/* MSK_getmaxnumbarvar */
MSKrescodee (MSKAPI MSK_getmaxnumbarvar) (
	MSKtask_t task,
	MSKint32t * maxnumbarvar);

/* MSK_getmaxnumcon */
MSKrescodee (MSKAPI MSK_getmaxnumcon) (
	MSKtask_t task,
	MSKint32t * maxnumcon);

/* MSK_getmaxnumcone */
MSKrescodee (MSKAPI MSK_getmaxnumcone) (
	MSKtask_t task,
	MSKint32t * maxnumcone);

/* MSK_getmaxnumqnz */
MSKrescodee (MSKAPI MSK_getmaxnumqnz) (
	MSKtask_t task,
	MSKint32t * maxnumqnz);

/* MSK_getmaxnumqnz64 */
MSKrescodee (MSKAPI MSK_getmaxnumqnz64) (
	MSKtask_t task,
	MSKint64t * maxnumqnz);

/* MSK_getmaxnumvar */
MSKrescodee (MSKAPI MSK_getmaxnumvar) (
	MSKtask_t task,
	MSKint32t * maxnumvar);

/* MSK_getmemusagetask */
MSKrescodee (MSKAPI MSK_getmemusagetask) (
	MSKtask_t task,
	MSKint64t * meminuse,
	MSKint64t * maxmemuse);

/* MSK_getnadouinf */
MSKrescodee (MSKAPI MSK_getnadouinf) (
	MSKtask_t task,
	const char * infitemname,
	MSKrealt * dvalue);

/* MSK_getnadouparam */
MSKrescodee (MSKAPI MSK_getnadouparam) (
	MSKtask_t task,
	const char * paramname,
	MSKrealt * parvalue);

/* MSK_getnaintinf */
MSKrescodee (MSKAPI MSK_getnaintinf) (
	MSKtask_t task,
	const char * infitemname,
	MSKint32t * ivalue);

/* MSK_getnaintparam */
MSKrescodee (MSKAPI MSK_getnaintparam) (
	MSKtask_t task,
	const char * paramname,
	MSKint32t * parvalue);

/* MSK_getnastrparam */
MSKrescodee (MSKAPI MSK_getnastrparam) (
	MSKtask_t task,
	const char * paramname,
	MSKint32t sizeparamname,
	MSKint32t * len,
	char * parvalue);

/* MSK_getnastrparamal */
MSKrescodee (MSKAPI MSK_getnastrparamal) (
	MSKtask_t task,
	const char * paramname,
	MSKint32t numaddchr,
	char ** value);

/* MSK_getnumacc */
MSKrescodee (MSKAPI MSK_getnumacc) (
	MSKtask_t task,
	MSKint64t * num);

/* MSK_getnumafe */
MSKrescodee (MSKAPI MSK_getnumafe) (
	MSKtask_t task,
	MSKint64t * numafe);

/* MSK_getnumanz */
MSKrescodee (MSKAPI MSK_getnumanz) (
	MSKtask_t task,
	MSKint32t * numanz);

/* MSK_getnumanz64 */
MSKrescodee (MSKAPI MSK_getnumanz64) (
	MSKtask_t task,
	MSKint64t * numanz);

/* MSK_getnumbarablocktriplets */
MSKrescodee (MSKAPI MSK_getnumbarablocktriplets) (
	MSKtask_t task,
	MSKint64t * num);

/* MSK_getnumbaranz */
MSKrescodee (MSKAPI MSK_getnumbaranz) (
	MSKtask_t task,
	MSKint64t * nz);

/* MSK_getnumbarcblocktriplets */
MSKrescodee (MSKAPI MSK_getnumbarcblocktriplets) (
	MSKtask_t task,
	MSKint64t * num);

/* MSK_getnumbarcnz */
MSKrescodee (MSKAPI MSK_getnumbarcnz) (
	MSKtask_t task,
	MSKint64t * nz);

/* MSK_getnumbarvar */
MSKrescodee (MSKAPI MSK_getnumbarvar) (
	MSKtask_t task,
	MSKint32t * numbarvar);

/* MSK_getnumcon */
MSKrescodee (MSKAPI MSK_getnumcon) (
	MSKtask_t task,
	MSKint32t * numcon);

/* MSK_getnumcone */
MSKrescodee (MSKAPI MSK_getnumcone) (
	MSKtask_t task,
	MSKint32t * numcone);

/* MSK_getnumconemem */
MSKrescodee (MSKAPI MSK_getnumconemem) (
	MSKtask_t task,
	MSKint32t k,
	MSKint32t * nummem);

/* MSK_getnumdjc */
MSKrescodee (MSKAPI MSK_getnumdjc) (
	MSKtask_t task,
	MSKint64t * num);

/* MSK_getnumdomain */
MSKrescodee (MSKAPI MSK_getnumdomain) (
	MSKtask_t task,
	MSKint64t * numdomain);

/* MSK_getnumintvar */
MSKrescodee (MSKAPI MSK_getnumintvar) (
	MSKtask_t task,
	MSKint32t * numintvar);

/* MSK_getnumparam */
MSKrescodee (MSKAPI MSK_getnumparam) (
	MSKtask_t task,
	MSKparametertypee partype,
	MSKint32t * numparam);

/* MSK_getnumqconknz */
MSKrescodee (MSKAPI MSK_getnumqconknz) (
	MSKtask_t task,
	MSKint32t k,
	MSKint32t * numqcnz);

/* MSK_getnumqconknz64 */
MSKrescodee (MSKAPI MSK_getnumqconknz64) (
	MSKtask_t task,
	MSKint32t k,
	MSKint64t * numqcnz);

/* MSK_getnumqobjnz */
MSKrescodee (MSKAPI MSK_getnumqobjnz) (
	MSKtask_t task,
	MSKint32t * numqonz);

/* MSK_getnumqobjnz64 */
MSKrescodee (MSKAPI MSK_getnumqobjnz64) (
	MSKtask_t task,
	MSKint64t * numqonz);

/* MSK_getnumsymmat */
MSKrescodee (MSKAPI MSK_getnumsymmat) (
	MSKtask_t task,
	MSKint64t * num);

/* MSK_getnumvar */
MSKrescodee (MSKAPI MSK_getnumvar) (
	MSKtask_t task,
	MSKint32t * numvar);

/* MSK_getobjname */
MSKrescodee (MSKAPI MSK_getobjname) (
	MSKtask_t task,
	MSKint32t sizeobjname,
	char * objname);

/* MSK_getobjnamelen */
MSKrescodee (MSKAPI MSK_getobjnamelen) (
	MSKtask_t task,
	MSKint32t * len);

/* MSK_getobjsense */
MSKrescodee (MSKAPI MSK_getobjsense) (
	MSKtask_t task,
	MSKobjsensee * sense);

/* MSK_getparammax */
MSKrescodee (MSKAPI MSK_getparammax) (
	MSKtask_t task,
	MSKparametertypee partype,
	MSKint32t * parammax);

/* MSK_getparamname */
MSKrescodee (MSKAPI MSK_getparamname) (
	MSKtask_t task,
	MSKparametertypee partype,
	MSKint32t param,
	char * parname);

/* MSK_getpowerdomainalpha */
MSKrescodee (MSKAPI MSK_getpowerdomainalpha) (
	MSKtask_t task,
	MSKint64t domidx,
	MSKrealt * alpha);

/* MSK_getpowerdomaininfo */
MSKrescodee (MSKAPI MSK_getpowerdomaininfo) (
	MSKtask_t task,
	MSKint64t domidx,
	MSKint64t * n,
	MSKint64t * nleft);

/* MSK_getprimalobj */
MSKrescodee (MSKAPI MSK_getprimalobj) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKrealt * primalobj);

/* MSK_getprimalsolutionnorms */
MSKrescodee (MSKAPI MSK_getprimalsolutionnorms) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKrealt * nrmxc,
	MSKrealt * nrmxx,
	MSKrealt * nrmbarx);

/* MSK_getprobtype */
MSKrescodee (MSKAPI MSK_getprobtype) (
	MSKtask_t task,
	MSKproblemtypee * probtype);

/* MSK_getprosta */
MSKrescodee (MSKAPI MSK_getprosta) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKprostae * problemsta);

/* MSK_getpviolacc */
MSKrescodee (MSKAPI MSK_getpviolacc) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint64t numaccidx,
	const MSKint64t * accidxlist,
	MSKrealt * viol);

/* MSK_getpviolbarvar */
MSKrescodee (MSKAPI MSK_getpviolbarvar) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint32t num,
	const MSKint32t * sub,
	MSKrealt * viol);

/* MSK_getpviolcon */
MSKrescodee (MSKAPI MSK_getpviolcon) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint32t num,
	const MSKint32t * sub,
	MSKrealt * viol);

/* MSK_getpviolcones */
MSKrescodee (MSKAPI MSK_getpviolcones) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint32t num,
	const MSKint32t * sub,
	MSKrealt * viol);

/* MSK_getpvioldjc */
MSKrescodee (MSKAPI MSK_getpvioldjc) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint64t numdjcidx,
	const MSKint64t * djcidxlist,
	MSKrealt * viol);

/* MSK_getpviolvar */
MSKrescodee (MSKAPI MSK_getpviolvar) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint32t num,
	const MSKint32t * sub,
	MSKrealt * viol);

/* MSK_getqconk */
MSKrescodee (MSKAPI MSK_getqconk) (
	MSKtask_t task,
	MSKint32t k,
	MSKint32t maxnumqcnz,
	MSKint32t * numqcnz,
	MSKint32t * qcsubi,
	MSKint32t * qcsubj,
	MSKrealt * qcval);

/* MSK_getqconk64 */
MSKrescodee (MSKAPI MSK_getqconk64) (
	MSKtask_t task,
	MSKint32t k,
	MSKint64t maxnumqcnz,
	MSKint64t * numqcnz,
	MSKint32t * qcsubi,
	MSKint32t * qcsubj,
	MSKrealt * qcval);

/* MSK_getqobj */
MSKrescodee (MSKAPI MSK_getqobj) (
	MSKtask_t task,
	MSKint32t maxnumqonz,
	MSKint32t * numqonz,
	MSKint32t * qosubi,
	MSKint32t * qosubj,
	MSKrealt * qoval);

/* MSK_getqobj64 */
MSKrescodee (MSKAPI MSK_getqobj64) (
	MSKtask_t task,
	MSKint64t maxnumqonz,
	MSKint64t * numqonz,
	MSKint32t * qosubi,
	MSKint32t * qosubj,
	MSKrealt * qoval);

/* MSK_getqobjij */
MSKrescodee (MSKAPI MSK_getqobjij) (
	MSKtask_t task,
	MSKint32t i,
	MSKint32t j,
	MSKrealt * qoij);

/* MSK_getreducedcosts */
MSKrescodee (MSKAPI MSK_getreducedcosts) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint32t first,
	MSKint32t last,
	MSKrealt * redcosts);

/* MSK_getskc */
MSKrescodee (MSKAPI MSK_getskc) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKstakeye * skc);

/* MSK_getskcslice */
MSKrescodee (MSKAPI MSK_getskcslice) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint32t first,
	MSKint32t last,
	MSKstakeye * skc);

/* MSK_getskn */
MSKrescodee (MSKAPI MSK_getskn) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKstakeye * skn);

/* MSK_getskx */
MSKrescodee (MSKAPI MSK_getskx) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKstakeye * skx);

/* MSK_getskxslice */
MSKrescodee (MSKAPI MSK_getskxslice) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint32t first,
	MSKint32t last,
	MSKstakeye * skx);

/* MSK_getslc */
MSKrescodee (MSKAPI MSK_getslc) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKrealt * slc);

/* MSK_getslcslice */
MSKrescodee (MSKAPI MSK_getslcslice) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint32t first,
	MSKint32t last,
	MSKrealt * slc);

/* MSK_getslx */
MSKrescodee (MSKAPI MSK_getslx) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKrealt * slx);

/* MSK_getslxslice */
MSKrescodee (MSKAPI MSK_getslxslice) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint32t first,
	MSKint32t last,
	MSKrealt * slx);

/* MSK_getsnx */
MSKrescodee (MSKAPI MSK_getsnx) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKrealt * snx);

/* MSK_getsnxslice */
MSKrescodee (MSKAPI MSK_getsnxslice) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint32t first,
	MSKint32t last,
	MSKrealt * snx);

/* MSK_getsolsta */
MSKrescodee (MSKAPI MSK_getsolsta) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKsolstae * solutionsta);

/* MSK_getsolution */
MSKrescodee (MSKAPI MSK_getsolution) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKprostae * problemsta,
	MSKsolstae * solutionsta,
	MSKstakeye * skc,
	MSKstakeye * skx,
	MSKstakeye * skn,
	MSKrealt * xc,
	MSKrealt * xx,
	MSKrealt * y,
	MSKrealt * slc,
	MSKrealt * suc,
	MSKrealt * slx,
	MSKrealt * sux,
	MSKrealt * snx);

/* MSK_getsolutioninfo */
MSKrescodee (MSKAPI MSK_getsolutioninfo) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKrealt * pobj,
	MSKrealt * pviolcon,
	MSKrealt * pviolvar,
	MSKrealt * pviolbarvar,
	MSKrealt * pviolcone,
	MSKrealt * pviolitg,
	MSKrealt * dobj,
	MSKrealt * dviolcon,
	MSKrealt * dviolvar,
	MSKrealt * dviolbarvar,
	MSKrealt * dviolcone);

/* MSK_getsolutioninfonew */
MSKrescodee (MSKAPI MSK_getsolutioninfonew) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKrealt * pobj,
	MSKrealt * pviolcon,
	MSKrealt * pviolvar,
	MSKrealt * pviolbarvar,
	MSKrealt * pviolcone,
	MSKrealt * pviolacc,
	MSKrealt * pvioldjc,
	MSKrealt * pviolitg,
	MSKrealt * dobj,
	MSKrealt * dviolcon,
	MSKrealt * dviolvar,
	MSKrealt * dviolbarvar,
	MSKrealt * dviolcone,
	MSKrealt * dviolacc);

/* MSK_getsolutionnew */
MSKrescodee (MSKAPI MSK_getsolutionnew) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKprostae * problemsta,
	MSKsolstae * solutionsta,
	MSKstakeye * skc,
	MSKstakeye * skx,
	MSKstakeye * skn,
	MSKrealt * xc,
	MSKrealt * xx,
	MSKrealt * y,
	MSKrealt * slc,
	MSKrealt * suc,
	MSKrealt * slx,
	MSKrealt * sux,
	MSKrealt * snx,
	MSKrealt * doty);

/* MSK_getsolutionslice */
MSKrescodee (MSKAPI MSK_getsolutionslice) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKsoliteme solitem,
	MSKint32t first,
	MSKint32t last,
	MSKrealt * values);

/* MSK_getsparsesymmat */
MSKrescodee (MSKAPI MSK_getsparsesymmat) (
	MSKtask_t task,
	MSKint64t idx,
	MSKint64t maxlen,
	MSKint32t * subi,
	MSKint32t * subj,
	MSKrealt * valij);

/* MSK_getstrparam */
MSKrescodee (MSKAPI MSK_getstrparam) (
	MSKtask_t task,
	MSKsparame param,
	MSKint32t maxlen,
	MSKint32t * len,
	char * parvalue);

/* MSK_getstrparamal */
MSKrescodee (MSKAPI MSK_getstrparamal) (
	MSKtask_t task,
	MSKsparame param,
	MSKint32t numaddchr,
	char ** value);

/* MSK_getstrparamlen */
MSKrescodee (MSKAPI MSK_getstrparamlen) (
	MSKtask_t task,
	MSKsparame param,
	MSKint32t * len);

/* MSK_getsuc */
MSKrescodee (MSKAPI MSK_getsuc) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKrealt * suc);

/* MSK_getsucslice */
MSKrescodee (MSKAPI MSK_getsucslice) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint32t first,
	MSKint32t last,
	MSKrealt * suc);

/* MSK_getsux */
MSKrescodee (MSKAPI MSK_getsux) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKrealt * sux);

/* MSK_getsuxslice */
MSKrescodee (MSKAPI MSK_getsuxslice) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint32t first,
	MSKint32t last,
	MSKrealt * sux);

/* MSK_getsymbcon */
MSKrescodee (MSKAPI MSK_getsymbcon) (
	MSKtask_t task,
	MSKint32t i,
	MSKint32t sizevalue,
	char * name,
	MSKint32t * value);

/* MSK_getsymmatinfo */
MSKrescodee (MSKAPI MSK_getsymmatinfo) (
	MSKtask_t task,
	MSKint64t idx,
	MSKint32t * dim,
	MSKint64t * nz,
	MSKsymmattypee * mattype);

/* MSK_gettaskname */
MSKrescodee (MSKAPI MSK_gettaskname) (
	MSKtask_t task,
	MSKint32t sizetaskname,
	char * taskname);

/* MSK_gettasknamelen */
MSKrescodee (MSKAPI MSK_gettasknamelen) (
	MSKtask_t task,
	MSKint32t * len);

/* MSK_getvarbound */
MSKrescodee (MSKAPI MSK_getvarbound) (
	MSKtask_t task,
	MSKint32t i,
	MSKboundkeye * bk,
	MSKrealt * bl,
	MSKrealt * bu);

/* MSK_getvarboundslice */
MSKrescodee (MSKAPI MSK_getvarboundslice) (
	MSKtask_t task,
	MSKint32t first,
	MSKint32t last,
	MSKboundkeye * bk,
	MSKrealt * bl,
	MSKrealt * bu);

/* MSK_getvarname */
MSKrescodee (MSKAPI MSK_getvarname) (
	MSKtask_t task,
	MSKint32t j,
	MSKint32t sizename,
	char * name);

/* MSK_getvarnameindex */
MSKrescodee (MSKAPI MSK_getvarnameindex) (
	MSKtask_t task,
	const char * somename,
	MSKint32t * asgn,
	MSKint32t * index);

/* MSK_getvarnamelen */
MSKrescodee (MSKAPI MSK_getvarnamelen) (
	MSKtask_t task,
	MSKint32t i,
	MSKint32t * len);

/* MSK_getvartype */
MSKrescodee (MSKAPI MSK_getvartype) (
	MSKtask_t task,
	MSKint32t j,
	MSKvariabletypee * vartype);

/* MSK_getvartypelist */
MSKrescodee (MSKAPI MSK_getvartypelist) (
	MSKtask_t task,
	MSKint32t num,
	const MSKint32t * subj,
	MSKvariabletypee * vartype);

/* MSK_getxc */
MSKrescodee (MSKAPI MSK_getxc) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKrealt * xc);

/* MSK_getxcslice */
MSKrescodee (MSKAPI MSK_getxcslice) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint32t first,
	MSKint32t last,
	MSKrealt * xc);

/* MSK_getxx */
MSKrescodee (MSKAPI MSK_getxx) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKrealt * xx);

/* MSK_getxxslice */
MSKrescodee (MSKAPI MSK_getxxslice) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint32t first,
	MSKint32t last,
	MSKrealt * xx);

/* MSK_gety */
MSKrescodee (MSKAPI MSK_gety) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKrealt * y);

/* MSK_getyslice */
MSKrescodee (MSKAPI MSK_getyslice) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint32t first,
	MSKint32t last,
	MSKrealt * y);

/* MSK_infeasibilityreport */
MSKrescodee (MSKAPI MSK_infeasibilityreport) (
	MSKtask_t task,
	MSKstreamtypee whichstream,
	MSKsoltypee whichsol);

/* MSK_initbasissolve */
MSKrescodee (MSKAPI MSK_initbasissolve) (
	MSKtask_t task,
	MSKint32t * basis);

/* MSK_inputdata */
MSKrescodee (MSKAPI MSK_inputdata) (
	MSKtask_t task,
	MSKint32t maxnumcon,
	MSKint32t maxnumvar,
	MSKint32t numcon,
	MSKint32t numvar,
	const MSKrealt * c,
	MSKrealt cfix,
	const MSKint32t * aptrb,
	const MSKint32t * aptre,
	const MSKint32t * asub,
	const MSKrealt * aval,
	const MSKboundkeye * bkc,
	const MSKrealt * blc,
	const MSKrealt * buc,
	const MSKboundkeye * bkx,
	const MSKrealt * blx,
	const MSKrealt * bux);

/* MSK_inputdata64 */
MSKrescodee (MSKAPI MSK_inputdata64) (
	MSKtask_t task,
	MSKint32t maxnumcon,
	MSKint32t maxnumvar,
	MSKint32t numcon,
	MSKint32t numvar,
	const MSKrealt * c,
	MSKrealt cfix,
	const MSKint64t * aptrb,
	const MSKint64t * aptre,
	const MSKint32t * asub,
	const MSKrealt * aval,
	const MSKboundkeye * bkc,
	const MSKrealt * blc,
	const MSKrealt * buc,
	const MSKboundkeye * bkx,
	const MSKrealt * blx,
	const MSKrealt * bux);

/* MSK_isdouparname */
MSKrescodee (MSKAPI MSK_isdouparname) (
	MSKtask_t task,
	const char * parname,
	MSKdparame * param);

/* MSK_isintparname */
MSKrescodee (MSKAPI MSK_isintparname) (
	MSKtask_t task,
	const char * parname,
	MSKiparame * param);

/* MSK_isstrparname */
MSKrescodee (MSKAPI MSK_isstrparname) (
	MSKtask_t task,
	const char * parname,
	MSKsparame * param);

/* MSK_linkfiletotaskstream */
MSKrescodee (MSKAPI MSK_linkfiletotaskstream) (
	MSKtask_t task,
	MSKstreamtypee whichstream,
	const char * filename,
	MSKint32t append);

/* MSK_linkfunctotaskstream */
MSKrescodee (MSKAPI MSK_linkfunctotaskstream) (
	MSKtask_t task,
	MSKstreamtypee whichstream,
	MSKuserhandle_t handle,
	MSKstreamfunc func);

/* MSK_onesolutionsummary */
MSKrescodee (MSKAPI MSK_onesolutionsummary) (
	MSKtask_t task,
	MSKstreamtypee whichstream,
	MSKsoltypee whichsol);

/* MSK_optimize */
MSKrescodee (MSKAPI MSK_optimize) (
	MSKtask_t task);

/* MSK_optimizermt */
MSKrescodee (MSKAPI MSK_optimizermt) (
	MSKtask_t task,
	const char * address,
	const char * accesstoken,
	MSKrescodee * trmcode);

/* MSK_optimizersummary */
MSKrescodee (MSKAPI MSK_optimizersummary) (
	MSKtask_t task,
	MSKstreamtypee whichstream);

/* MSK_optimizetrm */
MSKrescodee (MSKAPI MSK_optimizetrm) (
	MSKtask_t task,
	MSKrescodee * trmcode);

/* MSK_primalrepair */
MSKrescodee (MSKAPI MSK_primalrepair) (
	MSKtask_t task,
	const MSKrealt * wlc,
	const MSKrealt * wuc,
	const MSKrealt * wlx,
	const MSKrealt * wux);

/* MSK_primalsensitivity */
MSKrescodee (MSKAPI MSK_primalsensitivity) (
	MSKtask_t task,
	MSKint32t numi,
	const MSKint32t * subi,
	const MSKmarke * marki,
	MSKint32t numj,
	const MSKint32t * subj,
	const MSKmarke * markj,
	MSKrealt * leftpricei,
	MSKrealt * rightpricei,
	MSKrealt * leftrangei,
	MSKrealt * rightrangei,
	MSKrealt * leftpricej,
	MSKrealt * rightpricej,
	MSKrealt * leftrangej,
	MSKrealt * rightrangej);

/* MSK_printparam */
MSKrescodee (MSKAPI MSK_printparam) (
	MSKtask_t task);

/* MSK_probtypetostr */
MSKrescodee (MSKAPI MSK_probtypetostr) (
	MSKtask_t task,
	MSKproblemtypee probtype,
	char * str);

/* MSK_prostatostr */
MSKrescodee (MSKAPI MSK_prostatostr) (
	MSKtask_t task,
	MSKprostae problemsta,
	char * str);

/* MSK_putacc */
MSKrescodee (MSKAPI MSK_putacc) (
	MSKtask_t task,
	MSKint64t accidx,
	MSKint64t domidx,
	MSKint64t numafeidx,
	const MSKint64t * afeidxlist,
	const MSKrealt * b);

/* MSK_putaccb */
MSKrescodee (MSKAPI MSK_putaccb) (
	MSKtask_t task,
	MSKint64t accidx,
	MSKint64t lengthb,
	const MSKrealt * b);

/* MSK_putaccbj */
MSKrescodee (MSKAPI MSK_putaccbj) (
	MSKtask_t task,
	MSKint64t accidx,
	MSKint64t j,
	MSKrealt bj);

/* MSK_putaccdoty */
MSKrescodee (MSKAPI MSK_putaccdoty) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint64t accidx,
	MSKrealt * doty);

/* MSK_putacclist */
MSKrescodee (MSKAPI MSK_putacclist) (
	MSKtask_t task,
	MSKint64t numaccs,
	const MSKint64t * accidxs,
	const MSKint64t * domidxs,
	MSKint64t numafeidx,
	const MSKint64t * afeidxlist,
	const MSKrealt * b);

/* MSK_putaccname */
MSKrescodee (MSKAPI MSK_putaccname) (
	MSKtask_t task,
	MSKint64t accidx,
	const char * name);

/* MSK_putacol */
MSKrescodee (MSKAPI MSK_putacol) (
	MSKtask_t task,
	MSKint32t j,
	MSKint32t nzj,
	const MSKint32t * subj,
	const MSKrealt * valj);

/* MSK_putacollist */
MSKrescodee (MSKAPI MSK_putacollist) (
	MSKtask_t task,
	MSKint32t num,
	const MSKint32t * sub,
	const MSKint32t * ptrb,
	const MSKint32t * ptre,
	const MSKint32t * asub,
	const MSKrealt * aval);

/* MSK_putacollist64 */
MSKrescodee (MSKAPI MSK_putacollist64) (
	MSKtask_t task,
	MSKint32t num,
	const MSKint32t * sub,
	const MSKint64t * ptrb,
	const MSKint64t * ptre,
	const MSKint32t * asub,
	const MSKrealt * aval);

/* MSK_putacolslice */
MSKrescodee (MSKAPI MSK_putacolslice) (
	MSKtask_t task,
	MSKint32t first,
	MSKint32t last,
	const MSKint32t * ptrb,
	const MSKint32t * ptre,
	const MSKint32t * asub,
	const MSKrealt * aval);

/* MSK_putacolslice64 */
MSKrescodee (MSKAPI MSK_putacolslice64) (
	MSKtask_t task,
	MSKint32t first,
	MSKint32t last,
	const MSKint64t * ptrb,
	const MSKint64t * ptre,
	const MSKint32t * asub,
	const MSKrealt * aval);

/* MSK_putafebarfblocktriplet */
MSKrescodee (MSKAPI MSK_putafebarfblocktriplet) (
	MSKtask_t task,
	MSKint64t numtrip,
	const MSKint64t * afeidx,
	const MSKint32t * barvaridx,
	const MSKint32t * subk,
	const MSKint32t * subl,
	const MSKrealt * valkl);

/* MSK_putafebarfentry */
MSKrescodee (MSKAPI MSK_putafebarfentry) (
	MSKtask_t task,
	MSKint64t afeidx,
	MSKint32t barvaridx,
	MSKint64t numterm,
	const MSKint64t * termidx,
	const MSKrealt * termweight);

/* MSK_putafebarfentrylist */
MSKrescodee (MSKAPI MSK_putafebarfentrylist) (
	MSKtask_t task,
	MSKint64t numafeidx,
	const MSKint64t * afeidx,
	const MSKint32t * barvaridx,
	const MSKint64t * numterm,
	const MSKint64t * ptrterm,
	MSKint64t lenterm,
	const MSKint64t * termidx,
	const MSKrealt * termweight);

/* MSK_putafebarfrow */
MSKrescodee (MSKAPI MSK_putafebarfrow) (
	MSKtask_t task,
	MSKint64t afeidx,
	MSKint32t numentr,
	const MSKint32t * barvaridx,
	const MSKint64t * numterm,
	const MSKint64t * ptrterm,
	MSKint64t lenterm,
	const MSKint64t * termidx,
	const MSKrealt * termweight);

/* MSK_putafefcol */
MSKrescodee (MSKAPI MSK_putafefcol) (
	MSKtask_t task,
	MSKint32t varidx,
	MSKint64t numnz,
	const MSKint64t * afeidx,
	const MSKrealt * val);

/* MSK_putafefentry */
MSKrescodee (MSKAPI MSK_putafefentry) (
	MSKtask_t task,
	MSKint64t afeidx,
	MSKint32t varidx,
	MSKrealt value);

/* MSK_putafefentrylist */
MSKrescodee (MSKAPI MSK_putafefentrylist) (
	MSKtask_t task,
	MSKint64t numentr,
	const MSKint64t * afeidx,
	const MSKint32t * varidx,
	const MSKrealt * val);

/* MSK_putafefrow */
MSKrescodee (MSKAPI MSK_putafefrow) (
	MSKtask_t task,
	MSKint64t afeidx,
	MSKint32t numnz,
	const MSKint32t * varidx,
	const MSKrealt * val);

/* MSK_putafefrowlist */
MSKrescodee (MSKAPI MSK_putafefrowlist) (
	MSKtask_t task,
	MSKint64t numafeidx,
	const MSKint64t * afeidx,
	const MSKint32t * numnzrow,
	const MSKint64t * ptrrow,
	MSKint64t lenidxval,
	const MSKint32t * varidx,
	const MSKrealt * val);

/* MSK_putafeg */
MSKrescodee (MSKAPI MSK_putafeg) (
	MSKtask_t task,
	MSKint64t afeidx,
	MSKrealt g);

/* MSK_putafeglist */
MSKrescodee (MSKAPI MSK_putafeglist) (
	MSKtask_t task,
	MSKint64t numafeidx,
	const MSKint64t * afeidx,
	const MSKrealt * g);

/* MSK_putafegslice */
MSKrescodee (MSKAPI MSK_putafegslice) (
	MSKtask_t task,
	MSKint64t first,
	MSKint64t last,
	const MSKrealt * slice);

/* MSK_putaij */
MSKrescodee (MSKAPI MSK_putaij) (
	MSKtask_t task,
	MSKint32t i,
	MSKint32t j,
	MSKrealt aij);

/* MSK_putaijlist */
MSKrescodee (MSKAPI MSK_putaijlist) (
	MSKtask_t task,
	MSKint32t num,
	const MSKint32t * subi,
	const MSKint32t * subj,
	const MSKrealt * valij);

/* MSK_putaijlist64 */
MSKrescodee (MSKAPI MSK_putaijlist64) (
	MSKtask_t task,
	MSKint64t num,
	const MSKint32t * subi,
	const MSKint32t * subj,
	const MSKrealt * valij);

/* MSK_putarow */
MSKrescodee (MSKAPI MSK_putarow) (
	MSKtask_t task,
	MSKint32t i,
	MSKint32t nzi,
	const MSKint32t * subi,
	const MSKrealt * vali);

/* MSK_putarowlist */
MSKrescodee (MSKAPI MSK_putarowlist) (
	MSKtask_t task,
	MSKint32t num,
	const MSKint32t * sub,
	const MSKint32t * ptrb,
	const MSKint32t * ptre,
	const MSKint32t * asub,
	const MSKrealt * aval);

/* MSK_putarowlist64 */
MSKrescodee (MSKAPI MSK_putarowlist64) (
	MSKtask_t task,
	MSKint32t num,
	const MSKint32t * sub,
	const MSKint64t * ptrb,
	const MSKint64t * ptre,
	const MSKint32t * asub,
	const MSKrealt * aval);

/* MSK_putarowslice */
MSKrescodee (MSKAPI MSK_putarowslice) (
	MSKtask_t task,
	MSKint32t first,
	MSKint32t last,
	const MSKint32t * ptrb,
	const MSKint32t * ptre,
	const MSKint32t * asub,
	const MSKrealt * aval);

/* MSK_putarowslice64 */
MSKrescodee (MSKAPI MSK_putarowslice64) (
	MSKtask_t task,
	MSKint32t first,
	MSKint32t last,
	const MSKint64t * ptrb,
	const MSKint64t * ptre,
	const MSKint32t * asub,
	const MSKrealt * aval);

/* MSK_putatruncatetol */
MSKrescodee (MSKAPI MSK_putatruncatetol) (
	MSKtask_t task,
	MSKrealt tolzero);

/* MSK_putbarablocktriplet */
MSKrescodee (MSKAPI MSK_putbarablocktriplet) (
	MSKtask_t task,
	MSKint64t num,
	const MSKint32t * subi,
	const MSKint32t * subj,
	const MSKint32t * subk,
	const MSKint32t * subl,
	const MSKrealt * valijkl);

/* MSK_putbaraij */
MSKrescodee (MSKAPI MSK_putbaraij) (
	MSKtask_t task,
	MSKint32t i,
	MSKint32t j,
	MSKint64t num,
	const MSKint64t * sub,
	const MSKrealt * weights);

/* MSK_putbaraijlist */
MSKrescodee (MSKAPI MSK_putbaraijlist) (
	MSKtask_t task,
	MSKint32t num,
	const MSKint32t * subi,
	const MSKint32t * subj,
	const MSKint64t * alphaptrb,
	const MSKint64t * alphaptre,
	const MSKint64t * matidx,
	const MSKrealt * weights);

/* MSK_putbararowlist */
MSKrescodee (MSKAPI MSK_putbararowlist) (
	MSKtask_t task,
	MSKint32t num,
	const MSKint32t * subi,
	const MSKint64t * ptrb,
	const MSKint64t * ptre,
	const MSKint32t * subj,
	const MSKint64t * nummat,
	const MSKint64t * matidx,
	const MSKrealt * weights);

/* MSK_putbarcblocktriplet */
MSKrescodee (MSKAPI MSK_putbarcblocktriplet) (
	MSKtask_t task,
	MSKint64t num,
	const MSKint32t * subj,
	const MSKint32t * subk,
	const MSKint32t * subl,
	const MSKrealt * valjkl);

/* MSK_putbarcj */
MSKrescodee (MSKAPI MSK_putbarcj) (
	MSKtask_t task,
	MSKint32t j,
	MSKint64t num,
	const MSKint64t * sub,
	const MSKrealt * weights);

/* MSK_putbarsj */
MSKrescodee (MSKAPI MSK_putbarsj) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint32t j,
	const MSKrealt * barsj);

/* MSK_putbarvarname */
MSKrescodee (MSKAPI MSK_putbarvarname) (
	MSKtask_t task,
	MSKint32t j,
	const char * name);

/* MSK_putbarxj */
MSKrescodee (MSKAPI MSK_putbarxj) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint32t j,
	const MSKrealt * barxj);

/* MSK_putcallbackfunc */
MSKrescodee (MSKAPI MSK_putcallbackfunc) (
	MSKtask_t task,
	MSKcallbackfunc func,
	MSKuserhandle_t handle);

/* MSK_putcfix */
MSKrescodee (MSKAPI MSK_putcfix) (
	MSKtask_t task,
	MSKrealt cfix);

/* MSK_putcj */
MSKrescodee (MSKAPI MSK_putcj) (
	MSKtask_t task,
	MSKint32t j,
	MSKrealt cj);

/* MSK_putclist */
MSKrescodee (MSKAPI MSK_putclist) (
	MSKtask_t task,
	MSKint32t num,
	const MSKint32t * subj,
	const MSKrealt * val);

/* MSK_putconbound */
MSKrescodee (MSKAPI MSK_putconbound) (
	MSKtask_t task,
	MSKint32t i,
	MSKboundkeye bkc,
	MSKrealt blc,
	MSKrealt buc);

/* MSK_putconboundlist */
MSKrescodee (MSKAPI MSK_putconboundlist) (
	MSKtask_t task,
	MSKint32t num,
	const MSKint32t * sub,
	const MSKboundkeye * bkc,
	const MSKrealt * blc,
	const MSKrealt * buc);

/* MSK_putconboundlistconst */
MSKrescodee (MSKAPI MSK_putconboundlistconst) (
	MSKtask_t task,
	MSKint32t num,
	const MSKint32t * sub,
	MSKboundkeye bkc,
	MSKrealt blc,
	MSKrealt buc);

/* MSK_putconboundslice */
MSKrescodee (MSKAPI MSK_putconboundslice) (
	MSKtask_t task,
	MSKint32t first,
	MSKint32t last,
	const MSKboundkeye * bkc,
	const MSKrealt * blc,
	const MSKrealt * buc);

/* MSK_putconboundsliceconst */
MSKrescodee (MSKAPI MSK_putconboundsliceconst) (
	MSKtask_t task,
	MSKint32t first,
	MSKint32t last,
	MSKboundkeye bkc,
	MSKrealt blc,
	MSKrealt buc);

/* MSK_putcone */
MSKrescodee (MSKAPI MSK_putcone) (
	MSKtask_t task,
	MSKint32t k,
	MSKconetypee ct,
	MSKrealt conepar,
	MSKint32t nummem,
	const MSKint32t * submem);

/* MSK_putconename */
MSKrescodee (MSKAPI MSK_putconename) (
	MSKtask_t task,
	MSKint32t j,
	const char * name);

/* MSK_putconname */
MSKrescodee (MSKAPI MSK_putconname) (
	MSKtask_t task,
	MSKint32t i,
	const char * name);

/* MSK_putconsolutioni */
MSKrescodee (MSKAPI MSK_putconsolutioni) (
	MSKtask_t task,
	MSKint32t i,
	MSKsoltypee whichsol,
	MSKstakeye sk,
	MSKrealt x,
	MSKrealt sl,
	MSKrealt su);

/* MSK_putcslice */
MSKrescodee (MSKAPI MSK_putcslice) (
	MSKtask_t task,
	MSKint32t first,
	MSKint32t last,
	const MSKrealt * slice);

/* MSK_putdjc */
MSKrescodee (MSKAPI MSK_putdjc) (
	MSKtask_t task,
	MSKint64t djcidx,
	MSKint64t numdomidx,
	const MSKint64t * domidxlist,
	MSKint64t numafeidx,
	const MSKint64t * afeidxlist,
	const MSKrealt * b,
	MSKint64t numterms,
	const MSKint64t * termsizelist);

/* MSK_putdjcname */
MSKrescodee (MSKAPI MSK_putdjcname) (
	MSKtask_t task,
	MSKint64t djcidx,
	const char * name);

/* MSK_putdjcslice */
MSKrescodee (MSKAPI MSK_putdjcslice) (
	MSKtask_t task,
	MSKint64t idxfirst,
	MSKint64t idxlast,
	MSKint64t numdomidx,
	const MSKint64t * domidxlist,
	MSKint64t numafeidx,
	const MSKint64t * afeidxlist,
	const MSKrealt * b,
	MSKint64t numterms,
	const MSKint64t * termsizelist,
	const MSKint64t * termsindjc);

/* MSK_putdomainname */
MSKrescodee (MSKAPI MSK_putdomainname) (
	MSKtask_t task,
	MSKint64t domidx,
	const char * name);

/* MSK_putdouparam */
MSKrescodee (MSKAPI MSK_putdouparam) (
	MSKtask_t task,
	MSKdparame param,
	MSKrealt parvalue);

/* MSK_putintparam */
MSKrescodee (MSKAPI MSK_putintparam) (
	MSKtask_t task,
	MSKiparame param,
	MSKint32t parvalue);

/* MSK_putlintparam */
MSKrescodee (MSKAPI MSK_putlintparam) (
	MSKtask_t task,
	MSKiparame param,
	MSKint64t parvalue);

/* MSK_putmaxnumacc */
MSKrescodee (MSKAPI MSK_putmaxnumacc) (
	MSKtask_t task,
	MSKint64t maxnumacc);

/* MSK_putmaxnumafe */
MSKrescodee (MSKAPI MSK_putmaxnumafe) (
	MSKtask_t task,
	MSKint64t maxnumafe);

/* MSK_putmaxnumanz */
MSKrescodee (MSKAPI MSK_putmaxnumanz) (
	MSKtask_t task,
	MSKint64t maxnumanz);

/* MSK_putmaxnumbarvar */
MSKrescodee (MSKAPI MSK_putmaxnumbarvar) (
	MSKtask_t task,
	MSKint32t maxnumbarvar);

/* MSK_putmaxnumcon */
MSKrescodee (MSKAPI MSK_putmaxnumcon) (
	MSKtask_t task,
	MSKint32t maxnumcon);

/* MSK_putmaxnumcone */
MSKrescodee (MSKAPI MSK_putmaxnumcone) (
	MSKtask_t task,
	MSKint32t maxnumcone);

/* MSK_putmaxnumdjc */
MSKrescodee (MSKAPI MSK_putmaxnumdjc) (
	MSKtask_t task,
	MSKint64t maxnumdjc);

/* MSK_putmaxnumdomain */
MSKrescodee (MSKAPI MSK_putmaxnumdomain) (
	MSKtask_t task,
	MSKint64t maxnumdomain);

/* MSK_putmaxnumqnz */
MSKrescodee (MSKAPI MSK_putmaxnumqnz) (
	MSKtask_t task,
	MSKint64t maxnumqnz);

/* MSK_putmaxnumvar */
MSKrescodee (MSKAPI MSK_putmaxnumvar) (
	MSKtask_t task,
	MSKint32t maxnumvar);

/* MSK_putnadouparam */
MSKrescodee (MSKAPI MSK_putnadouparam) (
	MSKtask_t task,
	const char * paramname,
	MSKrealt parvalue);

/* MSK_putnaintparam */
MSKrescodee (MSKAPI MSK_putnaintparam) (
	MSKtask_t task,
	const char * paramname,
	MSKint32t parvalue);

/* MSK_putnastrparam */
MSKrescodee (MSKAPI MSK_putnastrparam) (
	MSKtask_t task,
	const char * paramname,
	const char * parvalue);

/* MSK_putobjname */
MSKrescodee (MSKAPI MSK_putobjname) (
	MSKtask_t task,
	const char * objname);

/* MSK_putobjsense */
MSKrescodee (MSKAPI MSK_putobjsense) (
	MSKtask_t task,
	MSKobjsensee sense);

/* MSK_putoptserverhost */
MSKrescodee (MSKAPI MSK_putoptserverhost) (
	MSKtask_t task,
	const char * host);

/* MSK_putparam */
MSKrescodee (MSKAPI MSK_putparam) (
	MSKtask_t task,
	const char * parname,
	const char * parvalue);

/* MSK_putqcon */
MSKrescodee (MSKAPI MSK_putqcon) (
	MSKtask_t task,
	MSKint32t numqcnz,
	const MSKint32t * qcsubk,
	const MSKint32t * qcsubi,
	const MSKint32t * qcsubj,
	const MSKrealt * qcval);

/* MSK_putqconk */
MSKrescodee (MSKAPI MSK_putqconk) (
	MSKtask_t task,
	MSKint32t k,
	MSKint32t numqcnz,
	const MSKint32t * qcsubi,
	const MSKint32t * qcsubj,
	const MSKrealt * qcval);

/* MSK_putqobj */
MSKrescodee (MSKAPI MSK_putqobj) (
	MSKtask_t task,
	MSKint32t numqonz,
	const MSKint32t * qosubi,
	const MSKint32t * qosubj,
	const MSKrealt * qoval);

/* MSK_putqobjij */
MSKrescodee (MSKAPI MSK_putqobjij) (
	MSKtask_t task,
	MSKint32t i,
	MSKint32t j,
	MSKrealt qoij);

/* MSK_putresponsefunc */
MSKrescodee (MSKAPI MSK_putresponsefunc) (
	MSKtask_t task,
	MSKresponsefunc responsefunc,
	MSKuserhandle_t handle);

/* MSK_putskc */
MSKrescodee (MSKAPI MSK_putskc) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	const MSKstakeye * skc);

/* MSK_putskcslice */
MSKrescodee (MSKAPI MSK_putskcslice) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint32t first,
	MSKint32t last,
	const MSKstakeye * skc);

/* MSK_putskx */
MSKrescodee (MSKAPI MSK_putskx) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	const MSKstakeye * skx);

/* MSK_putskxslice */
MSKrescodee (MSKAPI MSK_putskxslice) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint32t first,
	MSKint32t last,
	const MSKstakeye * skx);

/* MSK_putslc */
MSKrescodee (MSKAPI MSK_putslc) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	const MSKrealt * slc);

/* MSK_putslcslice */
MSKrescodee (MSKAPI MSK_putslcslice) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint32t first,
	MSKint32t last,
	const MSKrealt * slc);

/* MSK_putslx */
MSKrescodee (MSKAPI MSK_putslx) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	const MSKrealt * slx);

/* MSK_putslxslice */
MSKrescodee (MSKAPI MSK_putslxslice) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint32t first,
	MSKint32t last,
	const MSKrealt * slx);

/* MSK_putsnx */
MSKrescodee (MSKAPI MSK_putsnx) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	const MSKrealt * sux);

/* MSK_putsnxslice */
MSKrescodee (MSKAPI MSK_putsnxslice) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint32t first,
	MSKint32t last,
	const MSKrealt * snx);

/* MSK_putsolution */
MSKrescodee (MSKAPI MSK_putsolution) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	const MSKstakeye * skc,
	const MSKstakeye * skx,
	const MSKstakeye * skn,
	const MSKrealt * xc,
	const MSKrealt * xx,
	const MSKrealt * y,
	const MSKrealt * slc,
	const MSKrealt * suc,
	const MSKrealt * slx,
	const MSKrealt * sux,
	const MSKrealt * snx);

/* MSK_putsolutionnew */
MSKrescodee (MSKAPI MSK_putsolutionnew) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	const MSKstakeye * skc,
	const MSKstakeye * skx,
	const MSKstakeye * skn,
	const MSKrealt * xc,
	const MSKrealt * xx,
	const MSKrealt * y,
	const MSKrealt * slc,
	const MSKrealt * suc,
	const MSKrealt * slx,
	const MSKrealt * sux,
	const MSKrealt * snx,
	const MSKrealt * doty);

/* MSK_putsolutionyi */
MSKrescodee (MSKAPI MSK_putsolutionyi) (
	MSKtask_t task,
	MSKint32t i,
	MSKsoltypee whichsol,
	MSKrealt y);

/* MSK_putstrparam */
MSKrescodee (MSKAPI MSK_putstrparam) (
	MSKtask_t task,
	MSKsparame param,
	const char * parvalue);

/* MSK_putsuc */
MSKrescodee (MSKAPI MSK_putsuc) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	const MSKrealt * suc);

/* MSK_putsucslice */
MSKrescodee (MSKAPI MSK_putsucslice) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint32t first,
	MSKint32t last,
	const MSKrealt * suc);

/* MSK_putsux */
MSKrescodee (MSKAPI MSK_putsux) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	const MSKrealt * sux);

/* MSK_putsuxslice */
MSKrescodee (MSKAPI MSK_putsuxslice) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint32t first,
	MSKint32t last,
	const MSKrealt * sux);

/* MSK_puttaskname */
MSKrescodee (MSKAPI MSK_puttaskname) (
	MSKtask_t task,
	const char * taskname);

/* MSK_putvarbound */
MSKrescodee (MSKAPI MSK_putvarbound) (
	MSKtask_t task,
	MSKint32t j,
	MSKboundkeye bkx,
	MSKrealt blx,
	MSKrealt bux);

/* MSK_putvarboundlist */
MSKrescodee (MSKAPI MSK_putvarboundlist) (
	MSKtask_t task,
	MSKint32t num,
	const MSKint32t * sub,
	const MSKboundkeye * bkx,
	const MSKrealt * blx,
	const MSKrealt * bux);

/* MSK_putvarboundlistconst */
MSKrescodee (MSKAPI MSK_putvarboundlistconst) (
	MSKtask_t task,
	MSKint32t num,
	const MSKint32t * sub,
	MSKboundkeye bkx,
	MSKrealt blx,
	MSKrealt bux);

/* MSK_putvarboundslice */
MSKrescodee (MSKAPI MSK_putvarboundslice) (
	MSKtask_t task,
	MSKint32t first,
	MSKint32t last,
	const MSKboundkeye * bkx,
	const MSKrealt * blx,
	const MSKrealt * bux);

/* MSK_putvarboundsliceconst */
MSKrescodee (MSKAPI MSK_putvarboundsliceconst) (
	MSKtask_t task,
	MSKint32t first,
	MSKint32t last,
	MSKboundkeye bkx,
	MSKrealt blx,
	MSKrealt bux);

/* MSK_putvarname */
MSKrescodee (MSKAPI MSK_putvarname) (
	MSKtask_t task,
	MSKint32t j,
	const char * name);

/* MSK_putvarsolutionj */
MSKrescodee (MSKAPI MSK_putvarsolutionj) (
	MSKtask_t task,
	MSKint32t j,
	MSKsoltypee whichsol,
	MSKstakeye sk,
	MSKrealt x,
	MSKrealt sl,
	MSKrealt su,
	MSKrealt sn);

/* MSK_putvartype */
MSKrescodee (MSKAPI MSK_putvartype) (
	MSKtask_t task,
	MSKint32t j,
	MSKvariabletypee vartype);

/* MSK_putvartypelist */
MSKrescodee (MSKAPI MSK_putvartypelist) (
	MSKtask_t task,
	MSKint32t num,
	const MSKint32t * subj,
	const MSKvariabletypee * vartype);

/* MSK_putxc */
MSKrescodee (MSKAPI MSK_putxc) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKrealt * xc);

/* MSK_putxcslice */
MSKrescodee (MSKAPI MSK_putxcslice) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint32t first,
	MSKint32t last,
	const MSKrealt * xc);

/* MSK_putxx */
MSKrescodee (MSKAPI MSK_putxx) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	const MSKrealt * xx);

/* MSK_putxxslice */
MSKrescodee (MSKAPI MSK_putxxslice) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint32t first,
	MSKint32t last,
	const MSKrealt * xx);

/* MSK_puty */
MSKrescodee (MSKAPI MSK_puty) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	const MSKrealt * y);

/* MSK_putyslice */
MSKrescodee (MSKAPI MSK_putyslice) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKint32t first,
	MSKint32t last,
	const MSKrealt * y);

/* MSK_readbsolution */
MSKrescodee (MSKAPI MSK_readbsolution) (
	MSKtask_t task,
	const char * filename,
	MSKcompresstypee compress);

/* MSK_readdata */
MSKrescodee (MSKAPI MSK_readdata) (
	MSKtask_t task,
	const char * filename);

/* MSK_readdataautoformat */
MSKrescodee (MSKAPI MSK_readdataautoformat) (
	MSKtask_t task,
	const char * filename);

/* MSK_readdataformat */
MSKrescodee (MSKAPI MSK_readdataformat) (
	MSKtask_t task,
	const char * filename,
	MSKdataformate format,
	MSKcompresstypee compress);

/* MSK_readdatahandle */
MSKrescodee (MSKAPI MSK_readdatahandle) (
	MSKtask_t task,
	MSKhreadfunc hread,
	MSKuserhandle_t h,
	MSKdataformate format,
	MSKcompresstypee compress,
	const char * path);

/* MSK_readjsonsol */
MSKrescodee (MSKAPI MSK_readjsonsol) (
	MSKtask_t task,
	const char * filename);

/* MSK_readjsonstring */
MSKrescodee (MSKAPI MSK_readjsonstring) (
	MSKtask_t task,
	const char * data);

/* MSK_readlpstring */
MSKrescodee (MSKAPI MSK_readlpstring) (
	MSKtask_t task,
	const char * data);

/* MSK_readopfstring */
MSKrescodee (MSKAPI MSK_readopfstring) (
	MSKtask_t task,
	const char * data);

/* MSK_readparamfile */
MSKrescodee (MSKAPI MSK_readparamfile) (
	MSKtask_t task,
	const char * filename);

/* MSK_readptfstring */
MSKrescodee (MSKAPI MSK_readptfstring) (
	MSKtask_t task,
	const char * data);

/* MSK_readsolution */
MSKrescodee (MSKAPI MSK_readsolution) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	const char * filename);

/* MSK_readsolutionfile */
MSKrescodee (MSKAPI MSK_readsolutionfile) (
	MSKtask_t task,
	const char * filename);

/* MSK_readsummary */
MSKrescodee (MSKAPI MSK_readsummary) (
	MSKtask_t task,
	MSKstreamtypee whichstream);

/* MSK_readtask */
MSKrescodee (MSKAPI MSK_readtask) (
	MSKtask_t task,
	const char * filename);

/* MSK_removebarvars */
MSKrescodee (MSKAPI MSK_removebarvars) (
	MSKtask_t task,
	MSKint32t num,
	const MSKint32t * subset);

/* MSK_removecones */
MSKrescodee (MSKAPI MSK_removecones) (
	MSKtask_t task,
	MSKint32t num,
	const MSKint32t * subset);

/* MSK_removecons */
MSKrescodee (MSKAPI MSK_removecons) (
	MSKtask_t task,
	MSKint32t num,
	const MSKint32t * subset);

/* MSK_removevars */
MSKrescodee (MSKAPI MSK_removevars) (
	MSKtask_t task,
	MSKint32t num,
	const MSKint32t * subset);

/* MSK_resetdouparam */
MSKrescodee (MSKAPI MSK_resetdouparam) (
	MSKtask_t task,
	MSKdparame param);

/* MSK_resetintparam */
MSKrescodee (MSKAPI MSK_resetintparam) (
	MSKtask_t task,
	MSKiparame param);

/* MSK_resetparameters */
MSKrescodee (MSKAPI MSK_resetparameters) (
	MSKtask_t task);

/* MSK_resetstrparam */
MSKrescodee (MSKAPI MSK_resetstrparam) (
	MSKtask_t task,
	MSKsparame param);

/* MSK_resizetask */
MSKrescodee (MSKAPI MSK_resizetask) (
	MSKtask_t task,
	MSKint32t maxnumcon,
	MSKint32t maxnumvar,
	MSKint32t maxnumcone,
	MSKint64t maxnumanz,
	MSKint64t maxnumqnz);

/* MSK_sensitivityreport */
MSKrescodee (MSKAPI MSK_sensitivityreport) (
	MSKtask_t task,
	MSKstreamtypee whichstream);

/* MSK_sktostr */
MSKrescodee (MSKAPI MSK_sktostr) (
	MSKtask_t task,
	MSKstakeye sk,
	char * str);

/* MSK_solstatostr */
MSKrescodee (MSKAPI MSK_solstatostr) (
	MSKtask_t task,
	MSKsolstae solutionsta,
	char * str);

/* MSK_solutiondef */
MSKrescodee (MSKAPI MSK_solutiondef) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	MSKbooleant * isdef);

/* MSK_solutionsummary */
MSKrescodee (MSKAPI MSK_solutionsummary) (
	MSKtask_t task,
	MSKstreamtypee whichstream);

/* MSK_solvewithbasis */
MSKrescodee (MSKAPI MSK_solvewithbasis) (
	MSKtask_t task,
	MSKbooleant transp,
	MSKint32t numnz,
	MSKint32t * sub,
	MSKrealt * val,
	MSKint32t * numnzout);

/* MSK_strtoconetype */
MSKrescodee (MSKAPI MSK_strtoconetype) (
	MSKtask_t task,
	const char * str,
	MSKconetypee * conetype);

/* MSK_strtosk */
MSKrescodee (MSKAPI MSK_strtosk) (
	MSKtask_t task,
	const char * str,
	MSKstakeye * sk);

/* MSK_toconic */
MSKrescodee (MSKAPI MSK_toconic) (
	MSKtask_t task);

/* MSK_unlinkfuncfromtaskstream */
MSKrescodee (MSKAPI MSK_unlinkfuncfromtaskstream) (
	MSKtask_t task,
	MSKstreamtypee whichstream);

/* MSK_updatesolutioninfo */
MSKrescodee (MSKAPI MSK_updatesolutioninfo) (
	MSKtask_t task,
	MSKsoltypee whichsol);

/* MSK_whichparam */
MSKrescodee (MSKAPI MSK_whichparam) (
	MSKtask_t task,
	const char * parname,
	MSKparametertypee * partype,
	MSKint32t * param);

/* MSK_writebsolution */
MSKrescodee (MSKAPI MSK_writebsolution) (
	MSKtask_t task,
	const char * filename,
	MSKcompresstypee compress);

/* MSK_writebsolutionhandle */
MSKrescodee (MSKAPI MSK_writebsolutionhandle) (
	MSKtask_t task,
	MSKhwritefunc func,
	MSKuserhandle_t handle,
	MSKcompresstypee compress);

/* MSK_writedata */
MSKrescodee (MSKAPI MSK_writedata) (
	MSKtask_t task,
	const char * filename);

/* MSK_writedatahandle */
MSKrescodee (MSKAPI MSK_writedatahandle) (
	MSKtask_t task,
	MSKhwritefunc func,
	MSKuserhandle_t handle,
	MSKdataformate format,
	MSKcompresstypee compress);

/* MSK_writejsonsol */
MSKrescodee (MSKAPI MSK_writejsonsol) (
	MSKtask_t task,
	const char * filename);

/* MSK_writeparamfile */
MSKrescodee (MSKAPI MSK_writeparamfile) (
	MSKtask_t task,
	const char * filename);

/* MSK_writesolution */
MSKrescodee (MSKAPI MSK_writesolution) (
	MSKtask_t task,
	MSKsoltypee whichsol,
	const char * filename);

/* MSK_writesolutionfile */
MSKrescodee (MSKAPI MSK_writesolutionfile) (
	MSKtask_t task,
	const char * filename);

/* MSK_writetask */
MSKrescodee (MSKAPI MSK_writetask) (
	MSKtask_t task,
	const char * filename);

/* MSK_axpy */
MSKrescodee (MSKAPI MSK_axpy) (
	MSKenv_t env,
	MSKint32t n,
	MSKrealt alpha,
	const MSKrealt * x,
	MSKrealt * y);

/* MSK_callbackcodetostr */
MSKrescodee (MSKAPI MSK_callbackcodetostr) (
	MSKcallbackcodee code,
	char * callbackcodestr);

/* MSK_callocdbgenv */
void * (MSKAPI MSK_callocdbgenv) (
	MSKenv_t env,
	const size_t number,
	const size_t size,
	const char * file,
	const unsigned line);

/* MSK_callocenv */
void * (MSKAPI MSK_callocenv) (
	MSKenv_t env,
	const size_t number,
	const size_t size);

/* MSK_checkinall */
MSKrescodee (MSKAPI MSK_checkinall) (
	MSKenv_t env);

/* MSK_checkinlicense */
MSKrescodee (MSKAPI MSK_checkinlicense) (
	MSKenv_t env,
	MSKfeaturee feature);

/* MSK_checkmemenv */
MSKrescodee (MSKAPI MSK_checkmemenv) (
	MSKenv_t env,
	const char * file,
	MSKint32t line);

/* MSK_checkoutlicense */
MSKrescodee (MSKAPI MSK_checkoutlicense) (
	MSKenv_t env,
	MSKfeaturee feature);

/* MSK_checkversion */
MSKrescodee (MSKAPI MSK_checkversion) (
	MSKenv_t env,
	MSKint32t major,
	MSKint32t minor,
	MSKint32t revision);

/* MSK_computesparsecholesky */
MSKrescodee (MSKAPI MSK_computesparsecholesky) (
	MSKenv_t env,
	MSKint32t numthreads,
	MSKint32t ordermethod,
	MSKrealt tolsingular,
	MSKint32t n,
	const MSKint32t * anzc,
	const MSKint64t * aptrc,
	const MSKint32t * asubc,
	const MSKrealt * avalc,
	MSKint32t ** perm,
	MSKrealt ** diag,
	MSKint32t ** lnzc,
	MSKint64t ** lptrc,
	MSKint64t * lensubnval,
	MSKint32t ** lsubc,
	MSKrealt ** lvalc);

/* MSK_deleteenv */
MSKrescodee (MSKAPI MSK_deleteenv) (
	MSKenv_t * env);

/* MSK_dinfitemtostr */
MSKrescodee (MSKAPI MSK_dinfitemtostr) (
	MSKdinfiteme item,
	char * str);

/* MSK_dot */
MSKrescodee (MSKAPI MSK_dot) (
	MSKenv_t env,
	MSKint32t n,
	const MSKrealt * x,
	const MSKrealt * y,
	MSKrealt * xty);

/* MSK_echoenv */
MSKrescodee (MSKAPIVA MSK_echoenv) (
	MSKenv_t env,
	MSKstreamtypee whichstream,
	const char * format,
	...);

/* MSK_echointro */
MSKrescodee (MSKAPI MSK_echointro) (
	MSKenv_t env,
	MSKint32t longver);

/* MSK_expirylicenses */
MSKrescodee (MSKAPI MSK_expirylicenses) (
	MSKenv_t env,
	MSKint64t * expiry);

/* MSK_freedbgenv */
void (MSKAPI MSK_freedbgenv) (
	MSKenv_t env,
	void * buffer,
	const char * file,
	const unsigned line);

/* MSK_freeenv */
void (MSKAPI MSK_freeenv) (
	MSKenv_t env,
	void * buffer);

/* MSK_gemm */
MSKrescodee (MSKAPI MSK_gemm) (
	MSKenv_t env,
	MSKtransposee transa,
	MSKtransposee transb,
	MSKint32t m,
	MSKint32t n,
	MSKint32t k,
	MSKrealt alpha,
	const MSKrealt * a,
	const MSKrealt * b,
	MSKrealt beta,
	MSKrealt * c);

/* MSK_gemv */
MSKrescodee (MSKAPI MSK_gemv) (
	MSKenv_t env,
	MSKtransposee transa,
	MSKint32t m,
	MSKint32t n,
	MSKrealt alpha,
	const MSKrealt * a,
	const MSKrealt * x,
	MSKrealt beta,
	MSKrealt * y);

/* MSK_getbuildinfo */
MSKrescodee (MSKAPI MSK_getbuildinfo) (
	char * buildstate,
	char * builddate);

/* MSK_getcodedesc */
MSKrescodee (MSKAPI MSK_getcodedesc) (
	MSKrescodee code,
	char * symname,
	char * str);

/* MSK_getresponseclass */
MSKrescodee (MSKAPI MSK_getresponseclass) (
	MSKrescodee r,
	MSKrescodetypee * rc);

/* MSK_getsymbcondim */
MSKrescodee (MSKAPI MSK_getsymbcondim) (
	MSKenv_t env,
	MSKint32t * num,
	size_t * maxlen);

/* MSK_getversion */
MSKrescodee (MSKAPI MSK_getversion) (
	MSKint32t * major,
	MSKint32t * minor,
	MSKint32t * revision);

/* MSK_globalenvfinalize */
MSKrescodee (MSKAPI MSK_globalenvfinalize) (
void);

/* MSK_globalenvinitialize */
MSKrescodee (MSKAPI MSK_globalenvinitialize) (
	MSKint64t maxnumalloc,
	const char * dbgfile);

/* MSK_iinfitemtostr */
MSKrescodee (MSKAPI MSK_iinfitemtostr) (
	MSKiinfiteme item,
	char * str);

/* MSK_iparvaltosymnam */
MSKrescodee (MSKAPI MSK_iparvaltosymnam) (
	MSKenv_t env,
	MSKiparame whichparam,
	MSKint32t whichvalue,
	char * symbolicname);

/* MSK_isinfinity */
MSKbooleant (MSKAPI MSK_isinfinity) (
	MSKrealt value);

/* MSK_licensecleanup */
MSKrescodee (MSKAPI MSK_licensecleanup) (
void);

/* MSK_liinfitemtostr */
MSKrescodee (MSKAPI MSK_liinfitemtostr) (
	MSKliinfiteme item,
	char * str);

/* MSK_linkfiletoenvstream */
MSKrescodee (MSKAPI MSK_linkfiletoenvstream) (
	MSKenv_t env,
	MSKstreamtypee whichstream,
	const char * filename,
	MSKint32t append);

/* MSK_linkfunctoenvstream */
MSKrescodee (MSKAPI MSK_linkfunctoenvstream) (
	MSKenv_t env,
	MSKstreamtypee whichstream,
	MSKuserhandle_t handle,
	MSKstreamfunc func);

/* MSK_makeemptytask */
MSKrescodee (MSKAPI MSK_makeemptytask) (
	MSKenv_t env,
	MSKtask_t * task);

/* MSK_makeenv */
MSKrescodee (MSKAPI MSK_makeenv) (
	MSKenv_t * env,
	const char * dbgfile);

/* MSK_maketask */
MSKrescodee (MSKAPI MSK_maketask) (
	MSKenv_t env,
	MSKint32t maxnumcon,
	MSKint32t maxnumvar,
	MSKtask_t * task);

/* MSK_optimizebatch */
MSKrescodee (MSKAPI MSK_optimizebatch) (
	MSKenv_t env,
	MSKbooleant israce,
	MSKrealt maxtime,
	MSKint32t numthreads,
	MSKint64t numtask,
	const MSKtask_t * task,
	MSKrescodee * trmcode,
	MSKrescodee * rcode);

/* MSK_potrf */
MSKrescodee (MSKAPI MSK_potrf) (
	MSKenv_t env,
	MSKuploe uplo,
	MSKint32t n,
	MSKrealt * a);

/* MSK_putexitfunc */
MSKrescodee (MSKAPI MSK_putexitfunc) (
	MSKenv_t env,
	MSKexitfunc exitfunc,
	MSKuserhandle_t handle);

/* MSK_putlicensecode */
MSKrescodee (MSKAPI MSK_putlicensecode) (
	MSKenv_t env,
	const MSKint32t * code);

/* MSK_putlicensedebug */
MSKrescodee (MSKAPI MSK_putlicensedebug) (
	MSKenv_t env,
	MSKint32t licdebug);

/* MSK_putlicensepath */
MSKrescodee (MSKAPI MSK_putlicensepath) (
	MSKenv_t env,
	const char * licensepath);

/* MSK_putlicensewait */
MSKrescodee (MSKAPI MSK_putlicensewait) (
	MSKenv_t env,
	MSKint32t licwait);

/* MSK_rescodetostr */
MSKrescodee (MSKAPI MSK_rescodetostr) (
	MSKrescodee res,
	char * str);

/* MSK_resetexpirylicenses */
MSKrescodee (MSKAPI MSK_resetexpirylicenses) (
	MSKenv_t env);

/* MSK_sparsetriangularsolvedense */
MSKrescodee (MSKAPI MSK_sparsetriangularsolvedense) (
	MSKenv_t env,
	MSKtransposee transposed,
	MSKint32t n,
	const MSKint32t * lnzc,
	const MSKint64t * lptrc,
	MSKint64t lensubnval,
	const MSKint32t * lsubc,
	const MSKrealt * lvalc,
	MSKrealt * b);

/* MSK_syeig */
MSKrescodee (MSKAPI MSK_syeig) (
	MSKenv_t env,
	MSKuploe uplo,
	MSKint32t n,
	const MSKrealt * a,
	MSKrealt * w);

/* MSK_syevd */
MSKrescodee (MSKAPI MSK_syevd) (
	MSKenv_t env,
	MSKuploe uplo,
	MSKint32t n,
	MSKrealt * a,
	MSKrealt * w);

/* MSK_symnamtovalue */
MSKbooleant (MSKAPI MSK_symnamtovalue) (
	const char * name,
	char * value);

/* MSK_syrk */
MSKrescodee (MSKAPI MSK_syrk) (
	MSKenv_t env,
	MSKuploe uplo,
	MSKtransposee trans,
	MSKint32t n,
	MSKint32t k,
	MSKrealt alpha,
	const MSKrealt * a,
	MSKrealt beta,
	MSKrealt * c);

/* MSK_unlinkfuncfromenvstream */
MSKrescodee (MSKAPI MSK_unlinkfuncfromenvstream) (
	MSKenv_t env,
	MSKstreamtypee whichstream);

/* MSK_utf8towchar */
MSKrescodee (MSKAPI MSK_utf8towchar) (
	const size_t outputlen,
	size_t * len,
	size_t * conv,
	MSKwchart * output,
	const char * input);

/* MSK_wchartoutf8 */
MSKrescodee (MSKAPI MSK_wchartoutf8) (
	const size_t outputlen,
	size_t * len,
	size_t * conv,
	char * output,
	const MSKwchart * input);



#ifdef __cplusplus
}
#endif


#endif


