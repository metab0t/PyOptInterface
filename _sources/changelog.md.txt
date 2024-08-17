# Changelog

## 0.2.8
- Fix bugs in HiGHS and MOSEK when the quadratic objective function contains nondiagonal terms

## 0.2.7
- Fix bugs in HiGHS termination status

## 0.2.6
- Add rotated second-order cone support for COPT, Gurobi and Mosek
- Add exponential cone support for COPT and Mosek
- Requires COPT version >= 7.1.4 to support exponential cone

## 0.2.5
- Fix `add_linear_constraint` of HiGHS optimizer to consider the constant term in expression correctly
- Make `make_tupledict` slightly faster

## 0.2.4
- Add `map` method for `tupledict` class
- Add type stubs for C++ extension modules 

## 0.2.3
- Fix a bug when deleting constraint in HiGHS

## 0.2.2
- Fix the performance issue with HiGHS optimizer

## 0.2.1
- Fix the DLL search paths on Windows

## 0.2.0
- Supports callback for Gurobi and COPT
- Release GIL when calling `model.optimize()`

## 0.1.1
- Add `Model.write` method to write model to files

## 0.1.0
- First release on PyPI