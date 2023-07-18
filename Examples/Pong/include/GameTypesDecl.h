#pragma once

#include <cstdint>
#include "Core/Core.h"
#include "Core/Math.h"
#include "Serialization/Serialization.h"

#define GAME_COMPONENTS Pong::Paddle, Pong::Ball

namespace Pong {

    struct Paddle
    {
        float maxSpeed = 10.0f;
        float velocity = 0.0f;
        uint32_t score = 0;
        char side = 0;

        Paddle() = default;
        Paddle(float maxSpeed, uint8_t side) : maxSpeed(maxSpeed), side(side) {}
        ~Paddle() = default;

        template<typename Serializer>
        void Transfer(Serializer& serializer)
        {
            SL_TRANSFER_VAR(maxSpeed);
            SL_TRANSFER_VAR(side);
        }
    };

    struct Ball
    {
        float maxSpeed = 10.0f;
        Slayer::Vec2 velocity = Slayer::Vec2(0.0f, 0.0f);

        Ball() = default;
        Ball(float maxSpeed) : maxSpeed(maxSpeed) {}
        ~Ball() = default;

        template<typename Serializer>
        void Transfer(Serializer& serializer)
        {
            SL_TRANSFER_VAR(maxSpeed);
        }
    };

}
