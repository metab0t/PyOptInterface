# http://plato.asu.edu/ftp/ampl_files/parabol_ampl/cont5_2_1.mod
#
#   A linear-quadratic control problem as suggested by
#
#   K. Schittkowski,
#   ``Numerical solution of a time-optimal parabolic boundary-value
#     control problem'', J. Optim. Theory Appl., vol. 27, pp. 271-290, 1979,
#
#   H. Goldberg and F. Troeltzsch,
#   ``On a Lagrange-Newton method for a nonlinear parabolic boundary control
#    problem'', Optim. Meth. Software, vol. 8, pp. 225-247, 1998.
#
#   and
#
#   H. D. Mittelmann,
#   ``Sufficient Optimality for Discretized Parabolic and 
#     Elliptic Control Problems'', to appear in
#     Proc. Workshop Fast solution of discretized optimization problems,
#     WIAS Berlin, 5/2000, Birkhaeuser-Verlag, Basel
#   See also Hans Mittelmann's WWW article
#
#    http://plato.la.asu.edu/papers/paper91/paper.html
#
#   where the problem is example 5.2-I.
#
# this example was also coded in SIF by Nick Gould and added to CUTE
# as CONT5-QP, classification QLR2-MN-V-V
#
param n;
param m := n;
param n1 := n-1;
param m1 := m-1;
param dx := 1/n;
param T default 1.58;
param dt := T/m;
param h2 := dx^2;
param a := .001;
param yt{j in 0..n} := .5*(1 - (j*dx)^2);

var y{0..m, 0..n};
var u{i in 1..m};

minimize f: 
 .25*dx*( (y[m,0] - yt[0])^2 +
   2* sum{j in 1..n1} (y[m,j] - yt[j])^2 + (y[m,n] - yt[n])^2)
   + .25*a*dt*( 2* sum{i in 1..m1} u[i]^2 + u[m]^2);

s.t. pde{i in 0..m1, j in 1..n1}:
        (y[i+1,j] - y[i,j])/dt = .5*(y[i,j-1] - 2*y[i,j] + y[i,j+1]
         + y[i+1,j-1] - 2*y[i+1,j] + y[i+1,j+1])/h2;

s.t. ic {j in 0..n}: y[0,j] = 0;
s.t. bc1 {i in 1..m}: y[i,2] - 4*y[i,1] + 3*y[i,0] = 0;
s.t. bc2 {i in 1..m}: (y[i,n-2] - 4*y[i,n1] + 3*y[i,n])/(2*dx) = u[i]-y[i,n];


s.t. cc{i in 1..m}: -1 <= u[i] <= 1;
s.t. sc{i in 0..m,j in 0..n}: 0 <= y[i,j] <= 1;
