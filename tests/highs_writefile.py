import pyoptinterface as poi
from pyoptinterface import highs

model = highs.Model()

x = model.add_m_variables(10, lb=0.0, ub=10.0)
model.add_linear_constraint(poi.quicksum(x) >= 20.0)

model.set_objective(poi.quicksum(v * v for v in x))
model.optimize()

model.write("highs_test.sol")
model.write("highs_test_pretty.sol", pretty=True)
