import pyoptinterface as poi


def test_model_attribute_termination_before_solve(model_interface_oneshot):
    """Test termination status before solving."""
    model = model_interface_oneshot
    x = model.add_variable(lb=0.0, ub=10.0)
    model.set_objective(x, poi.ObjectiveSense.Minimize)

    status = model.get_model_attribute(poi.ModelAttribute.TerminationStatus)
    assert status == poi.TerminationStatusCode.OPTIMIZE_NOT_CALLED
