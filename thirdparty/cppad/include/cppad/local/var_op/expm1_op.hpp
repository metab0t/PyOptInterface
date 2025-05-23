# ifndef CPPAD_LOCAL_VAR_OP_EXPM1_OP_HPP
# define CPPAD_LOCAL_VAR_OP_EXPM1_OP_HPP

// SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later
// SPDX-FileCopyrightText: Bradley M. Bell <bradbell@seanet.com>
// SPDX-FileContributor: 2003-24 Bradley M. Bell
// ----------------------------------------------------------------------------


namespace CppAD { namespace local { namespace var_op {


template <class Base>
inline void forward_expm1_op(
   size_t p           ,
   size_t q           ,
   size_t i_z         ,
   size_t i_x         ,
   size_t cap_order   ,
   Base*  taylor      )
{
   // check assumptions
   CPPAD_ASSERT_UNKNOWN( NumArg(Expm1Op) == 1 );
   CPPAD_ASSERT_UNKNOWN( NumRes(Expm1Op) == 1 );
   CPPAD_ASSERT_UNKNOWN( q < cap_order );
   CPPAD_ASSERT_UNKNOWN( p <= q );

   // Taylor coefficients corresponding to argument and result
   Base* x = taylor + i_x * cap_order;
   Base* z = taylor + i_z * cap_order;

   size_t k;
   if( p == 0 )
   {  z[0] = expm1( x[0] );
      p++;
   }
   for(size_t j = p; j <= q; j++)
   {
      z[j] = x[1] * z[j-1];
      for(k = 2; k <= j; k++)
         z[j] += Base(double(k)) * x[k] * z[j-k];
      z[j] /= Base(double(j));
      z[j] += x[j];
   }
}


template <class Base>
inline void forward_expm1_op_dir(
   size_t q           ,
   size_t r           ,
   size_t i_z         ,
   size_t i_x         ,
   size_t cap_order   ,
   Base*  taylor      )
{
   // check assumptions
   CPPAD_ASSERT_UNKNOWN( NumArg(Expm1Op) == 1 );
   CPPAD_ASSERT_UNKNOWN( NumRes(Expm1Op) == 1 );
   CPPAD_ASSERT_UNKNOWN( q < cap_order );
   CPPAD_ASSERT_UNKNOWN( 0 < q );

   // Taylor coefficients corresponding to argument and result
   size_t num_taylor_per_var = (cap_order-1) * r + 1;
   Base* x = taylor + i_x * num_taylor_per_var;
   Base* z = taylor + i_z * num_taylor_per_var;

   size_t m = (q-1)*r + 1;
   for(size_t ell = 0; ell < r; ell++)
   {  z[m+ell] = Base(double(q)) * x[m+ell] * z[0];
      for(size_t k = 1; k < q; k++)
         z[m+ell] += Base(double(k)) * x[(k-1)*r+ell+1] * z[(q-k-1)*r+ell+1];
      z[m+ell] /= Base(double(q));
      z[m+ell] += x[m+ell];
   }
}

template <class Base>
inline void forward_expm1_op_0(
   size_t i_z         ,
   size_t i_x         ,
   size_t cap_order   ,
   Base*  taylor      )
{
   // check assumptions
   CPPAD_ASSERT_UNKNOWN( NumArg(Expm1Op) == 1 );
   CPPAD_ASSERT_UNKNOWN( NumRes(Expm1Op) == 1 );
   CPPAD_ASSERT_UNKNOWN( 0 < cap_order );

   // Taylor coefficients corresponding to argument and result
   Base* x = taylor + i_x * cap_order;
   Base* z = taylor + i_z * cap_order;

   z[0] = expm1( x[0] );
}

template <class Base>
inline void reverse_expm1_op(
   size_t      i_z          ,
   size_t      i_x          ,
   size_t      cap_order    ,
   const Base* taylor       ,
   size_t      n_order      ,
   Base*       partial      )
{  // d
   size_t d = n_order - 1;
   //
   // check assumptions
   CPPAD_ASSERT_UNKNOWN( NumArg(Expm1Op) == 1 );
   CPPAD_ASSERT_UNKNOWN( NumRes(Expm1Op) == 1 );
   CPPAD_ASSERT_UNKNOWN( n_order <= cap_order );

   // Taylor coefficients and partials corresponding to argument
   const Base* x  = taylor  + i_x * cap_order;
   Base* px       = partial + i_x * n_order;

   // Taylor coefficients and partials corresponding to result
   const Base* z  = taylor  + i_z * cap_order;
   Base* pz       = partial + i_z * n_order;

   // If pz is zero, make sure this operation has no effect
   // (zero times infinity or nan would be non-zero).
   bool skip(true);
   for(size_t i_d = 0; i_d <= d; i_d++)
      skip &= IdenticalZero(pz[i_d]);
   if( skip )
      return;

   // loop through orders in reverse
   size_t j, k;
   j = d;
   while(j)
   {  px[j] += pz[j];

      // scale partial w.r.t z[j]
      pz[j] /= Base(double(j));

      for(k = 1; k <= j; k++)
      {  px[k]   += Base(double(k)) * azmul(pz[j], z[j-k]);
         pz[j-k] += Base(double(k)) * azmul(pz[j], x[k]);
      }
      --j;
   }
   px[0] += pz[0] + azmul(pz[0], z[0]);
}

} } } // END namespace
# endif
