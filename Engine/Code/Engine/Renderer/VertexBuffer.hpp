#pragma once

//----------------------------------------------------------------------------------------------------------------------
struct ID3D11Buffer;

//----------------------------------------------------------------------------------------------------------------------
class VertexBuffer
{
public:
	friend class Renderer;

	VertexBuffer( size_t size );
	VertexBuffer( size_t size, unsigned int stride );
	VertexBuffer( VertexBuffer const& copy ) = delete;
	virtual ~VertexBuffer();

	unsigned int GetStride() const;

	ID3D11Buffer*	m_buffer	= nullptr;
	size_t			m_size		= 0;
	unsigned int	m_stride	= 0;
};