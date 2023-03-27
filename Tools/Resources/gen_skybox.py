import cv2 as cv
import argparse
import numpy as np

def main():
    parser = argparse.ArgumentParser(description='Generate skybox from equirectangular image')
    # parser.add_argument('-o', 'output', help='Path to the output image')
    args = parser.parse_args()

    # Create a 4096x2048 image with half being white, and the other half being a gradient from white to a darker blue
    img = np.zeros((2048, 4096, 3), np.float32)

    img[0:1024, 0:4096, 0] = 1.0
    img[1024:2048, 0:4096, 0] = np.linspace(1.0, 0.0, 1024).reshape(1024, 1)

    img[0:1024, 0:4096, 1] = 1.0
    img[1024:2048, 0:4096, 1] = np.linspace(1.0, 0.0, 1024).reshape(1024, 1)

    img[0:1024, 0:4096, 2] = 1.0
    img[1024:2048, 0:4096, 2] = np.linspace(1.0, 0.5, 1024).reshape(1024, 1)

    # # Convert the image to a cubemap
    # cubemap = cv.fisheye.equi2cube(img, (1024, 1024))

    cv.imshow("cubemap", cv.resize(img[:,:,::-1], (1024, 1024)))
    cv.waitKey(0)
    # Save the cubemap
    # cv.imwrite(args.output, cubemap)

if __name__ == "__main__":
    main()
