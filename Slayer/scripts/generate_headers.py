import os
import sys

def generate_headers(namespace, types, definition_str) -> str:
    header_str = "#pragma once\n\n"
    header_str += "#include \"Core/Core.h\"\n\n"
    header_str += "#include \"Serialization/Serialization.h\"\n\n"
    header_str += "namespace {} {{".format(namespace)

    # for type in types:
    #     header_str += "\n\tstruct {};".format(type)

    header_str += definition_str

    header_str += "\n}\n\n"

    header_str += "#define GAME_COMPONENTS \\\n"

    header_str += ", \\\n".join(["\t{}::{}".format(namespace, type) for type in types])

    return header_str
    

def parse_types(header_file_path) -> tuple[str, list, str]:
    namespace = ""
    types = []
    definition_str = ""

    with open(header_file_path, "r") as f:

        # Find struct definitions in namespace
        definition_str = f.read()
        first_bracket = definition_str.find("{")
        last_bracket = definition_str.rfind("}")

        if first_bracket != -1 and last_bracket != -1:
            definition_str = definition_str[first_bracket+1:last_bracket]
        else:
            definition_str = ""

        f.seek(0)
        for line in f:
            # Find namespace
            if line.startswith("//"):
                continue

            if not namespace:
                if "namespace" in line:
                    namespace = line.split("namespace")[1].split("{")[0].strip()
                    continue

            # Find types
            if "struct" in line:
                type = line.split("struct")[1].strip().split(" ")[0].strip()
                types.append(type)

    print("Found types: " + ", ".join(types), "in namespace", namespace)

    if namespace:
        namespace = namespace.strip()
    
    return namespace, types, definition_str

def main():
    # Arguments
    if len(sys.argv) != 3:
        print("Usage: generate_headers.py <search_path> <output_file_path>")
        return
    
    search_path = sys.argv[1]
    if not os.path.exists(search_path):
        print("Path does not exist: {}".format(search_path), "from", os.getcwd())
        return
    
    output_file_path = sys.argv[2]
    
    # Find file "Components.h"
    components_path = os.path.join(search_path, "Components.h")
    if not os.path.exists(components_path):
        print("Components.h not found")
        with open(components_path, "w") as f:
            f.write("")
        
    namespace, types, definition_str = parse_types(components_path)
    if not namespace:
        print("Namespace not found")
        return
    
    # Generate headers
    header_str = generate_headers(namespace, types, definition_str)

    # Write to file
    with open(output_file_path, "w") as f:
        f.write(header_str)

    print("Generated headers for {} types in {}".format(len(types), output_file_path))

if __name__ == "__main__":
    main()
    

    

    