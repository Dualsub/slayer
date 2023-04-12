import impasse as assimp
import hashlib

TEXTURE_2D_TARGET = 0x0DE1
TEXTURE_CUBE_MAP_TARGET = 0x8513

ASSET_ID_SIZE = 8
ASSET_TYPE_SIZE = 2
UINT32_SIZE = 4
MAGIC = "SLPCK\0"

MATERIAL_TEXTURE_TYPES = {
    "albedo": 4,
    "normal": 5,
    "metallic": 6,
    "roughness": 7,
    "ambient": 8
}

ASSET_TYPES = {
    "none": 0,
    "texture": 1,
    "shader": 2,
    "model": 3,
    "skeletal_model": 4,
    "material": 5,
    "animation": 6,
    "sound": 7,
    "font": 8,
    "prefab": 9,
    "scene": 10,
}

TEXTURE_EXTS = [".png", ".jpg", ".jpeg", ".tga", ".bmp", ".hdr"]
MODEL_EXTS = [".obj", ".fbx"]

MAX_BONE_WEIGHTS = 4

def format_bytes(num_bytes):
    for unit in ["B", "KB", "MB", "GB", "TB", "PB", "EB", "ZB"]:
        if num_bytes < 1024.0:
            return f"{num_bytes:.1f} {unit}"
        num_bytes /= 1024.0
    return f"{num_bytes:.1f} YB"

def hash_file(filename):
    with open(filename, 'rb', buffering=0) as f:
        return hashlib.sha256(f.read()).hexdigest()