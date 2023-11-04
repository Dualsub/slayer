#pragma once

#include "Core/Math.h"
#include "Physics/Helpers.h"

#include "Jolt/Jolt.h"
#include "Jolt/Physics/Collision/Shape/BoxShape.h"
#include "Jolt/Physics/Collision/Shape/SphereShape.h"
#include "Jolt/Physics/Collision/Shape/CapsuleShape.h"

namespace Slayer {

    class CollisionShape
    {
    public:
        CollisionShape() = default;
        virtual ~CollisionShape() = default;
        virtual JPH::ShapeSettings::ShapeResult GetShapeSettings() const = 0;
    };

    class SphereShape : public CollisionShape
    {
    private:
        float m_radius;
    public:
        SphereShape(float radius) : m_radius(radius) {}
        ~SphereShape() = default;

        JPH::ShapeSettings::ShapeResult GetShapeSettings() const override
        {
            JPH::SphereShapeSettings settings;
            settings.mRadius = JoltHelpers::ToJolt(m_radius);
            return settings.Create();
        }
    };

    class BoxShape : public CollisionShape
    {
    private:
        Vec3 m_halfExtents;
    public:
        BoxShape(Vec3 halfExtents) : m_halfExtents(halfExtents) {}
        ~BoxShape() = default;

        JPH::ShapeSettings::ShapeResult GetShapeSettings() const override
        {
            JPH::BoxShapeSettings settings;
            settings.mHalfExtent = JoltHelpers::Convert(m_halfExtents);
            return settings.Create();
        }
    };

    class CapsuleShape : public CollisionShape
    {
    private:
        float m_radius;
        float m_halfHeight;
    public:
        CapsuleShape(float radius, float halfHeight) : m_radius(radius), m_halfHeight(halfHeight) {}
        ~CapsuleShape() = default;

        JPH::ShapeSettings::ShapeResult GetShapeSettings() const override
        {
            JPH::CapsuleShapeSettings settings;
            settings.mRadius = JoltHelpers::ToJolt(m_radius);
            settings.mHalfHeightOfCylinder = JoltHelpers::ToJolt(m_halfHeight);
            return settings.Create();
        }
    };
}