#include "Game/App.hpp"
#include "Game/GameMode_Constraints_3D.hpp"

#include "Engine/ThirdParty/Squirrel/Noise/SmoothNoise.hpp"
#include "Engine/SkeletalSystem/CreatureBase.hpp"
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

//----------------------------------------------------------------------------------------------------------------------
GameMode_Constraints_3D::GameMode_Constraints_3D()
{
}

//----------------------------------------------------------------------------------------------------------------------
GameMode_Constraints_3D::~GameMode_Constraints_3D()
{
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode_Constraints_3D::Startup()
{
	g_debugFreeFly_F1			= true;
	g_debugText_F4				= true;
	g_debugAngles_F5			= false;
	g_debugToggleConstraints_F8 = true;
	g_debugFollowTarget_F11		= false;

	InitializeIK_ChainCCD();

	m_gameMode3DWorldCamera.m_position						= Vec3( -30.0f, -30.0f, 20.0f ); 
	m_gameMode3DWorldCamera.m_orientation.m_yawDegrees		= 30.0f;
	m_gameMode3DWorldCamera.m_orientation.m_pitchDegrees	= 20.0f;
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode_Constraints_3D::Update( float deltaSeconds )
{	
	// Update core systems
	UpdatePauseQuitAndSlowMo();
	UpdateDebugKeys();
	UpdateCameraInput( deltaSeconds );

	// Update Trees
	UpdateTargetInput( deltaSeconds );
	m_creatureCCD->Update();

	float targetDegrees				= Atan2Degrees( m_debugTargetPos.y, m_debugTargetPos.x );
	m_debugTargetAngle_PolarCoords	= targetDegrees;
	m_debugTargetLength_PolarCoords	= sqrtf( (m_debugTargetPos.x * m_debugTargetPos.x) + (m_debugTargetPos.y * m_debugTargetPos.y) );

	// Update Camera
	UpdateGameMode3DCamera();
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_Constraints_3D::Render() const
{
	RenderWorldObjects();
	RenderUIObjects();
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_Constraints_3D::Shutdown()
{
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_Constraints_3D::UpdateDebugKeys()
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
		SetIK_ChainConstraints();
	}
	if ( g_theInput->WasKeyJustPressed( KEYCODE_F11 ) )
	{
		g_debugFollowTarget_F11 = !g_debugFollowTarget_F11;
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Camera debug controls
	//----------------------------------------------------------------------------------------------------------------------
	// Control camera dist from player
	if ( g_theInput->IsKeyDown( '9' ) )
	{
		m_distCamAwayFromPlayer += 10.0f;
	}
	if ( g_theInput->IsKeyDown( '0' ) )
	{
		m_distCamAwayFromPlayer -= 10.0f;
	}
	// Preset camera positions
	if ( g_theInput->WasKeyJustPressed( 'X' ) )
	{
		// Look down X-axis (YZ plane)
		m_gameMode3DWorldCamera.m_position	  = Vec3( 120.0f, 0.0f, 20.0f );
		m_gameMode3DWorldCamera.m_orientation = EulerAngles( 180.0f, 5.0f, 0.0f );
	}
	if ( g_theInput->WasKeyJustPressed( 'Y' ) )
	{
		// Look down Y-axis (XZ plane)
		m_gameMode3DWorldCamera.m_position	  = Vec3( 0.0f, 120.0f, 20.0f );
		m_gameMode3DWorldCamera.m_orientation = EulerAngles( -90.0f, 5.0f, 0.0f );
	}
	if ( g_theInput->WasKeyJustPressed( 'Z' ) )
	{
		// Look down Z-axis (XY plane)
		m_gameMode3DWorldCamera.m_position	  = Vec3( 2.0f, 0.0f, 120.0f );
		m_gameMode3DWorldCamera.m_orientation = EulerAngles( 0.0f, 89.9f, 0.0f );
	}

	if ( g_theInput->WasKeyJustPressed( 'R' ) )
	{
		 m_ikChain_CCD->ResetAllJointsEuler();
	}

}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_Constraints_3D::UpdateCameraInput( float deltaSeconds )
{
	Vec3 iBasis, jBasis, kBasis;
	m_gameMode3DWorldCamera.m_orientation.GetAsVectors_XFwd_YLeft_ZUp( iBasis, jBasis, kBasis );
	iBasis.z = 0.0f;
	jBasis.z = 0.0f;
	iBasis.Normalize();
	jBasis.Normalize();
	kBasis.Normalize();

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

	// Camera controls
	if ( g_debugFreeFly_F1 )
	{
		// Forward
		if ( g_theInput->IsKeyDown( 'W' ) )
		{
			m_gameMode3DWorldCamera.m_position += ( iBasis * m_currentSpeed );
		}
		// Left
		if ( g_theInput->IsKeyDown( 'A' ) )
		{
			m_gameMode3DWorldCamera.m_position += ( jBasis * m_currentSpeed );
		}
		// Backwards
		if ( g_theInput->IsKeyDown( 'S' ) )
		{
			m_gameMode3DWorldCamera.m_position -= ( iBasis * m_currentSpeed );
		}
		// Right
		if ( g_theInput->IsKeyDown( 'D' ) )
		{
			m_gameMode3DWorldCamera.m_position -= ( jBasis * m_currentSpeed );
		}
		// Sky (+Z)
		if ( g_theInput->IsKeyDown( 'E' ) )
		{
			m_gameMode3DWorldCamera.m_position += ( Vec3( 0.0f, 0.0f, 1.0f ) * m_currentSpeed );
		}
		// Ground (-Z)
		if ( g_theInput->IsKeyDown( 'Q' ) )
		{
			m_gameMode3DWorldCamera.m_position -= ( Vec3( 0.0f, 0.0f, 1.0f ) * m_currentSpeed );
		}
	}
	else   
	{
		//----------------------------------------------------------------------------------------------------------------------
		// Model controls (movement)
		//----------------------------------------------------------------------------------------------------------------------
		// Forward
		if ( g_theInput->IsKeyDown( 'W' ) )
		{
			m_ikChain_CCD->m_position_WS += ( Vec3::X_FWD * m_currentSpeed ) * deltaSeconds;
		}
		// Left
		if ( g_theInput->IsKeyDown( 'A' ) )
		{
			m_ikChain_CCD->m_position_WS += ( Vec3::Y_LEFT * m_currentSpeed ) * deltaSeconds;
		}
		// Backwards
		if ( g_theInput->IsKeyDown( 'S' ) )
		{
			m_ikChain_CCD->m_position_WS -= ( Vec3::X_FWD * m_currentSpeed ) * deltaSeconds;
		}
		// Right
		if ( g_theInput->IsKeyDown( 'D' ) )
		{
			m_ikChain_CCD->m_position_WS -= ( Vec3::Y_LEFT * m_currentSpeed ) * deltaSeconds;
		}
		// Sky (+Z)
		if ( g_theInput->IsKeyDown( 'E' ) )
		{
			m_ikChain_CCD->m_position_WS += ( Vec3::Z_UP * m_currentSpeed ) * deltaSeconds;
		}
		// Ground (-Z)
		if ( g_theInput->IsKeyDown( 'Q' ) )
		{
			m_ikChain_CCD->m_position_WS -= ( Vec3::Z_UP * m_currentSpeed ) * deltaSeconds;
		}
		//----------------------------------------------------------------------------------------------------------------------
		// Model controls (euler)
		//----------------------------------------------------------------------------------------------------------------------
		// +Yaw
		if ( g_theInput->IsKeyDown( 'V' ) )
		{
			m_ikChain_CCD->m_eulerAngles_WS.m_yawDegrees += m_currentSpeed * deltaSeconds;
		}
		// -Yaw
		if ( g_theInput->IsKeyDown( 'B' ) )
		{
			m_ikChain_CCD->m_eulerAngles_WS.m_yawDegrees -= m_currentSpeed * deltaSeconds;
		}
		// +Pitch
		if ( g_theInput->IsKeyDown( 'N' ) )
		{
			m_ikChain_CCD->m_eulerAngles_WS.m_pitchDegrees += m_currentSpeed * deltaSeconds;
		}
		// -Pitch
		if ( g_theInput->IsKeyDown( 'M' ) )
		{
			m_ikChain_CCD->m_eulerAngles_WS.m_pitchDegrees -= m_currentSpeed * deltaSeconds;
		}
		// +Roll
		if ( g_theInput->IsKeyDown( KEYCODE_COMMA ) )
		{
			m_ikChain_CCD->m_eulerAngles_WS.m_rollDegrees += m_currentSpeed * deltaSeconds;
		}
		// -Roll
		if ( g_theInput->IsKeyDown( KEYCODE_PERIOD ) )
		{
			m_ikChain_CCD->m_eulerAngles_WS.m_rollDegrees -= m_currentSpeed * deltaSeconds;
		}
	}


	m_gameMode3DWorldCamera.m_orientation.m_pitchDegrees = GetClamped( m_gameMode3DWorldCamera.m_orientation.m_pitchDegrees, -85.0f, 85.0f );
	m_gameMode3DWorldCamera.m_orientation.m_rollDegrees  = GetClamped( m_gameMode3DWorldCamera.m_orientation.m_rollDegrees,  -45.0f, 45.0f );
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_Constraints_3D::UpdateGameMode3DCamera()
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
void GameMode_Constraints_3D::RenderWorldObjects() const
{
	//----------------------------------------------------------------------------------------------------------------------
	// Begin World Camera
	//----------------------------------------------------------------------------------------------------------------------
	g_theRenderer->BeginCamera( m_gameMode3DWorldCamera );

	// Initialize common render variables
	std::vector<Vertex_PCU> textVerts;
	std::vector<Vertex_PCU> verts;
	std::vector<Vertex_PCU> creatureVerts;
	std::vector<Vertex_PCU> creatureVerts2;
	std::vector<Vertex_PCU> compassVerts;
	std::vector<Vertex_PCU> verts_NoBackfaceCull;
	std::vector<Vertex_PCU> verts_model;

	//----------------------------------------------------------------------------------------------------------------------
	// Render world compass
	//----------------------------------------------------------------------------------------------------------------------
	AddVertsForCompass( compassVerts, Vec3( 0.0, 0.0f, 0.0f ), 100.0f, 0.1f );

	//----------------------------------------------------------------------------------------------------------------------
	// "ProtoMode" title
	//----------------------------------------------------------------------------------------------------------------------
	Vec3 textOrigin = Vec3( 200.0f, 150.0f,  0.0f );
	Vec3 iBasis		= Vec3(	  0.0f,  -1.0f,  0.0f );
	Vec3 jBasis		= Vec3(	  0.0f,   0.0f,  1.0f );
	g_theApp->m_textFont->AddVertsForText3D( textVerts, textOrigin, iBasis, jBasis, 25.0f, "GameMode Constraints 3D!", Rgba8::GREEN );

	DebugRenderWorld( m_gameMode3DWorldCamera );

	//----------------------------------------------------------------------------------------------------------------------
	// Render CCD creature
	//----------------------------------------------------------------------------------------------------------------------
	m_creatureCCD->Render( creatureVerts, Rgba8::WHITE,  Rgba8::YELLOW );
	m_ikChain_CCD->DebugDrawTarget( verts, 0.5f, Rgba8::MAGENTA, true );
	m_ikChain_CCD->DebugDrawJoints_IJK( verts, 0.2f );
	
	float length = 1.0f;
	AddVertsForAABB3D( verts_model, AABB3( Vec3( -length, -length, -length ), Vec3( length, length, length ) ), Rgba8::BROWN ); 

	if ( g_debugText_F4 )
	{
		Vec3 left = m_gameMode3DWorldCamera.m_orientation.GetAsMatrix_XFwd_YLeft_ZUp().GetJBasis3D();
		Vec3 up	  = m_gameMode3DWorldCamera.m_orientation.GetAsMatrix_XFwd_YLeft_ZUp().GetKBasis3D();
		m_ikChain_CCD->DebugTextJointPos_LocalSpace( textVerts, 0.75f, left, up, g_theApp->m_textFont );
	}
	if ( g_debugAngles_F5 )
	{
		m_ikChain_CCD->DebugDrawConstraints_YPR( verts_NoBackfaceCull, 2.0f );
	}


/*
	//----------------------------------------------------------------------------------------------------------------------
	// Testing matrix transforms
	//----------------------------------------------------------------------------------------------------------------------
	// parent arrow
	Vec3 startParent			= Vec3( 10.0f, 0.0f, 0.0f );
	Vec3 endParent				= startParent + ( Vec3::X_FWD * 10.0f );
	AddVertsForArrow3D( verts, startParent, endParent, 1.0f );
	Mat44 parentMatrix;
	parentMatrix.SetTranslation3D( startParent );

	// child1 arrow
	Mat44 childMatrix_1;
	childMatrix_1.SetTranslation3D( Vec3(10.0f, 0.0f, 0.0f) ); 
	Mat44 childToParentMatrix	= parentMatrix;
	childToParentMatrix.Append( childMatrix_1 );
	Vec3 childStart_1			= childToParentMatrix.GetTranslation3D();
	Vec3 rootJBasis				= childToParentMatrix.GetJBasis3D();
	Vec3 childEnd_1				= childStart_1 + rootJBasis * 10.0f;
	AddVertsForArrow3D( verts, childStart_1, childEnd_1, 1.0f, Rgba8::RED );

//	Mat44 worldToChildMatrix = childToParentMatrix_1.GetOrthoNormalInverse();

	Mat44 childMatrix_2;
	childMatrix_2.SetTranslation3D( Vec3(10.0f, 0.0f, 0.0f) );
	Mat44 child2ToWorldMatrix;
	child2ToWorldMatrix = parentMatrix;


	Vec3  childStart_2	= childToParentMatrix.GetTranslation3D();
	Vec3  parentIBasis	= childToParentMatrix.GetIBasis3D();
	Vec3  childEnd_2	= childStart_2 + parentIBasis * 10.0f;
	AddVertsForArrow3D( verts, childStart_2, childEnd_2, 1.0f, Rgba8::BLUE );
//	Vec3 sphereCenter_localSpace = Vec3( 10.0f, 0.0f, 0.0f );

//	AddVertsForSphere3D( verts, , 1.0f, 8.0f, 8.0f, Rgba8::CYAN );
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

	// Creature verts
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->DrawVertexArray( static_cast<int>( creatureVerts.size() ), creatureVerts.data() );

	// IK chain world pos verts
	Mat44 modelMat = m_ikChain_CCD->m_eulerAngles_WS.GetAsMatrix_XFwd_YLeft_ZUp();
	modelMat.SetTranslation3D( m_ikChain_CCD->m_position_WS );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( modelMat );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->DrawVertexArray( static_cast<int>( verts_model.size() ), verts_model.data() );
	
	// Creature verts2
	creatureVerts2 = creatureVerts;
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	Mat44 mat;
	mat.SetIJK3D( -Vec3::X_FWD, -Vec3::Y_LEFT, Vec3::Z_UP );
	mat.SetTranslation3D( Vec3( -2.0f, 0.0f, 0.0f ) );
	g_theRenderer->SetModelConstants( mat );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
//	g_theRenderer->DrawVertexArray( static_cast<int>( creatureVerts2.size() ), creatureVerts2.data() );

	// World objects, no back face cull
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_NONE );
	g_theRenderer->DrawVertexArray( static_cast<int>( verts_NoBackfaceCull.size() ), verts_NoBackfaceCull.data() );
	// Reset cull mode to default
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	
	
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
void GameMode_Constraints_3D::RenderUIObjects() const
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

	// Core Values
	float fps							= 1.0f / g_theApp->m_gameClock.GetDeltaSeconds();
	float scale							= g_theApp->m_gameClock.GetTimeScale();
	std::string cameraPosText			= Stringf( "Cam position:           %0.2f, %0.2f, %0.2f",	m_gameMode3DWorldCamera.m_position.x,					m_gameMode3DWorldCamera.m_position.y,						m_gameMode3DWorldCamera.m_position.z );
	std::string cameraOrientationText	= Stringf( "Cam Orientation (YPR):  %0.2f, %0.2f, %0.2f",	m_gameMode3DWorldCamera.m_orientation.m_yawDegrees,		m_gameMode3DWorldCamera.m_orientation.m_pitchDegrees,		m_gameMode3DWorldCamera.m_orientation.m_rollDegrees );
	std::string timeText				= Stringf( "Time: %0.2f. FPS: %0.2f, Scale %0.2f.", g_theApp->m_gameClock.GetTotalSeconds(), fps, scale );

	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight, 			 	   timeText, Rgba8::YELLOW, 0.75f,  Vec2( 1.0f, 1.0f  ), TextDrawMode::SHRINK_TO_FIT );	
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight, 			  cameraPosText, Rgba8::YELLOW, 0.75f,	Vec2( 0.0f, 0.97f ), TextDrawMode::SHRINK_TO_FIT );
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight,    cameraOrientationText, Rgba8::YELLOW, 0.75f,  Vec2( 0.0f, 0.94f ), TextDrawMode::SHRINK_TO_FIT );

	if ( g_debugText_F4 )
	{
		// IK chain WS pos
		g_theApp->m_textFont->AddVertsForTextInBox2D(	textVerts, textbox1, cellHeight, 
														Stringf( "IK_Chain pos_WS: %0.2f, %0.2f, %0.2f", m_ikChain_CCD->m_position_WS.x, 
																										 m_ikChain_CCD->m_position_WS.y, 
																										 m_ikChain_CCD->m_position_WS.z ),
														Rgba8::GREEN, 0.75f, Vec2( 1.0f, 0.97f ), TextDrawMode::SHRINK_TO_FIT );

		// IK chain WS euler
		g_theApp->m_textFont->AddVertsForTextInBox2D(	textVerts, textbox1, cellHeight, 
														Stringf( "IK_Chain YPR_WS: %0.2f, %0.2f, %0.2f", m_ikChain_CCD->m_eulerAngles_WS.m_yawDegrees, 
																										 m_ikChain_CCD->m_eulerAngles_WS.m_pitchDegrees, 
																										 m_ikChain_CCD->m_eulerAngles_WS.m_rollDegrees ),
														Rgba8::GREEN, 0.75f, Vec2( 1.0f, 0.94f ), TextDrawMode::SHRINK_TO_FIT );

		//----------------------------------------------------------------------------------------------------------------------
		// Joint 0 (root)
		g_theApp->m_textFont->AddVertsForTextInBox2D(	textVerts, textbox1, cellHeight, 
														Stringf( "Joint0_YPR: %0.2f, %0.2f, %0.2f", m_ikChain_CCD->m_firstJoint->m_eulerAngles_LS.m_yawDegrees,
																									m_ikChain_CCD->m_firstJoint->m_eulerAngles_LS.m_pitchDegrees,
																									m_ikChain_CCD->m_firstJoint->m_eulerAngles_LS.m_rollDegrees ),	
														Rgba8::YELLOW, 0.75f, Vec2( 1.0f, 0.91f ), TextDrawMode::SHRINK_TO_FIT );
		//----------------------------------------------------------------------------------------------------------------------
		// Joint 1
		g_theApp->m_textFont->AddVertsForTextInBox2D(	textVerts, textbox1, cellHeight, 
														Stringf( "Joint1_YPR: %0.2f, %0.2f, %0.2f", m_ikChain_CCD->m_jointList[1]->m_eulerAngles_LS.m_yawDegrees,
																									m_ikChain_CCD->m_jointList[1]->m_eulerAngles_LS.m_pitchDegrees,
																									m_ikChain_CCD->m_jointList[1]->m_eulerAngles_LS.m_rollDegrees ),	
														Rgba8::YELLOW, 0.75f, Vec2( 1.0f, 0.88f ), TextDrawMode::SHRINK_TO_FIT );
		//----------------------------------------------------------------------------------------------------------------------
		// Joint 2
//		g_theApp->m_textFont->AddVertsForTextInBox2D(	textVerts, textbox1, cellHeight, 
//														Stringf( "Joint2_YPR: %0.2f, %0.2f, %0.2f", m_ikChain_CCD->m_jointList[2]->m_eulerAngles_LS.m_yawDegrees,
//																									m_ikChain_CCD->m_jointList[2]->m_eulerAngles_LS.m_pitchDegrees,
//																									m_ikChain_CCD->m_jointList[2]->m_eulerAngles_LS.m_rollDegrees ),	
//														Rgba8::YELLOW, 0.75f, Vec2( 1.0f, 0.85f ), TextDrawMode::SHRINK_TO_FIT );		
		
		// DistEeToTarget 
		g_theApp->m_textFont->AddVertsForTextInBox2D(	textVerts, textbox1, cellHeight, Stringf( "DistEeToTarget:   %0.2f", m_ikChain_CCD->m_distEeToTarget ),
														Rgba8::GREEN, 0.75f, Vec2( 1.0f, 0.82f ), TextDrawMode::SHRINK_TO_FIT );
		
		//----------------------------------------------------------------------------------------------------------------------
		// Debug noise values (sine, cos, perlin)
		g_theApp->m_textFont->AddVertsForTextInBox2D(	textVerts, textbox1, cellHeight, Stringf( "Sine:   %0.2f", m_debugSine			), Rgba8::CYAN, 0.75f, Vec2( 1.0f, 0.79f ), TextDrawMode::SHRINK_TO_FIT );
		g_theApp->m_textFont->AddVertsForTextInBox2D(	textVerts, textbox1, cellHeight, Stringf( "Cos:    %0.2f", m_debugCos			), Rgba8::CYAN, 0.75f, Vec2( 1.0f, 0.76f ), TextDrawMode::SHRINK_TO_FIT );
		g_theApp->m_textFont->AddVertsForTextInBox2D(	textVerts, textbox1, cellHeight, Stringf( "Perlin: %0.2f", m_debugPerlinNoise	), Rgba8::CYAN, 0.75f, Vec2( 1.0f, 0.73f ), TextDrawMode::SHRINK_TO_FIT );
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
void GameMode_Constraints_3D::AddVertsForCompass( std::vector<Vertex_PCU>&compassVerts, Vec3 startPosition, float axisLength, float axisThickness ) const
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
void GameMode_Constraints_3D::UpdateTargetInput( float deltaSeconds )
{
	if ( g_debugFollowTarget_F11 )
	{
		float time								= g_theApp->m_gameClock.GetTotalSeconds();
		float sine								= SinDegrees( time *  10.0f ) * 20.0f;
		float cos								= CosDegrees( time *  60.0f ) * 45.0f;
		float fasterCos							= CosDegrees( time * 120.0f ) * 45.0f;
		float perlinNoise						= Compute2dPerlinNoise( time * 0.5f, 0.0f ) * 65.0f;
		m_ikChain_CCD->m_target.m_currentPos	= Vec3( 0.0f + perlinNoise, 0.0f -fasterCos, 0.0f + cos );
//		m_ikChain_CCD->m_position_WS			= Vec3( 0.0f + perlinNoise, 0.0f -fasterCos, 0.0f + cos );
		m_debugSine								= sine;
		m_debugCos								= cos;
		m_debugPerlinNoise						= perlinNoise;
//		m_ikChain_CCD->m_target.m_currentPos.
	}
	else
	{
		float stepAmount = 30.0f;

		// East (+X)
		if ( g_theInput->IsKeyDown( 'I' ) )
		{
			m_ikChain_CCD->m_target.m_currentPos += Vec3( stepAmount, 0.0f, 0.0f ) * deltaSeconds;
		}
		// West (-X)
		if ( g_theInput->IsKeyDown( 'K' ) )
		{
			m_ikChain_CCD->m_target.m_currentPos += Vec3( -stepAmount, 0.0f, 0.0f ) * deltaSeconds;
		}
		// North (+Y)
		if ( g_theInput->IsKeyDown( 'J' ) )
		{
			m_ikChain_CCD->m_target.m_currentPos += Vec3( 0.0f, stepAmount, 0.0f ) * deltaSeconds;
		}
		// South (-Y)
		if ( g_theInput->IsKeyDown( 'L' ) )
		{
			m_ikChain_CCD->m_target.m_currentPos += Vec3( 0.0f, -stepAmount, 0.0f ) * deltaSeconds;
		}
		// Sky (+Z)
		if ( g_theInput->IsKeyDown( 'O' ) )
		{
			m_ikChain_CCD->m_target.m_currentPos += Vec3( 0.0f, 0.0f, stepAmount ) * deltaSeconds;
		}
		// Ground (-Z)
		if ( g_theInput->IsKeyDown( 'U' ) )
		{
			m_ikChain_CCD->m_target.m_currentPos += Vec3( 0.0f, 0.0f, -stepAmount ) * deltaSeconds;
		}

		// Preset positions for target
		// X
		if ( g_theInput->WasKeyJustPressed( '1' ) )
		{
			m_ikChain_CCD->m_target.m_currentPos = Vec3( 30.0f, 0.0f, 0.0f );
		}
		// Y
		if ( g_theInput->WasKeyJustPressed( '2' ) )
		{
			m_ikChain_CCD->m_target.m_currentPos = Vec3( 0.0f, 30.0f, 0.0f );
		}
		// Z
		if ( g_theInput->WasKeyJustPressed( '3' ) )
		{
			m_ikChain_CCD->m_target.m_currentPos = Vec3( 0.0f, 0.0f, 30.0f );
		}

	/*
		//----------------------------------------------------------------------------------------------------------------------
		// Rotate endEffector orientation
		//----------------------------------------------------------------------------------------------------------------------
		if ( g_theInput->IsKeyDown( KEYCODE_LEFTARROW ) )
		{
			// Rotate "yaw" CCW
			m_spine->m_target.m_fwdDir		= RotateVectorAboutArbitraryAxis( m_spine->m_target.m_fwdDir, m_spine->m_target.m_upDir,	  1.0f  );
			m_spine->m_target.m_leftDir	= RotateVectorAboutArbitraryAxis( m_spine->m_target.m_fwdDir, m_spine->m_target.m_upDir,	  90.0f );
			m_spine->m_target.m_upDir		= RotateVectorAboutArbitraryAxis( m_spine->m_target.m_fwdDir, m_spine->m_target.m_leftDir, -90.0f );
		}
		if ( g_theInput->IsKeyDown( KEYCODE_RIGHTARROW ) )
		{
			// Rotate "yaw" CW
			m_spine->m_target.m_fwdDir		= RotateVectorAboutArbitraryAxis( m_spine->m_target.m_fwdDir, m_spine->m_target.m_upDir,	  -1.0f );
			m_spine->m_target.m_leftDir	= RotateVectorAboutArbitraryAxis( m_spine->m_target.m_fwdDir, m_spine->m_target.m_upDir,	  90.0f );
			m_spine->m_target.m_upDir		= RotateVectorAboutArbitraryAxis( m_spine->m_target.m_fwdDir, m_spine->m_target.m_leftDir, -90.0f );
		}
		if ( g_theInput->IsKeyDown( KEYCODE_DOWNARROW ) )
		{
			// Rotate positive "Pitch" 
			m_spine->m_target.m_fwdDir		= RotateVectorAboutArbitraryAxis( m_spine->m_target.m_fwdDir, m_spine->m_target.m_leftDir,   1.0f );
			m_spine->m_target.m_upDir		= RotateVectorAboutArbitraryAxis( m_spine->m_target.m_fwdDir, m_spine->m_target.m_leftDir, -90.0f );
			m_spine->m_target.m_leftDir	= RotateVectorAboutArbitraryAxis( m_spine->m_target.m_fwdDir, m_spine->m_target.m_upDir,    90.0f );
		}
		if ( g_theInput->IsKeyDown( KEYCODE_UPARROW ) )
		{
			// Rotate negative "Pitch" 
			m_spine->m_target.m_fwdDir		= RotateVectorAboutArbitraryAxis( m_spine->m_target.m_fwdDir, m_spine->m_target.m_leftDir,  -1.0f );
			m_spine->m_target.m_upDir		= RotateVectorAboutArbitraryAxis( m_spine->m_target.m_fwdDir, m_spine->m_target.m_leftDir, -90.0f );
			m_spine->m_target.m_leftDir	= RotateVectorAboutArbitraryAxis( m_spine->m_target.m_fwdDir, m_spine->m_target.m_upDir,    90.0f );
		}
	*/
	}
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_Constraints_3D::InitializeIK_ChainCCD()
{
	//----------------------------------------------------------------------------------------------------------------------
	// Creature
	//----------------------------------------------------------------------------------------------------------------------
	// Create creature root and arm
	m_creatureCCD						 = new CreatureBase( Vec3( 0.0f, 0.0f, 0.0f ) );
	m_creatureCCD->CreateChildSkeletalSystem( "m_ikChain_CCD", Vec3::ZERO, nullptr, m_creatureCCD, true );
	m_ikChain_CCD						 = m_creatureCCD->GetSkeletonByName( "m_ikChain_CCD" );
	m_ikChain_CCD->m_solverType			 = CHAIN_SOLVER_CCD;
	m_ikChain_CCD->CreateNewJoint( Vec3(  0.0f, 0.0f, 0.0f ), EulerAngles() );		// Root
	m_ikChain_CCD->CreateNewJoint( Vec3( 10.0f, 0.0f, 0.0f ), EulerAngles() );		// Child 1
	m_ikChain_CCD->CreateNewJoint( Vec3( 10.0f, 0.0f, 0.0f ), EulerAngles() );		// Child 2
//	m_ikChain_CCD->CreateNewJoint( Vec3( 10.0f, 0.0f, 0.0f ), EulerAngles() );		// Child 3
	// Actual parameters
	SetIK_ChainConstraints();
	m_ikChain_CCD->m_target.m_currentPos = Vec3( 80.0f, 0.0f, 0.0f );

	// Test cases for moving the model in WS
//	m_ikChain_CCD->m_target.m_currentPos = Vec3( 10.0f, 50.0f, -15.0f );
//	m_ikChain_CCD->m_position_WS		 = Vec3( -10.0f, 50.0f, -15.0f );
//	m_ikChain_CCD->m_eulerAngles_WS		 = EulerAngles( 0.0f, 0.0f, 0.0f );

	//----------------------------------------------------------------------------------------------------------------------
	// Hack Code for hinge joint constraint testing
	m_debugTargetPos = Vec3( 0.0f, 0.0f, 0.0f );
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode_Constraints_3D::SetIK_ChainConstraints()
{
	if ( g_debugToggleConstraints_F8 )
	{
		for ( int i = 0; i < m_ikChain_CCD->m_jointList.size(); i++ )
		{
			IK_Joint3D* currentJoint = m_ikChain_CCD->m_jointList[ i ];
			currentJoint->SetConstraints_YPR();
		}
	}
	else
	{
		for ( int i = 0; i < m_ikChain_CCD->m_jointList.size(); i++ )
		{
			IK_Joint3D* currentJoint = m_ikChain_CCD->m_jointList[ i ];
			// evens
			int remainder = i % 2;
			if ( remainder == 0 )
			{
				currentJoint->SetConstraints_YPR( FloatRange( -75.0f, 75.0f ), FloatRange( 0.0f, 0.0f ), FloatRange( -0.0f, 0.0f ) );
			}
			// odds
			else
			{
				currentJoint->SetConstraints_YPR( FloatRange( 0.0f, 0.0f ), FloatRange( -75.0f, 75.0f ), FloatRange( -0.0f, 0.0f ) );
			}
//			currentJoint->SetConstraints_YPR( FloatRange( -75.0f, 75.0f ), FloatRange( -75.0f, 75.0f ), FloatRange( -0.0f, 0.0f ) );
		}
	}
}
