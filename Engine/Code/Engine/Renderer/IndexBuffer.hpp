#pragma once

//----------------------------------------------------------------------------------------------------------------------
struct ID3D11Buffer;

//----------------------------------------------------------------------------------------------------------------------
class IndexBuffer
{
public:
	friend class Renderer;

	IndexBuffer( size_t size );
	IndexBuffer( IndexBuffer const& copy ) = delete;
	virtual ~IndexBuffer();

	unsigned int GetStride() const;

	ID3D11Buffer* m_buffer = nullptr;
	size_t			m_size = 0;
};