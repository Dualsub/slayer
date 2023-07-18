#include "Rendering/Renderer/SkeletalModel.h"
#include "Resources/AssetTypes.h"

namespace Slayer {

	const Mat4& SkeletalModel::GetSocketTransform(const std::string& name)
	{
		if (sockets.find(name) != sockets.end())
		{
			return sockets[name].GetWorldTransform();
		}
	}

	void SkeletalModel::AddSockets(const Vector<Socket>& inSockets)
	{
		for (auto& socket : inSockets)
		{
			if (sockets.find(socket.name) == sockets.end())
			{
				sockets[socket.name] = socket;
			}
		}
	}

	Shared<SkeletalModel> SkeletalModel::Create(Vector<SkeletalVertex>& vertices, Vector<uint32_t>& indices, Dict<std::string, BoneInfo>& bones, const Mat4& globalInverseTransform)
	{
		Vector<Shared<Mesh>> meshes;

		auto vao = VertexArray::Create();
		auto vbo = VertexBuffer::Create((void*)vertices.data(), vertices.size() * sizeof(SkeletalVertex));
	
		vbo->SetLayout({
			{"position", 3},
			{"texCoord", 2},
			{"normal", 3},
			{"boneIDs", SL_MAX_BONE_WEIGHTS, AttribType::SL_ATTRIB_INT},
			{"weights", SL_MAX_BONE_WEIGHTS}
		});
	
		vao->AddVertexBuffer(vbo);
		auto ebo = IndexBuffer::Create(indices.data(), indices.size() * sizeof(uint32_t));
		vao->SetIndexBuffer(ebo);
	
		meshes.push_back(MakeShared<Mesh>(vbo, ebo, vao, indices.size()));

		return MakeShared<SkeletalModel>(meshes, bones, bones.size(), globalInverseTransform);
	}

	SkeletalModel::SkeletalModel(Vector<Shared<Mesh>> meshes, Dict<std::string, BoneInfo> boneDict, int boneCounter, const Mat4& globalInverseTransform)
		: meshes(meshes), bones(bones), boneCounter(boneCounter), globalInverseTransform(globalInverseTransform)
	{
		bones = Vector<BoneInfo>(boneDict.size());
		// Set all parents in the static array to -1
		std::memset(parents, -1, SL_MAX_BONES * sizeof(int32_t));
		
		for (auto& [name, bone] : boneDict)
		{
			bones[bone.id] = bone;
			bonesIds[name] = bone.id;
			parents[bone.id] = bone.parentID;
		}

		for (size_t i = 0; i < bones.size(); i++)
		{
			auto& bone = bones[i];
			inverseBindPoseMatrices[i] = glm::inverse(bone.offset);
		}
	}

	void SkeletalModel::Dispose()
	{
		sockets.clear();
		bones.clear();
		bonesIds.clear();
		bones.clear();
		for (auto& mesh : meshes)
			mesh->Dispose();
	}
}