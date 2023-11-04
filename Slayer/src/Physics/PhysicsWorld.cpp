#include "Physics/PhysicsWorld.h"

#include "Physics/Helpers.h"
#include "Physics/Debug.h"

// Jolt includes
#include "Jolt/Jolt.h"
#include "Jolt/RegisterTypes.h"
#include "Jolt/Physics/Collision/Shape/BoxShape.h"
#include "Jolt/Physics/Collision/Shape/SphereShape.h"
#include "Jolt/Physics/Body/BodyCreationSettings.h"
#include "Jolt/Physics/Body/BodyActivationListener.h"

#include <cstdarg>
#include <thread>

namespace Slayer {

    using namespace JPH::literals;

    Unique<JPH::TempAllocatorImpl> PhysicsWorld::s_tempAllocator;
    Unique<JPH::JobSystemThreadPool> PhysicsWorld::s_jobSystem;

    // Callback for traces, connect this to your own trace function if you have one
    static void TraceImpl(const char* inFMT, ...)
    {
        // Format the message
        std::va_list list;
        va_start(list, inFMT);
        char buffer[1024];
        vsnprintf(buffer, sizeof(buffer), inFMT, list);
        va_end(list);

        // Print to the TTY
        std::cout << buffer << std::endl;
    }

    void PhysicsWorld::Initialize()
    {
        JPH::RegisterDefaultAllocator();

        JPH::Trace = TraceImpl;

        JPH::Factory::sInstance = new JPH::Factory();

        JPH::RegisterTypes();

        s_tempAllocator = MakeUnique<JPH::TempAllocatorImpl>(10 * 1024 * 1024);
        s_jobSystem = MakeUnique<JPH::JobSystemThreadPool>(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, 1);

        const uint32_t maxBodies = 1024;
        const uint32_t numBodyMutexes = 0;
        const uint32_t maxBodyPairs = 65536;
        const uint32_t maxContactConstraints = 10240;

        m_physicsSystem = MakeUnique<JPH::PhysicsSystem>();

        m_physicsSystem->Init(maxBodies, numBodyMutexes, maxBodyPairs, maxContactConstraints, m_broadPhaseLayerInterface, m_objectVsBroadphaseLayerFilter, m_objectLayerPairFilter);

        m_physicsSystem->SetBodyActivationListener(&m_bodyActivationListener);

        m_physicsSystem->SetContactListener(&m_contactListener);
    }

    void PhysicsWorld::Shutdown()
    {
        // Unregisters all types with the factory and cleans up the default material
        JPH::UnregisterTypes();

        // Destroy the factory
        delete JPH::Factory::sInstance;
        JPH::Factory::sInstance = nullptr;
    }

    void PhysicsWorld::StepSimulation(float dt)
    {
        m_physicsSystem->Update(dt, 1, s_tempAllocator.get(), s_jobSystem.get());
    }

    RigidBodyTransform PhysicsWorld::GetTransform(uint32_t id)
    {
        JPH::BodyInterface& interface = m_physicsSystem->GetBodyInterface();
        JPH::BodyID bodyId = m_bodyIDs[id];

        JPH::Vec3 position = interface.GetCenterOfMassPosition(bodyId);
        JPH::Quat rotation = interface.GetRotation(bodyId);

        return {
            JoltHelpers::Convert(position),
            JoltHelpers::Convert(rotation)
        };
    }

    void PhysicsWorld::AddRigidBody(uint32_t id, RigidBodySettings& info)
    {
        JPH::BodyInterface& interface = m_physicsSystem->GetBodyInterface();

        // Creating shape
        JPH::ShapeSettings::ShapeResult shapeResult = info.shape->GetShapeSettings();
        JPH::ShapeRefC shape = shapeResult.Get();

        // Creating body
        JPH::ObjectLayer layer = info.mass == 0.0f ? Layers::NON_MOVING : Layers::MOVING;
        JPH::EMotionType motionType = info.mass == 0.0f ? JPH::EMotionType::Static : JPH::EMotionType::Dynamic;
        JPH::BodyCreationSettings settings(shape, JoltHelpers::Convert(info.position), JoltHelpers::Convert(info.rotation), motionType, layer);
        JPH::BodyID bodyId = interface.CreateAndAddBody(settings, JPH::EActivation::Activate);
        m_bodyIDs[id] = bodyId;
    }

    void PhysicsWorld::RemoveRigidBody(uint32_t id)
    {
        JPH::BodyInterface& interface = m_physicsSystem->GetBodyInterface();
        JPH::BodyID bodyId = m_bodyIDs[id];
        interface.DestroyBody(bodyId);
        m_bodyIDs.erase(id);
    }

    void PhysicsWorld::RemoveAllRigidBodies()
    {
        JPH::BodyInterface& interface = m_physicsSystem->GetBodyInterface();
        for (auto& [id, bodyId] : m_bodyIDs)
        {
            interface.DestroyBody(bodyId);
        }
        m_bodyIDs.clear();
    }
}