from .gurobi_model_ext import RawModel, Env, GRB
from .attributes import (
    VariableAttribute,
    ModelAttribute,
    var_attr_type_map,
    default_variable_attribute_type,
    default_model_attribute_type,
)
from .core_ext import VariableDomain
from .ctypes_helper import pycapsule_to_cvoidp

DEFAULT_ENV = None


def init_default_env():
    global DEFAULT_ENV
    if DEFAULT_ENV is None:
        DEFAULT_ENV = Env()


variable_attribute_name_map = {
    VariableAttribute.Value: "X",
    VariableAttribute.LowerBound: "LB",
    VariableAttribute.UpperBound: "UB",
    VariableAttribute.PrimalStart: "Start",
    VariableAttribute.Domain: "VType",
    VariableAttribute.Name: "VarName",
}


def translate_variable_attribute_name(attribute: VariableAttribute) -> str:
    name = variable_attribute_name_map.get(attribute, None)
    if not name:
        raise ValueError(f"Unknown variable attribute: {attribute}")
    return name


variable_attribute_value_need_translate_type = {VariableDomain}

variable_attribute_value_to_gurobi_map = {
    VariableDomain.Binary: GRB.BINARY,
    VariableDomain.Integer: GRB.INTEGER,
    VariableDomain.Continuous: GRB.CONTINUOUS,
    VariableDomain.SemiContinuous: GRB.SEMICONT,
}

variable_attribute_gurobi_to_value_map = {
    v: k for k, v in variable_attribute_value_to_gurobi_map.items()
}

CHAR_TYPE = "char"
variable_attribute_query_type_map = var_attr_type_map | {
    VariableAttribute.Domain: CHAR_TYPE
}

model_attribute_name_map = {
    VariableDomain.Binary: GRB.BINARY,
    VariableDomain.Integer: GRB.INTEGER,
    VariableDomain.Continuous: GRB.CONTINUOUS,
    VariableDomain.SemiContinuous: GRB.SEMICONT,
}


class Model(RawModel):
    def __init__(self, env=None):
        if env is None:
            init_default_env()
            env = DEFAULT_ENV
        super().__init__(env)

    @property
    def raw_model_ptr(self):
        return pycapsule_to_cvoidp(self.get_raw_model())

    def get_variable_attribute(self, variable, attribute):
        value_type = default_variable_attribute_type(attribute)
        query_type = variable_attribute_query_type_map[attribute]
        assert query_type in (int, float, str, CHAR_TYPE)

        query_name = translate_variable_attribute_name(attribute)
        get_function_map = {
            int: self.get_variable_raw_attribute_int,
            float: self.get_variable_raw_attribute_double,
            str: self.get_variable_raw_attribute_string,
            CHAR_TYPE: self.get_variable_raw_attribute_char,
        }
        query_function = get_function_map[query_type]
        value = query_function(variable, query_name)

        if value_type in variable_attribute_value_need_translate_type:
            value = variable_attribute_gurobi_to_value_map[value]
        return value

    def set_variable_attribute(self, variable, attribute, value):
        value_type = default_variable_attribute_type(attribute)
        query_type = variable_attribute_query_type_map[attribute]
        assert query_type in (int, float, str, CHAR_TYPE)

        query_name = translate_variable_attribute_name(attribute)
        set_function_map = {
            int: self.set_variable_raw_attribute_int,
            float: self.set_variable_raw_attribute_double,
            str: self.set_variable_raw_attribute_string,
            CHAR_TYPE: self.set_variable_raw_attribute_char,
        }
        set_function = set_function_map[query_type]

        if value_type in variable_attribute_value_need_translate_type:
            value = variable_attribute_value_to_gurobi_map[value]
        set_function(variable, query_name, value)

    def get_model_attribute(self, attribute):
        pass
