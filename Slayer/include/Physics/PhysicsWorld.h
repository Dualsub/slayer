#pragma once

#include "Core/Core.h"
#include "Core/Containers.h"
#include "Physics/CollisionShapes.h"
#include "Physics/Layers.h"
#include "Physics/Listeners.h"

#include "Jolt/Core/TempAllocator.h"
#include "Jolt/Core/JobSystemThreadPool.h"
#include "Jolt/Physics/PhysicsSettings.h"

#include <thread>

#define SL_MAX_PHYSICS_BODIES 512

namespace Slayer {

    enum RigidBodyType
    {
        SL_RIGID_BODY_TYPE_STATIC,
        SL_RIGID_BODY_TYPE_DYNAMIC,
        SL_RIGID_BODY_TYPE_KINEMATIC
    };

    // Info for creating a rigid body, not runtime data
    struct RigidBodySettings
    {
        Vec3 position = Vec3(0.0f);
        Quat rotation = glm::identity<Quat>();

        RigidBodyType type = SL_RIGID_BODY_TYPE_STATIC;
        float mass = 1.0f;

        Unique<CollisionShape> shape;
    };

    struct RigidBodyTransform
    {
        Vec3 position;
        Quat rotation;
    };

    class PhysicsWorld
    {
    private:
        Unique<JPH::PhysicsSystem> m_physicsSystem;

        BPLayerInterfaceImpl m_broadPhaseLayerInterface;
        ObjectVsBroadPhaseLayerFilterImpl m_objectVsBroadphaseLayerFilter;
        ObjectLayerPairFilterImpl m_objectLayerPairFilter;
        BodyActivationListener m_bodyActivationListener;
        ContactListener m_contactListener;

        static Unique<JPH::TempAllocatorImpl> s_tempAllocator;
        static Unique<JPH::JobSystemThreadPool> s_jobSystem;

        Dict<uint32_t, JPH::BodyID> m_bodyIDs;
    public:
        PhysicsWorld() = default;
        ~PhysicsWorld() = default;

        void Initialize();
        void Shutdown();

        void StepSimulation(float dt);
        RigidBodyTransform GetTransform(uint32_t id);

        void AddRigidBody(uint32_t id, RigidBodySettings& info);
        void RemoveRigidBody(uint32_t id);
        // Function to remove all rigid bodies
        void RemoveAllRigidBodies();
    };
}