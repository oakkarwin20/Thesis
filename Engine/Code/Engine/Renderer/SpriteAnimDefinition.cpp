#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Math/MathUtils.hpp"

//----------------------------------------------------------------------------------------------------------------------
SpriteAnimDefinition::SpriteAnimDefinition( SpriteSheet const& sheet, int startSpriteIndex, int endSpriteIndex, float framesPerSeconds, SpriteAnimPlaybackType playbackType )
	: m_spriteSheet(sheet)
	, m_startSpriteIndex( startSpriteIndex)
	, m_endSpriteIndex(endSpriteIndex)
	, m_secondsPerFrame( 1.0f / framesPerSeconds )
	, m_playbackType( playbackType)
{
}

//----------------------------------------------------------------------------------------------------------------------
// #ToDo fix this function, the indexCount returned is still not correct
SpriteDefinition const& SpriteAnimDefinition::GetSpriteDefAtTime( float seconds ) const
{
	if ( m_playbackType == SpriteAnimPlaybackType::ONCE )
	{
		int animFrame = RoundDownToInt( seconds / m_secondsPerFrame );
		int currentSpriteIndex = animFrame + m_startSpriteIndex;

		if ( currentSpriteIndex > m_endSpriteIndex )
		{
			currentSpriteIndex = m_endSpriteIndex;
		}
		
		if ( currentSpriteIndex < m_startSpriteIndex )
		{
			currentSpriteIndex = m_startSpriteIndex;
		}

		return m_spriteSheet.GetSpriteDef( currentSpriteIndex );
	}
	else if ( m_playbackType == SpriteAnimPlaybackType::LOOP ) 
	{
		int numFramesInLoop = ( m_endSpriteIndex - m_startSpriteIndex ) + 1; 
		int rawAnimFrame = RoundDownToInt( seconds / m_secondsPerFrame );
		while ( rawAnimFrame < 0 )
		{
			rawAnimFrame += numFramesInLoop;
		}
		int loopedAnimFrame = rawAnimFrame % numFramesInLoop;
		int currentSpriteIndex = loopedAnimFrame + m_startSpriteIndex;

		return m_spriteSheet.GetSpriteDef( currentSpriteIndex );
	}
	else if ( m_playbackType == SpriteAnimPlaybackType::PING_PONG )
	{
		//	#ToDo solve this later
		return m_spriteSheet.GetSpriteDef( m_startSpriteIndex);
	}

	ERROR_AND_DIE( "Unknown spriteAnim PlaybackType");

	/* Previous code below
	*
	// 
	//	int numFrames				= ( m_endSpriteIndex - m_startSpriteIndex ) + 1;
	//	float lengthOfAnimation		= static_cast<float>(numFrames) * m_secondsPerFrame;
	//	float animationLoopCount	= seconds / lengthOfAnimation;
	//	float indexCount			= animationLoopCount * numFrames;
	//	
	////	SpriteDefinition sprite( m_spriteSheet, (int)indexCount, Vec2( 0.0f, 0.0f ), Vec2( 1.0f, 1.0f ) );
	//	int spriteIndex = m_startSpriteIndex;	// #ToDo Calculate this for real
	//	SpriteDefinition const& sprite = m_spriteSheet.GetSpriteDef( spriteIndex );

	//	DebuggerPrintf( "time=%f, start=%i, end=%i, #frames=#i, SPF=%f, currentFrame=%i, currentSprite=%i\n",
	//		seconds, m_startSpriteIndex, m_endSpriteIndex, numFrames, m_secondsPerFrame, numFrames, indexCount);

	//	return sprite;
	*
	*/
}