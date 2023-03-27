#pragma once

#include "Core/Core.h"

#include "Serialization/Serialization.h"

namespace Testbed {

    struct Player
    {
        std::string name;

        Player() = default;
        Player(const std::string& name) : name(name) {}
        ~Player() = default;

        template<typename Serializer>
        void Transfer(Serializer& serializer)
        {
            SL_TRANSFER_VAR(name);
        }
    };


}

#define GAME_COMPONENTS \
	Testbed::Player