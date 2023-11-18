#pragma once

#include "Core/Core.h"
#include "Core/Containers.h"
#include "glad/glad.h"

namespace Slayer {

	enum AttribType
	{
		SL_ATTRIB_NONE = 0,
		SL_ATTRIB_FLOAT = GL_FLOAT,
		SL_ATTRIB_INT = GL_INT
	};

	struct LayoutDescription
	{
		std::string name;
		unsigned int count;
		AttribType type;
		LayoutDescription(const std::string& name, unsigned int count, AttribType type = AttribType::SL_ATTRIB_FLOAT)
			: name(name), count(count), type(type)
		{
		}
	};

	class VertexBuffer
	{

	private:
		unsigned int vboID;
		Vector<LayoutDescription> layout;
	public:
		VertexBuffer(int vboID);
		~VertexBuffer();

		void  Bind();
		void  Unbind();

		const Vector<LayoutDescription>& GetLayout();
		void SetLayout(const Vector<LayoutDescription>& layout);
		void SetSubData(void* ptr, size_t size);

		static Shared<VertexBuffer> Create(void* vertcies, size_t size);
		static Shared<VertexBuffer> Create(size_t size);

		void Dispose();
	};

	class IndexBuffer
	{

	private:
		unsigned int eboID;
	public:
		IndexBuffer(int eboID);
		~IndexBuffer();

		void  Bind();
		void  Unbind();

		static Shared<IndexBuffer> Create(unsigned int* indices, int size);

		void Dispose();
	};

	class UniformBuffer
	{
	private:
		unsigned int uboID;
	public:
		UniformBuffer(int uboID);
		~UniformBuffer();

		void Bind();
		void Unbind();
		void Dispose();

		template<typename T>
		inline void SetSubData(T* data, size_t size, uint32_t offset = 0)
		{
			glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
		}

		template<typename T>
		void SetData(T* data, size_t size, uint32_t offset = 0)
		{
			Bind();
			SetSubData(data, size, offset);
			Unbind();
		}

		static Shared<UniformBuffer> Create(size_t size, int binding);
	};

	class ShaderStorageBuffer
	{
	private:
		unsigned int ssboID;
	public:
		ShaderStorageBuffer(int ssboID);
		~ShaderStorageBuffer();

		void Bind();
		void Unbind();
		void Dispose();

		template<typename T>
		inline void SetSubData(T* data, size_t size, int offset = 0)
		{
			glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, size, data);
		}

		template<typename T>
		void SetData(T* data, size_t size, int offset = 0)
		{
			Bind();
			glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, size, data);
			Unbind();
		}

		static Shared<ShaderStorageBuffer> Create(size_t size, int binding);
	};

	class PixelBuffer
	{
	private:
		bool m_hasBeenDisposed = false;
		unsigned int m_pboID;
	public:
		PixelBuffer(int pboID);
		~PixelBuffer();

		void Bind();
		void Unbind();
		void Dispose();

		template<typename T>
		T* MapBuffer()
		{
			return (T*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
		}

		void UnmapBuffer()
		{
			glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
		}

		static Shared<PixelBuffer> Create(size_t size);
	};
}