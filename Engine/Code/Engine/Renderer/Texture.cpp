#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/ThirdParty/stb/stb_image.h"

#include "Engine/Renderer/Renderer.hpp"

#include <d3d11.h>

//----------------------------------------------------------------------------------------------------------------------
Texture::Texture()
{
}

//----------------------------------------------------------------------------------------------------------------------
Texture::~Texture()
{
	DX_SAFE_RELEASE( m_texture );
	DX_SAFE_RELEASE( m_shaderResourceView );
}