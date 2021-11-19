#pragma once

#include "vertexformat.hpp"

namespace graphics {

	class GeometryBuffer
	{
	public:
		/// \param [in] _attributes List of vertex attributes for the geometry and the instance data.
		///		There will be two separated vertex buffer objects for the two types of data.
		///		If none of the attributes is instanced no second vertex buffer is generated.
		/// \param [in] _indexed Use an index buffer for indexed geometry if set greater 0.
		///		If greater 0 it must be 1, 2, or 4 defining the size of an index.
		/// \param [in] _initialSize Initial size of the buffer in bytes.
		GeometryBuffer(GLPrimitiveType _type, const VertexAttribute* _attributes, int _numAttributes, int _indexed, unsigned _initialSize = 1024);
		GeometryBuffer(const GeometryBuffer&) = delete;
		// Move semantics are possible but the default generated ones would be wrong.
		GeometryBuffer(GeometryBuffer&&) = delete;
		GeometryBuffer& operator=(GeometryBuffer&&) = delete;
		GeometryBuffer& operator=(const GeometryBuffer&) = delete;
		~GeometryBuffer();

		/// Replace current block of data.
		/// This might cause an glBufferData. Sub-data-updates are used if possible.
		void setData(const void* _data, unsigned _size);
		void setIndexData(const void* _data, unsigned _size);
		void setInstanceData(const void* _data, unsigned _size);

		/// Bind for draw calls. Do not change its content!
		void bind() const;
		void draw() const;

	private:
		unsigned m_vao;				///< OpenGL vertex array object
		unsigned m_vbo;				///< OpenGL vertex buffer object
		unsigned m_vboInstances;	///< Instance data
		unsigned m_ibo;				///< OpenGL index buffer object with 32bit indices
		GLPrimitiveType m_glType;	///< OpenGL primitive type

		unsigned m_capacity;		///< Allocated memory on CPU and GPU.
		unsigned m_indexCapacity;
		unsigned m_instanceCapacity;

		unsigned m_vertexSize;
		unsigned m_instanceVertexSize;
		unsigned m_indexSize;

		unsigned m_vertexCount;		///< Current number of vertices (from last set call)
		unsigned m_indexCount;		///< Current number of indices (from last set call)
		unsigned m_instanceCount;	///< Current number of instances (from last set call)
	};

} // namespace graphics