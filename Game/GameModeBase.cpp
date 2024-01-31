#include "Game/GameMode_BipedWalkAnim_3D.hpp"
#include "Game/GameMode_Euler3D.hpp"
#include "Game/GameMode_MultipleEndEffectors.hpp"
#include "Game/GameMode_Constraints_3D.hpp"
#include "Game/GameModeBase.hpp"
#include "Game/GameMode2D.hpp"
#include "Game/GameMode3D.hpp"
#include "Game/App.hpp"

#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"

//----------------------------------------------------------------------------------------------------------------------
GameModeBase::GameModeBase()
{
}

//----------------------------------------------------------------------------------------------------------------------
GameModeBase::~GameModeBase()
{
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeBase::Startup()
{
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeBase::Shutdown()
{
}

//----------------------------------------------------------------------------------------------------------------------
GameModeBase* GameModeBase::CreateNewGameOfType( GameMode type )
{
	switch ( type )
	{
		case GAMEMODE_2D:							return new GameMode2D();
		case GAMEMODE_3D:							return new GameMode3D();
		case GAMEMODE_CONSTRAINTS_TEST_3D:			return new GameMode_Constraints_3D();
		case GAMEMODE_BIPED_WALK_ANIM_3D:			return new GameMode_BipedWalkAnim_3D();
		case GAMEMODE_MULTIPLE_END_EFFECTORS_3D:	return new GameMode_MultipleEndEffectors();
		case GAMEMODE_EULER_3D:						return new GameMode_Euler3D();
		
		default:
		{
			ERROR_AND_DIE( Stringf( "ERROR: Unknown GameMode #%i", type ) );
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeBase::UpdatePauseQuitAndSlowMo()
{
	// Pause functionality
	if ( g_theInput->WasKeyJustPressed( 'P' ) || g_theInput->GetController( 0 ).WasButtonJustPressed( XboxButtonID::BUTTON_START ) )
	{
		SoundID testSound = g_theAudio->CreateOrGetSound( "Data/Audio/TestSound.mp3" );
		g_theAudio->StartSound( testSound );			// Comment out this line of code to remove pause sound playing

		g_theApp->m_gameClock.TogglePause();
	}

	// Slow-Mo functionality
	if ( g_theInput->IsKeyDown( 'T' ) )
	{
		g_theApp->m_gameClock.SetTimeScale( 0.1f );
	}
	// Fast-Mo functionality
	else if ( g_theInput->IsKeyDown( 'Y' ) )
	{
		g_theApp->m_gameClock.SetTimeScale( 2.0f );
	}
	else
	{
		g_theApp->m_gameClock.SetTimeScale( 1.0f );
	}

	// Step one frame
	if ( g_theInput->WasKeyJustPressed( 'O' ) )
	{
//		g_theApp->m_gameClock.StepSingleFrame();
	}	
}
