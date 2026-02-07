/*******************************************************/
/* Copyright (c) 2025 by Artelys                       */
/* All Rights Reserved                                 */
/*******************************************************/

/* Artelys Knitro 15.1.0 application interface header file. */

#ifndef KNITRO_H__
#define KNITRO_H__

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#include <stddef.h>
#include <float.h>

/*------------------------------------------------------------------*/
/*     EXPORT MACROS                                                */
/*------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
  #ifdef MAKE_KNITRO_DLL
    #define KNITRO_API __declspec(dllexport) __stdcall
  #else
    #define KNITRO_API __stdcall
  #endif
#else
  #if (__GNUC__ >= 4) || (__INTEL_COMPILER)
    #define KNITRO_API __attribute__ ((visibility ("default")))
  #else
    #define KNITRO_API
  #endif
#endif

/* Define Knitro integer types.  Use KNLONG to support 64-bit integers. */
typedef int KNINT;
#ifdef _WIN64
    typedef __int64 KNLONG;
#elif _WIN32
    typedef int KNLONG;
#else
    typedef long long KNLONG;
#endif
typedef KNINT   KNBOOL;
#define KNTRUE  1
#define KNFALSE 0

/*------------------------------------------------------------------*/
/*     FORWARD DECLARATION TYPE DEFINITIONS                         */
/*------------------------------------------------------------------*/

/** Type declaration for the Knitro solver context object.
 *  All methods pass a pointer to the solver.
 *  Applications must not modify any part of the solver context,
 *  except by making Knitro API calls.
 */
typedef struct KN_context KN_context, *KN_context_ptr;

/** Type declaration for the Artelys License Manager context object.
 *  Applications must not modify any part of the context.
 */
typedef struct LM_context LM_context, *LM_context_ptr;

/*------------------------------------------------------------------*/
/*     FUNCTION DECLARATIONS                                        */
/*------------------------------------------------------------------*/

/** Copy the Knitro release name into "release".  This variable must be
 *  preallocated to have "length" elements, including the string termination
 *  character.  For compatibility with future releases, please allocate at
 *  least 15 characters.
 *  Returns 0 if OK, nonzero if error.
 */
int  KNITRO_API KN_get_release (const int           length,
                                      char * const  release);

/* -----  Creating and destroying solver objects ----- */

/** Call KN_new first.  This routine returns a pointer to the
 *  solver object that is used in all other Knitro API calls in the
 *  argument "kc". The "kc" pointer is set to NULL if there is an error.
 *  A new Knitro license is acquired and held until KN_free has been
 *  called, or until the calling program ends.
 *  Returns 0 if OK, nonzero if error.
 */
int  KNITRO_API KN_new (KN_context_ptr * kc);


/** Free all memory and release any Knitro license acquired by calling
 *  KN_new.  The address of the pointer is set to NULL after freeing
 *  memory, to help avoid mistakes.
 *  Returns 0 if OK, nonzero if error.
 */
int  KNITRO_API KN_free (KN_context_ptr * kc);

/* -----  Creating and destroying solvers in high volume ----- */

/** High volume applications construct new Knitro instances repeatedly,
 *  each lasting only a short time.  Special functions allow a single
 *  license to be checked out once for a sequence of Knitro instances.
 *  Re-using a license saves time in the Knitro solver, and improves the
 *  performance of the Artelys License Manager server.
 *
 *  The typical calling sequence is:
 *    KN_checkout_license
 *      KN_new_lm
 *        KN_add* (setup problem structure)
 *        KN_solve
 *      KN_free
 *      KN_new_lm
 *        KN_add* (setup problem structure)
 *        KN_solve
 *      KN_free
 *      ...
 *    KN_release_license
 *
 *  Please see the Artelys License Manager user manual for more information.
 */

/** Allocate memory for a license from the Artelys License Manager for high
 *  volume Knitro applications.  The license object is returned in the "lmc"
 *  argument and will be set to NULL if there is an error.  Otherwise, the
 *  license will be checked out the first time KN_new_lm is called.  The
 *  license must be checked in later by calling KN_release_license.
 *  Returns 0 if OK, nonzero if error.
 */
int  KNITRO_API KN_checkout_license (LM_context_ptr * lmc);

/** Returns a pointer to the solver object that is used in all other Knitro
 *  API calls in the argument "kc". The "kc" pointer is set to NULL if
 *  there is an error. Pass in the license object "lmc" acquired by first
 *  calling KN_checkout_license.
 *  Returns 0 if OK, nonzero if error.
 */
int  KNITRO_API KN_new_lm (LM_context_ptr   lmc,
                           KN_context_ptr * kc);

/** Release the Knitro license and free allocated memory.
 *  Knitro will set the address of the pointer to NULL after freeing
 *  memory, to help avoid mistakes.
 *  Returns 0 if OK, nonzero if error.
 */
int  KNITRO_API KN_release_license (LM_context_ptr * lmc);


/* ----- Changing and reading solver parameters ----- */

/** All methods return 0 if OK, nonzero if there was an error.
 *  In most cases, parameter values are not validated until
 *  KN_solve is called.
 */

/** Reset all parameters to default values.
 */
int  KNITRO_API KN_reset_params_to_defaults (KN_context_ptr  kc);


/** Set all parameters specified in the given file. */
int  KNITRO_API KN_load_param_file
    (KN_context_ptr  kc, const char * const  filename);

/** Similar to KN_load_param_file (defined above) but specifically allows
 *  the user to specify a file of options (and option values) to explore for
 *  the Knitro-Tuner.
 */
int  KNITRO_API KN_load_tuner_file
    (KN_context_ptr  kc, const char * const  filename);

/** Write all current parameter values to a file. */
int  KNITRO_API KN_save_param_file
    (KN_context_ptr  kc, const char * const  filename);

/** Set a parameter using its string name. */
int  KNITRO_API KN_set_int_param_by_name
    (KN_context_ptr  kc, const char * const  name, const int  value);
int  KNITRO_API KN_set_char_param_by_name
    (KN_context_ptr  kc, const char * const  name, const char * const  value);
int  KNITRO_API KN_set_double_param_by_name
    (KN_context_ptr  kc, const char * const  name, const double  value);
/** Set an integer or double parameter by name. */
int  KNITRO_API KN_set_param_by_name
    (KN_context_ptr  kc, const char * const  name, const double  value);

/** Set a parameter using its integer identifier KN_PARAM_x
 *  (defined at the end of this file). */
int  KNITRO_API KN_set_int_param
    (KN_context_ptr  kc, const int  param_id, const int  value);
int  KNITRO_API KN_set_char_param
    (KN_context_ptr  kc, const int  param_id, const char * const  value);
int  KNITRO_API KN_set_double_param
    (KN_context_ptr  kc, const int  param_id, const double  value);

/** Get a parameter using its string name. */
int  KNITRO_API KN_get_int_param_by_name
    (KN_context_ptr  kc, const char * const  name, int * const  value);
int  KNITRO_API KN_get_double_param_by_name
    (KN_context_ptr  kc, const char * const  name, double * const  value);

/** Get a parameter using its integer identifier KN_PARAM_x
 *  (defined at the end of this file). */
int  KNITRO_API KN_get_int_param
    (KN_context_ptr  kc, const int  param_id, int * const  value);
int  KNITRO_API KN_get_double_param
    (KN_context_ptr  kc, const int  param_id, double * const  value);

/** Sets the string param_name with the name of parameter indexed by
 *  param_id and returns 0. Returns an error if param_id does not
 *  correspond to any parameter, or if the parameter output_size
 *  (the size of char array param_name) is less than the size of the
 *  parameter's description.
 */
int KNITRO_API KN_get_param_name(      KN_context_ptr  kc,
                                 const int             param_id,
                                       char * const    param_name,
                                 const size_t          output_size);

/** Sets the string description with the description of the parameter
 *  indexed by param_id and its possible values and returns 0. Returns an
 *  error if param_id does not correspond to any parameter, or if the
 *  parameter output_size (the size of char array description) is less than
 *  the size of the parameter's description.
 */
int KNITRO_API KN_get_param_doc(      KN_context_ptr  kc,
                                const int             param_id,
                                      char * const    description,
                                const size_t          output_size);

/** Sets the int * param_type to the parameter type of parameter indexed
 *  by param_id. Possible values are KN_PARAMTYPE_INTEGER, KN_PARAMTYPE_FLOAT,
 *  KN_PARAMTYPE_STRING. Returns an error if param_id does not correspond to
 *  any parameter.
 */
int KNITRO_API KN_get_param_type(      KN_context_ptr  kc,
                                 const int             param_id,
                                       int * const     param_type);

/** Set the int * num_param_values to the number of possible parameter
 *  values for parameter indexed by param_id and returns 0. If there is
 *  not a finite number of possible values, num_param_values will be zero.
 *  Returns an error if param_id does not correspond to any parameter.
 */
int KNITRO_API KN_get_num_param_values(      KN_context_ptr  kc,
                                       const int             param_id,
                                             int * const     num_param_values);

/** Set string param_value_string to the description of value value_id for the
 *  parameter param_id. Returns an error if param_id does not
 *  correspond to any parameter, or if value_id is not among possible parameter
 *  values, or if there are not a finite number of possible parameter values,
 *  or if the parameter output_size (the size of char array param_value_string)
 *  is less than the size of the parameter's description.
 */
int KNITRO_API KN_get_param_value_doc(      KN_context_ptr  kc,
                                      const int             param_id,
                                      const int             value_id,
                                            char * const    param_value_string,
                                      const size_t          output_size);

/** Set string param_value_string to the description of the i-th possible value
 *  for the parameter param_id. Returns an error if param_id does not
 *  correspond to any parameter, or if value_index is greater than the number
 *  of possible parameter values (see KN_get_num_param_values), or if there are
 *  not a finite number of possible parameter values, or if the parameter
 *  output_size (the size of char array param_value_string) is less than the
 *  size of the parameter's description.
 */
int KNITRO_API KN_get_param_value_doc_from_index(      KN_context_ptr  kc,
                                                 const int             param_id,
                                                 const int             value_index,
                                                       char * const    param_value_string,
                                                 const size_t          output_size);

/** Gets the integer value corresponding to the parameter name input and
 *  copies it into param_id input. Returns zero if successful and an error
 *  code otherwise.
 */
int KNITRO_API KN_get_param_id(      KN_context_ptr  kc,
                               const char * const    name,
                                     int  * const    param_id);

/**
 * Sets the integer param_id with the id of the parameter indexed by param_index
 * (between 0 and the number of existing parameters). Returns an error if the
 * param_index is non-positive or greater or equal than the number of existing
 * parameters.
 */
int KNITRO_API KN_get_param_id_from_index(      KN_context_ptr  kc,
                                                int*            param_id,
                                          const int             param_index);

/**
 * Write a JSON file describing all available parameters.
 * Returns zero if successful and an error code otherwise.
 */
int KNITRO_API KN_write_param_desc_file(      KN_context_ptr  kc,
                                        const char * const    filepath);


/* ----- Problem construction ----- */

/** The Knitro API is designed to provide the user maximum flexibility and
 *  ease-of-use in building a model.  In addition, it is designed to provide
 *  Knitro detailed structural information about the user model, so that
 *  Knitro can exploit special structures wherever possible to improve
 *  performance.  The user can use the API to build up a model in pieces in
 *  the way that is most convenient and intuitive for the user.
 *
 *  The API is designed so the user can load constant, linear, quadratic,
 *  and conic structures as well as complementarity constraints separately.
 *  This allows Knitro to mark these structure types internally and provide
 *  special care to different structure types. In addition, the more
 *  structural information Knitro has, the more extensive presolve operations
 *  Knitro can perform to try to simplify the model internally. For this
 *  reason we always recommend making use of the API functions below to provide
 *  as much fine-grained structural information as possible to Knitro.  More
 *  general nonlinear structure must be handled through callback evaluation
 *  routines.
 *
 *  Structures of the same type can be added in individual pieces, in groups,
 *  or all together.  Likewise, general nonlinear structures can all be handled
 *  by one callback object or broken up into separate callback objects if there
 *  are natural groupings that the user wants to treat differently.
 *
 *  The overhead costs for loading the user model by pieces using the API
 *  functions that follow should be trivial in most cases -- even for large
 *  models.  However, if not, it is always possible, and most efficient, to load
 *  all the structures of one type together in one API function call.
 *
 *  Problem structure is passed to Knitro using KN API functions.
 *  The problem is solved by calling KN_solve.  Applications must
 *  provide a means of evaluating the nonlinear objective, constraints,
 *  first derivatives, and (optionally) second derivatives.  (First
 *  derivatives are also optional, but highly recommended.)
 *
 *  The typical calling sequence is:
 *    KN_new
 *    KN_add_vars/KN_add_cons/KN_set_*bnds, etc. (problem setup)
 *    KN_add_*_linear_struct/KN_add_*_quadratic_struct (add special structures)
 *    KN_add_eval_callback (add callback for nonlinear evaluations if needed)
 *    KN_set_cb_* (set properties for nonlinear evaluation callbacks)
 *    KN_set_xxx_param (set any number of parameters/user options)
 *    KN_solve
 *    KN_free
 *
 *  Generally, as long as no nonlinear structural changes are made to the
 *  model, KN_solve can be called in succession to re-solve a model after
 *  changes.  For example, user options, variable bounds, and constraint
 *  bounds can be changed between calls to KN_solve, without having to call
 *  KN_free and reload the model from scratch.  In addition, constant and
 *  linear structures in the model may be added, deleted, or changed without
 *  having to reconstruct the model.  More extensive additions or changes to
 *  the model (such as adding quadratic structures or callbacks) require
 *  freeing the existing Knitro solver object and rebuilding the model from
 *  scratch.
 */

/** Add variables to the model.  The parameter indexVars may be set to NULL.
 *  Otherwise, on return it holds the global indices associated with the
 *  variables that were added (indices are typically allocated sequentially).
 *  Parameter indexVars can then be passed into other API routines that operate
 *  on the set of variables added through a particular call to KN_add_var*.
 *  Returns 0 if OK, nonzero if error.
 */
int  KNITRO_API KN_add_vars (      KN_context_ptr  kc,
                             const KNINT           nV,
                                   KNINT * const   indexVars);
int  KNITRO_API KN_add_var  (      KN_context_ptr  kc,
                                   KNINT * const   indexVar);

/** Add constraints to the model.  The parameter indexCons may be set to NULL.
 *  Otherwise, on return it holds the global indices associated with the
 *  constraints that were added (indices are typically allocated sequentially).
 *  Parameter indexCons can then be passed into other API routines that operate
 *  on the set of constraints added through a particular call to KN_add_con*.
 *  Returns 0 if OK, nonzero if error.
 */
int  KNITRO_API KN_add_cons (      KN_context_ptr  kc,
                             const KNINT           nC,
                                   KNINT * const   indexCons);
int  KNITRO_API KN_add_con  (      KN_context_ptr  kc,
                                   KNINT * const   indexCon);

/** Add residuals for least squares optimization.  The parameter indexRsds may
 *  be set to NULL.  Otherwise, on return it holds the global indices associated
 *  with the residuals that were added  (indices are typically allocated sequentially).
 *  Parameter indexRsds can then be passed into other API routines that operate
 *  on the set of residuals added through a particular call to KN_add_rsd*.
 *  Note that the current Knitro API does not support adding both constraints
 *  and residuals.
 *  Returns 0 if OK, nonzero if error.
 */
int  KNITRO_API KN_add_rsds (      KN_context_ptr  kc,
                             const KNINT           nR,
                                   KNINT * const   indexRsds);
int  KNITRO_API KN_add_rsd  (      KN_context_ptr  kc,
                                   KNINT * const   indexRsd);

/** Set lower, upper or fixed bounds on variables.
 *  If not set, variables are assumed to be unbounded (e.g. lower bounds
 *  are assumed to be -KN_INFINITY and upper bounds are assumed to be
 *  KN_INFINITY).
 *  Returns 0 if OK, nonzero if error.
 */
int  KNITRO_API KN_set_var_lobnds (      KN_context_ptr  kc,
                                   const KNINT           nV,
                                   const KNINT  * const  indexVars,
                                   const double * const  xLoBnds);
int  KNITRO_API KN_set_var_lobnds_all (      KN_context_ptr  kc,
                                       const double * const  xLoBnds);
int  KNITRO_API KN_set_var_lobnd (      KN_context_ptr  kc,
                                  const KNINT           indexVar,
                                  const double          xLoBnd);
int  KNITRO_API KN_set_var_upbnds (      KN_context_ptr  kc,
                                   const KNINT           nV,
                                   const KNINT  * const  indexVars,
                                   const double * const  xUpBnds);
int  KNITRO_API KN_set_var_upbnds_all (      KN_context_ptr  kc,
                                       const double * const  xUpBnds);
int  KNITRO_API KN_set_var_upbnd (      KN_context_ptr  kc,
                                  const KNINT           indexVar,
                                  const double          xUpBnd);
int  KNITRO_API KN_set_var_fxbnds (      KN_context_ptr  kc,
                                   const KNINT           nV,
                                   const KNINT  * const  indexVars,
                                   const double * const  xFxBnds);
int  KNITRO_API KN_set_var_fxbnds_all (      KN_context_ptr  kc,
                                       const double * const  xFxBnds);
int  KNITRO_API KN_set_var_fxbnd (      KN_context_ptr  kc,
                                  const KNINT           indexVar,
                                  const double          xFxBnd);

/** Get lower, upper or fixed bounds on variables.
 *  Returns 0 if OK, nonzero if error.
 */
int  KNITRO_API KN_get_var_lobnds (const KN_context_ptr  kc,
                                   const KNINT           nV,
                                   const KNINT  * const  indexVars,
                                         double * const  xLoBnds);
int  KNITRO_API KN_get_var_lobnds_all (const KN_context_ptr  kc,
                                             double * const  xLoBnds);
int  KNITRO_API KN_get_var_lobnd (const KN_context_ptr  kc,
                                  const KNINT           indexVar,
                                        double * const  xLoBnd);

int  KNITRO_API KN_get_var_upbnds (const KN_context_ptr  kc,
                                   const KNINT           nV,
                                   const KNINT  * const  indexVars,
                                         double * const  xUpBnds);
int  KNITRO_API KN_get_var_upbnds_all (const KN_context_ptr  kc,
                                             double * const  xUpBnds);
int  KNITRO_API KN_get_var_upbnd (const KN_context_ptr  kc,
                                  const KNINT           indexVar,
                                        double * const  xUpBnd);
int  KNITRO_API KN_get_var_fxbnds (const KN_context_ptr  kc,
                                   const KNINT           nV,
                                   const KNINT  * const  indexVars,
                                         double * const  xFxBnds);
int  KNITRO_API KN_get_var_fxbnds_all (const KN_context_ptr  kc,
                                             double * const  xFxBnds);
int  KNITRO_API KN_get_var_fxbnd (const KN_context_ptr  kc,
                                  const KNINT           indexVar,
                                        double * const  xFxBnd);

/** Set variable types (e.g. KN_VARTYPE_CONTINUOUS, KN_VARTYPE_BINARY,
 *  KN_VARTYPE_INTEGER). If not set, variables are assumed to be continuous.
 *  When a variable is set as binary, its lower bound is automatically
 *  set to 0 and its upper bound is automatically set to 1 at the same time.
 *  Returns 0 if OK, nonzero if error.
 */
int  KNITRO_API KN_set_var_types (      KN_context_ptr  kc,
                                  const KNINT           nV,
                                  const KNINT  * const  indexVars,
                                  const int    * const  xTypes);
int  KNITRO_API KN_set_var_types_all (      KN_context_ptr  kc,
                                      const int    * const  xTypes);
int  KNITRO_API KN_set_var_type (      KN_context_ptr  kc,
                                 const KNINT           indexVar,
                                 const int             xType);

/** Return the variable types. The array "xTypes" must be allocated
 *  by the user.
 *  Returns 0 if OK, nonzero if error.
 */
int  KNITRO_API KN_get_var_types (const KN_context_ptr  kc,
                                  const KNINT           nV,
                                  const KNINT  * const  indexVars,
                                        int    * const  xTypes);
int  KNITRO_API KN_get_var_types_all (const KN_context_ptr  kc,
                                            int    * const  xTypes);
int  KNITRO_API KN_get_var_type (const KN_context_ptr  kc,
                                 const KNINT           indexVar,
                                       int    * const  xType);

/** Specify some properties of the variables.  Currently
 *  this API routine is only used to mark variables as linear,
 *  but other variable properties will be added in the future.
 *  Note: use bit-wise specification of the features:
 *  bit value   meaning
 *    0     1   KN_VAR_LINEAR
 *  default = 0 (variables are assumed to be nonlinear)
 *
 *  If a variable only appears linearly in the model, it can be very
 *  helpful to mark this by enabling bit 0.  This information can then
 *  be used by Knitro to perform more extensive preprocessing. If a
 *  variable appears nonlinearly in any constraint or the objective (or
 *  if the user does not know) then it should not be marked as linear.
 *  Variables are assumed to be nonlinear variables by default.
 *  Knitro makes a local copy of all inputs, so the application may
 *  free memory after the call.
 *  Returns 0 if OK, nonzero if error.
 */
int  KNITRO_API KN_set_var_properties (      KN_context_ptr  kc,
                                       const KNINT           nV,
                                       const KNINT  * const  indexVars,
                                       const int    * const  xProperties);
int  KNITRO_API KN_set_var_properties_all (      KN_context_ptr  kc,
                                           const int    * const  xProperties);
int  KNITRO_API KN_set_var_property (      KN_context_ptr  kc,
                                     const KNINT           indexVar,
                                     const int             xProperty);

/** Set lower, upper or equality bounds on constraints.
 *  If not set, constraints are assumed to be unbounded (e.g. lower bounds
 *  are assumed to be -KN_INFINITY and upper bounds are assumed to be
 *  KN_INFINITY).
 *  Returns 0 if OK, nonzero if error.
 */
int  KNITRO_API KN_set_con_lobnds (      KN_context_ptr  kc,
                                   const KNINT           nC,
                                   const KNINT  * const  indexCons,
                                   const double * const  cLoBnds);
int  KNITRO_API KN_set_con_lobnds_all (      KN_context_ptr  kc,
                                       const double * const  cLoBnds);
int  KNITRO_API KN_set_con_lobnd (      KN_context_ptr  kc,
                                  const KNINT           indexCon,
                                  const double          cLoBnd);
int  KNITRO_API KN_set_con_upbnds (      KN_context_ptr  kc,
                                   const KNINT           nC,
                                   const KNINT  * const  indexCons,
                                   const double * const  cUpBnds);
int  KNITRO_API KN_set_con_upbnds_all (      KN_context_ptr  kc,
                                       const double * const  cUpBnds);
int  KNITRO_API KN_set_con_upbnd (      KN_context_ptr  kc,
                                  const KNINT           indexCon,
                                  const double          cUpBnd);
int  KNITRO_API KN_set_con_eqbnds (      KN_context_ptr  kc,
                                   const KNINT           nC,
                                   const KNINT  * const  indexCons,
                                   const double * const  cEqBnds);
int  KNITRO_API KN_set_con_eqbnds_all (      KN_context_ptr  kc,
                                       const double * const  cEqBnds);
int  KNITRO_API KN_set_con_eqbnd (      KN_context_ptr  kc,
                                  const KNINT           indexCon,
                                  const double          cEqBnd);

/** Get lower, upper or equality bounds on constraints.
 *  Returns 0 if OK, nonzero if error.
 */
int  KNITRO_API KN_get_con_lobnds (const KN_context_ptr  kc,
                                   const KNINT           nC,
                                   const KNINT  * const  indexCons,
                                         double * const  cLoBnds);
int  KNITRO_API KN_get_con_lobnds_all (const KN_context_ptr  kc,
                                             double * const  cLoBnds);
int  KNITRO_API KN_get_con_lobnd (const KN_context_ptr  kc,
                                  const KNINT           indexCon,
                                        double * const  cLoBnd);
int  KNITRO_API KN_get_con_upbnds (const KN_context_ptr  kc,
                                   const KNINT           nC,
                                   const KNINT  * const  indexCons,
                                         double * const  cUpBnds);
int  KNITRO_API KN_get_con_upbnds_all (const KN_context_ptr  kc,
                                             double * const  cUpBnds);
int  KNITRO_API KN_get_con_upbnd (const KN_context_ptr  kc,
                                  const KNINT           indexCon,
                                        double * const  cUpBnd);
int  KNITRO_API KN_get_con_eqbnds (const KN_context_ptr  kc,
                                   const KNINT           nC,
                                   const KNINT  * const  indexCons,
                                         double * const  cEqBnds);
int  KNITRO_API KN_get_con_eqbnds_all (const KN_context_ptr  kc,
                                             double * const  cEqBnds);
int  KNITRO_API KN_get_con_eqbnd (const KN_context_ptr  kc,
                                  const KNINT           indexCon,
                                        double * const  cEqBnd);

/** Specify some properties of the objective and constraint functions.
 *  Note: use bit-wise specification of the features:
 *  bit value   meaning
 *    0     1   KN_OBJ_CONVEX/KN_CON_CONVEX
 *    1     2   KN_OBJ_CONCAVE/KN_CON_CONCAVE
 *    2     4   KN_OBJ_CONTINUOUS/KN_CON_CONTINUOUS
 *    3     8   KN_OBJ_DIFFERENTIABLE/KN_CON_DIFFERENTIABLE
 *    4    16   KN_OBJ_TWICE_DIFFERENTIABLE/KN_CON_TWICE_DIFFERENTIABLE
 *    5    32   KN_OBJ_NOISY/KN_CON_NOISY
 *    6    64   KN_OBJ_NONDETERMINISTIC/KN_CON_NONDETERMINISTIC
 * default = 28 (bits 2-4 enabled: e.g. continuous, differentiable, twice-differentiable)
 */
int  KNITRO_API KN_set_obj_property (      KN_context_ptr  kc,
                                     const int             objProperty);
int  KNITRO_API KN_set_con_properties (      KN_context_ptr  kc,
                                       const KNINT           nC,
                                       const KNINT  * const  indexCons,
                                       const int    * const  cProperties);
int  KNITRO_API KN_set_con_properties_all (      KN_context_ptr  kc,
                                           const int    * const  cProperties);
int  KNITRO_API KN_set_con_property (      KN_context_ptr  kc,
                                     const KNINT           indexCon,
                                     const int             cProperty);

/** Set the objective goal (KN_OBJGOAL_MINIMIZE or KN_OBJGOAL_MAXIMIZE).
 *  Returns 0 if OK, nonzero if error.
 */
int  KNITRO_API KN_set_obj_goal (      KN_context_ptr  kc,
                                 const int             objGoal);

/** Get the objective goal (KN_OBJGOAL_MINIMIZE or KN_OBJGOAL_MAXIMIZE).
 *  Returns 0 if OK, nonzero if error.
 */
int  KNITRO_API KN_get_obj_goal (      KN_context_ptr  kc,
                                       int * const     objGoal);

/** Set initial values for primal variables.  If not set, variables
 *  may be initialized as 0 or initialized by Knitro based on some
 *  initialization strategy (perhaps determined by a user option).
 *  Returns 0 if OK, nonzero if error.
 */
int  KNITRO_API KN_set_var_primal_init_values (      KN_context_ptr  kc,
                                               const KNINT           nV,
                                               const KNINT  * const  indexVars,
                                               const double * const  xInitVals);
int  KNITRO_API KN_set_var_primal_init_values_all (      KN_context_ptr  kc,
                                                   const double * const  xInitVals);
int  KNITRO_API KN_set_var_primal_init_value (      KN_context_ptr  kc,
                                              const KNINT           indexVar,
                                              const double          xInitVal);

/** Set initial values for dual variables (i.e. the Lagrange multipliers
 *  corresponding to the potentially bounded variables).  If not set, dual
 *  variables may be initialized as 0 or initialized by Knitro based on some
 *  initialization strategy (perhaps determined by a user option).
 *  Returns 0 if OK, nonzero if error.
 */
int  KNITRO_API KN_set_var_dual_init_values (      KN_context_ptr  kc,
                                             const KNINT           nV,
                                             const KNINT  * const  indexVars,
                                             const double * const  lambdaInitVals);
int  KNITRO_API KN_set_var_dual_init_values_all (      KN_context_ptr  kc,
                                                 const double * const  lambdaInitVals);
int  KNITRO_API KN_set_var_dual_init_value (      KN_context_ptr  kc,
                                            const KNINT           indexVar,
                                            const double          lambdaInitVal);

/** Set initial values for constraint dual variables (i.e. the Lagrange
 *  multipliers for the constraints).  If not set, constraint dual
 *  variables may be initialized as 0 or initialized by Knitro based on some
 *  initialization strategy (perhaps determined by a user option).
 *  Returns 0 if OK, nonzero if error.
 */
int  KNITRO_API KN_set_con_dual_init_values (      KN_context_ptr  kc,
                                             const KNINT           nC,
                                             const KNINT  * const  indexCons,
                                             const double * const  lambdaInitVals);
int  KNITRO_API KN_set_con_dual_init_values_all (      KN_context_ptr  kc,
                                                 const double * const  lambdaInitVals);
int  KNITRO_API KN_set_con_dual_init_value (      KN_context_ptr  kc,
                                            const KNINT           indexCon,
                                            const double          lambdaInitVal);

/*----- Adding/removing/changing constant structure ----- */

/** Add a constant to the objective function. */
int  KNITRO_API KN_add_obj_constant (      KN_context_ptr  kc,
                                     const double          constant);

/** Delete all constant terms from the objective function.
 *  Only constant terms existing from a previous solve (i.e. added before the most
 *  recent call to "KN_solve") may be removed.
 */
int  KNITRO_API KN_del_obj_constant (      KN_context_ptr  kc);

/** Change constant term in the objective function.
 *  Equivalent to calling KN_del_obj_constant() + KN_add_obj_constant().
 */
int  KNITRO_API KN_chg_obj_constant (      KN_context_ptr  kc,
                                     const double          constant);

/** Add constants to the body of constraint functions.
 *  Each component i of arrays indexCons and constants adds a constant term
 *  constants[i] to the constraint c[indexCons[i]].
 */
int  KNITRO_API KN_add_con_constants (      KN_context_ptr  kc,
                                      const KNINT           nC,
                                      const KNINT  * const  indexCons,       /* size = nC */
                                      const double * const  constants);      /* size = nC */
int  KNITRO_API KN_add_con_constants_all (      KN_context_ptr  kc,
                                          const double * const  constants);
int  KNITRO_API KN_add_con_constant (      KN_context_ptr  kc,
                                     const KNINT           indexCon,
                                     const double          constant);

/** Delete constant terms from the body of constraint functions.
 *  Each component i of array indexCons deletes all constant terms
 *  from the constraint c[indexCons[i]].
 *  Only constant terms existing from a previous solve (i.e. added before the most
 *  recent call to "KN_solve") may be removed.
 */
int  KNITRO_API KN_del_con_constants (      KN_context_ptr  kc,
                                      const KNINT           nC,
                                      const KNINT  * const  indexCons);      /* size = nC */
int  KNITRO_API KN_del_con_constants_all (      KN_context_ptr  kc);
int  KNITRO_API KN_del_con_constant (      KN_context_ptr  kc,
                                     const KNINT           indexCon);

/** Change constant terms in the body of constraint functions.
 *  Each component i of arrays indexCons and constants changes the constant term
 *  in the constraint c[indexCons[i]] to constants[i].
 *  Equivalent to calling KN_del_con_constants() + KN_add_con_constants().
 */
int  KNITRO_API KN_chg_con_constants (      KN_context_ptr  kc,
                                      const KNINT           nC,
                                      const KNINT  * const  indexCons,       /* size = nC */
                                      const double * const  constants);      /* size = nC */
int  KNITRO_API KN_chg_con_constants_all (      KN_context_ptr  kc,
                                          const double * const  constants);
int  KNITRO_API KN_chg_con_constant (      KN_context_ptr  kc,
                                     const KNINT           indexCon,
                                     const double          constant);

/** Add constants to the body of residual functions.
 *  Each component i of arrays indexRsds and constants adds a constant term
 *  constants[i] to the residual r[indexRsds[i]].
 */
int  KNITRO_API KN_add_rsd_constants (      KN_context_ptr  kc,
                                      const KNINT           nR,
                                      const KNINT  * const  indexRsds,       /* size = nR */
                                      const double * const  constants);      /* size = nR */
int  KNITRO_API KN_add_rsd_constants_all (      KN_context_ptr  kc,
                                          const double * const  constants);
int  KNITRO_API KN_add_rsd_constant (      KN_context_ptr  kc,
                                     const KNINT           indexRsd,
                                     const double          constant);

/*----- Adding/removing/changing linear structure ----- */

/** Add linear structure to the objective function.
 *  Each component i of arrays indexVars and coefs adds a linear term
 *     coefs[i]*x[indexVars[i]]
 *  to the objective.
 *
 *  Use "KN_add_obj_linear_struct()" to add several linear objective structures
 *  at once, and "KN_add_obj_linear_term()" to add a single linear objective
 *  term.
 */
int  KNITRO_API KN_add_obj_linear_struct (      KN_context_ptr  kc,
                                          const KNINT           nnz,
                                          const KNINT  * const  indexVars,   /* size = nnz */
                                          const double * const  coefs);      /* size = nnz */
int  KNITRO_API KN_add_obj_linear_term (      KN_context_ptr  kc,
                                        const KNINT           indexVar,
                                        const double          coef);

/** Delete linear structure from the objective function.
 *  Each component i of array indexVars removes the linear objective terms
 *  corresponding to variable x[indexVars[i]].
 *  Only linear terms existing from the latest model update (i.e. added before
 *  the most recent call to "KN_solve" or "KN_update") will be removed.
 *
 *  Use "KN_del_obj_linear_struct()" to delete several linear objective structures
 *  at once, "KN_del_obj_linear_term()" to delete a single linear objective
 *  term, "KN_del_obj_linear_struct_all()" to delete all linear objective terms.
 */
int  KNITRO_API KN_del_obj_linear_struct (      KN_context_ptr  kc,
                                          const KNINT           nnz,
                                          const KNINT  * const  indexVars);   /* size = nnz */
int  KNITRO_API KN_del_obj_linear_term (      KN_context_ptr  kc,
                                        const KNINT           indexVar);
int  KNITRO_API KN_del_obj_linear_struct_all (KN_context_ptr  kc);

/** Change linear structure in the objective function.
 *  Each component i of arrays indexVars and coefs changes the coefficient
 *  value of the linear term
 *     coefs[i]*x[indexVars[i]]
 *  in the objective.
 *  Equivalent to calling KN_del_obj_linear_struct() + KN_add_obj_linear_struct().
 *
 *  Use "KN_chg_obj_linear_struct()" to change several linear objective structures
 *  at once, and "KN_chg_obj_linear_term()" to change a single linear objective
 *  term.
 */
int  KNITRO_API KN_chg_obj_linear_struct (      KN_context_ptr  kc,
                                          const KNINT           nnz,
                                          const KNINT  * const  indexVars,   /* size = nnz */
                                          const double * const  coefs);      /* size = nnz */
int  KNITRO_API KN_chg_obj_linear_term (      KN_context_ptr  kc,
                                        const KNINT           indexVar,
                                        const double          coef);

/** Add linear structure to the constraint functions.
 *  Each component i of arrays indexCons, indexVars and coefs adds a linear
 *  term:
 *     coefs[i]*x[indexVars[i]]
 *  to constraint c[indexCons[i]].
 *
 *  Use "KN_add_con_linear_struct()" to add linear structure for a group of
 *  constraints at once, "KN_add_con_linear_struct_one()" to add linear
 *  structure for just one constraint, and "KN_add_con_linear_term()" to
 *  add a single linear constraint term.
 */
int  KNITRO_API KN_add_con_linear_struct (      KN_context_ptr  kc,
                                          const KNLONG          nnz,
                                          const KNINT  * const  indexCons,      /* size = nnz */
                                          const KNINT  * const  indexVars,      /* size = nnz */
                                          const double * const  coefs);         /* size = nnz */
int  KNITRO_API KN_add_con_linear_struct_one (      KN_context_ptr  kc,
                                              const KNLONG          nnz,
                                              const KNINT           indexCon,
                                              const KNINT  * const  indexVars,  /* size = nnz */
                                              const double * const  coefs);     /* size = nnz */
int  KNITRO_API KN_add_con_linear_term (      KN_context_ptr  kc,
                                        const KNINT           indexCon,
                                        const KNINT           indexVar,
                                        const double          coef);

/** Delete linear structure from the constraint functions.
 *  Each component i of arrays indexCons and indexVars removes the linear constraint
 *  terms corresponding to variable x[indexVars[i]] in constraint c[indexCons[i]].
 *  Only linear terms existing from the latest model update (i.e. added before
 *  the most recent call to "KN_solve" or "KN_update") will be removed.
 *
 *  Use "KN_del_con_linear_struct()" to delete linear structure from a group of
 *  constraints at once, "KN_del_con_linear_struct_one()" to delete linear
 *  structure from just one constraint, "KN_del_con_linear_term()" to
 *  delete a single linear constraint term and KN_del_con_linear_struct_all()
 *  to delete all linear terms from a set of constraints.
 */
int  KNITRO_API KN_del_con_linear_struct (      KN_context_ptr  kc,
                                          const KNLONG          nnz,
                                          const KNINT  * const  indexCons,      /* size = nnz */
                                          const KNINT  * const  indexVars);     /* size = nnz */
int  KNITRO_API KN_del_con_linear_struct_one (      KN_context_ptr  kc,
                                              const KNLONG          nnz,
                                              const KNINT           indexCon,
                                              const KNINT  * const  indexVars); /* size = nnz */
int  KNITRO_API KN_del_con_linear_struct_all (      KN_context_ptr  kc,
                                              const KNINT           nC,
                                              const KNINT  * const  indexCons); /* size = nC */
int  KNITRO_API KN_del_con_linear_term (      KN_context_ptr  kc,
                                        const KNINT           indexCon,
                                        const KNINT           indexVar);

/** Change linear structure in the constraint functions.
 *  Each component i of arrays indexCons, indexVars and coefs changes the coefficient
 *  value of the linear term:
 *     coefs[i]*x[indexVars[i]]
 *  in constraint c[indexCons[i]].
 *  Equivalent to calling KN_del_con_linear_struct() + KN_add_con_linear_struct().
 *
 *  Use "KN_chg_con_linear_struct()" to change linear structure for a group of
 *  constraints at once, "KN_chg_con_linear_struct_one()" to change linear
 *  structure for just one constraint, and "KN_chg_con_linear_term()" to
 *  change a single linear constraint term.
 */
int  KNITRO_API KN_chg_con_linear_struct (      KN_context_ptr  kc,
                                          const KNLONG          nnz,
                                          const KNINT  * const  indexCons,      /* size = nnz */
                                          const KNINT  * const  indexVars,      /* size = nnz */
                                          const double * const  coefs);         /* size = nnz */
int  KNITRO_API KN_chg_con_linear_struct_one (      KN_context_ptr  kc,
                                              const KNLONG          nnz,
                                              const KNINT           indexCon,
                                              const KNINT  * const  indexVars,  /* size = nnz */
                                              const double * const  coefs);     /* size = nnz */
int  KNITRO_API KN_chg_con_linear_term (      KN_context_ptr  kc,
                                        const KNINT           indexCon,
                                        const KNINT           indexVar,
                                        const double          coef);

/** Add linear structure to the residual functions.
 *  Each component i of arrays indexRsds, indexVars and coefs adds a linear
 *  term:
 *     coefs[i]*x[indexVars[i]]
 *  to residual r[indexRsds[i]].
 *
 *  Use "KN_add_rsd_linear_struct()" to add linear structure for a group of
 *  residuals at once, "KN_add_rsd_linear_struct_one()" to add linear
 *  structure for just one residual, and "KN_add_rsd_linear_term()" to
 *  add a single linear residual term.
 */
int  KNITRO_API KN_add_rsd_linear_struct (      KN_context_ptr  kc,
                                          const KNLONG          nnz,
                                          const KNINT  * const  indexRsds,      /* size = nnz */
                                          const KNINT  * const  indexVars,      /* size = nnz */
                                          const double * const  coefs);         /* size = nnz */
int  KNITRO_API KN_add_rsd_linear_struct_one (      KN_context_ptr  kc,
                                              const KNLONG          nnz,
                                              const KNINT           indexRsd,
                                              const KNINT  * const  indexVars,  /* size = nnz */
                                              const double * const  coefs);     /* size = nnz */
int  KNITRO_API KN_add_rsd_linear_term (      KN_context_ptr  kc,
                                        const KNINT           indexRsd,
                                        const KNINT           indexVar,
                                        const double          coef);

/*----- Adding quadratic structure ----- */

/** Add quadratic structure to the objective function.
 *  Each component i of arrays indexVars1, indexVars2 and coefs adds a
 *  quadratic term
 *     coefs[i]*x[indexVars1[i]]*x[indexVars2[i]]
 *  to the objective.
 *
 *  Use "KN_add_obj_quadratic_struct()" to add several quadratic objective
 *  structures at once, and "KN_add_obj_quadratic_term()" to add a single
 *  quadratic objective term.
 *
 *  Note: if indexVars2[i] is < 0 then it adds a linear term
 *        coefs[i]*x[indexVars1[i]] instead.
 */
int  KNITRO_API KN_add_obj_quadratic_struct (      KN_context_ptr  kc,
                                             const KNLONG          nnz,
                                             const KNINT  * const  indexVars1,  /* size = nnz */
                                             const KNINT  * const  indexVars2,  /* size = nnz */
                                             const double * const  coefs);      /* size = nnz */
int  KNITRO_API KN_add_obj_quadratic_term (      KN_context_ptr  kc,
                                           const KNINT           indexVar1,
                                           const KNINT           indexVar2,
                                           const double          coef);

/** Delete quadratic structure from the objective function.
 *
 *  "KN_del_obj_quadratic_struct" will remove each component i of arrays
 *  indexVars1 and indexVars2 from the objective. That is any coefficient c:
 *     c*indexVars1[i]*indexVars2[i]
 *  "KN_del_obj_quadratic_struct_all" will delete all quadratic structure from
 *  the objective function.
 *
 *  Only quadratic terms existing from the latest model update (i.e. added
 *  before the most recent call to "KN_solve" or "KN_update") will be removed.
 */
int  KNITRO_API KN_del_obj_quadratic_struct(      KN_context_ptr  kc,
                                            const KNLONG          nnz,
                                            const KNINT  * const  indexVars1,  /* size = nnz */
                                            const KNINT  * const  indexVars2); /* size = nnz */
int  KNITRO_API KN_del_obj_quadratic_struct_all (KN_context_ptr  kc);

/** Add quadratic structure to the constraint functions.
 *  Each component i of arrays indexCons, indexVars1, indexVars2 and coefs adds a
 *  quadratic term:
 *     coefs[i]*x[indexVars1[i]]*x[indexVars2[i]]
 *  to the constraint c[indexCons[i]].
 *
 *  Use "KN_add_con_quadratic_struct()" to add quadratic structure for a group of
 *  constraints at once, "KN_add_con_quadratic_struct_one()" to add quadratic
 *  structure for just one constraint, and "KN_add_con_quadratic_term()" to
 *  add a single quadratic constraint term.
 *
 *  Note: if indexVars2[i] is < 0 then it adds a linear term
 *        coefs[i]*x[indexVars1[i]] instead.
 */
int  KNITRO_API KN_add_con_quadratic_struct (      KN_context_ptr  kc,
                                             const KNLONG          nnz,
                                             const KNINT  * const  indexCons,   /* size = nnz */
                                             const KNINT  * const  indexVars1,  /* size = nnz */
                                             const KNINT  * const  indexVars2,  /* size = nnz */
                                             const double * const  coefs);      /* size = nnz */
int  KNITRO_API KN_add_con_quadratic_struct_one (      KN_context_ptr  kc,
                                                 const KNLONG          nnz,
                                                 const KNINT           indexCon,
                                                 const KNINT  * const  indexVars1,  /* size = nnz */
                                                 const KNINT  * const  indexVars2,  /* size = nnz */
                                                 const double * const  coefs);      /* size = nnz */
int  KNITRO_API KN_add_con_quadratic_term (      KN_context_ptr  kc,
                                           const KNINT           indexCon,
                                           const KNINT           indexVar1,
                                           const KNINT           indexVar2,
                                           const double          coef);

/** Delete quadratric structure from the constraint functions.
 *  Each component i of arrays indexCons, indexVars1 and indexVars2 removes the
 *  quadratic constraint terms corresponding to variables x[indexVars1[i]] and
 *  x[indexVars2[i]] in constraint c[indexCons[i]].
 *  Only quadratic terms existing from the latest model update (i.e. added
 *  before the most recent call to "KN_solve" or "KN_update") will be removed.
 */
int  KNITRO_API KN_del_con_quadratic_struct(      KN_context_ptr  kc,
                                            const KNLONG          nnz,
                                            const KNINT  * const  indexCons,   /* size = nnz */
                                            const KNINT  * const  indexVars1,  /* size = nnz */
                                            const KNINT  * const  indexVars2); /* size = nnz */

/*----- Adding conic structure ----- */

/** Add L2 norm structure of the form ||Ax + b||_2 to a constraint.
 *    indexCon:    The constraint index that the L2 norm term will be added to.
 *    nCoords:     The number of rows in "A" (or dimension of "b")
 *    nnz:         The number of sparse non-zero elements in "A"
 *    indexCoords: The coordinate (row) index for each non-zero element in "A".
 *    indexVars:   The variable (column) index for each non-zero element in "A"
 *    coefs:       The coefficient value for each non-zero element in "A"
 *    constants:   The array "b" - may be set to NULL to ignore "b"
 *
 *  Note: L2 norm structure can currently only be added to constraints that
 *        otherwise only have linear (or constant) structure.  In this way
 *        they can be used to define conic constraints of the form
 *        ||Ax + b|| <= c'x + d.  The "c" coefficients should be added through
 *        "KN_add_con_linear_struct()" and "d" can be set as a constraint bound
 *        or through "KN_add_con_constants()".
 *
 *  Note: Models with L2 norm structure are currently only handled by the
 *        Interior/Direct (KN_ALG_BAR_DIRECT) algorithm in Knitro.  Any model
 *        with structure defined with KN_add_con_L2norm() will automatically be
 *        forced to use this algorithm.
 */
int  KNITRO_API KN_add_con_L2norm (      KN_context_ptr  kc,
                                   const KNINT           indexCon,
                                   const KNINT           nCoords,
                                   const KNLONG          nnz,
                                   const KNINT  * const  indexCoords,  /* size = nnz */
                                   const KNINT  * const  indexVars,    /* size = nnz */
                                   const double * const  coefs,        /* size = nnz */
                                   const double * const  constants);   /* size = nCoords or NULL */

/*----- Adding complementarity constraints ----- */

/** This function adds complementarity constraints to the problem.
 *  The parameter indexCompCons may be set to NULL. Otherwise, on return it
 *  holds the global indices associated with the complementarity constraints
 *  that were added (indices are typically allocated sequentially). Parameter
 *  indexCompCons can then be passed into other API routines that operate
 *  on the set of complementarity constraints added through a particular call
 *  to KN_add_compcon*.
 *  The two lists indexComps1/indexComps2 are of equal length, and contain
 *  nCC matching pairs of variable indices.  Each pair defines a complementarity
 *  constraint between the two variables.
 *  The array ccTypes specifies the type of complementarity:
 *     KN_CCTYPE_VARVAR: two (non-negative) variables
 *     KN_CCTYPE_VARCON: a variable and a constraint
 *     KN_CCTYPE_CONCON: two constraints
 *  Note: Currently only KN_CCTYPE_VARVAR is supported.  The other
 *        ccTypes will be added in future releases.
 *  Returns 0 if OK, or a negative value on error.
 */
int  KNITRO_API KN_add_compcons (      KN_context_ptr  kc,
                                 const KNINT           nCC,
                                 const int    * const  ccTypes,
                                 const KNINT  * const  indexComps1,
                                 const KNINT  * const  indexComps2,
                                       KNINT  * const  indexCompCons);
int  KNITRO_API KN_add_compcon  (      KN_context_ptr  kc,
                                 const int             ccType,
                                 const KNINT           indexComp1,
                                 const KNINT           indexComp2,
                                       KNINT  * const  indexCompCon);

/** This function sets all the complementarity constraints for the
 *  problem in one call. The function can only be called once.
 *  Returns 0 if OK, or a negative value on error.
 *  This function has been superseded by the KN_add_compcons function.
 */
int  KNITRO_API KN_set_compcons (      KN_context_ptr  kc,
                                 const KNINT           nCC,
                                 const int    * const  ccTypes,
                                 const KNINT  * const  indexComps1,
                                 const KNINT  * const  indexComps2);

/* ----- Loading MPS file ----- */

/** This function loads the problem specified in the MPS file "filename".
 *  The number of variables and constraints is specified in the MPS file,
 *  as well as the nature of the objective and the different constraints
 *  (should they be linear or quadratic).
 *  Return 0 if OK, or a negative value on error.
 */
int KNITRO_API KN_load_mps_file (    KN_context_ptr kc,
                                 const char * const filename);

int KNITRO_API KN_write_mps_file (KN_context_ptr kc,
                                    const char * filename);

/**
 * This function loads a problem from a file. The file format is deduced from
 * the file extension (case insensitive) or from the "read_options" flags.
 * Supported file formats are:
 *     MPS files with extension ".mps"
 *     CBF files with extension ".cbf" (experimental)
 *     LP files with extension ".lp"
 * Additional reading options can be given as a list of characters with
 * "read_options":
 *     'm': parsing MPS file without interpreting file extension
 *     'c': parsing CBF file without interpreting file extension
 *     'l': parsing LP file without interpreting file extension
 *
 *  Return 0 if OK, or a negative value on error.
 */
int KNITRO_API KN_read_problem(    KN_context_ptr kc,
                               const char * const filename,
                               const char * const read_options);


/**
 * This function writes a problem to a file. The file format is deduced from
 * the file extension (case insensitive) or from the "write_options" flags.
 * Supported file formats are:
 *     MPS files with extension ".mps"
 * Additional writing options can be given as a list of characters with
 * "write_options":
 *     'm': write in MPS format without interpreting file extension
 *     'l': write in LP format without interpreting file extension
 *     'b': write bounds at the beginning of the file. This option is only
 *          available when writing in LP format.
 *
 *  Return 0 if OK, or a negative value on error.
 */
int KNITRO_API KN_write_problem(    KN_context_ptr kc,
                               const char * const filename,
                               const char * const write_options);

/* ----- Callbacks ----- */

/** Applications may define functions for evaluating problem elements
 *  at a trial point.  The functions must match the prototype defined
 *  below, and passed to Knitro with the appropriate KN_set_cb_* call.
 *  Knitro may request different types of evaluation information,
 *  as specified in "evalRequest.type":
 *    KN_RC_EVALFC   - return objective and constraint function values
 *    KN_RC_EVALGA   - return first derivative values in "objGrad" and "jac"
 *    KN_RC_EVALFCGA - return objective and constraint function values
 *                     AND first derivative "objGrad" and "jac"
 *    KN_RC_EVALH    - return second derivative values in "hessian"
 *    KN_RC_EVALH_NO_F  (this version excludes the objective term)
 *    KN_RC_EVALHV   - return a Hessian-vector product in "hessVector"
 *    KN_RC_EVALHV_NO_F (this version excludes the objective term)
 *    KN_RC_EVALR    - return residual function values for least squares
 *    KN_RC_EVALRJ   - return residual Jacobian values for least squares
 *
 *  The argument "lambda" is not defined when requesting EVALFC, EVALGA,
 *  EVALFCGA, EVALR or EVALRJ.
 *    Usually, applications for standard optimization models define three
 *  callback functions: one for EVALFC, one for EVALGA, and one for EVALH / EVALHV.
 *  The last function is only used when providing the Hessian (as opposed to
 *  using one of the Knitro options to approximate it) and evaluates H or HV
 *  depending on the value of "evalRequest.type".  For least squares models,
 *  the application defines the two callback functions for EVALR and EVALRJ
 *  (instead of EVALFC and EVALGA).  Least squares applications do not provide
 *  a callback for the Hessian as it is always approximated.
 *    It is possible in most cases to combine EVALFC and EVALGA into a single
 *  callback function.  This may be advantageous if the application evaluates
 *  functions and their derivatives at the same time.  In order to do this, set
 *  the user option eval_fcga=KN_EVAL_FCGA_YES, and define one callback set in
 *  "KN_add_eval_callback()" that evaluates BOTH the functions and gradients
 *  (i.e. have it populate "obj", "c", "objGrad", and "jac" in the "evalResult"
 *  structure), and do not set a callback in "KN_set_cb_grad()".  Whenever Knitro
 *  needs a function + gradient evaluation, it will callback to the function
 *  passed to "KN_add_eval_callback()" with an EVALFCGA request.
 *    Combining function and gradient evaluations in one callback is not currently
 *  allowed if hessopt=KN_HESSOPT_PRODUCT_FINDIFF. It is not possible to combine
 *  EVALH / EVALHV because "lambda" may change after the EVALFC call.  Generally
 *  it is most efficient to separate function and gradient callbacks, since a
 *  gradient evaluation is not needed at every "x" value where functions are
 *  evaluated.
 *
 *  The "userParams" argument is an arbitrary pointer passed from the Knitro
 *  KN_solve call to the callback.  It should be used to pass parameters
 *  defined and controlled by the application, or left null if not used.
 *  Knitro does not modify or dereference the "userParams" pointer.
 *
 *  For simplicity, the following user-defined evaluation callback functions
 *  all use the same "KN_eval_callback()" function prototype defined below.
 *
 *      funcCallback
 *      gradCallback
 *      hessCallback
 *      rsdCallback    (for least squares)
 *      rsdJacCallback (for least squares)
 *
 *  Other user callbacks that aren't involved in evaluations use the
 *  "KN_user_callback" function prototype or some other function protoype
 *  defined below that is specific to that callback. These include:
 *
 *      KN_set_newpt_callback
 *      KN_set_mip_node_callback
 *      KN_set_mip_usercuts_callback
 *      KN_set_mip_lazyconstraints_callback
 *      KN_set_ms_callback
 *      KN_set_ms_process_callback
 *      KN_set_ms_initpt_callback
 *      KN_set_puts_callback
 *      KN_set_linsolver_callback
 *
 *  Callbacks should return 0 if successful, a negative error code if not.
 *  Possible unsuccessful (negative) error codes for the func/grad/hess/rsd/rsdJac
 *  callback functions include:
 *
 *      KN_RC_CALLBACK_ERR       (for generic callback errors)
 *      KN_RC_EVAL_ERR           (for evaluation errors, e.g log(-1))
 *
 *  The "linsolver" callback may use the return code
 *
 *      KN_RC_LINEAR_SOLVER_ERR
 *
 *  if it encounters a linear system it is unable to solve or prefers Knitro
 *  to handle.  In this case, Knitro will revert to its own internal linear
 *  solver for that linear system.
 *
 *  In addition, for the "func", "newpoint", "ms_process", "mip_node",
 *  "mip_usercuts" and "mip_lazyconstraints" callbacks, the user may set the
 *  following return code to force Knitro
 *  to terminate based on some user-defined condition.
 *
 *      KN_RC_USER_TERMINATION   (to use a callback routine
 *                                for user specified termination)
 */

/** Structure used to pass back evaluation information for evaluation callbacks.
 *
 *    type:      - indicates the type of evaluation requested
 *    threadID:  - the thread ID associated with this evaluation request;
 *                 useful for multi-threaded, concurrent evaluations
 *    x:         - values of unknown (primal) variables used for all evaluations
 *    lambda:    - values of unknown dual variables/Lagrange multipliers
 *                 used for the evaluation of the Hessian
 *    sigma:     - scalar multiplier for the objective component of the Hessian
 *    vec:       - vector array value for Hessian-vector products (only used
 *                 when user option hessopt=KN_HESSOPT_PRODUCT)
 */
typedef struct KN_eval_request {
          int      type;
          int      threadID;
    const double * x;
    const double * lambda;
    const double * sigma;
    const double * vec;
} KN_eval_request, *KN_eval_request_ptr;

/** Structure used to return results information for evaluation callbacks.
 *  The arrays (and their indices and sizes) returned in this structure are
 *  local to the specific callback structure used for the evaluation.
 *
 *    obj:             - objective function evaluated at "x" for EVALFC or
 *                       EVALFCGA request (funcCallback)
 *    c:               - (length nC) constraint values evaluated at "x" for
 *                       EVALFC or EVALFCGA request (funcCallback)
 *    objGrad:         - (length nV) objective gradient evaluated at "x" for
 *                       EVALGA request (gradCallback) or EVALFCGA request (funcCallback)
 *    jac:             - (length nnzJ) constraint Jacobian evaluated at "x" for
 *                       EVALGA request (gradCallback) or EVALFCGA request (funcCallback)
 *    hess:            - (length nnzH) Hessian evaluated at "x", "lambda", "sigma"
 *                       for EVALH or EVALH_NO_F request (hessCallback)
 *    hessVec:         - (length n=number variables in the model) Hessian-vector
 *                       product evaluated at "x", "lambda", "sigma"
 *                       for EVALHV or EVALHV_NO_F request (hessCallback)
 *    rsd:             - (length nR) residual values evaluated at "x" for EVALR
 *                       request (rsdCallback)
 *    rsdJac:          - (length nnzJ) residual Jacobian evaluated at "x" for
 *                       EVALRJ request (rsdJacCallback)
 */
typedef struct KN_eval_result {
    double * obj;
    double * c;
    double * objGrad;
    double * jac;
    double * hess;
    double * hessVec;
    double * rsd;
    double * rsdJac;
} KN_eval_result, *KN_eval_result_ptr;

/** The callback structure/object.  Note the CB_context_ptr is allocated and
 *  managed by Knitro: the user does not have to free it.
 */
typedef struct CB_context CB_context, *CB_context_ptr;

/** Function prototype for evaluation callbacks. */
typedef int KN_eval_callback (KN_context_ptr             kc,
                              CB_context_ptr             cb,
                              KN_eval_request_ptr const  evalRequest,
                              KN_eval_result_ptr  const  evalResult,
                              void              * const  userParams);

/** This is the routine for adding a callback for (nonlinear) evaluations
 *  of objective and constraint functions.  This routine can be called
 *  multiple times to add more than one callback structure (e.g. to create
 *  different callback structures to handle different blocks of constraints).
 *  This routine specifies the minimal information needed for a callback, and
 *  creates the callback structure "cb", which can then be passed to other
 *  callback functions to set additional information for that callback.
 *
 *    evalObj       - boolean indicating whether or not any part of the objective
 *                    function is evaluated in the callback
 *    nC            - number of constraints evaluated in the callback
 *    indexCons     - (length nC) index of constraints evaluated in the callback
 *                    (set to NULL if nC=0)
 *    funcCallback  - a pointer to a function that evaluates the objective parts
 *                    (if evalObj=KNTRUE) and any constraint parts (specified by
 *                    nC and indexCons) involved in this callback; when
 *                    eval_fcga=KN_EVAL_FCGA_YES, this callback should also evaluate
 *                    the relevant first derivatives/gradients
 *    cb            - (output) the callback structure that gets created by
 *                    calling this function; all the memory for this structure is
 *                    handled by Knitro
 *
 *  After a callback is created by "KN_add_eval_callback()", the user can then specify
 *  gradient information and structure through "KN_set_cb_grad()" and Hessian
 *  information and structure through "KN_set_cb_hess()".  If not set, Knitro will
 *  approximate these.  However, it is highly recommended to provide a callback routine
 *  to specify the gradients if at all possible as this will greatly improve the
 *  performance of Knitro.  Even if a gradient callback is not provided, it is still
 *  helpful to provide the sparse Jacobian structure through "KN_set_cb_grad()" to
 *  improve the efficiency of the finite-difference gradient approximations.
 *  Other optional information can also be set via "KN_set_cb_*() functions as
 *  detailed below.
 *
 *  Returns 0 if OK, nonzero if error.
 */
int  KNITRO_API KN_add_eval_callback (      KN_context_ptr            kc,
                                      const KNBOOL                    evalObj,
                                      const KNINT                     nC,
                                      const KNINT            * const  indexCons,    /* nullable if nC=0 */
                                            KN_eval_callback * const  funcCallback,
                                            CB_context_ptr   * const  cb);

/** Version of KN_add_eval_callback to create a callback that applies to the
 *  objective function and all constraints.
 */
int  KNITRO_API KN_add_eval_callback_all (      KN_context_ptr            kc,
                                                KN_eval_callback * const  funcCallback,
                                                CB_context_ptr   * const  cb);

/** Version of KN_add_eval_callback to create a callback that only applies to a
 *  single objective function or constraint.  Set index to the corresponding
 *  constraint index or use -1 for the objective.
 */
int  KNITRO_API KN_add_eval_callback_one (      KN_context_ptr            kc,
                                          const KNINT                     index,    /* -1 for obj */
                                                KN_eval_callback * const  funcCallback,
                                                CB_context_ptr   * const  cb);

/** Add an evaluation callback for a least-squares models.  Similar to KN_add_eval_callback()
 *  above, but for least-squares models.
 *
 *    nR            - number of residuals evaluated in the callback
 *    indexRsds     - (length nR) index of residuals evaluated in the callback
 *    rsdCallback   - a pointer to a function that evaluates any residual parts
 *                    (specified by nR and indexRsds) involved in this callback
 *    cb            - (output) the callback structure that gets created by
 *                    calling this function; all the memory for this structure is
 *                    handled by Knitro
 *
 *  After a callback is created by "KN_add_lsq_eval_callback()", the user can then
 *  specify residual Jacobian information and structure through "KN_set_cb_rsd_jac()".
 *  If not set, Knitro will approximate the residual Jacobian.  However, it is highly
 *  recommended to provide a callback routine to specify the residual Jacobian if at all
 *  possible as this will greatly improve the performance of Knitro.  Even if a callback
 *  for the residual Jacobian is not provided, it is still helpful to provide the sparse
 *  Jacobian structure for the residuals through "KN_set_cb_rsd_jac()" to improve the
 *  efficiency of the finite-difference Jacobian approximation.  Other optional
 *  information can also be set via "KN_set_cb_*() functions as detailed below.
 *
 *  Returns 0 if OK, nonzero if error.
 */
int  KNITRO_API KN_add_lsq_eval_callback (      KN_context_ptr            kc,
                                          const KNINT                     nR,
                                          const KNINT            * const  indexRsds,
                                                KN_eval_callback * const  rsdCallback,
                                                CB_context_ptr   * const  cb);

/** Version of KN_add_lsq_eval_callback to create a callback that applies to
 *  all residual functions.
 */
int  KNITRO_API KN_add_lsq_eval_callback_all (      KN_context_ptr            kc,
                                                    KN_eval_callback * const  rsdCallback,
                                                    CB_context_ptr   * const  cb);

/** Version of KN_add_lsq_eval_callback to create a callback that only applies to
 *  a single residual function.  Set indexRsd to the corresponding residual index.
 */
int  KNITRO_API KN_add_lsq_eval_callback_one (      KN_context_ptr            kc,
                                              const KNINT                     indexRsd,
                                                    KN_eval_callback * const  rsdCallback,
                                                    CB_context_ptr   * const  cb);

/** This API function is used to set the objective gradient and constraint Jacobian
 *  structure and also (optionally) a callback function to evaluate the objective
 *  gradient and constraint Jacobian provided through this callback.
 *
 *    cb               - a callback structure created from a previous call to
 *                       KN_add_eval_callback()
 *    nV               - number of nonzero components in the objective gradient
 *                       for this callback if providing in sparse form; set to
 *                       KN_DENSE to provide the full objective gradient
 *    objGradIndexVars - (length nV) the nonzero indices of the objective gradient;
 *                       set to NULL if nV=KN_DENSE or nV=0 (i.e. evalObj=KNFALSE)
 *    nnzJ             - number of nonzeroes in the sparse constraint Jacobian
 *                       computed through this callback; set to KN_DENSE_ROWMAJOR to
 *                       provide the full Jacobian in row major order (i.e. ordered
 *                       by rows/constraints), or KN_DENSE_COLMAJOR to provide the full
 *                       Jacobian in column major order (i.e. ordered by columns/
 *                       variables)
 *    jacIndexCons     - (length nnzJ) constraint index (row) of each nonzero;
 *                       set to NULL if nnzJ=KN_DENSE_ROWMAJOR/KN_DENSE_COLMAJOR or nnzJ=0
 *    jacIndexVars     - (length nnzJ) variable index (column) of each nonzero;
 *                       set to NULL if nnzJ=KN_DENSE_ROWMAJOR/KN_DENSE_COLMAJOR or nnzJ=0
 *    gradCallback     - a pointer to a function that evaluates the objective gradient
 *                       parts and any constraint Jacobian parts involved in this
 *                       callback; set to NULL if using finite-difference gradient
 *                       approximations (specified via KN_set_cb_gradopt()), or if
 *                       gradients and functions are provided together in the
 *                       funcCallback (i.e. eval_fcga=KN_EVAL_FCGA_YES).
 *
 *  The user should generally always try to define the sparsity structure
 *  for the Jacobian ("nnzJ", "jacIndexCons", "jacIndexVars").  Even when
 *  using finite-difference approximations to compute the gradients, knowing the
 *  sparse structure of the Jacobian can allow Knitro to compute these
 *  finite-difference approximations faster.  However, if the user is unable to
 *  provide this sparsity structure, then one can set "nnzJ" to KN_DENSE_ROWMAJOR or
 *  KN_DENSE_COLMAJOR and set "jacIndexCons" and "jacIndexVars" to NULL.
 */
int  KNITRO_API KN_set_cb_grad (      KN_context_ptr            kc,
                                      CB_context_ptr            cb,
                                const KNINT                     nV,   /* or KN_DENSE */
                                const KNINT            * const  objGradIndexVars,
                                const KNLONG                    nnzJ, /* or KN_DENSE_* */
                                const KNINT            * const  jacIndexCons,
                                const KNINT            * const  jacIndexVars,
                                      KN_eval_callback * const  gradCallback); /* nullable */

/** This API function is used to set the structure and a callback function to
 *  evaluate the components of the Hessian of the Lagrangian provided through this
 *  callback.  KN_set_cb_hess() should only be used when defining a user-supplied
 *  Hessian callback function (via the "hessopt=KN_HESSOPT_EXACT" user option), or
 *  a callback function to compute the Hessian-vector product array (via the
 *  "hessopt=KN_HESSOPT_PRODUCT" user option).  When providing a callback function
 *  for Hessian-vector products, the Hessian is not stored or used internally, so
 *  in this case set "nnzH"=0, "hessIndexVars1"=NULL, and "hessIndexVars2"=NULL.
 *  When Knitro is approximating the Hessian, it cannot make use of the Hessian
 *  sparsity structure.
 *
 *    cb               - a callback structure created from a previous call to
 *                       KN_add_eval_callback()
 *    nnzH             - number of nonzeroes in the sparse Hessian of the Lagrangian
 *                       computed through this callback; set to KN_DENSE_ROWMAJOR to
 *                       provide the full upper triangular Hessian in row major order,
 *                       or KN_DENSE_COLMAJOR to provide the full upper triangular Hessian
 *                       in column major order.  Note that the Hessian is symmetric, so
 *                       the lower triangular components are the same as the upper
 *                       triangular components with row and column indices swapped.
 *    hessIndexVars1   - (length nnzH) first variable index of each nonzero;
 *                       set to NULL if nnzH=KN_DENSE_ROWMAJOR/KN_DENSE_COLMAJOR
 *    hessIndexVars2   - (length nnzH) second variable index of each nonzero;
 *                       set to NULL if nnzH=KN_DENSE_ROWMAJOR/KN_DENSE_COLMAJOR
 *    hessCallback     - a pointer to a function that evaluates the components of the
 *                       Hessian of the Lagrangian (or Hessian-vector product array)
 *                       provided in this callback
 */
int  KNITRO_API KN_set_cb_hess (      KN_context_ptr            kc,
                                      CB_context_ptr            cb,
                                const KNLONG                    nnzH, /* or KN_DENSE_* */
                                const KNINT            * const  hessIndexVars1,
                                const KNINT            * const  hessIndexVars2,
                                      KN_eval_callback * const  hessCallback);


/** This API function is used to set the residual Jacobian structure and also
 *  (optionally) a callback function to evaluate the residual Jacobian provided
 *  through this callback.
 *
 *    cb               - a callback structure created from a previous call to
 *                       KN_add_lsq_eval_callback()
 *    nnzJ             - number of nonzeroes in the sparse residual Jacobian
 *                       computed through this callback; set to KN_DENSE_ROWMAJOR to
 *                       provide the full Jacobian in row major order (i.e. ordered
 *                       by rows/residuals), or KN_DENSE_COLMAJOR to provide the full
 *                       Jacobian in column major order (i.e. ordered by columns/
 *                       variables)
 *    jacIndexRsds     - (length nnzJ) residual index (row) of each nonzero;
 *                       set to NULL if nnzJ=KN_DENSE_ROWMAJOR/KN_DENSE_COLMAJOR or nnzJ=0
 *    jacIndexVars     - (length nnzJ) variable index (column) of each nonzero;
 *                       set to NULL if nnzJ=KN_DENSE_ROWMAJOR/KN_DENSE_COLMAJOR or nnzJ=0
 *    rsdJacCallback   - a pointer to a function that evaluates any residual Jacobian
 *                       parts involved in this callback; set to NULL if using a finite-
 *                       difference Jacobian approximation (specified via KN_set_cb_gradopt())
 *
 *  The user should generally always try to define the sparsity structure
 *  for the Jacobian ("nnzJ", "jacIndexRsds", "jacIndexVars").  Even when
 *  using a finite-difference approximation to compute the Jacobian, knowing the
 *  sparse structure of the Jacobian can allow Knitro to compute this
 *  finite-difference approximation faster.  However, if the user is unable to
 *  provide this sparsity structure, then one can set "nnzJ" to KN_DENSE_ROWMAJOR or
 *  KN_DENSE_COLMAJOR and set "jacIndexRsds" and "jacIndexVars" to NULL.
 */
int  KNITRO_API KN_set_cb_rsd_jac (      KN_context_ptr            kc,
                                         CB_context_ptr            cb,
                                   const KNLONG                    nnzJ, /* or KN_DENSE_* */
                                   const KNINT            * const  jacIndexRsds,
                                   const KNINT            * const  jacIndexVars,
                                         KN_eval_callback * const  rsdJacCallback); /* nullable */


/** Define a userParams structure for an evaluation callback. */
int  KNITRO_API KN_set_cb_user_params (KN_context_ptr  kc,
                                       CB_context_ptr  cb,
                                       void   * const  userParams);

/** Specify which gradient option "gradopt" will be used to evaluate
 *  the first derivatives of the callback functions.  If gradopt=KN_GRADOPT_EXACT
 *  then a gradient evaluation callback must be set by "KN_set_cb_grad()"
 *  (or "KN_set_cb_rsd_jac()" for least squares).
 */
int  KNITRO_API KN_set_cb_gradopt (      KN_context_ptr  kc,
                                         CB_context_ptr  cb,
                                   const int             gradopt);

/** Set an array of relative stepsizes to use for the finite-difference
 *  gradient/Jacobian computations when using finite-difference
 *  first derivatives.  The user option KN_PARAM_FINDIFF_RELSTEPSIZE
 *  can be used to set the relative stepsizes for ALL variables.  This
 *  routine takes precedence over the setting of KN_PARAM_FINDIFF_RELSTEPSIZE
 *  and is used to customize the settings for individual variables.
 *  Finite-difference step sizes "delta" in Knitro are computed as:
 *       delta[i] = relStepSizes[i]*max(abs(x[i]),1);
 *  The default relative step sizes for each component of "x" are sqrt(eps)
 *  for forward finite differences, and eps^(1/3) for central finite
 *  differences.  Use this function to overwrite the default values.
 *  Any zero values will use Knitro default values, while non-zero values
 *  will overwrite default values. Knitro makes a local copy of all inputs,
 *  so the application may free memory after the call.
 *  Returns 0 if OK, nonzero if error.
 */
int  KNITRO_API KN_set_cb_relstepsizes (      KN_context_ptr  kc,
                                              CB_context_ptr  cb,
                                        const KNINT           nV,
                                        const KNINT  * const  indexVars,
                                        const double * const  xRelStepSizes);
int  KNITRO_API KN_set_cb_relstepsizes_all (      KN_context_ptr  kc,
                                                  CB_context_ptr  cb,
                                            const double * const  xRelStepSizes);
int  KNITRO_API KN_set_cb_relstepsize (      KN_context_ptr  kc,
                                             CB_context_ptr  cb,
                                       const KNINT           indexVar,
                                       const double          xRelStepSize);

/** These API functions can be used to retrieve some information specific to evaluation
 *  callbacks. */

/** Retrieve the number of constraints "nC" being evaluated through callback "cb".
 *  Returns 0 if OK, nonzero if error.
 */
int  KNITRO_API KN_get_cb_number_cons (const KN_context_ptr  kc,
                                       const CB_context_ptr  cb,
                                             KNINT * const   nC);

/** Retrieve the number of residuals "nR" being evaluated through callback "cb".
 *  Returns 0 if OK, nonzero if error.
 */
int  KNITRO_API KN_get_cb_number_rsds (const KN_context_ptr  kc,
                                       const CB_context_ptr  cb,
                                             KNINT * const   nR);

/** Retrieve the number of non-zero objective gradient elements "nnz"
 *  evaluated through callback "cb".
 *  Returns 0 if OK, nonzero if error.
 */
int  KNITRO_API KN_get_cb_objgrad_nnz (const KN_context_ptr  kc,
                                       const CB_context_ptr  cb,
                                             KNINT * const   nnz);

/** Retrieve the number of non-zero Jacobian elements "nnz"
 *  evaluated through callback "cb".
 *  Returns 0 if OK, nonzero if error.
 */
int  KNITRO_API KN_get_cb_jacobian_nnz (const KN_context_ptr  kc,
                                        const CB_context_ptr  cb,
                                              KNLONG * const  nnz);

/** Retrieve the number of non-zero residual Jacobian elements "nnz"
 *  evaluated through callback "cb".
 *  Returns 0 if OK, nonzero if error.
 */
int  KNITRO_API KN_get_cb_rsd_jacobian_nnz (const KN_context_ptr  kc,
                                            const CB_context_ptr  cb,
                                                  KNLONG * const  nnz);

/** Retrieve the number of non-zero Hessian elements "nnz" being
 *  evaluated through callback "cb".
 *  Returns 0 if OK, nonzero if error.
 */
int  KNITRO_API KN_get_cb_hessian_nnz (const KN_context_ptr  kc,
                                       const CB_context_ptr  cb,
                                             KNLONG * const  nnz);

/** Delete all evaluation callbacks previously added with KN_add_eval_callback.
 *
 * Note: all previously used CB_context_ptr will automatically be freed and must
 * not be used after this call.
 */
int  KNITRO_API KN_del_eval_callbacks(const KN_context_ptr kc);

/** Delete the objective terms of an evaluation callback.
 *
 * The callback functions are not modified and will still be called, but the
 * objective components will not be used in the evaluations.
 *
 * Note: For the Hessian evaluations, the associated callback must multiply the
 * objective components of the hessian by the evalRequest.sigma value
 * given in the KN_eval_request struct or use the evalRequest.type (
 * KN_RC_EVALH_NO_F or KN_RC_EVALHV_NO_F) to omit the objective components of
 * the hessian.
 *
 * KN_del_obj_eval_callback will remove the objective term of one particular
 * callback while KN_del_obj_eval_callback_all will remove objective terms for
 * all callbacks.
 */
int  KNITRO_API KN_del_obj_eval_callback(const KN_context_ptr kc,
                                         const CB_context_ptr cb);
int  KNITRO_API KN_del_obj_eval_callback_all(const KN_context_ptr kc);

/** Type declaration for several non-evaluation user callbacks defined
 *  below.
 */
typedef int  KN_user_callback (      KN_context_ptr  kc,
                               const double * const  x,
                               const double * const  lambda,
                                     void   * const  userParams);

/** Set the callback function that is invoked after Knitro computes a
 *  new estimate of the solution point (i.e., after every iteration).
 *  The function should not modify any Knitro arguments.
 *  Argument "kc" passed to the callback from inside Knitro is the
 *  context pointer for the current problem being solved inside Knitro
 *  (either the main single-solve problem, or a subproblem when using
 *  multi-start, Tuner, etc.).
 *  Arguments "x" and "lambda" contain the latest solution estimates.
 *  Other values (such as objective, constraint, jacobian, etc.) can be
 *  queried using the corresponding KN_get_XXX_values methods.
 *  Note: Currently only active for continuous models.
 *  Return 0 if successful, a negative error code if not.
 */
int  KNITRO_API KN_set_newpt_callback (KN_context_ptr            kc,
                                       KN_user_callback * const  fnPtr,
                                       void             * const  userParams);

/** This callback function is for mixed integer (MIP) problems only.
 *  Set the callback function that is invoked after Knitro finishes
 *  processing a node on the branch-and-bound tree (i.e., after a relaxed
 *  subproblem solve in the branch-and-bound procedure).
 *  Argument "kc" passed to the callback from inside Knitro is the
 *  context pointer for the last node subproblem solved inside Knitro.
 *  The function should not modify any Knitro arguments.
 *  Arguments "x" and "lambda" contain the solution from the node solve.
 *  Return 0 if successful, a negative error code if not.
 */
int  KNITRO_API KN_set_mip_node_callback (KN_context_ptr            kc,
                                          KN_user_callback * const  fnPtr,
                                          void             * const  userParams);

/** This callback function is for mixed integer (MIP) problems only.
 *  Set the callback function that returns user cuts.
 *  This callback is invoked when Knitro search for cutting planes.
 *  It needs to add new cutting planes to argument "kc".
 *  Only linear cutting planes are supported.
 *  Arguments "x" and "lambda" contain the solution from the node solve.
 */
int  KNITRO_API KN_set_mip_usercuts_callback (KN_context_ptr            kc,
                                              KN_user_callback * const  fnPtr,
                                              void             * const  userParams);

/** This callback function is for mixed integer (MIP) problems only.
 *  Set the callback function that returns lazy constraints.
 *  This callback is invoked after all subproblem solves.
 *  It needs to add new (lazy) constraints to argument "kc".
 *  Only linear cutting planes are supported.
 *  Arguments "x" and "lambda" contain the solution from the node solve.
 */
int  KNITRO_API KN_set_mip_lazyconstraints_callback (KN_context_ptr            kc,
                                                     KN_user_callback * const  fnPtr,
                                                     void             * const  userParams);

/** This callback function is for multistart (MS) problems only.
 *  Set the callback function that is invoked regularly during the search.
 *  With parallelization, this callback is never called multiple times
 *  simultaneously. Argument "kc" passed to the callback from inside
 *  Knitro is the original context. The function should not modify any Knitro
 *  arguments. Arguments "x" and "lambda" contain the solution from the
 *  current best solve.
 *  Return 0 if successful, a negative error code if not.
 */
int  KNITRO_API KN_set_ms_callback (KN_context_ptr            kc,
                                    KN_user_callback * const  fnPtr,
                                    void             * const  userParams);

/** This callback function is for multistart (MS) problems only.
 *  Set the callback function that is invoked after Knitro finishes
 *  processing a multistart solve. Argument "kc" passed to the callback
 *  from inside Knitro is the context pointer for the last multistart
 *  subproblem solved inside Knitro.  The function should not modify any
 *  Knitro arguments.  Arguments "x" and "lambda" contain the solution from
 *  the last solve.
 *  Return 0 if successful, a negative error code if not.
 */
int  KNITRO_API KN_set_ms_process_callback (KN_context_ptr            kc,
                                            KN_user_callback * const  fnPtr,
                                            void             * const  userParams);

/** Type declaration for the callback that allows applications to
 *  specify an initial point before each local solve in the multistart
 *  procedure.  On input, arguments "x" and "lambda" are the randomly
 *  generated initial points determined by Knitro, which can be overwritten
 *  by the user.  The argument "nSolveNumber" is the number of the
 *  multistart solve.  Return 0 if successful, a negative error code if not.
 *  Use KN_set_ms_initpt_callback to set this callback function.
 */
typedef int  KN_ms_initpt_callback (      KN_context_ptr  kc,
                                    const KNINT           nSolveNumber,
                                          double * const  x,
                                          double * const  lambda,
                                          void   * const  userParams);

/** Return 0 if successful, a negative error code if not.
 */
int  KNITRO_API KN_set_ms_initpt_callback (KN_context_ptr                 kc,
                                           KN_ms_initpt_callback * const  fnPtr,
                                           void                  * const  userParams);

/** Type declaration for the callback that allows applications to handle
 *  output. Applications can set a "put string" callback function to handle
 *  output generated by the Knitro solver.  By default Knitro prints to
 *  stdout or a file named "knitro.log", as determined by KN_PARAM_OUTMODE.
 *  The KN_puts function takes a "userParams" argument which is a pointer
 *  passed directly from KN_solve. The function should return the number of
 *  characters that were printed.
 *  Use KN_set_puts_callback to set this callback function.
 */
typedef int  KN_puts (const char * const  str,
                            void * const  userParams);

/**  Return 0 if successful, a negative error code if not.
 */
int  KNITRO_API KN_set_puts_callback (KN_context_ptr   kc,
                                      KN_puts * const  fnPtr,
                                      void    * const  userParams);

/* ----- Callback for linear system solves ----- */

/** Applications may define a function for solving sparse linear systems
 *  of equations M*x=b, which need to be solved internally by Knitro.  The
 *  solution of these linear systems may be a significant cost for some
 *  algorithms (e.g. interior-point algorithms), especially for large
 *  problems.  The symmetric n-by-n coefficient matrix "M" for the linear system
 *  uses compressed sparse column (CSC) format and only the lower triangle
 *  plus diagonal is stored.  Row indices are not necessarily ordered within
 *  each column.  The matrix M may often have the following generic 2-by-2 block
 *  structure:
 *
 *    M = | H  A' |
 *        | A  D  |
 *
 *  where H is a n11-by-n11 symmetric matrix, A is a (n-n11)-by-n11 matrix,
 *  A' is the transpose of A, and D is a diagonal matrix of dimension (n-n11).
 *  The Knitro callback for linear system solves provides the sparse structure
 *  for the full matrix "M", and also provides the (1,1) block dimension n11,
 *  which can be used to access sub-blocks.
 */

/** Possible linsolver phases.
 */
#define KN_LINSOLVER_PHASE_INIT    0
#define KN_LINSOLVER_PHASE_ANALYZE 1
#define KN_LINSOLVER_PHASE_FACTOR  2
#define KN_LINSOLVER_PHASE_SOLVE   3
#define KN_LINSOLVER_PHASE_FREE    4

/** Structure used to pass back information for linear system solver callbacks.
 *
 *    phase:     - indicates the linear system solve phase
 *                 (e.g., init, analyze, factor, solve, free)
 *    linsysID:  - the linear system ID associated with the request
 *                 in the current local optimization
 *    threadID:  - the thread ID associated with this request;
 *                 threadID (along with linsysID) can be used to
 *                 identify a unique linear system, which can be
 *                 useful for multi-threaded, concurrent solves
 *    n:         - dimension of M
 *    n11:       - dimension of (1,1) block H inside M; useful for
 *                 extracting submatrices H, A, and D for 2-by-2
 *                 block structure (n11=n if no block structure)
 *    rhs:       - right-hand-side values needed for "solve" phase only
 *    values:    - coefficient matrix values
 *    indexRows: - coefficient matrix row indices
 *    ptrCols:   - coefficient matrix column pointers;
 *                 number of nonzero elements (nnz) = ptrCols[n]
 */
typedef struct KN_linsolver_request {
          int      phase;
          int      linsysID;
          int      threadID;
          KNINT    n;         /* dimension of coef matrix */
          KNINT    n11;       /* dimension of (1,1) block */
    const double * rhs;       /* right-hand side vector (size = n) */
    const double * values;    /* coefficient matrix values (size = nnz) */
    const KNINT  * indexRows; /* coefficient matrix row indices (size = nnz) */
    const KNLONG * ptrCols;   /* coefficient matrix column pointers (size = n+1) */
} KN_linsolver_request, *KN_linsolver_request_ptr;

/** Structure used to return results for linear system solver callbacks.
 *
 *    solution: - solution values (required for "solve" phase)
 *    negeig:   - number of negative eigenvalues (required for "factor" phase)
 *    poseig:   - number of positive eigenvalues (optional for "factor" phase)
 *    rank:     - coefficient matrix rank (optional for "factor" phase)
 */
typedef struct KN_linsolver_result {
          double * solution;
          KNINT    negeig;
          KNINT    poseig;
          KNINT    rank;
} KN_linsolver_result, *KN_linsolver_result_ptr;

/** Function prototype for linear system solve callbacks.
 *  Information/values needed for the linear system solve are provided in
 *  "linsolverRequest".  Solution values should be returned in "linsolverResult".
 *
 *  Return 0 if successful, a negative error code if not.
 *      KN_RC_CALLBACK_ERR       (use for fatal callback errors)
 *      KN_RC_LINEAR_SOLVER_ERR  (non-fatal for analyze phase;
 *                                causes Knitro to revert to one
 *                                of its own internal linear solvers)
 */
typedef int KN_linsolver_callback (KN_context_ptr                  kc,
                                   KN_linsolver_request_ptr const  linsolverRequest,
                                   KN_linsolver_result_ptr  const  linsolverResult,
                                   void                   * const  userParams);

/** This callback function is for solving linear systems inside Knitro.
 *  Return 0 if successful, a negative error code if not.
 */
int  KNITRO_API KN_set_linsolver_callback (KN_context_ptr                 kc,
                                           KN_linsolver_callback * const  fnPtr,
                                           void                  * const  userParams);


/* ----- Loading full models ----- */

/** This function loads all the basic data for a linear program (LP)
 *  in a single function call.  It can only be called on a newly created
 *  Knitro problem object. After calling this function, the model may
 *  be augmented through additional Knitro API calls before solving.
 *  Returns 0 if OK, or a negative value on error.
 */
int  KNITRO_API KN_load_lp (      KN_context_ptr  kc,
                            const KNINT           n,
                            const double * const  lobjCoefs,      /* size = n */
                            const double * const  xLoBnds,        /* size = n */
                            const double * const  xUpBnds,        /* size = n */
                            const KNINT           m,
                            const double * const  cLoBnds,        /* size = m */
                            const double * const  cUpBnds,        /* size = m */
                            const KNLONG          nnzJ,
                            const KNINT  * const  ljacIndexCons,  /* size = nnzJ */
                            const KNINT  * const  ljacIndexVars,  /* size = nnzJ */
                            const double * const  ljacCoefs);     /* size = nnzJ */

/** This function loads all the data for a quadratic program (QP)
 *  in a single function call.  It can only be called on a newly created
 *  Knitro problem object. After calling this function, the model may
 *  be augmented through additional Knitro API calls before solving.
 *  Returns 0 if OK, or a negative value on error.
 */
int  KNITRO_API KN_load_qp (      KN_context_ptr  kc,
                            const KNINT           n,
                            const double * const  lobjCoefs,      /* size = n */
                            const double * const  xLoBnds,        /* size = n */
                            const double * const  xUpBnds,        /* size = n */
                            const KNINT           m,
                            const double * const  cLoBnds,        /* size = m */
                            const double * const  cUpBnds,        /* size = m */
                            const KNLONG          nnzJ,
                            const KNINT  * const  ljacIndexCons,  /* size = nnzJ */
                            const KNINT  * const  ljacIndexVars,  /* size = nnzJ */
                            const double * const  ljacCoefs,      /* size = nnzJ */
                            const KNLONG          nnzH,
                            const KNINT  * const  qobjIndexVars1, /* size = nnzH */
                            const KNINT  * const  qobjIndexVars2, /* size = nnzH */
                            const double * const  qobjCoefs);     /* size = nnzH */

/** This function loads all the data for a quadratically constrained
 *  quadratic program (QCQP) in a single function call. It can only
 *  be called on a newly created Knitro problem object. After calling
 *  this function, the model may be augmented through additional
 *  Knitro API calls before solving.
 *  Returns 0 if OK, or a negative value on error.
 */
int  KNITRO_API KN_load_qcqp (      KN_context_ptr  kc,
                              const KNINT           n,
                              const double * const  lobjCoefs,      /* size = n */
                              const double * const  xLoBnds,        /* size = n */
                              const double * const  xUpBnds,        /* size = n */
                              const KNINT           m,
                              const double * const  cLoBnds,        /* size = m */
                              const double * const  cUpBnds,        /* size = m */
                              const KNLONG          nnzJ,
                              const KNINT  * const  ljacIndexCons,  /* size = nnzJ */
                              const KNINT  * const  ljacIndexVars,  /* size = nnzJ */
                              const double * const  ljacCoefs,      /* size = nnzJ */
                              const KNLONG          nnzH,
                              const KNINT  * const  qobjIndexVars1, /* size = nnzH */
                              const KNINT  * const  qobjIndexVars2, /* size = nnzH */
                              const double * const  qobjCoefs,      /* size = nnzH */
                              const KNLONG          nnzQ,
                              const KNINT  * const  qconIndexCons,  /* size = nnzQ */
                              const KNINT  * const  qconIndexVars1, /* size = nnzQ */
                              const KNINT  * const  qconIndexVars2, /* size = nnzQ */
                              const double * const  qconCoefs);     /* size = nnzQ */

/* ----- Algorithmic/modeling features ----- */

/** Set custom absolute feasibility tolerances to use for the
 *  termination tests.
 *  The user options KN_PARAM_FEASTOL/KN_PARAM_FEASTOLABS define
 *  a single tolerance that is applied equally to every constraint
 *  and variable.  This API function allows the user to specify
 *  separate feasibility termination tolerances for each constraint
 *  and variable.  Values specified through this function will override
 *  the value determined by KN_PARAM_FEASTOL/KN_PARAM_FEASTOLABS. The
 *  tolerances should be positive values.  If a non-positive value is
 *  specified, that constraint or variable will use the standard tolerances
 *  based on  KN_PARAM_FEASTOL/KN_PARAM_FEASTOLABS.
 *  The variables are considered to be satisfied when
 *      x[i] - xUpBnds[i] <= xFeasTols[i]  for all i=1..n, and
 *      xLoBnds[i] - x[i] <= xFeasTols[i]  for all i=1..n
 *  The regular constraints are considered to be satisfied when
 *      c[i] - cUpBnds[i] <= cFeasTols[i]  for all i=1..m, and
 *      cLoBnds[i] - c[i] <= cFeasTols[i]  for all i=1..m
 *  The complementarity constraints are considered to be satisfied when
 *      min(x1_i, x2_i) <= ccFeasTols[i]  for all i=1..ncc,
 *  where x1 and x2 are the arrays of complementary pairs.
 *  Knitro makes a local copy of all inputs, so the application
 *  may free memory after the call.
 *  Returns 0 if OK, nonzero if error.
 */
int  KNITRO_API KN_set_var_feastols (      KN_context_ptr  kc,
                                     const KNINT           nV,
                                     const KNINT  * const  indexVars,
                                     const double * const  xFeasTols);
int  KNITRO_API KN_set_var_feastols_all (      KN_context_ptr  kc,
                                         const double * const  xFeasTols);
int  KNITRO_API KN_set_var_feastol (      KN_context_ptr  kc,
                                    const KNINT           indexVar,
                                    const double          xFeasTol);

int  KNITRO_API KN_set_con_feastols (      KN_context_ptr  kc,
                                     const KNINT           nC,
                                     const KNINT  * const  indexCons,
                                     const double * const  cFeasTols);
int  KNITRO_API KN_set_con_feastols_all (      KN_context_ptr  kc,
                                         const double * const  cFeasTols);
int  KNITRO_API KN_set_con_feastol (      KN_context_ptr  kc,
                                    const KNINT           indexCon,
                                    const double          cFeasTol);

int  KNITRO_API KN_set_compcon_feastols (      KN_context_ptr  kc,
                                         const KNINT           nCC,
                                         const KNINT  * const  indexCompCons,
                                         const double * const  ccFeasTols);
int  KNITRO_API KN_set_compcon_feastols_all (      KN_context_ptr  kc,
                                             const double * const  ccFeasTols);
int  KNITRO_API KN_set_compcon_feastol (      KN_context_ptr  kc,
                                        const KNINT           indexCompCon,
                                        const double          ccFeasTol);

/** Set an array of variable scaling and centering values to
 *  perform a linear scaling
 *    x[i] = xScaleFactors[i] * xScaled[i] + xScaleCenters[i]
 *  for each variable. These scaling factors should try to
 *  represent the "typical" values of the "x" variables so that the
 *  scaled variables ("xScaled") used internally by Knitro are close
 *  to one.  The values for xScaleFactors should be positive.
 *  If a non-positive value is specified, that variable will not
 *  be scaled.
 *  Returns 0 if OK, nonzero if error.
 */
int  KNITRO_API KN_set_var_scalings (      KN_context_ptr  kc,
                                     const KNINT           nV,
                                     const KNINT  * const  indexVars,
                                     const double * const  xScaleFactors,
                                     const double * const  xScaleCenters);
int  KNITRO_API KN_set_var_scalings_all (      KN_context_ptr  kc,
                                         const double * const  xScaleFactors,
                                         const double * const  xScaleCenters);
int  KNITRO_API KN_set_var_scaling (      KN_context_ptr  kc,
                                    const KNINT           indexVar,
                                    const double          xScaleFactor,
                                    const double          xScaleCenter);

/** Set an array of constraint scaling values to perform a scaling
 *    cScaled[i] = cScaleFactors[i] * c[i]
 *  for each constraint. These scaling factors should try to
 *  represent the "typical" values of the inverse of the constraint
 *  values "c" so that the scaled constraints ("cScaled") used
 *  internally by Knitro are close to one.  Scaling factors for
 *  standard constraints can be provided with "cScaleFactors", while
 *  scalings for complementarity constraints can be specified with
 *  "ccScaleFactors".  The values for cScaleFactors/ccScaleFactors
 *  should be positive.  If a non-positive value is specified, that
 *  constraint will use either the standard Knitro scaling
 *  (KN_SCALE_USER_INTERNAL), or no scaling (KN_SCALE_USER_NONE).
 *  Returns 0 if OK, nonzero if error.
 */
int  KNITRO_API KN_set_con_scalings (      KN_context_ptr  kc,
                                     const KNINT           nC,
                                     const KNINT  * const  indexCons,
                                     const double * const  cScaleFactors);
int  KNITRO_API KN_set_con_scalings_all (      KN_context_ptr  kc,
                                         const double * const  cScaleFactors);
int  KNITRO_API KN_set_con_scaling (      KN_context_ptr  kc,
                                    const KNINT           indexCon,
                                    const double          cScaleFactor);

int  KNITRO_API KN_set_compcon_scalings (      KN_context_ptr  kc,
                                         const KNINT           nCC,
                                         const KNINT  * const  indexCompCons,
                                         const double * const  ccScaleFactors);
int  KNITRO_API KN_set_compcon_scalings_all (      KN_context_ptr  kc,
                                             const double * const  ccScaleFactors);
int  KNITRO_API KN_set_compcon_scaling (      KN_context_ptr  kc,
                                        const KNINT           indexCompCons,
                                        const double          ccScaleFactor);

/** Set a scaling value for the objective function
 *    objScaled = objScaleFactor * obj
 *  This scaling factor should try to represent the "typical"
 *  value of the inverse of the objective function value "obj" so
 *  that the scaled objective ("objScaled") used internally by
 *  Knitro is close to one. The value for objScaleFactor
 *  should be positive.  If a non-positive value is specified, then
 *  the objective will use either the standard Knitro scaling
 *  (KN_SCALE_USER_INTERNAL), or no scaling (KN_SCALE_USER_NONE).
 *  Returns 0 if OK, nonzero if error.
 */
int  KNITRO_API KN_set_obj_scaling (      KN_context_ptr  kc,
                                    const double          objScaleFactor);

/** Set names for model components passed in by the user/modeling
 *  language so that Knitro can internally print out these names.
 *  Knitro makes a local copy of all inputs, so the application may
 *  free memory after the call.
 *  Returns 0 if OK, nonzero if error.
 */
int  KNITRO_API KN_set_var_names (      KN_context_ptr  kc,
                                  const KNINT           nV,
                                  const KNINT * const   indexVars,
                                  const char  * const   xNames[]);
int  KNITRO_API KN_set_var_names_all (      KN_context_ptr  kc,
                                      const char * const    xNames[]);
int  KNITRO_API KN_set_var_name (      KN_context_ptr  kc,
                                 const KNINT           indexVars,
                                 const char  * const   xName);

int  KNITRO_API KN_set_con_names (      KN_context_ptr  kc,
                                  const KNINT           nC,
                                  const KNINT * const   indexCons,
                                  const char  * const   cNames[]);
int  KNITRO_API KN_set_con_names_all (      KN_context_ptr  kc,
                                      const char  * const   cNames[]);
int  KNITRO_API KN_set_con_name (      KN_context_ptr  kc,
                                 const KNINT           indexCon,
                                 const char  * const   cName);

int  KNITRO_API KN_set_compcon_names (      KN_context_ptr  kc,
                                      const KNINT           nCC,
                                      const KNINT * const   indexCompCons,
                                      const char  * const   ccNames[]);
int  KNITRO_API KN_set_compcon_names_all (      KN_context_ptr  kc,
                                          const char  * const   ccNames[]);
int  KNITRO_API KN_set_compcon_name (      KN_context_ptr  kc,
                                     const int             indexCompCon,
                                     const char  * const   ccName);

int  KNITRO_API KN_set_obj_name (      KN_context_ptr  kc,
                                 const char  * const   objName);

/** Get names for model components.
 *  Returns 0 if OK, nonzero if error.
 */
int  KNITRO_API KN_get_var_names (const KN_context_ptr  kc,
                                  const KNINT           nV,
                                  const KNINT * const   indexVars,
                                  const KNINT           nBufferSize,
                                        char  * const   xNames[]);
int  KNITRO_API KN_get_var_names_all (const KN_context_ptr  kc,
                                      const KNINT           nBufferSize,
                                            char * const    xNames[]);
int  KNITRO_API KN_get_var_name (const KN_context_ptr  kc,
                                 const KNINT           indexVars,
                                 const KNINT           nBufferSize,
                                       char  * const   xName);

int  KNITRO_API KN_get_con_names (const KN_context_ptr  kc,
                                  const KNINT           nC,
                                  const KNINT * const   indexCons,
                                  const KNINT           nBufferSize,
                                        char  * const   cNames[]);
int  KNITRO_API KN_get_con_names_all (const KN_context_ptr  kc,
                                      const KNINT           nBufferSize,
                                            char * const    cNames[]);
int  KNITRO_API KN_get_con_name (const KN_context_ptr  kc,
                                 const KNINT           indexCons,
                                 const KNINT           nBufferSize,
                                       char  * const   cName);
int  KNITRO_API KN_get_obj_name (const KN_context_ptr  kc,
                                 const KNINT           nBufferSize,
                                       char  * const   objName);

/** This API function can be used to identify which variables
 *  should satisfy their variable bounds throughout the optimization
 *  process (KN_HONORBNDS_ALWAYS).  The user option KN_PARAM_HONORBNDS
 *  can be used to set ALL variables to honor their bounds.  This
 *  routine takes precedence over the setting of KN_PARAM_HONORBNDS
 *  and is used to customize the settings for individual variables.
 *  Knitro makes a local copy of all inputs, so the application may
 *  free memory after the call.
 *  Returns 0 if OK, nonzero if error.
 */
int  KNITRO_API KN_set_var_honorbnds (      KN_context_ptr  kc,
                                      const KNINT           nV,
                                      const KNINT  * const  indexVars,
                                      const int    * const  xHonorBnds);
int  KNITRO_API KN_set_var_honorbnds_all (      KN_context_ptr  kc,
                                          const int    * const  xHonorBnds);
int  KNITRO_API KN_set_var_honorbnd (      KN_context_ptr  kc,
                                     const KNINT           indexVar,
                                     const int             xHonorBnd);

/** This API function can be used to identify which constraints
 *  should satisfy their bounds throughout the optimization process.
 *  Note that this feature currently only applies to INEQUALITY
 *  constraints when using one of the barrier/interior-point algorithms.
 *  The user option KN_PARAM_BAR_FEASIBLE can be used to set ALL
 *  inequality constraints to honor their bounds (i.e. stay feasible).
 *  This routine takes precedence over the setting of KN_PARAM_BAR_FEASIBLE
 *  and is used to customize the settings for individual inequality
 *  constraints.
 *
 *  The initial point must satisfy the inequality constraints specified
 *  through this API function to a sufficient degree; if not, Knitro may
 *  generate infeasible iterates and does not enable the "honorbnds"
 *  procedure until a sufficiently feasible point is found. Sufficient
 *  satisfaction occurs at a point x if it is true for all specified
 *  inequalities constraints that
 *      cl + tol \leq c(x) \leq cu - tol
 *  The constant "tol" is determined by the user option bar_feasmodetol.
 *
 *  Knitro makes a local copy of all inputs, so the application may
 *  free memory after the call.
 *  Returns 0 if OK, nonzero if error.
 */
int  KNITRO_API KN_set_con_honorbnds (      KN_context_ptr  kc,
                                      const KNINT           nC,
                                      const KNINT  * const  indexCons,
                                      const int    * const  cHonorBnds);
int  KNITRO_API KN_set_con_honorbnds_all (      KN_context_ptr  kc,
                                          const int    * const  cHonorBnds);
int  KNITRO_API KN_set_con_honorbnd (      KN_context_ptr  kc,
                                     const KNINT           indexCon,
                                     const int             cHonorBnd);

/* ----- MIP-specific settings ----- */

/** Set initial primal variables values for MIP.  This point may be used to
 *  search for an initial feasible point and may be different from the starting
 *  point for the root relaxation subproblem.
 *  Returns 0 if OK, nonzero if error.
 */
int  KNITRO_API KN_set_mip_var_primal_init_values (      KN_context_ptr  kc,
                                                   const KNINT           nV,
                                                   const KNINT  * const  indexVars,
                                                   const double * const  xInitVals);
int  KNITRO_API KN_set_mip_var_primal_init_values_all (      KN_context_ptr  kc,
                                                       const double * const  xInitVals);
int  KNITRO_API KN_set_mip_var_primal_init_value (      KN_context_ptr  kc,
                                                  const KNINT           indexVar,
                                                  const double          xInitVal);

/** Set the branching priorities for integer variables. Must first
 *  set the types of variables (e.g. by calling KN_set_var_types) before
 *  calling this function. Priorities must be positive numbers
 *  (variables with non-positive values are ignored).  Variables with
 *  higher priority values will be considered for branching before
 *  variables with lower priority values.  When priorities for a subset
 *  of variables are equal, the branching rule is applied as a tiebreaker.
 *  Values for continuous variables are ignored.  Knitro makes a local
 *  copy of all inputs, so the application may free memory after the call.
 *  Returns 0 if OK, nonzero if error.
 */
int  KNITRO_API KN_set_mip_branching_priorities
    (      KN_context_ptr  kc,
     const KNINT           nV,
     const KNINT * const   indexVars,
     const int   * const   xPriorities);
int  KNITRO_API KN_set_mip_branching_priorities_all
    (      KN_context_ptr  kc,
     const int   * const   xPriorities);
int  KNITRO_API KN_set_mip_branching_priority
    (      KN_context_ptr  kc,
     const KNINT           indexVar,
     const int             xPriority);

/** Set strategies for dealing with individual integer variables. Possible
 *  strategy values include:
 *    KN_MIP_INTVAR_STRATEGY_NONE    0 (default)
 *    KN_MIP_INTVAR_STRATEGY_RELAX   1
 *    KN_MIP_INTVAR_STRATEGY_MPEC    2 (binary variables only)
 *  indexVars should be an index value corresponding to an integer variable
 *  (nothing is done if the index value corresponds to a continuous variable),
 *  and xStrategies should correspond to one of the strategy values listed above.
 *  Returns 0 if OK, nonzero if error.
 */
int  KNITRO_API KN_set_mip_intvar_strategies
    (      KN_context_ptr  kc,
     const KNINT           nV,
     const KNINT * const   indexVars,
     const int   * const   xStrategies);
int  KNITRO_API KN_set_mip_intvar_strategies_all
    (      KN_context_ptr  kc,
     const int * const     xStrategies);
int  KNITRO_API KN_set_mip_intvar_strategy
    (      KN_context_ptr  kc,
     const KNINT           indexVar,
     const int             xStrategy);

/* ----- Solving ----- */

/** Call Knitro to solve the problem.  The return value indicates
 *  the solution status:
 *              0: the final solution is optimal to specified tolerances;
 *   -100 to -109: a feasible solution was found (but not verified optimal);
 *   -200 to -209: Knitro terminated at an infeasible point;
 *   -300 to -301: the problem was determined to be unbounded;
 *   -400 to -409: Knitro terminated because it reached a pre-defined limit
 *                (a feasible point was found before reaching the limit);
 *   -410 to -419: Knitro terminated because it reached a pre-defined limit
 *                (no feasible point was found before reaching the limit);
 *   -500 to -599: Knitro terminated with an input error or some non-standard error.
 *  Refer to the Knitro manual section on Return Codes for more details.  All
 *  possible return code values are defined at the bottom of this file.
 */
int  KNITRO_API KN_solve (KN_context_ptr  kc);

/** Call Knitro to update the problem.
 *  If the model has been modified (e.g. via adding, changing, deleting
 *  structures), since the last solve, calling KN_update will update the
 *  internal model to incorporate all the changes since the most recent
 *  solve, but will not proceed with solving. Typically, for efficiency,
 *  the model is not actually updated with the most recent modifications
 *  until KN_solve is called, but KN_update allows for updating of the model
 *  without solving.
 *  Returns 0 if OK, nonzero if error.
 */
int  KNITRO_API KN_update (KN_context_ptr  kc);

/* ----- Reading model/solution properties ----- */

/** Retrieve the number of variables "nV" in the model.
 *  Returns 0 if OK, nonzero if error.
 */
int  KNITRO_API KN_get_number_vars (const KN_context_ptr  kc,
                                          KNINT * const   nV);

/** Retrieve the number of constraints "nC" in the model.
 *  Returns 0 if OK, nonzero if error.
 */
int  KNITRO_API KN_get_number_cons (const KN_context_ptr  kc,
                                          KNINT * const   nC);

/** Retrieve the number of complementarity constraints "nCC" in the model.
 *  Returns 0 if OK, nonzero if error.
 */
int  KNITRO_API KN_get_number_compcons (const KN_context_ptr  kc,
                                              KNINT * const   nCC);

/** Retrieve the number of residuals "nR" in the model.
 *  Returns 0 if OK, nonzero if error.
 */
int  KNITRO_API KN_get_number_rsds (const KN_context_ptr  kc,
                                          KNINT * const   nR);

/** Return the number of function callback evaluations requested by KN_solve
 *  in "numFCevals". One evaluation count includes a single evaluation of the
 *  objective and all the constraints defined via callbacks (whether evaluated
 *  altogether in one callback or evaluated using several separate callbacks).
 *  Returns 0 if OK, nonzero if error.
 */
int  KNITRO_API KN_get_number_FC_evals (const KN_context_ptr  kc,
                                              int * const     numFCevals);

/** Return the number of gradient callback evaluations requested by KN_solve
 *  in "numGAevals". One evaluation count includes a single evaluation of the
 *  first derivatives of the objective and all the constraints defined via
 *  gradient callbacks (whether evaluated altogether in one callback or
 *  evaluated using several separate callbacks).
 *  Returns 0 if OK, nonzero if error.
 */
int  KNITRO_API KN_get_number_GA_evals (const KN_context_ptr  kc,
                                              int * const     numGAevals);

/** Return the number of Hessian callback evaluations requested by KN_solve
 *  in "numHevals". One evaluation count includes a single evaluation of all
 *  the components of the Hessian of the Lagrangian matrix defined via
 *  callbacks (whether evaluated altogether in one callback or evaluated using
 *  several separate callbacks).
 *  Returns 0 if OK, nonzero if error.
 */
int  KNITRO_API KN_get_number_H_evals (const KN_context_ptr  kc,
                                             int * const     numHevals);

/** Return the number of Hessian-vector callback evaluations requested
 *  by KN_solve in "numHVevals". One evaluation count includes a single
 *  evaluation of the product of the Hessian of the Lagrangian matrix with a
 *  vector submitted by Knitro (whether evaluated altogether in one callback
 *  or evaluated using several separate callbacks).
 *  Returns 0 if OK, nonzero if error.
 */
int  KNITRO_API KN_get_number_HV_evals (const KN_context_ptr  kc,
                                              int * const     numHVevals);

/** Retrieve the Knitro solve time either as CPU time or real time.
 */
int  KNITRO_API KN_get_solve_time_cpu(const KN_context_ptr  kc,
                                            double * const  time);
int  KNITRO_API KN_get_solve_time_real(const KN_context_ptr  kc,
                                             double * const  time);

/** Return the solution status, objective, primal and dual variables.
 *  The status and objective value scalars are returned as pointers
 *  that need to be de-referenced to get their values.  The arrays
 *  "x" and "lambda" must be allocated by the user.
 *  Returns 0 if call is successful;
 *         <0 if there is an error.
 */
int  KNITRO_API KN_get_solution (const KN_context_ptr  kc,
                                       int    * const  status,
                                       double * const  obj,
                                       double * const  x,
                                       double * const  lambda);

/** Return the objective, primal and dual variables, and constraint
 *  values for the best feasible iterate encountered throughout the
 *  optimization (i.e. the feasible iterate with the best objective
 *  value).  The (absolute) feasibility error computed at this point
 *  is also returned.  This point may not always correspond to the
 *  default final solution returned by Knitro. If no feasible point was
 *  found, then the least infeasible point found is returned.
 *  The "feasError" and "obj" value scalars are returned as pointers
 *  that need to be de-referenced to get their values.  The arrays
 *  "x", "lambda", and "c" must be allocated by the user.
 *  Returns 0 if feasible point found and call is successful;
 *          1 if no feasible point was found;
 *         <0 if there is an error.
 */
int  KNITRO_API KN_get_best_feasible_iterate (const KN_context_ptr  kc,
                                                    double * const  feasError,
                                                    double * const  obj,
                                                    double * const  x,
                                                    double * const  lambda,
                                                    double * const  c);

/** Return the value of the objective obj(x) in "obj".
 *  Returns 0 if call is successful;
 *         <0 if there is an error.
 */
int  KNITRO_API KN_get_obj_value (const KN_context_ptr  kc,
                                        double * const  obj);

/** Return the type (e.g. KN_OBJTYPE_GENERAL, KN_OBJTYPE_LINEAR,
 *  KN_OBJTYPE_QUADRATIC, etc.) of the objective obj(x) in "objType".
 *  Returns 0 if call is successful;
 *         <0 if there is an error.
 */
int  KNITRO_API KN_get_obj_type (const KN_context_ptr  kc,
                                       int    * const  objType);

/** Return the primal ("x") or dual ("lambda") variables.
 *  Returns 0 if call is successful;
 *         <0 if there is an error.
 */
int  KNITRO_API KN_get_var_primal_values (const KN_context_ptr  kc,
                                          const KNINT           nV,
                                          const KNINT  * const  indexVars,
                                                double * const  x);
int  KNITRO_API KN_get_var_primal_values_all (const KN_context_ptr  kc,
                                                    double * const  x);
int  KNITRO_API KN_get_var_primal_value (const KN_context_ptr  kc,
                                         const KNINT           indexVar,
                                               double * const  x);

int  KNITRO_API KN_get_var_dual_values (const KN_context_ptr  kc,
                                        const KNINT           nV,
                                        const KNINT  * const  indexVars,
                                              double * const  lambda);
int  KNITRO_API KN_get_var_dual_values_all (const KN_context_ptr  kc,
                                                double * const  lambda);
int  KNITRO_API KN_get_var_dual_value (const KN_context_ptr  kc,
                                       const KNINT           indexVar,
                                             double * const  lambda);

int  KNITRO_API KN_get_con_dual_values (const KN_context_ptr  kc,
                                        const KNINT           nC,
                                        const KNINT  * const  indexCons,
                                              double * const  lambda);
int  KNITRO_API KN_get_con_dual_values_all (const KN_context_ptr  kc,
                                                  double * const  lambda);
int  KNITRO_API KN_get_con_dual_value (const KN_context_ptr  kc,
                                       const KNINT           indexCons,
                                             double * const  lambda);

/** Return the values of the constraint vector c(x) in "c".
 *  The array "c" must be allocated by the user.
 *  Returns 0 if call is successful;
 *         <0 if there is an error.
 */
int  KNITRO_API KN_get_con_values (const KN_context_ptr  kc,
                                   const KNINT           nC,
                                   const KNINT  * const  indexCons,
                                         double * const  c);
int  KNITRO_API KN_get_con_values_all (const KN_context_ptr  kc,
                                             double * const  c);
int  KNITRO_API KN_get_con_value (const KN_context_ptr  kc,
                                  const KNINT           indexCon,
                                        double * const  c);

/** Return the types (e.g. KN_CONTYPE_GENERAL, KN_CONTYPE_LINEAR,
 *  KN_CONTYPE_QUADRATIC, etc.) of the constraint vector c(x) in "cTypes".
 *  The array "cTypes" must be allocated by the user.
 *  Returns 0 if call is successful;
 *         <0 if there is an error.
 */
int  KNITRO_API KN_get_con_types (const KN_context_ptr  kc,
                                  const KNINT           nC,
                                  const KNINT  * const  indexCons,
                                        int    * const  cTypes);
int  KNITRO_API KN_get_con_types_all (const KN_context_ptr  kc,
                                            int    * const  cTypes);
int  KNITRO_API KN_get_con_type (const KN_context_ptr  kc,
                                 const KNINT           indexCon,
                                       int    * const  cType);

/** Return the values of the residual vector r(x) in "r".
 *  The array "r" must be allocated by the user.
 *  Returns 0 if call is successful;
 *         <0 if there is an error.
 */
int  KNITRO_API KN_get_rsd_values (const KN_context_ptr  kc,
                                   const KNINT           nR,
                                   const KNINT  * const  indexRsds,
                                         double * const  r);
int  KNITRO_API KN_get_rsd_values_all (const KN_context_ptr  kc,
                                             double * const  r);
int  KNITRO_API KN_get_rsd_value (const KN_context_ptr  kc,
                                  const KNINT           indexRsd,
                                        double * const  r);

/** Return information about the feasibility of the variables x after
 *  solving.
 *
 *  The integer array "bndInfeas" indicates whether or not the variable
 *  was deemed to be infeasible with respect to its bounds by the Knitro
 *  termination test.
 *  Possible values of "bndInfeas" are:
 *     -1: The variable is considered infeasible with respect to
 *         its lower bound (or is less than its fixed value if fixed).
 *      0: The variable is considered feasible with respect to the
 *         feasibility tolerances.
 *      1: The variable is considered infeasible with respect to
 *         its upper bound (or is greater than its fixed value if fixed).
 *  Note that even if the variable is deemed feasible it may still have a
 *  non-zero violation returned in "viols" since the feasibility
 *  tolerances allow for small violations.
 *
 *  The integer array "intInfeas" indicates whether or not an integer
 *  variable was deemed to be infeasible with respect to integrality.
 *  Possible values of "intInfeas" are:
 *     -1: The integer variable is considered infeasible with respect to
 *         the integrality tolerance and is closer to its floor.
 *      0: The integer variable is considered feasible with respect to
 *         the integrality tolerance (or the variable is continuous).
 *      1: The integer variable is considered infeasible with respect to
 *         the integrality tolerance and is closer to its ceiling.
 *
 *  The non-negative array "viols" returns the amount by which the
 *  variable violates its lower or upper bound.  If the variable is an
 *  integer variable that satisfies its lower and upper bounds, "viols"
 *  returns the integrality error (i.e. the distance to the nearest
 *  integer value inside its bounds).
 *
 *  The arrays "bndInfeas", "intInfeas" and "viols" must be allocated by
 *  the user to retrieve these values.  Any of these arrays set to NULL
 *  will not be returned.
 *
 *  Returns 0 if call is successful;
 *         <0 if there is an error.
 */
int  KNITRO_API KN_get_var_viols (const KN_context_ptr  kc,
                                  const KNINT           nV,
                                  const KNINT  * const  indexVars,
                                        KNINT  * const  bndInfeas,
                                        KNINT  * const  intInfeas,
                                        double * const  viols);
int  KNITRO_API KN_get_var_viols_all (const KN_context_ptr  kc,
                                            KNINT  * const  bndInfeas,
                                            KNINT  * const  intInfeas,
                                            double * const  viols);
int  KNITRO_API KN_get_var_viol (const KN_context_ptr  kc,
                                 const KNINT           indexVar,
                                       KNINT  * const  bndInfeas,
                                       KNINT  * const  intInfeas,
                                       double * const  viol);

/** Return information about the feasibility of the constraints c(x)
 *  after solving.
 *
 *  The integer array "infeas" indicates whether or not the constraint
 *  was deemed to be infeasible by the Knitro termination test.
 *  Possible values of "infeas" are:
 *     -1: The constraint is considered infeasible with respect to
 *         its lower bound (or is less than the right-hand side if
 *         it is an equality).
 *      0: The constraint is considered feasible with respect to the
 *         feasibility tolerances.
 *      1: The constraint is considered infeasible with respect to
 *         its upper bound (or is greater than the right-hand side if
 *         it is an equality).
 *  Note that even if the constraint is deemed feasible it may still
 *  have a non-zero violation returned in "viols" since the feasibility
 *  tolerances allow for small violations.
 *
 *  The non-negative array "viols" returns the amount by which the
 *  constraint violates its lower or upper bound.
 *
 *  The arrays "infeas" and "viols" must be allocated by the user to
 *  retrieve these values.  Any of these arrays set to NULL will not be
 *  returned.
 *
 *  Returns 0 if call is successful;
 *         <0 if there is an error.
 */
int  KNITRO_API KN_get_con_viols (const KN_context_ptr  kc,
                                  const KNINT           nC,
                                  const KNINT  * const  indexCons,
                                        KNINT  * const  infeas,
                                        double * const  viols);
int  KNITRO_API KN_get_con_viols_all (const KN_context_ptr  kc,
                                            KNINT  * const  infeas,
                                            double * const  viols);
int  KNITRO_API KN_get_con_viol (const KN_context_ptr  kc,
                                 const KNINT           indexCon,
                                       KNINT  * const  infeas,
                                       double * const  viol);

/** Return information about any errors detected by the Knitro presolver.
 *  Parameters have the following values.
 *
 *  component: The component involved in the error
 *             (e.g., KN_COMPONENT_VAR, KN_COMPONENT_OBJ
 *             KN_COMPONENT_CON, KN_COMPONENT_RSD).
 *             Return value of 0 if no error.
 *
 *  index: The component index involved in the error (is set to -1 if no
 *         error).
 *
 *  error: Return code associated with the error
 *         (e.g., KN_RC_INFEAS_VAR_BOUNDS, KN_RC_INFEAS_CON_BOUNDS,
 *         KN_RC_UNBOUNDED_OR_INFEAS).  Return value of 0 if no error.
 *
 *  viol: If there was an infeasibility error, it returns the
 *        amount of infeasibility deduced by the presolve for the
 *        given component (otherwise is set to 0).
 *
 *  Returns 0 if no presolve errors were detected;
 *          1 if an error was detected in the presolve phase;
 *         <0 if there is an error in the function call.
 */
int  KNITRO_API KN_get_presolve_error (const KN_context_ptr  kc,
                                             KNINT  * const  component,
                                             KNINT  * const  index,
                                             KNINT  * const  error,
                                             double * const  viol);

/* ----- Solution properties for continuous problems only ----- */

/** Return the number of iterations made by KN_solve in "numIters".
 *  Returns 0 if OK, nonzero if error.
 */
int  KNITRO_API KN_get_number_iters (const KN_context_ptr  kc,
                                           int * const     numIters);

/** Return the number of conjugate gradient (CG) iterations made by
 *  KN_solve in "numCGiters".
 *  Returns 0 if OK, nonzero if error.
 */
int  KNITRO_API KN_get_number_cg_iters (const KN_context_ptr  kc,
                                              int * const     numCGiters);

/** Return the absolute feasibility error at the solution in "absFeasError".
 *  Refer to the Knitro manual section on Termination Tests for a
 *  detailed definition of this quantity.
 *  Returns 0 if OK, nonzero if error.
 */
int  KNITRO_API KN_get_abs_feas_error (const KN_context_ptr  kc,
                                             double * const  absFeasError);

/** Return the relative feasibility error at the solution in "relFeasError".
 *  Refer to the Knitro manual section on Termination Tests for a
 *  detailed definition of this quantity.
 *  Returns 0 if OK, nonzero if error.
 */
int  KNITRO_API KN_get_rel_feas_error (const KN_context_ptr  kc,
                                             double * const  relFeasError);

/** Return the absolute optimality error at the solution in "absOptError".
 *  Refer to the Knitro manual section on Termination Tests for a
 *  detailed definition of this quantity.
 *  Returns 0 if OK, nonzero if error.
 */
int  KNITRO_API KN_get_abs_opt_error (const KN_context_ptr  kc,
                                            double * const  absOptError);

/** Return the relative optimality error at the solution in "relOptError".
 *  Refer to the Knitro manual section on Termination Tests for a
 *  detailed definition of this quantity.
 *  Returns 0 if OK, nonzero if error.
 */
int  KNITRO_API KN_get_rel_opt_error (const KN_context_ptr  kc,
                                            double * const  relOptError);

/** Return the values of the objective gradient vector in "indexVars"
 *  and "objGrad".  The objective gradient values returned correspond
 *  to the non-zero sparse objective gradient indices provided by the user.
 *  The arrays "indexVars" and "objGrad" must be allocated by the user.
 *  The size of these arrays is obtained by first calling
 *  "KN_get_objgrad_nnz()".
 *  Returns 0 if call is successful;
 *         <0 if there is an error.
 */
int  KNITRO_API KN_get_objgrad_nnz  (const KN_context_ptr  kc,
                                           KNINT  * const  nnz);
int  KNITRO_API KN_get_objgrad_values (const KN_context_ptr  kc,
                                             KNINT  * const  indexVars,
                                             double * const  objGrad);
/** Return the values of the full (dense) objective gradient in "objGrad".
 *  The array "objGrad" must be allocated by the user (the size is equal
 *  to the total number of variables in the problem).
 *  Returns 0 if call is successful;
 *         <0 if there is an error.
 */
int  KNITRO_API KN_get_objgrad_values_all (const KN_context_ptr  kc,
                                                 double * const  objGrad);

/** Return the values of the constraint Jacobian in "indexCons", "indexVars",
 *  and "jac".  The Jacobian values returned correspond to the non-zero
 *  sparse Jacobian indices provided by the user.
 *  The arrays "indexCons", "indexVars", and "jac" must be allocated by
 *  the user.  The size of these arrays is obtained by first calling
 *  "KN_get_jacobian_nnz()".
 *
 *  Use "KN_get_jacobian_nnz_one()" / "KN_get_jacobian_values_one()" to
 *  get the gradient/Jacobian values corresponding to just one constraint.
 *
 *  Returns 0 if call is successful;
 *         <0 if there is an error.
 */
int  KNITRO_API KN_get_jacobian_nnz    (const KN_context_ptr  kc,
                                              KNLONG * const  nnz);
int  KNITRO_API KN_get_jacobian_values (const KN_context_ptr  kc,
                                              KNINT  * const  indexCons,
                                              KNINT  * const  indexVars,
                                              double * const  jac);
int  KNITRO_API KN_get_jacobian_nnz_one    (const KN_context_ptr  kc,
                                                  KNINT           indexCon,
                                                  KNINT * const   nnz);
int  KNITRO_API KN_get_jacobian_values_one (const KN_context_ptr  kc,
                                                  KNINT           indexCon,
                                                  KNINT  * const  indexVars,
                                                  double * const  jac);

/** Return the values of the residual Jacobian in "indexRsds", "indexVars",
 *  and "rsdJac".  The Jacobian values returned correspond to the non-zero
 *  sparse Jacobian indices provided by the user.
 *  The arrays "indexRsds", "indexVars" and "rsdJac" must be allocated
 *  by the user.  The size of these arrays is obtained by first calling
 *  "KN_get_rsd_jacobian_nnz()".
 *  Returns 0 if call is successful;
 *         <0 if there is an error.
 */
int  KNITRO_API KN_get_rsd_jacobian_nnz    (const KN_context_ptr  kc,
                                                  KNLONG * const  nnz);
int  KNITRO_API KN_get_rsd_jacobian_values (const KN_context_ptr  kc,
                                                  KNINT  * const  indexRsds,
                                                  KNINT  * const  indexVars,
                                                  double * const  rsdJac);

/** Return the values of the Hessian (or possibly Hessian
 *  approximation) in "hess".  This routine is currently only valid
 *  if one of the following cases holds:
 *  1) KN_HESSOPT_EXACT (presolver on or off), or;
 *  2) KN_HESSOPT_BFGS or KN_HESSOPT_SR1, but only with the
 *     Knitro presolver off (i.e. KN_PRESOLVE_NONE).
 *  3) Solving a least squares model with the Gauss-Newton Hessian
 *     and the Gauss-Newton Hessian is explicitly computed and
 *     stored in Knitro.
 *
 *  In all other cases, either Knitro does not have an internal
 *  representation of the Hessian (or Hessian approximation),
 *  or the internal Hessian approximation corresponds only to
 *  the presolved problem form and may not be valid for the
 *  original problem form.  In these cases "indexVars1", "indexVars2",
 *  and "hess" are left unmodified, and the routine has return code 1.
 *
 *  Note that in case 2 above (KN_HESSOPT_BFGS or KN_HESSOPT_SR1)
 *  the values returned in "hess" are the upper triangular values
 *  of the dense quasi-Newton Hessian approximation stored row-wise.
 *  There are ((n*n - n)/2 + n) such values (where "n" is the number
 *  of variables in the problem. These values may be quite different
 *  from the values of the exact Hessian.
 *
 *  When KN_HESSOPT_EXACT (case 1 above) the Hessian values
 *  returned correspond to the non-zero sparse Hessian indices
 *  provided by the user.
 *
 *  The arrays "indexVars1", "indexVars2" and "hess" must be allocated
 *  by the user.  The size of these arrays is obtained by first calling
 *  "KN_get_hessian_nnz()".
 *  Returns 0 if call is successful;
 *          1 if "hess" was not set because Knitro does not
 *            have a valid Hessian for the model stored.
 *         <0 if there is an error.
 */
int  KNITRO_API KN_get_hessian_nnz    (const KN_context_ptr  kc,
                                             KNLONG * const  nnz);
int  KNITRO_API KN_get_hessian_values (const KN_context_ptr  kc,
                                             KNINT  * const  indexVars1,
                                             KNINT  * const  indexVars2,
                                             double * const  hess);


/* ----- Solution properties for MIP problems only ----- */

/** Return the number of nodes processed in the MIP solve
 *  in "numNodes".
 *  Returns 0 if OK, nonzero if error.
 */
int  KNITRO_API KN_get_mip_number_nodes (const KN_context_ptr  kc,
                                               int * const     numNodes);

/** Return the number of continuous subproblems processed in the
 *  MIP solve in "numSolves".
 *  Returns 0 if OK, nonzero if error.
 */
int  KNITRO_API KN_get_mip_number_solves (const KN_context_ptr  kc,
                                                int * const     numSolves);

/** Return the final absolute optimality gap in the MIP solve
 *  in "absGap". Refer to the Knitro manual section on Termination
 *  Tests for a detailed definition of this quantity. Set to
 *  KN_INFINITY if no incumbent (i.e., integer feasible) point found.
 *  Returns 0 if OK, nonzero if error.
 */
int  KNITRO_API KN_get_mip_abs_gap (const KN_context_ptr  kc,
                                          double * const  absGap);

/** Return the final absolute optimality gap in the MIP solve
 *  int "relGap". Refer to the Knitro manual section on Termination
 *  Tests for a detailed definition of this quantity.  Set to
 *  KN_INFINITY if no incumbent (i.e., integer feasible) point found.
 *  Returns 0 if OK, nonzero if error.
 */
int  KNITRO_API KN_get_mip_rel_gap (const KN_context_ptr  kc,
                                          double * const  relGap);

/** Return the objective value of the MIP incumbent solution in
 *  "incumbentObj". Set to KN_INFINITY if no incumbent (i.e., integer
 *  feasible) point found.
 *  Returns 0 if incumbent solution exists and call is successful;
 *          1 if no incumbent (i.e., integer feasible) exists;
 *         <0 if there is an error.
 */
int  KNITRO_API KN_get_mip_incumbent_obj (const KN_context_ptr  kc,
                                                double * const  incumbentObj);

/** Return the value of the current MIP relaxation bound in "relaxBound".
 *  Returns 0 if OK, nonzero if error.
 */
int  KNITRO_API KN_get_mip_relaxation_bnd (const KN_context_ptr  kc,
                                                 double * const relaxBound);

/** Return the objective value of the most recently solved MIP
 *  node subproblem in "lastNodeObj".
 *  Returns 0 if OK, nonzero if error.
 */
int  KNITRO_API KN_get_mip_lastnode_obj (const KN_context_ptr  kc,
                                               double * const lastNodeObj);

/** Return the MIP incumbent solution in "x" if one exists.
 *  Returns 0 if incumbent solution exists and call is successful;
 *          1 if no incumbent (i.e., integer feasible) point exists
 *              and leaves "x" unmodified;
 *         <0 if there is an error.
 */
int  KNITRO_API KN_get_mip_incumbent_x (const KN_context_ptr  kc,
                                              double * const  x);


/*------------------------------------------------------------------*/
/*     DEFINES                                                      */
/*------------------------------------------------------------------*/

/** Use KN_INFINITY to set infinite variable and constraint bounds
 *  in Knitro.
 */
#define KN_INFINITY DBL_MAX

/** Possible parameter types.
 */
#define KN_PARAMTYPE_INTEGER 0
#define KN_PARAMTYPE_FLOAT   1
#define KN_PARAMTYPE_STRING  2

/** Possible model components.
 */
#define KN_COMPONENT_VAR     1
#define KN_COMPONENT_OBJ     2
#define KN_COMPONENT_CON     3
#define KN_COMPONENT_RSD     4

/** Possible objective goals for the solver (objGoal in KN_set_obj_goal).
 */
#define KN_OBJGOAL_MINIMIZE    0
#define KN_OBJGOAL_MAXIMIZE    1

/** Possible values for the objective type.
 */
#define KN_OBJTYPE_CONSTANT  -1
#define KN_OBJTYPE_GENERAL    0
#define KN_OBJTYPE_LINEAR     1
#define KN_OBJTYPE_QUADRATIC  2

/** Possible values for the constraint type.
 */
#define KN_CONTYPE_CONSTANT  -1
#define KN_CONTYPE_GENERAL    0
#define KN_CONTYPE_LINEAR     1
#define KN_CONTYPE_QUADRATIC  2
#define KN_CONTYPE_CONIC      3

/** Possible values for the residual type.
 */
#define KN_RSDTYPE_CONSTANT  -1
#define KN_RSDTYPE_GENERAL    0
#define KN_RSDTYPE_LINEAR     1

/** Possible values for the complementarity constraint type
 *  (ccTypes in KN_set_compcons).  Currently only KN_CCTYPE_VARVAR
 *  is supported.
 */
#define KN_CCTYPE_VARVAR      0
#define KN_CCTYPE_VARCON      1 /* NOT SUPPORTED YET */
#define KN_CCTYPE_CONCON      2 /* NOT SUPPORTED YET */

/** Possible values for the variable type (xTypes in KN_set_var_types).
 */
#define KN_VARTYPE_CONTINUOUS  0
#define KN_VARTYPE_INTEGER     1
#define KN_VARTYPE_BINARY      2

/** Possible values for enabling bits to set variable properties
 *  via KN_set_var_properties().
 */
#define KN_VAR_LINEAR                   1 /*-- LINEAR ONLY EVERYWHERE */

/** Possible values for bit flags used to set objective and
 *  constraint function properties via KN_set_obj_properties()
 *  and KN_set_con_properties().
 */
#define KN_OBJ_CONVEX                   1 /*-- CONVEX OBJECTIVE */
#define KN_OBJ_CONCAVE                  2 /*-- CONCAVE OBJECTIVE */
#define KN_OBJ_CONTINUOUS               4 /*-- OBJECTIVE IS CONTINUOUS */
#define KN_OBJ_DIFFERENTIABLE           8 /*-- (ONCE) DIFFERENTIABLE OBJECTIVE */
#define KN_OBJ_TWICE_DIFFERENTIABLE    16 /*-- TWICE DIFFERENTIABLE OBJECTIVE */
#define KN_OBJ_NOISY                   32 /*-- OBJECTIVE FUNCTION IS NOISY */
#define KN_OBJ_NONDETERMINISTIC        64 /*-- OBJECTIVE IS NONDETERMINISTIC */

#define KN_CON_CONVEX                   1 /*-- CONVEX CONSTRAINT */
#define KN_CON_CONCAVE                  2 /*-- CONCAVE CONSTRAINT */
#define KN_CON_CONTINUOUS               4 /*-- CONSTRAINT IS CONTINUOUS */
#define KN_CON_DIFFERENTIABLE           8 /*-- (ONCE) DIFFERENTIABLE CONSTRAINT */
#define KN_CON_TWICE_DIFFERENTIABLE    16 /*-- TWICE DIFFERENTIABLE CONSTRAINT */
#define KN_CON_NOISY                   32 /*-- CONSTRAINT FUNCTION IS NOISY */
#define KN_CON_NONDETERMINISTIC        64 /*-- CONSTRAINT IS NONDETERMINISTIC */

/** Possible values for dense arrays or matrices.
 */
#define KN_DENSE             -1 /*-- GENERIC DENSE (e.g. FOR ARRAYS) */
#define KN_DENSE_ROWMAJOR    -2 /*-- DENSE MATRIX IN ROW MAJOR ORDER  */
#define KN_DENSE_COLMAJOR    -3 /*-- DENSE MATRIX IN COLUMN MAJOR ORDER  */

/** Evaluation request codes
 */
#define KN_RC_EVALFC          1  /*-- OBJECTIVE AND CONSTRAINT FUNCTIONS */
#define KN_RC_EVALGA          2  /*-- OBJ. GRADIENT AND CONSTRAINT JACOBIAN */
#define KN_RC_EVALH           3  /*-- HESSIAN OF THE LAGRANGIAN */
#define KN_RC_EVALHV          7  /*-- HESSIAN-VECTOR PRODUCT */
#define KN_RC_EVALH_NO_F      8  /*-- NO OBJECTIVE COMPONENT INCLUDED */
#define KN_RC_EVALHV_NO_F     9  /*-- NO OBJECTIVE COMPONENT INCLUDED */
#define KN_RC_EVALR          10  /*-- RESIDUAL FUNCTIONS (LEAST SQUARES) */
#define KN_RC_EVALRJ         11  /*-- RESIDUAL JACOBIAN (LEAST SQUARES) */
#define KN_RC_EVALFCGA       12  /*-- BOTH FUNCTIONS AND GRADIENTS  */

/** Return codes when Knitro terminates.
 */
#define KN_RC_OPTIMAL_OR_SATISFACTORY 0   /*-- OPTIMAL CODE */
#define KN_RC_OPTIMAL                 0
#define KN_RC_NEAR_OPT               -100 /*-- FEASIBLE CODES */
#define KN_RC_FEAS_XTOL              -101
#define KN_RC_FEAS_NO_IMPROVE        -102
#define KN_RC_FEAS_FTOL              -103
#define KN_RC_FEAS_BEST              -104
#define KN_RC_FEAS_MULTISTART        -105
#define KN_RC_INFEASIBLE             -200 /*-- INFEASIBLE CODES */
#define KN_RC_INFEAS_XTOL            -201
#define KN_RC_INFEAS_NO_IMPROVE      -202
#define KN_RC_INFEAS_MULTISTART      -203
#define KN_RC_INFEAS_CON_BOUNDS      -204
#define KN_RC_INFEAS_VAR_BOUNDS      -205
#define KN_RC_UNBOUNDED              -300 /*-- UNBOUNDED CODES */
#define KN_RC_UNBOUNDED_OR_INFEAS    -301
#define KN_RC_ITER_LIMIT_FEAS        -400 /*-- LIMIT EXCEEDED CODES (FEASIBLE) */
#define KN_RC_TIME_LIMIT_FEAS        -401
#define KN_RC_FEVAL_LIMIT_FEAS       -402
#define KN_RC_MIP_EXH_FEAS           -403
#define KN_RC_MIP_TERM_FEAS          -404
#define KN_RC_MIP_SOLVE_LIMIT_FEAS   -405
#define KN_RC_MIP_NODE_LIMIT_FEAS    -406
#define KN_RC_ITER_LIMIT_INFEAS      -410 /*-- LIMIT EXCEEDED CODES (INFEASIBLE) */
#define KN_RC_TIME_LIMIT_INFEAS      -411
#define KN_RC_FEVAL_LIMIT_INFEAS     -412
#define KN_RC_MIP_EXH_INFEAS         -413
#define KN_RC_MIP_SOLVE_LIMIT_INFEAS -415
#define KN_RC_MIP_NODE_LIMIT_INFEAS  -416
#define KN_RC_CALLBACK_ERR           -500 /*-- OTHER FAILURES */
#define KN_RC_LP_SOLVER_ERR          -501
#define KN_RC_EVAL_ERR               -502
#define KN_RC_OUT_OF_MEMORY          -503
#define KN_RC_USER_TERMINATION       -504
#define KN_RC_OPEN_FILE_ERR          -505
#define KN_RC_BAD_N_OR_F             -506 /*-- PROBLEM DEFINITION ERROR */
#define KN_RC_BAD_CONSTRAINT         -507 /*-- PROBLEM DEFINITION ERROR */
#define KN_RC_BAD_JACOBIAN           -508 /*-- PROBLEM DEFINITION ERROR */
#define KN_RC_BAD_HESSIAN            -509 /*-- PROBLEM DEFINITION ERROR */
#define KN_RC_BAD_CON_INDEX          -510 /*-- PROBLEM DEFINITION ERROR */
#define KN_RC_BAD_JAC_INDEX          -511 /*-- PROBLEM DEFINITION ERROR */
#define KN_RC_BAD_HESS_INDEX         -512 /*-- PROBLEM DEFINITION ERROR */
#define KN_RC_BAD_CON_BOUNDS         -513 /*-- PROBLEM DEFINITION ERROR */
#define KN_RC_BAD_VAR_BOUNDS         -514 /*-- PROBLEM DEFINITION ERROR */
#define KN_RC_ILLEGAL_CALL           -515 /*-- KNITRO CALL IS OUT OF SEQUENCE */
#define KN_RC_BAD_KCPTR              -516 /*-- KNITRO PASSED A BAD KC POINTER */
#define KN_RC_NULL_POINTER           -517 /*-- KNITRO PASSED A NULL ARGUMENT */
#define KN_RC_BAD_INIT_VALUE         -518 /*-- APPLICATION INITIAL POINT IS BAD */
#define KN_RC_LICENSE_ERROR          -520 /*-- LICENSE CHECK FAILED */
#define KN_RC_BAD_PARAMINPUT         -521 /*-- INVALID PARAMETER INPUT DETECTED */
#define KN_RC_LINEAR_SOLVER_ERR      -522 /*-- ERROR IN LINEAR SOLVER */
#define KN_RC_DERIV_CHECK_FAILED     -523 /*-- DERIVATIVE CHECK FAILED */
#define KN_RC_DERIV_CHECK_TERMINATE  -524 /*-- DERIVATIVE CHECK TERMINATE */
#define KN_RC_OVERFLOW_ERR           -525 /*-- INTEGER OVERFLOW ERROR */
#define KN_RC_BAD_SIZE               -526 /*-- PROBLEM DEFINITION ERROR */
#define KN_RC_BAD_VARIABLE           -527 /*-- PROBLEM DEFINITION ERROR */
#define KN_RC_BAD_VAR_INDEX          -528 /*-- PROBLEM DEFINITION ERROR */
#define KN_RC_BAD_OBJECTIVE          -529 /*-- PROBLEM DEFINITION ERROR */
#define KN_RC_BAD_OBJ_INDEX          -530 /*-- PROBLEM DEFINITION ERROR */
#define KN_RC_BAD_RESIDUAL           -531 /*-- PROBLEM DEFINITION ERROR */
#define KN_RC_BAD_RSD_INDEX          -532 /*-- PROBLEM DEFINITION ERROR */
#define KN_RC_INTERNAL_ERROR         -600 /*-- CONTACT ARTELYS SUPPORT */

/** Parameter IDs used in functions KN_get_xxx_param and KN_set_xxx_param.
 *  In some cases, parameter values are defined underneath the parameter ID.
 */
#define KN_PARAM_NEWPOINT             1001
#  define KN_NEWPOINT_NONE               0
#  define KN_NEWPOINT_SAVEONE            1
#  define KN_NEWPOINT_SAVEALL            2
#define KN_PARAM_HONORBNDS            1002
#  define KN_HONORBNDS_AUTO             -1
#  define KN_HONORBNDS_NO                0
#  define KN_HONORBNDS_ALWAYS            1
#  define KN_HONORBNDS_INITPT            2
#define KN_PARAM_NLP_ALGORITHM        1003
#  define KN_NLP_ALG_AUTOMATIC           0
#  define KN_NLP_ALG_AUTO                0
#  define KN_NLP_ALG_BAR_DIRECT          1
#  define KN_NLP_ALG_BAR_CG              2
#  define KN_NLP_ALG_ACT_CG              3
#  define KN_NLP_ALG_ACT_SQP             4
#  define KN_NLP_ALG_MULTI               5
#  define KN_NLP_ALG_AL                  6
#define KN_PARAM_ALGORITHM            1003 /** PRE-15.0 OPTION NAME */
#define KN_PARAM_ALG                  1003
#  define KN_ALG_AUTOMATIC               0
#  define KN_ALG_AUTO                    0
#  define KN_ALG_BAR_DIRECT              1
#  define KN_ALG_BAR_CG                  2
#  define KN_ALG_ACT_CG                  3
#  define KN_ALG_ACT_SQP                 4
#  define KN_ALG_MULTI                   5
#  define KN_ALG_AL                      6
#define KN_PARAM_BAR_MURULE           1004
#  define KN_BAR_MURULE_AUTOMATIC        0
#  define KN_BAR_MURULE_AUTO             0
#  define KN_BAR_MURULE_MONOTONE         1
#  define KN_BAR_MURULE_ADAPTIVE         2
#  define KN_BAR_MURULE_PROBING          3
#  define KN_BAR_MURULE_DAMPMPC          4
#  define KN_BAR_MURULE_FULLMPC          5
#  define KN_BAR_MURULE_QUALITY          6
#define KN_PARAM_BAR_FEASIBLE         1006
#  define KN_BAR_FEASIBLE_NO             0
#  define KN_BAR_FEASIBLE_STAY           1
#  define KN_BAR_FEASIBLE_GET            2
#  define KN_BAR_FEASIBLE_GET_STAY       3
#define KN_PARAM_GRADOPT              1007
#  define KN_GRADOPT_EXACT               1
#  define KN_GRADOPT_FORWARD             2
#  define KN_GRADOPT_CENTRAL             3
#  define KN_GRADOPT_USER_FORWARD        4
#  define KN_GRADOPT_USER_CENTRAL        5
#define KN_PARAM_HESSOPT              1008
#  define KN_HESSOPT_AUTO                0
#  define KN_HESSOPT_EXACT               1
#  define KN_HESSOPT_BFGS                2
#  define KN_HESSOPT_SR1                 3
#  define KN_HESSOPT_PRODUCT_FINDIFF     4
#  define KN_HESSOPT_PRODUCT             5
#  define KN_HESSOPT_LBFGS               6
#  define KN_HESSOPT_GAUSS_NEWTON        7
#define KN_PARAM_BAR_INITPT           1009
#  define KN_BAR_INITPT_AUTO             0
#  define KN_BAR_INITPT_CONVEX           1
#  define KN_BAR_INITPT_NEARBND          2
#  define KN_BAR_INITPT_CENTRAL          3
#define KN_PARAM_ACT_LPSOLVER         1012
#  define KN_ACT_LPSOLVER_INTERNAL       1
#  define KN_ACT_LPSOLVER_CPLEX          2
#  define KN_ACT_LPSOLVER_XPRESS         3
#define KN_PARAM_CG_MAXIT             1013
#define KN_PARAM_MAXIT                1014
#define KN_PARAM_OUTLEV               1015
#  define KN_OUTLEV_NONE                 0
#  define KN_OUTLEV_SUMMARY              1
#  define KN_OUTLEV_ITER_10              2
#  define KN_OUTLEV_ITER                 3
#  define KN_OUTLEV_ITER_VERBOSE         4
#  define KN_OUTLEV_ITER_X               5
#  define KN_OUTLEV_ALL                  6
#define KN_PARAM_OUTMODE              1016
#  define KN_OUTMODE_SCREEN              0
#  define KN_OUTMODE_FILE                1
#  define KN_OUTMODE_BOTH                2
#define KN_PARAM_SCALE                1017
#  define KN_SCALE_NEVER                 0
#  define KN_SCALE_NO                    0
#  define KN_SCALE_USER_INTERNAL         1
#  define KN_SCALE_USER_NONE             2
#  define KN_SCALE_INTERNAL              3
#define KN_PARAM_SOC                  1019
#  define KN_SOC_NO                      0
#  define KN_SOC_MAYBE                   1
#  define KN_SOC_YES                     2
#define KN_PARAM_DELTA                1020
#define KN_PARAM_BAR_FEASMODETOL      1021
#define KN_PARAM_FEASTOL              1022
#define KN_PARAM_FEASTOLABS           1023
#define KN_PARAM_MAXTIMECPU           1024
#define KN_PARAM_BAR_INITMU           1025
#define KN_PARAM_OBJRANGE             1026
#define KN_PARAM_OPTTOL               1027
#define KN_PARAM_OPTTOLABS            1028
#define KN_PARAM_LINSOLVER_PIVOTTOL   1029
#define KN_PARAM_XTOL                 1030
#define KN_PARAM_DEBUG                1031
#  define KN_DEBUG_NONE                  0
#  define KN_DEBUG_PROBLEM               1
#  define KN_DEBUG_EXECUTION             2
#define KN_PARAM_MULTISTART           1033
#define KN_PARAM_MSENABLE             1033
#define KN_PARAM_MS_ENABLE            1033
#  define KN_MULTISTART_NO               0
#  define KN_MS_ENABLE_NO                0
#  define KN_MULTISTART_YES              1
#  define KN_MS_ENABLE_YES               1
#define KN_PARAM_MSMAXSOLVES          1034
#define KN_PARAM_MS_MAXSOLVES         1034
#define KN_PARAM_MSMAXBNDRANGE        1035
#define KN_PARAM_MS_MAXBNDRANGE       1035
#define KN_PARAM_MSMAXTIMECPU         1036
#define KN_PARAM_MS_MAXTIMECPU        1036
#define KN_PARAM_MSMAXTIMEREAL        1037
#define KN_PARAM_MS_MAXTIMEREAL       1037
#define KN_PARAM_LMSIZE               1038
#define KN_PARAM_BAR_MAXCROSSIT       1039
#define KN_PARAM_MAXTIMEREAL          1040
#define KN_PARAM_CG_PRECOND           1041
#  define KN_CG_PRECOND_NONE             0
#  define KN_CG_PRECOND_CHOL             1
#define KN_PARAM_BLASOPTION           1042
#  define KN_BLASOPTION_AUTO            -1
#  define KN_BLASOPTION_KNITRO           0
#  define KN_BLASOPTION_INTEL            1
#  define KN_BLASOPTION_DYNAMIC          2
#  define KN_BLASOPTION_BLIS             3
#  define KN_BLASOPTION_APPLE            4
#define KN_PARAM_BAR_MAXREFACTOR      1043
#define KN_PARAM_LINESEARCH_MAXTRIALS 1044
#define KN_PARAM_BLASOPTIONLIB        1045
#define KN_PARAM_OUTAPPEND            1046
#  define KN_OUTAPPEND_NO                0
#  define KN_OUTAPPEND_YES               1
#define KN_PARAM_OUTDIR               1047
#define KN_PARAM_CPLEXLIB             1048
#define KN_PARAM_BAR_PENRULE          1049
#  define KN_BAR_PENRULE_AUTO            0
#  define KN_BAR_PENRULE_SINGLE          1
#  define KN_BAR_PENRULE_FLEX            2
#define KN_PARAM_BAR_PENCONS          1050
#  define KN_BAR_PENCONS_AUTO           -1
#  define KN_BAR_PENCONS_NONE            0
#  define KN_BAR_PENCONS_ALL             2
#  define KN_BAR_PENCONS_EQUALITIES      3
#  define KN_BAR_PENCONS_INFEAS          4 /*-- DEPRECATED */
#define KN_PARAM_MSNUMTOSAVE          1051
#define KN_PARAM_MS_NUMTOSAVE         1051
#define KN_PARAM_MSSAVETOL            1052
#define KN_PARAM_MS_SAVETOL           1052
#define KN_PARAM_PRESOLVEDEBUG        1053 /*-- FOR AMPL ONLY */
#  define KN_PRESOLVEDBG_NONE            0
#  define KN_PRESOLVEDBG_BASIC           1
#  define KN_PRESOLVEDBG_VERBOSE         2
#  define KN_PRESOLVEDBG_DETAIL          3
#define KN_PARAM_MSTERMINATE          1054
#define KN_PARAM_MS_TERMINATE         1054
#  define KN_MSTERMINATE_MAXSOLVES       0
#  define KN_MS_TERMINATE_MAXSOLVES      0
#  define KN_MSTERMINATE_OPTIMAL         1
#  define KN_MS_TERMINATE_OPTIMAL        1
#  define KN_MSTERMINATE_FEASIBLE        2
#  define KN_MS_TERMINATE_FEASIBLE       2
#  define KN_MSTERMINATE_ANY             3
#  define KN_MS_TERMINATE_ANY            3
#  define KN_MSTERMINATE_RULEBASED       4
#  define KN_MS_TERMINATE_RULEBASED      4
#define KN_PARAM_MSSTARTPTRANGE       1055
#define KN_PARAM_MS_STARTPTRANGE      1055
#define KN_PARAM_INFEASTOL            1056
#define KN_PARAM_LINSOLVER            1057
#  define KN_LINSOLVER_AUTO              0
#  define KN_LINSOLVER_INTERNAL          1
#  define KN_LINSOLVER_HYBRID            2
#  define KN_LINSOLVER_DENSEQR           3
#  define KN_LINSOLVER_MA27              4
#  define KN_LINSOLVER_MA57              5
#  define KN_LINSOLVER_MKLPARDISO        6
#  define KN_LINSOLVER_MA97              7
#  define KN_LINSOLVER_MA86              8
#  define KN_LINSOLVER_APPLE             9
#define KN_PARAM_BAR_DIRECTINTERVAL   1058
#define KN_PARAM_PRESOLVE             1059
#  define KN_PRESOLVE_NO                 0
#  define KN_PRESOLVE_NONE               0 /*-- DEPRECATED */
#  define KN_PRESOLVE_YES                1
#  define KN_PRESOLVE_BASIC              1 /*-- DEPRECATED */
#  define KN_PRESOLVE_ADVANCED           2 /*-- DEPRECATED */
#define KN_PARAM_PRESOLVE_TOL         1060
#define KN_PARAM_BAR_SWITCHRULE       1061
#  define KN_BAR_SWITCHRULE_AUTO        -1
#  define KN_BAR_SWITCHRULE_NEVER        0
#  define KN_BAR_SWITCHRULE_MODERATE     2
#  define KN_BAR_SWITCHRULE_AGGRESSIVE   3
#define KN_PARAM_HESSIAN_NO_F         1062
#  define KN_HESSIAN_NO_F_FORBID         0
#  define KN_HESSIAN_NO_F_ALLOW          1
#define KN_PARAM_MA_TERMINATE         1063
#  define KN_MA_TERMINATE_ALL            0
#  define KN_MA_TERMINATE_OPTIMAL        1
#  define KN_MA_TERMINATE_FEASIBLE       2
#  define KN_MA_TERMINATE_ANY            3
#define KN_PARAM_MA_MAXTIMECPU        1064
#define KN_PARAM_MA_MAXTIMEREAL       1065
#define KN_PARAM_MSSEED               1066
#define KN_PARAM_MS_SEED              1066
#define KN_PARAM_MA_OUTSUB            1067
#  define KN_MA_OUTSUB_NONE              0
#  define KN_MA_OUTSUB_YES               1
#define KN_PARAM_MS_OUTSUB            1068
#  define KN_MS_OUTSUB_NONE              0
#  define KN_MS_OUTSUB_YES               1
#define KN_PARAM_XPRESSLIB            1069
#define KN_PARAM_TUNER                1070
#  define KN_TUNER_OFF                   0
#  define KN_TUNER_ON                    1
#define KN_PARAM_TUNER_OPTIONSFILE    1071
#define KN_PARAM_TUNER_MAXTIMECPU     1072
#define KN_PARAM_TUNER_MAXTIMEREAL    1073
#define KN_PARAM_TUNER_OUTSUB         1074
#  define KN_TUNER_OUTSUB_NONE           0
#  define KN_TUNER_OUTSUB_SUMMARY        1
#  define KN_TUNER_OUTSUB_ALL            2
#define KN_PARAM_TUNER_TERMINATE      1075
#  define KN_TUNER_TERMINATE_ALL         0
#  define KN_TUNER_TERMINATE_OPTIMAL     1
#  define KN_TUNER_TERMINATE_FEASIBLE    2
#  define KN_TUNER_TERMINATE_ANY         3
#define KN_PARAM_LINSOLVER_OOC        1076
#  define KN_LINSOLVER_OOC_NO            0
#  define KN_LINSOLVER_OOC_MAYBE         1
#  define KN_LINSOLVER_OOC_YES           2
#define KN_PARAM_BAR_RELAXCONS        1077
#  define KN_BAR_RELAXCONS_NONE          0
#  define KN_BAR_RELAXCONS_EQS           1
#  define KN_BAR_RELAXCONS_INEQS         2
#  define KN_BAR_RELAXCONS_ALL           3
#define KN_PARAM_MSDETERMINISTIC      1078
#define KN_PARAM_MS_DETERMINISTIC     1078
#  define KN_MSDETERMINISTIC_NO          0
#  define KN_MS_DETERMINISTIC_NO         0
#  define KN_MSDETERMINISTIC_YES         1
#  define KN_MS_DETERMINISTIC_YES        1
#define KN_PARAM_BAR_REFINEMENT       1079
#  define KN_BAR_REFINEMENT_NO           0
#  define KN_BAR_REFINEMENT_YES          1
#define KN_PARAM_DERIVCHECK           1080
#  define KN_DERIVCHECK_NONE             0
#  define KN_DERIVCHECK_FIRST            1
#  define KN_DERIVCHECK_SECOND           2
#  define KN_DERIVCHECK_ALL              3
#define KN_PARAM_DERIVCHECK_TYPE      1081
#  define KN_DERIVCHECK_FORWARD          1
#  define KN_DERIVCHECK_CENTRAL          2
#define KN_PARAM_DERIVCHECK_TOL       1082
#define KN_PARAM_LINSOLVER_INEXACT    1083 /*-- UNDOCUMENTED/EXPERIMENTAL */
#  define KN_LINSOLVER_INEXACT_NO        0
#  define KN_LINSOLVER_INEXACT_YES       1
#define KN_PARAM_LINSOLVER_INEXACTTOL 1084 /*-- UNDOCUMENTED/EXPERIMENTAL */
#define KN_PARAM_MAXFEVALS            1085
#define KN_PARAM_FSTOPVAL             1086
#define KN_PARAM_DATACHECK            1087
#  define KN_DATACHECK_NO                0
#  define KN_DATACHECK_YES               1
#define KN_PARAM_DERIVCHECK_TERMINATE 1088
#  define KN_DERIVCHECK_STOPERROR        1
#  define KN_DERIVCHECK_STOPALWAYS       2
#define KN_PARAM_BAR_WATCHDOG         1089
#  define KN_BAR_WATCHDOG_NO             0
#  define KN_BAR_WATCHDOG_YES            1
#define KN_PARAM_FTOL                 1090
#define KN_PARAM_FTOL_ITERS           1091
#define KN_PARAM_ACT_QPALG            1092
#  define KN_ACT_QPALG_AUTO              0
#  define KN_ACT_QPALG_BAR_DIRECT        1
#  define KN_ACT_QPALG_BAR_CG            2
#  define KN_ACT_QPALG_ACT_CG            3
#define KN_PARAM_BAR_INITPI_MPEC      1093
#define KN_PARAM_XTOL_ITERS           1094
#define KN_PARAM_LINESEARCH           1095
#  define KN_LINESEARCH_AUTO             0
#  define KN_LINESEARCH_BACKTRACK        1
#  define KN_LINESEARCH_INTERPOLATE      2
#  define KN_LINESEARCH_WEAKWOLFE        3
#define KN_PARAM_OUT_CSVINFO          1096
#  define KN_OUT_CSVINFO_NO              0
#  define KN_OUT_CSVINFO_YES             1
#define KN_PARAM_INITPENALTY          1097
#define KN_PARAM_ACT_LPFEASTOL        1098
#define KN_PARAM_CG_STOPTOL           1099
#define KN_PARAM_RESTARTS             1100
#define KN_PARAM_RESTARTS_MAXIT       1101
#define KN_PARAM_BAR_SLACKBOUNDPUSH   1102
#define KN_PARAM_CG_PMEM              1103
#define KN_PARAM_BAR_SWITCHOBJ        1104
#  define KN_BAR_SWITCHOBJ_NONE          0
#  define KN_BAR_SWITCHOBJ_SCALARPROX    1
#  define KN_BAR_SWITCHOBJ_DIAGPROX      2
#define KN_PARAM_OUTNAME              1105
#define KN_PARAM_OUT_CSVNAME          1106
#define KN_PARAM_ACT_PARAMETRIC       1107
#  define KN_ACT_PARAMETRIC_NO           0
#  define KN_ACT_PARAMETRIC_MAYBE        1
#  define KN_ACT_PARAMETRIC_YES          2
#define KN_PARAM_ACT_LPDUMPMPS        1108
#  define KN_ACT_LPDUMPMPS_NO            0
#  define KN_ACT_LPDUMPMPS_YES           1
#define KN_PARAM_ACT_LPALG            1109
#  define KN_ACT_LPALG_DEFAULT           0
#  define KN_ACT_LPALG_PRIMAL            1
#  define KN_ACT_LPALG_PRIMALSIMPLEX     1
#  define KN_ACT_LPALG_DUAL              2
#  define KN_ACT_LPALG_DUALSIMPLEX       2
#  define KN_ACT_LPALG_BARRIER           3
#define KN_PARAM_ACT_LPPRESOLVE       1110
#  define KN_ACT_LPPRESOLVE_OFF          0
#  define KN_ACT_LPPRESOLVE_ON           1
#define KN_PARAM_ACT_LPPENALTY        1111
#  define KN_ACT_LPPENALTY_ALL           1
#  define KN_ACT_LPPENALTY_NONLINEAR     2
#  define KN_ACT_LPPENALTY_DYNAMIC       3
#define KN_PARAM_BNDRANGE             1112
#define KN_PARAM_BAR_CONIC_ENABLE     1113
#  define KN_BAR_CONIC_ENABLE_AUTO      -1
#  define KN_BAR_CONIC_ENABLE_NONE       0
#  define KN_BAR_CONIC_ENABLE_SOC        1
#define KN_PARAM_CONVEX               1114
#  define KN_CONVEX_AUTO                -1
#  define KN_CONVEX_NO                   0
#  define KN_CONVEX_YES                  1
#define KN_PARAM_OUT_HINTS            1115
#  define KN_OUT_HINTS_NO                0
#  define KN_OUT_HINTS_YES               1
#define KN_PARAM_EVAL_FCGA            1116
#  define KN_EVAL_FCGA_NO                0
#  define KN_EVAL_FCGA_YES               1
#define KN_PARAM_BAR_MAXCORRECTORS    1117
#define KN_PARAM_STRAT_WARM_START     1118
#  define KN_STRAT_WARM_START_NO         0
#  define KN_STRAT_WARM_START_YES        1
#define KN_PARAM_FINDIFF_TERMINATE    1119
#  define KN_FINDIFF_TERMINATE_NONE      0
#  define KN_FINDIFF_TERMINATE_ERREST    1
#define KN_PARAM_CPUPLATFORM          1120
#  define KN_CPUPLATFORM_AUTO           -1
#  define KN_CPUPLATFORM_COMPATIBLE      1
#  define KN_CPUPLATFORM_SSE2            2
#  define KN_CPUPLATFORM_AVX             3
#  define KN_CPUPLATFORM_AVX2            4
#  define KN_CPUPLATFORM_AVX512          5 /*-- EXPERIMENTAL */
#define KN_PARAM_PRESOLVE_PASSES      1121
#define KN_PARAM_PRESOLVE_LEVEL       1122
#  define KN_PRESOLVE_LEVEL_AUTO        -1
#  define KN_PRESOLVE_LEVEL_1            1
#  define KN_PRESOLVE_LEVEL_2            2
#define KN_PARAM_FINDIFF_RELSTEPSIZE  1123
#define KN_PARAM_INFEASTOL_ITERS      1124
#define KN_PARAM_PRESOLVEOP_TIGHTEN   1125
#  define KN_PRESOLVEOP_TIGHTEN_AUTO    -1
#  define KN_PRESOLVEOP_TIGHTEN_NONE     0
#  define KN_PRESOLVEOP_TIGHTEN_VARBND   1
#  define KN_PRESOLVEOP_TIGHTEN_COEF     2 /*-- DEPRECATED */
#  define KN_PRESOLVEOP_TIGHTEN_ALL      3 /*-- DEPRECATED */
#define KN_PARAM_BAR_LINSYS           1126
#  define KN_BAR_LINSYS_AUTO            -1
#  define KN_BAR_LINSYS_FULL             0
#  define KN_BAR_LINSYS_COMPACT1         1 /*-- DEPRECATED */
#  define KN_BAR_LINSYS_ELIMINATE_SLACKS 1
#  define KN_BAR_LINSYS_COMPACT2         2 /*-- DEPRECATED */
#  define KN_BAR_LINSYS_ELIMINATE_BOUNDS 2
#  define KN_BAR_LINSYS_ELIMINATE_INEQS  3
#define KN_PARAM_PRESOLVE_INITPT      1127
#  define KN_PRESOLVE_INITPT_AUTO       -1
#  define KN_PRESOLVE_INITPT_NOSHIFT     0
#  define KN_PRESOLVE_INITPT_LINSHIFT    1
#  define KN_PRESOLVE_INITPT_ANYSHIFT    2
#define KN_PARAM_ACT_QPPENALTY        1128
#  define KN_ACT_QPPENALTY_AUTO         -1
#  define KN_ACT_QPPENALTY_NONE          0
#  define KN_ACT_QPPENALTY_ALL           1
#define KN_PARAM_BAR_LINSYS_STORAGE   1129
#  define KN_BAR_LINSYS_STORAGE_AUTO    -1
#  define KN_BAR_LINSYS_STORAGE_LOWMEM   1
#  define KN_BAR_LINSYS_STORAGE_NORMAL   2
#define KN_PARAM_LINSOLVER_MAXITREF   1130
#define KN_PARAM_BFGS_SCALING         1131
#  define KN_BFGS_SCALING_DYNAMIC        0
#  define KN_BFGS_SCALING_INVHESS        1
#  define KN_BFGS_SCALING_HESS           2
#define KN_PARAM_BAR_INITSHIFTTOL     1132
#define KN_PARAM_NUMTHREADS           1133
#define KN_PARAM_CONCURRENT_EVALS     1134
#  define KN_CONCURRENT_EVALS_NO         0
#  define KN_CONCURRENT_EVALS_YES        1
#define KN_PARAM_BLAS_NUMTHREADS      1135
#define KN_PARAM_LINSOLVER_NUMTHREADS 1136
#define KN_PARAM_MS_NUMTHREADS        1137
#define KN_PARAM_CONIC_NUMTHREADS     1138
#define KN_PARAM_NCVX_QCQP_INIT       1139
#  define KN_NCVX_QCQP_INIT_AUTO        -1
#  define KN_NCVX_QCQP_INIT_NONE         0
#  define KN_NCVX_QCQP_INIT_LINEAR       1
#  define KN_NCVX_QCQP_INIT_HYBRID       2
#  define KN_NCVX_QCQP_INIT_PENALTY      3
#  define KN_NCVX_QCQP_INIT_CVXQUAD      4
#define KN_PARAM_FINDIFF_ESTNOISE     1140
#  define KN_FINDIFF_ESTNOISE_NO         0
#  define KN_FINDIFF_ESTNOISE_YES        1
#  define KN_FINDIFF_ESTNOISE_WITHCURV   2
#define KN_PARAM_FINDIFF_NUMTHREADS   1141
#define KN_PARAM_BAR_MPEC_HEURISTIC   1142
#  define KN_BAR_MPEC_HEURISTIC_NO       0
#  define KN_BAR_MPEC_HEURISTIC_YES      1
#define KN_PARAM_PRESOLVEOP_REDUNDANT 1143
#  define KN_PRESOLVEOP_REDUNDANT_NONE   0
#  define KN_PRESOLVEOP_REDUNDANT_DUPCON 1
#  define KN_PRESOLVEOP_REDUNDANT_DEPCON 2
#define KN_PARAM_LINSOLVER_ORDERING   1144
#  define KN_LINSOLVER_ORDERING_AUTO    -1
#  define KN_LINSOLVER_ORDERING_BEST     0
#  define KN_LINSOLVER_ORDERING_AMD      1
#  define KN_LINSOLVER_ORDERING_METIS    2
#define KN_PARAM_LINSOLVER_NODEAMALG  1145
#define KN_PARAM_PRESOLVEOP_SUBSTITUTION 1146
#  define KN_PRESOLVEOP_SUBSTITUTION_AUTO  -1
#  define KN_PRESOLVEOP_SUBSTITUTION_NONE   0
#  define KN_PRESOLVEOP_SUBSTITUTION_SIMPLE 1
#  define KN_PRESOLVEOP_SUBSTITUTION_ALL    2
#define KN_PARAM_PRESOLVEOP_SUBSTITUTION_TOL 1147
#define KN_PARAM_MS_INITPT_CLUSTER    1149
#  define KN_MS_INITPT_CLUSTER_NONE      0
#  define KN_MS_INITPT_CLUSTER_SL        1
#define KN_PARAM_SCALE_VARS           1153
#  define KN_SCALE_VARS_NONE             0
#  define KN_SCALE_VARS_BNDS             1
#define KN_PARAM_BAR_MAXMU            1154
#define KN_PARAM_BAR_GLOBALIZE        1155
#  define KN_BAR_GLOBALIZE_NONE          0
#  define KN_BAR_GLOBALIZE_KKT           1
#  define KN_BAR_GLOBALIZE_FILTER        2
#define KN_PARAM_LINSOLVER_SCALING    1156
#  define KN_LINSOLVER_SCALING_NONE      0
#  define KN_LINSOLVER_SCALING_ALWAYS    1
#  define KN_LINSOLVER_SCALING_DYNAMIC   2
#define KN_PARAM_INITPT_STRATEGY      1158
#  define KN_INITPT_STRATEGY_AUTO       -1
#  define KN_INITPT_STRATEGY_BASIC       1
#  define KN_INITPT_STRATEGY_ADVANCED    2
#define KN_PARAM_EVAL_COST            1159
#  define KN_EVAL_COST_UNSPECIFIED       0
#  define KN_EVAL_COST_INEXPENSIVE       1
#  define KN_EVAL_COST_EXPENSIVE         2
#define KN_PARAM_MS_TERMINATERULE_TOL 1160
#define KN_PARAM_SOLTYPE              1161
#  define KN_SOLTYPE_FINAL               0
#  define KN_SOLTYPE_BESTFEAS            1
#define KN_PARAM_MAXTIME              1163
#define KN_PARAM_MA_SUB_MAXTIME       1164
#define KN_PARAM_MS_SUB_MAXTIME       1165
#define KN_PARAM_TUNER_SUB_MAXTIME    1166
#define KN_PARAM_INITPTFILE           1167
#define KN_PARAM_LP_ALGORITHM         1170
#define KN_PARAM_LP_ALG               1170
#  define KN_LP_ALG_AUTO                -1
#  define KN_LP_ALG_NLPALGORITHM         0
#  define KN_LP_ALG_PRIMALSIMPLEX        1
#  define KN_LP_ALG_DUALSIMPLEX          2
#  define KN_LP_ALG_BARRIER              3
#  define KN_LP_ALG_PDLP                 4
#define KN_PARAM_AL_INITPENALTY       1171
#define KN_PARAM_AL_MAXPENALTY        1172
#define KN_PARAM_PRESOLVEOP_PROBING   1174
#  define KN_PRESOLVEOP_PROBING_AUTO    -1
#  define KN_PRESOLVEOP_PROBING_OFF      0
#  define KN_PRESOLVEOP_PROBING_LIGHT    1
#  define KN_PRESOLVEOP_PROBING_FULL     2
#define KN_PARAM_PRESOLVEOP_CLIQUE_MERGING  1176
#  define KN_PRESOLVEOP_CLIQUE_MERGING_AUTO   -1
#  define KN_PRESOLVEOP_CLIQUE_MERGING_OFF     0
#  define KN_PRESOLVEOP_CLIQUE_MERGING_ON      1

/* ----- MIP-specific parameters ----- */
#define KN_PARAM_MIP_METHOD           2001
#  define KN_MIP_METHOD_AUTO             0
#  define KN_MIP_METHOD_BB               1
#  define KN_MIP_METHOD_HQG              2
#  define KN_MIP_METHOD_MISQP            3
#define KN_PARAM_MIP_BRANCHRULE       2002
#  define KN_MIP_BRANCH_AUTO             0
#  define KN_MIP_BRANCH_MOSTFRAC         1
#  define KN_MIP_BRANCH_PSEUDOCOST       2
#  define KN_MIP_BRANCH_STRONG           3
#define KN_PARAM_MIP_SELECTRULE       2003
#  define KN_MIP_SEL_AUTO                0
#  define KN_MIP_SEL_DEPTHFIRST          1
#  define KN_MIP_SEL_BESTBOUND           2
#  define KN_MIP_SEL_COMBO_1             3
#define KN_PARAM_MIP_INTGAPABS        2004 /*-- DEPRECATED */
#define KN_PARAM_MIP_OPTGAPABS        2004
#define KN_PARAM_MIP_INTGAPREL        2005 /*-- DEPRECATED */
#define KN_PARAM_MIP_OPTGAPREL        2005
#define KN_PARAM_MIP_MAXTIMECPU       2006
#define KN_PARAM_MIP_MAXTIMEREAL      2007
#define KN_PARAM_MIP_MAXSOLVES        2008
#define KN_PARAM_MIP_INTEGERTOL       2009
#define KN_PARAM_MIP_OUTLEVEL         2010
#  define KN_MIP_OUTLEVEL_NONE           0
#  define KN_MIP_OUTLEVEL_ITERS          1
#  define KN_MIP_OUTLEVEL_ITERSTIME      2
#  define KN_MIP_OUTLEVEL_ROOT           3
#define KN_PARAM_MIP_OUTINTERVAL      2011
#define KN_PARAM_MIP_OUTSUB           2012
#  define KN_MIP_OUTSUB_NONE             0
#  define KN_MIP_OUTSUB_YES              1
#  define KN_MIP_OUTSUB_YESPROB          2
#define KN_PARAM_MIP_DEBUG            2013
#  define KN_MIP_DEBUG_NONE              0
#  define KN_MIP_DEBUG_ALL               1
#define KN_PARAM_MIP_IMPLICATNS       2014  /*-- USE LOGICAL IMPLICATIONS */
#define KN_PARAM_MIP_IMPLICATIONS     2014
#  define KN_MIP_IMPLICATNS_NO           0
#  define KN_MIP_IMPLICATIONS_NO         0
#  define KN_MIP_IMPLICATNS_YES          1
#  define KN_MIP_IMPLICATIONS_YES        1
#define KN_PARAM_MIP_GUB_BRANCH       2015  /*-- BRANCH ON GENERALIZED BOUNDS */
#  define KN_MIP_GUB_BRANCH_NO           0
#  define KN_MIP_GUB_BRANCH_YES          1
#define KN_PARAM_MIP_KNAPSACK         2016  /*-- KNAPSACK CUTS */
#  define KN_MIP_KNAPSACK_AUTO          -1
#  define KN_MIP_KNAPSACK_NO             0
#  define KN_MIP_KNAPSACK_NONE           0  /*--   NONE */
#  define KN_MIP_KNAPSACK_ROOT           1  /*--   IN THE ROOT */
#  define KN_MIP_KNAPSACK_TREE           2  /*--   IN THE WHOLE TREE */
#  define KN_MIP_KNAPSACK_INEQ           1  /*--   DEPRECATED */
#  define KN_MIP_KNAPSACK_LIFTED         2  /*--   DEPRECATED */
#  define KN_MIP_KNAPSACK_ALL            3  /*--   DEPRECATED */
#define KN_PARAM_MIP_ROUNDING         2017
#  define KN_MIP_ROUND_AUTO             -1
#  define KN_MIP_ROUND_NONE              0  /*-- DO NOT ATTEMPT ROUNDING */
#  define KN_MIP_ROUND_HEURISTIC         2  /*-- USE FAST HEURISTIC */
#  define KN_MIP_ROUND_NLP_SOME          3  /*-- SOLVE NLP IF LIKELY TO WORK */
#  define KN_MIP_ROUND_NLP_ALWAYS        4  /*-- SOLVE NLP ALWAYS */
#define KN_PARAM_MIP_ROOT_NLPALG      2018
#define KN_PARAM_MIP_ROOTALG          2018  /*-- DEPRECATED */
#  define KN_MIP_ROOT_NLPALG_AUTO        0
#  define KN_MIP_ROOTALG_AUTO            0  /*-- DEPRECATED */
#  define KN_MIP_ROOT_NLPALG_BAR_DIRECT  1
#  define KN_MIP_ROOTALG_BAR_DIRECT      1  /*-- DEPRECATED */
#  define KN_MIP_ROOT_NLPALG_BAR_CG      2
#  define KN_MIP_ROOTALG_BAR_CG          2  /*-- DEPRECATED */
#  define KN_MIP_ROOT_NLPALG_ACT_CG      3
#  define KN_MIP_ROOTALG_ACT_CG          3  /*-- DEPRECATED */
#  define KN_MIP_ROOT_NLPALG_ACT_SQP     4
#  define KN_MIP_ROOTALG_ACT_SQP         4  /*-- DEPRECATED */
#  define KN_MIP_ROOTALG_MULTI           5  /*-- DEPRECATED */
#define KN_PARAM_MIP_LPALG            2019  /*-- DEPRECATED: USE */
#  define KN_MIP_LPALG_AUTO              0  /*-- KN_PARAM_MIP_ROOT_LPALG OR */
#  define KN_MIP_LPALG_BAR_DIRECT        1  /*-- KN_PARAM_MIP_NODE_LPALG */
#  define KN_MIP_LPALG_BAR_CG            2
#  define KN_MIP_LPALG_ACT_CG            3
#define KN_PARAM_MIP_TERMINATE        2020
#  define KN_MIP_TERMINATE_OPTIMAL       0
#  define KN_MIP_TERMINATE_FEASIBLE      1
#define KN_PARAM_MIP_MAXNODES         2021
#define KN_PARAM_MIP_HEURISTIC        2022 /*-- DEPRECATED */
#define KN_PARAM_MIP_HEUR_MAXIT       2023
#define KN_PARAM_MIP_HEUR_MAXTIMECPU  2024  /*-- INACTIVE */
#define KN_PARAM_MIP_HEUR_MAXTIMEREAL 2025  /*-- INACTIVE */
#define KN_PARAM_MIP_PSEUDOINIT       2026
#  define KN_MIP_PSEUDOINIT_AUTO         0
#  define KN_MIP_PSEUDOINIT_AVE          1
#  define KN_MIP_PSEUDOINIT_STRONG       2
#define KN_PARAM_MIP_STRONG_MAXIT     2027
#define KN_PARAM_MIP_STRONG_CANDLIM   2028
#define KN_PARAM_MIP_STRONG_LEVEL     2029
#define KN_PARAM_MIP_INTVAR_STRATEGY  2030
#  define KN_MIP_INTVAR_STRATEGY_NONE    0
#  define KN_MIP_INTVAR_STRATEGY_RELAX   1
#  define KN_MIP_INTVAR_STRATEGY_MPEC    2
#define KN_PARAM_MIP_RELAXABLE        2031
#  define KN_MIP_RELAXABLE_NONE          0
#  define KN_MIP_RELAXABLE_ALL           1
#define KN_PARAM_MIP_NODE_NLPALG      2032
#define KN_PARAM_MIP_NODEALG          2032  /*-- DEPRECATED */
#  define KN_MIP_NODE_NLPALG_AUTO        0
#  define KN_MIP_NODEALG_AUTO            0  /*-- DEPRECATED */
#  define KN_MIP_NODE_NLPALG_BAR_DIRECT  1
#  define KN_MIP_NODEALG_BAR_DIRECT      1  /*-- DEPRECATED */
#  define KN_MIP_NODE_NLPALG_BAR_CG      2
#  define KN_MIP_NODEALG_BAR_CG          2  /*-- DEPRECATED */
#  define KN_MIP_NODE_NLPALG_ACT_CG      3
#  define KN_MIP_NODEALG_ACT_CG          3  /*-- DEPRECATED */
#  define KN_MIP_NODE_NLPALG_ACT_SQP     4
#  define KN_MIP_NODEALG_ACT_SQP         4  /*-- DEPRECATED */
#  define KN_MIP_NODEALG_MULTI           5  /*-- DEPRECATED */
#define KN_PARAM_MIP_HEUR_TERMINATE   2033
#  define KN_MIP_HEUR_TERMINATE_FEASIBLE 1
#  define KN_MIP_HEUR_TERMINATE_LIMIT    2
#define KN_PARAM_MIP_SELECTDIR        2034
#  define KN_MIP_SELECTDIR_DOWN          0
#  define KN_MIP_SELECTDIR_UP            1
#define KN_PARAM_MIP_CUTFACTOR        2035
#define KN_PARAM_MIP_ZEROHALF         2036  /*-- ZEROHALF CUTS */
#  define KN_MIP_ZEROHALF_AUTO          -1
#  define KN_MIP_ZEROHALF_NONE           0  /*--   NONE */
#  define KN_MIP_ZEROHALF_ROOT           1  /*--   IN THE ROOT */
#  define KN_MIP_ZEROHALF_TREE           2  /*--   IN THE WHOLE TREE */
#  define KN_MIP_ZEROHALF_ALL            3  /*--   DEPRECATED */
#define KN_PARAM_MIP_MIR              2037  /*-- MIR CUTS */
#  define KN_MIP_MIR_AUTO               -1
#  define KN_MIP_MIR_NONE                0  /*--   NONE */
#  define KN_MIP_MIR_ROOT                1  /*--   IN THE ROOT */
#  define KN_MIP_MIR_TREE                2  /*--   IN THE WHOLE TREE */
#  define KN_MIP_MIR_NLP                 2  /*--   DEPRECATED*/
#define KN_PARAM_MIP_CLIQUE           2038  /*-- CLIQUE CUTS */
#  define KN_MIP_CLIQUE_AUTO            -1
#  define KN_MIP_CLIQUE_NONE             0  /*--   NONE */
#  define KN_MIP_CLIQUE_ROOT             1  /*--   IN THE ROOT */
#  define KN_MIP_CLIQUE_TREE             2  /*--   IN THE WHOLE TREE */
#  define KN_MIP_CLIQUE_ALL              3  /*--   DEPRECATED */
#define KN_PARAM_MIP_HEUR_STRATEGY    2039
#  define KN_MIP_HEUR_STRATEGY_AUTO     -1
#  define KN_MIP_HEUR_STRATEGY_NONE      0
#  define KN_MIP_HEUR_STRATEGY_BASIC     1
#  define KN_MIP_HEUR_STRATEGY_ADVANCED  2
#  define KN_MIP_HEUR_STRATEGY_EXTENSIVE 3
#define KN_PARAM_MIP_HEUR_FEASPUMP    2040
#  define KN_MIP_HEUR_FEASPUMP_AUTO     -1
#  define KN_MIP_HEUR_FEASPUMP_OFF       0
#  define KN_MIP_HEUR_FEASPUMP_ON        1
#define KN_PARAM_MIP_HEUR_MPEC        2041
#  define KN_MIP_HEUR_MPEC_AUTO         -1
#  define KN_MIP_HEUR_MPEC_OFF           0
#  define KN_MIP_HEUR_MPEC_ON            1
#define KN_PARAM_MIP_HEUR_DIVING      2042
#define KN_PARAM_MIP_CUTTINGPLANE     2043  /*-- CUTTING PLANE */
#  define KN_MIP_CUTTINGPLANE_NONE       0  /*--   NONE */
#  define KN_MIP_CUTTINGPLANE_ROOT       1  /*--   IN THE ROOT */
#define KN_PARAM_MIP_CUTOFF           2044
#define KN_PARAM_MIP_HEUR_LNS         2045
#define KN_PARAM_MIP_MULTISTART       2046
#  define KN_MIP_MULTISTART_OFF          0
#  define KN_MIP_MULTISTART_ON           1
#define KN_PARAM_MIP_LIFTPROJECT      2047  /*-- LIFT&PROJECT CUTS */
#  define KN_MIP_LIFTPROJECT_AUTO       -1
#  define KN_MIP_LIFTPROJECT_NONE        0  /*--   NONE */
#  define KN_MIP_LIFTPROJECT_ROOT        1  /*--   IN THE ROOT */
#define KN_PARAM_MIP_NUMTHREADS       2048
#define KN_PARAM_MIP_HEUR_MISQP       2049
#  define KN_MIP_HEUR_MISQP_AUTO        -1
#  define KN_MIP_HEUR_MISQP_OFF          0
#  define KN_MIP_HEUR_MISQP_ON           1
#define KN_PARAM_MIP_RESTART          2050
#  define KN_MIP_RESTART_OFF             0
#  define KN_MIP_RESTART_ON              1
#define KN_PARAM_MIP_GOMORY           2051  /*-- GOMORY CUTS */
#  define KN_MIP_GOMORY_AUTO            -1
#  define KN_MIP_GOMORY_NONE             0  /*--   NONE */
#  define KN_MIP_GOMORY_ROOT             1  /*--   IN THE ROOT ONLY */
#  define KN_MIP_GOMORY_TREE             2  /*--   IN THE WHOLE TREE */
#define KN_PARAM_MIP_CUT_PROBING      2052  /*-- PROBING CUTS */
#  define KN_MIP_CUT_PROBING_AUTO       -1
#  define KN_MIP_CUT_PROBING_NONE        0  /*--   NONE */
#  define KN_MIP_CUT_PROBING_ROOT        1  /*--   IN THE ROOT ONLY */
#  define KN_MIP_CUT_PROBING_TREE        2  /*--   IN THE WHOLE TREE */
#define KN_PARAM_MIP_CUT_FLOWCOVER    2053  /*-- FLOW COVER CUTS */
#  define KN_MIP_CUT_FLOWCOVER_AUTO     -1
#  define KN_MIP_CUT_FLOWCOVER_NONE      0  /*--   NONE */
#  define KN_MIP_CUT_FLOWCOVER_ROOT      1  /*--   IN THE ROOT ONLY */
#  define KN_MIP_CUT_FLOWCOVER_TREE      2  /*--   IN THE WHOLE TREE */
#define KN_PARAM_MIP_HEUR_LOCALSEARCH 2054
#  define KN_MIP_HEUR_LOCALSEARCH_AUTO  -1
#  define KN_MIP_HEUR_LOCALSEARCH_OFF    0
#  define KN_MIP_HEUR_LOCALSEARCH_ON     1
#define KN_PARAM_MIP_SUB_MAXTIME      2055
#define KN_PARAM_MIP_INITPTFILE       2056
#define KN_PARAM_MIP_ROOT_LPALG        2057
#  define KN_MIP_ROOT_LPALG_AUTO         -1
#  define KN_MIP_ROOT_LPALG_NLPALGORITHM  0
#  define KN_MIP_ROOT_LPALG_PRIMALSIMPLEX 1
#  define KN_MIP_ROOT_LPALG_DUALSIMPLEX   2
#  define KN_MIP_ROOT_LPALG_BARRIER       3
#  define KN_MIP_ROOT_LPALG_PDLP          4
#define KN_PARAM_MIP_NODE_LPALG        2058
#  define KN_MIP_NODE_LPALG_AUTO         -1
#  define KN_MIP_NODE_LPALG_NLPALGORITHM  0
#  define KN_MIP_NODE_LPALG_PRIMALSIMPLEX 1
#  define KN_MIP_NODE_LPALG_DUALSIMPLEX   2
#  define KN_MIP_NODE_LPALG_BARRIER       3
#  define KN_MIP_NODE_LPALG_PDLP          4
#define KN_PARAM_MIP_CUTOFFABS        2059
#define KN_PARAM_MIP_CUTOFFREL        2060
#define KN_PARAM_MIP_HEUR_FIXPROPAGATE 2061
#  define KN_MIP_HEUR_FIXPROPAGATE_AUTO  -1
#  define KN_MIP_HEUR_FIXPROPAGATE_OFF    0
#  define KN_MIP_HEUR_FIXPROPAGATE_ON     1

/*-- THE BELOW ARE DEPRECATED! */
#define KN_PARAM_PAR_NUMTHREADS       3001 /*-- USE KN_PARAM_NUMTHREADS */
#define KN_PARAM_PAR_CONCURRENT_EVALS 3002 /*-- USE KN_PARAM_CONCURRENT_EVALS */
#  define KN_PAR_CONCURRENT_EVALS_NO     0 /*-- USE KN_CONCURRENT_EVALS_NO */
#  define KN_PAR_CONCURRENT_EVALS_YES    1 /*-- USE KN_CONCURRENT_EVALS_YES */
#define KN_PARAM_PAR_BLASNUMTHREADS   3003 /*-- USE KN_PARAM_BLAS_NUMTHREADS */
#define KN_PARAM_PAR_LSNUMTHREADS     3004 /*-- USE KN_PARAM_LINSOLVER_NUMTHREADS */
#define KN_PARAM_PAR_MSNUMTHREADS     3005 /*-- USE KN_PARAM_MS_NUMTHREADS */
#  define KN_PAR_MSNUMTHREADS_AUTO       0 /*-- DEPRECATED */
#define KN_PARAM_PAR_CONICNUMTHREADS  3006 /*-- USE KN_PARAM_CONIC_NUMTHREADS */

#ifdef __cplusplus
}
#endif

#endif     /*-- KNITRO_H__ */
