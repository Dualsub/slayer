#pragma once

#include "Slayer.h"
#include "Scene/SingletonComponent.h"
#include "Serialization/Serialization.h"

#include <cstdint>

#define GAME_COMPONENTS Testbed::Player, Testbed::FPCamera
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

}
