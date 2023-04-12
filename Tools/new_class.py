import os
import argparse

# Creates new c++ class with header and source files

SRC_CONTENT = """\
#include "%s.h"

namespace Slayer {

    %s::%s()
    {
    }

    %s::~%s()
    {
    }

}
"""

HEADER_CONTENT = """\
#pragma once

#include "Slayer.h"

namespace Slayer {

    class %s
    {
    public:
        %s();
        ~%s();
    };

}
"""

def create_new_class(base_dir, path, name):
    header_path = os.path.join(base_dir, "include", path, name + ".h")
    source_path = os.path.join(base_dir, "src", path, name + ".cpp")

    header = open(header_path, "w")
    header.write(HEADER_CONTENT % (name, name, name))
    header.close()

    source = open(source_path, "w")
    source.write(SRC_CONTENT % (name, name, name, name, name))
    source.close()

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--name", help="class name")
    parser.add_argument("--path", help="path to class")
    parser.add_argument("--dir", help="base directory", default=".")

    args = parser.parse_args()

    create_new_class(args.dir, args.path, args.name)

if __name__ == "__main__":
    main()

