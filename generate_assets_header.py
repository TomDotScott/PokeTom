import yaml
from pathlib import Path

HEADER_TEMPLATE = """// THIS FILE IS AUTO-GENERATED. DO NOT EDIT. DO NOT COMMIT TO SOURCE CONTROL.
// Simply edit assets.yaml and re-run generate.bat!
#pragma once
#include <string_view>
#include <array>

enum class ResourceType {{
{resource_type_enum}
    COUNT
}};

struct ResourceEntry {{
    std::string_view resourceID;
    std::string_view resourcePath;
}};

extern const std::array<std::string_view, static_cast<size_t>(ResourceType::COUNT)> ResourceTypeNames;

{extern_arrays}

std::string_view get_resource(ResourceType type, std::string_view name);

#define GET_RESOURCE(type, name) get_resource(ResourceType::type, name)

{macros}
"""

CPP_TEMPLATE = """// THIS FILE IS AUTO-GENERATED. DO NOT EDIT. DO NOT COMMIT TO SOURCE CONTROL.
// Simply edit assets.yaml and re-run generate.bat!
#include "Resources.hpp"

const std::array<std::string_view, static_cast<size_t>(ResourceType::COUNT)> ResourceTypeNames = {{
{resource_type_names}
}};

{arrays}

std::string_view get_resource(ResourceType type, std::string_view name) {{
    switch (type) {{
{switch_cases}
    }}
    return {{}};
}}
"""

def main():
    input_yaml = Path("data/assets.yaml")
    out_header = Path("src/Engine/CodeGen/Resources.hpp")
    out_cpp = Path("src/Engine/CodeGen/Resources.cpp")

    data = yaml.safe_load(input_yaml.read_text())

    resource_types = list(data.keys())

    resource_type_enum = ""
    for t in resource_types:
        resource_type_enum += f"    {t.upper()},\n"

    resource_type_names = ""
    for t in resource_types:
        resource_type_names += f'    "{t}",\n'

    extern_arrays = ""
    arrays_cpp = ""
    switch_cases = ""
    macro_defs = ""

    for t in resource_types:
        entries = data[t]

        arr_name = f"{t}_resources"
        size = len(entries)

        # extern declaration
        extern_arrays += f"extern const std::array<ResourceEntry, {size}> {arr_name};\n"

        # C++ array
        arrays_cpp += f"const std::array<ResourceEntry, {size}> {arr_name} = {{\n"
        for e in entries:
            arrays_cpp += f'    ResourceEntry{{"{e["name"]}", "{e["source"]}"}},\n'
        arrays_cpp += "};\n\n"

        # switch-case clause
        switch_cases += f"        case ResourceType::{t.upper()}:\n"
        switch_cases += f"            for (const auto& e : {arr_name}) {{ if (e.resourceID == name) return e.resourcePath; }}\n"
        switch_cases += "            break;\n"

        # if the type is plural, remove the `s`
        name = t.upper()[:-1] if t.upper().endswith('S') else t.upper()
        macro_defs += f"#define GET_{name}_PATH(name) GET_RESOURCE({t.upper()}, name)\n"

    header = HEADER_TEMPLATE.format(
        resource_type_enum=resource_type_enum.rstrip(),
        extern_arrays=extern_arrays.rstrip(),
        macros=macro_defs.rstrip()
    )

    cpp = CPP_TEMPLATE.format(
        resource_type_names=resource_type_names.rstrip(),
        arrays=arrays_cpp.rstrip(),
        switch_cases=switch_cases.rstrip()
    )

    out_header.write_text(header)
    out_cpp.write_text(cpp)

    print(f"Generated {out_header} and {out_cpp}!")

if __name__ == "__main__":
    main()
