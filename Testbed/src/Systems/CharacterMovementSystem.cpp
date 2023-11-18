#include "Systems/CharacterMovementSystem.h"

#include "Core/Math.h"
#include "Input/Input.h"
#include "Scene/World.h"
#include "Scene/ComponentStore.h"
#include "Scene/Components.h"
#include "Resources/ResourceManager.h"

using namespace Slayer;

namespace Testbed {

    void CharacterMovementSystem::FixedUpdate(Timespan dt, Slayer::ComponentStore& store)
    {
        // const static Vec3 up = Vec3(0.0f, 1.0f, 0.0f);

        // auto& pw = World::GetPhysicsWorld();

        // store.ForEach<Transform, CharacterBody>([&pw](Entity entity, Transform* transform, CharacterBody* body) {

        //     // Set velocity
        //     auto& currentState = body->currentState;
        //     auto& previousState = body->lastState;

        //     Vec3 forward = glm::rotate(transform->rotation, Vec3(0.0f, 0.0f, -1.0f));
        //     Vec3 right = glm::cross(forward, up);
        //     CharacterGroundState state = pw.GetCharacterGroundState(body->id);

        //     Vec3 direction = Vec3(0.0f);
        //     if (Input::IsKeyPressed(SlayerKey::KEY_W))
        //         direction += forward;

        //     if (Input::IsKeyPressed(SlayerKey::KEY_S))
        //         direction -= forward;

        //     if (Input::IsKeyPressed(SlayerKey::KEY_A))
        //         direction -= right;

        //     if (Input::IsKeyPressed(SlayerKey::KEY_D))
        //         direction += right;

        //     if (direction != Vec3(0.0f))
        //         direction = glm::normalize(direction);

        //     float speed = (Input::IsKeyPressed(SlayerKey::KEY_LEFT_SHIFT) && glm::dot(direction, forward) > 0.5f) ? body->sprintSpeed : body->movementSpeed;

        //     Vec3 lastVelocity = pw.GetLinearVelocity(body->id);
        //     Vec3 targetVelocity = direction * speed;
        //     targetVelocity.y = lastVelocity.y;
        //     float controlFactor = state == CharacterGroundState::SL_CHARACTER_GROUND_STATE_GROUNDED ? body->movementControl : body->jumpControl;
        //     Vec3 velocity = glm::mix(lastVelocity, targetVelocity, controlFactor);

        //     if (state == CharacterGroundState::SL_CHARACTER_GROUND_STATE_GROUNDED && Input::IsKeyPressed(SlayerKey::KEY_SPACE))
        //     {
        //         velocity.y = body->jumpSpeed;
        //     }

        //     pw.SetLinearVelocity(body->id, velocity);
        //     });

    }

    void CharacterMovementSystem::Update(Timespan dt, Slayer::ComponentStore& store)
    {
        const static Vec3 up = Vec3(0.0f, 1.0f, 0.0f);

        auto& pw = World::GetPhysicsWorld();
        auto* mouseInput = store.GetSingleton<MouseInput>();
        // Set look rotation
        Vec2 delta = mouseInput->previousPosition - mouseInput->currentPosition;
        delta *= glm::radians(0.1f);

        store.ForEach<Transform, FPCamera>([&pw, mouseInput, dt, &delta](Entity entity, Transform* transform, FPCamera* camera) {
            // // Set pitch rotation
            Quat deltaPitch = glm::angleAxis(delta.y, glm::vec3(1.0f, 0.0f, 0.0f));

            // Apply pitch rotation to the current rotation
            Quat newRotation = deltaPitch * transform->rotation;

            // Clamp the pitch rotation
            float pitch = glm::pitch(newRotation);
            if (pitch > glm::radians(89.0f))
                newRotation = glm::rotate(newRotation, glm::radians(89.0f) - pitch, glm::vec3(1.0f, 0.0f, 0.0f));
            else if (pitch < glm::radians(-89.0f))
                newRotation = glm::rotate(newRotation, glm::radians(-89.0f) - pitch, glm::vec3(1.0f, 0.0f, 0.0f));

            transform->rotation = newRotation;
            });

        store.ForEach<Transform, CharacterBody>([&pw, mouseInput, dt, &delta](Entity entity, Transform* transform, CharacterBody* body) {

            // // Set yaw rotation

            auto& currentState = body->currentState;
            auto& previousState = body->lastState;

            if (glm::abs(delta.x) > glm::epsilon<float>())
            {
                Quat yaw = glm::angleAxis(delta.x, up);
                Quat newRoation = glm::normalize(yaw * transform->rotation);
                pw.SetCharacterRotation(body->id, newRoation);
                currentState.rotation = newRoation;
            }

            // Set velocity
            Vec3 forward = glm::rotate(transform->rotation, Vec3(0.0f, 0.0f, -1.0f));
            Vec3 right = glm::cross(forward, up);
            CharacterGroundState state = pw.GetCharacterGroundState(body->id);

            Vec3 direction = Vec3(0.0f);
            if (Input::IsKeyPressed(SlayerKey::KEY_W))
                direction += forward;

            if (Input::IsKeyPressed(SlayerKey::KEY_S))
                direction -= forward;

            if (Input::IsKeyPressed(SlayerKey::KEY_A))
                direction -= right;

            if (Input::IsKeyPressed(SlayerKey::KEY_D))
                direction += right;

            if (direction != Vec3(0.0f))
                direction = glm::normalize(direction);

            float speed = (Input::IsKeyPressed(SlayerKey::KEY_LEFT_SHIFT) && glm::dot(direction, forward) > 0.5f) ? body->sprintSpeed : body->movementSpeed;

            Vec3 lastVelocity = pw.GetLinearVelocity(body->id);
            Vec3 targetVelocity = direction * speed;
            targetVelocity.y = lastVelocity.y;
            float controlFactor = state == CharacterGroundState::SL_CHARACTER_GROUND_STATE_GROUNDED ? body->movementControl : body->jumpControl;
            Vec3 velocity = glm::mix(lastVelocity, targetVelocity, controlFactor);

            if (state == CharacterGroundState::SL_CHARACTER_GROUND_STATE_GROUNDED && Input::IsKeyPressed(SlayerKey::KEY_SPACE))
            {
                velocity.y = body->jumpSpeed;
            }

            pw.SetLinearVelocity(body->id, velocity);
            });


        mouseInput->previousPosition = mouseInput->currentPosition;

        auto* rm = ResourceManager::Get();

        store.ForEachAsync<Transform, AnimationPlayer>([&pw, &rm, &store](Entity entity, Transform* transform, AnimationPlayer* player) {
            Entity parentEntity = store.GetEntity(transform->parentId);
            if (!store.IsValid(parentEntity) || !store.HasComponent<CharacterBody>(parentEntity) || !store.HasComponent<Transform>(parentEntity))
                return;

            Transform* parentTransform = store.GetComponent<Transform>(parentEntity);
            CharacterBody* body = store.GetComponent<CharacterBody>(parentEntity);
            Vec3 velocity = pw.GetRigidBodyState(body->id).linearVelocity;

            if (player->animationClips.size() < 2)
                player->animationClips.resize(2);


            CharacterGroundState state = pw.GetCharacterGroundState(body->id);
            player->animationClips[0].animationID = state == CharacterGroundState::SL_CHARACTER_GROUND_STATE_GROUNDED ? rm->GetAssetID("A_Arms_Rifle_Idle01") : rm->GetAssetID("A_Arms_Rifle_Idle01");

            if (glm::length(velocity) < 0.1f || state == CharacterGroundState::SL_CHARACTER_GROUND_STATE_IN_AIR)
            {
                player->animationClips[0].weight = 1.0f;
                player->animationClips[1].weight = 0.0f;
                return;
            }

            Vec3 forward = glm::rotate(parentTransform->rotation, Vec3(0.0f, 0.0f, -1.0f));
            float forwardBlend = glm::dot(forward, velocity) / body->movementSpeed;

            AssetID runAnimtionId = forwardBlend < 0.0f ? rm->GetAssetID("A_Arms_Rifle_Idle01") : rm->GetAssetID("A_Arms_Rifle_Idle01");
            float blendFactor = glm::abs(forwardBlend);

            player->animationClips[0].weight = 1.0f - blendFactor;
            player->animationClips[1].animationID = runAnimtionId;
            player->animationClips[1].weight = blendFactor;

            });
    }
}