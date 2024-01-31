#include "Game/App.hpp"
#include "Game/GameMode_BipedWalkAnim_3D.hpp"
#include "Game/Quadruped.hpp"

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
GameMode_BipedWalkAnim_3D::GameMode_BipedWalkAnim_3D()
{
	//----------------------------------------------------------------------------------------------------------------------
	// Initialize Bezier Curve
	//----------------------------------------------------------------------------------------------------------------------
	m_timer.Restart();
}

//----------------------------------------------------------------------------------------------------------------------
GameMode_BipedWalkAnim_3D::~GameMode_BipedWalkAnim_3D()
{
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode_BipedWalkAnim_3D::Startup()
{
	g_debugFreeFly_F1 = true;

	InitializeCreatures();
	InitializeTrees();

	//----------------------------------------------------------------------------------------------------------------------
	// Initialize Player cam pos and orientaiton
	//----------------------------------------------------------------------------------------------------------------------
	m_gameMode3DWorldCamera.m_position						= Vec3( 0.0f, 50.0f, 10.0f ); 
	m_gameMode3DWorldCamera.m_orientation.m_yawDegrees		= -90.0f;
	m_gameMode3DWorldCamera.m_orientation.m_pitchDegrees	= 0.0f;

	m_bezierCurve.m_startPos	= m_rightFoot->m_target.m_currentPos;
	m_bezierCurve.m_guidePos1	= Vec3( m_rightFoot->m_target.m_currentPos.x +  3.0f, m_rightFoot->m_target.m_currentPos.y, m_rightFoot->m_target.m_currentPos.z + 5.0f );
	m_bezierCurve.m_guidePos2	= Vec3( m_rightFoot->m_target.m_currentPos.x +  7.0f, m_rightFoot->m_target.m_currentPos.y, m_rightFoot->m_target.m_currentPos.z + 5.0f );
	m_bezierCurve.m_endPos		= Vec3( m_rightFoot->m_target.m_currentPos.x + 10.0f, m_rightFoot->m_target.m_currentPos.y, m_rightFoot->m_target.m_currentPos.z );
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode_BipedWalkAnim_3D::Update( float deltaSeconds )
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

	//----------------------------------------------------------------------------------------------------------------------
	// Update Quadruped
	//----------------------------------------------------------------------------------------------------------------------
	//----------------------------------------------------------------------------------------------------------------------
	// Update Quadruped Hip EndEffector
	//----------------------------------------------------------------------------------------------------------------------
	m_quadruped_bindPose->m_hip->m_target.m_currentPos  = m_quadruped_bindPose->m_root->m_jointPos_LS;
	m_quadruped_bindPose->m_hip->m_target.m_fwdDir		= m_quadruped_bindPose->m_root->m_fwdDir;
	m_quadruped_bindPose->m_hip->m_target.m_leftDir		= m_quadruped_bindPose->m_root->m_leftDir;
	m_quadruped_bindPose->m_hip->m_target.m_upDir		= m_quadruped_bindPose->m_root->m_upDir;
	// Update limb EE
	Vec3 leftShoulder	= m_quadruped_bindPose->m_root->m_jointPos_LS + (  m_quadruped_bindPose->m_root->m_leftDir				 * m_quadruped_bindPose->m_offsetRootToHip_Biped );
	Vec3 rightShoulder	= m_quadruped_bindPose->m_root->m_jointPos_LS + ( -m_quadruped_bindPose->m_root->m_leftDir				 * m_quadruped_bindPose->m_offsetRootToHip_Biped );
	Vec3 leftHip		= m_quadruped_bindPose->m_hip->m_position_WS  + (  m_quadruped_bindPose->m_hip->m_firstJoint->m_leftDir * m_quadruped_bindPose->m_offsetRootToHip_Biped );
	Vec3 rightHip		= m_quadruped_bindPose->m_hip->m_position_WS  + ( -m_quadruped_bindPose->m_hip->m_firstJoint->m_leftDir * m_quadruped_bindPose->m_offsetRootToHip_Biped );

	m_quadruped_bindPose->m_leftArm->m_target.m_currentPos		= leftShoulder	- Vec3(0.0f, 0.0f, 20.0f);	
	m_quadruped_bindPose->m_rightArm->m_target.m_currentPos		= rightShoulder	- Vec3(0.0f, 0.0f, 20.0f);	
	m_quadruped_bindPose->m_leftFoot->m_target.m_currentPos		= leftHip		- Vec3(0.0f, 0.0f, 20.0f);		
	m_quadruped_bindPose->m_rightFoot->m_target.m_currentPos	= rightHip		- Vec3(0.0f, 0.0f, 20.0f);		
	
	m_quadruped_bindPose->Update();
	// Update Camera
	UpdateGameMode3DCamera();
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_BipedWalkAnim_3D::Render() const
{
	RenderWorldObjects();
	RenderUIObjects();
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_BipedWalkAnim_3D::Shutdown()
{
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_BipedWalkAnim_3D::UpdateDebugKeys()
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
void GameMode_BipedWalkAnim_3D::UpdateCameraInput( float deltaSeconds )
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
void GameMode_BipedWalkAnim_3D::UpdateGameMode3DCamera()
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
void GameMode_BipedWalkAnim_3D::RenderWorldObjects() const
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
	g_theApp->m_textFont->AddVertsForText3D( textVerts, textOrigin, iBasis, jBasis, 25.0f, "GameMode BipedWalkAnim 3D!", Rgba8::GREEN );

	//----------------------------------------------------------------------------------------------------------------------
	// Render Floor
	//----------------------------------------------------------------------------------------------------------------------
	AddVertsForAABB3D( verts, AABB3( -100.0f, -100.0f, -2.0f, 100.0f, 100.0f, -1.0f ), Rgba8::DARK_YELLOW );

	//----------------------------------------------------------------------------------------------------------------------
	// Render Creature
	//----------------------------------------------------------------------------------------------------------------------
	RenderCreature( verts );


	//----------------------------------------------------------------------------------------------------------------------
	// Test rendering for bezier curves
	//----------------------------------------------------------------------------------------------------------------------
	float elaspedTime = m_timer.GetElapsedTime();
	Vec3 bezierPosAtT = m_bezierCurve.GetPointAtTime( elaspedTime );
//	DebuggerPrintf( "%0.2f\n", elaspedTime );
	AddVertsForSphere3D( verts, bezierPosAtT, 2.0f, 8.0f, 16.0f, Rgba8::CYAN );

	AddVertsForSphere3D( verts, m_bezierCurve.m_startPos,  2.0f, 8.0f, 16.0f, Rgba8::RED	 );
	AddVertsForSphere3D( verts, m_bezierCurve.m_guidePos1, 2.0f, 8.0f, 16.0f, Rgba8::GREEN   );
	AddVertsForSphere3D( verts, m_bezierCurve.m_guidePos2, 2.0f, 8.0f, 16.0f, Rgba8::BLUE	 );
	AddVertsForSphere3D( verts, m_bezierCurve.m_endPos   , 2.0f, 8.0f, 16.0f, Rgba8::MAGENTA );
	
	//----------------------------------------------------------------------------------------------------------------------
	// Debug draw should bend more
	//----------------------------------------------------------------------------------------------------------------------
//	AddVertsForArrow3D( verts, m_rightFoot->m_firstJoint->m_jointPos_LS, m_rightFoot->m_target.m_currentPos, 1.0f );
/*
*/

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
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_BipedWalkAnim_3D::RenderUIObjects() const
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

/*
	// Trees Values
	std::string limb1StartPosText		= Stringf( "Limb1StartPos:     X: %0.2f, Y: %0.2f, Z: %0.2f\n",		 m_rightFoot->m_firstJoint->m_jointPos_LS.x,	   m_rightFoot->m_firstJoint->m_jointPos_LS.y,    m_rightFoot->m_firstJoint->m_jointPos_LS.z ).c_str();
	std::string limb1EndPosText			= Stringf( "Limb1EndPos:       X: %0.2f, Y: %0.2f, Z: %0.2f\n",		   m_rightFoot->m_firstJoint->m_endPos.x,	     m_rightFoot->m_firstJoint->m_endPos.y,      m_rightFoot->m_firstJoint->m_endPos.z ).c_str();
	std::string limb2StartPosText		= Stringf( "Limb2StartPos:     X: %0.2f, Y: %0.2f, Z: %0.2f\n",	   m_rightFoot->m_jointList[1]->m_jointPos_LS.x,	 m_rightFoot->m_jointList[1]->m_jointPos_LS.y,  m_rightFoot->m_jointList[1]->m_jointPos_LS.z ).c_str();
	std::string limb2EndPosText			= Stringf( "Limb2EndPos:       X: %0.2f, Y: %0.2f, Z: %0.2f\n",		 m_rightFoot->m_jointList[1]->m_endPos.x,	   m_rightFoot->m_jointList[1]->m_endPos.y,    m_rightFoot->m_jointList[1]->m_endPos.z ).c_str();
	std::string endEffectorPosText		= Stringf( "endEffectorPos:    X: %0.2f, Y: %0.2f, Z: %0.2f\n",	  m_rightFoot->m_target.m_currentPos.x,    m_rightFoot->m_target.m_currentPos.y, m_rightFoot->m_target.m_currentPos.z ).c_str();
	std::string limb1FwdText			= Stringf( "limb1Fwd:          X: %0.2f, Y: %0.2f, Z: %0.2f\n",		   m_rightFoot->m_firstJoint->m_fwdDir.x,         m_rightFoot->m_firstJoint->m_fwdDir.y,	    m_rightFoot->m_firstJoint->m_fwdDir.z ).c_str();
	std::string limb1LeftText			= Stringf( "limb1Left:         X: %0.2f, Y: %0.2f, Z: %0.2f\n",		  m_rightFoot->m_firstJoint->m_leftDir.x,        m_rightFoot->m_firstJoint->m_leftDir.y,	   m_rightFoot->m_firstJoint->m_leftDir.z ).c_str();
	std::string limb1UpText				= Stringf( "limb1Up:           X: %0.2f, Y: %0.2f, Z: %0.2f\n",			m_rightFoot->m_firstJoint->m_upDir.x,          m_rightFoot->m_firstJoint->m_upDir.y,	     m_rightFoot->m_firstJoint->m_upDir.z ).c_str();

	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight, 			  cameraPosText, Rgba8::YELLOW, 0.75f,		      cameraPosAlignment, TextDrawMode::SHRINK_TO_FIT );
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight, 			 	   timeText, Rgba8::YELLOW, 0.75f,  			   timeAlignment, TextDrawMode::SHRINK_TO_FIT );	
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight,    cameraOrientationText, Rgba8::YELLOW, 0.75f,    cameraOrientationAlignment, TextDrawMode::SHRINK_TO_FIT );

	if ( g_debugRenderText )
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
*/
	
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
void GameMode_BipedWalkAnim_3D::AddVertsForCompass( std::vector<Vertex_PCU>&compassVerts, Vec3 startPosition, float axisLength, float axisThickness ) const
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
void GameMode_BipedWalkAnim_3D::UpdateInputDebugPosEE()
{
	float stepAmount = 1.0f;

	// East (+X)
	if ( g_theInput->IsKeyDown( 'I' ) )
	{
		m_rightFoot->m_target.m_currentPos += Vec3( stepAmount, 0.0f, 0.0f );
	}
	// West (-X)
	if ( g_theInput->IsKeyDown( 'K' ) )
	{
		m_rightFoot->m_target.m_currentPos += Vec3( -stepAmount, 0.0f, 0.0f );
	}
	// North (+Y)
	if ( g_theInput->IsKeyDown( 'J' ) )
	{
		m_rightFoot->m_target.m_currentPos += Vec3( 0.0f, stepAmount, 0.0f );
	}
	// South (-Y)
	if ( g_theInput->IsKeyDown( 'L' ) )
	{
		m_rightFoot->m_target.m_currentPos += Vec3( 0.0f, -stepAmount, 0.0f );
	}
	// Sky (+Z)
	if ( g_theInput->IsKeyDown( 'O' ) )
	{
		m_rightFoot->m_target.m_currentPos += Vec3( 0.0f, 0.0f, stepAmount );
	}
	// Ground (-Z)
	if ( g_theInput->IsKeyDown( 'U' ) )
	{
		m_rightFoot->m_target.m_currentPos += Vec3( 0.0f, 0.0f, -stepAmount );
	}
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_BipedWalkAnim_3D::UpdateCreature()
{
	m_creature->Update();
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_BipedWalkAnim_3D::InitializeTrees()
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
	m_treeBranch1->m_firstJoint->SetConstraints_YPR( FloatRange( 0.0f, 0.0f ), FloatRange( 0.0f, 0.0f ), FloatRange( 0.0f, 0.0f ) );
	// Knee
	m_treeBranch1->CreateNewLimb( m_limbLength, JOINT_CONSTRAINT_TYPE_HINGE_KNEE );
	m_treeBranch1->m_jointList[1]->SetConstraints_YPR( FloatRange( 0.0f, 135.0f ), FloatRange( 0.0f, 0.0f ), FloatRange( 0.0f, 0.0f ) );
	// Ankle
	m_treeBranch1->CreateNewLimb( m_halfLimbLength, JOINT_CONSTRAINT_TYPE_HINGE_KNEE );
	m_treeBranch1->m_jointList[2]->SetConstraints_YPR( FloatRange( 45.0f, 135.0f ), FloatRange( 0.0f, 0.0f ), FloatRange( 0.0f, 0.0f ) );
*/
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_BipedWalkAnim_3D::RenderCreature( std::vector<Vertex_PCU>& verts ) const
{
	m_creature->Render( verts, Rgba8::DARK_GREEN, Rgba8::BROWN, true );
	m_rightFoot->RenderTarget_EE( verts );
	m_rightFoot->RenderTarget_IJK( verts, 10.0f );
	m_creature->m_root->RenderIJK( verts, 2.0f );
	
	AddVertsForCylinder3D( verts, m_creature->m_root->m_jointPos_LS, m_rightFoot->m_position_WS, 0.5f, Rgba8::WHITE );
//	AddVertsForSphere3D( verts, m_rightFoot->m_jointList[ 1 ]->m_poleVector, 2.0f, 8.0f, 16.0f, Rgba8::MAGENTA );


	//----------------------------------------------------------------------------------------------------------------------
	// Render Quadruped
	//----------------------------------------------------------------------------------------------------------------------
	m_quadruped_bindPose ->Render( verts, Rgba8::WHITE, Rgba8::MAGENTA );
	// Hiding joint gaps 
	AddVertsForSphere3D( verts, m_quadruped_bindPose ->m_leftArm->m_firstJoint->m_jointPos_LS,   1.2f, 4.0f, 4.0f, Rgba8::WHITE );
	AddVertsForSphere3D( verts, m_quadruped_bindPose ->m_rightArm->m_firstJoint->m_jointPos_LS,  1.2f, 4.0f, 4.0f, Rgba8::WHITE );
	AddVertsForSphere3D( verts, m_quadruped_bindPose ->m_leftFoot->m_firstJoint->m_jointPos_LS,  1.2f, 4.0f, 4.0f, Rgba8::WHITE );
	AddVertsForSphere3D( verts, m_quadruped_bindPose ->m_rightFoot->m_firstJoint->m_jointPos_LS, 1.2f, 4.0f, 4.0f, Rgba8::WHITE );
	// Debug info
	if ( g_debugBasis_F3 )
	{
		m_quadruped_bindPose->m_root->RenderIJK( verts, 10.0f );
		m_quadruped_bindPose->m_hip->DebugDrawJoints_IJK( verts );
		AddVertsForSphere3D( verts,  m_quadruped_bindPose ->m_leftArm->m_target.m_currentPos, 1.0f, 4.0f, 4.0f, Rgba8::WHITE );
		AddVertsForSphere3D( verts, m_quadruped_bindPose ->m_rightArm->m_target.m_currentPos, 1.0f, 4.0f, 4.0f, Rgba8::GRAY  );
		m_quadruped_bindPose->m_leftArm->RenderTarget_IJK( verts, 4.0f );
		m_quadruped_bindPose->m_rightArm->RenderTarget_IJK( verts, 4.0f );
	}
	// Joint basis
	//    m_quadruped_bindPose ->m_rightArm->DebugDrawJoints_IJK( verts );
	//	  m_quadruped_bindPose ->m_leftArm->DebugDrawJoints_IJK ( verts );
	//	  m_quadruped_bindPose ->m_leftFoot->DebugDrawJoints_IJK( verts );
	//	 m_quadruped_bindPose ->m_rightFoot->DebugDrawJoints_IJK( verts );

	// root to shoulders
	AddVertsForCylinder3D( verts, m_quadruped_bindPose ->m_root->m_jointPos_LS, m_quadruped_bindPose ->m_leftArm->m_firstJoint->m_jointPos_LS,  1.0f );
	AddVertsForCylinder3D( verts, m_quadruped_bindPose ->m_root->m_jointPos_LS, m_quadruped_bindPose ->m_rightArm->m_firstJoint->m_jointPos_LS, 1.0f );
	// Hip cone constraints
	//	m_quadruped_bindPose ->m_hip->DebugRenderConeConstraints( verts, 2.0f, Rgba8::MAGENTA );

	// Hip to pelvis
	AddVertsForCylinder3D( verts, m_quadruped_bindPose ->m_hip->m_firstJoint->m_jointPos_LS, m_quadruped_bindPose ->m_leftFoot->m_position_WS,  1.0f, Rgba8::ORANGE );
	AddVertsForCylinder3D( verts, m_quadruped_bindPose ->m_hip->m_firstJoint->m_jointPos_LS, m_quadruped_bindPose ->m_rightFoot->m_position_WS, 1.0f, Rgba8::SUNSET_ORANGE );

/*
	if ( g_debugRenderStepBezier )
	{
		// Debug Render Bezier
		m_quadruped_bindPose ->DebugRenderBezier( verts, m_quadruped_bindPose ->m_bezier_leftArm,   m_timer_LeftArm   );
		m_quadruped_bindPose ->DebugRenderBezier( verts, m_quadruped_bindPose ->m_bezier_rightArm,  m_timer_RightArm  );
		m_quadruped_bindPose ->DebugRenderBezier( verts, m_quadruped_bindPose ->m_bezier_leftFoot,  m_timer_LeftFoot  );
		m_quadruped_bindPose ->DebugRenderBezier( verts, m_quadruped_bindPose ->m_bezier_rightFoot, m_timer_RightFoot );
	}
*/
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_BipedWalkAnim_3D::InitializeCreatures()
{
	//----------------------------------------------------------------------------------------------------------------------
	// Creature
	//----------------------------------------------------------------------------------------------------------------------
	// Create creature root and arm
	m_creature   = new CreatureBase( Vec3( 0.0f, 0.0f, m_rootDefaultHeightZ ) );
	Vec3 leftHip = m_creature->m_root->m_jointPos_LS + ( m_creature->m_root->m_leftDir * 5.0f );
	m_creature->CreateChildSkeletalSystem( "rightFoot", leftHip, nullptr, m_creature, true );
	m_rightFoot  = m_creature->GetSkeletonByName( "rightFoot" );
	m_rightFoot->m_target.m_currentPos = Vec3( 0.0f, -1.0f, 0.0f );	

	//----------------------------------------------------------------------------------------------------------------------
	// Right Foot
	//----------------------------------------------------------------------------------------------------------------------
	// 1. Create limbs for leg
	m_creature->CreateLimbsForIKChain( "rightFoot", 1, m_limbLength,		JOINT_CONSTRAINT_TYPE_BALL_AND_SOCKET, FloatRange( -180.0f, 180.0f ), FloatRange( -180.0f, 180.0f ), FloatRange( -65.0f, 65.0f ) );		// Thigh
//	m_creature->CreateLimbsForIKChain( "rightFoot", 1, m_limbLength,		JOINT_CONSTRAINT_TYPE_BALL_AND_SOCKET, FloatRange( -180.0f, 180.0f ), FloatRange( -180.0f, 180.0f ) );		// Thigh
	m_creature->CreateLimbsForIKChain( "rightFoot", 1, m_limbLength,			 JOINT_CONSTRAINT_TYPE_HINGE_KNEE, FloatRange(    0.0f, 135.0f ), FloatRange(  -15.0f,  35.0f ) );		// Knee
//	m_creature->CreateLimbsForIKChain( "rightFoot", 1, m_halfLimbLength,		 JOINT_CONSTRAINT_TYPE_HINGE_KNEE, FloatRange(   20.0f,  75.0f ), FloatRange(    0.0f,   0.0f ) );		// Ankle
//	m_creature->CreateLimbsForIKChain( "rightFoot", 1, m_halfLimbLength, JOINT_CONSTRAINT_TYPE_BALL_AND_SOCKET, FloatRange(   22.0f,  22.0f ), FloatRange(    0.0f,	 0.0f ), FloatRange( 15.0f, 15.0f ) );

	// Save common variable
	m_maxLength = m_creature->m_skeletalSystemsList[ 0 ]->GetMaxLengthOfSkeleton();

	//----------------------------------------------------------------------------------------------------------------------
	// Init Quadruped
	//----------------------------------------------------------------------------------------------------------------------
	m_quadruped_bindPose = new Quadruped( this, Vec3(0.0f, 0.0f, 20.0f), m_limbLength );
	m_quadruped_bindPose->InitLimbs();

	m_quadruped_bindPose->m_root->m_jointPos_LS = Vec3( 0.0f, -50.0f, 20.0f );
	m_quadruped_bindPose->m_hip->m_firstJoint->m_jointPos_LS = m_quadruped_bindPose->m_root->m_jointPos_LS - Vec3( 20.0f, 0.0f, 0.0f );
	m_quadruped_bindPose->m_hip->m_finalJoint->m_jointPos_LS = m_quadruped_bindPose->m_root->m_jointPos_LS - Vec3(  4.0f, 0.0f, 0.0f );
	m_quadruped_bindPose->m_hip->m_finalJoint->m_endPos	 = m_quadruped_bindPose->m_root->m_jointPos_LS;

	int size = int( m_quadruped_bindPose->m_hip->m_jointList.size() );
	for ( int i = size - 1; i >= 0; i-- )
	{
		IK_Joint3D* currentLimb = m_quadruped_bindPose->m_hip->m_jointList[i];
		currentLimb->m_endPos	  = m_quadruped_bindPose->m_root->m_jointPos_LS - Vec3( ( ( size - i )  * 4.0f ), 0.0f, 0.0f );
		currentLimb->m_jointPos_LS	  = currentLimb->m_endPos - Vec3( 4.0f, 0.0f, 0.0f );
	}
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_BipedWalkAnim_3D::UpdateCreatureInput( float deltaSeconds )
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
void GameMode_BipedWalkAnim_3D::DetermineBestWalkStepPos()
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
		m_bezierCurve.m_startPos			= m_rightFoot->m_target.m_currentPos;
		m_bezierCurve.m_guidePos1			= m_rightFoot->m_target.m_currentPos + ( m_creature->m_root->m_fwdDir * halfArmLength ) + ( m_creature->m_root->m_upDir * halfArmLength );
		m_bezierCurve.m_guidePos2			= m_rightFoot->m_target.m_currentPos + ( m_creature->m_root->m_fwdDir * halfArmLength * 1.5f ) + ( m_creature->m_root->m_upDir * halfArmLength * 2.0f );
		m_bezierCurve.m_endPos				= m_rightFoot->m_target.m_currentPos + ( m_creature->m_root->m_fwdDir * halfArmLength * 2.0f );
		m_rightFoot->m_target.m_goalPos		= m_bezierCurve.m_endPos;
		m_timer.Start();
		DebuggerPrintf( "Start Timer\n" );
	}
}


//----------------------------------------------------------------------------------------------------------------------
bool GameMode_BipedWalkAnim_3D::IsLimbIsTooFarFromRoot( IK_Chain3D* currentLimb, Vec3 const& footTargetPos )
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
void GameMode_BipedWalkAnim_3D::SpecifyFootPlacementPos( Vec3& targetPos, float fwdStepAmount, float leftStepAmount )
{
	UNUSED( leftStepAmount );

	// Determine the ideal next step position
	targetPos = Vec3( m_creature->m_root->m_jointPos_LS.x, m_creature->m_root->m_jointPos_LS.y, 0.0f ) + ( m_creature->m_root->m_fwdDir * fwdStepAmount );
}
