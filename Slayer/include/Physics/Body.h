#pragma once

namespace Slayer {

    using BodyID = uint32_t;

    enum BodyType : uint8_t
    {
        SL_BODY_TYPE_RIGID,
        SL_BODY_TYPE_CHARACTER
    };
}