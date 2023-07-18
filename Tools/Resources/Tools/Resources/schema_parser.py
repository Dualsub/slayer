# This script parses schema files.
import json
import numpy as np
type_mapping = {
    "i32" : np.int32,
    "u32" : np.uint32,
    "u8": np.uint8,
    "i8": np.int8,
    "aid": np.uint64,
    "f32": np.float32,
    "f64": np.float64,
    ""
}
    
def parse_schema(schema_file):
    """Parse a schema file and return a list of tables."""
    
    types = {}
    raw_data = {}
    with open(schema_file, 'r') as f:
        raw_data = json.load(f)

    if 'schema' not in raw_data:
        raise Exception('Schema file does not contain a schema.')
    
    schema = raw_data['schema']
        
    deffered_types = []
        
    for type_name, type_desc in raw_data.items():

        # If it contains uppercase letters, it's a type.
        is_custom_type = type_name != type_name.lower()
        if is_custom_type:
            deffered_types.append((type_name, type_desc))
            continue

        is_array = type_desc.endswith('[]')
        type_str = type_desc if not is_array else type_desc[:-2]
        bits = int(type_str[1:3])

        types[type_name] = {
            "type": type_mapping[type_str],
            "bytes": bits // 8,
            "is_array": is_array
        }