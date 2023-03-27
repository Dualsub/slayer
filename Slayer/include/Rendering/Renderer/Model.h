#pragma once

#include "Core/Core.h"
#include "Core/Containers.h"
#include "Mesh.h"
#include "Resources/Asset.h"

#define MAX_MESHES 8

namespace Slayer {

	class Model
	{
	private:
		AssetID assetID;
		Vector<Shared<Mesh>> meshes;
	public:
		Model(const Vector<Shared<Mesh>>& inMeshes)
			: meshes(inMeshes)
		{
		}
		Model() = default;

		Vector<Shared<Mesh>>& GetMeshes() { return meshes; }

		void AddMesh(Shared<Mesh> mesh) { meshes.push_back(mesh); }

		void Dispose();
	};
}
