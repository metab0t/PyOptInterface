import pytest
import pyoptinterface as poi
from pyoptinterface import ipopt, nl

pytestmark = pytest.mark.skipif(
    not ipopt.is_library_loaded(), reason="IPOPT library not available"
)

OPTIMIZE_RE = r"optimize\(\)"


def _build_simple_model():
    """Build a simple model: min x^2 s.t. x >= 0.5"""
    model = ipopt.Model()
    x = model.add_variable(lb=0.0, ub=10.0)
    model.set_objective(x**2)
    con = model.add_linear_constraint(x, poi.Geq, 0.5)
    return model, x, con


class TestUnoptimizedModelRaises:
    """Accessing variable/constraint/objective values before calling optimize() should raise."""

    def test_get_variable_value_before_optimize(self):
        model, x, _con = _build_simple_model()
        with pytest.raises(RuntimeError, match=OPTIMIZE_RE):
            model.get_value(x)

    def test_get_objective_value_before_optimize(self):
        model, _x, _con = _build_simple_model()
        with pytest.raises(RuntimeError, match=OPTIMIZE_RE):
            model.get_model_attribute(poi.ModelAttribute.ObjectiveValue)

    def test_get_constraint_primal_before_optimize(self):
        model, _x, con = _build_simple_model()
        with pytest.raises(RuntimeError, match=OPTIMIZE_RE):
            model.get_constraint_attribute(con, poi.ConstraintAttribute.Primal)

    def test_get_constraint_dual_before_optimize(self):
        model, _x, con = _build_simple_model()
        with pytest.raises(RuntimeError, match=OPTIMIZE_RE):
            model.get_constraint_attribute(con, poi.ConstraintAttribute.Dual)


class TestDirtyAfterModificationRaises:
    """After a successful optimize(), modifying the model (adding variables/constraints)
    should make it dirty and re-querying values should raise."""

    def test_dirty_after_add_variable(self):
        model, x, con = _build_simple_model()
        model.optimize()
        # Values should be accessible now
        model.get_value(x)
        model.get_model_attribute(poi.ModelAttribute.ObjectiveValue)

        # Add a new variable to make the model dirty
        model.add_variable(lb=0.0, ub=10.0)

        with pytest.raises(RuntimeError, match=OPTIMIZE_RE):
            model.get_value(x)
        with pytest.raises(RuntimeError, match=OPTIMIZE_RE):
            model.get_model_attribute(poi.ModelAttribute.ObjectiveValue)
        with pytest.raises(RuntimeError, match=OPTIMIZE_RE):
            model.get_constraint_attribute(con, poi.ConstraintAttribute.Primal)
        with pytest.raises(RuntimeError, match=OPTIMIZE_RE):
            model.get_constraint_attribute(con, poi.ConstraintAttribute.Dual)

    def test_dirty_after_add_linear_constraint(self):
        model, x, _con = _build_simple_model()
        model.optimize()
        model.get_value(x)

        # Add a linear constraint to make the model dirty
        model.add_linear_constraint(x, poi.Leq, 5.0)

        with pytest.raises(RuntimeError, match=OPTIMIZE_RE):
            model.get_value(x)
        with pytest.raises(RuntimeError, match=OPTIMIZE_RE):
            model.get_model_attribute(poi.ModelAttribute.ObjectiveValue)

    def test_dirty_after_add_quadratic_constraint(self):
        model, x, _con = _build_simple_model()
        model.optimize()
        model.get_value(x)

        # Add a quadratic constraint to make the model dirty
        model.add_quadratic_constraint(x**2, poi.Leq, 25.0)

        with pytest.raises(RuntimeError, match=OPTIMIZE_RE):
            model.get_value(x)
        with pytest.raises(RuntimeError, match=OPTIMIZE_RE):
            model.get_model_attribute(poi.ModelAttribute.ObjectiveValue)

    def test_dirty_after_add_nl_constraint(self):
        model, x, _con = _build_simple_model()
        model.optimize()
        model.get_value(x)

        # Add a nonlinear constraint to make the model dirty
        with nl.graph():
            model.add_nl_constraint(nl.exp(x), poi.Leq, 100.0)

        with pytest.raises(RuntimeError, match=OPTIMIZE_RE):
            model.get_value(x)
        with pytest.raises(RuntimeError, match=OPTIMIZE_RE):
            model.get_model_attribute(poi.ModelAttribute.ObjectiveValue)

    def test_reoptimize_clears_dirty(self):
        """After re-optimizing, values should be accessible again."""
        model, x, con = _build_simple_model()
        model.optimize()

        # Modify model
        model.add_linear_constraint(x, poi.Leq, 5.0)

        # Dirty – raises
        with pytest.raises(RuntimeError, match=OPTIMIZE_RE):
            model.get_value(x)

        # Re-optimize – should clear the dirty flag
        model.optimize()

        val = model.get_value(x)
        assert val == pytest.approx(0.5, abs=1e-6)
        obj = model.get_model_attribute(poi.ModelAttribute.ObjectiveValue)
        assert obj == pytest.approx(0.25, abs=1e-6)
        primal = model.get_constraint_attribute(con, poi.ConstraintAttribute.Primal)
        assert primal == pytest.approx(0.5, abs=1e-6)
