param N;

var x{1..N} >= 0;

s.t. c1: sum {i in 1..N} x[i] = N;

minimize obj: sum {i in 1..N div 2} i / N * x[i];