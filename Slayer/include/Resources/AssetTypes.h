#pragma once

#include "Core/Core.h"
#include "Core/Containers.h"
#include "Resources/Asset.h"
#include "Rendering/Animation/AnimationChannel.h"
#include "Rendering/Animation/Socket.h"
#include "Serialization/BinarySerializer.h"

namespace Slayer
{
    struct TextureAsset
    {
        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t channels = 0;
        uint32_t target = 0;
        Vector<uint8_t> data = {};

        TextureAsset() = default;
        ~TextureAsset() = default;

        template<typename Serializer>
        void Transfer(Serializer& serializer)
        {
			SL_TRANSFER_VAR(width);
			SL_TRANSFER_VAR(height);
			SL_TRANSFER_VAR(channels);
			SL_TRANSFER_VAR(target);
            serializer.TransferVectorPacked(data, "data");
		}
    };

    struct ShaderAsset
    {
        std::string vsSource = "";
        std::string fsSource = "";

        template<typename Serializer>
        void Transfer(Serializer& serializer)
        {
			SL_TRANSFER_VAR(vsSource);
			SL_TRANSFER_VAR(fsSource);
		}
    };

    struct MaterialAsset
    {
        // Next is a map of texture names to texture ids.
        struct MaterialTexture
        {
            uint8_t type = 0;
            AssetID textureId = 0;

            MaterialTexture() = default;
            ~MaterialTexture() = default;

            template<typename Serializer>
            void Transfer(Serializer& serializer)
            {
				SL_TRANSFER_VAR(type);
				SL_TRANSFER_VAR(textureId);
			}
        };

        Vector<MaterialTexture> textures = {};

        template<typename Serializer>
        void Transfer(Serializer& serializer)
        {
			SL_TRANSFER_VEC(textures);
		}
     
        MaterialAsset() = default;
        ~MaterialAsset() = default;
    };

    struct ModelAsset
    {
        // Next is a map of texture names to texture ids.
        struct MeshAsset
        {
            Vector<float> vertices = {};
            Vector<uint32_t> indices = {};

            MeshAsset() = default;
            ~MeshAsset() = default;

            template<typename Serializer>
            void Transfer(Serializer& serializer)
            {
                serializer.TransferVectorPacked(vertices, "vertices");
                serializer.TransferVectorPacked(indices, "indices");
            }
        };

        Vector<MeshAsset> meshes = {};

        ModelAsset() = default;
        ~ModelAsset() = default;

        template<typename Serializer>
        void Transfer(Serializer& serializer)
        {
            SL_TRANSFER_VEC(meshes);
        }
    };

    struct Bone
    {
        std::string name = "";
        int32_t id = 0;
        int32_t parentId = 0;
        Mat4 transform = Mat4(1.0f);

        Bone() = default;
        ~Bone() = default;

        template<typename Serializer>
        void Transfer(Serializer& serializer)
        {
            SL_TRANSFER_VAR(name);
            SL_TRANSFER_VAR(id);
            SL_TRANSFER_VAR(parentId);
            SL_TRANSFER_VAR(transform);
        }
    };

#pragma pack(push, 1)
    struct SkeletalMeshVertex
    {
        float positions[3];
        float texCoords[2];
        float normals[3];
        uint32_t boneIds[4];
        float weights[4];
    };
#pragma pack(pop)

    struct SkeletalModelAsset
    {
        struct SkeletalMesh
        {

            Vector<SkeletalMeshVertex> vertices = {};
            Vector<uint32_t> indices = {};
            Vector<Bone> bones = {};
            Mat4 globalInverseTransform = Mat4(1.0f);

            template<typename Serializer>
            void Transfer(Serializer& serializer)
            {
                serializer.TransferVectorPacked(vertices, "vertices");
                serializer.TransferVectorPacked(indices, "indices");
                SL_TRANSFER_VEC(bones);
                SL_TRANSFER_VAR(globalInverseTransform);
            }
        };

        Vector<SkeletalMesh> meshes = {};
        Vector<Socket> sockets = {};

        template<typename Serializer>
        void Transfer(Serializer& serializer)
        {
			SL_TRANSFER_VEC(meshes);
            SL_TRANSFER_VEC(sockets);
		}

        SkeletalModelAsset() = default;
        ~SkeletalModelAsset() = default;
    };

    struct AnimationAsset
    {
        float duration = 0.0f;
        float ticksPerSecond = 0.0f;
        uint32_t numChannels = 0;
        Vector<float> times = {};
        Vector<float> data = {};

        template<typename Serializer>
        void Transfer(Serializer& serializer)
        {
			SL_TRANSFER_VAR(duration);
			SL_TRANSFER_VAR(ticksPerSecond);
            SL_TRANSFER_VAR(numChannels);
            serializer.TransferVectorPacked(times, "times");
            serializer.TransferVectorPacked(data, "data");
		}
    };

}