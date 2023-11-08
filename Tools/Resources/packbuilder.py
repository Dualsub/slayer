from concurrent.futures import ThreadPoolExecutor
from io import BufferedReader, BufferedWriter
import os
import struct
import argparse
import impasse as assimp
import numpy as np
import json
from termcolor import colored
from Resources.process_animation import process_animation
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


def serialize_shader(name, vs_source: str, fs_source: str, gs_source: str, meta: dict = {}):
    old_id = meta["old_data"]["id"] if "old_data" in meta and "id" in meta["old_data"] else None
    # Create header
    vs_data = vs_source.encode("utf-8")
    fs_data = fs_source.encode("utf-8")
    gs_data = gs_source.encode("utf-8")
    data = struct.pack("<I", len(vs_data)) + vs_data
    data += struct.pack("<I", len(fs_data)) + fs_data
    data += struct.pack("<I", len(gs_data)) + gs_data

    # Add header and data to the pack
    return serialize_asset(name, data, "shader", meta)


def serialize_compute_shader(name, source, meta: dict = {}):
    old_id = meta["old_data"]["id"] if "old_data" in meta and "id" in meta["old_data"] else None
    # Create header
    data = source.encode("utf-8")
    data = struct.pack("<I", len(data)) + data

    # Add header and data to the pack
    return serialize_asset(name, data, "compute_shader", meta)


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
        mesh_data += struct.pack("<I", len(vertices))
        mesh_data += np.array(vertices, dtype=np.float32).tobytes()
        mesh_data += struct.pack("<I", len(indices))
        mesh_data += np.array(indices, dtype=np.uint32).tobytes()

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
        bone_ids = mesh["bone_data"]
        inv_transform = mesh["inv_transform"]
        # Add mesh header and data to the mesh data

        vert_data = b""
        # Cast the third element of each vertex to an int32
        mesh_data += struct.pack("<I", len(vertices))
        # Redo with numpy
        vertices = np.array(vertices)

        dt = np.dtype([('vert', np.float32, 8), ('bone_ids',
                      np.int32, 4), ('weights', np.float32, 4)])

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

        mesh_data += struct.pack("<I", len(bone_ids))
        for bone_name, bone in bone_ids.items():
            # Bone name
            bone_name_data = bone_name.encode("utf-8")
            mesh_data += struct.pack("<I", len(bone_name_data))
            mesh_data += bone_name_data
            try:
                # Bone id
                mesh_data += struct.pack("<i", bone[0])
            except Exception as e:
                print(bone_name, bone, sep="\n")
                print(e)
                exit()
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
            socket_data += struct.pack("<16f", *
                                       np.array(socket["transform"]).flatten())
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


def serialize_animation_texture(name, duration, ticks_per_second, channels, bone_data: dict, meta: dict = {}):
    # Create header
    data = struct.pack("<f", duration)
    data += struct.pack("<f", ticks_per_second)
    data += struct.pack("<I", len(bone_data))

    assert all(len(channel["position_keys"]) == len(
        channels[0]["position_keys"]) for channel in channels)
    assert all(len(channel["rotation_keys"]) == len(
        channels[0]["rotation_keys"]) for channel in channels)
    assert all(len(channel["scale_keys"]) == len(
        channels[0]["scale_keys"]) for channel in channels)

    timestamps = np.array(
        channels[0]["position_keys"], dtype=np.float32).T[0] / ticks_per_second
    data += struct.pack("<I", timestamps.size)
    data += struct.pack("<" + "f" * timestamps.size, *timestamps.flatten())

    texture = np.zeros((len(timestamps), len(
        bone_data) * 3, 4), dtype=np.float32)
    for frame in range(len(timestamps)):
        for channel in channels:
            bone_name = channel["node_name"]
            if bone_name not in bone_data:
                print(colored("[WARNING]", "yellow"),
                      f"Bone {bone_name} not found for animation {name}.")
                continue

            bone_id = bone_data[bone_name]["id"]
            texture[frame, bone_id * 3 + 0,
                    :3] = np.array(channel["position_keys"])[frame][1:]
            texture[frame, bone_id * 3 + 1,
                    :4] = np.array(channel["rotation_keys"])[frame][1:]
            texture[frame, bone_id * 3 + 2,
                    :3] = np.array(channel["scale_keys"])[frame][1:]

        #     print(bone_name, bone_id, "\n",
        #           texture[frame, bone_id*3:bone_id*3+3, :4])
        # exit()

    # Transpose only in 2d, not the color vectors
    texture = np.transpose(texture, (1, 0, 2))
    # texture = np.flip(texture, axis=0)

    data += struct.pack("<I", texture.size)
    data += struct.pack("<" + "f" * texture.size, *texture.flatten())

    # Add header and data to the pack
    return serialize_asset(name, data, "animation", meta)


def serialize_material(name, textures: list, texture_ids: list, meta: dict = {}):
    old_id = meta["old_data"]["id"] if "old_data" in meta and "id" in meta["old_data"] else None
    # Create header
    header = struct.pack("<I", len(textures))

    texture_data = b""
    for texture in textures:
        if texture["name"] not in texture_ids:
            print(colored("[ERROR]", "red"),
                  f"Texture {texture['name']} not found.")
            raise Exception("Texture not found.")
        assetId = texture_ids[texture["name"]]
        # Textures consist of a uint8 for the type and a uint64 for assetId
        texture_data += struct.pack("<B", texture["type"])
        texture_data += struct.pack("<Q", assetId)

    # Add header and data to the pack
    return serialize_asset(name, header + texture_data, "material", meta)


def serialize_asset(name, asset_data, type_str: str, meta: dict = {}):
    old_id = meta["old_data"]["id"] if "old_data" in meta and "id" in meta["old_data"] else None
    if old_id is None and "asset_id" in meta:
        old_id = meta["asset_id"]
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

    meta["asset_id"] = int(asset_id)
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
            full_asset_data = f.read(
                ASSET_ID_SIZE + ASSET_TYPE_SIZE + UINT32_SIZE + name_size + UINT32_SIZE + asset_size)

            assetNameToMeta[asset_name] = {
                "id": asset_id,
                "type": asset_type,
                "data": full_asset_data
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


def pack_file(file_tuple: tuple, old_data: dict = {}, skeletons={}, texture_ids={}, force_rebuild=False) -> tuple:
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
        tries = 0
        while True:
            try:
                with open(meta_file, "r", encoding="utf8") as f:
                    meta_file_str = f.read()
                    if len(meta_file_str) > 0:
                        meta = {**meta, **json.loads(meta_file_str)}
                break
            except Exception as e:
                tries += 1
                if tries > 3:
                    print(colored("[ERROR]", "red"),
                          "Failed to read meta file.")
                    print(name)
                    print(e)
                    raise e

    if ("hash" in meta and hash_file(path) == meta["hash"] and "old_data" in meta and len(meta["old_data"]) > 0) and not force_rebuild:
        print(colored("[SKIPPED]", "yellow"),
              f"name: {name}, type: {ext[1:]}, id: {meta['old_data']['id']}")
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
            assert "skeleton" in meta, "No skeleton specified in meta file."
            bone_data = skeletons[meta["skeleton"]]["bone_data"]
            inv_transform = skeletons[meta["skeleton"]]["inv_transform"]
            return serialize_animation_texture(name, duration, ticks_per_second, channels, bone_data, meta)

        is_skeletal = all(len(mesh.bones) > 0 for mesh in scene.meshes)
        if "override_skeletal" in meta:
            is_skeletal = meta["override_skeletal"]
        if is_skeletal:
            meshes = load_skeletal_model(
                path, skeleton=(skeletons[meta["skeleton"]] if "skeleton" in meta else None))
            return serialize_skeletal_model(name, meshes, meta)
        else:
            meshes = load_model(scene)
            return serialize_model(name, meshes, meta)
    elif ext == ".shader":
        vs, fs, gs = load_shader(path)
        return serialize_shader(name, vs, fs, gs, meta)
    elif ext == ".material":
        textures = load_material(path)
        return serialize_material(name, textures, texture_ids, meta)
    elif ext == ".comp":
        source = load_compute_shader(path)
        return serialize_compute_shader(name, source, meta)
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

        skeleton_files = [file for file in files if os.path.splitext(
            os.path.basename(file[0]))[1].lower() in SKELETON_EXTS]

        other_files = list(set(files) - set(texture_files) -
                           set(model_files) - set(skeleton_files))

        skeletons = load_skeletons(model_files)

        pack_data = b""
        num_assets = 0

        with ThreadPoolExecutor() as executor:

            model_futures = [executor.submit(
                pack_file, file_tuple, old_data, skeletons) for file_tuple in model_files]

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
                pack_file, file_tuple, old_data, skeletons, texture_ids, True) for file_tuple in other_files]
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
