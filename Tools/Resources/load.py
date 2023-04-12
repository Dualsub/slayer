import os
import struct
import impasse as assimp
import numpy as np
import cv2 as cv
import PIL.Image as Image
import json
from slayer_bindings.assimp import load_bone_data

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
    channels = image.shape[2]
    bytes_per_channel = image.dtype.itemsize
    assert image.dtype.itemsize == 1, "Only 8-bit RGB textures are supported"
    # Flip image vertically
    # image = np.flip(image, 0)
    # Get image data
    data = image.tobytes()

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
    # image = np.power(image, 1.0/2.2)
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
    return vs_src, fs_src


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


def load_skeletal_model(path) -> tuple:
    # Load model
    vertices, indices, bone_data, inv_transform = load_bone_data(path)
    meshes = []
    meshes.append({
        "vertices": vertices,
        "indices": indices,
        "bone_data": bone_data,
        "inv_transform": inv_transform
    })
    return meshes


def load_animation(scene) -> list:

    animation = scene.animations[0]
    name = animation.name
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
