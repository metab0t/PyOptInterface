For CppAD, remember to change `# define CPPAD_C_COMPILER_MSVC_FLAGS 0` and `# define CPPAD_HAS_TMPNAM_S 0` in `cppad/include/configure.hpp`

Use `Get-ChildItem -Path . -Recurse -Filter *.xrst | Remove-Item -Force` to remove all `.xrst` files in CppAD directory.

For IPOPT, remember to mask `IPOPTLIB_EXPORT` in `solvers/ipopt/IpoptConfig.h`