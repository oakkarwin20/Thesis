#pragma once

#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Core/Rgba8.hpp"

#include <vector>
#include <string>

//----------------------------------------------------------------------------------------------------------------------
struct Vec2;
struct Vertex_PCU;
class Texture;

//----------------------------------------------------------------------------------------------------------------------
enum class TextDrawMode 
{
	SHRINK_TO_FIT,
	OVERRUN,
};

//----------------------------------------------------------------------------------------------------------------------
class BitmapFont
{
	friend class Renderer;

private:
	BitmapFont( char const* fontFilePathNameWithNoExtension, Texture& fontTexture );

public:
	Texture& GetTexture();

	void AddVertsForTextInBox2D( std::vector<Vertex_PCU>& vertexArray, AABB2 const& box, float cellHeight,
								 std::string const& text, Rgba8 const& tint = Rgba8::WHITE, float cellAspect = 1.f,
								 Vec2 const& alignment = Vec2( 0.5f, 0.5f ), TextDrawMode mode = TextDrawMode::OVERRUN, int maxGlyphsToDraw = 99999999 );
	void AddVertsForText2D( std::vector<Vertex_PCU>& vertexArray, Vec2 const& textMins, float cellHeight, std::string const& text, Rgba8 const& tint = Rgba8::WHITE, float cellAspect = 1.0f );
	void AddVertsForText3D( std::vector<Vertex_PCU>& verts, float cellHeight, std::string const& text, Rgba8 const& tint = Rgba8::WHITE, float cellAspect = 1.0f, Vec2 const& alignment = Vec2( 0.5f, 0.5f ), int maxGlyphsToDraw = 999999999 );
	void AddVertsForText3D( std::vector<Vertex_PCU>& verts, Vec3 const& textOrigin, Vec3 iBasis, Vec3 jBasis, float cellHeight, std::string const& text, Rgba8 const& tint = Rgba8::WHITE, float cellAspect = 1.0f, int maxGlyphsToDraw = 999999999 );
	float GetTextWidth( float cellHeight, std::string const& text, float cellAspect = 1.0f );

protected:
	float GetGlyphAspect( int glyphUnicode ) const; // for now, this will always return 1.0f

protected:
	std::string		m_fontFilePathNameWithNoExtension;
	SpriteSheet		m_fontGlyphsSpriteSheet;
};