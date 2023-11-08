import os
import struct
import numpy as np
import cv2 as cv
import PIL.Image as Image
import json
from slayer_bindings.assimp import load_bone_data, load_bone_data_with_skeleton

from common import *
import impasse as assimp

MODEL_FLAGS = (assimp.constants.ProcessingStep.FlipUVs | assimp.constants.ProcessingStep.GenSmoothNormals
               | assimp.constants.ProcessingStep.Triangulate | assimp.constants.ProcessingStep.CalcTangentSpace)

SEKELTAL_MODEL_FLAGS = MODEL_FLAGS | assimp.constants.ProcessingStep.JoinIdenticalVertices | 16384


def load_texture(path: str) -> tuple:
    # Load image
    target = TEXTURE_2D_TARGET
    image = Image.open(path)

    # Get numpy array
    image = np.array(image)

    width, height = image.shape[1], image.shape[0]
    # Get image channels
    channels = image.shape[2] if len(image.shape) == 3 else 1
    bytes_per_channel = image.dtype.itemsize
    assert image.dtype.itemsize == 1, "Only 8-bit RGB textures are supported: " + \
        path + " " + str(image.dtype.itemsize)
    # Flip image vertically
    # image = np.flip(image, 0)
    # Get image data
    image = Image.fromarray(image)
    # Get the binary data from the image formatted as a PNG
    with open(path, "rb") as f:
        data = f.read()

    return width, height, channels, target, data


def load_hdr_texture(path: str) -> tuple:
    # Load image
    target = TEXTURE_CUBE_MAP_TARGET
    # Get image data
    image = cv.imread(path, cv.IMREAD_UNCHANGED).astype(np.float32)
    width, height = image.shape[1], image.shape[0]
    channels = image.shape[2]
    bytes_per_channel = image.dtype.itemsize
    assert image.dtype.itemsize == 4, "Only 32-bit RGB textures are supported"

    image = image[:, :, [2, 1, 0]]
    image = np.power(image, 1.0/2.2)
    image = np.flip(image, 0)

    # cv.imshow("image", image)
    # cv.waitKey(0)
    # exit()

    # Get image data
    data = struct.pack(
        "<" + "f" * (width * height * channels), *image.flatten())

    return width, height, channels, target, data


def load_model(scene) -> list:
    meshes = []
    for mesh in scene.meshes:
        # # Get vertices
        vertices = np.array(mesh.vertices)
        normals = np.array(mesh.normals)
        texture_coords = np.array(mesh.texture_coords)[0, :, :2]
        vertices = np.concatenate(
            (vertices, texture_coords, normals), axis=1).flatten().tolist()
        # Get indices
        indices = np.array(mesh.faces).flatten().tolist()

        # Create mesh
        mesh = {"vertices": vertices, "indices": indices}

        # Add mesh to the list
        meshes.append(mesh)
    return meshes


def load_shader(path: str) -> tuple:
    src = ""
    with open(path, mode="r", encoding="utf8") as f:
        src = f.read()
    data = json.loads(src)
    base_path = os.path.dirname(path)
    vs_src = open(os.path.join(
        base_path, data["vs"]), mode="r", encoding="utf8").read() + "\0"
    fs_src = open(os.path.join(
        base_path, data["fs"]), mode="r", encoding="utf8").read() + "\0"
    gs_src = open(os.path.join(
        base_path, data["gs"]), mode="r", encoding="utf8").read() + "\0" if "gs" in data else ""

    return vs_src, fs_src, gs_src


def load_compute_shader(path: str) -> tuple:
    src = ""
    with open(path, mode="r", encoding="utf8") as f:
        src = f.read()
    return src + "\0"


def load_material(path: str) -> list:
    textures = []
    # Load material
    src = ""
    with open(path, mode="r", encoding="utf8") as f:
        src = f.read()
    material = json.loads(src)

    # Add textures
    for texture in material["textures"]:
        textures.append(
            {"type": MATERIAL_TEXTURE_TYPES[texture["type"]], "name": texture["name"]})

    return textures


def load_skeletons(model_files: list[str]):
    skeletons = {}
    for file_tuple in model_files:
        name, file = file_tuple
        scene = assimp.load(file)
        if len(scene.animations) > 0 or any(len(mesh.bones) == 0 for mesh in scene.meshes):
            continue

        _, _, bones, inv_transform = load_bone_data(file)

        skeletons[name] = {"bone_data": {bone_name: {"id": bone_id, "parent_id": parent_id, "offset_matrix": offset_matrix}
                           for bone_name, (bone_id, parent_id, offset_matrix) in bones.items()}, "inv_transform": inv_transform, "raw_bone_data": bones}

    return skeletons


def load_skeletal_model(path, skeleton=None) -> tuple:
    # Load model
    try:
        vertices, indices, bone_data, inv_transform = load_bone_data_with_skeleton(
            path, skeleton["raw_bone_data"]) if skeleton is not None else load_bone_data(path)
    except Exception as e:
        print("Failed to load skeletal model: " + path,
              "Skeleton exists" if skeleton is not None else "Skeleton does not exist", sep="\n")
        raise e

    meshes = []
    meshes.append({
        "vertices": vertices,
        "indices": indices,
        "bone_data": bone_data if skeleton is None else skeleton["raw_bone_data"],
        "inv_transform": inv_transform if skeleton is None else skeleton["inv_transform"]
    })
    return meshes


def load_animation(scene) -> list:
    animation = scene.animations[0]
    duration = animation.duration
    ticks_per_second = animation.ticks_per_second
    channels = []
    for channel in animation.channels:
        # Get channel name
        node_name = channel.node_name
        # Get channel position keys
        position_keys = []
        for key in channel.position_keys:
            position_keys.append((key.time, *key.value))
        # Get channel rotation keys
        rotation_keys = []
        for key in channel.rotation_keys:
            rotation_keys.append((key.time, *key.value))
        # Get channel scaling keys
        scale_keys = []
        for key in channel.scaling_keys:
            scale_keys.append((key.time, *key.value))
        # Create channel
        channel = {"node_name": node_name, "position_keys": position_keys,
                   "rotation_keys": rotation_keys, "scale_keys": scale_keys}
        # Add channel to the list
        channels.append(channel)

    return duration, ticks_per_second, channels
