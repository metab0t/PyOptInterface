#!/bin/bash

# Copyright (c) 2022: Miles Lubin and contributors
#
# Use of this source code is governed by an MIT-style license that can be found
# in the LICENSE.md file or at https://opensource.org/licenses/MIT.

# Run as
#   $ ./gurobi_cpp_build.sh gurobiXX
# where `gurobiXX` is the version of Gurobi you are compiling for.

# Update if needed
# export GUROBI_HOME="/Library/gurobi951/macos_universal2/"

clang++ -O2 facility.cpp -o cpp_facility -I$GUROBI_HOME/include/ -L$GUROBI_HOME/lib -std=c++11 -lgurobi_c++ -l${1}
clang++ -O2 lqcp.cpp -o cpp_lqcp -I$GUROBI_HOME/include/ -L$GUROBI_HOME/lib -std=c++11 -lgurobi_c++ -l${1}
