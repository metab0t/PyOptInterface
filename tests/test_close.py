from pyoptinterface import gurobi, xpress, copt, mosek

envs = []
models = []
if gurobi.is_library_loaded():
    envs.append(gurobi.Env)
    models.append(gurobi.Model)
if xpress.is_library_loaded():
    envs.append(xpress.Env)
    models.append(xpress.Model)
if copt.is_library_loaded():
    envs.append(copt.Env)
    models.append(copt.Model)
if mosek.is_library_loaded():
    envs.append(mosek.Env)
    models.append(mosek.Model)


def test_close():
    for env, model in zip(envs, models):
        env_instance = env()
        model_instance = model(env_instance)

        model_instance.close()
        env_instance.close()
