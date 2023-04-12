'''
Script for a mask texture with reoughness, metalness and ambient occlusion channels
'''

import os
import PIL.Image as Image
import argparse

def split_mask(mask_path, roughness_path, metalness_path, ao_path):
    '''
    Split a mask texture into its channels
    '''
    # Load the mask
    mask = Image.open(mask_path, formats="TGA")
    # Split the mask
    # Make masks rgb with the same values in each channel
    # Get Alpha channel
    parts = mask.split()
    mask_roughness = Image.merge("RGB", (parts[0], parts[0], parts[0]))
    mask_metalness = Image.merge("RGB", (parts[1], parts[1], parts[1]))
    mask_ao = Image.merge("RGB", (parts[2], parts[2], parts[2]))
    # Save the channels
    mask_roughness.save(roughness_path)
    mask_metalness.save(metalness_path)
    mask_ao.save(ao_path)

    print("Mask split into", roughness_path, metalness_path, "and", ao_path)

def main():
    # Parse arguments
    parser = argparse.ArgumentParser(description="Mask Split")
    parser.add_argument("-m", "--mask", help="Mask texture to split")
    parser.add_argument("-r", "--roughness", help="Roughness texture")
    parser.add_argument("-n", "--metalness", help="Metalness texture")
    parser.add_argument("-a", "--ao", help="Ambient occlusion texture")

    args = parser.parse_args()

    # Split the mask
    split_mask(args.mask, args.roughness, args.metalness, args.ao)

if __name__ == "__main__":
    main()

