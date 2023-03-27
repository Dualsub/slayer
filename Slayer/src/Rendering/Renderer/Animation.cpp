#include "Animation.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

namespace Slayer {
    Animation::Animation(float duration, const Vector<Bone>& bones, const Dict<std::string, BoneInfo>& boneDict)
        : duration(duration), bones(bones), bonesDict(bonesDict)
    {
    }

    Ptr<Animation> Animation::Create(const std::string& filePath, Ptr<SkeletalModel> skeletalModel)
    {
        return Animation::Create(filePath, skeletalModel->GetBones());
    }

    Ptr<Animation> Animation::Create(const std::string& filePath, const Dict<std::string, BoneInfo>& bonesDict)
    {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(filePath, aiProcess_Triangulate);
        assert(scene && scene->mRootNode);

        std::cout << "Loading Animation: " << scene->GetShortFilename(filePath.c_str()) << std::endl;
        
        auto animation = scene->mAnimations[0];

        float duration = (float)animation->mDuration;
        float ticksPerSecond = (float)animation->mTicksPerSecond;
        Vector<Bone> unorderedBones;

        for (uint32_t i = 0; i < animation->mNumChannels; i++)
        {
            int id = -1;
            Mat4 localTransform = Mat4(1.0f);
            Vector<KeyPosition> positions;
            Vector<KeyRotation> rotations;
            Vector<KeyScale> scales;

            auto channel = animation->mChannels[i];
            std::string boneName = channel->mNodeName.C_Str();
            if (boneName.empty())
                continue;
            
            if (bonesDict.find(boneName) != bonesDict.end())
            {
                id = bonesDict.at(boneName).id;
                localTransform = bonesDict.at(boneName).offset;
                for (uint32_t j = 0; j < channel->mNumPositionKeys; j++)
                {
                    auto posKey = channel->mPositionKeys[j];
                    KeyPosition key;
                    key.position = AssimpUtils::Convert(posKey.mValue);
                    key.timestamp = (float)posKey.mTime;
                    positions.push_back(key);
                }

                for (uint32_t j = 0; j < channel->mNumRotationKeys; j++)
                {
                    auto rotKey = channel->mRotationKeys[j];
                    KeyRotation key;
                    key.rotation = AssimpUtils::Convert(rotKey.mValue);
                    key.timestamp = (float)rotKey.mTime;
                    rotations.push_back(key);
                }

                for (uint32_t j = 0; j < channel->mNumScalingKeys; j++)
                {
                    auto scaleKey = channel->mScalingKeys[j];
                    KeyScale key;
                    key.scale = AssimpUtils::Convert(scaleKey.mValue);
                    key.timestamp = (float)scaleKey.mTime;
                    scales.push_back(key);
                }

                 unorderedBones.push_back({ id, boneName, localTransform, positions, rotations, scales });
            }
            else
            {
                std::cout << "Couldn't find bone: " << boneName << std::endl;
            }
        }

        Vector<Bone> bones(unorderedBones.size());
        for (auto& bone : unorderedBones)
        {
            bones[bone.GetBoneID()] = bone;
        }

        return std::make_shared<Animation>(duration, bones, bonesDict);
    }

    void Animation::Dispose()
    {
        bonesDict.clear();
        bones.clear();
    }

}