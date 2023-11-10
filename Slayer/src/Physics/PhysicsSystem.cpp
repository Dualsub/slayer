#include "Physics/PhysicsSystem.h"

#include "Scene/World.h"
#include "Rendering/Renderer/Renderer.h"

namespace Slayer {

    template<typename Shape, typename... ShapeArgs>
    void CreateBodyWithShape(Entity entity, RigidBody* body, Transform* transform, PhysicsWorld& pw, BodyType type, ShapeArgs... args)
    {
        RigidBodySettings settings;
        settings.mass = body->mass;
        settings.position = transform->position;
        settings.rotation = transform->rotation;
        settings.shape = MakeUnique<Shape>(args...);
        BodyID id = pw.CreateRigidBody(settings, type);
        body->id = id;
    }

    template<typename ComponentType>
    void CreateBodies(ComponentStore& store, PhysicsWorld& pw, BodyType type)
    {
        store.ForTransitionTo<ComponentType, BoxCollider, Transform>([&pw, &type](Entity entity, ComponentType* body, BoxCollider* collider, Transform* transform) {
            CreateBodyWithShape<BoxShape>(entity, body, transform, pw, type, collider->halfExtents);
            });

        store.ForTransitionTo<ComponentType, SphereCollider, Transform>([&pw, &type](Entity entity, ComponentType* body, SphereCollider* collider, Transform* transform) {
            CreateBodyWithShape<SphereShape>(entity, body, transform, pw, type, collider->radius);
            });

        store.ForTransitionTo<ComponentType, CapsuleCollider, Transform>([&pw, &type](Entity entity, ComponentType* body, CapsuleCollider* collider, Transform* transform) {
            CreateBodyWithShape<CapsuleShape>(entity, body, transform, pw, type, collider->radius, collider->halfHeight);
            });

    }

    void PhysicsSystem::Update(float dt, ComponentStore& store)
    {
        PhysicsWorld& pw = World::GetPhysicsWorld();
        // When a rigid body is added to the store, add it to the physics world
        CreateBodies<RigidBody>(store, pw, BodyType::SL_BODY_TYPE_RIGID);
        CreateBodies<CharacterBody>(store, pw, BodyType::SL_BODY_TYPE_CHARACTER);

        // When a rigid body is removed from the store, remove it from the physics world
        store.ForTransitionFrom<RigidBody>([&pw](Entity entity, RigidBody* body) {
            pw.RemoveRigidBody(entity);
            });

        // Update the physics world
        pw.StepSimulation(dt);

        store.ForEachAsync<RigidBody, Transform>([&pw](Entity entity, RigidBody* body, Transform* transform) {
            RigidBodyTransform rbTransform = pw.GetRigidBodyState(body->id);
            transform->position = rbTransform.position;
            transform->rotation = rbTransform.rotation;
            });

        store.ForEachAsync<CharacterBody, Transform>([&pw](Entity entity, RigidBody* body, Transform* transform) {
            RigidBodyTransform rbTransform = pw.GetRigidBodyState(body->id);
            transform->position = rbTransform.position;
            transform->rotation = rbTransform.rotation;

            CharacterGroundState state = pw.GetCharacterGroundState(body->id);
            switch (state)
            {
            case CharacterGroundState::SL_CHARACTER_GROUND_STATE_GROUNDED:
                std::cout << "On ground" << std::endl;
                break;
            case CharacterGroundState::SL_CHARACTER_GROUND_STATE_IN_AIR:
                std::cout << "In air" << std::endl;
                break;
            case CharacterGroundState::SL_CHARACTER_GROUND_STATE_NOT_SUPPORTED:
                std::cout << "Not supported" << std::endl;
                break;
            case CharacterGroundState::SL_CHARACTER_GROUND_STATE_ON_STEEP_SLOPE:
                std::cout << "On steep slope" << std::endl;
                break;
            case CharacterGroundState::SL_CHARACTER_GROUND_STATE_UNKNOWN:
            default:
                std::cout << "Unknown" << std::endl;
                break;
            }

            });

    }

}