# include some common methods for solver intergaces


def _get_model_attribute(
    model, attribute, get_func_map, value_translate_map, error_callback
):
    get_function = get_func_map.get(attribute, None)
    if not get_function:
        raise error_callback(attribute)
    value = get_function(model)
    translate_function = value_translate_map.get(attribute, None)
    if translate_function:
        value = translate_function(value)
    return value


def _direct_get_model_attribute(model, attribute, get_func_map, error_callback):
    get_function = get_func_map.get(attribute, None)
    if not get_function:
        raise error_callback(attribute)
    value = get_function(model)
    return value


def _set_model_attribute(
    model,
    attribute,
    value,
    set_func_map,
    value_translate_map,
    error_callback,
):
    translate_function = value_translate_map.get(attribute, None)
    if translate_function:
        value = translate_function(value)
    set_function = set_func_map.get(attribute, None)
    if not set_function:
        raise error_callback(attribute)
    set_function(model, value)


def _direct_set_model_attribute(
    model,
    attribute,
    value,
    set_func_map,
    error_callback,
):
    set_function = set_func_map.get(attribute, None)
    if not set_function:
        raise error_callback(attribute)
    set_function(model, value)


def _get_entity_attribute(
    model, entity, attribute, get_func_map, value_translate_map, error_callback
):
    get_function = get_func_map.get(attribute, None)
    if not get_function:
        raise error_callback(attribute)
    value = get_function(model, entity)
    translate_function = value_translate_map.get(attribute, None)
    if translate_function:
        value = translate_function(value)
    return value


def _direct_get_entity_attribute(
    model, entity, attribute, get_func_map, error_callback
):
    get_function = get_func_map.get(attribute, None)
    if not get_function:
        raise error_callback(attribute)
    value = get_function(model, entity)
    return value


def _set_entity_attribute(
    model,
    entity,
    attribute,
    value,
    set_func_map,
    value_translate_map,
    error_callback,
):
    translate_function = value_translate_map.get(attribute, None)
    if translate_function:
        value = translate_function(value)
    set_function = set_func_map.get(attribute, None)
    if not set_function:
        raise error_callback(attribute)
    set_function(model, entity, value)


def _direct_set_entity_attribute(
    model,
    entity,
    attribute,
    value,
    set_func_map,
    error_callback,
):
    set_function = set_func_map.get(attribute, None)
    if not set_function:
        raise error_callback(attribute)
    set_function(model, entity, value)
