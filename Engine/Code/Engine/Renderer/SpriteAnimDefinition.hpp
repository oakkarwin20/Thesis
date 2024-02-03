#pragma once

#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Engine/Core/XmlUtils.hpp"

//----------------------------------------------------------------------------------------------------------------------
enum class SpriteAnimPlaybackType
{
	ONCE,
	LOOP,
	PING_PONG,
};

//----------------------------------------------------------------------------------------------------------------------
class SpriteAnimDefinition
{
public:
	SpriteAnimDefinition( SpriteSheet const& sheet, int startSpriteIndex, int endSpriteIndex, float framesPerSeconds = 20.0f, SpriteAnimPlaybackType playbackType = SpriteAnimPlaybackType::LOOP );
	SpriteDefinition const& GetSpriteDefAtTime( float seconds ) const;
	bool LoadFromXmlElement( XmlElement const& element );

public:
	int							m_startSpriteIndex	= -1;
	int							m_endSpriteIndex	= -1;
	SpriteSheet const&			m_spriteSheet;
	float						m_secondsPerFrame	= 0.05f;
	SpriteAnimPlaybackType		m_playbackType		= SpriteAnimPlaybackType::LOOP;
};