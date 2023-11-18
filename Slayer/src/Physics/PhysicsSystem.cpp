#include "Physics/PhysicsSystem.h"

#include "Core/Log.h"
#include "Scene/World.h"
#include "Input/Input.h"
#include "Rendering/Renderer/Renderer.h"

namespace Slayer {

    Vec3 HermiteSpline(Vec3 p0, Vec3 m0, Vec3 p1, Vec3 m1, float x0, float x1, float x)
    {
        float t = glm::min((x - x0) / (x1 - x0), 1.0f);
        float t2 = t * t;
        float t3 = t2 * t;

        float h00 = 2.0f * t3 - 3.0f * t2 + 1.0f;
        float h01 = t3 - 2.0f * t2 + t;
        float h10 = -2.0f * t3 + 3.0f * t2;
        float h11 = t3 - t2;

        Vec3 interpolatedPoint = (h00 * p0) + (h01 * (x1 - x0) * m0) + (h10 * p1) + (h11 * (x1 - x0) * m1);

        return interpolatedPoint;
    }

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
        body->currentState = pw.GetRigidBodyState(id);
        body->lastState = body->currentState;
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

    void PhysicsSystem::OnActivated(ComponentStore& store)
    {
        PhysicsWorld& pw = World::GetPhysicsWorld();

        // When a rigid body is added to the store, add it to the physics world
        CreateBodies<RigidBody>(store, pw, BodyType::SL_BODY_TYPE_RIGID);
        CreateBodies<CharacterBody>(store, pw, BodyType::SL_BODY_TYPE_CHARACTER);
    }

    void PhysicsSystem::FixedUpdate(Timespan dt, ComponentStore& store)
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

        store.ForEachAsync<RigidBody>([&pw](Entity entity, RigidBody* body)
            {
                body->lastState = body->currentState;
                body->currentState = pw.GetRigidBodyState(body->id);
            }
        );

        store.ForEachAsync<CharacterBody>([&pw](Entity entity, RigidBody* body)
            {
                body->lastState = body->currentState;
                body->currentState = pw.GetRigidBodyState(body->id);
            }
        );

    }

    void PhysicsSystem::Update(Timespan dt, ComponentStore& store)
    {
        PhysicsWorld& pw = World::GetPhysicsWorld();

        const float fixedTimeStep = Application::Get()->GetFixedDeltaTime();
        const float timeSinceLastStep = Application::Get()->GetTimeSinceFixedUpdate();
        const float alpha = glm::min(timeSinceLastStep / fixedTimeStep, 1.0f);

        store.ForEachAsync<RigidBody, Transform>([&pw, &alpha, timeSinceLastStep, fixedTimeStep](Entity entity, RigidBody* body, Transform* transform) {
            transform->position = body->interpolatePosition ? glm::mix(body->lastState.position, body->currentState.position, alpha) : body->currentState.position;
            transform->rotation = body->interpolateRotation ? glm::slerp(body->lastState.rotation, body->currentState.rotation, alpha) : body->currentState.rotation;
            });

        store.ForEachAsync<CharacterBody, Transform>([&pw, &alpha, timeSinceLastStep, fixedTimeStep](Entity entity, CharacterBody* body, Transform* transform) {
            // Nice O.o
            transform->position = body->interpolatePosition ? (body->hermiteInterpolation ? HermiteSpline(
                body->lastState.position,
                body->lastState.linearVelocity,
                body->currentState.position,
                body->currentState.linearVelocity,
                0.0f,
                fixedTimeStep,
                timeSinceLastStep
            ) : glm::mix(body->lastState.position, body->currentState.position, alpha)) : body->currentState.position;
            transform->rotation = body->interpolateRotation ? glm::slerp(body->lastState.rotation, body->currentState.rotation, alpha) : body->currentState.rotation;
            });
    }

}