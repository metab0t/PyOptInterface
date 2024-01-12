param N;

var x{1..N} >= 0;

minimize obj: sum {i in 1..N div 2} x[i];