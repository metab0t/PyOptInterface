import pytest

import pyoptinterface_nb as poi

model_interface_dict = {}
try:
    import pyoptinterface_nb.gurobi as gurobi
    model_interface_dict['gurobi'] = gurobi.Model
except Exception:
    pass

try:
    import pyoptinterface_nb.copt as copt
    model_interface_dict['copt'] = copt.Model
except Exception:
    pass

@pytest.fixture(params=model_interface_dict.keys())
def model_interface(request):
    name = request.param
    model_interface_class = model_interface_dict[name]
    return model_interface_class()