#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/Renderer.hpp"

#include <d3d11.h>

//----------------------------------------------------------------------------------------------------------------------
VertexBuffer::VertexBuffer( size_t size )
{
	m_size = size;
}

//----------------------------------------------------------------------------------------------------------------------
VertexBuffer::VertexBuffer( size_t size, unsigned int stride )
{
	m_size	 = size;
	m_stride = stride;
}

//----------------------------------------------------------------------------------------------------------------------
VertexBuffer::~VertexBuffer()
{
	// #Question // is it ok to include renderer? Since renderer is a friend class, not sure of relationship or rules
	DX_SAFE_RELEASE( m_buffer );
}

//----------------------------------------------------------------------------------------------------------------------
unsigned int VertexBuffer::GetStride() const
{
	return m_stride;
}
