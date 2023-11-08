import os

# for each .fbx file in the directory, createb a .meta file with a skeleton key set to what is inputed as an argument to the script
# example: python gen_meta.py "TPose"

# get the skeleton key from the command line
import sys
skeleton_key = sys.argv[1]

# get the current directory
cwd = os.getcwd()

# get all the files in the current directory
files = os.listdir(cwd)

# for each file in the current directory
for file in files:
    # if the file is a .fbx file
    if file.lower().endswith(".fbx"):
        # create a .meta file with the same name as the .fbx file
        meta_file = open(file[:-4] + ".meta", "w+")
        # write the skeleton key to the .meta file
        meta_file.write("{\n    \"skeleton\": \"" + skeleton_key + "\"\n}")
        # close the .meta file
        meta_file.close()
