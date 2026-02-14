import pytest
from pytest import approx

from pyoptinterface import knitro
import pyoptinterface as poi


pytestmark = pytest.mark.skipif(
    not knitro.is_library_loaded(), reason="KNITRO library is not loaded"
)


def test_new_model_without_env():
    """Test creating a model without an environment (default behavior)."""
    model = knitro.Model()

    x = model.add_variable(lb=0.0, ub=10.0)
    y = model.add_variable(lb=0.0, ub=10.0)

    model.set_objective(x + y, poi.ObjectiveSense.Minimize)
    model.add_linear_constraint(x + y, poi.ConstraintSense.GreaterEqual, 5.0)

    model.optimize()

    status = model.get_model_attribute(poi.ModelAttribute.TerminationStatus)
    assert status == poi.TerminationStatusCode.OPTIMAL

    x_val = model.get_value(x)
    y_val = model.get_value(y)
    assert x_val + y_val == approx(5.0)


def test_new_model_with_env():
    """Test creating a model with an environment."""
    env = knitro.Env()
    model = knitro.Model(env=env)

    x = model.add_variable(lb=0.0, ub=10.0)
    y = model.add_variable(lb=0.0, ub=10.0)

    model.set_objective(x + y, poi.ObjectiveSense.Minimize)
    model.add_linear_constraint(x + y, poi.ConstraintSense.GreaterEqual, 5.0)

    model.optimize()

    status = model.get_model_attribute(poi.ModelAttribute.TerminationStatus)
    assert status == poi.TerminationStatusCode.OPTIMAL

    x_val = model.get_value(x)
    y_val = model.get_value(y)
    assert x_val + y_val == approx(5.0)


def test_multiple_models_single_env():
    """Test creating multiple models sharing the same environment."""
    env = knitro.Env()

    model1 = knitro.Model(env=env)
    model2 = knitro.Model(env=env)

    x1 = model1.add_variable(lb=0.0, ub=10.0)
    model1.set_objective(x1, poi.ObjectiveSense.Minimize)
    model1.add_linear_constraint(x1, poi.ConstraintSense.GreaterEqual, 3.0)
    model1.optimize()

    x2 = model2.add_variable(lb=0.0, ub=10.0)
    model2.set_objective(x2, poi.ObjectiveSense.Minimize)
    model2.add_linear_constraint(x2, poi.ConstraintSense.GreaterEqual, 5.0)
    model2.optimize()

    assert model1.get_value(x1) == approx(3.0)
    assert model2.get_value(x2) == approx(5.0)


def test_multiple_models_multiple_envs():
    """Test creating multiple models each with its own environment."""
    env1 = knitro.Env()
    env2 = knitro.Env()

    model1 = knitro.Model(env=env1)
    model2 = knitro.Model(env=env2)

    # Set up and solve model1
    x1 = model1.add_variable(lb=0.0, ub=10.0)
    model1.set_objective(x1, poi.ObjectiveSense.Minimize)
    model1.add_linear_constraint(x1, poi.ConstraintSense.GreaterEqual, 4.0)
    model1.optimize()

    # Set up and solve model2
    x2 = model2.add_variable(lb=0.0, ub=10.0)
    model2.set_objective(x2, poi.ObjectiveSense.Minimize)
    model2.add_linear_constraint(x2, poi.ConstraintSense.GreaterEqual, 6.0)
    model2.optimize()

    assert model1.get_value(x1) == approx(4.0)
    assert model2.get_value(x2) == approx(6.0)


def test_env_lifetime():
    """Test that environment properly manages its lifetime."""
    env = knitro.Env()
    model = knitro.Model(env=env)

    x = model.add_variable(lb=0.0, ub=10.0)
    model.set_objective(x, poi.ObjectiveSense.Minimize)
    model.add_linear_constraint(x, poi.ConstraintSense.GreaterEqual, 2.0)
    model.optimize()

    assert model.get_value(x) == approx(2.0)

    model.close()

    try:
        del env
    except Exception as e:
        pytest.fail(f"Deleting environment raised an error: {e}")


def test_env_close():
    """Test that env.close() properly releases the license."""
    env = knitro.Env()

    model = knitro.Model(env=env)
    x = model.add_variable(lb=0.0, ub=10.0)
    model.set_objective(x, poi.ObjectiveSense.Minimize)
    model.add_linear_constraint(x, poi.ConstraintSense.GreaterEqual, 1.0)
    model.optimize()
    assert model.get_value(x) == approx(1.0)
    model.close()

    try:
        env.close()
    except Exception as e:
        pytest.fail(f"env.close() raised an error: {e}")


def test_init_with_env():
    """Test using init method with an environment."""
    env = knitro.Env()
    model = knitro.Model()
    model.init(env)
    x = model.add_variable(lb=0.0, ub=10.0)
    model.set_objective(x, poi.ObjectiveSense.Minimize)
    model.add_linear_constraint(x, poi.ConstraintSense.GreaterEqual, 1.0)
    model.optimize()

    assert model.get_value(x) == approx(1.0)

    model.close()
    del model
    del env


def test_model_with_empty_env():
    """Test that creating a model with an empty environment raises an error."""
    env = knitro.Env(empty=True)
    assert env.is_empty

    with pytest.raises(RuntimeError, match="Empty environment"):
        knitro.Model(env=env)

def test_model_init_with_empty_env_after_start():
    """Test that initializing a model with an empty environment after starting raises an error."""
    env = knitro.Env(empty=True)
    assert env.is_empty

    env.start()
    assert knitro.Model(env=env) is not None
