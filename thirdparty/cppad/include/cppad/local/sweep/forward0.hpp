# ifndef CPPAD_LOCAL_SWEEP_FORWARD0_HPP
# define CPPAD_LOCAL_SWEEP_FORWARD0_HPP
// SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later
// SPDX-FileCopyrightText: Bradley M. Bell <bradbell@seanet.com>
// SPDX-FileContributor: 2003-24 Bradley M. Bell
// ----------------------------------------------------------------------------

# include <cppad/local/play/atom_op_info.hpp>
# include <cppad/local/sweep/call_atomic.hpp>
# include <cppad/local/var_op/compare.hpp>

// BEGIN_CPPAD_LOCAL_SWEEP_NAMESPACE
namespace CppAD { namespace local { namespace sweep {
/*!
\file sweep/forward0.hpp
Compute zero order forward mode Taylor coefficients.
*/

/*
 ------------------------------------------------------------------------------
{xrst_begin sweep_forward0 dev}
{xrst_spell
   cskip
   numvar
   pri
}
Compute Zero Order Forward Mode Taylor Coefficients
###################################################

Syntax
******
| ``forward0`` (
| |tab| *play* ,
| |tab| *s_out* ,
| |tab| *print* ,
| |tab| *n* ,
| |tab| *numvar* ,
| |tab| *J* ,
| |tab| *taylor* ,
| |tab| *cskip_op* ,
| |tab| *load_op2var* ,
| |tab| *compare_change_count* ,
| |tab| *compare_change_number* ,
| |tab| *compare_change_op_index* ,
| |tab| *not_used_rec_base*
| )

CPPAD_FORWARD0_TRACE
********************
This value is either zero or one.
Zero is the normal operational value.
If it is one, a trace of every zero order forward mode computation is printed.
{xrst_spell_off}
{xrst_code hpp} */
# define CPPAD_FORWARD0_TRACE 0
/* {xrst_code}
{xrst_spell_on}

Base
****
The type used during the forward mode computations; i.e., the corresponding
recording of operations used the type ``AD`` < *Base* > .

s_out
*****
Is the stream where output corresponding to PriOp operations will
be written.

print
*****
If print is false,
suppress the output that is otherwise generated by the PriOp instructions.

n
*
is the number of independent variables on the tape.

numvar
******
is the total number of variables on the tape.
This is also equal to the number of rows in the matrix taylor; i.e.,
*play* ``->num_var_rec`` () .

play
****
The information stored in play
is a recording of the operations corresponding to a function

.. math::

   f : \B{R}^n \rightarrow \B{R}^m

where *n* is the number of independent variables and
*m* is the number of dependent variables.

J
*
Is the number of columns in the coefficient matrix taylor.
This must be greater than or equal one.

taylor
******
Is the matrix of Taylor coefficients.

Input
=====
For *i* = 1 , ... , *n* ,
*taylor* [% *i* % * % *J* % + 0]%
is the zero order Taylor coefficient for variable with index
*j* on the tape (these are the independent variables).

Output
======
For *i* = *n* +1 , ... , *numvar* ``-1`` ,
*taylor* [% *i* % * % *J* % + 0]%
is the zero order Taylor coefficient for the variable with
index i on the tape.

cskip_op
********
Is a vector with size *play* ``->num_op_rec`` () .
The input value of the elements does not matter.
Upon return, if *cskip_op* [ *i* ] is true,
the operator index *i* does not affect any of the dependent variable
(given the value of the independent variables).

load_op2var
***********
Is a vector with size *play* ``->num_var_load_rec`` () .
The input value of the elements does not matter.
Upon return, *load_op2var* [ *i* ]
is the variable index corresponding to the *i*-th variable VecAD load operator.
Note that even though the VecAD vector is a variable, the load
can correspond to an element that is a parameter in which case
*load_op2var* [ *i* ] is zero.

compare_change_count
********************
Is the compare change count value at which *compare_change_op_index*
is returned. If it is zero, the comparison changes are not counted.

compare_change_number
*********************
If *compare_change_count* is zero, this value is set to zero.
Otherwise, the return value is the number of comparison operations
that have a different result from when the information in
*play* was recorded.

compare_change_op_index
***********************
If *compare_change_count* is zero, this value is set to zero.
Otherwise it is the operator index (see forward_next) for the
comparison operation that has a different result from when the information in
play was recorded.
This is not the first comparison that is different,
but rather the *compare_change_count* comparison.

not_used_rec_base
*****************
Specifies *RecBase* for this call.

{xrst_end sweep_forward0}
*/

template <class Base, class RecBase>
void forward0(
   const local::player<Base>* play,
   std::ostream&              s_out,
   bool                       print,
   size_t                     n,
   size_t                     numvar,
   size_t                     J,
   Base*                      taylor,
   bool*                      cskip_op,
   pod_vector<addr_t>&        load_op2var,
   size_t                     compare_change_count,
   size_t&                    compare_change_number,
   size_t&                    compare_change_op_index,
   const RecBase&             not_used_rec_base
)
{  CPPAD_ASSERT_UNKNOWN( J >= 1 );
   CPPAD_ASSERT_UNKNOWN( play->num_var_rec() == numvar );

   // use p, q, r so other forward sweeps can use code defined here
   size_t p = 0;
   size_t q = 0;
   size_t r = 1;

   // initialize the comparison operator counter
   if( p == 0 )
   {  compare_change_number   = 0;
      compare_change_op_index = 0;
   }

   // If this includes a zero calculation, initialize this information
   pod_vector<bool>   vec_ad2isvar;
   pod_vector<size_t> vec_ad2index;
   if( p == 0 )
   {  size_t i;

      // this includes order zero calculation, initialize vector indices
      size_t num = play->num_var_vecad_ind_rec();
      if( num > 0 )
      {  vec_ad2isvar.extend(num);
         vec_ad2index.extend(num);
         for(i = 0; i < num; i++)
         {  vec_ad2index[i] = play->GetVecInd(i);
            vec_ad2isvar[i] = false;
         }
      }
      // includes zero order, so initialize conditional skip flags
      num = play->num_op_rec();
      for(i = 0; i < num; i++)
         cskip_op[i] = false;
   }

   // information used by atomic function operators
   const pod_vector<bool>& dyn_par_is( play->dyn_par_is() );
   const size_t need_y    = size_t( variable_enum );
   const size_t order_low = p;
   const size_t order_up  = q;

   // vectors used by atomic function operators
   vector<Base>         atom_par_x;  // argument parameter values
   vector<ad_type_enum> atom_type_x; // argument type
   vector<Base>         atom_tx;     // argument vector Taylor coefficients
   vector<Base>         atom_ty;     // result vector Taylor coefficients
   vector<size_t>       atom_iy;     // variable indices for result vector
   vector<bool>         atom_sy;     // select_y for this atomic function
   //
   // information defined by atomic function operators
   size_t atom_index=0, atom_id=0, atom_m=0, atom_n=0, atom_i=0, atom_j=0;
   enum_atom_state atom_state = start_atom; // proper initialization

   // length of the parameter vector (used by CppAD assert macros)
   const size_t num_par = play->num_par_rec();

   // pointer to the beginning of the parameter vector
   CPPAD_ASSERT_UNKNOWN( num_par > 0 )
   const Base* parameter = play->GetPar();

   // length of the text vector (used by CppAD assert macros)
   const size_t num_text = play->num_text_rec();

   // pointer to the beginning of the text vector
   const char* text = nullptr;
   if( num_text > 0 )
      text = play->GetTxt(0);

# if CPPAD_FORWARD0_TRACE
   // flag as to when to trace atomic function values
   bool atom_trace            = false;
# endif

   // skip the BeginOp at the beginning of the recording
   play::const_sequential_iterator itr = play->begin();
   // op_info
   op_code_var op;
   size_t i_var;
   const addr_t* arg;
   itr.op_info(op, arg, i_var);
   CPPAD_ASSERT_UNKNOWN( op == BeginOp );
   //
# if CPPAD_FORWARD0_TRACE
   std::cout << std::endl;
# endif
   bool flag; // a temporary flag to use in switch cases
   bool more_operators = true;
   while(more_operators)
   {
      // next op
      (++itr).op_info(op, arg, i_var);
      CPPAD_ASSERT_UNKNOWN( itr.op_index() < play->num_op_rec() );

      // check if we are skipping this operation
      while( cskip_op[itr.op_index()] )
      {  switch(op)
         {
            case AFunOp:
            {  // get information for this atomic function call
               CPPAD_ASSERT_UNKNOWN( atom_state == start_atom );
               play::atom_op_info<Base>(
                  op, arg, atom_index, atom_id, atom_m, atom_n
               );
               //
               // skip to the second AFunOp
               for(size_t i = 0; i < atom_m + atom_n + 1; ++i)
                  ++itr;
# ifndef NDEBUG
               itr.op_info(op, arg, i_var);
               CPPAD_ASSERT_UNKNOWN( op == AFunOp );
# endif
            }
            break;

            case CSkipOp:
            case CSumOp:
            itr.correct_before_increment();
            break;

            default:
            break;
         }
         (++itr).op_info(op, arg, i_var);
      }

      // action to take depends on the case
      switch( op )
      {
         case EqppOp:
         case EqpvOp:
         case EqvvOp:
         case LeppOp:
         case LepvOp:
         case LevpOp:
         case LevvOp:
         case LtppOp:
         case LtpvOp:
         case LtvpOp:
         case LtvvOp:
         case NeppOp:
         case NepvOp:
         case NevvOp:
         var_op::compare(op,
            arg, parameter, J, taylor, itr.op_index(), compare_change_count,
            compare_change_number, compare_change_op_index
         );
         break;
         // -------------------------------------------------

         case AbsOp:
         var_op::forward_abs_op_0(i_var, size_t(arg[0]), J, taylor);
         break;
         // -------------------------------------------------

         case AddvvOp:
         var_op::forward_addvv_op_0(i_var, arg, parameter, J, taylor);
         break;
         // -------------------------------------------------

         case AddpvOp:
         CPPAD_ASSERT_UNKNOWN( size_t(arg[0]) < num_par );
         var_op::forward_addpv_op_0(i_var, arg, parameter, J, taylor);
         break;
         // -------------------------------------------------

         case AcosOp:
         // sqrt(1 - x * x), acos(x)
         CPPAD_ASSERT_UNKNOWN( i_var < numvar  );
         var_op::forward_acos_op_0(i_var, size_t(arg[0]), J, taylor);
         break;
         // -------------------------------------------------

         case AcoshOp:
         // sqrt(x * x - 1), acosh(x)
         CPPAD_ASSERT_UNKNOWN( i_var < numvar  );
         var_op::forward_acosh_op_0(i_var, size_t(arg[0]), J, taylor);
         break;
         // -------------------------------------------------

         case AsinOp:
         // sqrt(1 - x * x), asin(x)
         CPPAD_ASSERT_UNKNOWN( i_var < numvar  );
         var_op::forward_asin_op_0(i_var, size_t(arg[0]), J, taylor);
         break;
         // -------------------------------------------------

         case AsinhOp:
         // sqrt(1 + x * x), asinh(x)
         CPPAD_ASSERT_UNKNOWN( i_var < numvar  );
         var_op::forward_asinh_op_0(i_var, size_t(arg[0]), J, taylor);
         break;
         // -------------------------------------------------

         case AtanOp:
         // 1 + x * x, atan(x)
         CPPAD_ASSERT_UNKNOWN( i_var < numvar  );
         var_op::forward_atan_op_0(i_var, size_t(arg[0]), J, taylor);
         break;
         // -------------------------------------------------

         case AtanhOp:
         // 1 - x * x, atanh(x)
         CPPAD_ASSERT_UNKNOWN( i_var < numvar  );
         var_op::forward_atanh_op_0(i_var, size_t(arg[0]), J, taylor);
         break;
         // -------------------------------------------------

         case CExpOp:
         // Use the general case with d == 0
         // (could create an optimzied version for this case)
         var_op::forward_cond_op_0(
            i_var, arg, num_par, parameter, J, taylor
         );
         break;
         // ---------------------------------------------------

         case CosOp:
         // sin(x), cos(x)
         CPPAD_ASSERT_UNKNOWN( i_var < numvar  );
         var_op::forward_cos_op_0(i_var, size_t(arg[0]), J, taylor);
         break;
         // ---------------------------------------------------

         case CoshOp:
         // sinh(x), cosh(x)
         CPPAD_ASSERT_UNKNOWN( i_var < numvar  );
         var_op::forward_cosh_op_0(i_var, size_t(arg[0]), J, taylor);
         break;
         // -------------------------------------------------

         case CSkipOp:
         var_op::forward_cskip_op_0(
            i_var, arg, num_par, parameter, J, taylor, cskip_op
         );
         itr.correct_before_increment();
         break;
         // -------------------------------------------------

         case CSumOp:
         var_op::csum_forward_op(
            0, 0, i_var, arg, num_par, parameter, J, taylor
         );
         itr.correct_before_increment();
         break;
         // -------------------------------------------------

         case DisOp:
         var_op::forward_dis_op<RecBase>(p, q, r, i_var, arg, J, taylor);
         break;
         // -------------------------------------------------

         case DivvvOp:
         var_op::forward_divvv_op_0(i_var, arg, parameter, J, taylor);
         break;
         // -------------------------------------------------

         case DivpvOp:
         CPPAD_ASSERT_UNKNOWN( size_t(arg[0]) < num_par );
         var_op::forward_divpv_op_0(i_var, arg, parameter, J, taylor);
         break;
         // -------------------------------------------------

         case DivvpOp:
         CPPAD_ASSERT_UNKNOWN( size_t(arg[1]) < num_par );
         var_op::forward_divvp_op_0(i_var, arg, parameter, J, taylor);
         break;
         // -------------------------------------------------

         case EndOp:
         CPPAD_ASSERT_NARG_NRES(op, 0, 0);
         more_operators = false;
         break;
         // -------------------------------------------------

         case ErfOp:
         case ErfcOp:
         var_op::forward_erf_op_0(op, i_var, arg, parameter, J, taylor);
         break;
         // -------------------------------------------------

         case ExpOp:
         var_op::forward_exp_op_0(i_var, size_t(arg[0]), J, taylor);
         break;
         // -------------------------------------------------

         case Expm1Op:
         var_op::forward_expm1_op_0(i_var, size_t(arg[0]), J, taylor);
         break;
         // -------------------------------------------------

         case InvOp:
         CPPAD_ASSERT_NARG_NRES(op, 0, 1);
         break;
         // ---------------------------------------------------

         case LdpOp:
         case LdvOp:
         var_op::load_forward_0(
            op,
            i_var,
            play->num_var_vecad_ind_rec(),
            arg,
            numvar,
            num_par,
            parameter,
            J,
            taylor,
            vec_ad2isvar,
            vec_ad2index,
            load_op2var
         );
         break;
         // -------------------------------------------------

         case LogOp:
         var_op::forward_log_op_0(i_var, size_t(arg[0]), J, taylor);
         break;
         // -------------------------------------------------

         case Log1pOp:
         var_op::forward_log1p_op_0(i_var, size_t(arg[0]), J, taylor);
         break;
         // -------------------------------------------------

         case MulpvOp:
         CPPAD_ASSERT_UNKNOWN( size_t(arg[0]) < num_par );
         var_op::forward_mulpv_op_0(i_var, arg, parameter, J, taylor);
         break;
         // -------------------------------------------------

         case MulvvOp:
         var_op::forward_mulvv_op_0(i_var, arg, parameter, J, taylor);
         break;
         // -------------------------------------------------

         case NegOp:
         var_op::forward_neg_op_0(i_var, size_t(arg[0]), J, taylor);
         break;

         // -------------------------------------------------

         case ParOp:
         var_op::forward_par_op_0(
            i_var, arg, num_par, parameter, J, taylor
         );
         break;
         // -------------------------------------------------

         case PowvpOp:
         CPPAD_ASSERT_UNKNOWN( size_t(arg[1]) < num_par );
         var_op::forward_powvp_op_0(i_var, arg, parameter, J, taylor);
         break;
         // -------------------------------------------------

         case PowpvOp:
         CPPAD_ASSERT_UNKNOWN( size_t(arg[0]) < num_par );
         var_op::forward_powpv_op_0(i_var, arg, parameter, J, taylor);
         break;
         // -------------------------------------------------

         case PowvvOp:
         var_op::forward_powvv_op_0(i_var, arg, parameter, J, taylor);
         break;
         // -------------------------------------------------

         case PriOp:
         if( print ) var_op::forward_pri_0(s_out,
            arg, num_text, text, num_par, parameter, J, taylor
         );
         break;
         // -------------------------------------------------

         case SignOp:
         // cos(x), sin(x)
         CPPAD_ASSERT_UNKNOWN( i_var < numvar  );
         var_op::forward_sign_op_0(i_var, size_t(arg[0]), J, taylor);
         break;
         // -------------------------------------------------

         case SinOp:
         // cos(x), sin(x)
         CPPAD_ASSERT_UNKNOWN( i_var < numvar  );
         var_op::forward_sin_op_0(i_var, size_t(arg[0]), J, taylor);
         break;
         // -------------------------------------------------

         case SinhOp:
         // cosh(x), sinh(x)
         CPPAD_ASSERT_UNKNOWN( i_var < numvar  );
         var_op::forward_sinh_op_0(i_var, size_t(arg[0]), J, taylor);
         break;
         // -------------------------------------------------

         case SqrtOp:
         var_op::forward_sqrt_op_0(i_var, size_t(arg[0]), J, taylor);
         break;
         // -------------------------------------------------

         case StppOp:
         case StpvOp:
         case StvpOp:
         case StvvOp:
         var_op::store_forward_0(
            op,
            arg,
            numvar,
            num_par,
            parameter,
            J,
            taylor,
            vec_ad2isvar,
            vec_ad2index
         );
         break;
         // -------------------------------------------------

         case SubvvOp:
         var_op::forward_subvv_op_0(i_var, arg, parameter, J, taylor);
         break;
         // -------------------------------------------------

         case SubpvOp:
         CPPAD_ASSERT_UNKNOWN( size_t(arg[0]) < num_par );
         var_op::forward_subpv_op_0(i_var, arg, parameter, J, taylor);
         break;
         // -------------------------------------------------

         case SubvpOp:
         CPPAD_ASSERT_UNKNOWN( size_t(arg[1]) < num_par );
         var_op::forward_subvp_op_0(i_var, arg, parameter, J, taylor);
         break;
         // -------------------------------------------------

         case TanOp:
         // tan(x)^2, tan(x)
         CPPAD_ASSERT_UNKNOWN( i_var < numvar  );
         var_op::forward_tan_op_0(i_var, size_t(arg[0]), J, taylor);
         break;
         // -------------------------------------------------

         case TanhOp:
         // tanh(x)^2, tanh(x)
         CPPAD_ASSERT_UNKNOWN( i_var < numvar  );
         var_op::forward_tanh_op_0(i_var, size_t(arg[0]), J, taylor);
         break;
         // -------------------------------------------------

         case AFunOp:
         // start or end an atomic function call
         flag = atom_state == start_atom;
         play::atom_op_info<RecBase>(
            op, arg, atom_index, atom_id, atom_m, atom_n
         );
         if( flag )
         {  atom_state = arg_atom;
            atom_i     = 0;
            atom_j     = 0;
            //
            atom_par_x.resize(atom_n);
            atom_type_x.resize(atom_n);
            atom_tx.resize(atom_n);
            atom_ty.resize(atom_m);
            atom_iy.resize(atom_m);
            atom_sy.resize(atom_m);
         }
         else
         {  CPPAD_ASSERT_UNKNOWN( atom_i == atom_m );
            CPPAD_ASSERT_UNKNOWN( atom_j == atom_n );
            atom_state = start_atom;
            //
            for(size_t i = 0; i < atom_m; ++i)
               atom_sy[i] = atom_iy[i] != 0;
            //
            // call atomic function for this operation
            call_atomic_forward<Base, RecBase>(
               atom_par_x, atom_type_x, need_y, atom_sy,
               order_low, order_up, atom_index, atom_id, atom_tx, atom_ty
            );
            for(size_t i = 0; i < atom_m; ++i)
               if( atom_iy[i] > 0 )
                  taylor[ atom_iy[i] * J + 0 ] = atom_ty[i];
# if CPPAD_FORWARD0_TRACE
            atom_trace = true;
# endif
         }
         break;

         case FunapOp:
         // parameter argument for an atomic function
         CPPAD_ASSERT_UNKNOWN( NumArg(op) == 1 );
         CPPAD_ASSERT_UNKNOWN( atom_state == arg_atom );
         CPPAD_ASSERT_UNKNOWN( atom_i == 0 );
         CPPAD_ASSERT_UNKNOWN( atom_j < atom_n );
         CPPAD_ASSERT_UNKNOWN( size_t( arg[0] ) < num_par );
         //
         if( dyn_par_is[ arg[0] ] )
            atom_type_x[atom_j] = dynamic_enum;
         else
            atom_type_x[atom_j] = constant_enum;
         atom_par_x[atom_j] = parameter[ arg[0] ];
         atom_tx[atom_j++]  = parameter[ arg[0] ];
         //
         if( atom_j == atom_n )
         {  // call atomic function for this operation
            atom_state = ret_atom;
         }
         break;

         case FunavOp:
         // variable argument for an atomic function
         CPPAD_ASSERT_UNKNOWN( NumArg(op) == 1 );
         CPPAD_ASSERT_UNKNOWN( atom_state == arg_atom );
         CPPAD_ASSERT_UNKNOWN( atom_i == 0 );
         CPPAD_ASSERT_UNKNOWN( atom_j < atom_n );
         //
         atom_type_x[atom_j] = variable_enum;
         atom_par_x[atom_j]  = CppAD::numeric_limits<Base>::quiet_NaN();
         atom_tx[atom_j++]   = taylor[ size_t(arg[0]) * J + 0 ];
         //
         if( atom_j == atom_n )
            atom_state = ret_atom;
         break;

         case FunrpOp:
         // parameter result for an atomic function
         CPPAD_ASSERT_NARG_NRES(op, 1, 0);
         CPPAD_ASSERT_UNKNOWN( atom_state == ret_atom );
         CPPAD_ASSERT_UNKNOWN( atom_i < atom_m );
         CPPAD_ASSERT_UNKNOWN( atom_j == atom_n );
         CPPAD_ASSERT_UNKNOWN( size_t( arg[0] ) < num_par );
         atom_iy[atom_i++] = 0;
         if( atom_i == atom_m )
            atom_state = end_atom;
         break;

         case FunrvOp:
         // variable result for an atomic function
         CPPAD_ASSERT_NARG_NRES(op, 0, 1);
         CPPAD_ASSERT_UNKNOWN( atom_state == ret_atom );
         CPPAD_ASSERT_UNKNOWN( atom_i < atom_m );
         CPPAD_ASSERT_UNKNOWN( atom_j == atom_n );
         atom_iy[atom_i++] = i_var;
         if( atom_i == atom_m )
            atom_state = end_atom;
         break;
         // -------------------------------------------------

         case ZmulpvOp:
         CPPAD_ASSERT_UNKNOWN( size_t(arg[0]) < num_par );
         var_op::forward_zmulpv_op_0(i_var, arg, parameter, J, taylor);
         break;
         // -------------------------------------------------

         case ZmulvpOp:
         CPPAD_ASSERT_UNKNOWN( size_t(arg[1]) < num_par );
         var_op::forward_zmulvp_op_0(i_var, arg, parameter, J, taylor);
         break;
         // -------------------------------------------------

         case ZmulvvOp:
         var_op::forward_zmulvv_op_0(i_var, arg, parameter, J, taylor);
         break;
         // -------------------------------------------------

         default:
         CPPAD_ASSERT_UNKNOWN(false);
      }
# if CPPAD_FORWARD0_TRACE
      size_t  d  = 0;
      if( atom_trace )
      {  atom_trace = false;

         CPPAD_ASSERT_UNKNOWN( op == AFunOp );
         CPPAD_ASSERT_UNKNOWN( NumArg(FunrvOp) == 0 );
         for(size_t i = 0; i < atom_m; i++) if( atom_iy[i] > 0 )
         {  size_t i_tmp   = (itr.op_index() + i) - atom_m;
            printOp<Base, RecBase>(
               std::cout,
               play,
               i_tmp,
               atom_iy[i],
               FunrvOp,
               nullptr
            );
            Base* Z_tmp = taylor + atom_iy[i] * J;
            printOpResult(
               std::cout,
               d + 1,
               Z_tmp,
               0,
               (Base *) nullptr
            );
            std::cout << std::endl;
         }
      }
      Base*           Z_tmp   = taylor + i_var * J;
      if( op != FunrvOp )
      {
         printOp<Base, RecBase>(
            std::cout,
            play,
            itr.op_index(),
            i_var,
            op,
            arg
         );
         if( NumRes(op) > 0 ) printOpResult(
            std::cout,
            d + 1,
            Z_tmp,
            0,
            (Base *) nullptr
         );
         std::cout << std::endl;
      }
   }
   std::cout << std::endl;
# else
   }
# endif
   CPPAD_ASSERT_UNKNOWN( atom_state == start_atom );

   return;
}

} } } // END_CPPAD_LOCAL_SWEEP_NAMESPACE

// preprocessor symbols that are local to this file
# undef CPPAD_FORWARD0_TRACE

# endif
