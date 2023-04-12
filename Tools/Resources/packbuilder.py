from concurrent.futures import ThreadPoolExecutor, as_completed
from io import BufferedReader, BufferedWriter, BytesIO
import os
import struct
import argparse
import impasse as assimp
import numpy as np
import cv2 as cv
import PIL.Image as Image
import json
from termcolor import colored
from common import *
from Resources.load import *
import time

def save_meta(meta: dict):
    # Remove old_data element from meta
    if "old_data" in meta:
        del meta["old_data"]
    
    meta_file = meta["meta_file"]
    if "meta_file" in meta:
        del meta["meta_file"]

    with open(meta_file, "w") as f:
        json.dump(meta, f, indent=4)

def serialize_texture(name, width, height, channels, target, data, meta: dict = {}):
    old_id = meta["old_data"]["id"] if "old_data" in meta and "id" in meta["old_data"] else None
    # Create header
    header = struct.pack("<I", width)
    header += struct.pack("<I", height)
    header += struct.pack("<I", channels)
    header += struct.pack("<I", target)
    header += struct.pack("<I", len(data))

    # Add header and data to the pack
    return serialize_asset(name, header + data, "texture", meta)


def serialize_shader(name, vsSource: str, fsSource: str, meta: dict = {}):
    old_id = meta["old_data"]["id"] if "old_data" in meta and "id" in meta["old_data"] else None
    # Create header
    vsData = vsSource.encode("utf-8")
    fsData = fsSource.encode("utf-8")
    data = struct.pack("<I", len(vsData)) + vsData
    data += struct.pack("<I", len(fsData)) + fsData

    # Add header and data to the pack
    return serialize_asset(name, data, "shader", meta)


def serialize_model(name, meshes: list, meta: dict = {}):
    old_id = meta["old_data"]["id"] if "old_data" in meta and "id" in meta["old_data"] else None
    # Create header
    header = struct.pack("<I", len(meshes))

    mesh_data = b""
    for mesh in meshes:
        # Create mesh header, vertices are list of floats and indices are list of uint32
        vertices = mesh["vertices"]
        indices = mesh["indices"]
        # Add mesh header and data to the mesh data
        mesh_data += mesh_header
        mesh_header = struct.pack("<I", len(vertices))
        mesh_data += struct.pack("<" + "f" * len(vertices), *vertices)
        mesh_header += struct.pack("<I", len(indices))
        mesh_data += struct.pack("<" + "I" * len(indices), *indices)

    # Add header and data to the pack
    return serialize_asset(name, header + mesh_data, "model", meta)


def serialize_skeletal_model(name, meshes: list, meta: dict = {}):
    old_id = meta["old_data"]["id"] if "old_data" in meta and "id" in meta["old_data"] else None
    # Create header
    header = struct.pack("<I", len(meshes))

    mesh_data = b""
    for mesh in meshes:
        # Create mesh header, vertices are list of floats and indices are list of uint32
        vertices = mesh["vertices"]
        indices = mesh["indices"]
        bone_data = mesh["bone_data"]
        inv_transform = mesh["inv_transform"]
        # Add mesh header and data to the mesh data

        vert_data = b""
        # Cast the third element of each vertex to an int32
        mesh_data += struct.pack("<I", len(vertices))
        # Redo with numpy
        vertices = np.array(vertices)

        dt = np.dtype([('vert', np.float32, 8), ('bone_ids', np.int32, 4), ('weights', np.float32, 4)])

        # Convert vertices array to structured array
        vertices_struct = np.empty(vertices.shape[0], dtype=dt)
        vertices_struct['vert'] = vertices[:, :8].astype(np.float32)
        vertices_struct['bone_ids'] = vertices[:, 8:12].astype(np.int32)
        vertices_struct['weights'] = vertices[:, 12:16].astype(np.float32)

        # Convert structured array to bytes
        vert_data = vertices_struct.tobytes()

        # 16 floats(and one int32) per vertex, 4 bytes per float/int32
        assert len(vert_data) == vertices.shape[0] * 16 * 4
        mesh_data += vert_data

        mesh_data += struct.pack("<I", len(indices))
        mesh_data += struct.pack("<" + "I" * len(indices), *indices)

        mesh_data += struct.pack("<I", len(bone_data))
        for bone_name, bone in bone_data.items():
            # Bone name
            bone_name_data = bone_name.encode("utf-8")
            mesh_data += struct.pack("<I", len(bone_name_data))
            mesh_data += bone_name_data
            # Bone id
            mesh_data += struct.pack("<i", bone[0])
            # Parent id
            mesh_data += struct.pack("<i", bone[1])
            # Bone transform
            mesh_data += struct.pack("<16f", *bone[2].T.flatten())

        # Global inverse transform
        mesh_data += struct.pack("<16f", *inv_transform.T.flatten())

    socket_data = b""

    if "sockets" in meta:
        sockets = meta["sockets"]
        socket_data += struct.pack("<I", len(sockets))
        for socket in sockets:
            socket_name = socket["name"]
            socket_name_data = socket_name.encode("utf-8")
            socket_data += struct.pack("<I", len(socket_name_data))
            socket_data += socket_name_data
            bone_name = socket["bone"]
            bone_name_data = bone_name.encode("utf-8")
            socket_data += struct.pack("<I", len(bone_name_data))
            socket_data += bone_name_data
            socket_data += struct.pack("<16f", *np.array(socket["transform"]).flatten())
    else:
        socket_data += struct.pack("<I", 0)

    # Add header and data to the pack
    return serialize_asset(name, header + mesh_data + socket_data, "skeletal_model", meta)


def serialize_animation(name, duration, ticks_per_second, channels, meta: dict = {}):
    old_id = meta["old_data"]["id"] if "old_data" in meta and "id" in meta["old_data"] else None
    # Create header
    header = struct.pack("<f", duration)
    header += struct.pack("<f", ticks_per_second)
    header += struct.pack("<I", len(channels))

    channel_data = b""
    for channel in channels:
        # Create channel header, keyframes are list of floats
        bone_name = channel["node_name"]
        rotation_keys = channel["rotation_keys"]
        position_keys = channel["position_keys"]
        scale_keys = channel["scale_keys"]

        channel_data += struct.pack("<I", len(bone_name))
        channel_data += bone_name.encode("utf-8")

        # Add keyframes to the channel data
        channel_data += struct.pack("<I", len(position_keys))
        position_keys = np.array(position_keys).astype(np.float32).flatten()
        channel_data += struct.pack("<" + "f" *
                                    len(position_keys), *position_keys)

        channel_data += struct.pack("<I", len(rotation_keys))
        rotation_keys = np.array(rotation_keys).astype(np.float32).flatten()
        channel_data += struct.pack("<" + "f" *
                                    len(rotation_keys), *rotation_keys)

        channel_data += struct.pack("<I", len(scale_keys))
        scale_keys = np.array(scale_keys).astype(np.float32).flatten()
        channel_data += struct.pack("<" + "f" * len(scale_keys), *scale_keys)

    events_data = b""
    # if "events" in meta:
    #     events = meta["events"]
    #     events_data += struct.pack("<I", len(events))
    #     for event_name, event in events:
    #         event_name_data = event_name.encode("utf-8")
    #         events_data += struct.pack("<I", len(event_name_data))
    #         events_data += event_name_data
    #         events_data += struct.pack("<f", float(event))
    # else:
    #     events_data += struct.pack("<I", 0)

    # Add header and data to the pack
    return serialize_asset(name, header + channel_data + events_data, "animation", meta)


def serialize_material(name, textures: list, texture_ids: list, meta: dict = {}):
    old_id = meta["old_data"]["id"] if "old_data" in meta and "id" in meta["old_data"] else None
    # Create header
    header = struct.pack("<I", len(textures))

    texture_data = b""
    for texture in textures:
        assetId = texture_ids[texture["name"]]
        # Textures consist of a uint8 for the type and a uint64 for assetId
        texture_data += struct.pack("<B", texture["type"])
        texture_data += struct.pack("<Q", assetId)

    # Add header and data to the pack
    return serialize_asset(name, header + texture_data, "material", meta)


def serialize_asset(name, asset_data, type_str: str, meta: dict = {}):
    old_id = meta["old_data"]["id"] if "old_data" in meta and "id" in meta["old_data"] else None
    data = b""
    type = ASSET_TYPES[type_str]
    asset_id = (np.random.randint(0, 2**64, dtype=np.uint64)
                if old_id is None else old_id)
    try:
        data += struct.pack("<Q", asset_id)
    except Exception as e:
        print("Error:", asset_id)
        exit()
    data += struct.pack("<H", type)
    data += struct.pack("<I", len(name))
    data += name.encode("utf-8")
    data += struct.pack("<I", len(asset_data))
    data += asset_data
    has_new_id = old_id != asset_id
    print(colored(f"[{'ADDED' if has_new_id else 'UPDATED'}]", "green" if has_new_id else "blue"),
          f"name: {name}, type: {type_str}, id: {asset_id}")

    save_meta(meta)

    return asset_id, name, data


def load_pack_meta(f: BufferedReader):
    assetNameToMeta = {}
    try:
        # Read header
        magic = f.read(len(MAGIC)).decode("utf-8")
        if magic != MAGIC:
            raise Exception("Invalid pack file magic.")

        version = struct.unpack("<I", f.read(UINT32_SIZE))[0]

        num_assets = struct.unpack("<I", f.read(UINT32_SIZE))[0]
        print("Reading pack file with", num_assets, "assets.")

        # Read assets
        for _ in range(num_assets):
            # Read asset id
            start_pos = f.tell()
            asset_id = struct.unpack("<Q", f.read(ASSET_ID_SIZE))[0]
            asset_type = struct.unpack("<H", f.read(ASSET_TYPE_SIZE))[0]
            name_size = struct.unpack("<I", f.read(UINT32_SIZE))[0]
            asset_name = f.read(name_size).decode("utf-8")
            asset_size = struct.unpack("<I", f.read(UINT32_SIZE))[0]
            asset_data = f.read(asset_size)
            
            f.seek(start_pos)
            full_asset_data = f.read(ASSET_ID_SIZE + ASSET_TYPE_SIZE + UINT32_SIZE + name_size + UINT32_SIZE + asset_size) 

            assetNameToMeta[asset_name] = {
                "id": asset_id,
                "type": asset_type,
                "data" : full_asset_data
            }

        print("Loaded", len(assetNameToMeta), "asset IDs from pack file.")
    except Exception as e:
        print(colored("[WARNING]", "yellow"), "Invalid pack file.")
        ans = ""
        while ans != "y" and ans != "n":
            ans = input("Do you want to continue? (y/n) ")
            ans = ans.lower()
            if ans != "y":
                raise e
        return {}

    return assetNameToMeta


def serialize_pack(data, num_assets, f: BufferedWriter):
    f.write(MAGIC.encode())
    # Lengths as unsigned 32 bit integers, in reverse byte order
    f.write(struct.pack("I", 1))
    f.write(struct.pack("I", num_assets))
    f.write(data)

    return len(MAGIC) + UINT32_SIZE * 2 + len(data)


def pack_file(file_tuple: tuple, old_data: dict = {}, texture_ids={}, force_rebuild = False) -> tuple:
    file, path = file_tuple
    # Get file name
    name = os.path.splitext(os.path.basename(file))[0]
    # Get file extension
    ext = os.path.splitext(os.path.basename(file))[1].lower()

    # Check if there is a file wit the same name but with .meta extension

    meta_file = os.path.splitext(path)[0] + ".meta"
    meta = {
        "old_data": old_data.get(name, {}),
        "meta_file": meta_file,
    }
    if os.path.isfile(meta_file):
        # Read meta file
        with open(meta_file, "r") as f:
            meta = {**meta, **json.load(f)}

    if ("hash" in meta and hash_file(path) == meta["hash"] and "old_data" in meta and len(meta["old_data"]) > 0 and not force_rebuild):
        print(colored("[SKIPPED]", "yellow"), f"name: {name}, type: {ext[1:]}, id: {meta['old_data']['id']}")
        return meta["old_data"]["id"], name, meta["old_data"]["data"]
    else:
        meta["hash"] = hash_file(path)

    # Add asset to the pack
    if ext == ".png" or ext == ".jpg" or ext == ".jpeg" or ext == ".tga" or ext == ".bmp":
        # Load texture
        width, height, channels, target, data = load_texture(path)
        return serialize_texture(
            name, width, height, channels, target, data, meta)
    elif ext == ".hdr":
        # Load texture
        width, height, channels, target, data = load_hdr_texture(path)
        return serialize_texture(
            name, width, height, channels, target, data, meta)
    elif ext == ".obj" or ext == ".fbx":
        scene = assimp.load(path, processing=MODEL_FLAGS)

        if len(scene.animations) > 0:
            duration, ticks_per_second, channels = load_animation(scene)
            return serialize_animation(name, duration, ticks_per_second, channels, meta)

        is_skeletal = all(len(mesh.bones) > 0 for mesh in scene.meshes)
        if is_skeletal:
            meshes = load_skeletal_model(path)
            return serialize_skeletal_model(name, meshes, meta)
        else:
            meshes = load_model(scene)
            return serialize_model(name, meshes, meta)
    elif ext == ".shader":
        vs, fs = load_shader(path)
        return serialize_shader(name, vs, fs, meta)
    elif ext == ".material":
        textures = load_material(path)
        return serialize_material(name, textures, texture_ids, meta)
    else:
        return None, None, None


def main():
    # Parse arguments
    parser = argparse.ArgumentParser(description="PackBuilder")
    parser.add_argument("-p", "--pack", help="Pack file to open")
    parser.add_argument("-o", "--output", help="Output file")
    parser.add_argument("add", nargs="*",
                        help="Files to add to the pack file")
    parser.add_argument(
        "build", help="Files to build into the pack file", action="store_true")
    parser.add_argument("-d", "--directory", help="Directory to build from")

    parser.add_argument(
        "peek", help="Peek at the contents of a pack file", action="store_true")

    args = parser.parse_args()

    if args.build and args.directory:
        t0 = time.time()

        old_data = {}
        if os.path.exists(args.output):
            with open(args.output, "rb") as f:
                old_data = load_pack_meta(f)

        # Build pack file
        files = []
        for dirpath, dirnames, filenames in os.walk(args.directory):
            files.extend([(file, os.path.join(dirpath, file))
                         for file in filenames])

        texture_files = [file for file in files if os.path.splitext(
            os.path.basename(file[0]))[1].lower() in TEXTURE_EXTS]
        texture_ids = {}

        model_files = [file for file in files if os.path.splitext(
            os.path.basename(file[0]))[1].lower() in MODEL_EXTS]

        other_files = list(set(files) - set(texture_files) - set(model_files))

        pack_data = b""
        num_assets = 0

        with ThreadPoolExecutor() as executor:

            model_futures = [executor.submit(
                pack_file, file_tuple, old_data) for file_tuple in model_files]

            print("Packing textures...")

            futures = [executor.submit(pack_file, file_tuple, old_data)
                       for file_tuple in texture_files]
            results = [future.result() for future in futures]
            for assetId, name, data in results:
                if data is None:
                    continue
                if assetId:
                    texture_ids[name] = assetId
                pack_data += data
                num_assets += 1

            print("Textures done. Packing other files...")

            futures = [executor.submit(
                pack_file, file_tuple, old_data, texture_ids) for file_tuple in other_files]
            results = [future.result() for future in futures]
            for assetId, name, data in results:
                if data is None:
                    continue
                pack_data += data
                num_assets += 1

            print("Other files done. Packing models...")
            results = [future.result() for future in model_futures]
            for assetId, name, data in results:
                if data is None:
                    continue
                pack_data += data
                num_assets += 1

        # Write pack file
        if args.output:
            pack_size = 0
            with open(args.output, "wb") as f:
                pack_size = serialize_pack(pack_data, num_assets, f)

            t1 = time.time()

            print(colored("[SAVED]", "green"),
                  "Saved to pack file to:", args.output, f"({format_bytes(pack_size)}), took {t1 - t0:.2f}s")

    # Open output file


if __name__ == "__main__":
    main()
