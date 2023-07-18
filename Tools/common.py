from scipy.spatial.transform import Rotation
import impasse as assimp
import hashlib
import numpy as np
import numpy.linalg as LA

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
SKELETON_EXTS = [".skeleton"]

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


def decompose_transform_matrix(transform_matrix):
    # Compute position vector
    pos = transform_matrix[:3, 3]

    # Compute rotation quaternion
    rot = Rotation.from_matrix(
        transform_matrix[:3, :3]).as_quat()

    # Compute scale vector
    scale = np.zeros(3)
    scale[0] = LA.norm(transform_matrix[:3, 0])
    scale[1] = LA.norm(transform_matrix[:3, 1])
    scale[2] = LA.norm(transform_matrix[:3, 2])

    assert len(pos) == 3
    assert len(rot) == 4
    assert len(scale) == 3

    return pos, rot, scale


def compose_transform_matrix(pos, rot, scale):
    assert len(pos) == 3
    assert len(rot) == 4
    assert len(scale) == 3
    assert all(np.isclose(s, 1.0) for s in scale)

    # Compute rotation matrix from quaternion
    rot_mat = np.identity(4)
    rot_mat[:3, :3] = Rotation.from_quat(rot).as_matrix()

    # Compute scale matrix
    scale_mat = np.identity(4)
    scale_mat[0, 0] = scale[0]
    scale_mat[1, 1] = scale[1]
    scale_mat[2, 2] = scale[2]

    # Compute translation matrix
    trans_mat = np.identity(4)
    trans_mat[0, 3] = pos[0]
    trans_mat[1, 3] = pos[1]
    trans_mat[2, 3] = pos[2]

    # Compute transform matrix from the other matrices
    T = scale_mat @ rot_mat @ trans_mat

    return T
