#include <iostream>
#include <string>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include "glm/gtx/matrix_decompose.hpp"

namespace py = pybind11;

void PrintHeirarchy(aiNode* node, py::dict& bones, unsigned int level = 0)
{
    for (unsigned int i = 0; i < level; i++)
        std::cout << " ";

    std::string name(node->mName.C_Str());
    if (bones.contains(py::str(name)))
    {
        std::cout << name << ": ";
        std::cout << bones[py::str(name)].cast<py::tuple>()[0].cast<int>();
        std::cout << ", parent: " << bones[py::str(name)].cast<py::tuple>()[1].cast<int>();
        std::cout << std::endl;
    }


    for (unsigned int i = 0; i < node->mNumChildren; i++)
        PrintHeirarchy(node->mChildren[i], bones, level + 1);
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

    ReadParents(mesh->mBones[0]->mNode, bones);

    importer.FreeScene();
    buf_info = py::buffer_info(vertices.data(), sizeof(float), py::format_descriptor<float>::format(), 2, { (int)vertices.size() / vertex_size, vertex_size }, { vertex_size * sizeof(float), sizeof(float) });
    py::array_t<float> vertices_array(buf_info);
    py::tuple bone_data = py::make_tuple(vertices_array, indices, bones, global_inverse_transform);

    return bone_data;
}

py::tuple transform_decompose(const py::array_t<float>& transform)
{
    py::buffer_info buf_info = transform.request();
    if (buf_info.ndim != 2)
        throw std::runtime_error("Number of dimensions must be two");
    if (buf_info.shape[0] != 4 || buf_info.shape[1] != 4)
        throw std::runtime_error("Shape must be 4x4");

    glm::mat4 transform_mat = *(glm::mat4*)buf_info.ptr;
    glm::vec3 translation;
    glm::quat rotation;
    glm::vec3 scale;
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(transform_mat, scale, rotation, translation, skew, perspective);

    py::array::ShapeContainer shape = { 3 };
    py::array::StridesContainer strides = { sizeof(float) };
    py::buffer_info translation_buf_info = py::buffer_info(&translation.x, sizeof(float), py::format_descriptor<float>::format(), 1, shape, strides);
    py::array_t<float> translation_array(translation_buf_info);

    py::array::ShapeContainer quat_shape = { 4 };
    py::buffer_info rotation_buf_info = py::buffer_info(&rotation.x, sizeof(float), py::format_descriptor<float>::format(), 1, quat_shape, strides);
    py::array_t<float> rotation_array(rotation_buf_info);

    py::buffer_info scale_buf_info = py::buffer_info(&scale.x, sizeof(float), py::format_descriptor<float>::format(), 1, shape, strides);
    py::array_t<float> scale_array(scale_buf_info);

    return py::make_tuple(translation_array, rotation_array, scale_array);
}

py::array_t<float> transform_compose(const py::array_t<float>& translation, const py::array_t<float>& rotation, const py::array_t<float>& scale)
{
    py::buffer_info translation_buf_info = translation.request();
    if (translation_buf_info.ndim != 1)
        throw std::runtime_error("Number of dimensions must be one");
    if (translation_buf_info.shape[0] != 3)
        throw std::runtime_error("Translation shape must be 3");

    py::buffer_info rotation_buf_info = rotation.request();
    if (rotation_buf_info.ndim != 1)
        throw std::runtime_error("Number of dimensions must be one");
    if (rotation_buf_info.shape[0] != 4)
        throw std::runtime_error("Rotation shape must be 4");

    py::buffer_info scale_buf_info = scale.request();
    if (scale_buf_info.ndim != 1)
        throw std::runtime_error("Number of dimensions must be one");
    if (scale_buf_info.shape[0] != 3)
        throw std::runtime_error("Scale shape must be 3");

    glm::vec3 translation_vec = *(glm::vec3*)translation_buf_info.ptr;
    glm::quat rotation_quat = *(glm::quat*)rotation_buf_info.ptr;
    glm::vec3 scale_vec = *(glm::vec3*)scale_buf_info.ptr;

    // Shift the rotation quaternion to the w component
    rotation_quat = glm::quat(rotation_quat.x, rotation_quat.y, rotation_quat.z, rotation_quat.w);

    glm::mat4 transform_mat = glm::translate(glm::mat4(1.0f), translation_vec) * glm::mat4_cast(rotation_quat) * glm::scale(glm::mat4(1.0f), scale_vec);

    py::array::ShapeContainer shape = { 4, 4 };
    py::array::StridesContainer strides = { 4 * sizeof(float), sizeof(float) };
    py::buffer_info buf_info = py::buffer_info(&transform_mat[0][0], sizeof(float), py::format_descriptor<float>::format(), 2, shape, strides);
    py::array_t<float> transform_array(buf_info);

    return transform_array;
}

py::tuple transform_bone(const py::array_t<float>& translation, const py::array_t<float>& rotation, const py::array_t<float>& scale, const py::array_t<float>& parent, const py::array_t<float>& inv_transform, const py::array_t<float>& offset)
{
    py::buffer_info translation_buf_info = translation.request();
    if (translation_buf_info.ndim != 1)
        throw std::runtime_error("Number of dimensions must be one");
    if (translation_buf_info.shape[0] != 3)
        throw std::runtime_error("Translation shape must be 3");

    py::buffer_info rotation_buf_info = rotation.request();
    if (rotation_buf_info.ndim != 1)
        throw std::runtime_error("Number of dimensions must be one");
    if (rotation_buf_info.shape[0] != 4)
        throw std::runtime_error("Rotation shape must be 4");

    py::buffer_info scale_buf_info = scale.request();
    if (scale_buf_info.ndim != 1)
        throw std::runtime_error("Number of dimensions must be one");
    if (scale_buf_info.shape[0] != 3)
        throw std::runtime_error("Scale shape must be 3");

    py::buffer_info parent_buf_info = parent.request();
    if (parent_buf_info.ndim != 2)
        throw std::runtime_error("Number of dimensions must be two");
    if (parent_buf_info.shape[0] != 4 || parent_buf_info.shape[1] != 4)
        throw std::runtime_error("Shape must be 4x4");

    py::buffer_info inv_transform_buf_info = inv_transform.request();
    if (inv_transform_buf_info.ndim != 2)
        throw std::runtime_error("Number of dimensions must be two");
    if (inv_transform_buf_info.shape[0] != 4 || inv_transform_buf_info.shape[1] != 4)
        throw std::runtime_error("Shape must be 4x4");

    py::buffer_info offset_buf_info = offset.request();
    if (offset_buf_info.ndim != 2)
        throw std::runtime_error("Number of dimensions must be two");
    if (offset_buf_info.shape[0] != 4 || offset_buf_info.shape[1] != 4)
        throw std::runtime_error("Shape must be 4x4");

    glm::vec3 translation_vec = *(glm::vec3*)translation_buf_info.ptr;
    glm::quat rotation_quat = *(glm::quat*)rotation_buf_info.ptr;
    glm::vec3 scale_vec = *(glm::vec3*)scale_buf_info.ptr;
    glm::mat4 parent_mat = *(glm::mat4*)parent_buf_info.ptr;
    glm::mat4 inv_transform_mat = *(glm::mat4*)inv_transform_buf_info.ptr;
    glm::mat4 offset_mat = *(glm::mat4*)offset_buf_info.ptr;

    glm::mat4 local_transform = glm::translate(glm::mat4(1.0f), translation_vec) * glm::mat4_cast(rotation_quat) * glm::scale(glm::mat4(1.0f), scale_vec);
    glm::mat4 model_transform = parent_mat * local_transform;

    glm::mat4 bone_transform = inv_transform_mat * model_transform * offset_mat;

    glm::vec3 new_translation;
    glm::quat new_rotation;
    glm::vec3 new_scale;
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(bone_transform, new_scale, new_rotation, new_translation, skew, perspective);

    py::array::ShapeContainer shape = { 4, 4 };
    py::array::StridesContainer strides = { 4 * sizeof(float), sizeof(float) };
    py::buffer_info buf_info = py::buffer_info(&model_transform[0][0], sizeof(float), py::format_descriptor<float>::format(), 2, shape, strides);
    py::array_t<float> model_array(buf_info);

    std::vector<float> new_translation_vec = { new_translation.x, new_translation.y, new_translation.z };
    std::vector<float> new_rotation_vec = { new_rotation.w, new_rotation.x, new_rotation.y, new_rotation.z };
    std::vector<float> new_scale_vec = { new_scale.x, new_scale.y, new_scale.z };

    return py::make_tuple(new_translation_vec, new_rotation_vec, new_scale_vec, model_array);
}

// Define the Python module
PYBIND11_MODULE(slayer_bindings, m) {
    m.doc() = "Slayer bindings tools";
    m.def_submodule("assimp")
        .def("load_bone_data", &load_bone_data, "Compute bone weights, influencing bones, and offset matrices for a given mesh file path");
    m.def_submodule("glm")
        .def("transform_decompose", &transform_decompose, "Decompose a 4x4 transformation matrix into a translation, rotation, and scale")
        .def("transform_compose", &transform_compose, "Compose a 4x4 transformation matrix from a translation, rotation, and scale")
        .def("transform_bone", &transform_bone, "Transforms a local bone transform to global space");

}