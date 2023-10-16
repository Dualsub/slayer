#pragma once

#include <cstdint>
#include "Slayer.h"
#include "Scene/SingletonComponent.h"
#include "Serialization/Serialization.h"

#define GAME_COMPONENTS Testbed::Player
#define GAME_SINGLETONS Testbed::TestSingleton

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

    struct TestSingleton : public Slayer::SingletonComponent
    {
        float value = 0.0f;

        TestSingleton() = default;
        ~TestSingleton() = default;

        template<typename Serializer>
        void Transfer(Serializer& serializer)
        {
            SL_TRANSFER_VAR(value);
        }
    };

}
