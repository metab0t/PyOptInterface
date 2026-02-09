from typing import IO
from pathlib import Path


import pyoptinterface as poi
from pyoptinterface import gurobi, copt, mosek, highs, ipopt


def attribute_support_table(f, attribute_enum):
    results = []
    for attribute in attribute_enum:
        support_get = f(attribute)
        support_set = f(attribute, settable=True)
        results.append((attribute, support_get, support_set))
    return results


emoji_map = {True: "✅", False: "❌"}


def print_table(io: IO[str], results):
    io.write(""":::{list-table}
:header-rows: 1

*   - Attribute
    - Get
    - Set
""")

    for attribute, support_get, support_set in results:
        io.write(f"*   - {attribute.name}\n")
        io.write(f"    - {emoji_map[support_get]}\n")
        io.write(f"    - {emoji_map[support_set]}\n")

    io.write(":::\n\n")


io_pairs = [
    (gurobi.Model, "gurobi"),
    (copt.Model, "copt"),
    (mosek.Model, "mosek"),
    (highs.Model, "highs"),
    (ipopt.Model, "ipopt"),
]

rootdir = Path(__file__).parent.parent
docsource_dir = rootdir / "docs" / "source" / "attribute"

for model, name in io_pairs:
    path = docsource_dir / f"{name}.md"
    with open(path, "w", encoding="utf-8") as io:
        io.write(f"### Supported [model attribute](#pyoptinterface.ModelAttribute)\n\n")
        print_table(
            io,
            attribute_support_table(model.supports_model_attribute, poi.ModelAttribute),
        )
        io.write(
            f"### Supported [variable attribute](#pyoptinterface.VariableAttribute)\n\n"
        )
        print_table(
            io,
            attribute_support_table(
                model.supports_variable_attribute, poi.VariableAttribute
            ),
        )
        io.write(
            f"### Supported [constraint attribute](#pyoptinterface.ConstraintAttribute)\n\n"
        )
        print_table(
            io,
            attribute_support_table(
                model.supports_constraint_attribute, poi.ConstraintAttribute
            ),
        )
