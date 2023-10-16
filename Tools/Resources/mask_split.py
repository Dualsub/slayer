'''
Script for a mask texture with reoughness, metalness and ambient occlusion channels
'''

import os
import PIL.Image as Image
import argparse


def split_mask(mask_path, roughness_index, metalness_index, ao_index, roughness_inverse, metalness_inverse, ao_inverse, output_format="png"):
    '''
    Split a mask texture into its channels
    '''

    mask_folder = os.path.dirname(mask_path)
    mask_filename = os.path.basename(mask_path).split(".")[0]
    # Get format
    mask_format = mask_path.split(".")[-1].lower()
    # Load the mask
    mask = Image.open(mask_path, formats=[mask_format])
    # Split the mask
    # Make masks rgb with the same values in each channel
    # Get Alpha channel
    parts = mask.split()
    print("Mask split into", len(parts), "channels")

    mask_roughness = Image.merge(
        "RGB", (parts[roughness_index], parts[roughness_index], parts[roughness_index]))
    if roughness_inverse:
        mask_roughness = Image.eval(mask_roughness, lambda x: 255-x)
        print("Inversing roughness")

    mask_metalness = Image.merge(
        "RGB", (parts[metalness_index], parts[metalness_index], parts[metalness_index]))
    if metalness_inverse:
        mask_metalness = Image.eval(mask_metalness, lambda x: 255-x)
        print("Inversing metalness")

    mask_ao = Image.merge(
        "RGB", (parts[ao_index], parts[ao_index], parts[ao_index]))
    if ao_inverse:
        mask_ao = Image.eval(mask_ao, lambda x: 255-x)
        print("Inversing ao")

    roughness_path = os.path.join(
        mask_folder, mask_filename + "_roughness." + output_format)
    metalness_path = os.path.join(
        mask_folder, mask_filename + "_metalness." + output_format)
    ao_path = os.path.join(mask_folder, mask_filename + "_ao." + output_format)
    # Save the channels
    mask_roughness.save(roughness_path)
    mask_metalness.save(metalness_path)
    mask_ao.save(ao_path)

    print("Mask split into", roughness_path, metalness_path, "and", ao_path)


def main():
    # Parse arguments
    parser = argparse.ArgumentParser(description="Mask Split")
    parser.add_argument("-m", "--mask", help="Mask texture to split")
    parser.add_argument("-r", "--roughness",
                        help="Roughness texture channel index", default=0, type=int)
    parser.add_argument("-n", "--metalness",
                        help="Metalness texture channel index", default=1, type=int)
    parser.add_argument(
        "-a", "--ao", help="Ambient occlusion texture channel index", default=2, type=int)

    # Arguments for inversing the channels individually, ri, ni, ai
    parser.add_argument("-ri", "--roughness_inverse",
                        help="Inverse the roughness channel", action="store_true")
    parser.add_argument("-ni", "--metalness_inverse",
                        help="Inverse the metalness channel", action="store_true")
    parser.add_argument("-ai", "--ao_inverse",
                        help="Inverse the ambient occlusion channel", action="store_true")

    args = parser.parse_args()

    # Split the mask
    split_mask(args.mask, args.roughness, args.metalness, args.ao, args.roughness_inverse,
               args.metalness_inverse, args.ao_inverse)


if __name__ == "__main__":
    main()
