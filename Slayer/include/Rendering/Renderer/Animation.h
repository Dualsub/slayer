#pragma once
#include "Core.h"
#include "SkeletalModel.h"
#include "Assets/Assets.h"

namespace Slayer {
	
	struct KeyPosition
	{
		float timestamp;
		Vec3 position;
        template<typename Serializer>
        void Transfer(Serializer& serializer)
        {
            TRANSFER(position);
            TRANSFER(timestamp);
        }
	};
	
	struct KeyRotation
	{
		float timestamp;
		Quat rotation;
        template<typename Serializer>
        void Transfer(Serializer& serializer)
        {
            TRANSFER(rotation);
            TRANSFER(timestamp);
        }
	};

	struct KeyScale
	{
		float timestamp;
		Vec3 scale;
        template<typename Serializer>
        void Transfer(Serializer& serializer)
        {
            TRANSFER(scale);
            TRANSFER(timestamp);
        }
	};

	class Bone
	{
		Vector<KeyPosition> positions;
		Vector<KeyRotation> rotations;
		Vector<KeyScale> scales;
		int numPositions;
		int positionIndex;
		float nextPositionTimestamp;
        int numScalings;
        int numRotations;
        int rotationIndex;
		float nextRotationTimestamp;

		Mat4 localTransform;
		std::string name;
		int id;
	public:
        Bone() = default;

        Bone(int id, const std::string& name, const Mat4& localTransform, const Vector<KeyPosition>& positions, const Vector<KeyRotation>& rotations, const Vector<KeyScale>& scales)
            : id(id), name(name), localTransform(1.0f), positions(positions), rotations(rotations), scales(scales)
        {
            numPositions = (int)positions.size();
            positionIndex = 0;
            numRotations = (int)rotations.size();
            rotationIndex = 0;
            numScalings = (int)scales.size();
        }

        Mat4 GetLocalTransform(float animationTime)
        {
            Mat4 translation = InterpolatePosition(animationTime);
            Mat4 rotation = InterpolateRotation(animationTime);
            Mat4 LT = translation * rotation;
            return LT;
        }

        void Update(float animationTime)
        {
            SL_EVENT();
            Mat4 translation = InterpolatePosition(animationTime);
            Mat4 rotation = InterpolateRotation(animationTime);
            //Mat4 scale = InterpolateScaling(animationTime);
            //localTransform = translation * rotation * scale;
            localTransform = translation * rotation;
        }

		const Mat4& GetLocalTransform() const { return localTransform; }
		const std::string& GetBoneName() const { return name; }
		int GetBoneID() const { return id; }
	private:
        int GetPositionIndex(float animationTime)
        {
            SL_EVENT();
            return GetIndex<KeyPosition>(positions, numPositions, animationTime);
        }

        int GetRotationIndex(float animationTime)
        {
            SL_EVENT();
            return GetIndex<KeyRotation>(rotations, numRotations, animationTime);
        }

        int GetScaleIndex(float animationTime)
        {
            SL_EVENT();
            for (int index = 0; index < numScalings - 1; ++index)
            {
                if (animationTime < scales[index + 1].timestamp)
                    return index;
            }
            assert(0);
            return 0;
        }

        template<typename KeyType>
        int GetIndex(const Vector<KeyType>& keys, int length, float animationTime)
        {
            SL_EVENT();
            for (int index = 0; index < length - 1; ++index)
            {
                if (animationTime < keys[index + 1].timestamp)
                    return index;
            }

            return keys.size() - 1;
        }

        template<typename KeyType>
        int GetIndex(const Vector<KeyType>& keys, int& currentIndex, int length, float animationTime)
        {
            SL_EVENT();

            int oldIndex = currentIndex;
            int index = currentIndex;
            while (animationTime > keys[index+1].timestamp)
            {
                index = (index + 1) % (length-1);
                if (oldIndex == index)
                    assert(0 && "We have looped.");
            }
            currentIndex = index;
            std::cout << index << std::endl;
            return index;
        }


        float GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime)
        {
            SL_EVENT();
            float scaleFactor = 1.0f;
            float midWayLength = animationTime - lastTimeStamp;
            float framesDiff = nextTimeStamp - lastTimeStamp;
            scaleFactor = midWayLength / framesDiff;
            return scaleFactor;
        }

        Mat4 InterpolatePosition(float animationTime)
        {
            SL_EVENT();
            if (0 == numPositions)
                return Mat4(1.0f);
            
            if (1 == numPositions)
                return glm::translate(Mat4(1.0f), positions[0].position);

            int p0Index = GetPositionIndex(animationTime);
            int p1Index = (p0Index + 1) % positions.size();
            float scaleFactor = GetScaleFactor(positions[p0Index].timestamp, positions[p1Index].timestamp, animationTime);
            Vec3 finalPosition = glm::mix(positions[p0Index].position, positions[p1Index].position, scaleFactor);
            return glm::translate(Mat4(1.0f), finalPosition);
        }

        Mat4 InterpolateRotation(float animationTime)
        {
            SL_EVENT();
            if (0 == numRotations)
                return Mat4(1.0f);
            
            if (1 == numRotations)
            {
                auto rotation = glm::normalize(rotations[0].rotation);
                return Mat4(rotation);
            }

            int p0Index = GetRotationIndex(animationTime);
            int p1Index = (p0Index + 1) % rotations.size();
            float scaleFactor = GetScaleFactor(rotations[p0Index].timestamp, rotations[p1Index].timestamp, animationTime);
            Quat finalRotation = glm::slerp(rotations[p0Index].rotation, rotations[p1Index].rotation, scaleFactor);
            finalRotation = glm::normalize(finalRotation);
            return glm::toMat4(finalRotation);
        }

        Mat4 InterpolateScaling(float animationTime)
        {
            SL_EVENT();
            if (1 == numScalings)
                return glm::scale(Mat4(1.0f), scales[0].scale);

            int p0Index = GetScaleIndex(animationTime);
            int p1Index = (p0Index + 1) % scales.size();
            float scaleFactor = GetScaleFactor(scales[p0Index].timestamp,
                scales[p1Index].timestamp, animationTime);
            Vec3 finalScale = glm::mix(scales[p0Index].scale,
                scales[p1Index].scale, scaleFactor);
            return glm::scale(Mat4(1.0f), finalScale);
        }
    public:
        template<typename Serializer>
        void Transfer(Serializer& serializer)
        {
            TRANSFER(id);
            TRANSFER(name);
            TRANSFER(positions);
            TRANSFER(rotations);
            TRANSFER(scales);
        }
	};

	class Animation
	{
    private:
        float duration;
        Vector<Bone> bones;
        Dict<std::string, BoneInfo> bonesDict;
	public:
        AssetID assetID;
        Vector<Bone>& GetBones() { return bones; }
        float GetDuration() { return duration; }
        Animation(float duration, const Vector<Bone>& bones, const Dict<std::string, BoneInfo>& bonesDict);
		static Ptr<Animation> Create(const std::string& filePath, Ptr<SkeletalModel> skeletalModel);
        static Ptr<Animation> Create(const std::string& filePath, const Dict<std::string, BoneInfo>& bones);

        void Dispose();
	};

}
