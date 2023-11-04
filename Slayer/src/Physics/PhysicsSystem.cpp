#include "Physics/PhysicsSystem.h"

#include "Scene/World.h"
#include "Rendering/Renderer/Renderer.h"

namespace Slayer {

    template<typename Shape, typename... ShapeArgs>
    void CreateRigidBodyWithShape(Entity entity, RigidBody* body, Transform* transform, PhysicsWorld& pw, ShapeArgs... args)
    {
        RigidBodySettings settings;
        settings.mass = body->mass;
        settings.type = RigidBodyType::SL_RIGID_BODY_TYPE_DYNAMIC;
        settings.position = transform->position;
        settings.rotation = transform->rotation;
        settings.shape = MakeUnique<Shape>(args...);
        pw.AddRigidBody(entity, settings);
    }

    void PhysicsSystem::Update(float dt, ComponentStore& store)
    {
        PhysicsWorld& pw = World::GetPhysicsWorld();
        // When a rigid body is added to the store, add it to the physics world
        store.ForTransitionTo<RigidBody, BoxCollider, Transform>([&pw](Entity entity, RigidBody* body, BoxCollider* collider, Transform* transform) {
            CreateRigidBodyWithShape<BoxShape>(entity, body, transform, pw, collider->halfExtents);
            });

        store.ForTransitionTo<RigidBody, SphereCollider, Transform>([&pw](Entity entity, RigidBody* body, SphereCollider* collider, Transform* transform) {
            CreateRigidBodyWithShape<SphereShape>(entity, body, transform, pw, collider->radius);
            });

        store.ForTransitionTo<RigidBody, CapsuleCollider, Transform>([&pw](Entity entity, RigidBody* body, CapsuleCollider* collider, Transform* transform) {
            CreateRigidBodyWithShape<CapsuleShape>(entity, body, transform, pw, collider->radius, collider->halfHeight);
            });

        // When a rigid body is removed from the store, remove it from the physics world
        store.ForTransitionFrom<RigidBody>([&pw](Entity entity, RigidBody* body) {
            pw.RemoveRigidBody(entity);
            });

        // Update the physics world
        pw.StepSimulation(dt);

        store.ForEach<RigidBody, Transform>([&pw](Entity entity, RigidBody* body, Transform* transform) {
            RigidBodyTransform rbTransform = pw.GetTransform(entity);
            transform->position = rbTransform.position;
            transform->rotation = rbTransform.rotation;
            });

    }

    void PhysicsSystem::Render(Renderer& renderer, ComponentStore& store)
    {
        store.ForEach<Transform, RigidBody, BoxCollider>([&renderer](Entity entity, Transform* transform, RigidBody* body, BoxCollider* collider) {

            const Vec4 color = Vec4(0.0f, 0.0f, 1.0f, 0.4f);

            Vec3 halfExtents = collider->halfExtents;
            Vec3 position = transform->position;
            Quat rotation = transform->rotation;

            // Define the eight vertices of the box
            Vec3 vertices[8];
            vertices[0] = position + glm::rotate(rotation, Vec3(-halfExtents.x, -halfExtents.y, -halfExtents.z));
            vertices[1] = position + glm::rotate(rotation, Vec3(-halfExtents.x, -halfExtents.y, halfExtents.z));
            vertices[2] = position + glm::rotate(rotation, Vec3(-halfExtents.x, halfExtents.y, -halfExtents.z));
            vertices[3] = position + glm::rotate(rotation, Vec3(-halfExtents.x, halfExtents.y, halfExtents.z));
            vertices[4] = position + glm::rotate(rotation, Vec3(halfExtents.x, -halfExtents.y, -halfExtents.z));
            vertices[5] = position + glm::rotate(rotation, Vec3(halfExtents.x, -halfExtents.y, halfExtents.z));
            vertices[6] = position + glm::rotate(rotation, Vec3(halfExtents.x, halfExtents.y, -halfExtents.z));
            vertices[7] = position + glm::rotate(rotation, Vec3(halfExtents.x, halfExtents.y, halfExtents.z));

            // Define the indices for the edges of the box
            int edgeIndices[12][2] = {
                {0, 1}, {1, 3}, {3, 2}, {2, 0},
                {4, 5}, {5, 7}, {7, 6}, {6, 4},
                {0, 4}, {1, 5}, {2, 6}, {3, 7}
            };

            for (uint32_t i = 0; i < 12; i++) {
                uint32_t startIndex = edgeIndices[i][0];
                uint32_t endIndex = edgeIndices[i][1];
                Vec3 start = vertices[startIndex];
                Vec3 end = vertices[endIndex];
                renderer.SubmitLine(start, end, color);
            }

            });

        store.ForEach<Transform, RigidBody, SphereCollider>([&renderer](Entity entity, Transform* transform, RigidBody* body, SphereCollider* collider) {

            const Vec4 color = Vec4(0.0f, 1.0f, 0.0f, 0.4f);
            int numRings = 8; // You can adjust the number of rings as needed
            int linesPerRing = 8; // You can adjust the number of lines per ring as needed
            float radius = collider->radius;
            Vec3 center = transform->position;

            for (int i = 0; i < 12; i++)
            {
                float theta1 = glm::radians(i * 30.0f);
                float theta2 = glm::radians((i + 1) * 30.0f);

                glm::vec3 point1 = glm::vec3(radius * glm::cos(theta1), 0.0f, radius * glm::sin(theta1));
                glm::vec3 point2 = glm::vec3(radius * glm::cos(theta2), 0.0f, radius * glm::sin(theta2));

                // Rotate the points based on the transform's rotation
                point1 = glm::rotate(transform->rotation, point1);
                point2 = glm::rotate(transform->rotation, point2);

                // Offset the points by the center of the sphere
                point1 += center;
                point2 += center;

                renderer.SubmitLine(point1, point2, color);
            }

            // Add lines to connect the poles to the equator
            for (int i = 0; i < 12; i++)
            {
                float phi1 = glm::radians(i * 30.0f - 90.0f);
                float phi2 = glm::radians((i + 1) * 30.0f - 90.0f);

                glm::vec3 point1 = glm::vec3(0.0f, radius * glm::sin(phi1), radius * glm::cos(phi1));
                glm::vec3 point2 = glm::vec3(0.0f, radius * glm::sin(phi2), radius * glm::cos(phi2));

                // Rotate the points based on the transform's rotation
                point1 = glm::rotate(transform->rotation, point1);
                point2 = glm::rotate(transform->rotation, point2);

                // Offset the points by the center of the sphere
                point1 += center;
                point2 += center;

                renderer.SubmitLine(point1, point2, color);
            }

            });

        store.ForEach<Transform, RigidBody, CapsuleCollider>([&renderer](Entity entity, Transform* transform, RigidBody* body, CapsuleCollider* collider) {

            const Vec4 color = Vec4(1.0f, 0.0f, 0.0f, 0.4f);
            int numSegments = 32; // Number of segments to approximate the capsule
            float capsuleRadius = collider->radius;
            float capsuleHalfHeight = collider->halfHeight;
            glm::vec3 center = transform->position;
            glm::quat rotation = transform->rotation;


            // Draw the top ring
            for (int i = 0; i < 12; i++)
            {
                float theta1 = glm::radians(i * 30.0f);
                float theta2 = glm::radians((i + 1) * 30.0f);

                glm::vec3 point1 = glm::vec3(capsuleRadius * glm::cos(theta1), 0.0f, capsuleRadius * glm::sin(theta1)) + glm::vec3(0.0f, capsuleHalfHeight, 0.0f);
                glm::vec3 point2 = glm::vec3(capsuleRadius * glm::cos(theta2), 0.0f, capsuleRadius * glm::sin(theta2)) + glm::vec3(0.0f, capsuleHalfHeight, 0.0f);
                glm::vec3 point3 = glm::vec3(capsuleRadius * glm::cos(theta1), 0.0f, capsuleRadius * glm::sin(theta1)) + glm::vec3(0.0f, -capsuleHalfHeight, 0.0f);
                glm::vec3 point4 = glm::vec3(capsuleRadius * glm::cos(theta2), 0.0f, capsuleRadius * glm::sin(theta2)) + glm::vec3(0.0f, -capsuleHalfHeight, 0.0f);

                // Rotate the points based on the transform's rotation
                point1 = glm::rotate(transform->rotation, point1) + center;
                point2 = glm::rotate(transform->rotation, point2) + center;
                point3 = glm::rotate(transform->rotation, point3) + center;
                point4 = glm::rotate(transform->rotation, point4) + center;

                renderer.SubmitLine(point1, point2, color);
                renderer.SubmitLine(point3, point4, color);
            }

            // Add lines to connect the poles to the equator

            for (int i = 0; i < 6; i++)
            {
                float phi1 = glm::radians(i * 30.0f);
                float phi2 = glm::radians((i + 1) * 30.0f);
                float phi3 = glm::radians(i * 30.0f - 180.0f);
                float phi4 = glm::radians((i + 1) * 30.0f - 180.0f);

                glm::vec3 point1 = glm::vec3(0.0f, capsuleRadius * glm::sin(phi1), capsuleRadius * glm::cos(phi1)) + glm::vec3(0.0f, capsuleHalfHeight, 0.0f);
                glm::vec3 point2 = glm::vec3(0.0f, capsuleRadius * glm::sin(phi2), capsuleRadius * glm::cos(phi2)) + glm::vec3(0.0f, capsuleHalfHeight, 0.0f);
                glm::vec3 point3 = glm::vec3(0.0f, capsuleRadius * glm::sin(phi3), capsuleRadius * glm::cos(phi3)) + glm::vec3(0.0f, -capsuleHalfHeight, 0.0f);
                glm::vec3 point4 = glm::vec3(0.0f, capsuleRadius * glm::sin(phi4), capsuleRadius * glm::cos(phi4)) + glm::vec3(0.0f, -capsuleHalfHeight, 0.0f);


                // Rotate the points based on the transform's rotation
                point1 = glm::rotate(transform->rotation, point1) + center;
                point2 = glm::rotate(transform->rotation, point2) + center;
                point3 = glm::rotate(transform->rotation, point3) + center;
                point4 = glm::rotate(transform->rotation, point4) + center;

                renderer.SubmitLine(point1, point2, color);
                renderer.SubmitLine(point3, point4, color);
            }

            // Draw the 4 lines connecting the top and bottom rings
            for (int i = 0; i < 4; i++)
            {
                float theta = glm::radians(i * 90.0f);

                glm::vec3 point1 = glm::vec3(capsuleRadius * glm::cos(theta), 0.0f, capsuleRadius * glm::sin(theta)) + glm::vec3(0.0f, capsuleHalfHeight, 0.0f);
                glm::vec3 point2 = glm::vec3(capsuleRadius * glm::cos(theta), 0.0f, capsuleRadius * glm::sin(theta)) + glm::vec3(0.0f, -capsuleHalfHeight, 0.0f);

                // Rotate the points based on the transform's rotation
                point1 = glm::rotate(transform->rotation, point1);
                point2 = glm::rotate(transform->rotation, point2);

                // Offset the points by the center of the sphere
                point1 += center;
                point2 += center;

                renderer.SubmitLine(point1, point2, color);
            }

            });

    }

}