#pragma once

#include "Slayer.h"

namespace Slayer {

    class Skeleton
    {
    public:
        Skeleton() = default;
        ~Skeleton() = default;

        void SetBoneTransforms(const std::vector<Mat4>& transforms)
        {
            m_boneTransforms = transforms;
        }

        const std::vector<Mat4>& GetBoneTransforms() const
        {
            return m_boneTransforms;
        }

    private:
        Vector<Mat4> m_boneTransforms;
    };

}