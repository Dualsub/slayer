# Script that takes a regex for a texture file name, and inverets the colors of the texture.

import os
import re
import sys
import argparse

from PIL import Image
import numpy as np


def main():
    parser = argparse.ArgumentParser(description='Invert texture colors')
    parser.add_argument('--regex', type=str,
                        help='Regex for texture file name')
    parser.add_argument('--path', type=str,
                        help='Path to textures', default=".")
    args = parser.parse_args()

    search_strs = args.regex.split("|") if "|" in args.regex else [args.regex]
    texture_files = [file
                     for file in os.listdir(args.path) if all([search_str in file for search_str in search_strs])]

    print("Inverting colors for: ")
    for file in texture_files:
        print("  ", file)
    if input("Continue? [y/n]: ").lower() != "y":
        exit()

    for file in texture_files:
        image = Image.open(os.path.join(args.path, file))
        data = np.array(image)
        channels = data.shape[2] if len(data.shape) == 3 else 1
        # if there is an alpha channel, dont invert it
        if channels == 4:
            data[:, :, :3] = 255 - data[:, :, :3]
        else:
            data = 255 - data
        image = Image.fromarray(data)
        image.save(os.path.join(args.path, file))


if __name__ == "__main__":
    main()
