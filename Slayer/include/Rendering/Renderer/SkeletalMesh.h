#pragma once

#include "Core.h"
//#include "Mesh.h"
//#include "assimp/scene.h"
//
//#define MAX_BONE_WEIGHTS 4
//#define MAX_BONES 100
//
//namespace Slayer {
//
//	namespace AssimpUtils
//	{
//		static Mat4 Convert(const aiMatrix4x4& from)
//		{
//			Mat4 to;
//			to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
//			to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
//			to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
//			to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
//			return to;
//		}
//		
//		static Quat Convert(const aiQuaternion& quat)
//		{
//			return Quat(quat.w, quat.x, quat.y, quat.z);
//		}
//
//		static Vec3 Convert(const aiVector3D& vector)
//		{
//			return Vec3(vector.x, vector.y, vector.z);
//		}
//
//		static Vec2 Convert(const aiVector2D& vector)
//		{
//			return Vec2(vector.x, vector.y);
//		}
//	}
//
//	struct SkeletalVertex
//	{
//		Vec3 position;
//		Vec2 texCoord;
//		Vec3 normal;
//		int boneIDs[MAX_BONE_WEIGHTS];
//		float weights[MAX_BONE_WEIGHTS];
//	};
//
//	struct BoneInfo
//	{
//		int id;
//		int parentID;
//		Mat4 offset;
//		Mat4 transform;
//	};
//
//	class Socket
//	{
//	private:
//		std::string name;
//		std::string boneName;
//		Mat4 worldTransform;
//		Mat4 offset;
//	public:
//		const Mat4& GetOffset() const { return offset; }
//		void SetOffset(const Mat4& inOffset) { offset = inOffset; }
//		const Mat4& GetWorldTransform() const { return worldTransform; }
//		void SetWorldTransform(const Mat4& inWorldTransform) { worldTransform = inWorldTransform; }
//		const std::string& GetName() const { return name; }
//		const std::string& GetBoneName() const { return boneName; }
//		Socket() = default;
//		Socket(const std::string& name, const std::string& boneName, const Mat4& offset)
//			: name(name), boneName(boneName), offset(offset)
//		{
//		}
//	};
//
//	class SkeletalMesh : public Mesh
//	{
//	private:
//		Dict<std::string, Socket> sockets;
//		Dict<std::string, BoneInfo> boneDict;
//		Dict<std::string, int> boneNamesToID;
//		Vector<BoneInfo> bones;
//		int boneCounter = 0;
//		Mat4 globalInverseTransform = Mat4(1.0f);
//	public:
//		Dict<std::string, Socket>& GetSockets() { return sockets; }
//		const Dict<std::string, BoneInfo>& GetBones() { return boneDict; }
//		const Mat4& GetGlobalInverseTransform() { return globalInverseTransform; }
//		const Mat4& GetSocketTransform(const std::string& name);
//		void SetSocketOffset(const std::string& name, const Mat4& offset);
//		void AddSocket(const std::string& name, const std::string& boneName, const Mat4& offset);
//		void AddSockets(const Vector<Socket>& inSockets);
//		static Ptr<SkeletalMesh> Create(const std::string& filePath);
//		SkeletalMesh(Ptr<VertexBuffer> vbo, Ptr<IndexBuffer> ebo, Ptr<VertexArray> vao, int indexCount, Dict<std::string, BoneInfo> boneDict, int boneCounter, const Mat4& globalInverseTransform);
//	};
//
//}
