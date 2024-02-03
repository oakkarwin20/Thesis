#include "Engine/Renderer/ConstantBuffer.hpp"
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Renderer/Renderer.hpp"

#include <d3d11.h>

//----------------------------------------------------------------------------------------------------------------------
ConstantBuffer::ConstantBuffer( size_t size )
{
	m_size = size;
}

//----------------------------------------------------------------------------------------------------------------------
ConstantBuffer::~ConstantBuffer()
{
	DX_SAFE_RELEASE( m_buffer );
}