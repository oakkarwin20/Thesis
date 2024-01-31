#include "Game/App.hpp"
#include "Game/GameMode_MultipleEndEffectors.hpp"

#include "Engine/Core/Time.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/SkeletalSystem/IK_Chain3D.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/SkeletalSystem/CreatureBase.hpp"

//----------------------------------------------------------------------------------------------------------------------
GameMode_MultipleEndEffectors::GameMode_MultipleEndEffectors()
{
	//----------------------------------------------------------------------------------------------------------------------
	// Initialize Bezier Curve
	//----------------------------------------------------------------------------------------------------------------------
	m_timer.Restart();
}

//----------------------------------------------------------------------------------------------------------------------
GameMode_MultipleEndEffectors::~GameMode_MultipleEndEffectors()
{
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode_MultipleEndEffectors::Startup()
{
	g_debugFreeFly_F1 = true;

	InitializeCreature();
	InitializeTrees();

	//----------------------------------------------------------------------------------------------------------------------
	// Initialize Player cam pos and orientaiton
	//----------------------------------------------------------------------------------------------------------------------
	m_gameMode3DWorldCamera.m_position						= Vec3( -30.0f, 0.0f, 50.0f ); 
	m_gameMode3DWorldCamera.m_orientation.m_yawDegrees		=  0.0f;
	m_gameMode3DWorldCamera.m_orientation.m_pitchDegrees	= 50.0f;

	m_bezierCurve.m_startPos	= m_rightFoot->m_target.m_currentPos;
	m_bezierCurve.m_guidePos1	= Vec3( m_rightFoot->m_target.m_currentPos.x +  3.0f, m_rightFoot->m_target.m_currentPos.y, m_rightFoot->m_target.m_currentPos.z + 5.0f );
	m_bezierCurve.m_guidePos2	= Vec3( m_rightFoot->m_target.m_currentPos.x +  7.0f, m_rightFoot->m_target.m_currentPos.y, m_rightFoot->m_target.m_currentPos.z + 5.0f );
	m_bezierCurve.m_endPos		= Vec3( m_rightFoot->m_target.m_currentPos.x + 10.0f, m_rightFoot->m_target.m_currentPos.y, m_rightFoot->m_target.m_currentPos.z );
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode_MultipleEndEffectors::Update( float deltaSeconds )
{	
	// Update core systems
	UpdatePauseQuitAndSlowMo();
	UpdateDebugKeys();
	UpdateCameraInput( deltaSeconds );
	UpdateCreatureInput( deltaSeconds );

	//----------------------------------------------------------------------------------------------------------------------
	// Rotate endEffector orientation
	//----------------------------------------------------------------------------------------------------------------------
	if ( g_theInput->IsKeyDown( KEYCODE_LEFTARROW  ) )
	{
		// Rotate "yaw" CCW
		m_rightFoot->m_target.m_fwdDir		= RotateVectorAboutArbitraryAxis( m_rightFoot->m_target.m_fwdDir, m_rightFoot->m_target.m_upDir,     1.0f );
		m_rightFoot->m_target.m_leftDir	= RotateVectorAboutArbitraryAxis( m_rightFoot->m_target.m_fwdDir, m_rightFoot->m_target.m_upDir,    90.0f );
		m_rightFoot->m_target.m_upDir		= RotateVectorAboutArbitraryAxis( m_rightFoot->m_target.m_fwdDir, m_rightFoot->m_target.m_leftDir, -90.0f );
	}
	if ( g_theInput->IsKeyDown( KEYCODE_RIGHTARROW ) )
	{
		// Rotate "yaw" CW
		m_rightFoot->m_target.m_fwdDir		= RotateVectorAboutArbitraryAxis( m_rightFoot->m_target.m_fwdDir, m_rightFoot->m_target.m_upDir, -1.0f );
		m_rightFoot->m_target.m_leftDir	= RotateVectorAboutArbitraryAxis( m_rightFoot->m_target.m_fwdDir, m_rightFoot->m_target.m_upDir, 90.0f );
		m_rightFoot->m_target.m_upDir		= RotateVectorAboutArbitraryAxis( m_rightFoot->m_target.m_fwdDir, m_rightFoot->m_target.m_leftDir, -90.0f );
	}
	if ( g_theInput->IsKeyDown( KEYCODE_DOWNARROW  ) )
	{
		// Rotate positive "Pitch" 
		m_rightFoot->m_target.m_fwdDir		= RotateVectorAboutArbitraryAxis( m_rightFoot->m_target.m_fwdDir, m_rightFoot->m_target.m_leftDir,   1.0f );
		m_rightFoot->m_target.m_upDir		= RotateVectorAboutArbitraryAxis( m_rightFoot->m_target.m_fwdDir, m_rightFoot->m_target.m_leftDir, -90.0f );
		m_rightFoot->m_target.m_leftDir	= RotateVectorAboutArbitraryAxis( m_rightFoot->m_target.m_fwdDir, m_rightFoot->m_target.m_upDir,    90.0f );
	}
	if ( g_theInput->IsKeyDown( KEYCODE_UPARROW    ) )
	{
		// Rotate negative "Pitch" 
		m_rightFoot->m_target.m_fwdDir		= RotateVectorAboutArbitraryAxis( m_rightFoot->m_target.m_fwdDir, m_rightFoot->m_target.m_leftDir,  -1.0f );
		m_rightFoot->m_target.m_upDir		= RotateVectorAboutArbitraryAxis( m_rightFoot->m_target.m_fwdDir, m_rightFoot->m_target.m_leftDir, -90.0f );
		m_rightFoot->m_target.m_leftDir	= RotateVectorAboutArbitraryAxis( m_rightFoot->m_target.m_fwdDir, m_rightFoot->m_target.m_upDir,    90.0f );
	}
	if ( g_theInput->IsKeyDown( KEYCODE_COMMA  ) )
	{
		// Rotate positive "Roll" 
		m_rightFoot->m_target.m_upDir		= RotateVectorAboutArbitraryAxis( m_rightFoot->m_target.m_upDir,   m_rightFoot->m_target.m_fwdDir, -1.0f );
		m_rightFoot->m_target.m_leftDir	= RotateVectorAboutArbitraryAxis( m_rightFoot->m_target.m_leftDir, m_rightFoot->m_target.m_fwdDir, -1.0f );
	}
	if ( g_theInput->IsKeyDown( KEYCODE_PERIOD ) )
	{
		// Rotate positive "Roll" 
		m_rightFoot->m_target.m_upDir		= RotateVectorAboutArbitraryAxis( m_rightFoot->m_target.m_upDir,   m_rightFoot->m_target.m_fwdDir,  1.0f );
		m_rightFoot->m_target.m_leftDir	= RotateVectorAboutArbitraryAxis( m_rightFoot->m_target.m_leftDir, m_rightFoot->m_target.m_fwdDir,  1.0f );
	}

	// Update Creature
	if ( g_debugFreeFly_F1 )
	{
		UpdateInputDebugPosEE();
	}
	else
	{
		DetermineBestWalkStepPos();
	}
	UpdateCreature();

	//----------------------------------------------------------------------------------------------------------------------
	// Update footstep by lerping with bezier curve
	//----------------------------------------------------------------------------------------------------------------------

	//----------------------------------------------------------------------------------------------------------------------
	// Update Bezier Curve "t"
	//----------------------------------------------------------------------------------------------------------------------
	if ( m_timer.HasDurationElapsed() )
	{
		m_timer.Stop();
	}
	else if ( m_timer.GetElapsedFraction() > 0.0f )
	{
		Vec3 bezierLerp							= m_bezierCurve.GetPointAtTime( m_timer.GetElapsedTime() );
		m_rightFoot->m_target.m_currentPos = bezierLerp;
		DebuggerPrintf( "not stopped\n" );
	}

	// Update Camera
	UpdateGameMode3DCamera();
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_MultipleEndEffectors::Render() const
{
	RenderWorldObjects();
	RenderUIObjects();
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_MultipleEndEffectors::Shutdown()
{
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_MultipleEndEffectors::UpdateDebugKeys()
{
	// Toggle common debug bools
	if ( g_theInput->WasKeyJustPressed( KEYCODE_F1 ) )
	{
		g_debugFreeFly_F1 = !g_debugFreeFly_F1;
	}
	if ( g_theInput->WasKeyJustPressed( KEYCODE_F2 ) )
	{
		g_debugRenderRaycast_F2 = !g_debugRenderRaycast_F2;
	}
	if ( g_theInput->WasKeyJustPressed( KEYCODE_F3 ) )
	{
		g_debugBasis_F3 = !g_debugBasis_F3;
	}
	if ( g_theInput->WasKeyJustPressed( KEYCODE_F4 ) )
	{
		g_debugText_F4 = !g_debugText_F4;
	}
	if ( g_theInput->WasKeyJustPressed( KEYCODE_F5 ) )
	{
		g_debugAngles_F5 = !g_debugAngles_F5;
	}

	// Control camera dist from player
	if ( g_theInput->IsKeyDown( '9' ) )
	{
		m_distCamAwayFromPlayer += 10.0f;
	}
	if ( g_theInput->IsKeyDown( '0' ) )
	{
		m_distCamAwayFromPlayer -= 10.0f;
	}
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_MultipleEndEffectors::UpdateCameraInput( float deltaSeconds )
{
	if ( g_debugFreeFly_F1 == false )
	{
		return;
	}

	Vec3 iBasis, jBasis, kBasis;
	m_gameMode3DWorldCamera.m_orientation.GetAsVectors_XFwd_YLeft_ZUp( iBasis, jBasis, kBasis );
	iBasis.z = 0.0f;
	jBasis.z = 0.0f;
	iBasis	 = iBasis.GetNormalized(); 
	jBasis	 = jBasis.GetNormalized(); 
	kBasis   = kBasis.GetNormalized();

	//----------------------------------------------------------------------------------------------------------------------
	// All directions are local
	//----------------------------------------------------------------------------------------------------------------------
	if ( g_theInput->IsKeyDown( KEYCODE_SHIFT ) )
	{
		m_currentSpeed = m_fasterSpeed;
	}
	if ( g_theInput->WasKeyJustReleased( KEYCODE_SHIFT ) )
	{
		m_currentSpeed = m_defaultSpeed;
	}
	// Forward
	if ( g_theInput->IsKeyDown( 'W' ) )
	{
		m_gameMode3DWorldCamera.m_position += ( iBasis * m_currentSpeed ) * deltaSeconds;
	}
	// Left
	if ( g_theInput->IsKeyDown( 'A' ) )
	{
		m_gameMode3DWorldCamera.m_position += ( jBasis * m_currentSpeed ) * deltaSeconds;
	}
	// Backwards
	if ( g_theInput->IsKeyDown( 'S' ) )
	{
		m_gameMode3DWorldCamera.m_position -= ( iBasis * m_currentSpeed ) * deltaSeconds;
	}
	// Right
	if ( g_theInput->IsKeyDown( 'D' ) )
	{
		m_gameMode3DWorldCamera.m_position -= ( jBasis * m_currentSpeed ) * deltaSeconds;
	}
	// Sky (+Z)
	if ( g_theInput->IsKeyDown( 'E' ) )
	{
//		m_gameMode3DWorldCamera.m_position += ( kBasis * m_currentSpeed ) * deltaSeconds;
		m_gameMode3DWorldCamera.m_position += ( Vec3( 0.0f, 0.0f, 1.0f ) * m_currentSpeed ) * deltaSeconds;
	}
	// Ground (-Z)
	if ( g_theInput->IsKeyDown( 'Q' ) )
	{
//		m_gameMode3DWorldCamera.m_position -= ( kBasis * m_currentSpeed ) * deltaSeconds;
		m_gameMode3DWorldCamera.m_position -= ( Vec3( 0.0f, 0.0f, 1.0f ) * m_currentSpeed ) * deltaSeconds;
	}

	m_gameMode3DWorldCamera.m_orientation.m_pitchDegrees = GetClamped( m_gameMode3DWorldCamera.m_orientation.m_pitchDegrees, -85.0f, 85.0f );
	m_gameMode3DWorldCamera.m_orientation.m_rollDegrees  = GetClamped(  m_gameMode3DWorldCamera.m_orientation.m_rollDegrees, -45.0f, 45.0f );
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_MultipleEndEffectors::UpdateGameMode3DCamera()
{
	//----------------------------------------------------------------------------------------------------------------------
	// Camera follows mouse
	//----------------------------------------------------------------------------------------------------------------------
	// Implement displacement.x to yaw and displacement.y to pitch
	//----------------------------------------------------------------------------------------------------------------------

	// Update WORLD camera as perspective
	Vec2 cursorClientDelta								  = g_theInput->GetCursorClientDelta();
	float mouseSpeed									  = 0.05f;
	float yaw											  = cursorClientDelta.x * mouseSpeed;
	float pitch											  = cursorClientDelta.y * mouseSpeed;
	m_gameMode3DWorldCamera.m_orientation.m_yawDegrees	 -= yaw;
	m_gameMode3DWorldCamera.m_orientation.m_pitchDegrees += pitch;
	
	m_gameMode3DWorldCamera.m_orientation.m_pitchDegrees = GetClamped( m_gameMode3DWorldCamera.m_orientation.m_pitchDegrees, -85.0f, 85.0f );

	if ( g_debugFreeFly_F1 )
	{
		// Set cameraPos roam "freely"
		m_gameMode3DWorldCamera.SetPerspectiveView( 2.0f, 60.0f, 0.1f, 10000.0f );
		m_gameMode3DWorldCamera.SetRenderBasis( Vec3(0.0f, 0.0f, 1.0f), Vec3(-1.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f) );
	}

	// Update UI camera
	m_gameMode3DUICamera.SetOrthoView( Vec2( 0.0f, 0.0f ), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y ) );		
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_MultipleEndEffectors::RenderWorldObjects() const
{
	//----------------------------------------------------------------------------------------------------------------------
	// Begin World Camera
	//----------------------------------------------------------------------------------------------------------------------
	g_theRenderer->BeginCamera( m_gameMode3DWorldCamera );

	// Initialize common render variables
	std::vector<Vertex_PCU> textVerts;
	std::vector<Vertex_PCU> verts;
	std::vector<Vertex_PCU> compassVerts;

	//----------------------------------------------------------------------------------------------------------------------
	// Render world compass
	//----------------------------------------------------------------------------------------------------------------------
	AddVertsForCompass( compassVerts, Vec3( 0.0, 0.0f, -1.0f ), 100.0f, 0.1f );

	//----------------------------------------------------------------------------------------------------------------------
	// "ProtoMode" title
	//----------------------------------------------------------------------------------------------------------------------
	Vec3 textOrigin = Vec3( 200.0f, 150.0f,  0.0f );
	Vec3 iBasis		= Vec3(	  0.0f,  -1.0f,  0.0f );
	Vec3 jBasis		= Vec3(	  0.0f,   0.0f,  1.0f );
	g_theApp->m_textFont->AddVertsForText3D( textVerts, textOrigin, iBasis, jBasis, 25.0f, "GameMode Multiple End Effectors 3D!", Rgba8::GREEN );

	//----------------------------------------------------------------------------------------------------------------------
	// Render Floor
	//----------------------------------------------------------------------------------------------------------------------
	AddVertsForAABB3D( verts, AABB3( -100.0f, -100.0f, -2.0f, 100.0f, 100.0f, -1.0f ), Rgba8::DARK_YELLOW );

	//----------------------------------------------------------------------------------------------------------------------
	// Render Creature
	//----------------------------------------------------------------------------------------------------------------------
//	RenderCreature( verts );


/*
	//----------------------------------------------------------------------------------------------------------------------
	// Test rendering for bezier curves
	//----------------------------------------------------------------------------------------------------------------------
	float elaspedTime = m_timer.GetElapsedTime();
	Vec3 bezierPosAtT = m_bezierCurve.GetPointAtTime( elaspedTime );
//	DebuggerPrintf( "%0.2f\n", elaspedTime );
	AddVertsForSphere3D( verts, bezierPosAtT, 2.0f, 8.0f, 16.0f, Rgba8::CYAN );
	AddVertsForSphere3D( verts, m_bezierCurve.m_jointPos_LS , 2.0f, 8.0f, 16.0f, Rgba8::RED	 );
	AddVertsForSphere3D( verts, m_bezierCurve.m_guidePos1, 2.0f, 8.0f, 16.0f, Rgba8::GREEN   );
	AddVertsForSphere3D( verts, m_bezierCurve.m_guidePos2, 2.0f, 8.0f, 16.0f, Rgba8::BLUE	 );
	AddVertsForSphere3D( verts, m_bezierCurve.m_endPos   , 2.0f, 8.0f, 16.0f, Rgba8::MAGENTA );
*/
	
	//----------------------------------------------------------------------------------------------------------------------
	// Debug draw should bend more
	//----------------------------------------------------------------------------------------------------------------------
//	AddVertsForArrow3D( verts, m_rightFoot->m_firstJoint->m_jointPos_LS, m_rightFoot->m_target.m_currentPos, 1.0f );
/*
*/

	//----------------------------------------------------------------------------------------------------------------------
	// Hand
	//----------------------------------------------------------------------------------------------------------------------
	m_hand->Render( verts, Rgba8::DARK_BLUE, Rgba8::WHITE );
	m_indexFinger->Render( verts, Rgba8::DARKER_GREEN, Rgba8::WHITE );
	m_indexFinger->RenderTarget_EE( verts );
	m_thumb->Render( verts, Rgba8::DARKER_GRAY, Rgba8::WHITE );
	m_thumb->RenderTarget_EE( verts );
	m_middleFinger->Render( verts, Rgba8::DARKER_GRAY, Rgba8::WHITE );
	m_middleFinger->RenderTarget_EE( verts );
	m_ringFinger->Render( verts, Rgba8::DARKER_GRAY, Rgba8::WHITE );
	m_ringFinger->RenderTarget_EE( verts );
	m_pinkyFinger->Render( verts, Rgba8::DARKER_GRAY, Rgba8::WHITE );
	m_pinkyFinger->RenderTarget_EE( verts );

	//----------------------------------------------------------------------------------------------------------------------
	// End world camera
	//----------------------------------------------------------------------------------------------------------------------
	g_theRenderer->EndCamera( m_gameMode3DWorldCamera );

	//----------------------------------------------------------------------------------------------------------------------
	// Draw calls 
	//----------------------------------------------------------------------------------------------------------------------
	// World objects
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->DrawVertexArray( static_cast<int>( verts.size() ), verts.data() );
	
	// Text objects
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( &g_theApp->m_textFont->GetTexture() );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->DrawVertexArray( static_cast<int>( textVerts.size() ), textVerts.data() );
	// Reset binded texture
	g_theRenderer->BindTexture( nullptr );

	// Compass 
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->DrawVertexArray( static_cast<int>( compassVerts.size() ), compassVerts.data() );

	DebugRenderWorld( m_gameMode3DWorldCamera );
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_MultipleEndEffectors::RenderUIObjects() const
{
	//----------------------------------------------------------------------------------------------------------------------
	// Begin UI Camera
	g_theRenderer->BeginCamera( m_gameMode3DUICamera );

	//----------------------------------------------------------------------------------------------------------------------
	// Get or Create font
	std::vector<Vertex_PCU> textVerts;
	float cellHeight	= 2.0f;
	AABB2 textbox1		= AABB2( Vec2( 0.0f, 0.0f ), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y - 1.0f ) );
	Vec2 alignment		= Vec2( 0.0f, 1.0f );
	
	std::string gameInfo;
	if ( g_debugFreeFly_F1 )
	{
		gameInfo = Stringf( "F1 (Toggle camera/player) Camera || Mode (F6/F7 for prev/next) || GameMode (3D)" );
	}						
	else
	{
		gameInfo = Stringf( "F1 (Toggle camera/player) Player || Mode (F6/F7 for prev/next) || GameMode (3D)" );
	}
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight, gameInfo, Rgba8::YELLOW, 0.75f, alignment, TextDrawMode::SHRINK_TO_FIT );

	// Initialize and set UI variables
	Vec2 timeAlignment					= Vec2( 1.0f, 1.0f  );

	Vec2 cameraPosAlignment				= Vec2( 0.0f, 0.97f );
	Vec2 cameraOrientationAlignment		= Vec2( 0.0f, 0.94f );
	Vec2 treeOrientationAlignment		= Vec2( 0.0f, 0.91f );

	Vec2 yprAlignment					= Vec2( 0.0f, 0.88f );

	Vec2 limb1StartPosAlignment			= Vec2( 0.0f, 0.86f );
	Vec2 limb1EndPosAlignment			= Vec2( 0.0f, 0.83f );
	Vec2 limb2StartPosAlignment			= Vec2( 0.0f, 0.80f );
	Vec2 limb2EndPosAlignment			= Vec2( 0.0f, 0.77f );
	Vec2 debugTreeTargetPosAlignment	= Vec2( 0.0f, 0.74f );
	Vec2 Limb1FwdAlignment				= Vec2( 0.0f, 0.71f );
	Vec2 Limb1LeftAlignment				= Vec2( 0.0f, 0.68f );
	Vec2 Limb1UpAlignment				= Vec2( 0.0f, 0.65f );

	// Core Values
	float fps							= 1.0f / g_theApp->m_gameClock.GetDeltaSeconds();
	float scale							= g_theApp->m_gameClock.GetTimeScale();
	std::string cameraPosText			= Stringf( "Cam position:           %0.2f, %0.2f, %0.2f",	m_gameMode3DWorldCamera.m_position.x,					m_gameMode3DWorldCamera.m_position.y,						m_gameMode3DWorldCamera.m_position.z );
	std::string cameraOrientationText	= Stringf( "Cam Orientation (YPR):  %0.2f, %0.2f, %0.2f",	m_gameMode3DWorldCamera.m_orientation.m_yawDegrees,		m_gameMode3DWorldCamera.m_orientation.m_pitchDegrees,		m_gameMode3DWorldCamera.m_orientation.m_rollDegrees );
	std::string timeText				= Stringf( "Time: %0.2f. FPS: %0.2f, Scale %0.2f.", g_theApp->m_gameClock.GetTotalSeconds(), fps, scale );


	// Trees Values
	std::string limb1StartPosText		= Stringf( "Limb1StartPos:     X: %0.2f, Y: %0.2f, Z: %0.2f\n",		 m_rightFoot->m_firstJoint->m_jointPos_LS.x,	   m_rightFoot->m_firstJoint->m_jointPos_LS.y,    m_rightFoot->m_firstJoint->m_jointPos_LS.z ).c_str();
	std::string limb1EndPosText			= Stringf( "Limb1EndPos:       X: %0.2f, Y: %0.2f, Z: %0.2f\n",		   m_rightFoot->m_firstJoint->m_endPos.x,	     m_rightFoot->m_firstJoint->m_endPos.y,      m_rightFoot->m_firstJoint->m_endPos.z ).c_str();
	std::string limb2StartPosText		= Stringf( "Limb2StartPos:     X: %0.2f, Y: %0.2f, Z: %0.2f\n",	   m_rightFoot->m_jointList[1]->m_jointPos_LS.x,	 m_rightFoot->m_jointList[1]->m_jointPos_LS.y,  m_rightFoot->m_jointList[1]->m_jointPos_LS.z ).c_str();
	std::string limb2EndPosText			= Stringf( "Limb2EndPos:       X: %0.2f, Y: %0.2f, Z: %0.2f\n",		 m_rightFoot->m_jointList[1]->m_endPos.x,	   m_rightFoot->m_jointList[1]->m_endPos.y,    m_rightFoot->m_jointList[1]->m_endPos.z ).c_str();
	std::string endEffectorPosText		= Stringf( "endEffectorPos:    X: %0.2f, Y: %0.2f, Z: %0.2f\n",			m_rightFoot->m_target.m_currentPos.x,    m_rightFoot->m_target.m_currentPos.y, m_rightFoot->m_target.m_currentPos.z ).c_str();
	std::string limb1FwdText			= Stringf( "limb1Fwd:          X: %0.2f, Y: %0.2f, Z: %0.2f\n",		   m_rightFoot->m_firstJoint->m_fwdDir.x,         m_rightFoot->m_firstJoint->m_fwdDir.y,	    m_rightFoot->m_firstJoint->m_fwdDir.z ).c_str();
	std::string limb1LeftText			= Stringf( "limb1Left:         X: %0.2f, Y: %0.2f, Z: %0.2f\n",		  m_rightFoot->m_firstJoint->m_leftDir.x,        m_rightFoot->m_firstJoint->m_leftDir.y,	   m_rightFoot->m_firstJoint->m_leftDir.z ).c_str();
	std::string limb1UpText				= Stringf( "limb1Up:           X: %0.2f, Y: %0.2f, Z: %0.2f\n",			m_rightFoot->m_firstJoint->m_upDir.x,          m_rightFoot->m_firstJoint->m_upDir.y,	     m_rightFoot->m_firstJoint->m_upDir.z ).c_str();

	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight, 			  cameraPosText, Rgba8::YELLOW, 0.75f,		      cameraPosAlignment, TextDrawMode::SHRINK_TO_FIT );
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight, 			 	   timeText, Rgba8::YELLOW, 0.75f,  			   timeAlignment, TextDrawMode::SHRINK_TO_FIT );	
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight,    cameraOrientationText, Rgba8::YELLOW, 0.75f,    cameraOrientationAlignment, TextDrawMode::SHRINK_TO_FIT );

	if ( g_debugText_F4 )
	{
		g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight,		    limb1StartPosText, Rgba8::YELLOW, 0.75f,	     limb1StartPosAlignment, TextDrawMode::SHRINK_TO_FIT );
		g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight,		      limb1EndPosText, Rgba8::YELLOW, 0.75f,	       limb1EndPosAlignment, TextDrawMode::SHRINK_TO_FIT );
		g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight,	        limb2StartPosText, Rgba8::YELLOW, 0.75f,	     limb2StartPosAlignment, TextDrawMode::SHRINK_TO_FIT );
		g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight,	          limb2EndPosText, Rgba8::YELLOW, 0.75f,	       limb2EndPosAlignment, TextDrawMode::SHRINK_TO_FIT );
		g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight,		   endEffectorPosText, Rgba8::CYAN,   0.75f,	debugTreeTargetPosAlignment, TextDrawMode::SHRINK_TO_FIT );
		g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight,	   	         limb1FwdText, Rgba8::YELLOW, 0.75f,			  Limb1FwdAlignment, TextDrawMode::SHRINK_TO_FIT );
		g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight,	   	        limb1LeftText, Rgba8::YELLOW, 0.75f,			 Limb1LeftAlignment, TextDrawMode::SHRINK_TO_FIT );
		g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight,	   	          limb1UpText, Rgba8::YELLOW, 0.75f,			   Limb1UpAlignment, TextDrawMode::SHRINK_TO_FIT );
	}	

	
	//----------------------------------------------------------------------------------------------------------------------
	// End UI Camera
	g_theRenderer->EndCamera( m_gameMode3DUICamera );

	//----------------------------------------------------------------------------------------------------------------------
	// Draw for UI camera text
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( &g_theApp->m_textFont->GetTexture() );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->DrawVertexArray( static_cast<int>( textVerts.size() ), textVerts.data() );
	g_theRenderer->BindTexture( nullptr );
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_MultipleEndEffectors::AddVertsForCompass( std::vector<Vertex_PCU>&compassVerts, Vec3 startPosition, float axisLength, float axisThickness ) const
{
	// Render stationary world compass
	// X
	Vec3 endPosX = startPosition + Vec3( axisLength, 0.0f, 0.0f );
	AddVertsForArrow3D( compassVerts, startPosition, endPosX, axisThickness, Rgba8::RED );
	// Y
	Vec3 endPosY = startPosition + Vec3( 0.0f, axisLength, 0.0f );
	AddVertsForArrow3D( compassVerts, startPosition, endPosY, axisThickness, Rgba8::GREEN );
	// Z
	Vec3 endPosZ = startPosition + Vec3( 0.0f, 0.0f, axisLength );
	AddVertsForArrow3D( compassVerts, startPosition, endPosZ, axisThickness, Rgba8::BLUE );
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_MultipleEndEffectors::UpdateInputDebugPosEE()
{
	float stepAmount = 1.0f;

	if ( g_theInput->WasKeyJustPressed( '1' ) )
	{
		m_fingerTarget = INDEX;
	}
	else if ( g_theInput->WasKeyJustPressed( '2' ) )
	{
		m_fingerTarget = MIDDLE;
	}
	else if ( g_theInput->WasKeyJustPressed( '3' ) )
	{
		m_fingerTarget = RING;
	}
	else if ( g_theInput->WasKeyJustPressed( '4' ) )
	{
		m_fingerTarget = PINKY;
	}

	// East (+X)
	if ( g_theInput->IsKeyDown( 'I' ) )
	{
		if ( m_fingerTarget == INDEX )
		{
			m_indexFinger->m_target.m_currentPos += Vec3( stepAmount, 0.0f, 0.0f );
		}
		else if ( m_fingerTarget == MIDDLE )
		{
			m_middleFinger->m_target.m_currentPos += Vec3( stepAmount, 0.0f, 0.0f );
		}
		else if ( m_fingerTarget == RING )
		{
			m_ringFinger->m_target.m_currentPos += Vec3( stepAmount, 0.0f, 0.0f );
		}
		else if ( m_fingerTarget == PINKY )
		{
			m_pinkyFinger->m_target.m_currentPos += Vec3( stepAmount, 0.0f, 0.0f );
		}
//		m_rightFoot->m_target.m_currentPos += Vec3( stepAmount, 0.0f, 0.0f );
	}
	// West (-X)
	if ( g_theInput->IsKeyDown( 'K' ) )
	{
		if ( m_fingerTarget == INDEX )
		{
			m_indexFinger->m_target.m_currentPos += Vec3( -stepAmount, 0.0f, 0.0f );
		}
		else if ( m_fingerTarget == MIDDLE )
		{
			m_middleFinger->m_target.m_currentPos += Vec3( -stepAmount, 0.0f, 0.0f );
		}
		else if ( m_fingerTarget == RING )
		{
			m_ringFinger->m_target.m_currentPos += Vec3( -stepAmount, 0.0f, 0.0f );
		}
		else if ( m_fingerTarget == PINKY )
		{
			m_pinkyFinger->m_target.m_currentPos += Vec3( -stepAmount, 0.0f, 0.0f );
		}
//		m_rightFoot->m_target.m_currentPos += Vec3( -stepAmount, 0.0f, 0.0f );
	}
	// North (+Y)
	if ( g_theInput->IsKeyDown( 'J' ) )
	{	
		if ( m_fingerTarget == INDEX )
		{
			m_indexFinger->m_target.m_currentPos += Vec3( 0.0f, stepAmount, 0.0f );
		}
		else if ( m_fingerTarget == MIDDLE )
		{
			m_middleFinger->m_target.m_currentPos += Vec3( 0.0f, stepAmount, 0.0f );
		}
		else if ( m_fingerTarget == RING )
		{
			m_ringFinger->m_target.m_currentPos += Vec3( 0.0f, stepAmount, 0.0f );
		}
		else if ( m_fingerTarget == PINKY )
		{
			m_pinkyFinger->m_target.m_currentPos += Vec3( 0.0f, stepAmount, 0.0f );
		}
//		m_rightFoot->m_target.m_currentPos += Vec3( 0.0f, stepAmount, 0.0f );
	}
	// South (-Y)
	if ( g_theInput->IsKeyDown( 'L' ) )
	{
		if ( m_fingerTarget == INDEX )
		{
			m_indexFinger->m_target.m_currentPos += Vec3( 0.0f, -stepAmount, 0.0f );
		}
		else if ( m_fingerTarget == MIDDLE )
		{
			m_middleFinger->m_target.m_currentPos += Vec3( 0.0f, -stepAmount, 0.0f );
		}
		else if ( m_fingerTarget == RING )
		{
			m_ringFinger->m_target.m_currentPos += Vec3( 0.0f, -stepAmount, 0.0f );
		}
		else if ( m_fingerTarget == PINKY )
		{
			m_pinkyFinger->m_target.m_currentPos += Vec3( 0.0f, -stepAmount, 0.0f );
		}
//		m_rightFoot->m_target.m_currentPos += Vec3( 0.0f, -stepAmount, 0.0f );
	}
	// Sky (+Z)
	if ( g_theInput->IsKeyDown( 'O' ) )
	{
		if ( m_fingerTarget == INDEX )
		{
			m_indexFinger->m_target.m_currentPos += Vec3( 0.0f, 0.0f, stepAmount );
		}
		else if ( m_fingerTarget == MIDDLE )
		{
			m_middleFinger->m_target.m_currentPos += Vec3( 0.0f, 0.0f, stepAmount );
		}
		else if ( m_fingerTarget == RING )
		{
			m_ringFinger->m_target.m_currentPos += Vec3( 0.0f, 0.0f, stepAmount );
		}
		else if ( m_fingerTarget == PINKY )
		{
			m_pinkyFinger->m_target.m_currentPos += Vec3( 0.0f, 0.0f, stepAmount );
		}
//		m_rightFoot->m_target.m_currentPos += Vec3( 0.0f, 0.0f, stepAmount );
	}
	// Ground (-Z)
	if ( g_theInput->IsKeyDown( 'U' ) )
	{
		if ( m_fingerTarget == INDEX )
		{
			m_indexFinger->m_target.m_currentPos += Vec3( 0.0f, 0.0f, -stepAmount );
		}
		else if ( m_fingerTarget == MIDDLE )
		{
			m_middleFinger->m_target.m_currentPos += Vec3( 0.0f, 0.0f, -stepAmount );
		}
		else if ( m_fingerTarget == RING )
		{
			m_ringFinger->m_target.m_currentPos += Vec3( 0.0f, 0.0f, -stepAmount );
		}
		else if ( m_fingerTarget == PINKY )
		{
			m_pinkyFinger->m_target.m_currentPos += Vec3( 0.0f, 0.0f, -stepAmount );
		}
//		m_rightFoot->m_target.m_currentPos += Vec3( 0.0f, 0.0f, -stepAmount );
	}

	//----------------------------------------------------------------------------------------------------------------------
	//----------------------------------------------------------------------------------------------------------------------
	
	// East (+X)
	if ( g_theInput->IsKeyDown( 'T' ) )
	{
		m_thumb->m_target.m_currentPos += Vec3( stepAmount, 0.0f, 0.0f );
	}
	// West (-X)
	if ( g_theInput->IsKeyDown( 'G' ) )
	{
	 	  m_thumb->m_target.m_currentPos += Vec3( -stepAmount, 0.0f, 0.0f );
	}
	// North (+Y)
	if ( g_theInput->IsKeyDown( 'F' ) )
	{
		m_thumb->m_target.m_currentPos += Vec3( 0.0f, stepAmount, 0.0f );
	}
	// South (-Y)
	if ( g_theInput->IsKeyDown( 'H' ) )
	{
		m_thumb->m_target.m_currentPos += Vec3( 0.0f, -stepAmount, 0.0f );
	}
	// Sky (+Z)
	if ( g_theInput->IsKeyDown( 'Y' ) )
	{
		m_thumb->m_target.m_currentPos += Vec3( 0.0f, 0.0f, stepAmount );
	}
	// Ground (-Z)
	if ( g_theInput->IsKeyDown( 'R' ) )
	{
		m_thumb->m_target.m_currentPos += Vec3( 0.0f, 0.0f, -stepAmount );
	}
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_MultipleEndEffectors::UpdateCreature()
{
	m_creature->Update();
	
	//----------------------------------------------------------------------------------------------------------------------
	// Update process for childChain1, childChain2, and parentChain 
	//----------------------------------------------------------------------------------------------------------------------
	// 1a. Set parent target as 'childChain1'
	m_hand->m_target.m_currentPos  = m_indexFinger->m_firstJoint->m_jointPos_LS;
	// 1b. Calling 'Update()' will solve FABRIK 
	// Drag childChain1 and the parent to target (Forwards)
	// Set parent back to startPos, all the way down to childChain1 (Backwards)
	m_indexFinger->Update();
	// 1d. Update childChain2 (Forwards to target, then Backwards to parentFinalEndPos)
	//----------------------------------------------------------------------------------------------------------------------
	m_thumb->m_position_WS			    = m_hand->m_finalJoint->m_endPos;
	m_thumb->Update();
	m_middleFinger->m_position_WS	    = m_hand->m_finalJoint->m_endPos;
	m_middleFinger->Update();
	m_ringFinger->m_position_WS	    = m_hand->m_finalJoint->m_endPos;
	m_ringFinger->Update();
	m_pinkyFinger->m_position_WS	    = m_hand->m_finalJoint->m_endPos;
	m_pinkyFinger->Update();
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_MultipleEndEffectors::InitializeTrees()
{
/*
	//----------------------------------------------------------------------------------------------------------------------
	// Trees
	//----------------------------------------------------------------------------------------------------------------------
	m_treeBranch1								= new IkChain( "Tree1", Vec3( 0.0f,  1.0f, 20.0f ) );
	m_treeBranch1->m_target.m_currentPos	= Vec3( 0.0f,  1.0f, 0.0f );

	//----------------------------------------------------------------------------------------------------------------------
	// Hack Code for hinge joint constraint testing
	m_debugTargetPos = Vec3( 0.0f, 0.0f, 0.0f );

	//----------------------------------------------------------------------------------------------------------------------
	// Tree 1
	//----------------------------------------------------------------------------------------------------------------------
	// Thigh
	m_treeBranch1->CreateNewLimb( m_limbLength, JOINT_CONSTRAINT_TYPE_BALL_AND_SOCKET );
	m_treeBranch1->m_firstJoint->m_fwdDir			= Vec3( 0.0f, 0.0f, -1.0f );
	m_treeBranch1->m_firstJoint->m_leftDir			= Vec3( 0.0f, 1.0f,  0.0f );
	m_treeBranch1->m_firstJoint->m_upDir				= Vec3( 1.0f, 0.0f,  0.0f );
	m_treeBranch1->m_firstJoint->m_axisOfRotation	= Vec3( 0.0f, 1.0f,  0.0f );
	m_treeBranch1->m_firstJoint->SetConstraints_YPR(  0.0f, 0.0f ),  0.0f, 0.0f ),  0.0f, 0.0f ) );
	// Knee
	m_treeBranch1->CreateNewLimb( m_limbLength, JOINT_CONSTRAINT_TYPE_HINGE_KNEE );
	m_treeBranch1->m_jointList[1]->SetConstraints_YPR(  0.0f, 135.0f ),  0.0f, 0.0f ),  0.0f, 0.0f ) );
	// Ankle
	m_treeBranch1->CreateNewLimb( m_halfLimbLength, JOINT_CONSTRAINT_TYPE_HINGE_KNEE );
	m_treeBranch1->m_jointList[2]->SetConstraints_YPR(  45.0f, 135.0f ),  0.0f, 0.0f ),  0.0f, 0.0f ) );
*/
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_MultipleEndEffectors::RenderCreature( std::vector<Vertex_PCU>& verts ) const
{
	m_creature->Render( verts, Rgba8::DARK_GREEN, Rgba8::BROWN, true );
	m_rightFoot->RenderTarget_EE( verts );
	m_rightFoot->RenderTarget_IJK( verts, 10.0f );
	m_creature->m_root->RenderIJK( verts, 2.0f );
	
	AddVertsForCylinder3D( verts, m_creature->m_root->m_jointPos_LS, m_rightFoot->m_position_WS, 0.5f, Rgba8::WHITE );
//	AddVertsForSphere3D( verts, m_rightFoot->m_jointList[ 1 ]->m_poleVector, 2.0f, 8.0f, 16.0f, Rgba8::MAGENTA );
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_MultipleEndEffectors::InitializeCreature()
{
	//----------------------------------------------------------------------------------------------------------------------
	// Creature
	//----------------------------------------------------------------------------------------------------------------------
	// Create creature root and arm
	m_creature   = new CreatureBase( Vec3( 0.0f, 0.0f, 0.0f ) );
	m_creature->CreateChildSkeletalSystem( "rightFoot", m_creature->m_root->m_jointPos_LS + Vec3( 10.0f, 0.0f, 0.0f ), nullptr, m_creature, true );
	m_rightFoot  = m_creature->GetSkeletonByName( "rightFoot" );
	m_rightFoot->m_target.m_currentPos = Vec3( 50.0f, 0.0f, 0.0f );	

	//----------------------------------------------------------------------------------------------------------------------
	// Right Foot
	//----------------------------------------------------------------------------------------------------------------------
	// 1. Create limbs for leg
	m_creature->CreateLimbsForIKChain( "rightFoot", 3, m_limbLength );
	
	// Save common variable
	m_maxLength								= m_creature->m_skeletalSystemsList[ 0 ]->GetMaxLengthOfSkeleton();
	m_rightFoot								= m_creature->GetSkeletonByName( "rightFoot" );
	m_rightFoot->m_firstJoint->m_isSubBase	= true;


	//----------------------------------------------------------------------------------------------------------------------
	//----------------------------------------------------------------------------------------------------------------------
	m_hand		= new IK_Chain3D( "hand1", Vec3::ZERO );
	m_hand->CreateNewLimbs( 10.0f, 2.0f, Vec3::X_FWD, true );
	//----------------------------------------------------------------------------------------------------------------------
	float fingerSegments = 2.0f;
	m_indexFinger	= new IK_Chain3D( "indexFinger", m_hand->m_finalJoint->m_endPos );
	m_indexFinger->CreateNewLimbs( 10.0f, fingerSegments, Vec3( 0.7f, -0.6f, 0.0f ) );
	m_indexFinger->CreateNewLimbs( 5.0f,				1, Vec3( 0.7f, -0.6f, 0.0f ) );
	m_hand->m_childChainList.push_back( m_indexFinger );
	m_indexFinger->m_target.m_currentPos = m_indexFinger->m_finalJoint->m_endPos;
	m_indexFinger->m_parentChain = m_hand;
	//----------------------------------------------------------------------------------------------------------------------
	m_middleFinger	= new IK_Chain3D( "middleFinger", m_hand->m_finalJoint->m_endPos );
	m_middleFinger->CreateNewLimbs( 10.0f, fingerSegments, Vec3( 0.7f, -0.3f, 0.0f ) );
	m_middleFinger->CreateNewLimbs(  5.0f,				 1, Vec3( 0.7f, -0.3f, 0.0f ) );
	m_hand->m_childChainList.push_back( m_middleFinger );
	m_middleFinger->m_target.m_currentPos = m_middleFinger->m_finalJoint->m_endPos;
	m_middleFinger->m_parentChain = m_hand;
	//----------------------------------------------------------------------------------------------------------------------
	m_ringFinger	= new IK_Chain3D( "ringFinger", m_hand->m_finalJoint->m_endPos );
	m_ringFinger->CreateNewLimbs(  10.0f, fingerSegments, Vec3::X_FWD );
	m_ringFinger->CreateNewLimbs(   5.0f,			   1, Vec3::X_FWD );
	m_hand->m_childChainList.push_back( m_ringFinger );
	m_ringFinger->m_target.m_currentPos = m_ringFinger->m_finalJoint->m_endPos;
	m_ringFinger->m_parentChain = m_hand;
	//----------------------------------------------------------------------------------------------------------------------
	m_pinkyFinger	= new IK_Chain3D( "pinkyFinger", m_hand->m_finalJoint->m_endPos );
	m_pinkyFinger->CreateNewLimbs(  10.0f, fingerSegments, Vec3( 0.7f, 0.7f, 0.0f ) );
	m_pinkyFinger->CreateNewLimbs(   5.0f,			    1, Vec3( 0.7f, 0.7f, 0.0f ) );
	m_hand->m_childChainList.push_back( m_pinkyFinger );
	m_pinkyFinger->m_target.m_currentPos = m_pinkyFinger->m_finalJoint->m_endPos;
	m_pinkyFinger->m_parentChain = m_hand;
	//----------------------------------------------------------------------------------------------------------------------
	m_thumb	= new IK_Chain3D( "thumb", m_hand->m_finalJoint->m_endPos );
	m_thumb->CreateNewLimbs(  5.0f, 2.0f, -Vec3::Y_LEFT );
	m_hand->m_childChainList.push_back( m_thumb );
	m_thumb->m_target.m_currentPos = m_thumb->m_finalJoint->m_endPos;
	m_thumb->m_parentChain = m_hand;

	//----------------------------------------------------------------------------------------------------------------------
	// 1a. Set child and parent pointers for parent chain and childChain1
//	m_chain_1->m_position			    = m_hand->m_finalJoint->m_endPos;
	m_indexFinger->m_firstJoint->m_parent	= m_hand->m_finalJoint;
	m_hand->m_finalJoint->m_child		= m_indexFinger->m_firstJoint;
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_MultipleEndEffectors::UpdateCreatureInput( float deltaSeconds )
{
	UNUSED( deltaSeconds );

	if ( g_debugFreeFly_F1 == true )
	{
		return;
	}

	Vec3 iBasis, jBasis, kBasis;
	m_gameMode3DWorldCamera.m_orientation.GetAsVectors_XFwd_YLeft_ZUp( iBasis, jBasis, kBasis );
	iBasis.z = 0.0f;
	jBasis.z = 0.0f;
	iBasis	 = iBasis.GetNormalized(); 
	jBasis	 = jBasis.GetNormalized(); 
	kBasis   = kBasis.GetNormalized();

	//----------------------------------------------------------------------------------------------------------------------
	// All directions are local
	//----------------------------------------------------------------------------------------------------------------------
	if ( g_theInput->IsKeyDown( KEYCODE_SHIFT ) )
	{
		m_currentSpeed		= m_fasterSpeed;
		m_goalWalkLerpSpeed	= m_sprintLerpSpeed;
	}
	if ( g_theInput->WasKeyJustReleased( KEYCODE_SHIFT ) )
	{
		m_currentSpeed		= m_defaultSpeed;
		m_goalWalkLerpSpeed	= m_walkLerpSpeed;
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Start Pos
	//----------------------------------------------------------------------------------------------------------------------
	// East (+X)
	if ( g_theInput->IsKeyDown( 'W' ) )
	{
		m_rightFoot->m_position_WS += Vec3( 1.0f, 0.0f, 0.0f );
	}
	// West (-X)
	if ( g_theInput->IsKeyDown( 'S' ) )
	{
		m_rightFoot->m_position_WS += Vec3( -1.0f, 0.0f, 0.0f );
	}
	// North (+Y)
	if ( g_theInput->IsKeyDown( 'A' ) )
	{
		m_rightFoot->m_position_WS += Vec3( 0.0f, 1.0f, 0.0f );
	}
	// South (-Y)
	if ( g_theInput->IsKeyDown( 'D' ) )
	{
		m_rightFoot->m_position_WS += Vec3( 0.0f, -1.0f, 0.0f );
	}
	// Sky (+Z)
	if ( g_theInput->IsKeyDown( 'E' ) )
	{
		m_rightFoot->m_position_WS += Vec3( 0.0f, 0.0f, 1.0f );
	}
	// Sky (-Z)
	if ( g_theInput->IsKeyDown( 'Q' ) )
	{
		m_rightFoot->m_position_WS += Vec3( 0.0f, 0.0f, -1.0f );
	}
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_MultipleEndEffectors::DetermineBestWalkStepPos()
{
	// The positions only needs to be updated for one frame (not every frame) if the distance from currentLimb pos to root is too far away
	// The Z height of the rayImpactPos always needs to be updated
	float halfArmLength		= m_maxLength * 0.2f;
//	float quarterArmLength	= m_maxLength * 0.3f;

	//----------------------------------------------------------------------------------------------------------------------
	// Right Foot
//	if ( IsLimbIsTooFarFromRoot( m_rightFoot, m_rightFoot->m_target.m_currentPos ) )
	if ( IsLimbIsTooFarFromRoot( m_rightFoot, m_rightFoot->m_target.m_goalPos ) )
	{	
		m_bezierCurve.m_startPos				= m_rightFoot->m_target.m_currentPos;
		m_bezierCurve.m_guidePos1				= m_rightFoot->m_target.m_currentPos + ( m_creature->m_root->m_fwdDir * halfArmLength ) + ( m_creature->m_root->m_upDir * halfArmLength );
		m_bezierCurve.m_guidePos2				= m_rightFoot->m_target.m_currentPos + ( m_creature->m_root->m_fwdDir * halfArmLength * 1.5f ) + ( m_creature->m_root->m_upDir * halfArmLength * 2.0f );
		m_bezierCurve.m_endPos					= m_rightFoot->m_target.m_currentPos + ( m_creature->m_root->m_fwdDir * halfArmLength * 2.0f );
		m_rightFoot->m_target.m_goalPos	= m_bezierCurve.m_endPos;
		m_timer.Start();
		DebuggerPrintf( "Start Timer\n" );
	}
}


//----------------------------------------------------------------------------------------------------------------------
bool GameMode_MultipleEndEffectors::IsLimbIsTooFarFromRoot( IK_Chain3D* currentLimb, Vec3 const& footTargetPos )
{
	// Check if limb is placed too far from Root
	float maxLimbLength		= ( currentLimb->m_jointList.size() * m_limbLength ) * 0.7f;
	float distFootPosToRoot = GetDistance3D( footTargetPos, currentLimb->m_position_WS );
	if ( distFootPosToRoot > maxLimbLength )
	{
		return true;
	}
	return false;
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_MultipleEndEffectors::SpecifyFootPlacementPos( Vec3& targetPos, float fwdStepAmount, float leftStepAmount )
{
	UNUSED( leftStepAmount );

	// Determine the ideal next step position
	targetPos = Vec3( m_creature->m_root->m_jointPos_LS.x, m_creature->m_root->m_jointPos_LS.y, 0.0f ) + ( m_creature->m_root->m_fwdDir * fwdStepAmount );
}
