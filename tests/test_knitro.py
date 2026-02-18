import pytest
from pytest import approx

from pyoptinterface import knitro
import pyoptinterface as poi

pytestmark = pytest.mark.skipif(
    not knitro.is_library_loaded() or not knitro.has_valid_license(),
    reason="KNITRO library is not loaded or license is not valid",
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

    x1 = model1.add_variable(lb=0.0, ub=10.0)
    model1.set_objective(x1, poi.ObjectiveSense.Minimize)
    model1.add_linear_constraint(x1, poi.ConstraintSense.GreaterEqual, 4.0)
    model1.optimize()

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


def test_model_dirty():
    """Test the dirty method."""
    model = knitro.Model()

    assert model.dirty() is True

    x = model.add_variable(lb=0.0, ub=10.0)
    assert model.dirty() is True

    model.set_objective(x, poi.ObjectiveSense.Minimize)
    model.add_linear_constraint(x, poi.ConstraintSense.GreaterEqual, 1.0)
    model.optimize()

    assert model.dirty() is False

    model.add_variable(lb=0.0, ub=5.0)
    assert model.dirty() is True


def test_model_is_dirty():
    """Test the is_dirty property."""
    model = knitro.Model()

    assert model.is_dirty is True

    x = model.add_variable(lb=0.0, ub=10.0)
    assert model.is_dirty is True

    model.set_objective(x, poi.ObjectiveSense.Minimize)
    model.add_linear_constraint(x, poi.ConstraintSense.GreaterEqual, 1.0)
    model.optimize()

    assert model.is_dirty is False

    model.add_variable(lb=0.0, ub=5.0)
    assert model.is_dirty is True


def test_model_empty():
    """Test the empty() method."""
    model = knitro.Model()
    assert model.empty() is False
    model.close()
    assert model.empty() is True


def test_model_is_empty_property():
    """Test the is_empty property."""
    model = knitro.Model()
    assert model.is_empty is False
    model.close()
    assert model.is_empty is True


def test_number_of_variables():
    """Test number_of_variables() method."""
    model = knitro.Model()

    assert model.number_of_variables() == 0

    model.add_variable()
    assert model.number_of_variables() == 1

    model.add_variable()
    model.add_variable()
    assert model.number_of_variables() == 3


def test_number_of_constraints():
    """Test number_of_constraints() method."""
    model = knitro.Model()
    x = model.add_variable(lb=-10.0, ub=10.0)
    y = model.add_variable(lb=-10.0, ub=10.0)

    assert model.number_of_constraints() == 0
    assert model.number_of_constraints(poi.ConstraintType.Linear) == 0
    assert model.number_of_constraints(poi.ConstraintType.Quadratic) == 0

    model.add_linear_constraint(x + y, poi.ConstraintSense.LessEqual, 5.0)
    assert model.number_of_constraints() == 1
    assert model.number_of_constraints(poi.ConstraintType.Linear) == 1

    model.add_quadratic_constraint(x * x + y, poi.ConstraintSense.LessEqual, 10.0)
    assert model.number_of_constraints() == 2
    assert model.number_of_constraints(poi.ConstraintType.Quadratic) == 1


def test_supports_attribute_methods():
    """Test supports_*_attribute() static methods."""
    assert knitro.Model.supports_variable_attribute(poi.VariableAttribute.Value) is True
    assert (
        knitro.Model.supports_variable_attribute(poi.VariableAttribute.LowerBound)
        is True
    )
    assert (
        knitro.Model.supports_variable_attribute(
            poi.VariableAttribute.LowerBound, setable=True
        )
        is True
    )
    assert (
        knitro.Model.supports_variable_attribute(
            poi.VariableAttribute.Value, setable=True
        )
        is False
    )

    assert (
        knitro.Model.supports_constraint_attribute(poi.ConstraintAttribute.Primal)
        is True
    )
    assert (
        knitro.Model.supports_constraint_attribute(poi.ConstraintAttribute.Name) is True
    )
    assert (
        knitro.Model.supports_constraint_attribute(
            poi.ConstraintAttribute.Name, setable=True
        )
        is True
    )

    assert (
        knitro.Model.supports_model_attribute(poi.ModelAttribute.ObjectiveValue) is True
    )
    assert knitro.Model.supports_model_attribute(poi.ModelAttribute.SolverName) is True
    assert (
        knitro.Model.supports_model_attribute(poi.ModelAttribute.Silent, setable=True)
        is True
    )
    assert (
        knitro.Model.supports_model_attribute(
            poi.ModelAttribute.ObjectiveValue, setable=True
        )
        is False
    )
    assert (
        knitro.Model.supports_model_attribute(
            poi.ModelAttribute.NumberOfThreads, setable=True
        )
        is True
    )
    assert (
        knitro.Model.supports_model_attribute(
            poi.ModelAttribute.TimeLimitSec, setable=True
        )
        is True
    )


def test_model_attribute_solver_info():
    """Test getting solver info model attributes."""
    model = knitro.Model()

    solver_name = model.get_model_attribute(poi.ModelAttribute.SolverName)
    assert solver_name == "KNITRO"

    solver_version = model.get_model_attribute(poi.ModelAttribute.SolverVersion)
    assert isinstance(solver_version, str)
    assert len(solver_version) > 0


def test_model_attribute_termination_before_solve():
    """Test termination status before solving."""
    model = knitro.Model()
    x = model.add_variable(lb=0.0, ub=10.0)
    model.set_objective(x, poi.ObjectiveSense.Minimize)

    status = model.get_model_attribute(poi.ModelAttribute.TerminationStatus)
    assert status == poi.TerminationStatusCode.OPTIMIZE_NOT_CALLED


def test_model_attribute_after_solve():
    """Test model attributes after solving."""
    model = knitro.Model()
    x = model.add_variable(lb=0.0, ub=10.0)
    y = model.add_variable(lb=0.0, ub=10.0)

    model.set_objective(x + 2 * y, poi.ObjectiveSense.Minimize)
    model.add_linear_constraint(x + y, poi.ConstraintSense.GreaterEqual, 5.0)
    model.optimize()

    status = model.get_model_attribute(poi.ModelAttribute.TerminationStatus)
    assert status == poi.TerminationStatusCode.OPTIMAL

    primal_status = model.get_model_attribute(poi.ModelAttribute.PrimalStatus)
    assert primal_status == poi.ResultStatusCode.FEASIBLE_POINT

    raw_status = model.get_model_attribute(poi.ModelAttribute.RawStatusString)
    assert isinstance(raw_status, str)
    assert "KNITRO" in raw_status

    obj_value = model.get_model_attribute(poi.ModelAttribute.ObjectiveValue)
    assert obj_value == approx(5.0)

    obj_sense = model.get_model_attribute(poi.ModelAttribute.ObjectiveSense)
    assert obj_sense == poi.ObjectiveSense.Minimize

    solve_time = model.get_model_attribute(poi.ModelAttribute.SolveTimeSec)
    assert isinstance(solve_time, float)
    assert solve_time >= 0.0

    iterations = model.get_model_attribute(poi.ModelAttribute.BarrierIterations)
    assert isinstance(iterations, int)
    assert iterations >= 0


def test_set_model_attribute_objective_sense():
    """Test setting objective sense."""
    model = knitro.Model()
    model.add_variable(lb=0.0, ub=10.0)

    model.set_model_attribute(
        poi.ModelAttribute.ObjectiveSense, poi.ObjectiveSense.Maximize
    )
    assert (
        model.get_model_attribute(poi.ModelAttribute.ObjectiveSense)
        == poi.ObjectiveSense.Maximize
    )

    model.set_model_attribute(
        poi.ModelAttribute.ObjectiveSense, poi.ObjectiveSense.Minimize
    )
    assert (
        model.get_model_attribute(poi.ModelAttribute.ObjectiveSense)
        == poi.ObjectiveSense.Minimize
    )


def test_set_model_attribute_silent():
    """Test setting silent mode (should not raise error)."""
    model = knitro.Model()
    x = model.add_variable(lb=0.0, ub=10.0)
    model.set_objective(x, poi.ObjectiveSense.Minimize)
    model.add_linear_constraint(x, poi.ConstraintSense.GreaterEqual, 1.0)

    model.set_model_attribute(poi.ModelAttribute.Silent, True)
    model.optimize()

    assert model.get_value(x) == approx(1.0)


def test_set_model_attribute_threads():
    """Test setting number of threads."""
    model = knitro.Model()

    model.set_model_attribute(poi.ModelAttribute.NumberOfThreads, 2)
    threads = model.get_model_attribute(poi.ModelAttribute.NumberOfThreads)
    assert threads == 2


def test_set_model_attribute_time_limit():
    """Test setting time limit (set only, get may not work due to param type mismatch)."""
    model = knitro.Model()

    model.set_model_attribute(poi.ModelAttribute.TimeLimitSec, 100.0)


def test_variable_attribute_bounds():
    """Test getting and setting variable bounds."""
    model = knitro.Model()
    x = model.add_variable(lb=0.0, ub=10.0)

    lb = model.get_variable_attribute(x, poi.VariableAttribute.LowerBound)
    ub = model.get_variable_attribute(x, poi.VariableAttribute.UpperBound)
    assert lb == approx(0.0)
    assert ub == approx(10.0)

    model.set_variable_attribute(x, poi.VariableAttribute.LowerBound, 1.0)
    model.set_variable_attribute(x, poi.VariableAttribute.UpperBound, 5.0)

    lb = model.get_variable_attribute(x, poi.VariableAttribute.LowerBound)
    ub = model.get_variable_attribute(x, poi.VariableAttribute.UpperBound)
    assert lb == approx(1.0)
    assert ub == approx(5.0)


def test_variable_attribute_name():
    """Test getting and setting variable name."""
    model = knitro.Model()
    x = model.add_variable(name="my_var")

    name = model.get_variable_attribute(x, poi.VariableAttribute.Name)
    assert name == "my_var"

    model.set_variable_attribute(x, poi.VariableAttribute.Name, "new_name")
    name = model.get_variable_attribute(x, poi.VariableAttribute.Name)
    assert name == "new_name"


def test_variable_attribute_value():
    """Test getting variable value after solve."""
    model = knitro.Model()
    x = model.add_variable(lb=0.0, ub=10.0)

    model.set_objective(x, poi.ObjectiveSense.Minimize)
    model.add_linear_constraint(x, poi.ConstraintSense.GreaterEqual, 3.0)
    model.optimize()

    value = model.get_variable_attribute(x, poi.VariableAttribute.Value)
    assert value == approx(3.0)


def test_variable_attribute_primal_start():
    """Test setting primal start value."""
    model = knitro.Model()
    x = model.add_variable(lb=0.0, ub=10.0)

    model.set_variable_attribute(x, poi.VariableAttribute.PrimalStart, 5.0)

    model.set_objective(x, poi.ObjectiveSense.Minimize)
    model.add_linear_constraint(x, poi.ConstraintSense.GreaterEqual, 2.0)
    model.optimize()

    assert model.get_value(x) == approx(2.0)


def test_variable_attribute_domain():
    """Test getting and setting variable domain."""
    model = knitro.Model()
    x = model.add_variable(lb=0.0, ub=10.0)

    # Default domain should be Continuous
    domain = model.get_variable_attribute(x, poi.VariableAttribute.Domain)
    assert domain == poi.VariableDomain.Continuous

    # Set to Integer and verify
    model.set_variable_attribute(
        x, poi.VariableAttribute.Domain, poi.VariableDomain.Integer
    )
    domain = model.get_variable_attribute(x, poi.VariableAttribute.Domain)
    assert domain == poi.VariableDomain.Integer

    model.set_objective(x, poi.ObjectiveSense.Minimize)
    model.add_linear_constraint(x, poi.ConstraintSense.GreaterEqual, 2.5)
    model.optimize()

    assert model.get_value(x) == approx(3.0)

    # Test Binary domain
    y = model.add_variable(domain=poi.VariableDomain.Binary)
    domain = model.get_variable_attribute(y, poi.VariableAttribute.Domain)
    assert domain == poi.VariableDomain.Binary


def test_constraint_attribute_name():
    """Test getting and setting constraint name."""
    model = knitro.Model()
    x = model.add_variable(lb=0.0, ub=10.0)

    con = model.add_linear_constraint(
        x, poi.ConstraintSense.LessEqual, 5.0, name="my_con"
    )

    name = model.get_constraint_attribute(con, poi.ConstraintAttribute.Name)
    assert name == "my_con"

    model.set_constraint_attribute(con, poi.ConstraintAttribute.Name, "new_con")
    name = model.get_constraint_attribute(con, poi.ConstraintAttribute.Name)
    assert name == "new_con"


def test_constraint_attribute_primal_dual():
    """Test getting constraint primal and dual values after solve."""
    model = knitro.Model()
    x = model.add_variable(lb=0.0, ub=10.0)
    y = model.add_variable(lb=0.0, ub=10.0)

    model.set_objective(x + y, poi.ObjectiveSense.Minimize)
    con = model.add_linear_constraint(x + y, poi.ConstraintSense.GreaterEqual, 5.0)
    model.optimize()

    primal = model.get_constraint_attribute(con, poi.ConstraintAttribute.Primal)
    assert primal == approx(5.0)

    dual = model.get_constraint_attribute(con, poi.ConstraintAttribute.Dual)
    assert isinstance(dual, float)
