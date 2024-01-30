@echo off

REM Copyright (c) 2022: Miles Lubin and contributors
REM
REM Use of this source code is governed by an MIT-style license that can be found
REM in the LICENSE.md file or at https://opensource.org/licenses/MIT.
REM
REM Run as
REM   > gurobi_cpp_build.bat gurobiXX
REM where `gurobiXX` is the version of Gurobi you are compiling for.

REM Update if needed
REM SET GUROBI_HOME=C:\path\to\gurobi

REM Compile facility.cpp
cl /EHsc /O2 facility.cpp /I %GUROBI_HOME%\include /link /LIBPATH:%GUROBI_HOME%\lib gurobi_c++mt2017.lib %1.lib /out:cpp_facility.exe

REM Compile lqcp.cpp
cl /EHsc /O2 lqcp.cpp /I %GUROBI_HOME%\include /link /LIBPATH:%GUROBI_HOME%\lib gurobi_c++mt2017.lib %1.lib /out:cpp_lqcp.exe

del *.obj