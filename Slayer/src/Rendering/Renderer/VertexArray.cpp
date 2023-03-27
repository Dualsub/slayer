#include "Rendering/Renderer/VertexArray.h"
#include "Rendering/Renderer/Buffer.h"
#include "glad/glad.h"

namespace Slayer {

	VertexArray::VertexArray(unsigned int vaoID)
		: vaoID(vaoID)
	{
	}

	Shared<VertexArray> VertexArray::Create()
	{
		unsigned int vaoID;
		glGenVertexArrays(1, &vaoID);
		return std::make_shared<VertexArray>(vaoID);
	}
	
	Unique<VertexArray> VertexArray::CreateUnique()
	{
		unsigned int vaoID;
		glGenVertexArrays(1, &vaoID);
		return std::make_unique<VertexArray>(vaoID);
	}

	void VertexArray::Bind(unsigned int vaoID)
	{
		glBindVertexArray(vaoID);
	}

	void VertexArray::Bind()
	{
		glBindVertexArray(vaoID);
	}

	void VertexArray::Unbind()
	{
		glBindVertexArray(0);
	}

	void VertexArray::AddVertexBuffer(Shared<VertexBuffer> buffer)
	{
		// Calc Stride
		auto& layout = buffer->GetLayout();
		int stride = 0;
		for (auto& element : layout)
		{
			switch (element.type)
			{
			case AttribType::SL_ATTRIB_FLOAT: 
				stride += element.count * sizeof(float);
				break;
			case AttribType::SL_ATTRIB_INT:
				stride += element.count * sizeof(int);
				break;
			}
		}

		Bind();
		buffer->Bind();
		int vertexBufferIndex = 0;
		size_t offset = 0;
		for (auto& element : layout)
		{
			glEnableVertexAttribArray(vertexBufferIndex);
			switch (element.type)
			{
			case AttribType::SL_ATTRIB_FLOAT:
				glVertexAttribPointer(vertexBufferIndex,
					element.count,
					GL_FLOAT,
					GL_FALSE,
					stride,
					(void*)offset);
				offset += element.count * sizeof(float);
				break;
			case AttribType::SL_ATTRIB_INT:
				glVertexAttribIPointer(vertexBufferIndex,
					element.count,
					GL_INT,
					stride,
					(void*)offset);
				offset += element.count * sizeof(int);
				break;
			}
			vertexBufferIndex++;
		}
	}

	void VertexArray::SetIndexBuffer(Shared<IndexBuffer> buffer)
	{
		Bind();
		buffer->Bind();
		indexBuffer = buffer;
	}

}