#pragma once

#include "opengl.hpp"

namespace graphics {

	/// A buffer is a pure memory block on GPU side.
	class Buffer
	{
	public:
		// The type of a buffer determines its main purpose.
		// The buffer will be bound to that slot for several operations.
		// However, you can always bind the same buffer for different
		// purposes.
		enum class Type
		{
			VERTEX = GL_ARRAY_BUFFER,
			INDEX = GL_ELEMENT_ARRAY_BUFFER,
			SHADER_STORAGE = GL_SHADER_STORAGE_BUFFER,
			TEXTURE = GL_TEXTURE_BUFFER,
			UNIFORM = GL_UNIFORM_BUFFER,
			ATOMIC = GL_ATOMIC_COUNTER_BUFFER,
			INDIRECT_DISPATCH = GL_DISPATCH_INDIRECT_BUFFER,
			INDIRECT_DRAW = GL_DRAW_INDIRECT_BUFFER,
			TRANSFORM_FEEDBACK = GL_TRANSFORM_FEEDBACK_BUFFER,
		};

		enum Usage
		{
			// Allow glBufferSubData updates for this buffer.
			SUB_DATA_UPDATE = GL_DYNAMIC_STORAGE_BIT,
			// Allow read accesses from CPU side
			MAP_READ = GL_MAP_READ_BIT,
			// Allow write accesses form CPU side
			MAP_WRITE = GL_MAP_WRITE_BIT,
			// The buffer can always be mapped. Even during draw
			// CPU side accesses are possible.
			// Without this bit the buffer must be unmapped before
			// used by the GPU.
			// To make writes visible use flush() or receive().
			MAP_PERSISTENT = GL_MAP_PERSISTENT_BIT,
			// You don't need to call flush(). Consider this as
			// less performant. receive() is still necessary for
			// synchronization issues.
			MAP_COHERENT = GL_MAP_COHERENT_BIT
		};

		// Buffers can only use a restricted set of the internal formats
		enum class TextureFormat
		{
			// One channel
			R8 = GL_R8,
			R8I = GL_R8I,
			R8UI = GL_R8UI,
			R16 = GL_R16,
			R16I = GL_R16I,
			R16UI = GL_R16UI,
			R16F = GL_R16F,
			R32I = GL_R32I,
			R32UI = GL_R32UI,
			R32F = GL_R32F,

			// Two channels
			RG8 = GL_RG8,
			RG8I = GL_RG8I,
			RG8UI = GL_RG8UI,
			RG16 = GL_RG16,
			RG16I = GL_RG16I,
			RG16UI = GL_RG16UI,
			RG16F = GL_RG16F,
			RG32I = GL_RG32I,
			RG32UI = GL_RG32UI,
			RG32F = GL_RG32F,

			// Three channels
			RGB32F = GL_RGB32F,
			RGB32I = GL_RGB32I,
			RGB32UI = GL_RGB32UI,

			// Four channels
			RGBA8 = GL_RGBA8,
			RGBA8I = GL_RGBA8I,
			RGBA8UI = GL_RGBA8UI,
			RGBA16 = GL_RGBA16,
			RGBA16I = GL_RGBA16I,
			RGBA16UI = GL_RGBA16UI,
			RGBA16F = GL_RGBA16F,
			RGBA32I = GL_RGBA32I,
			RGBA32UI = GL_RGBA32UI,
			RGBA32F = GL_RGBA32F,
		};

		// Create a raw buffer for data. Buffers created with this method
		// cannot be used as texture buffers!
		// _data: data for initialization.
		Buffer(Type _type, GLuint _elementSize, GLuint _numElements, Usage _usageBits = Usage(), const GLvoid* _data = nullptr);
		// Create a buffer including a texture view
		Buffer(Type _type, GLuint _elementSize, GLuint _numElements, TextureFormat _format, Usage _usageBits = Usage(), const GLvoid* _data = nullptr);
		~Buffer();
		// Move but not copy-able
		Buffer(Buffer&& _rhs) noexcept;
		Buffer(const Buffer&) = delete;
		Buffer& operator = (Buffer&& _rhs) noexcept;
		Buffer& operator = (const Buffer& _rhs) = delete;

		// Bind as vertex buffer
		// _offset: offset to the first element in number of elements
		void bindAsVertexBuffer(GLuint _bindingIndex, GLuint _offset = 0);
		// Bind to GL_ELEMENT_ARRAY_BUFFER (index buffer)
		void bindAsIndexBuffer();
		// Bind as uniform buffer
		// _bindingIndex: binding slot of the uniform buffer
		// _offset: it is possible to bind a part of a larger buffer.
		//		This gives the offset in byte to the begin of the range.
		// _size: size of the range in bytes. -1 binds the entire buffer.
		void bindAsUniformBuffer(GLuint _bindingIndex, GLintptr _offset = 0, GLsizeiptr _size = GLsizeiptr(-1));
		// Bind as shader storage buffer
		// See bindAsUBO for details on the range.
		void bindAsShaderStorageBuffer(GLuint _bindingIndex, GLintptr _offset = 0, GLsizeiptr _size = GLsizeiptr(-1));
		// Bind as texture buffer. This requires an initialization with a TextureFormat.
		void bindAsTextureBuffer(GLuint _bindingIndex);
		// Bind as atomic counter buffer.
		void bindAsAtomicCounterBuffer(GLuint _bindingIndex, GLintptr _offset = 0, GLsizeiptr _size = GLsizeiptr(-1));

		// Upload a small chunk of data to a specific position.
		// Requires Usage::SUB_DATA_UPDATE.
		void subDataUpdate(GLintptr _offset, GLsizei _size, const GLvoid* _data);


		// Set the entire buffer content to zero.
		void clear();


		enum MappingFlags
		{
			READ = GL_MAP_READ_BIT,
			WRITE = GL_MAP_WRITE_BIT,
			// Open a persistent mapping.
			// Coherent is added automatically if set in Usage during initialization.
			// Otherwise, GL_MAP_FLUSH_EXPLICIT_BIT will be set.
			PERSISTENT = GL_MAP_PERSISTENT_BIT,
			// Discard the previous buffer contents. Cannot be used with READ.
			INVALIDATE = GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_INVALIDATE_RANGE_BIT,
			// Do not wait for GPU operations on the buffer. Map it immediately.
			ASYNCHRONOUS = GL_MAP_UNSYNCHRONIZED_BIT,
		};

		// Mapping is the recommended way to handle buffer data.
		// You can read/write the returned pointer until unmap(). If mapped
		// persistently you must make changes visible through flush() and receive().
		void* map(MappingFlags _access, GLintptr _offset = 0, GLsizei _size = GLsizei(-1));
		// Release mapping (pointer becomes invalid), a flush is done automatically
		// except for PERSISTENT INCOHERENT buffers.
		void unmap();
		// Make CPU sided writes visible for the GPU.
		void flush();
		// Make GPU sided writes visible on CPU (slow sync!).
		// Mapping with the READ flag is doing this automatically except
		// ASYNCHRONOUS is set.
		void receive();

		GLuint numElements() const { return m_size / m_elementSize; }

		GLuint glID() const { return m_id; }
	private:
		GLuint m_id;
		GLuint m_tbID;			// An extra ID for a texture buffer view
		Type m_type;
		GLuint m_size;
		GLuint m_elementSize;
		Usage m_usage;

		GLintptr m_mappedOffset;
		GLuint m_mappedSize;
	};

} // namespace graphics