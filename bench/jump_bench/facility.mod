param G;
param F;

var d;
var y {f in 1..F, k in 1..2} >= 0, <= 1;
var z {i in 0..G, j in 0..G, f in 1..F} binary; 
var s {i in 0..G, j in 0..G, f in 1..F} >= 0;
var r {i in 0..G, j in 0..G, f in 1..F, k in 1..2};

minimize Max_Dist: d;

subject to Assign_Customers {i in 0..G, j in 0..G}:
    sum {f in 1..F} z[i,j,f] == 1;

param M := 2*sqrt(2);

subject to Quad_Dist_RHS {i in 0..G, j in 0..G, f in 1..F}:
     s[i,j,f] == d + M*(1 - z[i,j,f]);
subject to Quad_Dist_K1 {i in 0..G, j in 0..G, f in 1..F}:
     r[i,j,f,1] == i/G - y[f,1];
subject to Quad_Dist_K2 {i in 0..G, j in 0..G, f in 1..F}:
     r[i,j,f,2] == j/G - y[f,2];
subject to Quad_Dist {i in 0..G, j in 0..G, f in 1..F}:
     sum {k in 1..2} r[i,j,f,k]^2 <= s[i,j,f]^2;
