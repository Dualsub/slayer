#pragma once

#include "Core/Core.h"
#include "GameTypesDecl.h"
#include "Resources/Asset.h"
#include "Rendering/Renderer/SkeletalModel.h"
#include "Rendering/Animation/AnimationState.h"

#define ENGINE_COMPONENTS \
    Slayer::EntityID, \
    Slayer::Transform, \
    Slayer::ModelRenderer, \
    Slayer::SkeletalRenderer, \
    Slayer::SkeletalSockets, \
    Slayer::SocketAttacher, \
    Slayer::AnimationPlayer, \
    Slayer::DirectionalLight

namespace Slayer {

    struct EntityID
    {
        AssetID id;

        EntityID() = default;
        ~EntityID() = default;

        template<typename Serializer>
        void Transfer(Serializer& serializer)
        {
            SL_TRANSFER_VAR(id);
        }
    };

    struct TagComponent
    {
        std::string tag;

        TagComponent() = default;
        TagComponent(const std::string& tag) :
            tag(tag)
        {
        }
        ~TagComponent() = default;

        template<typename Serializer>
        void Transfer(Serializer& serializer)
        {
            SL_TRANSFER_VAR(tag);
        }
    };

    struct Transform
    {
        AssetID parentId;
        Mat4 worldTransform = Mat4(1.0f);
        Vec3 position = Vec3(0.0f);
        Quat rotation = Quat(0.0f, 0.0f, 0.0f, 1.0f);
        Vec3 eulerProxy = Vec3(0.0f);
        Vec3 scale = Vec3(1.0f);

        Transform() = default;
        Transform(const Vec3& position, const Quat& rotation, const Vec3& scale) :
            position(position), rotation(rotation), scale(scale)
        {
            eulerProxy = glm::eulerAngles(rotation) * 180.0f / glm::pi<float>();
        }
        ~Transform() = default;

        Mat4 GetMatrix() const
        {
            Mat4 matrix = Mat4(1.0f);
            matrix = glm::translate(matrix, position);
            matrix = matrix * glm::mat4_cast(rotation);
            matrix = glm::scale(matrix, scale);
            return matrix;
        }

        template<typename Serializer>
        void Transfer(Serializer& serializer)
        {
            SL_TRANSFER_VAR(parentId);
            SL_TRANSFER_VAR(position);
            SL_TRANSFER_VAR(scale);
            // We serialize the rotation as a Euler angle because it's easier to read, in degrees
            if (serializer.GetFlags() == SerializationFlags::Write)
            {
                Vec3 euler;
                serializer.Transfer(euler, "rotation");
                eulerProxy = euler;
                euler *= glm::pi<float>() / 180.0f;
                rotation = Quat(euler);
            }
            else if (serializer.GetFlags() == SerializationFlags::Read)
            {
                Vec3 euler = glm::eulerAngles(rotation);
                euler *= 180.0f / glm::pi<float>();
                serializer.Transfer(euler, "rotation");
            }
            else if (serializer.GetFlags() == SerializationFlags::ReadWrite)
            {
                rotation = Quat(eulerProxy * glm::pi<float>() / 180.0f);
                serializer.Transfer(eulerProxy, "rotation");
            }
        }
    };

    struct ModelRenderer
    {
        AssetID modelID;
        AssetID materialID;

        ModelRenderer() = default;
        ModelRenderer(const AssetID& modelID, const AssetID& materialID) :
            modelID(modelID), materialID(materialID)
        {
        }
        ~ModelRenderer() = default;

        template<typename Serializer>
        void Transfer(Serializer& serializer)
        {
            SL_TRANSFER_ASSET(modelID, AssetType::SL_ASSET_TYPE_MODEL);
            SL_TRANSFER_ASSET(materialID, AssetType::SL_ASSET_TYPE_MATERIAL);
        }
    };

    struct SkeletalRenderer
    {
        AssetID modelID;
        AssetID materialID;
        AnimationState state;

        SkeletalRenderer() = default;
        SkeletalRenderer(const AssetID& modelID, const AssetID& materialID) :
            modelID(modelID), materialID(materialID)
        {
        }
        ~SkeletalRenderer() = default;

        template<typename Serializer>
        void Transfer(Serializer& serializer)
        {
            SL_TRANSFER_ASSET(modelID, AssetType::SL_ASSET_TYPE_SKELETAL_MODEL);
            SL_TRANSFER_ASSET(materialID, AssetType::SL_ASSET_TYPE_MATERIAL);
        }
    };

    // Socket on a skeletal model
    struct SkeletalSockets
    {
        Vector<Socket> sockets;
        Dict<std::string, Mat4> worldTransforms;

        SkeletalSockets() = default;
        ~SkeletalSockets() = default;

        const Mat4& GetWorldTransform(const std::string& name) const
        {
            SL_ASSERT(worldTransforms.find(name) != worldTransforms.end() && "Can't find socket.");
            return worldTransforms.at(name);
        }

        template<typename Serializer>
        void Transfer(Serializer& serializer)
        {
            SL_TRANSFER_VEC(sockets);
            if (serializer.GetFlags() == SerializationFlags::Write)
            {
                for (auto& socket : sockets)
                {
                    worldTransforms[socket.name] = Mat4(1.0f);
                }
            }
        }
    };

    // Attaches a entity to a socket on a skeletal model
    struct SocketAttacher
    {
        std::string name;

        SocketAttacher() = default;
        ~SocketAttacher() = default;

        template<typename Serializer>
        void Transfer(Serializer& serializer)
        {
            SL_TRANSFER_VAR(name);
        }
    };

    struct AnimationPlayer
    {
        struct AnimationClip
        {
            AssetID animationID;
            float time = 0.0f;
            float weight = 1.0f;

            AnimationClip() = default;
            AnimationClip(const AssetID& animationID, float time = 0.0f, float weight = 1.0f) :
                animationID(animationID), time(time), weight(weight)
            {
            }

            ~AnimationClip() = default;

            template<typename Serializer>
            void Transfer(Serializer& serializer)
            {
                SL_TRANSFER_ASSET(animationID, AssetType::SL_ASSET_TYPE_ANIMATION);
                SL_TRANSFER_VAR(weight);
            }
        };

        Vector<AnimationClip> animationClips;

        AnimationPlayer() = default;
        AnimationPlayer(const AssetID& animationID, float time = 0.0f) :
            animationClips({ AnimationClip(animationID, time) })
        {
        }

        AnimationPlayer(const std::initializer_list<AnimationClip> animations) :
            animationClips(animations)
        {
        }

        ~AnimationPlayer() = default;

        template<typename Serializer>
        void Transfer(Serializer& serializer)
        {
            SL_TRANSFER_VEC(animationClips);
        }
    };

    struct DirectionalLight
    {
        Vec3 orientation = Vec3(0.0f); // Euler angles
        Vec3 color = Vec3(1.0f);

        DirectionalLight() = default;
        DirectionalLight(const Vec3& orientation, const Vec3& color) :
            orientation(orientation), color(color)
        {

        }

        ~DirectionalLight() = default;

        template<typename Serializer>
        void Transfer(Serializer& serializer)
        {
            SL_TRANSFER_VAR(orientation);
            SL_TRANSFER_VAR(color);
        }
    };

    template<typename... Components>
    void ForEachComponentTypeImpl(auto&& f)
    {
        // Call the templated lambda with each component as a template parameter
        (f.template operator() < Components > (), ...);
    }

    void ForEachComponentType(auto&& f)
    {
        ForEachComponentTypeImpl<ENGINE_COMPONENTS, GAME_COMPONENTS>(f);
    }
}


