#pragma once

#include "Core/Core.h"

namespace Slayer {

	class IndexBuffer;
	class VertexBuffer;

	class VertexArray
	{
	private:
		unsigned int vaoID;
		Shared<IndexBuffer> indexBuffer;
	public:
		VertexArray(unsigned int vaoID);
		static Shared<VertexArray> Create();
		static Unique<VertexArray> CreateUnique();

		inline unsigned int GetID() { return vaoID; };

		static void Bind(unsigned int vaoID);
		static void Unbind();
		void Bind();

		void AddVertexBuffer(Shared<VertexBuffer> buffer);
		void SetIndexBuffer(Shared<IndexBuffer> buffer);

		void Dispose();
	};

}
