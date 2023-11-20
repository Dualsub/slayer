#pragma once

#include "Slayer.h"
#include "Scene/Entity.h"
#include "Scene/SingletonComponent.h"
#include "Serialization/Serialization.h"

#include <cstdint>

#define GAME_COMPONENTS Testbed::Player, Testbed::FPCamera, Testbed::BulletSource, Testbed::Bullet
#define GAME_SINGLETONS Testbed::MouseInput

namespace Testbed {

    struct Player
    {
        std::string name;

        float speed = 1.0f;
        float time = 0.0f;

        Player() = default;
        Player(const std::string& name) : name(name) {}
        ~Player() = default;

        template<typename Serializer>
        void Transfer(Serializer& serializer)
        {
            SL_TRANSFER_VAR(name);
        }
    };

    struct MouseInput : public Slayer::SingletonComponent
    {
        Slayer::Vec2 currentPosition = { 0.0f, 0.0f };
        Slayer::Vec2 previousPosition = { 0.0f, 0.0f };
        bool firstMouse = true;

        MouseInput() = default;
        ~MouseInput() = default;

        template<typename Serializer>
        void Transfer(Serializer& serializer)
        {

        }
    };

    struct FPCamera
    {
        template<typename Serializer>
        void Transfer(Serializer& serializer)
        {

        }
    };

    struct BulletSource
    {

        Slayer::Vec3 offset = Slayer::Vec3(0.0f);
        float fireRate = 450.0f;
        float bulletSpeed = 10000.0f;
        float bulletTimeToLive = 1.0f;
        float spread = 0.1f;

        Slayer::Timespan timeSinceLastShot = 0.0f;
        bool isFiring = false;

        template<typename Serializer>
        void Transfer(Serializer& serializer)
        {
            SL_TRANSFER_VAR(offset);
            SL_TRANSFER_VAR(fireRate);
            SL_TRANSFER_VAR(bulletSpeed);
            SL_TRANSFER_VAR(bulletTimeToLive);
            SL_TRANSFER_VAR(spread);
        }
    };

    struct Bullet
    {
        Slayer::Entity owner = SL_INVALID_ENTITY;
        float timeToLive = 1.0f;
        float lifeTime = 0.0f;

        Bullet(Slayer::Entity owner, float timeToLive) : owner(owner), timeToLive(timeToLive) {}
        Bullet() = default;
        ~Bullet() = default;

        template<typename Serializer>
        void Transfer(Serializer& serializer)
        {
        }
    };

}
