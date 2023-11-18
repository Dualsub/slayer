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

        const float collisionTolerance = 0.05f;
        for (auto& [id, character] : m_characters)
        {
            character->PostSimulation(collisionTolerance);
        }
    }

    RigidBodyState PhysicsWorld::GetRigidBodyState(BodyID id)
    {
        JPH::BodyInterface& interface = m_physicsSystem->GetBodyInterface();
        JPH::BodyID bodyId = m_bodyIDs[id];

        JPH::Vec3 position = interface.GetCenterOfMassPosition(bodyId);
        JPH::Quat rotation = interface.GetRotation(bodyId);
        JPH::Vec3 linearVelocity = interface.GetLinearVelocity(bodyId);
        JPH::Vec3 angularVelocity = interface.GetAngularVelocity(bodyId);

        return {
            JoltHelpers::ConvertWithUnits(position),
            JoltHelpers::Convert(rotation),
            JoltHelpers::ConvertWithUnits(linearVelocity),
            JoltHelpers::Convert(angularVelocity)
        };
    }

    Vec3 PhysicsWorld::GetLinearVelocity(BodyID id)
    {
        JPH::BodyInterface& interface = m_physicsSystem->GetBodyInterface();
        JPH::BodyID bodyId = m_bodyIDs[id];
        JPH::Vec3 linearVelocity = interface.GetLinearVelocity(bodyId);
        return JoltHelpers::ConvertWithUnits(linearVelocity);
    }

    BodyID PhysicsWorld::CreateRigidBody(RigidBodySettings& info, BodyType type)
    {
        SL_ASSERT(m_nextBodyID < std::numeric_limits<BodyID>::max() && "BodyID overflow");

        BodyID id = m_nextBodyID++;
        JPH::BodyInterface& interface = m_physicsSystem->GetBodyInterface();

        // Creating shape
        JPH::ShapeSettings::ShapeResult shapeResult = info.shape->GetShapeSettings();
        JPH::ShapeRefC shape = shapeResult.Get();

        if (type == BodyType::SL_BODY_TYPE_RIGID)
        {
            JPH::ObjectLayer layer = info.mass > 0.0f ? Layers::MOVING : Layers::NON_MOVING;
            JPH::EMotionType motionType = info.mass > 0.0f ? JPH::EMotionType::Dynamic : JPH::EMotionType::Static;

            JPH::BodyCreationSettings settings(shape, JoltHelpers::ConvertWithUnits(info.position), JoltHelpers::Convert(info.rotation), motionType, layer);
            JPH::BodyID bodyId = interface.CreateAndAddBody(settings, JPH::EActivation::Activate);
            m_bodyIDs[id] = bodyId;
        }
        else if (type == BodyType::SL_BODY_TYPE_CHARACTER)
        {
            JPH::CharacterSettings settings;
            settings.mShape = shape;
            settings.mLayer = Layers::MOVING;
            settings.mUp = JPH::Vec3::sAxisY();
            settings.mSupportingVolume = JPH::Plane(JPH::Vec3::sAxisY(), -shape->GetLocalBounds().GetExtent().GetY());
            settings.mFriction = info.friction;
            // Hardcoded for now
            settings.mMaxSlopeAngle = glm::radians(45.0f);

            Unique<JPH::Character> character = MakeUnique<JPH::Character>(&settings, JoltHelpers::ConvertWithUnits(info.position), JoltHelpers::Convert(info.rotation), 0, m_physicsSystem.get());
            character->AddToPhysicsSystem(JPH::EActivation::Activate);
            JPH::BodyID bodyId = character->GetBodyID();
            m_characters[id] = std::move(character);
            m_bodyIDs[id] = bodyId;
        }
        else
        {
            SL_ASSERT(false && "Unknown body type");
        }

        return id;
    }

    void PhysicsWorld::RemoveRigidBody(BodyID id)
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

        for (auto& [id, character] : m_characters)
        {
            character->RemoveFromPhysicsSystem();
        }
        m_characters.clear();
    }


    void PhysicsWorld::SetPosition(BodyID id, Vec3 position)
    {
        JPH::BodyInterface& interface = m_physicsSystem->GetBodyInterface();
        JPH::BodyID bodyId = m_bodyIDs[id];
        interface.SetPosition(bodyId, JoltHelpers::ConvertWithUnits(position), JPH::EActivation::Activate);
    }

    void PhysicsWorld::SetRotation(BodyID id, Quat rotation)
    {
        JPH::BodyInterface& interface = m_physicsSystem->GetBodyInterface();
        JPH::BodyID bodyId = m_bodyIDs[id];
        interface.SetRotation(bodyId, JoltHelpers::Convert(rotation), JPH::EActivation::Activate);
    }

    void PhysicsWorld::SetLinearVelocity(BodyID id, Vec3 velocity)
    {
        JPH::BodyInterface& interface = m_physicsSystem->GetBodyInterface();
        JPH::BodyID bodyId = m_bodyIDs[id];
        interface.SetLinearVelocity(bodyId, JoltHelpers::ConvertWithUnits(velocity));
    }

    void PhysicsWorld::SetAngularVelocity(BodyID id, Vec3 velocity)
    {
        JPH::BodyInterface& interface = m_physicsSystem->GetBodyInterface();
        JPH::BodyID bodyId = m_bodyIDs[id];
        interface.SetAngularVelocity(bodyId, JoltHelpers::Convert(velocity));
    }

    CharacterGroundState PhysicsWorld::GetCharacterGroundState(BodyID id)
    {
        if (m_characters.find(id) == m_characters.end())
            return CharacterGroundState::SL_CHARACTER_GROUND_STATE_UNKNOWN;

        JPH::Character* character = m_characters[id].get();
        JPH::Character::EGroundState state = character->GetGroundState();
        return static_cast<CharacterGroundState>(state);
    }

    void PhysicsWorld::SetCharacterRotation(BodyID id, Quat rotation)
    {
        if (m_characters.find(id) == m_characters.end())
            return;

        JPH::Character* character = m_characters[id].get();
        character->SetRotation(JoltHelpers::Convert(rotation));
    }
}