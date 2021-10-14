#include "buffer.hpp"
#include <spdlog/spdlog.h>

namespace graphics {

	Buffer::Buffer(Type _type, GLuint _elementSize, GLuint _numElements, Usage _usageBits, const GLvoid* _data) :
		m_tbID(0),
		m_type(_type),
		m_size(_elementSize * _numElements),
		m_elementSize(_elementSize),
		m_usage(_usageBits),
		m_mappedOffset(0),
		m_mappedSize(0)
	{
		// Generated one buffer
		glGenBuffers(1, &m_id);
		// Allocate space
		glBindBuffer(static_cast<GLenum>(m_type), m_id);
		glBufferStorage(static_cast<GLenum>(m_type), m_size, _data, static_cast<GLbitfield>(_usageBits));

		spdlog::debug("Created buffer {} with size {}.", m_id, m_size);
	}

	Buffer::Buffer(Type _type, GLuint _elementSize, GLuint _numElements, TextureFormat _format, Usage _usageBits, const GLvoid * _data) :
		Buffer(_type, _elementSize, _numElements, _usageBits, _data)
	{
		// Create the texture view
		glGenTextures(1, &m_tbID);
		glBindTexture(GL_TEXTURE_BUFFER, m_tbID);
		glTexBuffer(GL_TEXTURE_BUFFER, static_cast<GLenum>(_format), m_id);

		spdlog::debug("Created texture view {} for the buffer {}.", m_tbID, m_id);
	}

	Buffer::~Buffer()
	{
		glDeleteBuffers(1, &m_id);
		glDeleteTextures(1, &m_tbID);

		spdlog::debug("Deleted buffer {} (w/o texture view {}).", m_id, m_tbID);
	}

	Buffer::Buffer(Buffer&& _rhs) noexcept :
		m_id(_rhs.m_id),
		m_tbID(_rhs.m_tbID),
		m_type(_rhs.m_type),
		m_size(_rhs.m_size),
		m_elementSize(_rhs.m_elementSize),
		m_usage(_rhs.m_usage),
		m_mappedOffset(_rhs.m_mappedOffset),
		m_mappedSize(_rhs.m_mappedSize)
	{
		_rhs.m_id = 0;
		_rhs.m_tbID = 0;
	}

	Buffer& Buffer::operator=(Buffer&& _rhs) noexcept
	{
		glDeleteBuffers(1, &m_id);
		glDeleteTextures(1, &m_tbID);

		m_id = _rhs.m_id;
		m_tbID = _rhs.m_tbID;
		m_type = _rhs.m_type;
		m_size = _rhs.m_size;
		m_elementSize = _rhs.m_elementSize;
		m_usage = _rhs.m_usage;
		m_mappedOffset = _rhs.m_mappedOffset;
		m_mappedSize = _rhs.m_mappedSize;
		_rhs.m_id = 0;
		_rhs.m_tbID = 0;
		return *this;
	}

	void Buffer::bindAsVertexBuffer(GLuint _bindingIndex, GLuint _offset)
	{
		glBindVertexBuffer(_bindingIndex, m_id, _offset * m_elementSize, m_elementSize);
	}

	void Buffer::bindAsIndexBuffer()
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id);
	}

	void Buffer::bindAsUniformBuffer(GLuint _bindingIndex, GLintptr _offset, GLsizeiptr _size)
	{
		if(_size == -1)
			_size = m_size - _offset;
		glBindBufferRange(GL_UNIFORM_BUFFER, _bindingIndex, m_id, _offset, _size);
	}

	void Buffer::bindAsShaderStorageBuffer(GLuint _bindingIndex, GLintptr _offset, GLsizeiptr _size)
	{
		if(_size == -1)
			_size = m_size - _offset;
		glBindBufferRange(GL_SHADER_STORAGE_BUFFER, _bindingIndex, m_id, _offset, _size);
	}

	void Buffer::bindAsTextureBuffer(GLuint _bindingIndex)
	{
		glActiveTexture(GL_TEXTURE0 + _bindingIndex);
		glBindTexture(GL_TEXTURE_BUFFER, m_tbID);
	}

	void Buffer::bindAsAtomicCounterBuffer(GLuint _bindingIndex, GLintptr _offset, GLsizeiptr _size)
	{
		if(_size == -1)
			_size = m_size - _offset;
		glBindBufferRange(GL_ATOMIC_COUNTER_BUFFER, _bindingIndex, m_id, _offset, _size);
	}

	void Buffer::subDataUpdate(GLintptr _offset, GLsizei _size, const GLvoid* _data)
	{
		if(!(m_usage & Usage::SUB_DATA_UPDATE)) 
		{
			spdlog::error("Buffer::subDataUpdate requires Usage::SUB_DATA_UPDATE. The current buffer is static.");
			return;
		}

		if(_size == -1)
			_size = m_size - (GLsizei)_offset;

		// Bind to arbitrary buffer point to call the storage command
		glBindBuffer(static_cast<GLenum>(m_type), m_id);
		glBufferSubData(static_cast<GLenum>(m_type), _offset, _size, _data);
	}

	void Buffer::clear()
	{
		glBindBuffer(static_cast<GLenum>(m_type), m_id);
		unsigned zero = 0;
		glClearBufferData(static_cast<GLenum>(m_type), GL_R32UI, GL_RED, GL_UNSIGNED_INT, &zero);
	}

	void* Buffer::map(MappingFlags _access, GLintptr _offset, GLsizei _size)
	{
		if(_size == -1)
			_size = m_size - (GLsizei)_offset;

		GLbitfield access = _access & (GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
		if(_access & MappingFlags::PERSISTENT)
		{
			if(m_usage & Usage::MAP_COHERENT)
				access |= GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
			else
				access |= GL_MAP_PERSISTENT_BIT | GL_MAP_FLUSH_EXPLICIT_BIT;
		}
		if(_access & MappingFlags::INVALIDATE)
			access |= (_size==GLsizei(m_size)) ? GL_MAP_INVALIDATE_BUFFER_BIT : GL_MAP_INVALIDATE_RANGE_BIT;

		m_mappedOffset = _offset;
		m_mappedSize = _size;
		glBindBuffer(static_cast<GLenum>(m_type), m_id);
		return glMapBufferRange(static_cast<GLenum>(m_type), _offset, _size, access);
	}

	void Buffer::unmap()
	{
		glBindBuffer(static_cast<GLenum>(m_type), m_id);
		glUnmapBuffer(static_cast<GLenum>(m_type));
	}

	void Buffer::flush()
	{
		glBindBuffer(static_cast<GLenum>(m_type), m_id);
		if(m_usage & Usage::MAP_PERSISTENT && !(m_usage & Usage::MAP_COHERENT))
			glFlushMappedBufferRange(static_cast<GLenum>(m_type), m_mappedOffset, m_mappedSize);
		glMemoryBarrier(GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT);
	}

	void Buffer::receive()
	{
		glMemoryBarrier(GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT);
		glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	}

}
