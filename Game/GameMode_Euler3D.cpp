#include "Game/App.hpp"
#include "Game/GameMode_Euler3D.hpp"
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
GameMode_Euler3D::GameMode_Euler3D()
{
	//----------------------------------------------------------------------------------------------------------------------
	// Initialize Bezier Curve
	//----------------------------------------------------------------------------------------------------------------------
	m_timer.Restart();
}

//----------------------------------------------------------------------------------------------------------------------
GameMode_Euler3D::~GameMode_Euler3D()
{
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode_Euler3D::Startup()
{
	g_debugFreeFly_F1 = true;

	InitializeCreatures();
	InitializeTrees();

	//----------------------------------------------------------------------------------------------------------------------
	// Initialize Player cam pos and orientation
	//----------------------------------------------------------------------------------------------------------------------
	m_gameMode3DWorldCamera.m_position						= Vec3( 0.0f, 50.0f, 10.0f ); 
	m_gameMode3DWorldCamera.m_orientation.m_yawDegrees		= -90.0f;
	m_gameMode3DWorldCamera.m_orientation.m_pitchDegrees	=  15.0f;

	m_bezierCurve.m_startPos	= m_leftFoot->m_target.m_currentPos;
	m_bezierCurve.m_guidePos1	= Vec3( m_leftFoot->m_target.m_currentPos.x +  3.0f, m_leftFoot->m_target.m_currentPos.y, m_leftFoot->m_target.m_currentPos.z + 5.0f );
	m_bezierCurve.m_guidePos2	= Vec3( m_leftFoot->m_target.m_currentPos.x +  7.0f, m_leftFoot->m_target.m_currentPos.y, m_leftFoot->m_target.m_currentPos.z + 5.0f );
	m_bezierCurve.m_endPos		= Vec3( m_leftFoot->m_target.m_currentPos.x + 10.0f, m_leftFoot->m_target.m_currentPos.y, m_leftFoot->m_target.m_currentPos.z );
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode_Euler3D::Update( float deltaSeconds )
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
		m_leftFoot->m_target.m_fwdDir	= RotateVectorAboutArbitraryAxis( m_leftFoot->m_target.m_fwdDir, m_leftFoot->m_target.m_upDir,     1.0f );
		m_leftFoot->m_target.m_leftDir	= RotateVectorAboutArbitraryAxis( m_leftFoot->m_target.m_fwdDir, m_leftFoot->m_target.m_upDir,    90.0f );
		m_leftFoot->m_target.m_upDir	= RotateVectorAboutArbitraryAxis( m_leftFoot->m_target.m_fwdDir, m_leftFoot->m_target.m_leftDir, -90.0f );
	}
	if ( g_theInput->IsKeyDown( KEYCODE_RIGHTARROW ) )
	{
		// Rotate "yaw" CW
		m_leftFoot->m_target.m_fwdDir	= RotateVectorAboutArbitraryAxis( m_leftFoot->m_target.m_fwdDir, m_leftFoot->m_target.m_upDir, -1.0f );
		m_leftFoot->m_target.m_leftDir	= RotateVectorAboutArbitraryAxis( m_leftFoot->m_target.m_fwdDir, m_leftFoot->m_target.m_upDir, 90.0f );
		m_leftFoot->m_target.m_upDir	= RotateVectorAboutArbitraryAxis( m_leftFoot->m_target.m_fwdDir, m_leftFoot->m_target.m_leftDir, -90.0f );
	}
	if ( g_theInput->IsKeyDown( KEYCODE_DOWNARROW  ) )
	{
		// Rotate positive "Pitch" 
		m_leftFoot->m_target.m_fwdDir	= RotateVectorAboutArbitraryAxis( m_leftFoot->m_target.m_fwdDir, m_leftFoot->m_target.m_leftDir,   1.0f );
		m_leftFoot->m_target.m_upDir	= RotateVectorAboutArbitraryAxis( m_leftFoot->m_target.m_fwdDir, m_leftFoot->m_target.m_leftDir, -90.0f );
		m_leftFoot->m_target.m_leftDir	= RotateVectorAboutArbitraryAxis( m_leftFoot->m_target.m_fwdDir, m_leftFoot->m_target.m_upDir,    90.0f );
	}
	if ( g_theInput->IsKeyDown( KEYCODE_UPARROW    ) )
	{
		// Rotate negative "Pitch" 
		m_leftFoot->m_target.m_fwdDir	= RotateVectorAboutArbitraryAxis( m_leftFoot->m_target.m_fwdDir, m_leftFoot->m_target.m_leftDir,  -1.0f );
		m_leftFoot->m_target.m_upDir	= RotateVectorAboutArbitraryAxis( m_leftFoot->m_target.m_fwdDir, m_leftFoot->m_target.m_leftDir, -90.0f );
		m_leftFoot->m_target.m_leftDir	= RotateVectorAboutArbitraryAxis( m_leftFoot->m_target.m_fwdDir, m_leftFoot->m_target.m_upDir,    90.0f );
	}
	if ( g_theInput->IsKeyDown( KEYCODE_COMMA  ) )
	{
		// Rotate positive "Roll" 
		m_leftFoot->m_target.m_upDir	= RotateVectorAboutArbitraryAxis( m_leftFoot->m_target.m_upDir,   m_leftFoot->m_target.m_fwdDir, -1.0f );
		m_leftFoot->m_target.m_leftDir	= RotateVectorAboutArbitraryAxis( m_leftFoot->m_target.m_leftDir, m_leftFoot->m_target.m_fwdDir, -1.0f );
	}
	if ( g_theInput->IsKeyDown( KEYCODE_PERIOD ) )
	{
		// Rotate positive "Roll" 
		m_leftFoot->m_target.m_upDir	= RotateVectorAboutArbitraryAxis( m_leftFoot->m_target.m_upDir,   m_leftFoot->m_target.m_fwdDir,  1.0f );
		m_leftFoot->m_target.m_leftDir	= RotateVectorAboutArbitraryAxis( m_leftFoot->m_target.m_leftDir, m_leftFoot->m_target.m_fwdDir,  1.0f );
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
		m_leftFoot->m_target.m_currentPos = bezierLerp;
		DebuggerPrintf( "not stopped\n" );
	}

/*
	//----------------------------------------------------------------------------------------------------------------------
	// Update Quadruped
	//----------------------------------------------------------------------------------------------------------------------
	//----------------------------------------------------------------------------------------------------------------------
	// Update Quadruped Hip EndEffector
	//----------------------------------------------------------------------------------------------------------------------
	m_quadruped_bindPose->m_hip->m_target.m_currentPos = m_quadruped_bindPose->m_root->m_jointPos_LS;
	m_quadruped_bindPose->m_hip->m_target.m_fwdDir		= m_quadruped_bindPose->m_root->m_fwdDir;
	m_quadruped_bindPose->m_hip->m_target.m_leftDir	= m_quadruped_bindPose->m_root->m_leftDir;
	m_quadruped_bindPose->m_hip->m_target.m_upDir		= m_quadruped_bindPose->m_root->m_upDir;
	// Update limb EE
	Vec3 leftShoulder	= m_quadruped_bindPose->m_root->m_jointPos_LS + (  m_quadruped_bindPose->m_root->m_leftDir * m_quadruped_bindPose->m_offsetRootToHip_Biped );
	Vec3 rightShoulder	= m_quadruped_bindPose->m_root->m_jointPos_LS + ( -m_quadruped_bindPose->m_root->m_leftDir * m_quadruped_bindPose->m_offsetRootToHip_Biped );
	Vec3 leftHip		= m_quadruped_bindPose->m_hip->m_position  + (  m_quadruped_bindPose->m_hip->m_firstJoint->m_leftDir * m_quadruped_bindPose->m_offsetRootToHip_Biped );
	Vec3 rightHip		= m_quadruped_bindPose->m_hip->m_position  + ( -m_quadruped_bindPose->m_hip->m_firstJoint->m_leftDir * m_quadruped_bindPose->m_offsetRootToHip_Biped );

	m_quadruped_bindPose->m_leftArm->m_target.m_currentPos		= leftShoulder	- Vec3(0.0f, 0.0f, 20.0f);	
	m_quadruped_bindPose->m_rightArm->m_target.m_currentPos	= rightShoulder	- Vec3(0.0f, 0.0f, 20.0f);	
	m_quadruped_bindPose->m_leftFoot->m_target.m_currentPos	= leftHip		- Vec3(0.0f, 0.0f, 20.0f);		
	m_quadruped_bindPose->m_leftFoot->m_target.m_currentPos	= rightHip		- Vec3(0.0f, 0.0f, 20.0f);		
	
	m_quadruped_bindPose->Update();
*/

	// Update Camera
	UpdateGameMode3DCamera();
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_Euler3D::Render() const
{
	RenderWorldObjects();
	RenderUIObjects();
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_Euler3D::Shutdown()
{
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_Euler3D::UpdateDebugKeys()
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
	if ( g_theInput->WasKeyJustPressed( KEYCODE_F8 ) )
	{
		g_debugToggleConstraints_F8 = !g_debugToggleConstraints_F8;
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

	if ( g_theInput->WasKeyJustPressed( '1' ) )
	{
		m_leftFoot->m_target.m_currentPos = Vec3( -8.0f, 6.0f, -3.0f );
	}

	//----------------------------------------------------------------------------------------------------------------------
	if ( !g_debugToggleConstraints_F8 )
	{
		SetLeftFootConstraints();
	}
	else
	{
		for ( int i = 0; i < m_leftFoot->m_jointList.size(); i++ )
		{
			IK_Joint3D* currentSegment = m_leftFoot->m_jointList[i];
			currentSegment->SetConstraints_YPR();
		}
	}
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_Euler3D::UpdateCameraInput( float deltaSeconds )
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
void GameMode_Euler3D::UpdateGameMode3DCamera()
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
void GameMode_Euler3D::RenderWorldObjects() const
{
	//----------------------------------------------------------------------------------------------------------------------
	// Begin World Camera
	//----------------------------------------------------------------------------------------------------------------------
	g_theRenderer->BeginCamera( m_gameMode3DWorldCamera );

	// Initialize common render variables
	std::vector<Vertex_PCU> textVerts;
	std::vector<Vertex_PCU> verts;
	std::vector<Vertex_PCU> compassVerts;
	std::vector<Vertex_PCU> verts_BackFace;
	std::vector<Vertex_PCU> verts_testTexture;

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
	g_theApp->m_textFont->AddVertsForText3D( textVerts, textOrigin, iBasis, jBasis, 25.0f, "GameMode Euler 3D!", Rgba8::GREEN );

	//----------------------------------------------------------------------------------------------------------------------
	// Render Floor
	//----------------------------------------------------------------------------------------------------------------------
//	AddVertsForAABB3D( verts, AABB3( -100.0f, -100.0f, -2.0f, 100.0f, 100.0f, -1.0f ), Rgba8::DARK_YELLOW );

	//----------------------------------------------------------------------------------------------------------------------
	// Render Creature
	//----------------------------------------------------------------------------------------------------------------------
	RenderCreature( verts, verts_testTexture );

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
	// Debug draws
	//----------------------------------------------------------------------------------------------------------------------
	float textHeight = 0.4f;
	// Joint positions
	Vec3 left = m_gameMode3DWorldCamera.m_orientation.GetAsMatrix_XFwd_YLeft_ZUp().GetJBasis3D();
	Vec3 up	  = m_gameMode3DWorldCamera.m_orientation.GetAsMatrix_XFwd_YLeft_ZUp().GetKBasis3D();
	if ( g_debugAngles_F5 )
	{
		m_leftFoot->DebugDrawConstraints_YPR( verts_BackFace, 5.0f );
	}
	if ( g_debugText_F4 )
	{
		// Euler Constraints
//		m_leftFoot->DebugTextConstraints_YPR( textVerts, textHeight, 5.0f, 0.5f, left, up, g_theApp->m_textFont );
		m_leftFoot->DebugTextJointPos_WorldSpace( textVerts, textHeight, left, up, g_theApp->m_textFont );
		m_leftFoot->DebugDrawParentBasis( verts );
	}

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

	// Textured objects
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( g_theApp->m_texture_TestOpenGL );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->DrawVertexArray( static_cast<int>( verts_testTexture.size() ), verts_testTexture.data() );

	// Backface objects
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_NONE );
	g_theRenderer->DrawVertexArray( static_cast<int>( verts_BackFace.size() ), verts_BackFace.data() );
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	
	// Compass 
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->DrawVertexArray( static_cast<int>( compassVerts.size() ), compassVerts.data() );

	// Text objects
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( &g_theApp->m_textFont->GetTexture() );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->DrawVertexArray( static_cast<int>( textVerts.size() ), textVerts.data() );
	// Reset binded texture
	g_theRenderer->BindTexture( nullptr );
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_Euler3D::RenderUIObjects() const
{
	//----------------------------------------------------------------------------------------------------------------------
	// Begin UI Camera
	g_theRenderer->BeginCamera( m_gameMode3DUICamera );

	//----------------------------------------------------------------------------------------------------------------------
	// Get or Create font
	std::vector<Vertex_PCU> textVerts;
	float cellHeight		= 2.0f;
	AABB2 textbox1			= AABB2( Vec2( 0.0f, 0.0f ), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y - 1.0f ) );	
	std::string gameInfo	= Stringf( "F1 (Toggle camera/player) Player || Mode (F6/F7 for prev/next) || GameMode (3D)" );
	if ( g_debugFreeFly_F1 )
	{
		gameInfo = Stringf( "F1 (Toggle camera/player) Camera || Mode (F6/F7 for prev/next) || GameMode (3D)" );
	}
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight, gameInfo, Rgba8::YELLOW, 0.75f, Vec2( 0.0f, 1.0f ), TextDrawMode::SHRINK_TO_FIT );

	// Initialize and set UI variables
	Vec2 timeAlignment					= Vec2( 1.0f, 1.0f  );

	// Core Values
	float fps							= 1.0f / g_theApp->m_gameClock.GetDeltaSeconds();
	float scale							= g_theApp->m_gameClock.GetTimeScale();
	std::string cameraPosText			= Stringf( "Cam position:           %0.2f, %0.2f, %0.2f",	m_gameMode3DWorldCamera.m_position.x,					m_gameMode3DWorldCamera.m_position.y,						m_gameMode3DWorldCamera.m_position.z );
	std::string cameraOrientationText	= Stringf( "Cam Orientation (YPR):  %0.2f, %0.2f, %0.2f",	m_gameMode3DWorldCamera.m_orientation.m_yawDegrees,		m_gameMode3DWorldCamera.m_orientation.m_pitchDegrees,		m_gameMode3DWorldCamera.m_orientation.m_rollDegrees );
	std::string timeText				= Stringf( "Time: %0.2f. FPS: %0.2f, Scale %0.2f.", g_theApp->m_gameClock.GetTotalSeconds(), fps, scale );
	std::string stepModeFABRIK 			= Stringf( "Step mode: FABRIK\n").c_str();
	if ( m_leftFoot->m_isSingleStep_Debug )
	{
		stepModeFABRIK = Stringf( "Step mode: Debug\n").c_str();
	}
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight, stepModeFABRIK, Rgba8::YELLOW, 0.75f, Vec2( 0.0f, 0.97f ), TextDrawMode::SHRINK_TO_FIT );

	if ( g_debugText_F4 )
	{
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
void GameMode_Euler3D::AddVertsForCompass( std::vector<Vertex_PCU>&compassVerts, Vec3 startPosition, float axisLength, float axisThickness ) const
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
void GameMode_Euler3D::UpdateInputDebugPosEE()
{
	float stepAmount = 1.0f;

	// East (+X)
	if ( g_theInput->IsKeyDown( 'I' ) )
	{
		m_leftFoot->m_target.m_currentPos += Vec3( stepAmount, 0.0f, 0.0f );
	}
	// West (-X)
	if ( g_theInput->IsKeyDown( 'K' ) )
	{
		m_leftFoot->m_target.m_currentPos += Vec3( -stepAmount, 0.0f, 0.0f );
	}
	// North (+Y)
	if ( g_theInput->IsKeyDown( 'J' ) )
	{
		m_leftFoot->m_target.m_currentPos += Vec3( 0.0f, stepAmount, 0.0f );
	}
	// South (-Y)
	if ( g_theInput->IsKeyDown( 'L' ) )
	{
		m_leftFoot->m_target.m_currentPos += Vec3( 0.0f, -stepAmount, 0.0f );
	}
	// Sky (+Z)
	if ( g_theInput->IsKeyDown( 'O' ) )
	{
		m_leftFoot->m_target.m_currentPos += Vec3( 0.0f, 0.0f, stepAmount );
	}
	// Ground (-Z)
	if ( g_theInput->IsKeyDown( 'U' ) )
	{
		m_leftFoot->m_target.m_currentPos += Vec3( 0.0f, 0.0f, -stepAmount );
	}
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_Euler3D::UpdateCreature()
{
	// Debug single step
	if ( g_theInput->WasKeyJustPressed( KEYCODE_RIGHTBRACKET ) )
	{
		m_leftFoot->m_isSingleStep_Debug = !m_leftFoot->m_isSingleStep_Debug;
	}
	if ( m_leftFoot->m_isSingleStep_Debug )
	{
		if ( g_theInput->WasKeyJustPressed( KEYCODE_LEFTBRACKET ) )
		{
			UpdatePoleVector();
			m_creature->Update();
		}
	}
	else
	{
		UpdatePoleVector();
		m_creature->Update();
	}

}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_Euler3D::InitializeTrees()
{
/*
	//----------------------------------------------------------------------------------------------------------------------
	// Trees
	//----------------------------------------------------------------------------------------------------------------------
	m_treeBranch1								= new SkeletalSystem3D( "Tree1", Vec3( 0.0f,  1.0f, 20.0f ) );
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
void GameMode_Euler3D::RenderCreature( std::vector<Vertex_PCU>& verts, std::vector<Vertex_PCU>& verts_textured ) const
{
	m_creature->Render( verts_textured, Rgba8::DARK_GREEN, Rgba8::BROWN, true );
	m_leftFoot->RenderTarget_EE( verts );
	m_leftFoot->RenderTarget_IJK( verts, 10.0f );
	m_creature->m_root->RenderIJK( verts, 2.0f );
	
	AddVertsForCylinder3D( verts, m_creature->m_root->m_jointPos_LS,		m_leftFoot->m_position_WS, 0.5f, Rgba8::WHITE		);
	AddVertsForSphere3D	 ( verts, m_leftFoot->m_firstJoint->m_poleVector,			1.0f, 8.0f, 16.0f, Rgba8::MAGENTA	);
	AddVertsForSphere3D	 ( verts, m_leftFoot->m_firstJoint->m_child->m_poleVector,	1.0f, 8.0f, 16.0f, Rgba8::CYAN		);

	Vec3 end = m_leftFoot->m_firstJoint->m_jointPos_LS + m_leftFoot->m_debugVector * 2.0f;
	AddVertsForArrow3D( verts, m_leftFoot->m_firstJoint->m_jointPos_LS, end, 1.0f, Rgba8::MAGENTA );

//	AddVertsForSphere3D( verts, m_leftFoot->m_jointList[ 1 ]->m_poleVector, 2.0f, 8.0f, 16.0f, Rgba8::MAGENTA );

/*
	//----------------------------------------------------------------------------------------------------------------------
	// Render Quadruped
	//----------------------------------------------------------------------------------------------------------------------
	m_quadruped_bindPose ->Render( verts, Rgba8::WHITE, Rgba8::MAGENTA );
	// Hiding joint gaps 
	AddVertsForSphere3D( verts, m_quadruped_bindPose ->m_leftArm->m_firstJoint->m_jointPos_LS,   1.2f, 4.0f, 4.0f, Rgba8::WHITE );
	AddVertsForSphere3D( verts, m_quadruped_bindPose ->m_rightArm->m_firstJoint->m_jointPos_LS,  1.2f, 4.0f, 4.0f, Rgba8::WHITE );
	AddVertsForSphere3D( verts, m_quadruped_bindPose ->m_leftFoot->m_firstJoint->m_jointPos_LS,  1.2f, 4.0f, 4.0f, Rgba8::WHITE );
	AddVertsForSphere3D( verts, m_quadruped_bindPose ->m_leftFoot->m_firstJoint->m_jointPos_LS, 1.2f, 4.0f, 4.0f, Rgba8::WHITE );
	// Debug info
	if ( g_debugRenderIdealLimbPositions )
	{
		m_quadruped_bindPose ->m_root->RenderIJK( verts, 10.0f );
		m_quadruped_bindPose ->m_hip->DebugRenderAllJoints_IJK_Vectors( verts );
		AddVertsForSphere3D( verts,  m_quadruped_bindPose ->m_leftArm->m_target.m_currentPos, 1.0f, 4.0f, 4.0f, Rgba8::WHITE );
		AddVertsForSphere3D( verts, m_quadruped_bindPose ->m_rightArm->m_target.m_currentPos, 1.0f, 4.0f, 4.0f, Rgba8::GRAY  );
		m_quadruped_bindPose ->m_leftArm->RenderEndEffector_IJK_Vectors( verts, 4.0f );
		m_quadruped_bindPose ->m_rightArm->RenderEndEffector_IJK_Vectors( verts, 4.0f );
	}
	// Joint basis
	//    m_quadruped_bindPose ->m_rightArm->DebugRenderAllJoints_IJK_Vectors( verts );
	//	  m_quadruped_bindPose ->m_leftArm->DebugRenderAllJoints_IJK_Vectors ( verts );
	//	  m_quadruped_bindPose ->m_leftFoot->DebugRenderAllJoints_IJK_Vectors( verts );
	//	 m_quadruped_bindPose ->m_leftFoot->DebugRenderAllJoints_IJK_Vectors( verts );

	// root to shoulders
	AddVertsForCylinder3D( verts, m_quadruped_bindPose ->m_root->m_jointPos_LS, m_quadruped_bindPose ->m_leftArm->m_firstJoint->m_jointPos_LS,  1.0f );
	AddVertsForCylinder3D( verts, m_quadruped_bindPose ->m_root->m_jointPos_LS, m_quadruped_bindPose ->m_rightArm->m_firstJoint->m_jointPos_LS, 1.0f );
	// Hip cone constraints
	//	m_quadruped_bindPose ->m_hip->DebugRenderConeConstraints( verts, 2.0f, Rgba8::MAGENTA );

	// Hip to pelvis
	AddVertsForCylinder3D( verts, m_quadruped_bindPose ->m_hip->m_firstJoint->m_jointPos_LS, m_quadruped_bindPose ->m_leftFoot->m_position,  1.0f, Rgba8::ORANGE );
	AddVertsForCylinder3D( verts, m_quadruped_bindPose ->m_hip->m_firstJoint->m_jointPos_LS, m_quadruped_bindPose ->m_leftFoot->m_position, 1.0f, Rgba8::SUNSET_ORANGE );

/*
	if ( g_debugRenderStepBezier )
	{
		// Debug Render Bezier
		m_quadruped_bindPose ->DebugRenderBezier( verts, m_quadruped_bindPose ->m_bezier_leftArm,   m_timer_LeftArm   );
		m_quadruped_bindPose ->DebugRenderBezier( verts, m_quadruped_bindPose ->m_bezier_rightArm,  m_timer_RightArm  );
		m_quadruped_bindPose ->DebugRenderBezier( verts, m_quadruped_bindPose ->m_bezier_leftFoot,  m_timer_LeftFoot  );
		m_quadruped_bindPose ->DebugRenderBezier( verts, m_quadruped_bindPose ->m_bezier_leftFoot, m_timer_leftFoot );
	}
*/
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_Euler3D::InitializeCreatures()
{
	//----------------------------------------------------------------------------------------------------------------------
	// Creature
	//----------------------------------------------------------------------------------------------------------------------
	// Create creature root and arm
	m_creature   = new CreatureBase( Vec3( 0.0f, -5.0f, 0.0f ) );
	Vec3 leftHip = Vec3( 0.0f, 0.0f, 0.0f );
	m_creature->CreateChildSkeletalSystem( "leftFoot", leftHip, nullptr, m_creature, true );
	m_leftFoot   = m_creature->GetSkeletonByName( "leftFoot" );
	m_leftFoot->m_target.m_currentPos = Vec3( 20.0f, 0.0f, 0.0f );

	//----------------------------------------------------------------------------------------------------------------------
	// Left Foot
	//----------------------------------------------------------------------------------------------------------------------
	// 1. Create limbs for leg
 	// Testing (no constraints)
	m_creature->CreateLimbsForIKChain( "leftFoot", 6, m_limbLength, JOINT_CONSTRAINT_TYPE_EULER );		
 	// Actual parameters
	SetLeftFootConstraints();

	// Save common variable
	m_maxLength = m_creature->m_skeletalSystemsList[ 0 ]->GetMaxLengthOfSkeleton();
	Vec3 upNess								= m_leftFoot->m_firstJoint->m_jointPos_LS - m_leftFoot->m_firstJoint->m_upDir  * 10.0f;
	Vec3 fwdNess							= m_leftFoot->m_firstJoint->m_jointPos_LS + m_leftFoot->m_firstJoint->m_fwdDir *  5.0f;
	m_leftFoot->m_firstJoint->m_poleVector	= fwdNess + upNess;
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_Euler3D::SetLeftFootConstraints()
{
	for ( int i = 0; i < m_leftFoot->m_jointList.size(); i++ )
	{
		IK_Joint3D* currentSegment = m_leftFoot->m_jointList[ i ];
		currentSegment->SetConstraints_YPR( FloatRange( -75.0f, 75.0f ), FloatRange( -75.0f, 75.0f ), FloatRange( -0.0f,  0.0f ) );
	}

/*
	m_leftFoot->m_firstJoint->		  SetConstraints_YPR( FloatRange( -75.0f, 75.0f ), FloatRange( -180.0f, 180.0f ), FloatRange(  -0.0f,  0.0f ) );		// Thigh
	m_leftFoot->m_firstJoint->m_child->SetConstraints_YPR( FloatRange(  -0.0f,  0.0f ), FloatRange(   -0.0f, 150.0f ), FloatRange(  -0.0f,  0.0f ) );		// Knee
//	m_leftFoot->m_firstJoint->m_child->SetConstraints_YPR( FloatRange(  -0.0f,  0.0f ), FloatRange( -150.0f,   0.0f ), FloatRange(  -0.0f,  0.0f ) );		// Knee
	m_leftFoot->m_finalJoint->m_parent->SetConstraints_YPR( FloatRange(  -0.0f,  0.0f ), FloatRange( -150.0f,   0.0f ), FloatRange(  -0.0f,  0.0f ) );		// Ankle
	m_leftFoot->m_finalJoint->		  SetConstraints_YPR( FloatRange(  -0.0f,  0.0f ), FloatRange( -150.0f,   0.0f ), FloatRange(  -0.0f,  0.0f ) );		// Ankle
*/
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_Euler3D::UpdateCreatureInput( float deltaSeconds )
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
		m_leftFoot->m_position_WS += Vec3( 1.0f, 0.0f, 0.0f );
	}
	// West (-X)
	if ( g_theInput->IsKeyDown( 'S' ) )
	{
		m_leftFoot->m_position_WS += Vec3( -1.0f, 0.0f, 0.0f );
	}
	// North (+Y)
	if ( g_theInput->IsKeyDown( 'A' ) )
	{
		m_leftFoot->m_position_WS += Vec3( 0.0f, 1.0f, 0.0f );
	}
	// South (-Y)
	if ( g_theInput->IsKeyDown( 'D' ) )
	{
		m_leftFoot->m_position_WS += Vec3( 0.0f, -1.0f, 0.0f );
	}
	// Sky (+Z)
	if ( g_theInput->IsKeyDown( 'E' ) )
	{
		m_leftFoot->m_position_WS += Vec3( 0.0f, 0.0f, 1.0f );
	}
	// Sky (-Z)
	if ( g_theInput->IsKeyDown( 'Q' ) )
	{
		m_leftFoot->m_position_WS += Vec3( 0.0f, 0.0f, -1.0f );
	}
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_Euler3D::UpdatePoleVector()
{
	// Update pole vector every frame
	Vec3 upNess										= m_leftFoot->m_firstJoint->m_jointPos_LS + m_leftFoot->m_firstJoint->m_upDir  * 10.0f;
	Vec3 fwdNess									= m_leftFoot->m_firstJoint->m_jointPos_LS + m_leftFoot->m_firstJoint->m_fwdDir *  5.0f;
	m_leftFoot->m_firstJoint->m_poleVector			= fwdNess + upNess;

	upNess											= m_leftFoot->m_firstJoint->m_child->m_jointPos_LS + m_leftFoot->m_firstJoint->m_child->m_upDir  * 10.0f;
	fwdNess											= m_leftFoot->m_firstJoint->m_child->m_jointPos_LS + m_leftFoot->m_firstJoint->m_child->m_fwdDir *  5.0f;
	m_leftFoot->m_firstJoint->m_child->m_poleVector	= fwdNess + upNess;
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_Euler3D::DetermineBestWalkStepPos()
{
	// The positions only needs to be updated for one frame (not every frame) if the distance from currentLimb pos to root is too far away
	// The Z height of the rayImpactPos always needs to be updated
	float halfArmLength		= m_maxLength * 0.2f;
//	float quarterArmLength	= m_maxLength * 0.3f;

	//----------------------------------------------------------------------------------------------------------------------
	// Right Foot
//	if ( IsLimbIsTooFarFromRoot( m_leftFoot, m_leftFoot->m_target.m_currentPos ) )
	if ( IsLimbIsTooFarFromRoot( m_leftFoot, m_leftFoot->m_target.m_goalPos ) )
	{	
		m_bezierCurve.m_startPos				= m_leftFoot->m_target.m_currentPos;
		m_bezierCurve.m_guidePos1				= m_leftFoot->m_target.m_currentPos + ( m_creature->m_root->m_fwdDir * halfArmLength ) + ( m_creature->m_root->m_upDir * halfArmLength );
		m_bezierCurve.m_guidePos2				= m_leftFoot->m_target.m_currentPos + ( m_creature->m_root->m_fwdDir * halfArmLength * 1.5f ) + ( m_creature->m_root->m_upDir * halfArmLength * 2.0f );
		m_bezierCurve.m_endPos					= m_leftFoot->m_target.m_currentPos + ( m_creature->m_root->m_fwdDir * halfArmLength * 2.0f );
		m_leftFoot->m_target.m_goalPos	= m_bezierCurve.m_endPos;
		m_timer.Start();
		DebuggerPrintf( "Start Timer\n" );
	}
}


//----------------------------------------------------------------------------------------------------------------------
bool GameMode_Euler3D::IsLimbIsTooFarFromRoot( IK_Chain3D* currentLimb, Vec3 const& footTargetPos )
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
void GameMode_Euler3D::SpecifyFootPlacementPos( Vec3& targetPos, float fwdStepAmount, float leftStepAmount )
{
	UNUSED( leftStepAmount );

	// Determine the ideal next step position
	targetPos = Vec3( m_creature->m_root->m_jointPos_LS.x, m_creature->m_root->m_jointPos_LS.y, 0.0f ) + ( m_creature->m_root->m_fwdDir * fwdStepAmount );
}
