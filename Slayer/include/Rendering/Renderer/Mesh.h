#pragma once

#include "Core/Core.h"

#include "Rendering/Renderer/VertexArray.h"
#include "Rendering/Renderer/Buffer.h"
#include "Rendering/Renderer/Material.h"

#define DEFAULT_LAYOUT {\
	{ "position",	3},\
	{ "texcoord",	2 },\
	{ "normal",	3 }}

namespace Slayer {

	class Mesh
	{
		struct Quad
		{
			Unique<VertexArray> quadVAO;

			Quad()
			{
				if (!quadVAO)
				{
					float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
						// positions   // texCoords
						-1.0f,  1.0f,  0.0f, 1.0f,
						-1.0f, -1.0f,  0.0f, 0.0f,
						 1.0f, -1.0f,  1.0f, 0.0f,

						-1.0f,  1.0f,  0.0f, 1.0f,
						 1.0f, -1.0f,  1.0f, 0.0f,
						 1.0f,  1.0f,  1.0f, 1.0f
					};

					auto quadVBO = VertexBuffer::Create(quadVertices, sizeof(quadVertices));
					quadVBO->Bind();
					quadVBO->SetLayout({ {"position", 2}, {"texcoord", 2} });
					quadVAO = VertexArray::CreateUnique();
					quadVAO->AddVertexBuffer(quadVBO);
					quadVBO->Unbind();
				}
			}
			~Quad()
			{

			}

			VertexArray* GetVertexArray()
			{
				return quadVAO.get();
			}

			unsigned int GetIndexCount() { return 6; }
		};

		static Unique<Quad> quad;

	public:

		static unsigned int GetQuadVaoID()
		{
			if (!quad)
			{
				quad = std::make_unique<Quad>();
			}

			return quad->GetVertexArray()->GetID();
		}

		static unsigned int GetQuadIndexCount()
		{
			if (!quad)
			{
				quad = std::make_unique<Quad>();
			}

			return quad->GetIndexCount();
		}

		Mesh() = default;
		Mesh(Shared<VertexBuffer> vbo, Shared<IndexBuffer> ebo, Shared<VertexArray> vao, int numIndicies);

		void Bind();
		void Unbind();
		void SetMaterial(Shared<Material> inMaterial) { material = inMaterial; }
		void SetDefaultMaterial(const AssetID& id) { defaultMaterialId = id; }
		inline unsigned int GetIndexCount() { return numIndicies; }
		inline Shared<Material> GetMaterial() { return material; }
		inline unsigned int GetVaoID() { return vao->GetID(); }
		static Shared<Mesh> Create(float* vertices, uint32_t vertciesSize, uint32_t* indicies, uint32_t indiciesCount, const Vector<LayoutDescription>& layout = Vector<LayoutDescription>());
		void Dispose();
	protected:
		Shared<Material> material;
		AssetID defaultMaterialId;
		Shared<VertexBuffer> vbo;
		Shared<IndexBuffer> ebo;
		Shared<VertexArray> vao;
		unsigned int numIndicies = 0;
	};

}
