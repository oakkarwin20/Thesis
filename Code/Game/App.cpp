#include "Game/App.hpp"
#include "Game/GameModeBase.hpp"
#include "Game/GameCommon.hpp"

#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Window/Window.hpp"

//----------------------------------------------------------------------------------------------------------------------
InputSystem*			g_theInput			= nullptr;
Window*					g_theWindow			= nullptr;
Renderer*				g_theRenderer		= nullptr; //Created and owned by the App
RandomNumberGenerator*	g_theRNG			= nullptr;
AudioSystem*			g_theAudio			= nullptr;
App*					g_theApp			= nullptr; //Created and owned by Main_Windows.cpp

//----------------------------------------------------------------------------------------------------------------------
GameMode				g_gameModeNum = GAMEMODE_3D;

//----------------------------------------------------------------------------------------------------------------------
App::App()
{  
}

//----------------------------------------------------------------------------------------------------------------------
App::~App()
{
	m_textFont				= nullptr;
	m_texture_TestUV		= nullptr;
	m_texture_MoonSurface	= nullptr;
	m_texture_RockWithGrass	= nullptr;
	m_texture_GlowingRock	= nullptr;
	m_texture_Grass			= nullptr;
	m_texture_Brick			= nullptr;
	m_texture_Galaxy		= nullptr;
}

//----------------------------------------------------------------------------------------------------------------------
void App::Startup()
{
	// Creating EventSystem
	g_theEventSystem = new EventSystem();

	// Create engine subsystems and game
	InputSystemConfig inputSystemConfig;
	g_theInput = new InputSystem( inputSystemConfig );

	// Creating Window
	WindowConfig windowConfig;
	windowConfig.m_windowTitle	= "SkeletalPlayground";
	windowConfig.m_clientAspect = 2.0f;
	windowConfig.m_inputSystem	= g_theInput;
	windowConfig.m_isFullScreen = true;
	g_theWindow					= new Window( windowConfig );

	// Creating Renderer
	RendererConfig rendererConfig;
	rendererConfig.m_window		= g_theWindow;
	g_theRenderer				= new Renderer( rendererConfig );

	// Creating DevConsole
	DevConsoleConfig devConsoleConfig;
	m_devConsoleCamera.SetOrthoView( Vec2( 0.0f, 0.0f), Vec2(WORLD_SIZE_X, WORLD_SIZE_Y) );
	devConsoleConfig.m_camera	= &m_devConsoleCamera; 
	devConsoleConfig.m_renderer	= g_theRenderer;
	g_theDevConsole				= new DevConsole( devConsoleConfig );

	// Creating AudioSystem
	AudioSystemConfig audioConfig;
	g_theAudio = new AudioSystem( audioConfig );
	
	// Creating DebugRenderSystem
	DebugRenderConfig debugRenderConfig;
	debugRenderConfig.m_renderer = g_theRenderer;
	DebugRenderSystemStartup( debugRenderConfig );

	// Creating RNG
	g_theRNG = new RandomNumberGenerator();

	// Start up engine subsystems and game
	g_theEventSystem->Startup();
	 g_theDevConsole->Startup();
  	      g_theInput->StartUp();
	     g_theWindow->Startup();
	   g_theRenderer->Startup();
	      g_theAudio->Startup();

//	m_theGame = new GameModeProtogame3D();
//	m_theGame->StartUp();

	//----------------------------------------------------------------------------------------------------------------------
	// Initializing bitmap font text
	m_textFont = g_theRenderer->CreateOrGetBitmapFontFromFile( "Data/Fonts/SquirrelFixedFont" );

	//----------------------------------------------------------------------------------------------------------------------
	// Initializing texture
	m_texture_TestUV		= g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/TestUV.png" );
	m_texture_MoonSurface	= g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/MoonSurface.png" );
	m_texture_RockWithGrass = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/rockWithGrass.png" );
	m_texture_Grass			= g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Grass.png" );
	m_texture_GlowingRock	= g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/GlowingRock.png" );
	m_texture_Galaxy		= g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/galaxy.png" );
	m_texture_TestOpenGL	= g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Test_StbiFlippedAndOpenGL.png" );

	//----------------------------------------------------------------------------------------------------------------------
	g_theEventSystem->SubscribeToEvent( "quit", App::Quit );
	g_theEventSystem->SubscribeToEvent( "debugrenderclear", Command_DebugRenderClear );
	g_theEventSystem->SubscribeToEvent( "debugrendertoggle", Command_DebugRenderToggle );

	//----------------------------------------------------------------------------------------------------------------------
	// Debug keys for "FIFA_TEST_3D"
	g_theDevConsole->AddLine( Rgba8::GREEN, "Keys"														 );
	g_theDevConsole->AddLine( Rgba8::GREEN, "Mouse          - Aim (While camera controlled)"			 );
	g_theDevConsole->AddLine( Rgba8::GREEN, "WASD           - Move Actor"								 );	
	g_theDevConsole->AddLine( Rgba8::GREEN, "Q              - Descend"									 );
	g_theDevConsole->AddLine( Rgba8::GREEN, "E              - Ascend"									 );
	g_theDevConsole->AddLine( Rgba8::GREEN, "Shift          - Run"										 );	
	g_theDevConsole->AddLine( Rgba8::GREEN, "F1             - Toggle controls (Camera/Player)"			 );	
	g_theDevConsole->AddLine( Rgba8::GREEN, "F2             - Toggle Raycast"							 );	
	g_theDevConsole->AddLine( Rgba8::GREEN, "F3             - Toggle Ideal Limb Positions"				 );	
	g_theDevConsole->AddLine( Rgba8::GREEN, "F4             - Toggle Debug text"						 );	
	g_theDevConsole->AddLine( Rgba8::GREEN, "F5             - Toggle Render Legs"						 );	
	g_theDevConsole->AddLine( Rgba8::GREEN, "0 / 9          - Zoom in / out"							 );	
	g_theDevConsole->AddLine( Rgba8::GREEN, "U / O          - Raise / lower Elevator"					 );	
	g_theDevConsole->AddLine( Rgba8::GREEN, "P              - Pause Game"								 );	
	g_theDevConsole->AddLine( Rgba8::GREEN, "Escape         - Exit Game"								 );	
}

//----------------------------------------------------------------------------------------------------------------------
void App::Shutdown()
{
	     g_theAudio->Shutdown();
	  g_theRenderer->Shutdown();
	    g_theWindow->Shutdown();
	     g_theInput->ShutDown();
	g_theDevConsole->Shutdown();
	DebugRenderSystemShutdown();
//	  m_theGameMode->Shutdown();

	delete g_theAudio;
	g_theAudio = nullptr;

	delete g_theRenderer;
	g_theRenderer = nullptr;

	delete g_theWindow;
	g_theWindow = nullptr;

	delete g_theInput;
	g_theInput = nullptr;

	delete g_theDevConsole;
	g_theDevConsole = nullptr;
	
	delete m_theGameMode;
	m_theGameMode = nullptr;

}
 
//-----------------------------------------------------------------------------------------------
// One "frame" of the game.  Generally: Input, Update, Render.  We call this 60+ times per second.
void App::RunFrame()
{
	float deltaSeconds = m_gameClock.GetDeltaSeconds();
	BeginFrame();
	Update( deltaSeconds );
	Render();	
	EndFrame();	
}
 
//----------------------------------------------------------------------------------------------------------------------
bool App::HandleKeyPressed(unsigned char keyCode)
{
	g_theInput->HandleKeyPressed( keyCode );
	return true;
}

//----------------------------------------------------------------------------------------------------------------------
bool App::HandleKeyReleased(unsigned char keyCode)
{
	g_theInput->HandleKeyReleased( keyCode );
	return true;
}

//----------------------------------------------------------------------------------------------------------------------
bool App::HandleQuitRequested()
{
	m_isQuitting = true;
	return false;
}

//----------------------------------------------------------------------------------------------------------------------
bool App::Quit( EventArgs& args )
{
	UNUSED( args ); 
	return g_theApp->HandleQuitRequested();
}

//----------------------------------------------------------------------------------------------------------------------
void App::Run()
{
	// Program main loop; keep running frames until it's time to quit
	while (!IsQuitting())
	{
		g_theApp->RunFrame();
	}
}

//----------------------------------------------------------------------------------------------------------------------
void App::BeginFrame()
{
	Clock::TickSystemClock();

	g_theDevConsole->BeginFrame();
	     g_theInput->BeginFrame();
	    g_theWindow->BeginFrame();
	  g_theRenderer->BeginFrame();
	     g_theAudio->BeginFrame();

	DebugRenderBeginFrame();
}	 
 
//----------------------------------------------------------------------------------------------------------------------
void App::Update( float deltaSeconds ) 
{ 
	if ( m_attractModeIsOn )
	{
		// updates only attractMode cam
		UpdateAttractModeCam();
		AttractModeInput();
		return;
	}
	m_theGameMode->Update( deltaSeconds );

	//----------------------------------------------------------------------------------------------------------------------
	// Mouse is visible if ( window does NOT have focus || devConsole is open || m_attractModeIsOn
	if ( g_theWindow->HasFocus() == false || g_theDevConsole->m_isOpen == true || m_attractModeIsOn == true || g_gameModeNum == GAMEMODE_2D )
	{
		g_theInput->SetCursorMode( false, false );
	}
	// Else, Mouse is hidden
	else
	{
		g_theInput->SetCursorMode( true, true );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// F7 for next gameMode
	if ( g_theInput->WasKeyJustPressed( KEYCODE_F7 ) )
	{
		g_gameModeNum = static_cast<GameMode>( ( g_gameModeNum + 1 ) % NUM_GAMEMODES );
		if ( m_theGameMode )
		{
			m_theGameMode->Shutdown();
			delete m_theGameMode;
		}
		m_theGameMode = GameModeBase::CreateNewGameOfType( g_gameModeNum );
		m_theGameMode->Startup();
	}

	//----------------------------------------------------------------------------------------------------------------------
	// F6 for previous gameMode
	if ( g_theInput->WasKeyJustPressed( KEYCODE_F6 ) )
	{
		g_gameModeNum = static_cast<GameMode>( ( g_gameModeNum + NUM_GAMEMODES - 1 ) % NUM_GAMEMODES );
		if ( m_theGameMode )
		{
			m_theGameMode->Shutdown();
			delete m_theGameMode;
		}
		m_theGameMode = GameModeBase::CreateNewGameOfType( g_gameModeNum );
		m_theGameMode->Startup();
	}

	UpdateReturnToAttractMode();
}

//----------------------------------------------------------------------------------------------------------------------
void App::Render() const
{
	// Draw attract mode
	if ( m_attractModeIsOn )
	{
		// uses attractModeCam and draws relevant stuff
		g_theRenderer->ClearScreen( Rgba8::BLACK );
		RenderAttractMode();
	}

	if ( !m_attractModeIsOn )
	{
		g_theRenderer->ClearScreen( Rgba8::DARKER_GRAY );
		m_theGameMode->Render();
	}

	//----------------------------------------------------------------------------------------------------------------------
	if ( g_theDevConsole )
	{
		AABB2 bounds = AABB2( 0.0f, 0.0f, WORLD_SIZE_X, WORLD_SIZE_Y );
		g_theDevConsole->Render( bounds );
	}
} 

//----------------------------------------------------------------------------------------------------------------------
void App::EndFrame()
{
	g_theDevConsole->EndFrame();
	     g_theInput->EndFrame();
	    g_theWindow->EndFrame();
	  g_theRenderer->EndFrame();
	     g_theAudio->EndFrame();

	DebugRenderEndFrame();
}

//----------------------------------------------------------------------------------------------------------------------
void App::UpdateAttractModeCam()
{
	// update AttractCam only if in AttractMode
	if ( m_attractModeIsOn )
	{
		m_attractCamera.SetOrthoView( Vec2( 0.0f, 0.0f ), Vec2( ATTRACT_MODE_SIZE_X, ATTRACT_MODE_SIZE_Y ) );
		return;
	}
}

//----------------------------------------------------------------------------------------------------------------------
void App::UpdateReturnToAttractMode()
{
	if ( g_theInput->WasKeyJustPressed( KEYCODE_ESC ) && m_attractModeIsOn == false )
	{
		m_attractModeIsOn = true;

		// Reset common debug variables
		g_debugFreeFly_F1 = false;
	}
}

//----------------------------------------------------------------------------------------------------------------------
void App::AttractModeInput()
{
	if ( g_theInput->WasKeyJustPressed( KEYCODE_ESC ) )
	{
		g_theApp->HandleQuitRequested();
	}

	XboxController const& controller = g_theInput->GetController( 0 );

	if ( g_theInput->WasKeyJustPressed( ' ' ) || g_theInput->WasKeyJustPressed( 'N' ) || g_theInput->WasKeyJustPressed( 'P' ) || 
		controller.WasButtonJustPressed( BUTTON_START ) || controller.WasButtonJustPressed( BUTTON_A ) )
	{
		g_gameModeNum = GAMEMODE_2D;
		m_theGameMode = GameModeBase::CreateNewGameOfType( g_gameModeNum );
		m_attractModeIsOn = false;
		m_theGameMode->Startup();
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Setting up attractMode menu input
	if ( g_theInput->WasKeyJustPressed( '1' ) )
	{
		g_gameModeNum	  = GAMEMODE_2D;
		m_theGameMode	  = GameModeBase::CreateNewGameOfType( g_gameModeNum );
		m_attractModeIsOn = false;
		m_theGameMode->Startup();
	}
	if ( g_theInput->WasKeyJustPressed( '2' ) )
	{
		g_gameModeNum	  = GAMEMODE_3D;
		m_theGameMode	  = GameModeBase::CreateNewGameOfType( g_gameModeNum );
		m_attractModeIsOn = false;
		m_theGameMode->Startup();
	}
	if ( g_theInput->WasKeyJustPressed( '3' ) )
	{
		g_gameModeNum	  = GAMEMODE_CONSTRAINTS_TEST_3D;
		m_theGameMode	  = GameModeBase::CreateNewGameOfType( g_gameModeNum );
		m_attractModeIsOn = false;
		m_theGameMode->Startup();
	}
	if ( g_theInput->WasKeyJustPressed( '4' ) )
	{
		g_gameModeNum	  = GAMEMODE_BIPED_WALK_ANIM_3D;
		m_theGameMode	  = GameModeBase::CreateNewGameOfType( g_gameModeNum );
		m_attractModeIsOn = false;
		m_theGameMode->Startup();
	}
	if ( g_theInput->WasKeyJustPressed( '5' ) )
	{
		g_gameModeNum	  = GAMEMODE_MULTIPLE_END_EFFECTORS_3D;
		m_theGameMode	  = GameModeBase::CreateNewGameOfType( g_gameModeNum );
		m_attractModeIsOn = false;
		m_theGameMode->Startup();
	}
	if ( g_theInput->WasKeyJustPressed( '6' ) )
	{
		g_gameModeNum	  = GAMEMODE_EULER_3D;
		m_theGameMode	  = GameModeBase::CreateNewGameOfType( g_gameModeNum );
		m_attractModeIsOn = false;
		m_theGameMode->Startup();
	}
}

//----------------------------------------------------------------------------------------------------------------------
void App::RenderAttractMode() const
{
	g_theRenderer->BeginCamera( m_attractCamera );

	std::vector<Vertex_PCU> attractModeVerts;
	attractModeVerts.reserve( 9 );

	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( &m_textFont->GetTexture() );
	g_theRenderer->DrawVertexArray( static_cast<int>( attractModeVerts.size() ), attractModeVerts.data() );
	g_theRenderer->BindTexture( nullptr );

	RenderGameModeMenu();

	g_theRenderer->EndCamera( m_attractCamera );
}

//----------------------------------------------------------------------------------------------------------------------
void App::RenderGameModeMenu() const
{
	std::vector<Vertex_PCU> textVerts;
	std::vector<Vertex_PCU> boxVerts;

	//----------------------------------------------------------------------------------------------------------------------
	// Menu background filler box
//	AABB2 backgroundBounds = AABB2( Vec2( 0.0f, 0.0f ), Vec2( 380.0f, 300.0f ) );
//	backgroundBounds.SetCenter( Vec2( ATTRACT_MODE_SIZE_X * 0.78f , ATTRACT_MODE_SIZE_Y * 0.47f ) );
//	AddVertsForAABB2D( textVerts, backgroundBounds, Rgba8::DARK_RED );

	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexArray( (int)textVerts.size(), textVerts.data() );
	g_theRenderer->BindTexture( nullptr );

	//----------------------------------------------------------------------------------------------------------------------
	// ProtoMode selection text
	AABB2 box = AABB2( Vec2( 0.0f, 0.0f ), Vec2( ATTRACT_MODE_SIZE_X, ATTRACT_MODE_SIZE_Y ) );
	box.SetCenter( Vec2( ATTRACT_MODE_CENTER_X, ATTRACT_MODE_SIZE_Y * 0.5f ) );
	m_textFont->AddVertsForTextInBox2D( textVerts, box, 15.0f, 
		"1: Skeletal PlayGround 2D\n\n2: Skeletal PlayGround 3D\n\n3: Constraints TestMode 3D\n\n4: GameMode_BipedWalkAnim_3D\n\n5: GameMode_MultipleEndEffectors\n\n6: GameMode_Euler", 
		Rgba8::WHITE );

	//----------------------------------------------------------------------------------------------------------------------
	// "ProtoMode" title
	AABB2 textBox = AABB2( Vec2( 0.0f, 0.0f ), Vec2( ATTRACT_MODE_SIZE_X, ATTRACT_MODE_SIZE_Y ) );
	textBox.SetCenter( Vec2(ATTRACT_MODE_CENTER_X, ATTRACT_MODE_SIZE_Y * 0.75f) );
	m_textFont->AddVertsForTextInBox2D( textVerts, textBox, 80.0f, "Skeletal PlayGround", Rgba8::CYAN );

	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( &m_textFont->GetTexture() );
	g_theRenderer->DrawVertexArray( (int)textVerts.size(), textVerts.data() );
	g_theRenderer->BindTexture( nullptr );
}