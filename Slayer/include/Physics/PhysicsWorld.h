#pragma once

#include "Core/Core.h"
#include "Core/Containers.h"
#include "Physics/CollisionShapes.h"
#include "Physics/Layers.h"
#include "Physics/Listeners.h"

#include "Jolt/Core/TempAllocator.h"
#include "Jolt/Core/JobSystemThreadPool.h"
#include "Jolt/Physics/PhysicsSettings.h"
#include "Jolt/Physics/Character/Character.h"

#include <thread>

#define SL_MAX_PHYSICS_BODIES 512

namespace Slayer {

    using BodyID = uint32_t;

    enum BodyType : uint8_t
    {
        SL_BODY_TYPE_RIGID,
        SL_BODY_TYPE_CHARACTER
    };

    enum CharacterGroundState : uint8_t
    {
        // On ground is 0 so that we can use it as a bool
        SL_CHARACTER_GROUND_STATE_GROUNDED = JPH::Character::EGroundState::OnGround,
        SL_CHARACTER_GROUND_STATE_ON_STEEP_SLOPE = JPH::Character::EGroundState::OnSteepGround,
        SL_CHARACTER_GROUND_STATE_NOT_SUPPORTED = JPH::Character::EGroundState::NotSupported,
        SL_CHARACTER_GROUND_STATE_IN_AIR = JPH::Character::EGroundState::InAir,
        SL_CHARACTER_GROUND_STATE_UNKNOWN
    };

    // Info for creating a rigid body, not runtime data
    struct RigidBodySettings
    {
        Vec3 position = Vec3(0.0f);
        Quat rotation = glm::identity<Quat>();

        float mass = 1.0f;

        Unique<CollisionShape> shape;
    };

    struct RigidBodyTransform
    {
        Vec3 position;
        Quat rotation;
        Vec3 linearVelocity;
        Vec3 angularVelocity;
    };

    class PhysicsWorld
    {
    private:
        BodyID m_nextBodyID = 0;

        Unique<JPH::PhysicsSystem> m_physicsSystem;

        BPLayerInterfaceImpl m_broadPhaseLayerInterface;
        ObjectVsBroadPhaseLayerFilterImpl m_objectVsBroadphaseLayerFilter;
        ObjectLayerPairFilterImpl m_objectLayerPairFilter;
        BodyActivationListener m_bodyActivationListener;
        ContactListener m_contactListener;

        static Unique<JPH::TempAllocatorImpl> s_tempAllocator;
        static Unique<JPH::JobSystemThreadPool> s_jobSystem;

        Dict<uint32_t, JPH::BodyID> m_bodyIDs;
        Dict<uint32_t, Unique<JPH::Character>> m_characters;
    public:
        PhysicsWorld() = default;
        ~PhysicsWorld() = default;

        void Initialize();
        void Shutdown();

        void StepSimulation(float dt);
        RigidBodyTransform GetRigidBodyState(BodyID id);

        BodyID CreateRigidBody(RigidBodySettings& info, BodyType type);
        void UpdateRigidBody(BodyID id, RigidBodySettings& info);
        void RemoveRigidBody(BodyID id);
        void RemoveAllRigidBodies();

        void SetPosition(BodyID id, Vec3 position);
        void SetRotation(BodyID id, Quat rotation);
        void SetLinearVelocity(BodyID id, Vec3 velocity);
        void SetAngularVelocity(BodyID id, Vec3 velocity);

        CharacterGroundState GetCharacterGroundState(BodyID id);
    };
}