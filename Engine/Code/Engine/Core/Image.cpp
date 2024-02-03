#define STB_IMAGE_IMPLEMENTATION // Exactly one .CPP (this Image.cpp) should #define this before #including stb_image.h
#include "Engine/ThirdParty/stb/stb_image.h"

#include "Engine/Core/Image.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

//----------------------------------------------------------------------------------------------------------------------
Image::Image()
{
}

//----------------------------------------------------------------------------------------------------------------------
Image::~Image()
{
}

//----------------------------------------------------------------------------------------------------------------------
Image::Image( char const* imageFilePath )
{
    m_imageFilePath = imageFilePath;

	int bytesPerTexel			= 0;					// This will be filled in for us to indicate how many color components the image had (e.g. 3=RGB=24bit, 4=RGBA=32bit)
	int numComponentsRequested	= 0;					// don't care; we support 3 (24-bit RGB) or 4 (32-bit RGBA)

														// Load (and decompress) the image RGB(A) bytes from a file on disk into a memory buffer (array of bytes)
	stbi_set_flip_vertically_on_load( 1 );				// We prefer uvTexCoords has origin (0,0) at BOTTOM LEFT
	unsigned char* texelData = stbi_load( imageFilePath, &m_dimensions.x, &m_dimensions.y, &bytesPerTexel, numComponentsRequested );

	// Check if the load was successful
	GUARANTEE_OR_DIE( texelData, Stringf( "Failed to load image \"%s\"", imageFilePath ) );

	int numPixels = (m_dimensions.x) * (m_dimensions.y);
	m_rgbaTexels.reserve( numPixels );

	Rgba8 currentTexelData;
	for ( int i = 0; i < numPixels; i++ )
	{
		int offset			= i * bytesPerTexel;
		currentTexelData.r	= *( texelData + offset );
		if ( bytesPerTexel == 1 )
		{
			currentTexelData.g = currentTexelData.r;
			currentTexelData.b = currentTexelData.r;
			currentTexelData.a = 255;
		}
		else
		{
			currentTexelData.g	= *( texelData + offset + 1 );
			currentTexelData.b	= *( texelData + offset + 2 );
		}
		if ( bytesPerTexel	== 4 )
		{
			currentTexelData.a = *( texelData + offset + 3 );
		}
		m_rgbaTexels.push_back( currentTexelData );
	}

	// Free the raw image texel data now that we've sent a copy of it down to the GPU to be stored in video memory
	stbi_image_free( texelData );
}

//----------------------------------------------------------------------------------------------------------------------
Image::Image( IntVec2 size, Rgba8 color )
{
	m_dimensions = size;

	int numPixels;
	numPixels = (size.x) * (size.y);
	m_rgbaTexels.resize( numPixels );
    
	for ( int i = 0; i < numPixels; i++ )
	{
		m_rgbaTexels[i] = color;
	}
}

//----------------------------------------------------------------------------------------------------------------------
std::string const& Image::GetImageFilePath() const
{
    return m_imageFilePath;
}

//----------------------------------------------------------------------------------------------------------------------
IntVec2 Image::GetDimensions() const
{
    return m_dimensions;
}

//----------------------------------------------------------------------------------------------------------------------
const void* Image::GetRawData() const
{
    const void* data;
    data = m_rgbaTexels.data();
    return data;
}

//----------------------------------------------------------------------------------------------------------------------
// Get Texel color at index calculated from texelCoords
Rgba8 Image::GetTexelColor( IntVec2 const& texelCoords ) const
{
	int texelIndex = ( ( texelCoords.y * m_dimensions.x ) + texelCoords.x );
	return m_rgbaTexels[texelIndex];
}

//----------------------------------------------------------------------------------------------------------------------
void Image::SetTexelColor( IntVec2 const& texelCoords, Rgba8 const& newColor )
{
    UNUSED( texelCoords );
    UNUSED( newColor );
}
