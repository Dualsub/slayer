#include "Physics/ColliderRenderingSystem.h"

#include "Scene/World.h"
#include "Rendering/Renderer/Renderer.h"

namespace Slayer {

    void ColliderRenderingSystem::Render(Renderer& renderer, ComponentStore& store)
    {
        store.ForEach<Transform, BoxCollider>([&renderer](Entity entity, Transform* transform, BoxCollider* collider) {

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

        store.ForEach<Transform, SphereCollider>([&renderer](Entity entity, Transform* transform, SphereCollider* collider) {

            const Vec4 color = Vec4(0.0f, 1.0f, 0.0f, 0.4f);
            const uint32_t numSegments = 32; // Number of segments to approximate the capsule
            const float angleIncrement = glm::radians(360.0f / numSegments);
            float radius = collider->radius;
            Vec3 center = transform->position;

            for (int i = 0; i < numSegments; i++)
            {
                float theta1 = i * angleIncrement;
                float theta2 = (i + 1) * angleIncrement;

                glm::vec3 point1 = glm::vec3(radius * glm::cos(theta1), 0.0f, radius * glm::sin(theta1));
                glm::vec3 point2 = glm::vec3(radius * glm::cos(theta2), 0.0f, radius * glm::sin(theta2));

                // Rotate the points based on the transform's rotation
                point1 = glm::rotate(transform->rotation, point1) + center;
                point2 = glm::rotate(transform->rotation, point2) + center;

                renderer.SubmitLine(point1, point2, color);
            }

            for (int i = 0; i < numSegments / 2; i++)
            {
                float phi1 = i * angleIncrement;
                float phi2 = (i + 1) * angleIncrement;
                float phi3 = i * angleIncrement - glm::radians(180.0f);
                float phi4 = (i + 1) * angleIncrement - glm::radians(180.0f);

                glm::vec3 point1 = glm::vec3(0.0f, radius * glm::sin(phi1), radius * glm::cos(phi1));
                glm::vec3 point2 = glm::vec3(0.0f, radius * glm::sin(phi2), radius * glm::cos(phi2));
                glm::vec3 point3 = glm::vec3(0.0f, radius * glm::sin(phi3), radius * glm::cos(phi3));
                glm::vec3 point4 = glm::vec3(0.0f, radius * glm::sin(phi4), radius * glm::cos(phi4));

                glm::vec3 point5 = glm::vec3(radius * glm::cos(phi1), radius * glm::sin(phi1), 0.0f);
                glm::vec3 point6 = glm::vec3(radius * glm::cos(phi2), radius * glm::sin(phi2), 0.0f);
                glm::vec3 point7 = glm::vec3(radius * glm::cos(phi3), radius * glm::sin(phi3), 0.0f);
                glm::vec3 point8 = glm::vec3(radius * glm::cos(phi4), radius * glm::sin(phi4), 0.0f);

                // Rotate the points based on the transform's rotation
                point1 = glm::rotate(transform->rotation, point1) + center;
                point2 = glm::rotate(transform->rotation, point2) + center;
                point3 = glm::rotate(transform->rotation, point3) + center;
                point4 = glm::rotate(transform->rotation, point4) + center;
                point5 = glm::rotate(transform->rotation, point5) + center;
                point6 = glm::rotate(transform->rotation, point6) + center;
                point7 = glm::rotate(transform->rotation, point7) + center;
                point8 = glm::rotate(transform->rotation, point8) + center;

                renderer.SubmitLine(point1, point2, color);
                renderer.SubmitLine(point3, point4, color);
                renderer.SubmitLine(point5, point6, color);
                renderer.SubmitLine(point7, point8, color);
            }

            });

        store.ForEach<Transform, CapsuleCollider>([&renderer](Entity entity, Transform* transform, CapsuleCollider* collider) {

            const Vec4 color = Vec4(1.0f, 0.0f, 0.0f, 0.4f);
            const uint32_t numSegments = 32; // Number of segments to approximate the capsule
            const float angleIncrement = glm::radians(360.0f / numSegments);
            float capsuleRadius = collider->radius;
            float capsuleHalfHeight = collider->halfHeight;
            glm::vec3 center = transform->position;
            glm::quat rotation = transform->rotation;


            // Draw the top rings
            for (int i = 0; i < numSegments; i++)
            {
                float theta1 = i * angleIncrement;
                float theta2 = (i + 1) * angleIncrement;

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

            for (int i = 0; i < numSegments / 2; i++)
            {
                float phi1 = i * angleIncrement;
                float phi2 = (i + 1) * angleIncrement;
                float phi3 = i * angleIncrement - glm::radians(180.0f);
                float phi4 = (i + 1) * angleIncrement - glm::radians(180.0f);

                glm::vec3 point1 = glm::vec3(0.0f, capsuleRadius * glm::sin(phi1), capsuleRadius * glm::cos(phi1)) + glm::vec3(0.0f, capsuleHalfHeight, 0.0f);
                glm::vec3 point2 = glm::vec3(0.0f, capsuleRadius * glm::sin(phi2), capsuleRadius * glm::cos(phi2)) + glm::vec3(0.0f, capsuleHalfHeight, 0.0f);
                glm::vec3 point3 = glm::vec3(0.0f, capsuleRadius * glm::sin(phi3), capsuleRadius * glm::cos(phi3)) + glm::vec3(0.0f, -capsuleHalfHeight, 0.0f);
                glm::vec3 point4 = glm::vec3(0.0f, capsuleRadius * glm::sin(phi4), capsuleRadius * glm::cos(phi4)) + glm::vec3(0.0f, -capsuleHalfHeight, 0.0f);

                glm::vec3 point5 = glm::vec3(capsuleRadius * glm::cos(phi1), capsuleRadius * glm::sin(phi1), 0.0f) + glm::vec3(0.0f, capsuleHalfHeight, 0.0f);
                glm::vec3 point6 = glm::vec3(capsuleRadius * glm::cos(phi2), capsuleRadius * glm::sin(phi2), 0.0f) + glm::vec3(0.0f, capsuleHalfHeight, 0.0f);
                glm::vec3 point7 = glm::vec3(capsuleRadius * glm::cos(phi3), capsuleRadius * glm::sin(phi3), 0.0f) + glm::vec3(0.0f, -capsuleHalfHeight, 0.0f);
                glm::vec3 point8 = glm::vec3(capsuleRadius * glm::cos(phi4), capsuleRadius * glm::sin(phi4), 0.0f) + glm::vec3(0.0f, -capsuleHalfHeight, 0.0f);


                // Rotate the points based on the transform's rotation
                point1 = glm::rotate(transform->rotation, point1) + center;
                point2 = glm::rotate(transform->rotation, point2) + center;
                point3 = glm::rotate(transform->rotation, point3) + center;
                point4 = glm::rotate(transform->rotation, point4) + center;
                point5 = glm::rotate(transform->rotation, point5) + center;
                point6 = glm::rotate(transform->rotation, point6) + center;
                point7 = glm::rotate(transform->rotation, point7) + center;
                point8 = glm::rotate(transform->rotation, point8) + center;

                renderer.SubmitLine(point1, point2, color);
                renderer.SubmitLine(point3, point4, color);
                renderer.SubmitLine(point5, point6, color);
                renderer.SubmitLine(point7, point8, color);
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