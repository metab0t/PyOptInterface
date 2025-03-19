import pyoptinterface as poi
from pyoptinterface import highs

model = highs.Model()

x = model.add_m_variables(10, name="x", lb=0.0, ub=10.0)
model.add_linear_constraint(poi.quicksum(x) >= 20.0, name="con")

model.set_objective(poi.quicksum(v * v for v in x))
model.optimize()

model.write("highs_test.lp")
model.write("highs_test.sol")
model.write("highs_test_pretty.sol", pretty=True)
