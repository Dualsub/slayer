#include "Rendering/Renderer/Buffer.h"

#include "glad/glad.h"

namespace Slayer {

	VertexBuffer::VertexBuffer(int vboID) :
		vboID(vboID)
	{

	}

	VertexBuffer::~VertexBuffer()
	{
	}

	void VertexBuffer::Bind()
	{
		glBindBuffer(GL_ARRAY_BUFFER, vboID);
	}

	void VertexBuffer::Unbind()
	{
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void VertexBuffer::SetSubData(void* ptr, size_t size)
	{
		glBindBuffer(GL_ARRAY_BUFFER, vboID);
		glBufferSubData(GL_ARRAY_BUFFER, 0, size, ptr);
	}

	void VertexBuffer::SetLayout(const Vector<LayoutDescription>& layout)
	{
		this->layout = layout;
	}

	const Vector<LayoutDescription>& VertexBuffer::GetLayout()
	{
		return layout;
	}

	Shared<VertexBuffer> VertexBuffer::Create(void* vertices, size_t size)
	{
		unsigned int vboID;
		glCreateBuffers(1, &vboID);
		glBindBuffer(GL_ARRAY_BUFFER, vboID);
		glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
		return MakeShared<VertexBuffer>(vboID);
	}

	void VertexBuffer::Dispose()
	{
		glDeleteBuffers(1, &vboID);
	}

	Shared<VertexBuffer> VertexBuffer::Create(size_t size)
	{
		unsigned int vboID;
		glCreateBuffers(1, &vboID);
		glBindBuffer(GL_ARRAY_BUFFER, vboID);
		glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
		return MakeShared<VertexBuffer>(vboID);
	}

	IndexBuffer::IndexBuffer(int eboID) : eboID(eboID)
	{
	}

	IndexBuffer::~IndexBuffer()
	{
	}

	void IndexBuffer::Bind()
	{
	}

	void IndexBuffer::Unbind()
	{
	}

	Shared<IndexBuffer> IndexBuffer::Create(unsigned int* indices, int size)
	{
		unsigned int eboID;
		glCreateBuffers(1, &eboID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, indices, GL_STATIC_DRAW);
		return MakeShared<IndexBuffer>(eboID);
	}

	UniformBuffer::UniformBuffer(int uboID) : uboID(uboID)
	{
	}

	UniformBuffer::~UniformBuffer()
	{
	}

	void UniformBuffer::Bind()
	{
		glBindBuffer(GL_UNIFORM_BUFFER, uboID);
	}

	void UniformBuffer::Unbind()
	{
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	void UniformBuffer::Dispose()
	{
		glDeleteBuffers(1, &uboID);
	}

	Shared<UniformBuffer> UniformBuffer::Create(size_t size, int binding)
	{
		unsigned int uboID;
		glGenBuffers(1, &uboID);
		glBindBuffer(GL_UNIFORM_BUFFER, uboID);
		glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		glBindBufferBase(GL_UNIFORM_BUFFER, binding, uboID);
		return MakeShared<UniformBuffer>(uboID);
	}

	ShaderStorageBuffer::ShaderStorageBuffer(int ssboID) : ssboID(ssboID)
	{
	}

	ShaderStorageBuffer::~ShaderStorageBuffer()
	{
	}

	void ShaderStorageBuffer::Bind()
	{
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboID);
	}

	void ShaderStorageBuffer::Unbind()
	{
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	void ShaderStorageBuffer::Dispose()
	{
		glDeleteBuffers(1, &ssboID);
	}

	Shared<ShaderStorageBuffer> ShaderStorageBuffer::Create(size_t size, int binding)
	{
		uint32_t ssbo;
		glGenBuffers(1, &ssbo);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
		glBufferData(GL_SHADER_STORAGE_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, ssbo);

		return MakeShared<ShaderStorageBuffer>(ssbo);
	}

}