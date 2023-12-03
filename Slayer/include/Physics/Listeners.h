#pragma once

#include "Physics/Body.h"

#include "Jolt/Jolt.h"
#include "Jolt/Physics/Body/BodyActivationListener.h"

namespace Slayer {

    struct Contact
    {
        BodyID body;
        Vec3 position;
        Vec3 normal;
        float penetration;
    };

    class ContactListener : public JPH::ContactListener
    {
    private:
        // Bodies we listen to contact events for:
        Set<BodyID> m_listeningBodies;
        Map<BodyID, Vector<Contact>> m_contacts;
    public:
        // See: ContactListener
        virtual JPH::ValidateResult OnContactValidate(const JPH::Body& inBody1, const JPH::Body& inBody2, JPH::RVec3Arg inBaseOffset, const JPH::CollideShapeResult& inCollisionResult) override
        {
            // Allows you to ignore a contact before it is created (using layers to not make objects collide is cheaper!)
            return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
        }

        virtual void OnContactAdded(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) override
        {
            BodyID bodyId1 = static_cast<uint32_t>(inBody1.GetUserData());
            BodyID bodyId2 = static_cast<uint32_t>(inBody2.GetUserData());
            if (m_listeningBodies.find(bodyId1) == m_listeningBodies.end() || m_listeningBodies.find(bodyId2) == m_listeningBodies.end())
            {
                return;
            }

            auto position = JoltHelpers::ConvertWithUnits(inManifold.mBaseOffset);
            auto normal = JoltHelpers::Convert(inManifold.mWorldSpaceNormal);
            auto penetration = JoltHelpers::FromJolt(inManifold.mPenetrationDepth);
            m_contacts[bodyId1].push_back({ bodyId2, position, normal, penetration });
            m_contacts[bodyId2].push_back({ bodyId1, position, normal, penetration });
        }

        virtual void OnContactPersisted(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) override
        {
        }

        virtual void OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair) override
        {
        }

        const void Register(BodyID bodyId)
        {
            m_listeningBodies.insert(bodyId);
        }

        const void Unregister(BodyID bodyId)
        {
            m_listeningBodies.erase(bodyId);
        }

        const Vector<Contact>& GetContacts(BodyID bodyId) const
        {
            auto it = m_contacts.find(bodyId);
            if (it != m_contacts.end())
            {
                return it->second;
            }
            else
            {
                return Vector<Contact>();
            }
        }

        void ClearContacts()
        {
            m_contacts.clear();
        }
    };

    // An example activation listener
    class BodyActivationListener : public JPH::BodyActivationListener
    {
    public:
        virtual void OnBodyActivated(const JPH::BodyID& inBodyID, JPH::uint64 inBodyUserData) override
        {
        }

        virtual void OnBodyDeactivated(const JPH::BodyID& inBodyID, JPH::uint64 inBodyUserData) override
        {
        }
    };

}