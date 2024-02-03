#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"

SpriteDefinition::SpriteDefinition( SpriteSheet const& spriteSheet, int spriteIndex, Vec2 const& uvAtMins, Vec2 const& uvAtMaxs ) 
	: m_spriteSheet(spriteSheet)
	,m_uvAtMins(uvAtMins)
	,m_uvAtMaxs(uvAtMaxs)
	,m_spriteIndex(spriteIndex)
{
}

void SpriteDefinition::GetUVs( Vec2& out_uvAtMins, Vec2& out_uvAtMaxs ) const
{
	out_uvAtMins = m_uvAtMins;
	out_uvAtMaxs = m_uvAtMaxs;
}

AABB2 SpriteDefinition::GetUVs() const
{
	return AABB2( m_uvAtMins, m_uvAtMaxs );
}

SpriteSheet const& SpriteDefinition::GetSpriteSheet() const
{
	// TODO: insert return statement here
	return m_spriteSheet;
}

Texture& SpriteDefinition::GetTexture() const
{
	// TODO: insert return statement here
	return m_spriteSheet.GetTexture();
}

float SpriteDefinition::GetAspect() const
{
	return 0.0f;
}
