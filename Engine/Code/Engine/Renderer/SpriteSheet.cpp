#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/Renderer.hpp"

//----------------------------------------------------------------------------------------------------------------------
SpriteSheet::SpriteSheet( Texture& texture, IntVec2 const& simpleGridLayout ) 
	: m_texture( texture )
{
	float offsetAmount = 1.0f / 128.0f;
	IntVec2 textureDimension = m_texture.GetDimensions();

	float textureOffsetX = ( 1.0f /  textureDimension.x ) * offsetAmount;
	float textureOffsetY = ( 1.0f /  textureDimension.y ) * offsetAmount;

	// e.g. gets 1/8th for 8x8
	float uPerSpriteX = 1.0f / (float) simpleGridLayout.x;
	float vPerSpriteY = 1.0f / (float) simpleGridLayout.y;

	for ( int spriteY = 0; spriteY < simpleGridLayout.y; spriteY++ )
	{
		for ( int spriteX = 0; spriteX < simpleGridLayout.x; spriteX++ )
		{
			int spriteIndex = ( spriteY * simpleGridLayout.x ) + spriteX;

			float minU = ( uPerSpriteX * (float)spriteX ) + textureOffsetX;
			float maxU = ( uPerSpriteX + minU ) - textureOffsetX;
			
			float maxV = ( 1.0f - ( vPerSpriteY * (float)spriteY ) ) - textureOffsetY;
			float minV = ( maxV - vPerSpriteY ) + textureOffsetY;

			SpriteDefinition spriteDef( *this, spriteIndex, Vec2( minU, minV ), Vec2( maxU, maxV ) );
			m_spriteDefs.push_back( spriteDef );
		}
	}
}

Texture& SpriteSheet::GetTexture() const
{
	// TODO: insert return statement here
	return m_texture;
}

int SpriteSheet::GetNumSprites() const
{
	return (int)m_spriteDefs.size();
} 

SpriteDefinition const& SpriteSheet::GetSpriteDef( int spriteIndex ) const
{
	// TODO: insert return statement here
	return m_spriteDefs[spriteIndex];
}

void SpriteSheet::GetSpriteUVs( Vec2& out_uvAtMins, Vec2& out_uvAtMaxs, int spriteIndex ) const
{
	m_spriteDefs[spriteIndex].GetUVs( out_uvAtMins, out_uvAtMaxs );
}

AABB2 SpriteSheet::GetSpriteUVs( int spriteIndex ) const
{
	return m_spriteDefs[spriteIndex].GetUVs();
}
