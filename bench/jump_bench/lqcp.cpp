// Copyright (c) 2022: Miles Lubin and contributors
//
// Use of this source code is governed by an MIT-style license that can be found
// in the LICENSE.md file or at https://opensource.org/licenses/MIT.

#define _GLIBCXX_USE_CXX11_ABI 0
#include "gurobi_c++.h"
#include <cmath>
#include <cstdlib>
#include <vector>
using namespace std;

double yt(int j, double dx) { return 0.5 * (1 - (j * dx) * (j * dx)); }

int main(int argc, char **argv) {
  int n = atoi(argv[1]);
  int m = n;
  int n1 = n - 1;
  int m1 = m - 1;
  double dx = 1.0 / n;
  double T = 1.58;
  double dt = T / m;
  double h2 = dx * dx;
  double a = 0.001;
  GRBEnv env;
  GRBModel model(env);
  model.getEnv().set(GRB_DoubleParam_TimeLimit, 0);
  // Add variables
  vector<vector<GRBVar>> y(m + 1, vector<GRBVar>(n + 1));
  for (int i = 0; i <= m; i++) {
    for (int j = 0; j <= n; j++) {
      y[i][j] = model.addVar(0.0, 1.0, 0.0, GRB_CONTINUOUS);
    }
  }
  vector<GRBVar> u(m + 1);
  for (int i = 1; i <= m; i++) {
    u[i] = model.addVar(-1.0, 1.0, 0.0, GRB_CONTINUOUS);
  }
  model.update();
  // objective
  GRBQuadExpr term1;
  term1 += (y[m][0] - yt(0, dx)) * (y[m][0] - yt(0, dx));
  for (int j = 1; j <= n - 1; j++) {
    term1 += 2 * (y[m][j] - yt(j, dx)) * (y[m][j] - yt(j, dx));
  }
  term1 += (y[m][n] - yt(n, dx)) * (y[m][n] - yt(n, dx));
  GRBQuadExpr term2;
  for (int i = 1; i <= m - 1; i++) {
    term2 += 2 * u[i] * u[i];
  }
  term2 += u[m] * u[m];
  model.setObjective(0.25 * dx * term1 + 0.25 * a * dt * term2);
  // PDE
  for (int i = 0; i <= m1; i++) {
    for (int j = 1; j <= n1; j++) {
      model.addConstr(h2 * (y[i + 1][j] - y[i][j]) ==
                      dt * 0.5 *
                          (y[i][j - 1] - 2 * y[i][j] + y[i][j + 1] +
                           y[i + 1][j - 1] - 2 * y[i + 1][j] +
                           y[i + 1][j + 1]));
    }
  }
  // IC
  for (int j = 0; j <= n; j++) {
    model.addConstr(y[0][j] == 0);
  }
  // BC
  for (int i = 1; i <= m; i++) {
    model.addConstr(y[i][2] - 4 * y[i][1] + 3 * y[i][0] == 0);
    model.addConstr((y[i][n - 2] - 4 * y[i][n1] + 3 * y[i][n]) / (2 * dx) ==
                    u[i] - y[i][n]);
  }
  model.optimize();
  return 0;
}
