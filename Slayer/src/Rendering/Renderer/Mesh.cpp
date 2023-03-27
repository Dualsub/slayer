#include "Rendering/Renderer/Mesh.h"

namespace Slayer {

	Unique<Mesh::Quad> Mesh::quad;

	Mesh::Mesh(Shared<VertexBuffer> vbo, Shared<IndexBuffer> ebo, Shared<VertexArray> vao, int numIndicies) :
		vbo(vbo), ebo(ebo), vao(vao), numIndicies(numIndicies)
	{
	}
	
	void Mesh::Bind()
	{
		vao->Bind();
	}

	void Mesh::Unbind()
	{
		vao->Unbind();
	}

	Shared<Mesh> Mesh::Create(float* vertices, uint32_t vertciesSize, uint32_t* indicies, uint32_t indiciesCount, const Vector<LayoutDescription>& layout)
	{
		auto vao = VertexArray::Create();
		auto vbo = VertexBuffer::Create(vertices, vertciesSize);
		
		if(layout.empty())
			vbo->SetLayout(DEFAULT_LAYOUT);
		else
			vbo->SetLayout(layout);

		vao->AddVertexBuffer(vbo);
		auto ebo = IndexBuffer::Create(indicies, indiciesCount * sizeof(uint32_t));
		vao->SetIndexBuffer(ebo);
		return std::make_shared<Mesh>(vbo, ebo, vao, indiciesCount);
	}

	void Mesh::Dispose()
	{
		//vbo->Dispose();
		//ebo->Dispose();
		//vao->Dispose();
	}
}