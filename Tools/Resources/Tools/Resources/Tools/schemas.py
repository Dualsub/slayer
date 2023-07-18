import numpy as np

SCEHMAS = {
    "Texture": {
        "extentions": [
            "material"
        ],
        "schema": {
            "width": np.uint32,
            "height": np.uint32,
            "channels": np.uint32,
            "target": np.uint32,
            "data": list[np.uint8]
        },
    },
    "Shader": {
        "extentions": [
            "shader"
        ],
        "schema": {
            "vsSource": str,
            "fsSource": str
        }
    },
    "Material": {
        "extentions": [
            "material"
        ],
        "schema": {
            "textures": "MaterialTexture[]"
        },
        "dependencies": [
            "Texture"
        ]
    },
    "MaterialTexture": {
        "schema": {
            "type": "u8",
            "texture": "aid"
        },
        "dependencies": [
            "Texture"
        ]
    },
    "Mesh": {
        "schema": {
            "vertices": "f32[]",
            "indices": "u32[]"
        },

    },
    "Model": {
        "schema": {
            "meshes": "Mesh[]",
        }
    },
}