#pragma once

#include "Core/Core.h"
#include "Mesh.h"

#define MAX_BONE_WEIGHTS 4
#define MAX_BONES 100

namespace Slayer {

	struct SkeletalVertex
	{
		Vec3 position;
		Vec2 texCoord;
		Vec3 normal;
		int boneIDs[MAX_BONE_WEIGHTS];
		float weights[MAX_BONE_WEIGHTS];
	};

	struct BoneInfo
	{
		int id;
		int parentID = 0;
		Mat4 offset;
		Mat4 transform;
	};

	class Socket
	{
	private:
		std::string name;
		std::string boneName;
		Mat4 worldTransform = Mat4(1.0f);
		Mat4 offset = Mat4(1.0f);
	public:
		const Mat4& GetOffset() const { return offset; }
		void SetOffset(const Mat4& inOffset) { offset = inOffset; }
		const Mat4& GetWorldTransform() const { return worldTransform; }
		void SetWorldTransform(const Mat4& inWorldTransform) { worldTransform = inWorldTransform; }
		const std::string& GetName() const { return name; }
		const std::string& GetBoneName() const { return boneName; }
		Socket() = default;
		Socket(const std::string& name, const std::string& boneName, const Mat4& offset)
			: name(name), boneName(boneName), offset(offset)
		{
		}

		template<typename Serializer>
		void Transfer(Serializer& serializer)
		{
			SL_TRANSFER_VAR(name);
			SL_TRANSFER_VAR(boneName);
		}
	};

	class SkeletalModel
	{
	private:
		Dict<std::string, Socket> sockets;
		Dict<std::string, BoneInfo> boneDict;
		Dict<std::string, int> boneNamesToID;
		Vector<BoneInfo> bones;
		int boneCounter = 0;
		Mat4 globalInverseTransform = Mat4(1.0f);
		Vector<Shared<Mesh>> meshes;
	public:
		AssetID assetID;
		const Vector<Shared<Mesh>>& GetMeshes() { return meshes; }
		bool HasSocket(const std::string& name) { return sockets.find(name) != sockets.end(); }
		Dict<std::string, Socket>& GetSockets() { return sockets; }
		const Dict<std::string, BoneInfo>& GetBones() { return boneDict; }
		const Mat4& GetGlobalInverseTransform() { return globalInverseTransform; }
		const Mat4& GetSocketTransform(const std::string& name);
		void SetSocketOffset(const std::string& name, const Mat4& offset);
		void AddSocket(const std::string& name, const std::string& boneName, const Mat4& offset);
		void AddSockets(const Vector<Socket>& inSockets);
		SkeletalModel(Vector<Shared<Mesh>> meshes, Dict<std::string, BoneInfo> boneDict, int boneCounter, const Mat4& globalInverseTransform);
		static Shared<SkeletalModel> Create(const std::string& filePath);
		static Dict<std::string, BoneInfo> LoadBones(const std::string& filePath);
		void Dispose();
	};

}