#include <iostream>
#include <string>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace py = pybind11;

void PrintHeirarchy(aiNode* node, unsigned int level = 0)
{
    for (unsigned int i = 0; i < level; i++)
        std::cout << " ";

    std::string name = node->mName.C_Str();
    std::cout << level << " " << name << std::endl;

    for (unsigned int i = 0; i < node->mNumChildren; i++)
        PrintHeirarchy(node->mChildren[i], level + 1);
}

void ReadParents(aiNode* node, py::dict& bones)
{
    std::string boneName = node->mName.C_Str();
    py::tuple bone_tuple = bones[py::str(boneName)];
    if (node->mParent)
    {
        std::string parentName = node->mParent->mName.C_Str();
        if (bones.contains(py::str(parentName)))
        {
            py::tuple parent_tuple = bones[py::str(parentName)];
            bones[py::str(boneName)] = py::make_tuple(bone_tuple[0], parent_tuple[0], bone_tuple[2]);
        }
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++)
        ReadParents(node->mChildren[i], bones);
}

py::tuple load_bone_data(const std::string& file_path) {
    // Create the importer and read the file
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(file_path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_PopulateArmatureData | aiProcess_LimitBoneWeights);

    if (!scene) {
        std::cout << "Failed to load file: " << file_path << " with error message: " << importer.GetErrorString() << std::endl;
        throw std::runtime_error("Failed to load file: " + file_path + " with error message: " + importer.GetErrorString());
    }

    std::vector<float> vertices;

    // Get the mesh
    const aiMesh* mesh = scene->mMeshes[0];

    const uint32_t vertex_size = 3 + 2 + 3 + 4 + 4;

    aiMatrix4x4 globalInv = scene->mRootNode->mTransformation;
    globalInv = globalInv.Inverse();
    py::array::ShapeContainer shape = { 4, 4 };
    py::array::StridesContainer strides = { 4 * sizeof(float), sizeof(float) };
    py::buffer_info buf_info = py::buffer_info(&globalInv.a1, sizeof(float), py::format_descriptor<float>::format(), 2, shape, strides);
    py::array_t<float> global_inverse_transform(buf_info);

    for (unsigned int v = 0; v < mesh->mNumVertices; v++)
    {
        vertices.push_back(mesh->mVertices[v].x);
        vertices.push_back(mesh->mVertices[v].y);
        vertices.push_back(mesh->mVertices[v].z);

        vertices.push_back(mesh->mTextureCoords[0][v].x);
        vertices.push_back(mesh->mTextureCoords[0][v].y);

        vertices.push_back(mesh->mNormals[v].x);
        vertices.push_back(mesh->mNormals[v].y);
        vertices.push_back(mesh->mNormals[v].z);

        for (unsigned int i = 0; i < 4; i++)
            vertices.push_back(-1.0f);

        for (unsigned int i = 0; i < 4; i++)
            vertices.push_back(0.0f);
    }

    py::list indices;

    for (unsigned int f = 0; f < mesh->mNumFaces; f++)
    {
        aiFace face = mesh->mFaces[f];
        for (unsigned int ind = 0; ind < face.mNumIndices; ind++)
        {
            indices.append(face.mIndices[ind]);
        }
    }

    // Initialize a dictionary to store the bone weights, names, and offset matrices
    py::dict bones;
    const uint32_t bone_id_offset = 8;
    const uint32_t bone_weight_offset = bone_id_offset + 4;

    int boneCount = 0;
    for (unsigned int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
    {
        auto* bone = mesh->mBones[boneIndex];
        int boneID = -1;
        std::string boneName = bone->mName.C_Str();
        if (!bones.contains(py::str(boneName)))
        {
            aiMatrix4x4 transform = bone->mOffsetMatrix;
            // Append the tuple of weight, name of the influencing bone, and offset matrix to the bone weights list
            py::array::ShapeContainer shape = { 4, 4 };
            py::array::StridesContainer strides = { 4 * sizeof(float), sizeof(float) };
            py::buffer_info buf_info = py::buffer_info(&transform.a1, sizeof(float), py::format_descriptor<float>::format(), 2, shape, strides);
            py::array_t<float> offset_matrix(buf_info);
            bones[py::str(boneName)] = py::make_tuple(boneCount, -1, offset_matrix);
            boneID = boneCount;
            boneCount++;
        }
        else
        {
            boneID = bones[py::str(boneName)][0].cast<int>();
        }

        assert(boneID != -1);

        auto weights = mesh->mBones[boneIndex]->mWeights;
        int numWeights = mesh->mBones[boneIndex]->mNumWeights;

        for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex)
        {
            int vertexId = weights[weightIndex].mVertexId;
            float weight = weights[weightIndex].mWeight;

            float* vertex = &vertices[vertexId * vertex_size];
            for (int i = 0; i < 4; ++i)
            {
                if (vertex[bone_id_offset + i] < 0)
                {
                    vertex[bone_id_offset + i] = (float)boneID;
                    vertex[bone_weight_offset + i] = weight;
                    break;
                }
            }
        }
    }

    // PrintHeirarchy(mesh->mBones[0]->mNode);
    ReadParents(mesh->mBones[0]->mNode, bones);

    importer.FreeScene();
    buf_info = py::buffer_info(vertices.data(), sizeof(float), py::format_descriptor<float>::format(), 2, { (int)vertices.size() / vertex_size, vertex_size }, { vertex_size * sizeof(float), sizeof(float) });
    py::array_t<float> vertices_array(buf_info);
    py::tuple bone_data = py::make_tuple(vertices_array, indices, bones, global_inverse_transform);

    return bone_data;
}

// Define the Python module
PYBIND11_MODULE(slayer_bindings, m) {
    m.doc() = "Slayer bindings tools";
    m.def_submodule("assimp")
        .def("load_bone_data", &load_bone_data, "Compute bone weights, influencing bones, and offset matrices for a given mesh file path");
}