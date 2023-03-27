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
    data = struct.pack("<" + "f" * (width * height * channels), *image.flatten())

    return width, height, channels, target, data

def load_model(path: str) -> list:
    meshes = []
    flags = (assimp.constants.ProcessingStep.FlipUVs | assimp.constants.ProcessingStep.GenSmoothNormals | assimp.constants.ProcessingStep.Triangulate)
    scene = assimp.load(path, processing=flags)
    for mesh in scene.meshes:
        # # Get vertices
        vertices = np.array(mesh.vertices)
        normals = np.array(mesh.normals)
        texture_coords = np.array(mesh.texture_coords)[0,:,:2]
        vertices = np.concatenate((vertices, texture_coords, normals), axis=1).flatten().tolist()
        # Get indices
        indices = np.array(mesh.faces).flatten().tolist()

        # Create mesh
        mesh = { "vertices": vertices, "indices": indices }

        # Add mesh to the list
        meshes.append(mesh)
    return meshes

def load_shader(path: str) -> tuple:
    src = ""
    with open(path, mode="r", encoding="utf8") as f:
        src = f.read() 
    data = json.loads(src)
    base_path = os.path.dirname(path)
    vs_src = open(os.path.join(base_path, data["vs"]), mode="r", encoding="utf8").read() + "\0"
    fs_src = open(os.path.join(base_path, data["fs"]), mode="r", encoding="utf8").read() + "\0"
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
        textures.append({ "type": MATERIAL_TEXTURE_TYPES[texture["type"]], "name": texture["name"] })

    return textures

class AssetPack():
    def __init__(self, verbose=False):
        self.data = b""
        self.assets = {}
        self.verbose = verbose
        self.assetNameToId = {}

    def add_texture(self, name, width, height, channels, target, data):
        # Create header
        header = struct.pack("<I", width)
        header += struct.pack("<I", height)
        header += struct.pack("<I", channels)
        header += struct.pack("<I", target)

        # Add header and data to the pack
        return self.add_asset(name, header + data, "texture")


    def add_shader(self, name, vsSource: str, fsSource: str):
        # Create header
        header = struct.pack("<I", len(vsSource))
        header += struct.pack("<I", len(fsSource))

        # Add header and data to the pack
        self.add_asset(name, header + vsSource.encode("utf-8") + fsSource.encode("utf-8"), "shader")

    def add_model(self, name, meshes: list):
        # Create header
        header = struct.pack("<I", len(meshes))

        mesh_data = b""
        for mesh in meshes:
            # Create mesh header, vertices are list of floats and indices are list of uint32
            vertices = mesh["vertices"]
            indices = mesh["indices"]
            mesh_header = struct.pack("<I", len(vertices))
            mesh_header += struct.pack("<I", len(indices))
            # Add mesh header and data to the mesh data
            mesh_data += mesh_header
            mesh_data += struct.pack("<" + "f" * len(vertices), *vertices)
            mesh_data += struct.pack("<" + "I" * len(indices), *indices)

        # Add header and data to the pack
        self.add_asset(name, header + mesh_data, "model")

    def add_material(self, name, textures: list, textureIds: list):
        # Create header
        header = struct.pack("<I", len(textures))

        texture_data = b""
        for texture in textures:
            assetId = textureIds[texture["name"]]
            # Textures consist of a uint8 for the type and a uint64 for assetId
            texture_data += struct.pack("<B", texture["type"])
            texture_data += struct.pack("<Q", assetId)

        # Add header and data to the pack
        self.add_asset(name, header + texture_data, "material")

    def add_asset(self, name, data, type_str: str):
        is_new = name not in self.assetNameToId
        # Add asset to the pack
        type = ASSET_TYPES[type_str]
        self.assets[name] = len(self.data)
        asset_id = (np.random.randint(0, 2**64, dtype=np.uint64) if is_new else self.assetNameToId[name])
        self.data += struct.pack("<Q", asset_id)
        self.data += struct.pack("<H", type)
        self.data += struct.pack("<I", len(name))
        self.data += struct.pack("<I", len(data))
        self.data += name.encode("utf-8")
        self.data += data

        print(colored(f"[{'ADDED' if is_new else 'UPDATED'}]", "green" if is_new else "blue"), f"name: {name}, type: {type_str}, id: {asset_id}")

        return asset_id

    def load_ids(self, f: BufferedReader):
        # Read header
        magic = f.read(len(MAGIC)).decode("utf-8")
        if magic != MAGIC:
            raise Exception("Invalid pack file")

        version = struct.unpack("<I", f.read(UINT32_SIZE))[0]

        num_assets = struct.unpack("<I", f.read(UINT32_SIZE))[0]
        print("Reading pack file with", num_assets, "assets.")

        # Read assets
        for _ in range(num_assets):
            # Read asset id
            asset_id = struct.unpack("<Q", f.read(ASSET_ID_SIZE))[0]
            asset_type = struct.unpack("<H", f.read(ASSET_TYPE_SIZE))[0]
            name_size = struct.unpack("<I", f.read(UINT32_SIZE))[0]
            asset_size = struct.unpack("<I", f.read(UINT32_SIZE))[0]
            asset_name = f.read(name_size).decode("utf-8")
            if self.verbose:
                print(f"Found {asset_name} ({asset_id})")
            f.read(asset_size)

            self.assetNameToId[asset_name] = asset_id

        print("Loaded", len(self.assetNameToId), "asset IDs from pack file.")


    def serialize(self, f: BufferedWriter):
        f.write(MAGIC.encode())
        # Lengths as unsigned 32 bit integers, in reverse byte order
        f.write(struct.pack("I", 1))
        f.write(struct.pack("I", len(self.assets)))
        f.write(self.data)


def main():
        # Parse arguments
    parser = argparse.ArgumentParser(description="PackBuilder")
    parser.add_argument("-p", "--pack", help="Pack file to open")
    parser.add_argument("-o", "--output", help="Output file")
    parser.add_argument("add", nargs="*", help="Files to add to the pack file")
    parser.add_argument("build", help="Files to build into the pack file")
    parser.add_argument("-d", "--directory", help="Directory to build from")
    
    args = parser.parse_args()

    pack = AssetPack()
    
    if args.build and args.directory:
        if os.path.exists(args.output):
            with open(args.output, "rb") as f:
                pack.load_ids(f)
    
        # Build pack file
        files = []
        for dirpath, dirnames, filenames in os.walk(args.directory):
            files.extend([(file, os.path.join(dirpath, file)) for file in filenames])
            
        files = sorted(files, key=lambda file: 0 if os.path.splitext(os.path.basename(file[0]))[1].lower() in [".png", ".jpg", ".jpeg", ".tga", ".bmp"] else 1)
        textureIds = {}
        for file, path in files:
            # Get file name
            name = os.path.splitext(os.path.basename(file))[0]
            # Get file extension
            ext = os.path.splitext(os.path.basename(file))[1].lower()

            # Add asset to the pack
            if ext == ".png" or ext == ".jpg" or ext == ".jpeg" or ext == ".tga" or ext == ".bmp":
                # Load texture
                width, height, channels, target, data = load_texture(path)
                assetId = pack.add_texture(name, width, height, channels, target, data)
                textureIds[name] = assetId
            elif ext == ".hdr":
                # Load texture
                width, height, channels, target, data = load_hdr_texture(path)
                assetId = pack.add_texture(name, width, height, channels, target, data)
                textureIds[name] = assetId
            elif ext == ".obj" or ext == ".fbx":
                meshes = load_model(path)
                pack.add_model(name, meshes)
            elif ext == ".shader":
                vs, fs = load_shader(path)
                pack.add_shader(name, vs, fs)
            elif ext == ".material":
                textures = load_material(path)
                pack.add_material(name, textures, textureIds)

    # Write pack file
    if args.output:
        with open(args.output, "wb") as f:
            pack.serialize(f)

        print(colored("[SAVED]", "green"), "Saved to pack file to:", args.output)

    # Open output file

if __name__ == "__main__":
    main()