# ifndef CPPAD_CORE_ATOMIC_TWO_ATOMIC_HPP
# define CPPAD_CORE_ATOMIC_TWO_ATOMIC_HPP
// SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later
// SPDX-FileCopyrightText: Bradley M. Bell <bradbell@seanet.com>
// SPDX-FileContributor: 2003-24 Bradley M. Bell
// ----------------------------------------------------------------------------
/*
{xrst_begin atomic_two app}
{xrst_spell
   px
   py
   tx
   ty
   vx
   vy
}

Defining Atomic Functions: Second Generation
############################################

Deprecated 2019-01-01
*********************
Using the ``atomic_base`` class has been deprecated.
Use :ref:`atomic_three-name` instead.

Syntax
******
| *atomic_user* *afun* ( *ctor_arg_list* )
| *afun* ( *ax* , *ay* )
| *ok* = *afun* . ``forward`` ( *p* , *q* , *vx* , *vy* , *tx* , *ty* )
| *ok* = *afun* . ``reverse`` ( *q* , *tx* , *ty* , *px* , *py* )
| *ok* = *afun* . ``for_sparse_jac`` ( *q* , *r* , *s* , *x* )
| *ok* = *afun* . ``rev_sparse_jac`` ( *q* , *r* , *s* , *x* )
| *ok* = *afun* . ``for_sparse_hes`` ( *vx* , *r* , *s* , *h* , *x* )
| *ok* = *afun* . ``rev_sparse_hes`` ( *vx* , *s* , *t* , *q* , *r* , *u* , *v* , *x* )
| *atomic_base* < ``Base`` >:: *clear* ()

See Also
********
:ref:`checkpoint<chkpoint_one-name>`

Purpose
*******

Speed
=====
In some cases, the user knows how to compute derivatives of a function

.. math::

   y = f(x) \; {\rm where} \; f : \B{R}^n \rightarrow \B{R}^m

more efficiently than by coding it using ``AD`` < *Base* >
:ref:`atomic_base<glossary@Operation@Atomic>` operations
and letting CppAD do the rest.
In this case ``atomic_base`` < ``Base`` > can use
the user code for :math:`f(x)`, and its derivatives,
as ``AD`` < *Base* > atomic operations.

Reduce Memory
=============
If the function :math:`f(x)` is used often,
using an atomic version of :math:`f(x)` remove the need for repeated
copies of the corresponding ``AD`` < *Base* > operations.

Virtual Functions
*****************
User defined derivatives are implemented by defining the
following virtual functions in the *atomic_base* class:
:ref:`forward<atomic_two_forward-name>` ,
:ref:`reverse<atomic_two_reverse-name>` ,
:ref:`for_sparse_jac<atomic_two_for_sparse_jac-name>` ,
:ref:`rev_sparse_jac<atomic_two_rev_sparse_jac-name>` , and
:ref:`rev_sparse_hes<atomic_two_rev_sparse_hes-name>` .
These virtual functions have a default implementation
that returns *ok* == ``false`` .
The ``forward`` function,
for the case *q*  == 0 , must be implemented.
Otherwise, only those functions
required by the your calculations need to be implemented.
For example,
*forward* for the case *q*  == 2 can just return
*ok* == ``false`` unless you require
forward mode calculation of second derivatives.

Examples
********
See :ref:`atomic_two_example-name` .

Contents
********
{xrst_toc_table
   include/cppad/core/atomic/two/ctor.hpp
   include/cppad/core/atomic/two/option.hpp
   include/cppad/core/atomic/two/afun.hpp
   include/cppad/core/atomic/two/forward.hpp
   include/cppad/core/atomic/two/reverse.hpp
   include/cppad/core/atomic/two/for_sparse_jac.hpp
   include/cppad/core/atomic/two/rev_sparse_jac.hpp
   include/cppad/core/atomic/two/for_sparse_hes.hpp
   include/cppad/core/atomic/two/rev_sparse_hes.hpp
   include/cppad/core/atomic/two/clear.hpp
}

{xrst_end atomic_two}
-------------------------------------------------------------------------------
{xrst_begin atomic_two_example app}

Example Defining Atomic Functions: Second Generation
####################################################

Getting Started
***************
that shows the minimal amount of information required to create
a user defined atomic operation.

Scalar Function
***************
where the user provides the code for computing derivatives.
This example is simple because the domain and range are scalars.

Vector Range
************
where the user provides the code for computing derivatives.
This example is more complex because the range has two components.

Hessian Sparsity Patterns
*************************
where the user provides the code for computing Hessian sparsity patterns.

Contents
********
{xrst_toc_table
   example/atomic_two/eigen_mat_mul.cpp
   example/atomic_two/eigen_mat_inv.cpp
   example/atomic_two/eigen_cholesky.cpp
}

{xrst_end atomic_two_example}
-------------------------------------------------------------------------------
*/

# include <set>
# include <cppad/core/cppad_assert.hpp>
# include <cppad/local/sparse/internal.hpp>
# include <cppad/local/atomic_index.hpp>
# include <cppad/core/ad.hpp>

// needed before one can use in_parallel
# include <cppad/utility/thread_alloc.hpp>

namespace CppAD { // BEGIN_CPPAD_NAMESPACE
/*!
\file atomic_two.hpp
Base class for atomic function operations.
*/

template <class Base>
class atomic_base {
// ===================================================================
public:
   enum option_enum {
      pack_sparsity_enum   ,
      bool_sparsity_enum   ,
      set_sparsity_enum
   };
   //
   // atomic_index
   size_t atomic_index(void) const
   { return index_; }
private:
   // ------------------------------------------------------
   // constants
   //
   /// index of this object in local::atomic_index
   /// (set by constructor and not changed; i.e., effectively const)
   size_t index_;
   //
   // -----------------------------------------------------
   // variables
   //
   /// sparsity pattern this object is currently using
   /// (set by constructor and option member functions)
   option_enum sparsity_;
   //
   /// temporary work space used by member functions, declared here to avoid
   // memory allocation/deallocation for each usage
   struct work_struct {
      vector<bool>               vx;
      vector<bool>               vy;
      //
      vector<Base>               tx;
      vector<Base>               ty;
      //
      vector< AD<Base> >         atx;
      vector< AD<Base> >         aty;
      //
      vector<bool>               bool_t;
      //
      vectorBool                 pack_h;
      vectorBool                 pack_r;
      vectorBool                 pack_s;
      vectorBool                 pack_u;
      //
      vector<bool>               bool_h;
      vector<bool>               bool_r;
      vector<bool>               bool_s;
      vector<bool>               bool_u;
      //
      vector< std::set<size_t> > set_h;
      vector< std::set<size_t> > set_r;
      vector< std::set<size_t> > set_s;
      vector< std::set<size_t> > set_u;
   };
   // Use pointers, to avoid false sharing between threads.
   // Not using: vector<work_struct*> work_;
   // so that deprecated atomic examples do not result in a memory leak.
   work_struct* work_[CPPAD_MAX_NUM_THREADS];
public:
   // =====================================================================
   // In User API
   // =====================================================================
   //
   // ---------------------------------------------------------------------
   // ctor: doxygen in atomic_base/ctor.hpp
   atomic_base(void);
   atomic_base(
      const std::string&     name,
      option_enum            sparsity = bool_sparsity_enum
   );

   // option: see doxygen in atomic_base/option.hpp
   void option(enum option_enum option_value);

   // operator(): see doxygen in atomic_base/afun.hpp
   template <class ADVector>
   void operator()(
      const ADVector&  ax     ,
              ADVector&  ay     ,
      size_t           id = 0
   );

   // ------------------------------------------------------------------------
   // base_two version of forward
   virtual bool forward(
      size_t                    p  ,
      size_t                    q  ,
      const vector<bool>&       vx ,
      vector<bool>&             vy ,
      const vector<Base>&       tx ,
      vector<Base>&             ty
   );
   virtual bool forward(
      size_t                    p  ,
      size_t                    q  ,
      const vector<bool>&       vx ,
      vector<bool>&             vy ,
      const vector< AD<Base> >& atx ,
      vector< AD<Base> >&       aty
   );
   // base_three version of forward
   bool forward(
      size_t                       order_low  ,
      size_t                       order_up   ,
      const vector<ad_type_enum>&  type_x     ,
      vector<ad_type_enum>&        type_y     ,
      const vector<Base>&          taylor_x   ,
      vector<Base>&                taylor_y
   );
   bool forward(
      size_t                       order_low  ,
      size_t                       order_up   ,
      const vector<ad_type_enum>&  type_x     ,
      vector<ad_type_enum>&        type_y     ,
      const vector< AD<Base> >&    ataylor_x  ,
      vector< AD<Base> >&          ataylor_y
   );
   // ------------------------------------------------------------------------
   // reverse: see doxygen in atomic_base/reverse.hpp
   virtual bool reverse(
      size_t                    q  ,
      const vector<Base>&       tx ,
      const vector<Base>&       ty ,
              vector<Base>&       px ,
      const vector<Base>&       py
   );
   virtual bool reverse(
      size_t                    q   ,
      const vector< AD<Base> >& atx ,
      const vector< AD<Base> >& aty ,
              vector< AD<Base> >& apx ,
      const vector< AD<Base> >& apy
   );

   // ------------------------------------------------------------
   // for_sparse_jac: see doxygen in atomic_base/for_sparse_jac.hpp
   virtual bool for_sparse_jac(
      size_t                                  q  ,
      const vector< std::set<size_t> >&       r  ,
              vector< std::set<size_t> >&       s  ,
      const vector<Base>&                     x
   );
   virtual bool for_sparse_jac(
      size_t                                  q  ,
      const vector<bool>&                     r  ,
              vector<bool>&                     s  ,
      const vector<Base>&                     x
   );
   virtual bool for_sparse_jac(
      size_t                                  q  ,
      const vectorBool&                       r  ,
              vectorBool&                       s  ,
      const vector<Base>&                     x
   );
   template <class InternalSparsity>
   bool for_sparse_jac(
      const vector<Base>&              x            ,
      const local::pod_vector<size_t>& x_index      ,
      const local::pod_vector<size_t>& y_index      ,
      InternalSparsity&                var_sparsity
   );
   // deprecated versions
   virtual bool for_sparse_jac(
      size_t                                  q  ,
      const vector< std::set<size_t> >&       r  ,
              vector< std::set<size_t> >&       s
   );
   virtual bool for_sparse_jac(
      size_t                                  q  ,
      const vector<bool>&                     r  ,
              vector<bool>&                     s
   );
   virtual bool for_sparse_jac(
      size_t                                  q  ,
      const vectorBool&                       r  ,
              vectorBool&                       s
   );
   // ------------------------------------------------------------
   // rev_sparse_jac: see doxygen in atomic_base/rev_sparse_jac.hpp
   virtual bool rev_sparse_jac(
      size_t                                  q  ,
      const vector< std::set<size_t> >&       rt ,
              vector< std::set<size_t> >&       st ,
      const vector<Base>&                     x
   );
   virtual bool rev_sparse_jac(
      size_t                                  q  ,
      const vector<bool>&                     rt ,
              vector<bool>&                     st ,
      const vector<Base>&                     x
   );
   virtual bool rev_sparse_jac(
      size_t                                  q  ,
      const vectorBool&                       rt ,
              vectorBool&                       st ,
      const vector<Base>&                     x
   );
   template <class InternalSparsity>
   bool rev_sparse_jac(
      const vector<Base>&        x            ,
      const local::pod_vector<size_t>& x_index ,
      const local::pod_vector<size_t>& y_index ,
      InternalSparsity&          var_sparsity
   );
   // deprecated versions
   virtual bool rev_sparse_jac(
      size_t                                  q  ,
      const vector< std::set<size_t> >&       rt ,
              vector< std::set<size_t> >&       st
   );
   virtual bool rev_sparse_jac(
      size_t                                  q  ,
      const vector<bool>&                     rt ,
              vector<bool>&                     st
   );
   virtual bool rev_sparse_jac(
      size_t                                  q  ,
      const vectorBool&                       rt ,
              vectorBool&                       st
   );
   // ------------------------------------------------------------
   // for_sparse_hes: see doxygen in atomic_base/for_sparse_hes.hpp
   virtual bool for_sparse_hes(
      const vector<bool>&             vx ,
      const vector<bool>&             r  ,
      const vector<bool>&             s  ,
      vector< std::set<size_t> >&     h  ,
      const vector<Base>&             x
   );
   virtual bool for_sparse_hes(
      const vector<bool>&             vx ,
      const vector<bool>&             r  ,
      const vector<bool>&             s  ,
      vector<bool>&                   h  ,
      const vector<Base>&             x
   );
   virtual bool for_sparse_hes(
      const vector<bool>&             vx ,
      const vector<bool>&             r  ,
      const vector<bool>&             s  ,
      vectorBool&                     h  ,
      const vector<Base>&             x
   );
   template <class InternalSparsity>
   bool for_sparse_hes(
      const vector<Base>&              x                ,
      const local::pod_vector<size_t>& x_index          ,
      const local::pod_vector<size_t>& y_index          ,
      size_t                           np1              ,
      size_t                           numvar           ,
      const InternalSparsity&          rev_jac_sparsity ,
      InternalSparsity&                for_sparsity
   );
   // deprecated versions
   virtual bool for_sparse_hes(
      const vector<bool>&             vx ,
      const vector<bool>&             r  ,
      const vector<bool>&             s  ,
      vector< std::set<size_t> >&     h
   );
   virtual bool for_sparse_hes(
      const vector<bool>&             vx ,
      const vector<bool>&             r  ,
      const vector<bool>&             s  ,
      vector<bool>&                   h
   );
   virtual bool for_sparse_hes(
      const vector<bool>&             vx ,
      const vector<bool>&             r  ,
      const vector<bool>&             s  ,
      vectorBool&                     h
   );
   // ------------------------------------------------------------
   // rev_sparse_hes: see doxygen in atomic_base/rev_sparse_hes.hpp
   virtual bool rev_sparse_hes(
      const vector<bool>&                     vx ,
      const vector<bool>&                     s  ,
              vector<bool>&                     t  ,
      size_t                                  q  ,
      const vector< std::set<size_t> >&       r  ,
      const vector< std::set<size_t> >&       u  ,
              vector< std::set<size_t> >&       v  ,
      const vector<Base>&                     x
   );
   virtual bool rev_sparse_hes(
      const vector<bool>&                     vx ,
      const vector<bool>&                     s  ,
              vector<bool>&                     t  ,
      size_t                                  q  ,
      const vector<bool>&                     r  ,
      const vector<bool>&                     u  ,
              vector<bool>&                     v  ,
      const vector<Base>&                     x
   );
   virtual bool rev_sparse_hes(
      const vector<bool>&                     vx ,
      const vector<bool>&                     s  ,
              vector<bool>&                     t  ,
      size_t                                  q  ,
      const vectorBool&                       r  ,
      const vectorBool&                       u  ,
              vectorBool&                       v  ,
      const vector<Base>&                     x
   );
   template <class InternalSparsity>
   bool rev_sparse_hes(
      const vector<Base>&              x                ,
      const local::pod_vector<size_t>& x_index          ,
      const local::pod_vector<size_t>& y_index          ,
      const InternalSparsity&          for_jac_sparsity ,
      bool*                            rev_jac_flag     ,
      InternalSparsity&                rev_hes_sparsity
   );
   // deprecated
   virtual bool rev_sparse_hes(
      const vector<bool>&                     vx ,
      const vector<bool>&                     s  ,
              vector<bool>&                     t  ,
      size_t                                  q  ,
      const vector< std::set<size_t> >&       r  ,
      const vector< std::set<size_t> >&       u  ,
              vector< std::set<size_t> >&       v
   );
   virtual bool rev_sparse_hes(
      const vector<bool>&                     vx ,
      const vector<bool>&                     s  ,
              vector<bool>&                     t  ,
      size_t                                  q  ,
      const vector<bool>&                     r  ,
      const vector<bool>&                     u  ,
              vector<bool>&                     v
   );
   virtual bool rev_sparse_hes(
      const vector<bool>&                     vx ,
      const vector<bool>&                     s  ,
              vector<bool>&                     t  ,
      size_t                                  q  ,
      const vectorBool&                       r  ,
      const vectorBool&                       u  ,
              vectorBool&                       v
   );
   // ------------------------------------------------------------
   // atomic_three like interface for reverse dependency analysis
   bool rev_depend(
      const vector<Base>&         parameter_x ,
      const vector<ad_type_enum>& type_x      ,
      vector<bool>&               depend_x    ,
      const vector<bool>&         depend_y
   );
   // ------------------------------------------------------------
   // clear: see doxygen in atomic_base/clear.hpp
   static void clear(void);

   // =====================================================================
   // Not in User API
   // =====================================================================

   /// current sparsity setting
   option_enum sparsity(void) const
   {  return sparsity_; }

   /// Name corresponding to a atomic_base object
   const std::string atomic_name(void) const
   {  bool        set_null = false;
      size_t      type  = 0;          // set to avoid warning
      std::string name;
      void*       v_ptr = nullptr; // set to avoid warning
      local::atomic_index<Base>(set_null, index_, type, &name, v_ptr);
      CPPAD_ASSERT_UNKNOWN( type == 2 );
      return name;
   }
   /// destructor informs CppAD that this atomic function with this index
   /// has dropped out of scope by setting its pointer to null
   virtual ~atomic_base(void)
   {  // change object pointer to null, but leave name for error reporting
      bool         set_null = true;
      size_t       type  = 0;          // set to avoid warning
      std::string* name  = nullptr;
      void*        v_ptr = nullptr; // set to avoid warning
      local::atomic_index<Base>(set_null, index_, type, name, v_ptr);
      CPPAD_ASSERT_UNKNOWN( type == 2 );
      //
      // free temporary work memory
      for(size_t thread = 0; thread < CPPAD_MAX_NUM_THREADS; thread++)
         free_work(thread);
   }
   /// allocates work_ for a specified thread
   void allocate_work(size_t thread)
   {  if( work_[thread] == nullptr )
      {  // allocate the raw memory
         size_t min_bytes = sizeof(work_struct);
         size_t num_bytes;
         void*  v_ptr     = thread_alloc::get_memory(min_bytes, num_bytes);
         // save in work_
         work_[thread]    = reinterpret_cast<work_struct*>( v_ptr );
         // call constructor
         new( work_[thread] ) work_struct;
      }
      return;
   }
   /// frees work_ for a specified thread
   void free_work(size_t thread)
   {  if( work_[thread] != nullptr )
      {  // call destructor
         work_[thread]->~work_struct();
         // return memory to avialable pool for this thread
         thread_alloc::return_memory(
            reinterpret_cast<void*>(work_[thread])
         );
         // mark this thread as not allocated
         work_[thread] = nullptr;
      }
      return;
   }
   /// atomic_base function object corresponding to a certain index
   static atomic_base* class_object(size_t index)
   {  bool         set_null = false;
      size_t       type  = 0;          // set to avoid warning
      std::string* name  = nullptr;
      void*        v_ptr = nullptr; // set to avoid warning
      local::atomic_index<Base>(set_null, index, type, name, v_ptr);
      CPPAD_ASSERT_UNKNOWN( type == 2 );
      return reinterpret_cast<atomic_base*>( v_ptr );
   }
   /// atomic_base function name corresponding to a certain index
   static const std::string class_name(size_t index)
   {  bool        set_null = false;
      size_t      type  = 0;          // set to avoid warning
      std::string name;
      void*       v_ptr = nullptr; // set to avoid warning
      local::atomic_index<Base>(set_null, index, type, &name, v_ptr);
      CPPAD_ASSERT_UNKNOWN( type == 2 );
      return name;
   }

   /*!
   Set value of id (used by deprecated atomic_one class)

   This function is called just before calling any of the virtual function
   and has the corresponding id of the corresponding virtual call.
   */
   virtual void set_old(size_t id)
   { }
// ---------------------------------------------------------------------------
};
} // END_CPPAD_NAMESPACE

// functitons implemented in cppad/core/atomic_base files
# include <cppad/core/atomic/two/ctor.hpp>
# include <cppad/core/atomic/two/option.hpp>
# include <cppad/core/atomic/two/afun.hpp>
# include <cppad/core/atomic/two/forward.hpp>
# include <cppad/core/atomic/two/reverse.hpp>
# include <cppad/core/atomic/two/for_sparse_jac.hpp>
# include <cppad/core/atomic/two/rev_sparse_jac.hpp>
# include <cppad/core/atomic/two/for_sparse_hes.hpp>
# include <cppad/core/atomic/two/rev_sparse_hes.hpp>
# include <cppad/core/atomic/two/rev_depend.hpp>
# include <cppad/core/atomic/two/clear.hpp>

# endif
