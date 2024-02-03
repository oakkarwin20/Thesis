#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"

//----------------------------------------------------------------------------------------------------------------------
BitmapFont::BitmapFont( char const* fontFilePathNameWithNoExtension, Texture& fontTexture )
	: m_fontGlyphsSpriteSheet( fontTexture, IntVec2( 16, 16 ) )
	, m_fontFilePathNameWithNoExtension( fontFilePathNameWithNoExtension )
{
}

//----------------------------------------------------------------------------------------------------------------------
Texture& BitmapFont::GetTexture()
{
	return m_fontGlyphsSpriteSheet.GetTexture();
}

void BitmapFont::AddVertsForTextInBox2D( std::vector<Vertex_PCU>& vertexArray, AABB2 const& box, float cellHeight, std::string const& text, 
										 Rgba8 const& tint, float cellAspect, Vec2 const& alignment, TextDrawMode mode, int maxGlyphsToDraw )
{
	// Find Box dimensions X & Y
	// Find Text and Cell width
	// Use BoxDimensions and TextWidth to find UnusedSpace
	// UnusedSpaceX = BoxWidth - TextWidth
	// UnusedSpaceY = BoxHeight - TextHeight
	// textMinsX = (UnusedSpaceX * Alignment.x) + BoxMins.x

	// do a for loop through all characters checking string "\n"
	// splitStringOnDelimiter

	Strings vectorOfStrings = SplitStringOnDelimiter( text, '\n' );
	int		numLines		= static_cast<int>( vectorOfStrings.size() );
//	figure out longer text size
//	int		numLettersInText	= static_cast<int>( text.size() );

	float longestStringLength = -1.0f;
	for ( int i = 0; i < numLines; i++ )
	{
		float eachTextLineWidth	= GetTextWidth( cellHeight, vectorOfStrings[i], cellAspect );
		if ( longestStringLength < eachTextLineWidth )
		{
			longestStringLength = eachTextLineWidth;
		}
	}

	Vec2	boxDimensions		= box.GetDimensions();
	float	textHeight			= cellHeight * numLines;
	float	unusedSpaceX		= boxDimensions.x - longestStringLength;
	float	unusedSpaceY		= boxDimensions.y - textHeight;		

	Vec2 percentageOfUnusedAlignment;
	percentageOfUnusedAlignment.x = ( unusedSpaceX * alignment.x );
	percentageOfUnusedAlignment.y = ( unusedSpaceY * alignment.y );

	if ( mode == TextDrawMode::SHRINK_TO_FIT )
	{
		if ( unusedSpaceX < 0 && unusedSpaceX < unusedSpaceY )
		{
			float ratio = boxDimensions.x / longestStringLength;
			cellHeight *= ratio;
			longestStringLength = boxDimensions.x;
			textHeight *= ratio;

			unusedSpaceX = boxDimensions.x - longestStringLength;
			unusedSpaceY = boxDimensions.y - textHeight;

			percentageOfUnusedAlignment.x = alignment.x * unusedSpaceX;
			percentageOfUnusedAlignment.y = alignment.y * unusedSpaceY;
		}

		if ( unusedSpaceX < 0 && unusedSpaceX < unusedSpaceY )
		{
			float ratio = boxDimensions.y / textHeight;
			cellHeight *= ratio;
			longestStringLength *= ratio;
			textHeight = boxDimensions.y;

			unusedSpaceX = boxDimensions.x - longestStringLength;
			unusedSpaceY = boxDimensions.y - textHeight;
			
			percentageOfUnusedAlignment.x = alignment.x * unusedSpaceX;
			percentageOfUnusedAlignment.y = alignment.y * unusedSpaceY;
		}
	}

	Vec2 textBoxMins;
	textBoxMins.x = box.m_mins.x + percentageOfUnusedAlignment.x; 
	textBoxMins.y = box.m_mins.y + percentageOfUnusedAlignment.y;

	Vec2 textMins;
	textMins.y  = textBoxMins.y + ( ( cellHeight * (numLines - 1) ) );

	int glyphsToDisplayed = maxGlyphsToDraw;

	for ( int i = 0; i < numLines; i++ )
	{
		// how many chars in current line
		// currentLine length
		// 

//		int eachStringLength	= vectorOfStrings.size();
		int currentStringLength		= (int)vectorOfStrings[i].size();
		float currentStringWidth	= GetTextWidth( cellHeight, vectorOfStrings[i], cellAspect );
		float unusedStringSpaceX	=  longestStringLength - currentStringWidth;
		float currentStringPos		=  unusedStringSpaceX * alignment.x;
		textMins.x = textBoxMins.x + currentStringPos;

		if ( glyphsToDisplayed >= 0 )
		{
			if ( glyphsToDisplayed > currentStringLength )
			{
				AddVertsForText2D( vertexArray, textMins, cellHeight, vectorOfStrings[i], tint, cellAspect );
				glyphsToDisplayed -= currentStringLength;
			}
			else
			{
				std::string const& glyphysToRender = std::string( vectorOfStrings[i], 0 , glyphsToDisplayed);
				AddVertsForText2D( vertexArray, textMins, cellHeight, glyphysToRender, tint, cellAspect );
				glyphsToDisplayed = 0;
			}
		}
		else
		{
			break;
		}
		textMins.y -= cellHeight;
	}
}

//----------------------------------------------------------------------------------------------------------------------
void BitmapFont::AddVertsForText2D( std::vector<Vertex_PCU>& vertexArray, Vec2 const& textMins, float cellHeight, std::string const& text, Rgba8 const& tint, float cellAspect )
{
	for ( int i = 0; i < text.size(); i++ )
	{
		AABB2 cellUVs		= m_fontGlyphsSpriteSheet.GetSpriteUVs( static_cast<int>(text[i]) );
		float cellWidth		= cellAspect * cellHeight;
		Vec2  cellMins		= Vec2( ( cellWidth * i ) + textMins.x, textMins.y );
		Vec2  cellMaxs		= Vec2( cellMins.x + cellWidth, cellMins.y + cellHeight );
		AABB2 cellBounds	= AABB2( cellMins, cellMaxs );
		AABB2 uvs			= AABB2( Vec2( 0.0f, 0.0f ), Vec2( 1.0f, 1.0f ) );

		AddVertsForAABB2D( vertexArray, cellBounds, tint, cellUVs );
	}
}

//----------------------------------------------------------------------------------------------------------------------
void BitmapFont::AddVertsForText3D( std::vector<Vertex_PCU>& verts, float cellHeight, std::string const& text, Rgba8 const& tint, float cellAspect, Vec2 const& alignment, int maxGlyphsToDraw )
{
	UNUSED ( maxGlyphsToDraw );
	AddVertsForText2D( verts, Vec2( 0.0f, 0.0f ), cellHeight, text, tint, cellAspect );
	AABB2 textBounds = AABB2( Vec2( 0.0f, 0.0f ), Vec2( 0.0f, 0.0f ) );
	for ( int i = 0; i < verts.size(); i++ )
	{
		textBounds.StretchToIncludePoint( Vec2(verts[i].m_position.x, verts[i].m_position.y) );
	}
	
	Mat44 textMatrix;
	float textOffset_X = -alignment.x * textBounds.GetDimensions().x;
	float textOffset_Y = -alignment.y * textBounds.GetDimensions().y;
	textMatrix.AppendYRotation( 90.0f );
	textMatrix.AppendZRotation( 90.0f );
	textMatrix.AppendTranslation3D( Vec3( textOffset_X, textOffset_Y, 0.0f ) );
	TransformVertexArray3D( verts, textMatrix );
}

//----------------------------------------------------------------------------------------------------------------------
void BitmapFont::AddVertsForText3D( std::vector<Vertex_PCU>& verts, Vec3 const& textOrigin, 
	Vec3 iBasis, Vec3 jBasis, float cellHeight, std::string const& text, Rgba8 const& tint, float cellAspect, int maxGlyphsToDraw )
{
	UNUSED( maxGlyphsToDraw );

	// Add verts in 2D first (in identity world position)
	int numVertsBefore	= int( verts.size() );
	AddVertsForText2D( verts, Vec2::ZERO, cellHeight, text, tint, cellAspect );
	int numVertsAfter	= int( verts.size() );
	int numVertsAdded	= numVertsAfter - numVertsBefore;

	// Transform JUST THESE NEW VERTS into their proper 3D position
	int indexWhereNextVertsStart	= numVertsBefore;
	Vec3 kBasis						= CrossProduct3D( iBasis, jBasis ).GetNormalized();
	Mat44 transform( iBasis, jBasis, kBasis, textOrigin );
 	TransformVertexArray3D( numVertsAdded, &verts[indexWhereNextVertsStart], transform );
}

//----------------------------------------------------------------------------------------------------------------------
float BitmapFont::GetTextWidth( float cellHeight, std::string const& text, float cellAspect )
{
	float cellWidth = cellAspect * cellHeight;
//	float numOfChars = (float)text.size();
	float textWidth = cellWidth * text.length();
	return textWidth;
}

//----------------------------------------------------------------------------------------------------------------------
float BitmapFont::GetGlyphAspect( int glyphUnicode ) const
{
	UNUSED(glyphUnicode);
	return 1.0f;
}