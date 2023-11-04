#pragma once

#include "Core/Math.h"
#include "Jolt/Jolt.h"

namespace Slayer::JoltHelpers {

    const float spaceScale = 100.0f;
    const float spaceScaleInv = 1 / spaceScale;

    inline Vec3 Convert(const JPH::Vec3& inVec)
    {
        return Vec3(inVec.GetX(), inVec.GetY(), inVec.GetZ()) * spaceScale;
    }

    inline JPH::Vec3 Convert(const Vec3& inVec)
    {
        return JPH::Vec3(inVec.x, inVec.y, inVec.z) * spaceScaleInv;
    }

    inline Quat Convert(const JPH::Quat& inQuat)
    {
        return Quat(inQuat.GetW(), inQuat.GetX(), inQuat.GetY(), inQuat.GetZ());
    }

    inline JPH::Quat Convert(const Quat& inQuat)
    {
        return JPH::Quat(inQuat.x, inQuat.y, inQuat.z, inQuat.w);
    }

    inline float ToJolt(float inValue)
    {
        return inValue * spaceScaleInv;
    }

    inline float FromJolt(float inValue)
    {
        return inValue * spaceScale;
    }

}