#pragma once
#include "Engine/Math/IntVec2.hpp"

#include <string>
#include <vector>

//----------------------------------------------------------------------------------------------------------------------
struct Rgba8;

//----------------------------------------------------------------------------------------------------------------------
class Image
{
	friend class Renderer;

public:
	Image();
	~Image();
	Image( char const* imageFilePath );
	Image( IntVec2 size, Rgba8 color );

	std::string const&		GetImageFilePath() const;
	IntVec2					GetDimensions()	const;
	const void*				GetRawData() const;
	Rgba8					GetTexelColor( IntVec2 const& texelCoords ) const;
	void					SetTexelColor( IntVec2 const& texelCoords, Rgba8 const& newColor );

private:
	std::string				m_imageFilePath;
	IntVec2					m_dimensions;
	std::vector<Rgba8>		m_rgbaTexels;
};