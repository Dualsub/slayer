#include "SkeletalMesh.h"
//#include "assimp/Importer.hpp"
//#include "assimp/postprocess.h"
//
//namespace Slayer {
//
//	void ReadParents(aiNode* node, Dict<std::string, BoneInfo>& boneDict)
//	{
//		std::string boneName = node->mName.C_Str();
//		if (node->mParent)
//		{
//			std::string parentName = node->mParent->mName.C_Str();
//			boneDict[boneName].parentID = boneDict[parentName].id;
//		}
//		else
//		{
//			boneDict[boneName].parentID = -1;
//		}
//		
//		boneDict[boneName].transform = AssimpUtils::Convert(node->mTransformation);
//
//		for (int i = 0; i < node->mNumChildren; i++)
//			ReadParents(node->mChildren[i], boneDict);
//	}
//
//	void PrintHeirarchy(aiNode* node, unsigned int level)
//	{
//		for (int i = 0; i < level; i++)
//			std::cout << " ";
//
//		std::string name = node->mName.C_Str();
//		std::cout << level << " " << name << std::endl;
//
//		for (int i = 0; i < node->mNumChildren; i++)
//			PrintHeirarchy(node->mChildren[i], level + 1);
//	}
//
//	const Mat4& SkeletalMesh::GetSocketTransform(const std::string& name)
//	{
//		if (sockets.find(name) != sockets.end())
//		{
//			return sockets[name].GetWorldTransform();
//		}
//	}
//
//	void SkeletalMesh::SetSocketOffset(const std::string& name, const Mat4& offset)
//	{
//		if (sockets.find(name) != sockets.end())
//		{
//			sockets[name].SetOffset(offset);
//		}
//	}
//
//	void SkeletalMesh::AddSocket(const std::string& name, const std::string& boneName, const Mat4& offset)
//	{
//		if (sockets.find(name) == sockets.end())
//		{
//			sockets[name] = Socket(name, boneName, offset);
//		}
//	}
//
//	void SkeletalMesh::AddSockets(const Vector<Socket>& inSockets)
//	{
//		for (auto& socket : inSockets)
//		{
//			if (sockets.find(socket.GetName()) == sockets.end())
//			{
//				sockets[socket.GetName()] = Socket(socket.GetName(), socket.GetBoneName(), socket.GetOffset());
//			}
//		}
//	}
//
//	Ptr<SkeletalMesh> SkeletalMesh::Create(const std::string& filePath)
//	{
//		Assimp::Importer importer;
//
//		const aiScene* scene = importer.ReadFile(filePath,
//			aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_PopulateArmatureData
//		);
//
//		Unique<SkeletalVertex> vertices = std::make_unique<SkeletalVertex>();
//		Unique<unsigned int> indices = std::make_unique<unsigned int>();
//
//		size_t vertciesSize = 0;
//		unsigned int indexCount = 0;
//
//		Dict<std::string, BoneInfo> boneDict;
//		int boneCount = 0;
//		Mat4 globalInv = Mat4(1.0f);
//
//		if (scene->HasMeshes())
//		{
//
//			aiMesh* mesh = scene->mMeshes[0];
//			vertices.reset(new SkeletalVertex[mesh->mNumVertices]);
//			vertciesSize = sizeof(SkeletalVertex) * mesh->mNumVertices;
//
//			for (unsigned int v = 0; v < mesh->mNumVertices; v++)
//			{
//				// Positions
//				vertices.get()[v].position.x = mesh->mVertices[v].x;
//				vertices.get()[v].position.y = mesh->mVertices[v].y;
//				vertices.get()[v].position.z = mesh->mVertices[v].z;
//
//				// Texture Coordinates
//				vertices.get()[v].texCoord.x = mesh->mTextureCoords[0][v].x;
//				vertices.get()[v].texCoord.y = mesh->mTextureCoords[0][v].y;
//
//				// Normals
//				vertices.get()[v].normal.x = mesh->mNormals[v].x;
//				vertices.get()[v].normal.y = mesh->mNormals[v].y;
//				vertices.get()[v].normal.z = mesh->mNormals[v].z;
//				
//				for (int i = 0; i < MAX_BONE_WEIGHTS; i++)
//				{
//					vertices.get()[v].boneIDs[i] = -1;
//					vertices.get()[v].weights[i] = 0.0f;
//				}
//			}
//
//
//			std::vector<unsigned int> indicesList;
//
//			for (unsigned int f = 0; f < mesh->mNumFaces; f++) {
//				aiFace face = mesh->mFaces[f];
//				for (unsigned int ind = 0; ind < face.mNumIndices; ind++)
//				{
//					indicesList.push_back(face.mIndices[ind]);
//				}
//			}
//
//			indices.reset(new unsigned int[indicesList.size()]);
//			indexCount = indicesList.size();
//			for (int k = 0; k < indicesList.size(); k++)
//				indices.get()[k] = indicesList[k];
//
//			// Bones
//			
//			if (mesh->HasBones())
//			{
//				for (int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
//				{
//					int boneID = -1;
//					std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
//					if (boneDict.find(boneName) == boneDict.end())
//					{
//						BoneInfo newBoneInfo;
//						newBoneInfo.id = boneCount;
//						newBoneInfo.offset = AssimpUtils::Convert(mesh->mBones[boneIndex]->mOffsetMatrix);
//						boneDict[boneName] = newBoneInfo;
//						boneID = boneCount;
//						boneCount++;
//					}
//					else
//					{
//						boneID = boneDict[boneName].id;
//					}
//					assert(boneID != -1);
//					auto weights = mesh->mBones[boneIndex]->mWeights;
//					int numWeights = mesh->mBones[boneIndex]->mNumWeights;
//
//					for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex)
//					{
//						int vertexId = weights[weightIndex].mVertexId;
//						float weight = weights[weightIndex].mWeight;
//					
//						for (int i = 0; i < MAX_BONE_WEIGHTS; ++i)
//						{
//							if (vertices.get()[vertexId].boneIDs[i] < 0)
//							{
//								vertices.get()[vertexId].weights[i] = weight;
//								vertices.get()[vertexId].boneIDs[i] = boneID;
//								break;
//							}
//						}
//					}
//				}
//				globalInv = glm::inverse(AssimpUtils::Convert(scene->mRootNode->mTransformation));
//				ReadParents(mesh->mBones[0]->mNode, boneDict);
//			}
//		}
//
//		importer.FreeScene();
//
//		auto vao = VertexArray::Create();
//		auto vbo = VertexBuffer::Create((void*)vertices.get(), vertciesSize);
//
//		vbo->SetLayout({
//			{"position", 3},
//			{"texCoord", 2},
//			{"normal", 3},
//			{"boneIDs", MAX_BONE_WEIGHTS, AttribType::SL_ATTRIB_INT},
//			{"weights", MAX_BONE_WEIGHTS}
//		});
//
//		vao->AddVertexBuffer(vbo);
//		auto ebo = IndexBuffer::Create(indices.get(), indexCount * sizeof(unsigned int));
//		vao->SetIndexBuffer(ebo);
//
//		return std::make_shared<SkeletalMesh>(vbo, ebo, vao, indexCount, boneDict, boneCount, globalInv);
//	}
//	
//	SkeletalMesh::SkeletalMesh(Ptr<VertexBuffer> vbo, Ptr<IndexBuffer> ebo, Ptr<VertexArray> vao, int indexCount, Dict<std::string, BoneInfo> boneDict, int boneCounter, const Mat4& globalInverseTransform)
//		: Mesh(vbo, ebo, vao, indexCount), boneDict(boneDict), boneCounter(boneCounter), globalInverseTransform(globalInverseTransform)
//	{	
//	}
//
//
//}
