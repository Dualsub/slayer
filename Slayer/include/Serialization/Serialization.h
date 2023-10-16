#pragma once

#include "Core/Core.h"

#define SL_TRANSFER_VAR(variable) serializer.Transfer(variable, #variable)
#define SL_TRANSFER_ASSET(variable, type) serializer.TransferAsset(variable, type, #variable)
#define SL_TRANSFER_VEC(variable) serializer.TransferVector(variable, #variable)
#define SL_TRANSFER_VEC_FIXED(variable, maxCount) serializer.TransferVector(variable, #variable, maxCount)
#define SL_TRANSFER_DICT(variable) serializer.TransferDict(variable, #variable)

namespace Slayer {

    enum SerializationFlags : uint8_t
    {
        None = 0,
        Read = 1 << 0,
        Write = 1 << 1,
        ReadWrite = Read | Write
    };

    template<uint8_t Flags>
    class Serializer
    {
    public:
        constexpr uint8_t GetFlags() { return Flags; }
    };

}