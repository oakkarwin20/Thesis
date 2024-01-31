#pragma once

#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/BitmapFont.hpp"

//----------------------------------------------------------------------------------------------------------------------
enum GameMode
{
	GAMEMODE_INVALID = -1,
	GAMEMODE_2D,
	GAMEMODE_3D,
	GAMEMODE_CONSTRAINTS_TEST_3D,
	GAMEMODE_BIPED_WALK_ANIM_3D,
	GAMEMODE_MULTIPLE_END_EFFECTORS_3D,
	GAMEMODE_EULER_3D,
	NUM_GAMEMODES,
};

//----------------------------------------------------------------------------------------------------------------------
class GameModeBase
{
public:
	GameModeBase();
	virtual ~GameModeBase();
	virtual void Startup();
	virtual void Update( float deltaSeconds )	= 0;
	virtual void Render() const					= 0;
	virtual void Shutdown();

	static GameModeBase* CreateNewGameOfType( GameMode type );

	// Game common functionality
	void UpdatePauseQuitAndSlowMo();

public:
};