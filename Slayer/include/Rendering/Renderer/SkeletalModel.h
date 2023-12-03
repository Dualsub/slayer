#pragma once

#include "Core/Core.h"
#include "Mesh.h"
#include "Rendering/Animation/Socket.h"

#define SL_MAX_BONE_WEIGHTS 4
#define SL_MAX_BONES 96

namespace Slayer {

	struct SkeletalVertex
	{
		Vec3 position;
		Vec2 texCoord;
		Vec3 normal;
		int32_t boneIDs[SL_MAX_BONE_WEIGHTS];
		float weights[SL_MAX_BONE_WEIGHTS];
	};

	struct BoneInfo
	{
		std::string name = "";
		int32_t id = 0;
		int32_t parentID = -1;
		Mat4 offset;
		Mat4 transform;
	};

	class SkeletalModel
	{
	private:
		Dict<std::string, Socket> sockets;
		Dict<std::string, size_t> bonesIds;
		Vector<BoneInfo> bones;
		int32_t parents[SL_MAX_BONES];
		Mat4 inverseBindPoseMatrices[SL_MAX_BONES];
		int boneCounter = 0;
		Mat4 globalInverseTransform = Mat4(1.0f);
		Vector<Shared<Mesh>> meshes;
	public:
		AssetID assetID;
		const Vector<Shared<Mesh>>& GetMeshes() { return meshes; }
		bool HasSocket(const std::string& name) { return sockets.find(name) != sockets.end(); }
		Dict<std::string, Socket>& GetSockets() { return sockets; }
		const Vector<BoneInfo>& GetBones() { return bones; }
		int32_t GetBoneID(const std::string& name)
		{
			return bonesIds.find(name) != bonesIds.end() ? bonesIds[name] : -1;
		}
		const BoneInfo& GetBone(const std::string& name)
		{
			SL_ASSERT(bonesIds.find(name) != bonesIds.end() && "Cannot find bone.");
			return bones[bonesIds[name]];
		}
		const Mat4& GetGlobalInverseTransform() { return globalInverseTransform; }
		Mat4* GetInverseBindPoseMatrices() { return inverseBindPoseMatrices; }
		int32_t* GetParents() { return parents; }
		const Mat4& GetSocketTransform(const std::string& name);
		void SetSocketOffset(const std::string& name, const Mat4& offset);
		void AddSocket(const std::string& name, const std::string& boneName, const Mat4& offset);
		void AddSockets(const Vector<Socket>& inSockets);
		SkeletalModel(Vector<Shared<Mesh>> meshes, Dict<std::string, BoneInfo> boneDict, int boneCounter, const Mat4& globalInverseTransform);
		static Shared<SkeletalModel> Create(Vector<SkeletalVertex>& vertices, Vector<uint32_t>& indices, Dict<std::string, BoneInfo>& bones, const Mat4& globalInverseTransform);
		void Dispose();
	};

}