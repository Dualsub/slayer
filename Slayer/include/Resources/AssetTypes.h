#pragma once

#include "Core/Core.h"
#include "Core/Containers.h"
#include "Resources/Asset.h"

namespace Slayer
{
    struct TextureAsset
    {
        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t channels = 0;
        uint32_t target = 0;
        std::vector<uint8_t> data = {};

        void Read(char* src)
        {
            char* srcPtr = src;
            Copy(srcPtr, &width, sizeof(width));
            srcPtr += sizeof(width);
            Copy(srcPtr, &height, sizeof(height));
            srcPtr += sizeof(height);
            Copy(srcPtr, &channels, sizeof(channels));
            srcPtr += sizeof(channels);
            Copy(srcPtr, &target, sizeof(target));
            srcPtr += sizeof(target);

            uint32_t bytesPerChannel = 1;
            if (target == uint32_t(0x8513))
            {
                bytesPerChannel = 4;
            }

            const size_t size = width * height * channels * bytesPerChannel;
            data.resize(size);         
            Copy(srcPtr, data.data(), size);
        }

        uint32_t GetSize() const
        {
            return sizeof(width) + sizeof(height) + sizeof(channels) + sizeof(target) + data.size();
        }

        TextureAsset() = default;
        ~TextureAsset() = default;
    };

    struct ShaderAsset
    {
        std::string vsSource = "";
        std::string fsSource = "";

        void Read(char* src)
        {
            char* srcPtr = src;
            uint32_t vsSourceLength;
            uint32_t fsSourceLength;
            Copy(srcPtr, &vsSourceLength, sizeof(vsSourceLength));
            srcPtr += sizeof(vsSourceLength);
            Copy(srcPtr, &fsSourceLength, sizeof(fsSourceLength));
            srcPtr += sizeof(fsSourceLength);

            vsSource = std::string(srcPtr, vsSourceLength);
            srcPtr += vsSourceLength;
            fsSource = std::string(srcPtr, fsSourceLength);
            srcPtr += fsSourceLength;
        }
    };

    struct MaterialAsset
    {
        // Next is a map of texture names to texture ids.
        struct MaterialTexture
        {
            uint8_t type = 0;
            AssetID textureId = 0;

            void Read(char* src)
            {
                char* srcPtr = src;
                Copy(srcPtr, &type, sizeof(type));
                srcPtr += sizeof(type);
                Copy(srcPtr, &textureId, sizeof(textureId));
                srcPtr += sizeof(textureId);
            }

            uint32_t GetSize() const
            {
                return sizeof(type) + sizeof(textureId);
            }

            MaterialTexture() = default;
            ~MaterialTexture() = default;
        };

        Vector<MaterialTexture> textures = {};

        void Read(char* src)
        {
            char* srcPtr = src;
            uint32_t textureCount;
            Copy(srcPtr, &textureCount, sizeof(textureCount));
            srcPtr += sizeof(textureCount);
            textures.resize(textureCount);
            for (uint32_t i = 0; i < textureCount; i++)
            {
                MaterialTexture texture;
                texture.Read(srcPtr);
                srcPtr += texture.GetSize();
                textures[i] = texture;
            }
        }

        uint32_t GetSize() const
        {
            uint32_t size = sizeof(uint32_t);
            for (const auto& texture : textures)
            {
                size += texture.GetSize();
            }
            return size;
        }

        MaterialAsset() = default;
        ~MaterialAsset() = default;
    };

    struct ModelAsset
    {
        uint32_t meshCount = 0;
        // Next is a map of texture names to texture ids.
        struct MeshAsset
        {
            std::vector<float> vertices = {};
            std::vector<uint32_t> indices = {};

            void Read(char* src)
            {
                char* srcPtr = src;
                uint32_t vertexCount;
                uint32_t indexCount;

                Copy(srcPtr, &vertexCount, sizeof(vertexCount));
                srcPtr += sizeof(vertexCount);
                Copy(srcPtr, &indexCount, sizeof(indexCount));
                srcPtr += sizeof(indexCount);

                size_t verticesSize = vertexCount;
                vertices.resize(verticesSize);
                Copy(srcPtr, vertices.data(), vertices.size() * sizeof(float));
                srcPtr += vertices.size() * sizeof(float);

                indices.resize(indexCount);
                Copy(srcPtr, indices.data(), indices.size() * sizeof(uint32_t));
                srcPtr += indices.size() * sizeof(uint32_t);
            }

            uint32_t GetSize() const
            {
                return sizeof(uint32_t) + sizeof(uint32_t) + vertices.size() * sizeof(float) + indices.size() * sizeof(uint32_t);
            }

            MeshAsset() = default;
            ~MeshAsset() = default;
        };

        Vector<MeshAsset> meshes = {};

        void Read(char* src)
        {
            char* srcPtr = src;
            uint32_t meshCount;

            Copy(srcPtr, &meshCount, sizeof(meshCount));
            srcPtr += sizeof(meshCount);
            for (uint32_t i = 0; i < meshCount; i++)
            {
                MeshAsset mesh;
                mesh.Read(srcPtr);
                srcPtr += mesh.GetSize();
                meshes.push_back(mesh);
            }
        }

        uint32_t GetSize() const
        {
            uint32_t size = sizeof(meshCount);
            for (const auto& mesh : meshes)
            {
                size += mesh.GetSize();
            }
            return size;
        }

        ModelAsset() = default;
        ~ModelAsset() = default;
    };

    struct SkeletalModelAsset
    {
        AssetID materialId = 0;
        uint32_t meshCount = 0;
        // Next is a map of texture names to texture ids.
        struct SkeletalMesh
        {
            uint32_t vertexCount = 0;
            uint32_t indexCount = 0;
            uint32_t materialIndex = 0;
            uint32_t boneCount = 0;
            uint32_t boneOffset = 0;

            SkeletalMesh() = default;
            ~SkeletalMesh() = default;
        };

        struct Bone
        {
            uint32_t nameLength = 0;
            uint32_t parentIndex = 0;
            Mat4 transform = Mat4(1.0f);

            Bone() = default;
            ~Bone() = default;
        };

        SkeletalModelAsset() = default;
        ~SkeletalModelAsset() = default;
    };


}