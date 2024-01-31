#pragma once

#include "Engine/Math/Vec2.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Renderer/BitmapFont.hpp"

//------------------------------------------------------------------------------------------------------------------------
class GameModeProtogame3D; 
class GameModeBase;

//----------------------------------------------------------------------------------------------------------------------
class App
{
public:
	App();
	~App();
	void Startup();
	void Shutdown();
	void Run();
	void RunFrame();
	 
	bool IsQuitting() const { return m_isQuitting; }
	bool HandleKeyPressed(unsigned char keyCode);	
	bool HandleKeyReleased(unsigned char keyCode);
	bool HandleQuitRequested();

	static bool Quit( EventArgs& args );

	//----------------------------------------------------------------------------------------------------------------------
	// AttractMode functions
	void UpdateAttractModeCam();
	void UpdateReturnToAttractMode();
	void AttractModeInput();
	void RenderAttractMode() const;
	void RenderGameModeMenu() const;

private:
	void BeginFrame();
	void Update( float deltaSeconds );				// updates entities' position
	void Render() const;		// draws entities' every frame
	void EndFrame();

private:
	bool					m_isQuitting		= false;
//	GameModeProtogame3D*	m_theGame			= nullptr;
	Camera					m_devConsoleCamera;
	Camera					m_attractCamera;
	bool					m_attractModeIsOn	= true;
	GameModeBase*			m_theGameMode		= nullptr;

public:
	Clock					m_gameClock;
	// Textures
	BitmapFont*				m_textFont				= nullptr;
	Texture*				m_texture_TestUV		= nullptr;
	Texture*				m_texture_TestOpenGL	= nullptr;
	Texture*				m_texture_MoonSurface	= nullptr;
	Texture*				m_texture_RockWithGrass	= nullptr;
	Texture*				m_texture_GlowingRock	= nullptr;
	Texture*				m_texture_Grass			= nullptr;
	Texture*				m_texture_Brick			= nullptr;
	Texture*				m_texture_Galaxy		= nullptr;
};