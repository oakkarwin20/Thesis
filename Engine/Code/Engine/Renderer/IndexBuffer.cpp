#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/Renderer/Renderer.hpp"

#include <d3d11.h>

//----------------------------------------------------------------------------------------------------------------------
IndexBuffer::IndexBuffer( size_t size )
{
	m_size = size;
}

//----------------------------------------------------------------------------------------------------------------------
IndexBuffer::~IndexBuffer()
{
	DX_SAFE_RELEASE( m_buffer );
}

//----------------------------------------------------------------------------------------------------------------------
unsigned int IndexBuffer::GetStride() const
{
	return sizeof( unsigned int );
}
