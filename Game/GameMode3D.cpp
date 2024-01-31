#include "Game/App.hpp"
#include "Game/GameMode3D.hpp"
#include "Game/Map_GameMode3D.hpp"
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
GameMode3D::GameMode3D()
{
}

//----------------------------------------------------------------------------------------------------------------------
GameMode3D::~GameMode3D()
{
	m_root = nullptr;
	delete m_root;
	m_rightArm = nullptr;
	delete m_rightArm;
	delete m_quadruped;
	m_quadruped = nullptr;

	delete m_map;
	m_map = nullptr;
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode3D::Startup()
{
	g_debugFreeFly_F1 = false;

	//----------------------------------------------------------------------------------------------------------------------
	// Creature
	//----------------------------------------------------------------------------------------------------------------------
	InitializeCreature();
	
	//----------------------------------------------------------------------------------------------------------------------
	// Trees
	//----------------------------------------------------------------------------------------------------------------------
	InitializeTrees();

	//----------------------------------------------------------------------------------------------------------------------
	// WalkableObjects (Floors, environmental proxies)
	//----------------------------------------------------------------------------------------------------------------------
	InitializeEnvironment();

	//----------------------------------------------------------------------------------------------------------------------
	// InitializeWalk Logic
	//----------------------------------------------------------------------------------------------------------------------
	// Setting logic to enable anchoring and walk logic on startup
	m_rightArm->m_anchorState  = ANCHOR_STATE_FREE;
	m_rightFoot->m_anchorState = ANCHOR_STATE_FREE;
	SpecifyFootPlacementPos(  m_rightArm->m_target.m_goalPos,					    m_maxArmLength * 0.5f,  -m_maxArmLength * 0.25f );
	SpecifyFootPlacementPos(   m_leftArm->m_target.m_goalPos,					    m_maxArmLength * 0.5f,   m_maxArmLength * 0.25f );
	SpecifyFootPlacementPos( m_rightFoot->m_target.m_goalPos, m_hip->m_firstJoint, m_maxFeetLength * 0.5f, -m_maxFeetLength * 0.25f );
	SpecifyFootPlacementPos(  m_leftFoot->m_target.m_goalPos, m_hip->m_firstJoint, m_maxFeetLength * 0.5f,  m_maxFeetLength * 0.25f );

	//----------------------------------------------------------------------------------------------------------------------
	// Initialize Raycasts
	//----------------------------------------------------------------------------------------------------------------------
	m_raycast_rightArmDown.m_rayStartPos	= Vec3(   5.0f, 10.0f, 5.0f );
	m_raycast_rightArmDown.m_rayEndPos		= Vec3( -15.0f, 10.0f, 5.0f );
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode3D::Update( float deltaSeconds )
{	
	// Move "elevator" using sine
	float time			= float( GetCurrentTimeSeconds() );
	m_sine				= SinDegrees( time * 100.0f );
	Vec3 elevatorCenter = m_elevator_1->m_aabb3.GetCenter();
	m_elevator_1->m_aabb3.SetCenterXYZ( Vec3( elevatorCenter.x, elevatorCenter.y, elevatorCenter.z + m_sine ) );

	// Update core systems
	UpdatePauseQuitAndSlowMo();
	UpdateDebugKeys();
	UpdateCameraInput( deltaSeconds );
//	UpdateDebugTargetPosInput();
	
	// Update Creature
	UpdateCreatureRootPosInput_Climbing( deltaSeconds );
	UpdateCreatureRootPosInput_Walking(  deltaSeconds );


	//----------------------------------------------------------------------------------------------------------------------
	// Creature Mount
	//----------------------------------------------------------------------------------------------------------------------
	if ( m_shouldMount )
	{
		m_root->m_jointPos_LS = Interpolate( m_root->m_jointPos_LS, m_mountGoalPos, deltaSeconds * 8.0f );

		// Check when to stop lerping
		// If dist is close enough, set m_shouldMount back to false
		float heightOffset = 8.0f;
		float distRootToMountGoal = GetDistance3D( m_root->m_jointPos_LS, Vec3( m_mountGoalPos.x, m_mountGoalPos.y, m_mountGoalPos.z + heightOffset ) );
		if ( CompareIfFloatsAreEqual( distRootToMountGoal, 0.0f, 2.0f ) )
		{
			m_shouldMount = false;
		}
	}

//	MoveRaycastInput( deltaSeconds );				// Might be useful for debugging later
	UpdateRaycastResult3D();						// Raycasts
	UpdateDebugLimbGoalPositions();

	//----------------------------------------------------------------------------------------------------------------------
	// Sprinting lerp toggle
	//----------------------------------------------------------------------------------------------------------------------
	if ( m_isSprinting )
	{
		m_goalWalkLerpSpeed = Interpolate( m_goalWalkLerpSpeed, m_sprintLerpSpeed, deltaSeconds );
	}
	if ( m_isSprinting )
	{
		m_goalWalkLerpSpeed = Interpolate( m_goalWalkLerpSpeed, m_walkLerpSpeed, deltaSeconds );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Climbing logic 
	//----------------------------------------------------------------------------------------------------------------------
	if ( m_isClimbing ) 
	{
		// Execute climbing logic
			// Place hand on wall
			// Place another hand on wall
			// Repeat

		// Initialize common variables
//		float halfArmLength			= m_maxArmLength  * 0.5f;
		float quarterArmLength		= m_maxArmLength  * 0.25f;
//		float tenthArmLength		= m_maxArmLength  * 0.1f;
		float halfFeetLength		= m_maxFeetLength * 0.5f;
		float quarterFeetLength		= m_maxFeetLength * 0.25f;
		Vec3 kBasis					= Vec3( 0.0f, 0.0f, 1.0f );
		Vec3 jBasis					= m_moveFwdDir.GetRotatedAboutZDegrees( 90.0f );

		//----------------------------------------------------------------------------------------------------------------------
		// Place hand on wall
		//----------------------------------------------------------------------------------------------------------------------

		//----------------------------------------------------------------------------------------------------------------------
		// Right Arm
		if ( IsLimbIsTooFarFromRoot( m_rightArm, m_rightArm->m_target.m_currentPos ) )
		{
			m_rightArm->m_target.m_goalPos = m_updatedImpactPos_FWD + ( kBasis * quarterArmLength ) + ( jBasis * -quarterArmLength );
		}

		//----------------------------------------------------------------------------------------------------------------------
		// Left Arm
		if ( IsLimbIsTooFarFromRoot( m_leftArm, m_leftArm->m_target.m_currentPos ) )
		{
			m_leftArm->m_target.m_goalPos = m_updatedImpactPos_FWD + ( kBasis * quarterArmLength ) + ( jBasis * quarterArmLength );
		}

		//----------------------------------------------------------------------------------------------------------------------
		// Right Foot
		if ( IsLimbIsTooFarFromRoot( m_rightFoot, m_rightFoot->m_target.m_currentPos ) )
		{
			m_rightFoot->m_target.m_goalPos = m_updatedImpactPos_FWD - ( kBasis * halfFeetLength ) + ( jBasis * -quarterFeetLength );
		}

		//----------------------------------------------------------------------------------------------------------------------
		// Left Foot
		if ( IsLimbIsTooFarFromRoot( m_leftFoot, m_leftFoot->m_target.m_currentPos ) )
		{
			m_leftFoot->m_target.m_goalPos = m_updatedImpactPos_FWD - ( kBasis * halfFeetLength ) + ( jBasis * quarterFeetLength );
		}

		// Height should automatically update through the arm positions
//		DetermineBestClimbPos();

		//----------------------------------------------------------------------------------------------------------------------
		// Push body "out of wall"
	}
	else
	{		
		DetermineBestWalkStepPos();	
		DetermineBestSprintStepPos();
	}
	UpdateCreature( deltaSeconds );
	ToggleAnchorStates();
	
	// Update Trees
	UpdateDebugTreePosInput();
	UpdateTreesToReachOutToPlayer( deltaSeconds );
	UpdateTrees();

	//----------------------------------------------------------------------------------------------------------------------
	// Update Quadruped
	//----------------------------------------------------------------------------------------------------------------------
	m_quadruped->UpdateLimbs( deltaSeconds );
	m_quadruped->Update();

	//----------------------------------------------------------------------------------------------------------------------
	// Update quad creature Bezier Curve
	//----------------------------------------------------------------------------------------------------------------------
	// Right Arm
	UpdateBezier( m_bezierCurve_RightArm,   m_rightArm, m_timer_RightArm	);
	UpdateBezier( m_bezierCurve_LeftArm,	 m_leftArm, m_timer_LeftArm		);
	UpdateBezier( m_bezierCurve_RightFoot, m_rightFoot, m_timer_RightFoot	);
	UpdateBezier( m_bezierCurve_LeftFoot,	m_leftFoot, m_timer_LeftFoot	);

	m_map->Update();

	// Update Camera
	UpdateGameMode3DCamera();
	TurnCreatureTowardsCameraDir();
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode3D::Render() const
{
	RenderWorldObjects();
	RenderUIObjects();
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode3D::Shutdown()
{
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode3D::UpdateDebugKeys()
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
	if ( g_theInput->WasKeyJustPressed( '1' ) )
	{
		g_debugRenderBezier_1 = !g_debugRenderBezier_1;
	}
	if ( g_theInput->WasKeyJustPressed( '2' ) )
	{
		g_debugToggleLegs_2 = !g_debugToggleLegs_2;
	}

	// Control Elevator
	// Sky (+Z)
	if ( g_theInput->IsKeyDown( 'O' ) )
	{
		Vec3 newPos = m_elevator_2->m_aabb3.GetCenter() + Vec3( 0.0f, 0.0f, 1.0f );
		m_elevator_2->m_aabb3.SetCenterXYZ( newPos );
	}
	// Ground (-Z)
	if ( g_theInput->IsKeyDown( 'U' ) )
	{
		Vec3 newPos = m_elevator_2->m_aabb3.GetCenter() + Vec3( 0.0f, 0.0f, -1.0f );
		m_elevator_2->m_aabb3.SetCenterXYZ( newPos );
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
void GameMode3D::UpdateCameraInput( float deltaSeconds )
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
		m_gameMode3DWorldCamera.m_position += ( Vec3( 0.0f, 0.0f, 1.0f ) * m_currentSpeed ) * deltaSeconds;
	}
	// Ground (-Z)
	if ( g_theInput->IsKeyDown( 'Q' ) )
	{
		m_gameMode3DWorldCamera.m_position -= ( Vec3( 0.0f, 0.0f, 1.0f ) * m_currentSpeed ) * deltaSeconds;
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Rotations
	//----------------------------------------------------------------------------------------------------------------------
	// Yaw (-Z, X->Y)
	if ( g_theInput->IsKeyDown( KEYCODE_LEFTARROW ) )
	{
		m_gameMode3DWorldCamera.m_orientation.m_yawDegrees += m_currentSpeed * deltaSeconds;
	}
	// Yaw (-Z, Y->X)
	if ( g_theInput->IsKeyDown( KEYCODE_RIGHTARROW ) )
	{
		m_gameMode3DWorldCamera.m_orientation.m_yawDegrees -= m_currentSpeed * deltaSeconds;
	}
	// Pitch (+Y, Z->X)
	if ( g_theInput->IsKeyDown( KEYCODE_UPARROW ) )
	{
		m_gameMode3DWorldCamera.m_orientation.m_pitchDegrees += m_currentSpeed * deltaSeconds;
	}
	// Pitch (-Y, X->Z)
	if ( g_theInput->IsKeyDown( KEYCODE_DOWNARROW) )
	{
		m_gameMode3DWorldCamera.m_orientation.m_pitchDegrees -= m_currentSpeed * deltaSeconds;
	}
	// Roll (+X, Y->Z)
	if ( g_theInput->IsKeyDown( 'C' ) )
	{
		m_gameMode3DWorldCamera.m_orientation.m_rollDegrees += m_currentSpeed  * deltaSeconds;
	}
	// Roll (-X, Z->Y)
	if ( g_theInput->IsKeyDown( 'Z' ) )
	{
		m_gameMode3DWorldCamera.m_orientation.m_rollDegrees -= m_currentSpeed * deltaSeconds;
	}
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode3D::UpdateGameMode3DCamera()
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
//	m_gameMode3DWorldCamera.SetTransform( m_gameMode3DWorldCamera.m_position, m_gameMode3DWorldCamera.m_orientation );
	
	m_gameMode3DWorldCamera.m_orientation.m_pitchDegrees = GetClamped( m_gameMode3DWorldCamera.m_orientation.m_pitchDegrees, -85.0f, 85.0f );
	m_gameMode3DWorldCamera.m_orientation.m_rollDegrees  = GetClamped(  m_gameMode3DWorldCamera.m_orientation.m_rollDegrees, -45.0f, 45.0f );

	if ( g_debugFreeFly_F1 )
	{
		// Set cameraPos roam "freely"
		m_gameMode3DWorldCamera.SetPerspectiveView( 2.0f, 60.0f, 0.1f, 10000.0f );
		m_gameMode3DWorldCamera.SetRenderBasis( Vec3(0.0f, 0.0f, 1.0f), Vec3(-1.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f) );
	}
	else
	{
		// Set cameraPos to stay attached to playerPos
		Vec3 playerPos = Vec3::ZERO;
		if ( g_debugToggleLegs_2 )
		{
			playerPos = m_root->m_jointPos_LS - ( m_gameMode3DWorldCamera.m_orientation.GetForwardDir_XFwd_YLeft_ZUp() * m_distCamAwayFromPlayer );					// #QuadupedHack
		}
		else
		{
			playerPos = m_quadruped->m_root->m_jointPos_LS - ( m_gameMode3DWorldCamera.m_orientation.GetForwardDir_XFwd_YLeft_ZUp() * m_distCamAwayFromPlayer );		// #QuadupedHack
		}
		m_gameMode3DWorldCamera.SetTransform( playerPos, m_gameMode3DWorldCamera.m_orientation );

		m_gameMode3DWorldCamera.SetPerspectiveView( 2.0f, 60.0f, 0.1f, 10000.0f );
		m_gameMode3DWorldCamera.SetRenderBasis( Vec3(0.0f, 0.0f, 1.0f), Vec3(-1.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f) );
	}

	// Update UI camera
	m_gameMode3DUICamera.SetOrthoView( Vec2( 0.0f, 0.0f ), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y ) );		
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode3D::RenderWorldObjects() const
{
	//----------------------------------------------------------------------------------------------------------------------
	// Begin World Camera
	//----------------------------------------------------------------------------------------------------------------------
	g_theRenderer->BeginCamera( m_gameMode3DWorldCamera );
	DebugRenderWorld( m_gameMode3DWorldCamera );

	// Initialize common render variables
	std::vector<Vertex_PCU> verts_Text;
	std::vector<Vertex_PCU> verts_world;
	std::vector<Vertex_PCU> verts_Creature;
	verts_world.reserve( 90'000 );
	std::vector<Vertex_PCU> verts_NoTexture;
	std::vector<Vertex_PCU> verts_BackfaceCull;

	AddVertsForCompass( verts_NoTexture, Vec3::ZERO, 10.0f, 0.5f );
	//----------------------------------------------------------------------------------------------------------------------
	// "ProtoMode" title
	//----------------------------------------------------------------------------------------------------------------------
	Vec3 textOrigin = Vec3( 200.0f, 130.0f, 100.0f );
	Vec3 iBasis		= Vec3(	  0.0f,  -1.0f,   0.0f );
	Vec3 jBasis		= Vec3(	  0.0f,   0.0f,   1.0f );
	g_theApp->m_textFont->AddVertsForText3D( verts_Text, textOrigin, iBasis, jBasis, 25.0f, "ProtoMode3D!", Rgba8::GREEN );
	// Render Functions
	RenderEnvironment( verts_world );
	RenderCreature( verts_Creature, verts_NoTexture, verts_BackfaceCull, verts_Text );
	RenderRaycasts( verts_NoTexture );
	m_map->Render();
	
/*
	//----------------------------------------------------------------------------------------------------------------------
	// Render debug rayVsTri hit info
	//----------------------------------------------------------------------------------------------------------------------
	Vec3 rayEnd				= m_rayVsTri.m_rayStartPosition + m_rayVsTri.m_rayFwdNormal * m_rayVsTri.m_rayMaxLength;
	Vec3 impactNormalEnd	= m_rayVsTri.m_impactPos + m_rayVsTri.m_impactNormal * 1.0f;
	AddVertsForArrow3D ( verts,	m_rayVsTri.m_rayStartPosition,				    rayEnd, 0.5f, Rgba8::DARK_YELLOW	);
	AddVertsForArrow3D ( verts, m_rayVsTri.m_rayStartPosition,	m_rayVsTri.m_impactPos, 0.1f, Rgba8::CYAN			);
	AddVertsForArrow3D ( verts,		   m_rayVsTri.m_impactPos,		   impactNormalEnd, 0.1f, Rgba8::BLUE			);
	AddVertsForSphere3D( verts,		   m_rayVsTri.m_impactPos,				0.5f, 6.0f, 4.0f, Rgba8::MAGENTA		);
*/

	//----------------------------------------------------------------------------------------------------------------------
	// End world camera
	//----------------------------------------------------------------------------------------------------------------------
	g_theRenderer->EndCamera( m_gameMode3DWorldCamera );

	//----------------------------------------------------------------------------------------------------------------------
	// Draw calls 
	//----------------------------------------------------------------------------------------------------------------------
	// backface culled objects
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_NONE );
	g_theRenderer->DrawVertexArray( static_cast<int>( verts_BackfaceCull.size() ), verts_BackfaceCull.data() );
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );

	// World objects
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( g_theApp->m_texture_RockWithGrass );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->DrawVertexArray( static_cast<int>( verts_world.size() ), verts_world.data() );

	// Creature 
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( g_theApp->m_texture_GlowingRock );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->DrawVertexArray( static_cast<int>( verts_Creature.size() ), verts_Creature.data() );
	
	// Text objects
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( &g_theApp->m_textFont->GetTexture() );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->DrawVertexArray( static_cast<int>( verts_Text.size() ), verts_Text.data() );
	// Reset binded texture
	g_theRenderer->BindTexture( nullptr );

	// No Texture
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->DrawVertexArray( static_cast<int>( verts_NoTexture.size() ), verts_NoTexture.data() );
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode3D::RenderUIObjects() const
{
	//----------------------------------------------------------------------------------------------------------------------
	// Begin UI Camera
	g_theRenderer->BeginCamera( m_gameMode3DUICamera );
	DebugRenderScreen( m_gameMode3DUICamera );

	//----------------------------------------------------------------------------------------------------------------------
	// Get or Create font
	std::vector<Vertex_PCU> textVerts;
	float cellHeight	= 2.0f;
	AABB2 textbox1		= AABB2( Vec2( 0.0f, 0.0f ), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y - 1.0f ) );
	
	std::string gameInfo;
	if ( g_debugFreeFly_F1 )
	{
		if ( g_debugText_F4 )
		{
			gameInfo = Stringf( "F1 (Toggle camera/player) Camera || Mode (F6/F7 for prev/next) || GameMode (3D)" );
		}
	}						
	else
	{
		if ( g_debugText_F4 )
		{
			gameInfo = Stringf( "F1 (Toggle camera/player) Player || Mode (F6/F7 for prev/next) || GameMode (3D)" );
		}
	}
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight, gameInfo, Rgba8::YELLOW, 0.8f, Vec2( 0.0f, 1.0f ), TextDrawMode::SHRINK_TO_FIT );
																											   
	if ( g_debugText_F4 )
	{
		// Core Values
		float fps							 = 1.0f / g_theApp->m_gameClock.GetDeltaSeconds();
		float scale							 = g_theApp->m_gameClock.GetTimeScale();
		std::string cameraPosText			 = Stringf( "Cam position:           %0.2f, %0.2f, %0.2f",	m_gameMode3DWorldCamera.m_position.x,				m_gameMode3DWorldCamera.m_position.y,					m_gameMode3DWorldCamera.m_position.z );
		std::string cameraOrientationText	 = Stringf( "Cam Orientation (YPR):  %0.2f, %0.2f, %0.2f",	m_gameMode3DWorldCamera.m_orientation.m_yawDegrees, m_gameMode3DWorldCamera.m_orientation.m_pitchDegrees,	m_gameMode3DWorldCamera.m_orientation.m_rollDegrees );
		std::string timeText				 = Stringf( "Time: %0.2f. FPS: %0.2f, Scale %0.2f.", g_theApp->m_gameClock.GetTotalSeconds(), fps, scale );
		std::string rightArmAngleText		 = Stringf( "RootRightAngle:       X: %0.2f, Y: %0.2f, Z: %0.2f\n",				m_rightArm->m_firstJoint->m_eulerAngles_LS.m_yawDegrees, 
																															m_rightArm->m_firstJoint->m_eulerAngles_LS.m_pitchDegrees, 
																															m_rightArm->m_firstJoint->m_eulerAngles_LS.m_rollDegrees ).c_str();

		std::string rightArmDirText			 = Stringf( "rightArmDirText:      X: %0.2f, Y: %0.2f, Z: %0.2f\n",				m_rightArm->m_firstJoint->m_fwdDir.x, 
																															m_rightArm->m_firstJoint->m_fwdDir.y, 
																															m_rightArm->m_firstJoint->m_fwdDir.z ).c_str();	

		g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight, 			 	   timeText, Rgba8::YELLOW, 0.75f,  Vec2( 1.0f, 1.0f  ), TextDrawMode::SHRINK_TO_FIT );	
		g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight, 			  cameraPosText, Rgba8::YELLOW, 0.75f,	Vec2( 0.0f, 0.97f ), TextDrawMode::SHRINK_TO_FIT );
		g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight,    cameraOrientationText, Rgba8::YELLOW, 0.75f,  Vec2( 0.0f, 0.94f ), TextDrawMode::SHRINK_TO_FIT );
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
void GameMode3D::AddVertsForCompass( std::vector<Vertex_PCU>&compassVerts, Vec3 startPosition, float axisLength, float axisThickness ) const
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
void GameMode3D::UpdateDebugLimbGoalPositions()
{
	// Update arm debug goal positions
	Vec3 rootFwdDir		= m_root->m_eulerAngles_LS.GetForwardDir_XFwd_YLeft_ZUp();
	Vec3 rootLeftDir	= m_root->m_eulerAngles_LS.GetAsMatrix_XFwd_YLeft_ZUp().GetJBasis3D();
	m_debugGoalPos_RA	= Vec3( m_root->m_jointPos_LS.x, m_root->m_jointPos_LS.y, m_root->m_jointPos_LS.z - 10.0f ) + ( rootFwdDir * ( m_maxArmLength * 0.5f ) ) + ( rootLeftDir * ( -m_maxArmLength * 0.25f ) );
	m_debugGoalPos_LA	= Vec3( m_root->m_jointPos_LS.x, m_root->m_jointPos_LS.y, m_root->m_jointPos_LS.z - 10.0f ) + ( rootFwdDir * ( m_maxArmLength * 0.5f ) ) + ( rootLeftDir * (  m_maxArmLength * 0.25f ) );

	// Update foot debug goal positions
	m_hipFwdDir				= m_hip->m_firstJoint->m_eulerAngles_LS.GetAsMatrix_XFwd_YLeft_ZUp().GetIBasis3D();
	m_hipLeftDir			= m_hip->m_firstJoint->m_eulerAngles_LS.GetAsMatrix_XFwd_YLeft_ZUp().GetJBasis3D();
	m_loweredHipPos			= Vec3( m_hip->m_firstJoint->m_jointPos_LS.x, m_hip->m_firstJoint->m_jointPos_LS.y, m_hip->m_firstJoint->m_jointPos_LS.z - 10.0f );
	m_debugGoalPos_RL		= m_loweredHipPos + ( m_hipFwdDir * ( m_maxFeetLength * 0.5f ) ) + ( -m_hipLeftDir * ( m_maxFeetLength * 0.25f ) );
	m_debugGoalPos_LL		= m_loweredHipPos + ( m_hipFwdDir * ( m_maxFeetLength * 0.5f ) ) + (  m_hipLeftDir * ( m_maxFeetLength * 0.25f ) );
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode3D::UpdateDebugTreePosInput()
{
	float stepAmount = 1.0f;

	// East (+X)
	if ( g_theInput->IsKeyDown( 'I' ) )
	{
		m_treeDebugTargetPos += Vec3( stepAmount, 0.0f, 0.0f );
	}
	// West (-X)
	if ( g_theInput->IsKeyDown( 'K' ) )
	{
		m_treeDebugTargetPos += Vec3( -stepAmount, 0.0f, 0.0f );
	}
	// North (+Y)
	if ( g_theInput->IsKeyDown( 'J' ) )
	{
		m_treeDebugTargetPos += Vec3( 0.0f, stepAmount, 0.0f );
	}
	// South (-Y)
	if ( g_theInput->IsKeyDown( 'L' ) )
	{
		m_treeDebugTargetPos += Vec3( 0.0f, -stepAmount, 0.0f );
	}
	// Sky (+Z)
	if ( g_theInput->IsKeyDown( 'O' ) )
	{
		m_treeDebugTargetPos += Vec3( 0.0f, 0.0f, stepAmount );
	}
	// Ground (-Z)
	if ( g_theInput->IsKeyDown( 'U' ) )
	{
		m_treeDebugTargetPos += Vec3( 0.0f, 0.0f, -stepAmount );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Start Pos
	//----------------------------------------------------------------------------------------------------------------------
	// East (+X)
	if ( g_theInput->IsKeyDown( 'T' ) )
	{
		m_treeBranch2->m_position_WS += Vec3( 1.0f, 0.0f, 0.0f );
	}
	// West (-X)
	if ( g_theInput->IsKeyDown( 'G' ) )
	{
		m_treeBranch2->m_position_WS += Vec3( -1.0f, 0.0f, 0.0f );
	}
	// North (+Y)
	if ( g_theInput->IsKeyDown( 'F' ) )
	{
		m_treeBranch2->m_position_WS += Vec3( 0.0f, 1.0f, 0.0f );
	}
	// South (-Y)
	if ( g_theInput->IsKeyDown( 'H' ) )
	{
		m_treeBranch2->m_position_WS += Vec3( 0.0f, -1.0f, 0.0f );
	}
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode3D::UpdateTrees()
{
	m_treeCreature->Update();

	m_treeBranch1->Update();
	m_treeBranch2->Update();
	m_treeBranch3->Update();
}
	
//----------------------------------------------------------------------------------------------------------------------
void GameMode3D::InitializeCreature()
{
	//----------------------------------------------------------------------------------------------------------------------
	// Quadruped
	//----------------------------------------------------------------------------------------------------------------------
	m_quadruped	= new Quadruped( this, Vec3( 0.0f, 0.0f, m_rootDefaultHeightZ ), 1.0f );
	m_quadruped->InitLimbs();

	//----------------------------------------------------------------------------------------------------------------------
	// Creature
	//----------------------------------------------------------------------------------------------------------------------
	// Create creature root and arm
	m_root					= new IK_Joint3D( 0, Vec3( 0.0f, 0.0f, m_rootDefaultHeightZ ), 1.0f );
	m_hip					= new IK_Chain3D(		  "hip", m_root->m_jointPos_LS, nullptr, nullptr, false );
	m_tail					= new IK_Chain3D(		 "tail", m_root->m_jointPos_LS			);
	m_head					= new IK_Chain3D(		 "head", m_root->m_jointPos_LS			);
	m_neck					= new IK_Chain3D(		 "neck", m_root->m_jointPos_LS			);
	m_rightArm				= new IK_Chain3D(  "rightArm", m_root->m_jointPos_LS			);
	m_leftArm				= new IK_Chain3D(	  "leftArm", m_root->m_jointPos_LS			);
	m_rightFoot				= new IK_Chain3D( "rightFoot", m_root->m_jointPos_LS			);
	m_leftFoot				= new IK_Chain3D(  "leftFoot", m_root->m_jointPos_LS			);
	m_creatureSkeletalSystemsList.emplace_back( m_hip		);
	m_creatureSkeletalSystemsList.emplace_back( m_tail		);
	m_creatureSkeletalSystemsList.emplace_back( m_head		);
	m_creatureSkeletalSystemsList.emplace_back( m_neck		);
	m_creatureSkeletalSystemsList.emplace_back( m_rightArm	);
	m_creatureSkeletalSystemsList.emplace_back( m_leftArm	);
	m_creatureSkeletalSystemsList.emplace_back( m_rightFoot );
	m_creatureSkeletalSystemsList.emplace_back( m_leftFoot	);
	// Create head
	m_head->CreateNewLimb( 0, m_limbLength );

	//----------------------------------------------------------------------------------------------------------------------
	// Create arms with hinge constraints
	//----------------------------------------------------------------------------------------------------------------------
	FloatRange halfLengthRange = FloatRange( m_limbLength * 0.5f, m_limbLength * 0.5f );

/*
	//----------------------------------------------------------------------------------------------------------------------
	// Left Arm
	//----------------------------------------------------------------------------------------------------------------------
	// Thigh
	m_leftArm->CreateNewLimb( m_limbLength, JOINT_CONSTRAINT_TYPE_BALL_AND_SOCKET );
	m_leftArm->m_jointList[0]->SetConstraints_YPR( FloatRange( 180.0f, 180.0f ), FloatRange( 180.0f, 180.0f ) );
	// Knee
	m_leftArm->CreateNewLimb( m_limbLength, JOINT_CONSTRAINT_TYPE_HINGE_KNEE );
	m_leftArm->m_jointList[1]->SetConstraints_YPR( FloatRange( 0.0f, 135.0f ), FloatRange( 15.0f, 35.0f ) );
	// Ankle
	m_leftArm->CreateNewLimb( halfLengthRange, JOINT_CONSTRAINT_TYPE_HINGE_KNEE );
	m_leftArm->m_jointList[2]->SetConstraints_YPR( FloatRange( 0.0f, 0.0f ), FloatRange( 0.0f, 0.0f ) );
	
	//----------------------------------------------------------------------------------------------------------------------
	// Right Arm
	//----------------------------------------------------------------------------------------------------------------------
	// Thigh
	m_rightArm->CreateNewLimb( m_limbLength, JOINT_CONSTRAINT_TYPE_BALL_AND_SOCKET );
	m_rightArm->m_jointList[0]->SetConstraints_YPR( FloatRange( 180.0f, 180.0f ), FloatRange( 180.0f, 180.0f ) );
	// Knee
	m_rightArm->CreateNewLimb( m_limbLength, JOINT_CONSTRAINT_TYPE_HINGE_KNEE );
	m_rightArm->m_jointList[1]->SetConstraints_YPR(  FloatRange( 0.0f, 135.0f ), FloatRange( 15.0f, 35.0f ) );
	// Ankle
	m_rightArm->CreateNewLimb( halfLengthRange, JOINT_CONSTRAINT_TYPE_HINGE_KNEE );
	m_rightArm->m_jointList[2]->SetConstraints_YPR( FloatRange( 0.0f, 0.0f ), FloatRange( 0.0f, 0.0f ) );
	//----------------------------------------------------------------------------------------------------------------------
	// Left Foot
	//----------------------------------------------------------------------------------------------------------------------
	// Thigh
	m_leftFoot->CreateNewLimb( m_limbLength, JOINT_CONSTRAINT_TYPE_BALL_AND_SOCKET );
	m_leftFoot->m_jointList[0]->SetConstraints_YPR( FloatRange( 180.0f, 180.0f ), FloatRange( 180.0f, 180.0f ) );
	// Knee
	m_leftFoot->CreateNewLimb( m_limbLength, JOINT_CONSTRAINT_TYPE_HINGE_KNEE );
	m_leftFoot->m_jointList[1]->SetConstraints_YPR( FloatRange( 0.0f, 135.0f ), FloatRange( 15.0f, 35.0f ) );
	// Ankle
	m_leftFoot->CreateNewLimb( halfLengthRange, JOINT_CONSTRAINT_TYPE_HINGE_KNEE );
	m_leftFoot->m_jointList[2]->SetConstraints_YPR( FloatRange( 0.0f, 0.0f ), FloatRange( 0.0f, 0.0f ) );

	//----------------------------------------------------------------------------------------------------------------------
	// Right Foot
	//----------------------------------------------------------------------------------------------------------------------
	// Thigh
	m_rightFoot->CreateNewLimb( m_limbLength, JOINT_CONSTRAINT_TYPE_BALL_AND_SOCKET );
	m_rightFoot->m_jointList[0]->SetConstraints_YPR( FloatRange( 180.0f, 180.0f ), FloatRange( 180.0f, 180.0f ) );
	// Knee
	m_rightFoot->CreateNewLimb( m_limbLength, JOINT_CONSTRAINT_TYPE_HINGE_KNEE );
	m_rightFoot->m_jointList[1]->SetConstraints_YPR(  FloatRange( 0.0f, 135.0f ), FloatRange( 15.0f, 35.0f ) );
	// Ankle
	m_rightFoot->CreateNewLimb( halfLengthRange, JOINT_CONSTRAINT_TYPE_HINGE_KNEE );
	m_rightFoot->m_jointList[2]->SetConstraints_YPR( FloatRange( 0.0f, 0.0f ), FloatRange( 0.0f, 0.0f ) );
*/


	//----------------------------------------------------------------------------------------------------------------------
	// Old method for creating new limbs (W/o specifying angle constraints)
	//----------------------------------------------------------------------------------------------------------------------
	// Create specified num segments for rightFoot & leftFoot
	for ( int i = 0; i < m_numArms; i++ )
	{
		 m_rightArm->CreateNewLimb( i, m_limbLength );
		  m_leftArm->CreateNewLimb( i, m_limbLength );

		 m_rightArm->m_jointList[i]->SetConstraints_YPR( FloatRange( 0.0f, 90.0f ) );
		  m_leftArm->m_jointList[i]->SetConstraints_YPR( FloatRange( 0.0f, 90.0f ) );
	}
	for ( int i = 0; i < m_numFeet; i++ )
	{
		m_rightFoot->CreateNewLimb( i, m_limbLength );
		 m_leftFoot->CreateNewLimb( i, m_limbLength );
		 
		 m_rightFoot->m_jointList[i]->SetConstraints_YPR( FloatRange( 0.0f, 90.0f ) );
		  m_leftFoot->m_jointList[i]->SetConstraints_YPR( FloatRange( 0.0f, 90.0f ) );
	}
	// Create segments for hip
	for ( int i = 0; i < m_numHips; i++ )
	{
		m_hip->CreateNewLimb( i, m_limbLength );
	}
	// Create segments for tail
	for ( int i = 0; i < m_numTailSegments; i++ )
	{
		m_tail->CreateNewLimb( i, ( m_limbLength * 0.2f ) );
	}
	// Create segments for neck
	for ( int i = 0; i < m_numNeckSegments; i++ )
	{
		m_neck->CreateNewLimb( i, m_limbLength );
	}
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode3D::ToggleAnchorStates()
{
	//----------------------------------------------------------------------------------------------------------------------
	// Toggle Anchor bools
	//----------------------------------------------------------------------------------------------------------------------
	
	//----------------------------------------------------------------------------------------------------------------------
	// Right Arm
	float armAnchorDistTolerance  = 2.0f;
	float feetAnchorDistTolerance = 4.0f;
	if ( m_rightArm->m_anchorState == ANCHOR_STATE_LOCKED )
	{
	}
	if ( m_rightArm->m_anchorState == ANCHOR_STATE_MOVING )
	{
		// Check if I'm done moving then lock myself and free the other limb
		float distRightArmToGoal = GetDistance3D( m_rightArm->m_target.m_goalPos, m_rightArm->m_finalJoint->m_endPos );
		if ( ( distRightArmToGoal >= -armAnchorDistTolerance ) && ( distRightArmToGoal <= armAnchorDistTolerance ) )
		{
			m_rightArm->m_anchorState = ANCHOR_STATE_LOCKED;		// Current limb just planted
//			 m_leftArm->m_anchorState = ANCHOR_STATE_FREE;			// Unlock other limb
			m_leftFoot->m_anchorState = ANCHOR_STATE_FREE;			// Unlock other limb
		}

		// If goalPos is too far away, just find a new position.
		// If dist goalPos to root > maxLimbLength, find a new position
		float distRootToGoal = GetDistance3D( m_rightArm->m_target.m_goalPos, m_root->m_jointPos_LS );
		if ( distRootToGoal > ( m_maxArmLength * 0.95f ) )
		{
			m_rightArm->m_anchorState = ANCHOR_STATE_FREE;
		}
	}
	if ( m_rightArm->m_anchorState == ANCHOR_STATE_FREE )
	{
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Left Arm
	if ( m_leftArm->m_anchorState == ANCHOR_STATE_LOCKED )
	{
	}
	if ( m_leftArm->m_anchorState == ANCHOR_STATE_MOVING )
	{
		float distLeftArmToGoal	= GetDistance3D( m_leftArm->m_target.m_goalPos, m_leftArm->m_finalJoint->m_endPos );
		if ( ( distLeftArmToGoal >= -armAnchorDistTolerance ) && ( distLeftArmToGoal <= armAnchorDistTolerance ) )
		{
			 m_leftArm->m_anchorState = ANCHOR_STATE_LOCKED;		// Current limb just planted
//			m_rightArm->m_anchorState = ANCHOR_STATE_FREE;			// Unlock other limb
		   m_rightFoot->m_anchorState = ANCHOR_STATE_FREE;			// Unlock other limb
		}

		// If goalPos is too far away, just find a new position.
		// If dist goalPos to root > maxLimbLength, find a new position
//		float distRootToGoal = GetDistance3D(  m_rightArmGoalPos, m_root->m_jointPos_LS );
		float distRootToGoal = GetDistance3D( m_leftArm->m_target.m_goalPos, m_root->m_jointPos_LS );
		if ( distRootToGoal > m_maxArmLength )
		{
			m_leftArm->m_anchorState = ANCHOR_STATE_FREE;
		}
	}
	if ( m_leftArm->m_anchorState == ANCHOR_STATE_FREE )
	{
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Right Foot
	if ( m_rightFoot->m_anchorState == ANCHOR_STATE_LOCKED )
	{
	}
	if ( m_rightFoot->m_anchorState == ANCHOR_STATE_MOVING )
	{
		// Check if I'm done moving then lock myself and free the other limb
		float distRightFootToGoal = GetDistance3D( m_rightFoot->m_target.m_goalPos, m_rightFoot->m_finalJoint->m_endPos );
		if ( ( distRightFootToGoal >= -feetAnchorDistTolerance ) && ( distRightFootToGoal <= feetAnchorDistTolerance ) )
		{
			m_rightFoot->m_anchorState = ANCHOR_STATE_LOCKED;		// Current limb just planted
//			 m_leftFoot->m_anchorState = ANCHOR_STATE_FREE;			// Unlock other limb
			 m_leftArm->m_anchorState = ANCHOR_STATE_FREE;			// Unlock other limb
		}

		// If goalPos is too far away, just find a new position.
		// If dist goalPos to root > maxLimbLength, find a new position
		float distHipToGoal = GetDistance3D( m_rightFoot->m_target.m_goalPos, m_hip->m_firstJoint->m_jointPos_LS );
		if ( distHipToGoal > ( m_maxFeetLength * 0.95f ) )
		{
			m_rightFoot->m_anchorState = ANCHOR_STATE_FREE;
		}
	}
	if ( m_rightFoot->m_anchorState == ANCHOR_STATE_FREE )
	{
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Left Foot
	if ( m_leftFoot->m_anchorState == ANCHOR_STATE_LOCKED )
	{
	}
	if ( m_leftFoot->m_anchorState == ANCHOR_STATE_MOVING )
	{
		float distLeftFootToGoal   = GetDistance3D( m_leftFoot->m_target.m_goalPos, m_leftFoot->m_finalJoint->m_endPos );
		if ( ( distLeftFootToGoal >= -feetAnchorDistTolerance ) && ( distLeftFootToGoal <= feetAnchorDistTolerance ) )
		{
			 m_leftFoot->m_anchorState = ANCHOR_STATE_LOCKED;		// Current limb just planted
//			m_rightFoot->m_anchorState = ANCHOR_STATE_FREE;			// Unlock other limb
			 m_rightArm->m_anchorState = ANCHOR_STATE_FREE;			// Unlock other limb
		}

		// If goalPos is too far away, just find a new position.
		// If dist goalPos to root > maxLimbLength, find a new position
		float distRootToHip = GetDistance3D( m_leftFoot->m_target.m_goalPos, m_hip->m_firstJoint->m_jointPos_LS );
		if ( distRootToHip > ( m_maxFeetLength * 0.95f ) )
		{
			m_leftFoot->m_anchorState = ANCHOR_STATE_FREE;
		}
	}
	if ( m_leftFoot->m_anchorState == ANCHOR_STATE_FREE )
	{
	}	
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode3D::RenderCreature( std::vector<Vertex_PCU>& verts_Creature, std::vector<Vertex_PCU>& verts_NoTexture, std::vector<Vertex_PCU>& verts_BackfaceCull, std::vector<Vertex_PCU>& verts_Text ) const
{
	//----------------------------------------------------------------------------------------------------------------------
	// Quadruped
	//----------------------------------------------------------------------------------------------------------------------
	m_quadruped->m_leftFoot->m_shouldRender  = false;
	m_quadruped->m_rightFoot->m_shouldRender = false;
	m_quadruped->Render( verts_Creature, Rgba8::WHITE, Rgba8::MAGENTA );
	// Hiding joint gaps 
	AddVertsForSphere3D( verts_Creature, m_quadruped->m_leftArm->m_firstJoint->m_jointPos_LS,   1.2f, 4.0f, 4.0f, Rgba8::WHITE );
	AddVertsForSphere3D( verts_Creature, m_quadruped->m_rightArm->m_firstJoint->m_jointPos_LS,  1.2f, 4.0f, 4.0f, Rgba8::WHITE );
	AddVertsForSphere3D( verts_Creature, m_quadruped->m_leftFoot->m_firstJoint->m_jointPos_LS,  1.2f, 4.0f, 4.0f, Rgba8::WHITE );
	AddVertsForSphere3D( verts_Creature, m_quadruped->m_rightFoot->m_firstJoint->m_jointPos_LS, 1.2f, 4.0f, 4.0f, Rgba8::WHITE );
	// Joint basis
      m_quadruped->m_rightArm->DebugDrawJoints_IJK( verts_NoTexture );
 	   m_quadruped->m_leftArm->DebugDrawJoints_IJK( verts_NoTexture );
	  m_quadruped->m_leftFoot->DebugDrawJoints_IJK( verts_NoTexture );
	 m_quadruped->m_rightFoot->DebugDrawJoints_IJK( verts_NoTexture );
		
	// root to shoulders
	AddVertsForCylinder3D( verts_Creature, m_quadruped->m_root->m_jointPos_LS, m_quadruped->m_leftArm->m_firstJoint->m_jointPos_LS,  1.0f );
	AddVertsForCylinder3D( verts_Creature, m_quadruped->m_root->m_jointPos_LS, m_quadruped->m_rightArm->m_firstJoint->m_jointPos_LS, 1.0f );
	 
	// Hip to pelvis
	AddVertsForCylinder3D( verts_Creature, m_quadruped->m_hip->m_firstJoint->m_jointPos_LS, m_quadruped->m_leftFoot->m_position_WS,  1.0f, Rgba8::ORANGE );
	AddVertsForCylinder3D( verts_Creature, m_quadruped->m_hip->m_firstJoint->m_jointPos_LS, m_quadruped->m_rightFoot->m_position_WS, 1.0f, Rgba8::SUNSET_ORANGE );
	
	// Neck & Head
	m_quadruped->m_head->Render( verts_Creature, Rgba8::WHITE, Rgba8::MAGENTA );
	m_quadruped->m_neck->Render( verts_Creature, Rgba8::WHITE, Rgba8::MAGENTA );

	// Debug info
	if ( g_debugBasis_F3 )
	{
		m_quadruped->m_root->RenderIJK( verts_NoTexture, 10.0f );
		m_quadruped->m_hip->DebugDrawJoints_IJK( verts_NoTexture );
		AddVertsForSphere3D( verts_NoTexture,  m_quadruped->m_leftArm->m_target.m_currentPos, 1.0f, 4.0f, 4.0f, Rgba8::WHITE );
		AddVertsForSphere3D( verts_NoTexture, m_quadruped->m_rightArm->m_target.m_currentPos, 1.0f, 4.0f, 4.0f, Rgba8::GRAY  );
		m_quadruped->m_leftArm->RenderTarget_IJK( verts_NoTexture, 4.0f );
		m_quadruped->m_rightArm->RenderTarget_IJK( verts_NoTexture, 4.0f );
	}
	if ( g_debugText_F4 )
	{
		float textHeight = 0.4f;
		Vec3  left		 = m_gameMode3DWorldCamera.m_orientation.GetAsMatrix_XFwd_YLeft_ZUp().GetJBasis3D();
		Vec3  up		 = m_gameMode3DWorldCamera.m_orientation.GetAsMatrix_XFwd_YLeft_ZUp().GetKBasis3D();
		 m_quadruped->m_leftArm->DebugTextJointPos_WorldSpace( verts_Text, textHeight, left, up, g_theApp->m_textFont );
		m_quadruped->m_rightArm->DebugTextJointPos_WorldSpace( verts_Text, textHeight, left, up, g_theApp->m_textFont );
	}
	if ( g_debugRenderBezier_1 )
	{
		// Debug Render Bezier
		m_quadruped->DebugRenderBezier( verts_NoTexture, m_quadruped->m_bezier_leftArm,   m_timer_LeftArm   );
		m_quadruped->DebugRenderBezier( verts_NoTexture, m_quadruped->m_bezier_rightArm,  m_timer_RightArm  );
		m_quadruped->DebugRenderBezier( verts_NoTexture, m_quadruped->m_bezier_leftFoot,  m_timer_LeftFoot  );
		m_quadruped->DebugRenderBezier( verts_NoTexture, m_quadruped->m_bezier_rightFoot, m_timer_RightFoot );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Render Palms
	//----------------------------------------------------------------------------------------------------------------------
	m_quadruped->m_leftPalm->Render ( verts_Creature, Rgba8::WHITE, Rgba8::PINK );
	m_quadruped->m_rightPalm->Render( verts_Creature, Rgba8::WHITE, Rgba8::PINK );
	
	Vec3 leftEnd  = m_quadruped->m_leftArm->m_finalJoint->m_endPos + m_quadruped->m_debugVector * 5.0f;
	AddVertsForArrow3D( verts_NoTexture, m_quadruped->m_leftArm->m_finalJoint->m_endPos, leftEnd, 0.2f );
	leftEnd		  = m_quadruped->m_leftArm->m_finalJoint->m_endPos + m_quadruped->m_raycast_LeftArmDown.m_raycastResult.m_impactNormal * 5.0f;
	AddVertsForArrow3D( verts_NoTexture, m_quadruped->m_leftArm->m_finalJoint->m_endPos, leftEnd, 0.2f, Rgba8::DARKER_GREEN );

	Vec3 rightEnd = m_quadruped->m_rightArm->m_finalJoint->m_endPos + m_quadruped->m_debugVector * 5.0f;
	AddVertsForArrow3D( verts_NoTexture, m_quadruped->m_rightArm->m_finalJoint->m_endPos, rightEnd, 0.2f, Rgba8::GRAY );
	rightEnd	  = m_quadruped->m_rightArm->m_finalJoint->m_endPos + m_quadruped->m_raycast_RightArmDown.m_raycastResult.m_impactNormal * 5.0f;
	AddVertsForArrow3D( verts_NoTexture, m_quadruped->m_rightArm->m_finalJoint->m_endPos, rightEnd, 0.2f, Rgba8::DARKER_GREEN );

	//----------------------------------------------------------------------------------------------------------------------
	// Debug render pole vectors
	//----------------------------------------------------------------------------------------------------------------------
	AddVertsForSphere3D( verts_NoTexture, m_quadruped->m_rightArm->m_firstJoint->m_poleVector, 1.0f, 8.0f, 16.0f, Rgba8::MAGENTA );
	AddVertsForSphere3D( verts_NoTexture, m_quadruped->m_leftArm-> m_firstJoint->m_poleVector, 1.0f, 8.0f, 16.0f, Rgba8::CYAN	 );

	//----------------------------------------------------------------------------------------------------------------------
	// Old Quad creature
	//----------------------------------------------------------------------------------------------------------------------
	// Root
	AddVertsForSphere3D( verts_Creature, m_root->m_jointPos_LS, 2.0f, 16.0f, 16.0f, Rgba8::MAGENTA );
	// Neck
	m_neck->Render( verts_Creature, Rgba8::DARK_GRAY, Rgba8::MAGENTA );
	// Head 
	m_head->Render( verts_Creature, Rgba8::LIGHTBLUE, Rgba8::GRAY );
	AddVertsForCylinder3D( verts_Creature, m_neck->m_firstJoint->m_endPos, m_head->m_firstJoint->m_jointPos_LS, 1.0f );
	// Arms 
	m_rightArm->Render( verts_Creature, Rgba8::DARK_RED,		Rgba8::CYAN	);
	 m_leftArm->Render( verts_Creature, Rgba8::DARKER_GREEN,	Rgba8::CYAN	);
	 // Debug info
	if ( g_debugBasis_F3 )
	{
		m_rightArm->RenderTarget_IJK	( verts_NoTexture, 10.0f );
		m_rightArm->DebugDrawJoints_IJK ( verts_NoTexture );
		 m_leftArm->RenderTarget_IJK	( verts_NoTexture, 10.0f );
		 m_leftArm->DebugDrawJoints_IJK ( verts_NoTexture );
	}
	// Arm shoulders
	Vec3 leftArmShoulder     = m_root->m_jointPos_LS + ( m_root->m_leftDir * 10.0f );
	Vec3 rightArmShoulder    = m_root->m_jointPos_LS - ( m_root->m_leftDir * 10.0f );
	AddVertsForCylinder3D( verts_Creature, m_root->m_jointPos_LS,  leftArmShoulder, 1.0f, Rgba8::CYAN );
	AddVertsForCylinder3D( verts_Creature, m_root->m_jointPos_LS, rightArmShoulder, 1.0f, Rgba8::CYAN );
	
	if ( g_debugAngles_F5 )
	{
		// Hip cone constraints
		m_quadruped->m_hip->DebugDrawConstraints_YPR( verts_BackfaceCull, 2.0f );

		float constraintHeight = 2.5f;
		m_quadruped->m_leftArm-> DebugDrawConstraints_YPR( verts_BackfaceCull, constraintHeight );
		m_quadruped->m_rightArm->DebugDrawConstraints_YPR( verts_BackfaceCull, constraintHeight );

		// Leg shoulders
		Vec3 leftFootShoulder	 = m_hip->m_position_WS  + ( m_root->m_leftDir * 10.0f );
		Vec3 rightFootShoulder	 = m_hip->m_position_WS  - ( m_root->m_leftDir * 10.0f );
		AddVertsForCylinder3D( verts_Creature, m_hip->m_position_WS,  leftFootShoulder, 1.0f, Rgba8::CYAN );
		AddVertsForCylinder3D( verts_Creature, m_hip->m_position_WS, rightFootShoulder, 1.0f, Rgba8::CYAN );
		// Feet
		m_rightFoot->Render( verts_Creature, Rgba8::DARK_BLUE,	  Rgba8::CYAN );
		 m_leftFoot->Render( verts_Creature, Rgba8::SUNSET_ORANGE, Rgba8::CYAN );
		 if ( g_debugBasis_F3 )
		 {
			 m_rightFoot->RenderTarget_IJK( verts_NoTexture, 10.0f );
			 m_leftFoot->RenderTarget_IJK ( verts_NoTexture, 10.0f );
		 }
		// Hip
		m_hip->Render( verts_Creature, Rgba8::DARK_GRAY, Rgba8::MAGENTA );
		// Tail
		m_tail->Render( verts_Creature, Rgba8::DARK_GRAY, Rgba8::MAGENTA );
	}
	// Root Orientation
	Vec3 rootFwdDir			= m_root->m_eulerAngles_LS.GetForwardDir_XFwd_YLeft_ZUp();
	AddVertsForArrow3D( verts_NoTexture, m_root->m_jointPos_LS, m_root->m_jointPos_LS + ( rootFwdDir * 10.0f ), 1.0f, Rgba8::BLUE );
	// Head Orientation
	Vec3 headFwdDir			= m_head->m_firstJoint->m_eulerAngles_LS.GetForwardDir_XFwd_YLeft_ZUp();
	Vec3 headFwdDirEndPos	= m_head->m_firstJoint->m_endPos + ( headFwdDir * 10.0f );
	AddVertsForArrow3D( verts_NoTexture, m_head->m_firstJoint->m_endPos, headFwdDirEndPos, 1.0f, Rgba8::SKYBLUE );
	// Goal positions
	if ( g_debugBasis_F3 )
	{
		// Goal End Effectors
		AddVertsForSphere3D( verts_NoTexture,  m_rightArm->m_target.m_goalPos, 2.0f, 8.0f, 16.0f, Rgba8::DARKER_RED	);
		AddVertsForSphere3D( verts_NoTexture,   m_leftArm->m_target.m_goalPos, 2.0f, 8.0f, 16.0f, Rgba8::DARK_CYAN	);
		AddVertsForSphere3D( verts_NoTexture, m_rightFoot->m_target.m_goalPos, 2.0f, 8.0f, 16.0f, Rgba8::DARK_BLUE	);
		AddVertsForSphere3D( verts_NoTexture,  m_leftFoot->m_target.m_goalPos, 2.0f, 8.0f, 16.0f, Rgba8::PURPLE		);

		// Root basis vectors
		Vec3 endPos = m_root->m_jointPos_LS + ( m_root->m_fwdDir * 15.0f );
		AddVertsForArrow3D( verts_NoTexture, m_root->m_jointPos_LS, endPos, 1.0f, Rgba8::RED );
		endPos		= m_root->m_jointPos_LS + ( m_root->m_leftDir * 15.0f );
		AddVertsForArrow3D( verts_NoTexture, m_root->m_jointPos_LS, endPos, 1.0f, Rgba8::GREEN );
		endPos		= m_root->m_jointPos_LS + ( m_root->m_upDir * 15.0f );
		AddVertsForArrow3D( verts_NoTexture, m_root->m_jointPos_LS, endPos, 1.0f, Rgba8::BLUE  );

		// Render arms/feet "ideal" placement positions
		AddVertsForSphere3D( verts_NoTexture, m_debugGoalPos_RA, 2.0f, 8.0f, 16.0f, Rgba8::WHITE );
		AddVertsForSphere3D( verts_NoTexture, m_debugGoalPos_LA, 2.0f, 8.0f, 16.0f, Rgba8::BROWN );
		AddVertsForSphere3D( verts_NoTexture, m_debugGoalPos_RL, 2.0f, 8.0f, 16.0f, Rgba8::WHITE );
		AddVertsForSphere3D( verts_NoTexture, m_debugGoalPos_LL, 2.0f, 8.0f, 16.0f, Rgba8::BROWN );
	}

	if ( g_debugRenderBezier_1 )
	{
		//----------------------------------------------------------------------------------------------------------------------
		// Test rendering for bezier curves 
		//----------------------------------------------------------------------------------------------------------------------
		// Right Arm
		float elaspedTime	= m_timer_RightArm.GetElapsedTime();
		Vec3 bezierPosAtT	= m_bezierCurve_RightArm.GetPointAtTime( elaspedTime );
		AddVertsForSphere3D( verts_NoTexture, bezierPosAtT, 2.0f, 8.0f, 16.0f, Rgba8::CYAN );
		AddVertsForSphere3D( verts_NoTexture, m_bezierCurve_RightArm.m_startPos , 2.0f, 8.0f, 16.0f, Rgba8::RED	  );
		AddVertsForSphere3D( verts_NoTexture, m_bezierCurve_RightArm.m_guidePos1, 2.0f, 8.0f, 16.0f, Rgba8::GREEN   );
		AddVertsForSphere3D( verts_NoTexture, m_bezierCurve_RightArm.m_guidePos2, 2.0f, 8.0f, 16.0f, Rgba8::BLUE	  );
		AddVertsForSphere3D( verts_NoTexture, m_bezierCurve_RightArm.m_endPos   , 2.0f, 8.0f, 16.0f, Rgba8::MAGENTA );
		// Left Arm
		elaspedTime			= m_timer_LeftArm.GetElapsedTime();
		bezierPosAtT		= m_bezierCurve_LeftArm.GetPointAtTime( elaspedTime );
		AddVertsForSphere3D( verts_NoTexture, bezierPosAtT, 2.0f, 8.0f, 16.0f, Rgba8::CYAN );
		AddVertsForSphere3D( verts_NoTexture, m_bezierCurve_LeftArm.m_startPos , 2.0f, 8.0f, 16.0f, Rgba8::RED	  );
		AddVertsForSphere3D( verts_NoTexture, m_bezierCurve_LeftArm.m_guidePos1, 2.0f, 8.0f, 16.0f, Rgba8::GREEN    );
		AddVertsForSphere3D( verts_NoTexture, m_bezierCurve_LeftArm.m_guidePos2, 2.0f, 8.0f, 16.0f, Rgba8::BLUE	  );
		AddVertsForSphere3D( verts_NoTexture, m_bezierCurve_LeftArm.m_endPos   , 2.0f, 8.0f, 16.0f, Rgba8::MAGENTA  );
		// Right Foot
		elaspedTime			= m_timer_RightFoot.GetElapsedTime();
		bezierPosAtT		= m_bezierCurve_RightFoot.GetPointAtTime( elaspedTime );
		AddVertsForSphere3D( verts_NoTexture, bezierPosAtT, 2.0f, 8.0f, 16.0f, Rgba8::CYAN );
		AddVertsForSphere3D( verts_NoTexture, m_bezierCurve_RightFoot.m_startPos , 2.0f, 8.0f, 16.0f, Rgba8::RED	  );
		AddVertsForSphere3D( verts_NoTexture, m_bezierCurve_RightFoot.m_guidePos1, 2.0f, 8.0f, 16.0f, Rgba8::GREEN   );
		AddVertsForSphere3D( verts_NoTexture, m_bezierCurve_RightFoot.m_guidePos2, 2.0f, 8.0f, 16.0f, Rgba8::BLUE	  );
		AddVertsForSphere3D( verts_NoTexture, m_bezierCurve_RightFoot.m_endPos   , 2.0f, 8.0f, 16.0f, Rgba8::MAGENTA );
		// Left Arm
		elaspedTime			= m_timer_LeftFoot.GetElapsedTime();
		bezierPosAtT		= m_bezierCurve_LeftFoot.GetPointAtTime( elaspedTime );
		AddVertsForSphere3D( verts_NoTexture, bezierPosAtT, 2.0f, 8.0f, 16.0f, Rgba8::CYAN );
		AddVertsForSphere3D( verts_NoTexture, m_bezierCurve_LeftFoot.m_startPos , 2.0f, 8.0f, 16.0f, Rgba8::RED	  );
		AddVertsForSphere3D( verts_NoTexture, m_bezierCurve_LeftFoot.m_guidePos1, 2.0f, 8.0f, 16.0f, Rgba8::GREEN    );
		AddVertsForSphere3D( verts_NoTexture, m_bezierCurve_LeftFoot.m_guidePos2, 2.0f, 8.0f, 16.0f, Rgba8::BLUE	  );
		AddVertsForSphere3D( verts_NoTexture, m_bezierCurve_LeftFoot.m_endPos   , 2.0f, 8.0f, 16.0f, Rgba8::MAGENTA  );

		// Left arm bezierCurve trail
		float thickness = 0.5f;
		Vec3 previousBezierDotPos = m_bezierCurve_LeftArm.m_startPos;
		int		m_numSubdivisions = 64;
		for ( int subdivisionIndex = 0; subdivisionIndex <= (m_numSubdivisions - 1); subdivisionIndex++ )
		{
			// Calculate subdivisions
			float t						= (1.0f / static_cast<float>(m_numSubdivisions)) * (subdivisionIndex + 1);
			Vec3 currentBezierDotPos	= m_bezierCurve_LeftArm.GetPointAtTime( t );
			// Render curved line graph
			AddVertsForLineSegment3D( verts_NoTexture, previousBezierDotPos, currentBezierDotPos, thickness, Rgba8::PURPLE );
			previousBezierDotPos = currentBezierDotPos;
		}
		// Right arm bezierCurve trail
		previousBezierDotPos = m_bezierCurve_RightArm.m_startPos;
		for ( int subdivisionIndex = 0; subdivisionIndex <= (m_numSubdivisions - 1); subdivisionIndex++ )
		{
			// Calculate subdivisions
			float t						= (1.0f / static_cast<float>(m_numSubdivisions)) * (subdivisionIndex + 1);
			Vec3 currentBezierDotPos	= m_bezierCurve_RightArm.GetPointAtTime( t );
			// Render curved line graph
			AddVertsForLineSegment3D( verts_NoTexture, previousBezierDotPos, currentBezierDotPos, thickness, Rgba8::PURPLE );
			previousBezierDotPos = currentBezierDotPos;
		}
		// Left Foot bezierCurve trail
		previousBezierDotPos = m_bezierCurve_RightFoot.m_startPos;
		m_numSubdivisions	 = 64;
		for ( int subdivisionIndex = 0; subdivisionIndex <= (m_numSubdivisions - 1); subdivisionIndex++ )
		{
			// Calculate subdivisions
			float t						= (1.0f / static_cast<float>(m_numSubdivisions)) * (subdivisionIndex + 1);
			Vec3 currentBezierDotPos	= m_bezierCurve_RightFoot.GetPointAtTime( t );
			// Render curved line graph
			AddVertsForLineSegment3D( verts_NoTexture, previousBezierDotPos, currentBezierDotPos, thickness, Rgba8::PURPLE );
			previousBezierDotPos = currentBezierDotPos;
		}
		// Right Foot bezierCurve trail
		previousBezierDotPos = m_bezierCurve_LeftFoot.m_startPos;
		for ( int subdivisionIndex = 0; subdivisionIndex <= (m_numSubdivisions - 1); subdivisionIndex++ )
		{
			// Calculate subdivisions
			float t						= (1.0f / static_cast<float>(m_numSubdivisions)) * (subdivisionIndex + 1);
			Vec3 currentBezierDotPos	= m_bezierCurve_LeftFoot.GetPointAtTime( t );
			// Render curved line graph
			AddVertsForLineSegment3D( verts_NoTexture, previousBezierDotPos, currentBezierDotPos, thickness, Rgba8::PURPLE );
			previousBezierDotPos = currentBezierDotPos;
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode3D::UpdateCreature( float deltaSeconds )
{
	//----------------------------------------------------------------------------------------------------------------------
	// Keep things attached 
	//----------------------------------------------------------------------------------------------------------------------
	// Ensure arm positions stay "attached" to the creature's root pos
	Vec3 leftArmShoulder	= m_root->m_jointPos_LS + ( m_root->m_leftDir * 10.0f );
	Vec3 rightArmShoulder	= m_root->m_jointPos_LS - ( m_root->m_leftDir * 10.0f );
	m_leftArm->m_position_WS   = leftArmShoulder;
	m_rightArm->m_position_WS  = rightArmShoulder;
	
	 m_hip->m_target.m_currentPos = m_root->m_jointPos_LS;

	 // Drag tail when moving the hip
	 m_tail->m_position_WS = m_hip->m_position_WS;

	 // Keep neck attached to root
	 m_neck->m_position_WS = m_root->m_jointPos_LS;

	 // Keep head in "place"	 
	Vec3 rootFwdDir						= m_root->m_eulerAngles_LS.GetForwardDir_XFwd_YLeft_ZUp();
	float neckLength					= m_numNeckSegments * m_limbLength;
	m_head->m_firstJoint->m_fwdDir		= rootFwdDir;
	m_head->m_position_WS					= ( m_root->m_jointPos_LS + ( m_head->m_firstJoint->m_fwdDir * neckLength ) ) + Vec3( 0.0f, 0.0f, 10.0f );
	m_head->m_firstJoint->m_jointPos_LS		= m_head->m_position_WS;
	 
	//----------------------------------------------------------------------------------------------------------------------
	// LookAtTarget (head to trees if close enough)
	//----------------------------------------------------------------------------------------------------------------------
	// Check if trees are nearby (within a certain distance to head)
		// True: Look at trees
		// False: set m_headHasLookAtTarget to false.
	 float distHeadToTree		 = GetDistance3D( m_head->m_target.m_goalPos, m_treeBranch2->m_finalJoint->m_endPos );
	 float minDistToLookAtTarget = 50.0f;
	 if ( distHeadToTree < minDistToLookAtTarget )
	 {
//		 m_headHasLookAtTarget = true;
		 m_headHasLookAtTarget = false;
	 }
	 else
	 {
		 m_headHasLookAtTarget = false;
	 }

	// Ensure head is looking "fwd", if there is no target.
	if ( m_headHasLookAtTarget )
	{
		Vec3 dirHeadToTree									= ( m_treeBranch2->m_finalJoint->m_endPos - m_head->m_firstJoint->m_endPos ).GetNormalized();
		m_head->m_target.m_goalPos						= m_head->m_firstJoint->m_endPos + ( dirHeadToTree * 5.0f );
		EulerAngles eulerAngles;
		Vec3 left											= dirHeadToTree.GetRotatedAboutZDegrees( 90.0f );
		left												= left.GetNormalized();
		eulerAngles											= EulerAngles::GetAsEuler_XFwd_YLeft_ZUp( dirHeadToTree, left );
		float yawDegreesToTree								= eulerAngles.m_yawDegrees;
		float pitchDegreesToTree							= eulerAngles.m_pitchDegrees;
		m_head->m_firstJoint->m_eulerAngles_LS.m_yawDegrees		= yawDegreesToTree;
		m_head->m_firstJoint->m_eulerAngles_LS.m_pitchDegrees	= pitchDegreesToTree;
	}
	else
	{
		m_head->m_target.m_goalPos = m_head->m_firstJoint->m_endPos + ( m_head->m_firstJoint->m_fwdDir * 5.0f );
	}
	
	 // Ensure foot positions stay "attached" to the creature's root pos
		  m_hip->m_position_WS  = m_hip->m_firstJoint->m_jointPos_LS;
    Vec3 leftFootShoulder	 = m_hip->m_position_WS + ( m_root->m_leftDir * 10.0f );
    Vec3 rightFootShoulder	 = m_hip->m_position_WS - ( m_root->m_leftDir * 10.0f );
	 m_leftFoot->m_position_WS  = leftFootShoulder;
	 m_rightFoot->m_position_WS = rightFootShoulder;

	//----------------------------------------------------------------------------------------------------------------------
	// Reach out to targets
	//----------------------------------------------------------------------------------------------------------------------
	// Have neck reach out to head
	 m_neck->m_target.m_goalPos = m_head->m_firstJoint->m_jointPos_LS;
/*
	 if ( !m_isClimbing )
	 {
		 if ( m_raycast_LeftArmDown.m_didRayImpact )
		 {
			  m_leftArm->m_target.m_goalPos.z  = m_raycast_LeftArmDown.m_updatedImpactPos.z;
		 }
		 else
		 {
	//		 m_leftArmGoalPos.z = 
		 }
		 if ( m_raycast_rightArmDown.m_didRayImpact )
		 {
			 m_rightArm->m_target.m_goalPos.z  = m_raycast_rightArmDown.m_updatedImpactPos.z;
		 }
		 if ( m_raycast_leftFootDown.m_didRayImpact )
		 {
			 m_leftFoot->m_target.m_goalPos.z  = m_raycast_leftFootDown.m_updatedImpactPos.z;
		 }
		 if ( m_raycast_rightFootDown.m_didRayImpact )
		 {
			 m_rightFoot->m_target.m_goalPos.z = m_raycast_rightFootDown.m_updatedImpactPos.z;
		 }
	 }
 */

	 //----------------------------------------------------------------------------------------------------------------------
	 // Lerp currentEE to goal EndEffector positions
	 //----------------------------------------------------------------------------------------------------------------------
	 m_currentWalkLerpSpeed			   = Interpolate( m_currentWalkLerpSpeed, m_goalWalkLerpSpeed, deltaSeconds );
	 float fractionTowardsEnd		   = 0.1f + ( deltaSeconds * m_currentWalkLerpSpeed );
	 m_rightArm->m_target.m_currentPos = Interpolate(  m_rightArm->m_target.m_currentPos,	m_rightArm->m_target.m_goalPos, fractionTowardsEnd );
	  m_leftArm->m_target.m_currentPos = Interpolate(   m_leftArm->m_target.m_currentPos,   m_leftArm->m_target.m_goalPos, fractionTowardsEnd );
	 m_leftFoot->m_target.m_currentPos = Interpolate(  m_leftFoot->m_target.m_currentPos,  m_leftFoot->m_target.m_goalPos, fractionTowardsEnd );
	m_rightFoot->m_target.m_currentPos = Interpolate( m_rightFoot->m_target.m_currentPos, m_rightFoot->m_target.m_goalPos, fractionTowardsEnd );
		 m_tail->m_target.m_currentPos = Interpolate( 	  m_tail->m_target.m_currentPos, 		m_tail->m_target.m_goalPos, fractionTowardsEnd );
		 m_head->m_target.m_currentPos = m_head->m_target.m_goalPos;
		 m_neck->m_target.m_currentPos = m_neck->m_target.m_goalPos;

	 //----------------------------------------------------------------------------------------------------------------------
	 // "Wag tail"
	 //----------------------------------------------------------------------------------------------------------------------
	 // Update tail goalPos
	 float cos						= CosDegrees( m_currentTime * 25.0f );
	 float maxTailLength			= ( m_numTailSegments * m_limbLength ) * 0.75f;
	 m_loweredHipPos				= Vec3( m_loweredHipPos.x, m_loweredHipPos.y, ( m_loweredHipPos.z - 10.0f ) * cos ); 
	 m_tail->m_target.m_goalPos	= m_loweredHipPos + ( -m_hipFwdDir * maxTailLength ) + ( m_hipLeftDir * maxTailLength * m_sine );

	//----------------------------------------------------------------------------------------------------------------------
	// Solve IK to have arms reach out to respective targetPos
	//---------------------------------------------------------------------------------------------------------------------
	// Update SkeletalSystems
	for ( int i = 0; i < m_creatureSkeletalSystemsList.size(); i++ )
	{
		m_creatureSkeletalSystemsList[i]->Update();
	}

	 UpdateCreatureHeight( deltaSeconds );
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode3D::UpdateCreatureRootPosInput_Walking( float deltaSeconds )
{
	if ( g_debugFreeFly_F1 == true )
	{
		return;
	}
	if ( m_isClimbing == true )
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

	Vec3 moveIntention = m_moveFwdDir;
	//----------------------------------------------------------------------------------------------------------------------
	// All directions are local
	//----------------------------------------------------------------------------------------------------------------------
	if ( g_theInput->IsKeyDown( KEYCODE_SHIFT ) )
	{
		m_currentSpeed		= m_fasterSpeed;
		m_isSprinting		= true;
		m_goalWalkLerpSpeed	= m_sprintLerpSpeed;
	}
	if ( g_theInput->WasKeyJustReleased( KEYCODE_SHIFT ) )
	{
		m_currentSpeed		= m_defaultSpeed;
		m_isSprinting		= false;
		m_goalWalkLerpSpeed	= m_walkLerpSpeed;
	}

	// Forward
	if ( g_theInput->IsKeyDown( 'W' ) )
	{
		if ( g_debugToggleLegs_2 )
		{
			m_root->m_jointPos_LS += ( Vec3( iBasis.x, iBasis.y, 0.0f ) * m_currentSpeed ) * deltaSeconds;					// #QuadupedHack
		}
		else
		{
			m_quadruped->m_root->m_jointPos_LS += ( Vec3( iBasis.x, iBasis.y, 0.0f ) * m_currentSpeed ) * deltaSeconds;
		}
		moveIntention += ( Vec3( iBasis.x, iBasis.y, 0.0f ) * m_currentSpeed ) * deltaSeconds;
	}
	// Left
	if ( g_theInput->IsKeyDown( 'A' ) )
	{
		if ( g_debugToggleLegs_2 )
		{
			m_root->m_jointPos_LS += ( Vec3( jBasis.x, jBasis.y, 0.0f ) * m_currentSpeed ) * deltaSeconds;					// #QuadupedHack
		}
		else
		{
			m_quadruped->m_root->m_jointPos_LS += ( Vec3( jBasis.x, jBasis.y, 0.0f ) * m_currentSpeed ) * deltaSeconds;
		}
		moveIntention += ( Vec3( jBasis.x, jBasis.y, 0.0f ) * m_currentSpeed ) * deltaSeconds;
	}
	// Backwards
	if ( g_theInput->IsKeyDown( 'S' ) )
	{
		if ( g_debugToggleLegs_2 )
		{
			m_root->m_jointPos_LS -= ( Vec3( iBasis.x, iBasis.y, 0.0f ) * m_currentSpeed ) * deltaSeconds;					// #QuadupedHack
		}
		else
		{
			m_quadruped->m_root->m_jointPos_LS -= ( Vec3( iBasis.x, iBasis.y, 0.0f ) * m_currentSpeed ) * deltaSeconds;
		}
		moveIntention -= ( Vec3( iBasis.x, iBasis.y, 0.0f ) * m_currentSpeed ) * deltaSeconds;
	}
	// Right
	if ( g_theInput->IsKeyDown( 'D' ) )
	{
		if ( g_debugToggleLegs_2 )
		{
			m_root->m_jointPos_LS -= ( Vec3( jBasis.x, jBasis.y, 0.0f ) * m_currentSpeed ) * deltaSeconds;					// #QuadupedHack
		}
		else
		{
			m_quadruped->m_root->m_jointPos_LS -= ( Vec3( jBasis.x, jBasis.y, 0.0f ) * m_currentSpeed ) * deltaSeconds;
		}
		moveIntention -= ( Vec3( jBasis.x, jBasis.y, 0.0f ) * m_currentSpeed ) * deltaSeconds;
	}
	// Sky (+Z)
	if ( g_theInput->IsKeyDown( 'E' ) )
	{
		if ( g_debugToggleLegs_2 )
		{
			m_root->m_jointPos_LS += ( kBasis * m_currentSpeed ) * deltaSeconds;				// #QuadupedHack
		}
		else
		{
			m_quadruped->m_root->m_jointPos_LS += ( kBasis * m_currentSpeed ) * deltaSeconds;
		}
		moveIntention += ( kBasis * m_currentSpeed ) * deltaSeconds;
	}
	// Ground (-Z)
	if ( g_theInput->IsKeyDown( 'Q' ) )
	{
		if ( g_debugToggleLegs_2 )
		{
			m_root->m_jointPos_LS -= ( kBasis * m_currentSpeed ) * deltaSeconds;				// #QuadupedHack
		}
		else
		{
			m_quadruped->m_root->m_jointPos_LS -= ( kBasis * m_currentSpeed ) * deltaSeconds;
		}
		moveIntention -= ( kBasis * m_currentSpeed ) * deltaSeconds;
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Enable Climb
	//----------------------------------------------------------------------------------------------------------------------
	if ( g_theInput->WasKeyJustPressed( 'E' ) )
	{
		if ( m_didRayImpactClimbableObject_FWD )
		{
			m_isClimbing = true;
		}
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Mount Input
	//----------------------------------------------------------------------------------------------------------------------
	if ( g_theInput->WasKeyJustPressed( ' ' ) )
	{
		if ( m_raycast_Mount.m_didRayImpact )
		{
			// Do jump logic
			m_shouldMount = true;
		}
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Set movementFwdDir
	//----------------------------------------------------------------------------------------------------------------------
	m_moveFwdDir = moveIntention.GetNormalized();
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode3D::UpdateCreatureRootPosInput_Climbing( float deltaSeconds )
{
	if ( g_debugFreeFly_F1 == true )
	{
		return;
	}
	if ( m_isClimbing == false )
	{
		return;
	}

	Vec3 iBasis, jBasis, kBasis;
//	m_gameMode3DWorldCamera.m_orientation.GetAsVectors_XFwd_YLeft_ZUp( iBasis, jBasis, kBasis );
	m_root->m_eulerAngles_LS.GetAsVectors_XFwd_YLeft_ZUp( iBasis, jBasis, kBasis );
//	iBasis.z = 0.0f;
//	jBasis.z = 0.0f;
	iBasis	 = iBasis.GetNormalized(); 
	jBasis	 = jBasis.GetNormalized(); 
	kBasis   = kBasis.GetNormalized();

	Vec3 moveIntention = m_moveFwdDir;
	//----------------------------------------------------------------------------------------------------------------------
	// All directions are local
	//----------------------------------------------------------------------------------------------------------------------
	if ( g_theInput->IsKeyDown( KEYCODE_SHIFT ) )
	{
		m_currentSpeed					= m_fasterSpeed;
		m_goalWalkLerpSpeed	= m_sprintLerpSpeed;
	}
	if ( g_theInput->WasKeyJustReleased( KEYCODE_SHIFT ) )
	{
		m_currentSpeed					= m_defaultSpeed;
		m_goalWalkLerpSpeed	= m_walkLerpSpeed;
	}

	// Forward
	if ( g_theInput->IsKeyDown( 'W' ) )
	{
		m_root->m_jointPos_LS += ( kBasis * m_currentSpeed ) * deltaSeconds;
		moveIntention	   += ( kBasis * m_currentSpeed ) * deltaSeconds;
	}
	// Left
	if ( g_theInput->IsKeyDown( 'A' ) )
	{
		m_root->m_jointPos_LS += ( Vec3( jBasis.x, jBasis.y, 0.0f ) * m_currentSpeed ) * deltaSeconds;
		moveIntention	   += ( Vec3( jBasis.x, jBasis.y, 0.0f ) * m_currentSpeed ) * deltaSeconds;
	}
	// Backwards
	if ( g_theInput->IsKeyDown( 'S' ) )
	{
		m_root->m_jointPos_LS -= ( kBasis * m_currentSpeed ) * deltaSeconds;
		moveIntention	   -= ( kBasis * m_currentSpeed ) * deltaSeconds;
	}
	// Right
	if ( g_theInput->IsKeyDown( 'D' ) )
	{
		m_root->m_jointPos_LS -= ( Vec3( jBasis.x, jBasis.y, 0.0f ) * m_currentSpeed ) * deltaSeconds;
		moveIntention	   -= ( Vec3( jBasis.x, jBasis.y, 0.0f ) * m_currentSpeed ) * deltaSeconds;
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Climbing 
	//----------------------------------------------------------------------------------------------------------------------
	if ( g_theInput->WasKeyJustPressed( 'E' ) )
	{
		if ( m_isClimbing )
		{
			m_isClimbing = false;
		}
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Set movementFwdDir
	//----------------------------------------------------------------------------------------------------------------------
	m_moveFwdDir = moveIntention.GetNormalized();
}

//-------------------------------------------  ---------------------------------------------------------------------------
void GameMode3D::UpdateCreatureHeight( float deltaSeconds )
{
	// Breathing
	m_currentTime		= float( GetCurrentTimeSeconds() );
	float sine			= SinDegrees( m_currentTime * 60.0f );
	float heightOffset	= 1.0f;

	float avgArmHeight			= (  m_rightArm->m_target.m_currentPos.z +  m_leftArm->m_target.m_currentPos.z ) * 0.5f;
	float avgFootHeight			= ( m_rightFoot->m_target.m_currentPos.z + m_leftFoot->m_target.m_currentPos.z ) * 0.5f;
	float totalLengthOfArms		= ( m_numArms * m_limbLength ) * 0.6f;
	float totalLengthOfFeet		= ( m_numFeet * m_limbLength ) * 0.6f;
	
	float rootGoalHeightZ		= avgArmHeight  + totalLengthOfArms + ( heightOffset * sine );
	float hipGoalHeightZ		= avgFootHeight	+ totalLengthOfFeet;

	// Lerp from currentRootPos to goalPos
	float fractionTowardsEnd			= 0.01f;
	fractionTowardsEnd					+= deltaSeconds * 8.0f;
	m_root->m_jointPos_LS.z			    = Interpolate( m_root->m_jointPos_LS.z, rootGoalHeightZ, fractionTowardsEnd );
//	m_root->m_jointPos_LS.z			    = 30.0f;		// #Constraints Hack code
//	hipGoalHeightZ						= 30.0f;		// #Constraints Hack code
//	m_root->m_jointPos_LS.z			    = 20.0f;		// #Constraints Hack code
//	hipGoalHeightZ						= 20.0f;		// #Constraints Hack code

	for ( int i = 0; i < m_hip->m_jointList.size(); i++ )
	{
		m_hip->m_jointList[i]->m_jointPos_LS.z = Interpolate( m_hip->m_jointList[i]->m_jointPos_LS.z, hipGoalHeightZ, fractionTowardsEnd );
	}
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode3D::DetermineBestSprintStepPos()
{
	if ( !m_isSprinting )
	{
		return;
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Create animations for running
	//----------------------------------------------------------------------------------------------------------------------
	float halfArmLength		= m_maxArmLength  * 0.6f;
	float quarterArmLength	= m_maxArmLength  * 0.15f;
	float halfFeetLength	= m_maxFeetLength * 0.5f;
	float quarterFeetLength	= m_maxFeetLength * 0.25f;

	//----------------------------------------------------------------------------------------------------------------------
	// Right Arm
	if ( IsLimbIsTooFarFromRoot( m_rightArm, m_rightArm->m_target.m_currentPos) )
	{
		// Cannot move, is locked.					// Stay in the same place
		if ( m_rightArm->m_anchorState == ANCHOR_STATE_LOCKED )
		{
			// Clamp my body to stay in the same place until the other limb is finished moving and this limb unlocks
			// Then set a new foot placement and move
//			m_rightArm->m_position = m_rightArm->m_position;

			// Check if other limb can be anchored and my limb can be unlocked
			if ( m_rightFoot->m_anchorState == ANCHOR_STATE_FREE || m_rightFoot->m_anchorState == ANCHOR_STATE_LOCKED )
			{
				 m_rightArm->m_anchorState = ANCHOR_STATE_FREE;
				m_rightFoot->m_anchorState = ANCHOR_STATE_LOCKED;
			}
		}
		// Is moving, cannot lock.					// In the process of moving, don't do anything
		if ( m_rightArm->m_anchorState == ANCHOR_STATE_MOVING )
		{
		}
		// Is not moving, can lock, can also move,	// Limb can be locked or moved
		if ( m_rightArm->m_anchorState == ANCHOR_STATE_FREE )
		{
			if ( m_raycast_rightArmDown.m_didRayImpact )
			{
				m_rightArm->m_target.m_goalPos = m_raycast_rightArmDown.m_updatedImpactPos;
			}
			SpecifyFootPlacementPos( m_rightArm->m_target.m_goalPos, halfArmLength, -quarterArmLength );
			 m_rightArm->m_anchorState = ANCHOR_STATE_MOVING;
			m_rightFoot->m_anchorState = ANCHOR_STATE_LOCKED;
			 m_leftFoot->m_anchorState = ANCHOR_STATE_LOCKED;
		}
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Left Arm
	if ( IsLimbIsTooFarFromRoot( m_leftArm, m_leftArm->m_target.m_currentPos) )
	{
		// Cannot move, is locked.					// Stay in the same place
		if ( m_leftArm->m_anchorState == ANCHOR_STATE_LOCKED )
		{
			// Clamp my body to stay in the same place until the other limb is finished moving and this limb unlocks
			// Then set a new foot placement and move


			// Unlock the other limb if this limb is locked
			if ( m_leftFoot->m_anchorState == ANCHOR_STATE_FREE || m_leftFoot->m_anchorState == ANCHOR_STATE_LOCKED )
			{
				 m_leftArm->m_anchorState = ANCHOR_STATE_FREE;
				m_leftFoot->m_anchorState = ANCHOR_STATE_LOCKED;
			}
		}
		// Is moving, cannot lock.					// In the process of moving, don't do anything
		if ( m_leftArm->m_anchorState == ANCHOR_STATE_MOVING )
		{
		}
		// Is not moving, can lock, can also move,	// Limb can be locked or moved
		if ( m_leftArm->m_anchorState == ANCHOR_STATE_FREE )
		{
			if ( m_raycast_LeftArmDown.m_didRayImpact )
			{
				m_leftArm->m_target.m_goalPos = m_raycast_LeftArmDown.m_updatedImpactPos;
			}
			SpecifyFootPlacementPos( m_leftArm->m_target.m_goalPos, halfArmLength, quarterArmLength );
			 m_leftArm->m_anchorState  = ANCHOR_STATE_MOVING;	
			m_leftFoot->m_anchorState  = ANCHOR_STATE_LOCKED;
			m_rightFoot->m_anchorState = ANCHOR_STATE_LOCKED;
		}
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Right Foot
	if ( IsLimbIsTooFarFromHip( m_rightFoot, m_rightFoot->m_target.m_currentPos) )
	{
		// Cannot move, is locked.					// Stay in the same place
		if ( m_rightFoot->m_anchorState == ANCHOR_STATE_LOCKED )
		{

			// Clamp my body to stay in the same place until the other limb is finished moving and this limb unlocks
			// Then set a new foot placement and move

			// Unlock the other limb if this limb is locked
			if ( m_rightArm->m_anchorState == ANCHOR_STATE_FREE || m_rightArm->m_anchorState == ANCHOR_STATE_LOCKED )
			{
				 m_rightArm->m_anchorState = ANCHOR_STATE_LOCKED;
				m_rightFoot->m_anchorState = ANCHOR_STATE_FREE;
			}
		}
		// Is moving, cannot lock.					// In the process of moving, don't do anything
		if ( m_rightFoot->m_anchorState == ANCHOR_STATE_MOVING )
		{
		}
		// Is not moving, can lock, can also move,	// Limb can be locked or moved
		if ( m_rightFoot->m_anchorState == ANCHOR_STATE_FREE )
		{
			if ( m_raycast_rightFootDown.m_didRayImpact )
			{
				m_rightFoot->m_target.m_goalPos = m_raycast_rightFootDown.m_updatedImpactPos;
			}
			SpecifyFootPlacementPos( m_rightFoot->m_target.m_goalPos, m_hip->m_firstJoint, halfFeetLength, -quarterFeetLength );
			m_rightFoot->m_anchorState = ANCHOR_STATE_MOVING;
			 m_rightArm->m_anchorState = ANCHOR_STATE_LOCKED;
			 m_leftFoot->m_anchorState = ANCHOR_STATE_LOCKED;
		}
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Left Foot
	if ( IsLimbIsTooFarFromHip( m_leftFoot, m_leftFoot->m_target.m_currentPos) )
	{
		// Cannot move, is locked.					// Stay in the same place
		if ( m_leftFoot->m_anchorState == ANCHOR_STATE_LOCKED )
		{

			// Clamp my body to stay in the same place until the other limb is finished moving and this limb unlocks
			// Then set a new foot placement and move
			// Unlock the other limb if this limb is locked
			if ( m_leftArm->m_anchorState == ANCHOR_STATE_FREE || m_leftArm->m_anchorState == ANCHOR_STATE_LOCKED )
			{
				 m_leftArm->m_anchorState = ANCHOR_STATE_LOCKED;
				m_leftFoot->m_anchorState = ANCHOR_STATE_FREE;
			}

		}
		// Is moving, cannot lock.					// In the process of moving, don't do anything
		if ( m_leftFoot->m_anchorState == ANCHOR_STATE_MOVING )
		{
		}
		// Is not moving, can lock, can also move,	// Limb can be locked or moved
		if ( m_leftFoot->m_anchorState == ANCHOR_STATE_FREE )
		{
			if ( m_raycast_leftFootDown.m_didRayImpact )
			{
				m_leftFoot->m_target.m_goalPos = m_raycast_leftFootDown.m_updatedImpactPos;
			}
			SpecifyFootPlacementPos( m_leftFoot->m_target.m_goalPos, m_hip->m_firstJoint, halfFeetLength, quarterFeetLength );
			m_leftFoot->m_anchorState = ANCHOR_STATE_MOVING;
			 m_leftArm->m_anchorState  = ANCHOR_STATE_LOCKED;
			m_rightArm->m_anchorState  = ANCHOR_STATE_LOCKED;
		}
	}
	// Toggle anchor states for sprinting
	// Specify sprint step positions
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode3D::DetermineBestWalkStepPos()
{
	if ( m_isSprinting )
	{
		return;
	}

	// The positions only needs to be updated for one frame (not every frame) if the distance from currentLimb pos to root is too far away
	// The Z height of the rayImpactPos always needs to be updated
	float halfArmLength		= m_maxArmLength  * 0.5f;
//	float halfArmLength		= m_maxArmLength  * 0.5f;
//	float quarterArmLength	= m_maxArmLength  * 0.3f;
	float quarterArmLength	= m_maxArmLength  * 0.2f;
	float halfFeetLength	= m_maxFeetLength * 0.5f;
	float quarterFeetLength	= m_maxFeetLength * 0.25f;

	//----------------------------------------------------------------------------------------------------------------------
	// Right Arm
	if ( IsLimbIsTooFarFromRoot( m_rightArm, m_rightArm->m_target.m_currentPos ) )
	{ 
		// Cannot move, is locked.					// Stay in the same place
		if ( m_rightArm->m_anchorState == ANCHOR_STATE_LOCKED )
		{
			// Clamp my body to stay in the same place until the other limb is finished moving and this limb unlocks
			// Then set a new foot placement and move

			// Check if other limb can be anchored and my limb can be unlocked
			if ( m_leftArm->m_anchorState == ANCHOR_STATE_FREE || m_leftArm->m_anchorState == ANCHOR_STATE_LOCKED )
			{
				m_rightArm->m_anchorState = ANCHOR_STATE_FREE;
				m_leftArm->m_anchorState  = ANCHOR_STATE_LOCKED;
			}
			else
			{
				if ( m_raycast_rightArmDown.m_didRayImpact )
				{
					m_rightArm->m_target.m_goalPos = m_raycast_rightArmDown.m_updatedImpactPos;
				}
			}
		}
		// Is moving, cannot lock.					// In the process of moving, don't do anything
		if ( m_rightArm->m_anchorState == ANCHOR_STATE_MOVING )
		{
		}
		// Is not moving, can lock, can also move,	// Limb can be locked or moved
		if ( m_rightArm->m_anchorState == ANCHOR_STATE_FREE )
		{
			if ( m_raycast_rightArmDown.m_didRayImpact )
			{
				m_rightArm->m_target.m_goalPos = m_raycast_rightArmDown.m_updatedImpactPos;
			}
//			SpecifyFootPlacementPos( m_rightArm->m_target.m_goalPos, quarterArmLength, -quarterArmLength );
			SpecifyFootPlacementPos( m_rightArm->m_target.m_goalPos, halfArmLength, -quarterArmLength );
			m_root->m_eulerAngles_LS.GetAsVectors_XFwd_YLeft_ZUp( m_root->m_fwdDir, m_root->m_leftDir, m_root->m_upDir );
			m_rightArm->m_target.m_fwdDir  = m_root->m_fwdDir;
			m_rightArm->m_target.m_leftDir = m_root->m_leftDir;
			m_rightArm->m_target.m_upDir	= m_root->m_upDir;

			//----------------------------------------------------------------------------------------------------------------------
			// Set Bezier Curve points
			//----------------------------------------------------------------------------------------------------------------------
			m_bezierCurve_RightArm.m_startPos			= m_rightArm->m_target.m_currentPos;
			m_bezierCurve_RightArm.m_endPos				= m_rightArm->m_target.m_goalPos;
			Vec3 distStartEnd							= m_bezierCurve_RightArm.m_endPos - m_bezierCurve_RightArm.m_startPos;
			float length								= distStartEnd.GetLength();
			m_bezierCurve_RightArm.m_guidePos1			= m_rightArm->m_target.m_goalPos - ( m_root->m_fwdDir * ( length * 0.9f  ) ) + ( m_root->m_upDir * 20.0f );
			m_bezierCurve_RightArm.m_guidePos2			= m_rightArm->m_target.m_goalPos - ( m_root->m_fwdDir * ( length * 0.33f ) ) + ( m_root->m_upDir * 30.0f );

			m_timer_RightArm.Start();
			DebuggerPrintf( "Right Arm Step\n" );

			// Toggle Anchor States
			m_rightArm->m_anchorState			= ANCHOR_STATE_MOVING;
			 m_leftArm->m_anchorState			= ANCHOR_STATE_LOCKED;
		}
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Left Arm
	if ( IsLimbIsTooFarFromRoot( m_leftArm, m_leftArm->m_target.m_currentPos) )
	{
		// Cannot move, is locked.					// Stay in the same place
		if ( m_leftArm->m_anchorState == ANCHOR_STATE_LOCKED )
		{
			// Clamp my body to stay in the same place until the other limb is finished moving and this limb unlocks
			// Then set a new foot placement and move
			

			// Unlock the other limb if this limb is locked
			if ( m_rightArm->m_anchorState == ANCHOR_STATE_FREE || m_rightArm->m_anchorState == ANCHOR_STATE_LOCKED )
			{
				m_leftArm->m_anchorState  = ANCHOR_STATE_FREE;
				m_rightArm->m_anchorState = ANCHOR_STATE_LOCKED;
			}
			else
			{
				if ( m_raycast_LeftArmDown.m_didRayImpact )
				{
					m_leftArm->m_target.m_goalPos = m_raycast_LeftArmDown.m_updatedImpactPos;
				}
			}
		}
		// Is moving, cannot lock.					// In the process of moving, don't do anything
		if ( m_leftArm->m_anchorState == ANCHOR_STATE_MOVING )
		{
		}
		// Is not moving, can lock, can also move,	// Limb can be locked or moved
		if ( m_leftArm->m_anchorState == ANCHOR_STATE_FREE )
		{
			if ( m_raycast_LeftArmDown.m_didRayImpact )
			{
				m_leftArm->m_target.m_goalPos = m_raycast_LeftArmDown.m_updatedImpactPos;
			}
//			SpecifyFootPlacementPos( m_leftArm->m_target.m_goalPos, quarterArmLength , quarterArmLength );
			SpecifyFootPlacementPos( m_leftArm->m_target.m_goalPos, halfArmLength, quarterArmLength );
			m_root->m_eulerAngles_LS.GetAsVectors_XFwd_YLeft_ZUp( m_root->m_fwdDir, m_root->m_leftDir, m_root->m_upDir );
			m_leftArm->m_target.m_fwdDir   = m_root->m_fwdDir;
			m_leftArm->m_target.m_leftDir  = m_root->m_leftDir;
			m_leftArm->m_target.m_upDir	= m_root->m_upDir;

			//----------------------------------------------------------------------------------------------------------------------
			// Set Bezier Curve points
			//----------------------------------------------------------------------------------------------------------------------
			m_bezierCurve_LeftArm.m_startPos	= m_leftArm->m_target.m_currentPos;
			m_bezierCurve_LeftArm.m_endPos		= m_leftArm->m_target.m_goalPos;
			Vec3 distStartEnd					= m_bezierCurve_LeftArm.m_endPos - m_bezierCurve_LeftArm.m_startPos;
			float length						= distStartEnd.GetLength();
			m_bezierCurve_LeftArm.m_guidePos1 = m_leftArm->m_target.m_goalPos - ( m_root->m_fwdDir * ( length * 0.9f  ) ) + ( m_root->m_upDir * 20.0f );
			m_bezierCurve_LeftArm.m_guidePos2 = m_leftArm->m_target.m_goalPos - ( m_root->m_fwdDir * ( length * 0.33f ) ) + ( m_root->m_upDir * 30.0f );
			m_timer_LeftArm.Start();
			DebuggerPrintf( "Left Arm Step\n" );

			// Toggle Anchor States
			m_leftArm->m_anchorState  = ANCHOR_STATE_MOVING;	
			m_rightArm->m_anchorState = ANCHOR_STATE_LOCKED;
		}
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Right Foot
	if ( IsLimbIsTooFarFromHip( m_rightFoot, m_rightFoot->m_target.m_currentPos) )
	{
		// Cannot move, is locked.					// Stay in the same place
		if ( m_rightFoot->m_anchorState == ANCHOR_STATE_LOCKED )
		{

			// Clamp my body to stay in the same place until the other limb is finished moving and this limb unlocks
			// Then set a new foot placement and move

			// Unlock the other limb if this limb is locked
			if ( m_leftFoot->m_anchorState == ANCHOR_STATE_FREE || m_leftFoot->m_anchorState == ANCHOR_STATE_LOCKED )
			{
				m_leftFoot->m_anchorState  = ANCHOR_STATE_LOCKED;
				m_rightFoot->m_anchorState = ANCHOR_STATE_FREE;
			}
			else
			{
				if ( m_raycast_rightFootDown.m_didRayImpact )
				{
					m_rightFoot->m_target.m_goalPos = m_raycast_rightFootDown.m_updatedImpactPos;
				}
			}
		}
		// Is moving, cannot lock.					// In the process of moving, don't do anything
		if ( m_rightFoot->m_anchorState == ANCHOR_STATE_MOVING )
		{
		}
		// Is not moving, can lock, can also move,	// Limb can be locked or moved
		if ( m_rightFoot->m_anchorState == ANCHOR_STATE_FREE )
		{
			if ( m_raycast_rightFootDown.m_didRayImpact  )
			{
				m_rightFoot->m_target.m_goalPos = m_raycast_rightFootDown.m_updatedImpactPos;
			}
			SpecifyFootPlacementPos( m_rightFoot->m_target.m_goalPos, m_hip->m_firstJoint, halfFeetLength, -quarterFeetLength );
			m_root->m_eulerAngles_LS.GetAsVectors_XFwd_YLeft_ZUp( m_root->m_fwdDir, m_root->m_leftDir, m_root->m_upDir );
			m_rightFoot->m_target.m_fwdDir  = m_root->m_fwdDir;
			m_rightFoot->m_target.m_leftDir = m_root->m_leftDir;
			m_rightFoot->m_target.m_upDir	 = m_root->m_upDir;

			//----------------------------------------------------------------------------------------------------------------------
			// Set Bezier Curve points
			//----------------------------------------------------------------------------------------------------------------------
			m_bezierCurve_RightFoot.m_startPos	= m_rightFoot->m_target.m_currentPos;
			m_bezierCurve_RightFoot.m_endPos	= m_rightFoot->m_target.m_goalPos;
			Vec3 distStartEnd					= m_bezierCurve_RightFoot.m_endPos - m_bezierCurve_RightFoot.m_startPos;
			float length						= distStartEnd.GetLength();
			m_bezierCurve_RightFoot.m_guidePos1 = m_rightFoot->m_target.m_goalPos - ( m_root->m_fwdDir * ( length * 0.9f  ) ) + ( m_root->m_upDir * 20.0f );
			m_bezierCurve_RightFoot.m_guidePos2 = m_rightFoot->m_target.m_goalPos - ( m_root->m_fwdDir * ( length * 0.33f ) ) + ( m_root->m_upDir * 30.0f );
			m_timer_RightFoot.Start();
			DebuggerPrintf( "Right Foot Step\n" );

			// Toggle Anchor States
			m_rightFoot->m_anchorState = ANCHOR_STATE_MOVING;
			 m_leftFoot->m_anchorState = ANCHOR_STATE_LOCKED;
		}
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Left Foot
	if ( IsLimbIsTooFarFromHip( m_leftFoot, m_leftFoot->m_target.m_currentPos) )
	{
		// Cannot move, is locked.					// Stay in the same place
		if ( m_leftFoot->m_anchorState == ANCHOR_STATE_LOCKED )
		{

			// Clamp my body to stay in the same place until the other limb is finished moving and this limb unlocks
			// Then set a new foot placement and move
			// Unlock the other limb if this limb is locked
			if ( m_rightFoot->m_anchorState == ANCHOR_STATE_FREE || m_rightFoot->m_anchorState == ANCHOR_STATE_LOCKED )
			{
				m_rightFoot->m_anchorState  = ANCHOR_STATE_LOCKED;
				m_leftFoot->m_anchorState	= ANCHOR_STATE_FREE;
			}
			else
			{
				if ( m_raycast_leftFootDown.m_didRayImpact )
				{
					m_leftFoot->m_target.m_goalPos = m_raycast_leftFootDown.m_updatedImpactPos;
				}
			}
		}
		// Is moving, cannot lock.					// In the process of moving, don't do anything
		if ( m_leftFoot->m_anchorState == ANCHOR_STATE_MOVING )
		{
		}
		// Is not moving, can lock, can also move.	// Limb can be locked or moved
		if ( m_leftFoot->m_anchorState == ANCHOR_STATE_FREE )
		{
			if ( m_raycast_leftFootDown.m_didRayImpact )
			{
				m_leftFoot->m_target.m_goalPos = m_raycast_leftFootDown.m_updatedImpactPos;
			}
			SpecifyFootPlacementPos( m_leftFoot->m_target.m_goalPos, m_hip->m_firstJoint, halfFeetLength, quarterFeetLength );
			m_root->m_eulerAngles_LS.GetAsVectors_XFwd_YLeft_ZUp( m_root->m_fwdDir, m_root->m_leftDir, m_root->m_upDir );
			m_leftFoot->m_target.m_fwdDir  = m_root->m_fwdDir;
			m_leftFoot->m_target.m_leftDir = m_root->m_leftDir;
			m_leftFoot->m_target.m_upDir	= m_root->m_upDir;

			//----------------------------------------------------------------------------------------------------------------------
			// Set Bezier Curve points
			//----------------------------------------------------------------------------------------------------------------------
			m_bezierCurve_LeftFoot.m_startPos	= m_leftFoot->m_target.m_currentPos;
			m_bezierCurve_LeftFoot.m_endPos		= m_leftFoot->m_target.m_goalPos;
			Vec3 distStartEnd					= m_bezierCurve_LeftFoot.m_endPos - m_bezierCurve_LeftFoot.m_startPos;
			float length						= distStartEnd.GetLength();
			m_bezierCurve_LeftFoot.m_guidePos1  = m_leftFoot->m_target.m_goalPos - ( m_root->m_fwdDir * ( length * 0.9f  ) ) + ( m_root->m_upDir * 20.0f );
			m_bezierCurve_LeftFoot.m_guidePos2  = m_leftFoot->m_target.m_goalPos - ( m_root->m_fwdDir * ( length * 0.33f ) ) + ( m_root->m_upDir * 30.0f );
			m_timer_LeftFoot.Start();
			DebuggerPrintf( "Left Foot Step\n" );

			// Toggle Anchor States
			 m_leftFoot->m_anchorState = ANCHOR_STATE_MOVING;
			m_rightFoot->m_anchorState = ANCHOR_STATE_LOCKED;
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode3D::DetermineBestClimbPos()
{
	// The positions only needs to be updated for one frame (not every frame) if the distance from currentLimb pos to root is too far away
	// The Z height of the rayImpactPos always needs to be updated
	float halfArmLength		= m_maxArmLength * 0.5f;
	float quarterArmLength	= m_maxArmLength * 0.3f;
//	float halfFeetLength	= m_maxFeetLength * 0.5f;
//	float quarterFeetLength	= m_maxFeetLength * 0.25f;

	//----------------------------------------------------------------------------------------------------------------------
	// Right Arm
	if ( IsLimbIsTooFarFromRoot( m_rightArm, m_rightArm->m_target.m_currentPos) )
	{
		// Cannot move, is locked.					// Stay in the same place
		if ( m_rightArm->m_anchorState == ANCHOR_STATE_LOCKED )
		{
			// Clamp my body to stay in the same place until the other limb is finished moving and this limb unlocks
			// Then set a new foot placement and move

			// Check if other limb can be anchored and my limb can be unlocked
			if ( m_leftArm->m_anchorState == ANCHOR_STATE_FREE || m_leftArm->m_anchorState == ANCHOR_STATE_LOCKED )
			{
				m_rightArm->m_anchorState = ANCHOR_STATE_FREE;
				m_leftArm->m_anchorState  = ANCHOR_STATE_LOCKED;
			}
			else
			{
				if ( m_raycast_rightArmDown.m_didRayImpact )
				{
					m_rightArm->m_target.m_goalPos = m_raycast_rightArmDown.m_updatedImpactPos;
				}
			}
		}
		// Is moving, cannot lock.					// In the process of moving, don't do anything
		if ( m_rightArm->m_anchorState == ANCHOR_STATE_MOVING )
		{
		}
		// Is not moving, can lock, can also move,	// Limb can be locked or moved
		if ( m_rightArm->m_anchorState == ANCHOR_STATE_FREE )
		{
			if ( m_raycast_rightArmDown.m_didRayImpact )
			{
				m_rightArm->m_target.m_goalPos = m_raycast_rightArmDown.m_updatedImpactPos;
			}
			SpecifyTargetPosForClimbing( m_rightArm->m_target.m_goalPos, halfArmLength, -quarterArmLength );
			m_rightArm->m_anchorState = ANCHOR_STATE_MOVING;
			m_leftArm->m_anchorState  = ANCHOR_STATE_LOCKED;
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
bool GameMode3D::IsLimbIsTooFarFromRoot( IK_Chain3D* currentLimb, Vec3 footTargetPos )
{
	// Check if limb is placed too far from Root
//	float maxLimbLength		= 25.0f;
	float maxLimbLength		= ( currentLimb->m_jointList.size() * m_limbLength ) * 0.99f;
	float distFootPosToRoot = GetDistance3D( footTargetPos, m_root->m_jointPos_LS );
	if ( distFootPosToRoot > maxLimbLength )
	{
		return true;
	}
	return false;
}

//----------------------------------------------------------------------------------------------------------------------
bool GameMode3D::IsLimbIsTooFarFromHip( IK_Chain3D* currentLimb, Vec3 footTargetPos )
{
	// Check if limb is placed too far from Root
	float maxLimbLength		= ( currentLimb->m_jointList.size() * m_limbLength ) * 0.99f;
	float distFootPosToHip	= GetDistance3D( footTargetPos, m_hip->m_firstJoint->m_jointPos_LS );
	if ( distFootPosToHip > maxLimbLength )
	{
		return true;
	}
	return false;
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode3D::SpecifyTargetPosForClimbing( Vec3& targetPos, float fwdStepAmount, float leftStepAmount )
{
	Vec3 prevTargetPos		= targetPos;

	// Determine the ideal next step position
	float maxLength			= ( m_numArms * m_limbLength ) * 0.95f;
	Vec3 moveLeftDir		= m_moveFwdDir.GetRotatedAboutZDegrees( 90.0f );
	Vec3 kBasis				= Vec3( 0.0f, 0.0f, 1.0f );
	Vec3 idealNewPos		= m_root->m_jointPos_LS + ( kBasis * fwdStepAmount ) + ( moveLeftDir * leftStepAmount );

//	wallPosition + zOffset + leftStepOffset;

	// Use raycast to ensure the ideal next step is "placed" on a walkable block
	RaycastResult3D raycastResult3D;
//	Vec3 rayStartPos		= idealNewPos + ( m_moveFwdDir *  );
	Vec3 rayStartPos		= idealNewPos;
	Vec3 impactPos			= Vec3::ZERO;
	Vec3 impactNormal		= Vec3::ZERO;
	bool didRayImpactBlock	= false;
	didRayImpactBlock		= DidRaycastHitWalkableBlock( raycastResult3D, rayStartPos, Vec3::NEGATIVE_Z, m_raylength_Long, impactPos, impactNormal );

	float distRootToPreviousAlternativePos = 500.0f;
	// Ensure ideal next step is close enough AND on a walkable block
	float distRootToNewPos = GetDistance3D( idealNewPos, m_root->m_jointPos_LS );
	if ( CompareIfFloatsAreEqual( distRootToNewPos, maxLength, 2.0f ) && didRayImpactBlock )
	{		
		// Set to optimal "next step" foot placement position since targetPos is valid
		targetPos = idealNewPos;
	}
	else
	{
		// Since normal "next step" position is invalid, find better footPlacement position

		/*
		//  Get nearest valid footstep algorithm
		1. Check if this block is walkable
		2. Check if alternative next step is close enough
		2a. True: Step
		2b. False: Keep searching
		2b1. Stay in the same position if next position was not found
		*/

		Vec3 nearestPoint3D = Vec3( 0.0f, 0.0f, -1000.0f );
		for ( int i = 0; i < m_blockList.size(); i++ )
		{
			// Ensure currentBlock is "Walkable"
			Block* currentBlock = m_blockList[i];
			if ( !currentBlock->m_isWalkable )
			{
				continue;
			}

			// Get nearestPoint
			Vec3 alternativeNewPos	= currentBlock->m_aabb3.GetNearestPoint( idealNewPos );
			alternativeNewPos.z		= currentBlock->m_aabb3.m_maxs.z;

			// Determine the closest position as valid 
			float distRootToNewAlternativePos = GetDistance3D( alternativeNewPos, m_root->m_jointPos_LS );
			if ( distRootToNewAlternativePos <= maxLength )
			{
				nearestPoint3D						= alternativeNewPos;
				distRootToPreviousAlternativePos	= distRootToNewAlternativePos;
			}
			else
			{
				// Keep track of the closest position to creature
				if ( distRootToNewAlternativePos <= distRootToPreviousAlternativePos )
				{
					nearestPoint3D						= alternativeNewPos;
					distRootToPreviousAlternativePos	= distRootToNewAlternativePos;
				}
			}
		}

		if ( nearestPoint3D == Vec3( 0.0f, 0.0f, -1000.0f ) )
		{
			nearestPoint3D = prevTargetPos;
		}

		// Set targetPos to nearestPoint
		targetPos = nearestPoint3D;
	}
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode3D::SpecifyFootPlacementPos( Vec3& targetPos, float fwdStepAmount, float leftStepAmount )
{
	Vec3 prevTargetPos		= targetPos;

	// Determine the ideal next step position
	float maxLength			= ( m_numArms * m_limbLength ) * 0.95f;
	Vec3 moveLeftDir		= m_moveFwdDir.GetRotatedAboutZDegrees( 90.0f );	
	Vec3 idealNewPos		= Vec3( m_root->m_jointPos_LS.x, m_root->m_jointPos_LS.y, 0.0f ) + ( m_moveFwdDir * fwdStepAmount ) + ( moveLeftDir * leftStepAmount );

	// Use raycast to ensure the ideal next step is "placed" on a walkable block
	RaycastResult3D raycastResult3D;
	Vec3 rayStartPos		= idealNewPos + Vec3( 0.0f, 0.0f, 5.0f );
	Vec3 impactPos			= Vec3::ZERO;
	Vec3 impactNormal		= Vec3::ZERO;
	bool didRayImpactBlock	= false;
//	didRayImpactBlock		= DidRaycastHitWalkableBlock( raycastResult3D, rayStartPos, Vec3::NEGATIVE_Z, m_raylength_Long, impactPos, impactNormal );
	didRayImpactBlock		= m_quadruped->DoesRaycastHitFloor( raycastResult3D, rayStartPos, Vec3::NEGATIVE_Z, maxLength );
	idealNewPos				= raycastResult3D.m_impactPos;

	float distRootToPreviousAlternativePos = 500.0f;
	// Ensure ideal next step is close enough AND on a walkable block
//	float distRootToNewPos = GetDistance3D( idealNewPos, m_root->m_jointPos_LS );
//	if ( CompareIfFloatsAreEqual( distRootToNewPos, maxLength, 2.0f ) && didRayImpactBlock )
	if ( didRayImpactBlock )
	{		
		// Set to optimal "next step" foot placement position since targetPos is valid
		targetPos = idealNewPos;
	}
	else
	{
		// Since normal "next step" position is invalid, find better footPlacement position

		/*
		//  Get nearest valid footstep algorithm
		  	1. Check if this block is walkable
		  	2. Check if alternative next step is close enough
		  		2a. True: Step
		  		2b. False: Keep searching
		  			2b1. Stay in the same position if next position was not found
		*/

		Vec3 nearestPoint3D = Vec3( 0.0f, 0.0f, -1000.0f );
		for ( int i = 0; i < m_blockList.size(); i++ )
		{
			// Ensure currentBlock is "Walkable"
			Block* currentBlock = m_blockList[i];
			if ( !currentBlock->m_isWalkable )
			{
				continue;
			}

			// Get nearestPoint
			Vec3 alternativeNewPos	= currentBlock->m_aabb3.GetNearestPoint( idealNewPos );
			alternativeNewPos.z		= currentBlock->m_aabb3.m_maxs.z;

			// Determine the closest position as valid 
			float distRootToNewAlternativePos = GetDistance3D( alternativeNewPos, m_root->m_jointPos_LS );
			if ( distRootToNewAlternativePos <= maxLength )
			{
				nearestPoint3D						= alternativeNewPos;
				distRootToPreviousAlternativePos	= distRootToNewAlternativePos;
			}
			else
			{
				// Keep track of the closest position to creature
				if ( distRootToNewAlternativePos <= distRootToPreviousAlternativePos )
				{
					nearestPoint3D						= alternativeNewPos;
					distRootToPreviousAlternativePos	= distRootToNewAlternativePos;
				}
			}
		}

		if ( nearestPoint3D == Vec3( 0.0f, 0.0f, -1000.0f ) )
		{
			nearestPoint3D = prevTargetPos;
		}

		// Set targetPos to nearestPoint
		targetPos = nearestPoint3D;
	}
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode3D::SpecifyFootPlacementPos( Vec3& targetPos, IK_Joint3D* refLimb, float fwdStepAmount, float leftStepAmount )
{
	Vec3 prevTargetPos	= targetPos;
	// Determine the ideal next step position
	float maxLength		= ( m_numFeet * m_limbLength ) * 0.9f;
	Vec3 refLimbFwdDir	= refLimb->m_eulerAngles_LS.GetAsMatrix_XFwd_YLeft_ZUp().GetIBasis3D();
	Vec3 refLimbLeftDir	= refLimb->m_eulerAngles_LS.GetAsMatrix_XFwd_YLeft_ZUp().GetJBasis3D();
	Vec3 idealNewPos	= Vec3( refLimb->m_jointPos_LS.x, refLimb->m_jointPos_LS.y, 0.0f ) + ( refLimbFwdDir * fwdStepAmount ) + ( refLimbLeftDir * leftStepAmount );

	// Use raycast to ensure the ideal next step is "placed" on a walkable block
	RaycastResult3D raycastResult3D;
	Vec3 rayStartPos		= idealNewPos + Vec3( 0.0f, 0.0f, 5.0f );
	Vec3 impactPos			= Vec3::ZERO;
	Vec3 impactNormal		= Vec3::ZERO;
	bool didRayImpactBlock	= false;
//	didRayImpactBlock		= DidRaycastHitWalkableBlock( raycastResult3D, rayStartPos, Vec3::NEGATIVE_Z, m_raylength_Long, impactPos, impactNormal );
	didRayImpactBlock		= m_quadruped->DoesRaycastHitFloor( raycastResult3D, rayStartPos, Vec3::NEGATIVE_Z, maxLength );
	idealNewPos				= raycastResult3D.m_impactPos;

	float distRootToPreviousAlternativePos = 500.0f;
	// Ensure ideal next step is close enough AND on a walkable block
//	float distRootToNewPos	= GetDistance3D( idealNewPos, refLimb->m_jointPos_LS );
//	if ( CompareIfFloatsAreEqual( distRootToNewPos, maxLength, 2.0f ) && didRayImpactBlock )
	if ( didRayImpactBlock )
	{		
		// Set to optimal "next step" foot placement position since targetPos is valid
		targetPos = idealNewPos;
	}
	else
	{
		// Since normal "next step" position is invalid, find better footPlacement position
		/*
		//  Get nearest valid footstep algorithm
		  	1. Check if this block is walkable
		  	2. Check if alternative next step is close enough
		  		2a. True: Step
		  		2b. False: Keep searching
		  			2b1. Stay in the same position if next position was not found
		*/

		Vec3 nearestPoint3D = Vec3( 0.0f, 0.0f, -1000.0f );
		for ( int i = 0; i < m_blockList.size(); i++ )
		{
			// Ensure currentBlock is "Walkable"
			Block* currentBlock = m_blockList[i];
			if ( !currentBlock->m_isWalkable )
			{
				continue;
			}

			// Get nearestPoint
			Vec3 alternativeNewPos	= currentBlock->m_aabb3.GetNearestPoint( idealNewPos );
			alternativeNewPos.z		= currentBlock->m_aabb3.m_maxs.z;

			// Determine the closest position as valid 
			float distRootToNewAlternativePos = GetDistance3D( alternativeNewPos, m_root->m_jointPos_LS );
			if ( distRootToNewAlternativePos <= maxLength )
			{
				nearestPoint3D						= alternativeNewPos;
				distRootToPreviousAlternativePos	= distRootToNewAlternativePos;
			}
			else
			{
				// Keep track of the closest position to creature
				if ( distRootToNewAlternativePos <= distRootToPreviousAlternativePos )
				{
					nearestPoint3D						= alternativeNewPos;
					distRootToPreviousAlternativePos	= distRootToNewAlternativePos;
				}
			}

/*
			// Determine the closest position as valid 
			float distRootToNewAlternativePos = GetDistance3D( alternativeNewPos, m_root->m_jointPos_LS );
			if ( distRootToNewAlternativePos <= maxLength )
			{
				nearestPoint3D						= alternativeNewPos;
				distRootToPreviousAlternativePos	= distRootToNewAlternativePos;
			}
			else
			{
				// Keep track of the closest position to creature
				if ( distRootToNewAlternativePos <= distRootToPreviousAlternativePos )
				{
					nearestPoint3D = alternativeNewPos;
				}
			}
*/
		}

		if ( nearestPoint3D == Vec3( 0.0f, 0.0f, -1000.0f ) )
		{
			nearestPoint3D = prevTargetPos;
		}

		// Set targetPos to nearestPoint
		targetPos = nearestPoint3D;
	}
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode3D::SpecifyFootPlacementPos( Vec3& targetPos, IK_Joint3D* refLimb, float maxLength, float fwdStepAmount, float leftStepAmount )
{
	Vec3 prevTargetPos	= targetPos;
	// Determine the ideal next step position
	Vec3 refLimbFwdDir	= refLimb->m_eulerAngles_LS.GetAsMatrix_XFwd_YLeft_ZUp().GetIBasis3D();
	Vec3 refLimbLeftDir	= refLimb->m_eulerAngles_LS.GetAsMatrix_XFwd_YLeft_ZUp().GetJBasis3D();
	Vec3 idealNewPos	= Vec3( refLimb->m_jointPos_LS.x, refLimb->m_jointPos_LS.y, 0.0f ) + ( refLimbFwdDir * fwdStepAmount ) + ( refLimbLeftDir * leftStepAmount );

	// Use raycast to ensure the ideal next step is "placed" on a walkable block
	RaycastResult3D raycastResult3D;
	Vec3 rayStartPos		= idealNewPos + Vec3( 0.0f, 0.0f, 5.0f );
	Vec3 impactPos			= Vec3::ZERO;
	Vec3 impactNormal		= Vec3::ZERO;
	bool didRayImpactBlock	= false;
	didRayImpactBlock		= DidRaycastHitWalkableBlock( raycastResult3D, rayStartPos, Vec3::NEGATIVE_Z, m_raylength_Long, impactPos, impactNormal );

	float distRootToPreviousAlternativePos = 500.0f;
	// Ensure ideal next step is close enough AND on a walkable block
	float distRootToNewPos	= GetDistance3D( idealNewPos, refLimb->m_jointPos_LS );
	if ( CompareIfFloatsAreEqual( distRootToNewPos, maxLength, 2.0f ) && didRayImpactBlock )
	{		
		// Set to optimal "next step" foot placement position since targetPos is valid
		targetPos = idealNewPos;
	}
	else
	{
		// Since normal "next step" position is invalid, find better footPlacement position
		/*
			Get nearest valid footstep algorithm
			1. Check if this block is walkable
			2. Check if alternative next step is close enough
			2a. True: Step
			2b. False: Keep searching
			2b1. Stay in the same position if next position was not found
		*/

		Vec3 nearestPoint3D = Vec3( 0.0f, 0.0f, -1000.0f );
		for ( int i = 0; i < m_blockList.size(); i++ )
		{
			// Ensure currentBlock is "Walkable"
			Block* currentBlock = m_blockList[i];
			if ( !currentBlock->m_isWalkable )
			{
				continue;
			}

			// Get nearestPoint
			Vec3 alternativeNewPos	= currentBlock->m_aabb3.GetNearestPoint( idealNewPos );
			alternativeNewPos.z		= currentBlock->m_aabb3.m_maxs.z;

			// Determine the closest position as valid 
			float distRootToNewAlternativePos = GetDistance3D( alternativeNewPos, refLimb->m_jointPos_LS );
			if ( distRootToNewAlternativePos <= maxLength )
			{
				nearestPoint3D						= alternativeNewPos;
				distRootToPreviousAlternativePos	= distRootToNewAlternativePos;
			}
			else
			{
				// Keep track of the closest position to creature
				if ( distRootToNewAlternativePos <= distRootToPreviousAlternativePos )
				{
					nearestPoint3D						= alternativeNewPos;
					distRootToPreviousAlternativePos	= distRootToNewAlternativePos;
				}
			}
		}

		if ( nearestPoint3D == Vec3( 0.0f, 0.0f, -1000.0f ) )
		{
			nearestPoint3D = prevTargetPos;
		}

		// Set targetPos to nearestPoint
		targetPos = nearestPoint3D;
	}
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode3D::InitStepBezier( CubicBezierCurve3D& bezierCurve, IK_Chain3D* const skeleton, Stopwatch& bezierTimer )
{	
	if ( !bezierTimer.IsStopped() )
	{
		return;
	}

	// Calculate curve positions 
	Vec3 const& start			= skeleton->m_target.m_currentPos;
	Vec3 const& end				= skeleton->m_target.m_goalPos;
	Vec3		dispCurrentGoal	= end - start;
	Vec3		fwdNess1		= ( dispCurrentGoal * 0.33f );
	Vec3		fwdNess2		= ( dispCurrentGoal * 0.66f );
	float		maxLength		= skeleton->GetMaxLengthOfSkeleton();
	Vec3		ownerUpDir		= skeleton->m_ownerSkeletonFirstJoint->m_upDir;
	Vec3		upNess			= ( maxLength * 0.75f ) * ownerUpDir;						// #ToDo: Replace ( maxLength * 0.5f ) with values from raycast approach
//	Vec3		upNess			= ( maxLength * 0.25f ) * ownerUpDir;						// #ToDo: Replace ( maxLength * 0.5f ) with values from raycast approach
	Vec3		guide1			= start + fwdNess1 + upNess;
	Vec3		guide2			= start + fwdNess2 + upNess; 
	// Set curve positions
	bezierCurve.m_startPos	= start;
	bezierCurve.m_guidePos1	= guide1;
	bezierCurve.m_guidePos2	= guide2;
	bezierCurve.m_endPos	= end;
	// Start bezierTimer;
	bezierTimer.Start();
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode3D::UpdateBezier( CubicBezierCurve3D& bezierCurve, IK_Chain3D* const skeleton, Stopwatch& bezierTimer )
{
	if ( !skeleton->CanMove() )
	{
		return;
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Update Bezier Curve "t"
	//----------------------------------------------------------------------------------------------------------------------
	// Right Arm
	if ( bezierTimer.HasDurationElapsed() )
	{
		bezierTimer.Stop();
		skeleton->SetAnchor_Free();
	}
	else if ( bezierTimer.GetElapsedFraction() > 0.0f )
	{
//		float currentTime	= bezierTimer.GetElapsedTime();
//		currentTime			= RangeMapClamped( currentTime, 0.0f, bezierTimer.m_duration, 0.0f, 1.0f );
//		Vec3 bezierLerp						 = bezierCurve.GetPointAtTime( currentTime );
		Vec3 bezierLerp						 = bezierCurve.GetPointAtTime( bezierTimer.GetElapsedTime() );
		skeleton->m_target.m_currentPos = bezierLerp;
	}
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode3D::TurnCreatureTowardsCameraDir()
{
	if ( g_debugFreeFly_F1 )
	{
		return;
	} 

	Vec3 camFwdDir			=  m_gameMode3DWorldCamera.m_orientation.GetForwardDir_XFwd_YLeft_ZUp();
	float goalDegrees		= Atan2Degrees( m_moveFwdDir.y, m_moveFwdDir.x );	// #bipedHack
	float currentRootYaw	= 0.0f;
	if ( g_debugToggleLegs_2 )
	{
		currentRootYaw = m_root->m_eulerAngles_LS.m_yawDegrees;				// #QuadupedHack
	}
	else
	{
		currentRootYaw = m_quadruped->m_root->m_eulerAngles_LS.m_yawDegrees;
	}
	float currentHipYaw									= m_hip->m_firstJoint->m_eulerAngles_LS.m_yawDegrees;
	float newRootYawAngle								= GetTurnedTowardDegrees( currentRootYaw, goalDegrees, 8.0f );
	float newHipYawAngle								= GetTurnedTowardDegrees(  currentHipYaw, goalDegrees, 8.0f );
	if ( g_debugToggleLegs_2 )
	{
		m_root->m_eulerAngles_LS.m_yawDegrees = newRootYawAngle;				// #QuadupedHack
	}
	else
	{
		m_quadruped->m_root->m_eulerAngles_LS.m_yawDegrees = newRootYawAngle; 
	}
	m_hip->m_firstJoint->m_eulerAngles_LS.m_yawDegrees		= newHipYawAngle;
	if ( !m_headHasLookAtTarget )
	{
		m_head->m_firstJoint->m_eulerAngles_LS.m_yawDegrees		= newRootYawAngle;
	}

	if ( g_debugToggleLegs_2 )
	{
		m_root->m_eulerAngles_LS.GetAsVectors_XFwd_YLeft_ZUp( m_root->m_fwdDir, m_root->m_leftDir, m_root->m_upDir );			// #QuadupedHack
	}
	else
	{
		m_quadruped->m_root->m_eulerAngles_LS.GetAsVectors_XFwd_YLeft_ZUp( m_quadruped->m_root->m_fwdDir, m_quadruped->m_root->m_leftDir, m_quadruped->m_root->m_upDir );
	}
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode3D::InitializeEnvironment()
{
	//----------------------------------------------------------------------------------------------------------------------
	// WalkableObjects (Floors, environmental proxies)
	//----------------------------------------------------------------------------------------------------------------------
	// Floors
	m_blockList.emplace_back( m_floor_NE ); 
	m_blockList.emplace_back( m_floor_NW ); 
	m_blockList.emplace_back( m_floor_SE ); 
	m_blockList.emplace_back( m_floor_SW ); 
	// Boxes
	 m_box1_E->m_aabb3.SetCenterXYZ( Vec3( 300.0f,  -80.0f,  0.0f ) );
	m_box2_NE->m_aabb3.SetCenterXYZ( Vec3( 250.0f,  100.0f,  0.0f ) );
	m_box3_SE->m_aabb3.SetCenterXYZ( Vec3( 250.0f, -150.0f,  0.0f ) );
	m_box4_SE->m_aabb3.SetCenterXYZ( Vec3( 250.0f,  -80.0f, 10.0f ) );
	m_blockList.emplace_back( m_box1_E  ); 
	m_blockList.emplace_back( m_box2_NE );
	m_blockList.emplace_back( m_box3_SE );
	m_blockList.emplace_back( m_box4_SE );
	// Stairs
	 m_stairs1->m_aabb3.SetCenterXYZ( Vec3( 220.0f, -20.0f,  0.0f ) );
	 m_stairs2->m_aabb3.SetCenterXYZ( Vec3( 240.0f, -20.0f,  5.0f ) ); 
	 m_stairs3->m_aabb3.SetCenterXYZ( Vec3( 260.0f, -20.0f, 10.0f ) ); 
	 m_stairs4->m_aabb3.SetCenterXYZ( Vec3( 280.0f, -20.0f, 15.0f ) ); 
	 m_stairs5->m_aabb3.SetCenterXYZ( Vec3( 300.0f, -20.0f, 20.0f ) ); 
	 m_stairs6->m_aabb3.SetCenterXYZ( Vec3( 320.0f, -20.0f, 25.0f ) ); 
	 m_stairs7->m_aabb3.SetCenterXYZ( Vec3( 340.0f, -20.0f, 30.0f ) ); 
	 m_stairs8->m_aabb3.SetCenterXYZ( Vec3( 360.0f, -20.0f, 35.0f ) ); 
	 m_stairs9->m_aabb3.SetCenterXYZ( Vec3( 380.0f, -20.0f, 40.0f ) ); 
	m_stairs10->m_aabb3.SetCenterXYZ( Vec3( 400.0f, -20.0f, 45.0f ) ); 
	m_blockList.emplace_back( m_stairs1  );
	m_blockList.emplace_back( m_stairs2  );
	m_blockList.emplace_back( m_stairs3  );
	m_blockList.emplace_back( m_stairs4  );
	m_blockList.emplace_back( m_stairs5  );
	m_blockList.emplace_back( m_stairs6  );
	m_blockList.emplace_back( m_stairs7  );
	m_blockList.emplace_back( m_stairs8  );
	m_blockList.emplace_back( m_stairs9  );
	m_blockList.emplace_back( m_stairs10 );
	// Slopes
	m_slope_SE->m_aabb3.SetCenterXYZ( Vec3( 210.0f, -80.0f, 0.0f ) ); 
	m_blockList.emplace_back( m_slope_SE );
	// Elevator
	m_elevator_1->m_aabb3.SetCenterXYZ( Vec3(  50.0f, -270.0f, 0.0f ) ); 
	m_elevator_2->m_aabb3.SetCenterXYZ( Vec3( 200.0f, -270.0f, 0.0f ) ); 
	m_blockList.emplace_back( m_elevator_1 );
	m_blockList.emplace_back( m_elevator_2 );
	// Cliff
	m_cliff->m_aabb3.SetCenterXY( Vec3( 50.0f, 500.0f, 0.0f ) );
	m_cliff->m_aabb3.SetNewZ( 0.0f );
	m_blockList.emplace_back( m_cliff );

	m_map = new Map_GameMode3D( this );
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode3D::RenderEnvironment( std::vector<Vertex_PCU>& verts ) const
{
	// Floors
	AddVertsForAABB3D( verts, m_floor_NE->m_aabb3, Rgba8::DARK_YELLOW	); 
	AddVertsForAABB3D( verts, m_floor_NW->m_aabb3, Rgba8::DARKER_GRAY	); 
	AddVertsForAABB3D( verts, m_floor_SE->m_aabb3, Rgba8::DARKER_GRAY	); 
	AddVertsForAABB3D( verts, m_floor_SW->m_aabb3, Rgba8::INDIGO		); 
	// Boxes
	AddVertsForAABB3D( verts,   m_box1_E->m_aabb3, Rgba8::DARK_GREEN	); 
	AddVertsForAABB3D( verts,  m_box2_NE->m_aabb3, Rgba8::DARK_GREEN	);
	AddVertsForAABB3D( verts,  m_box3_SE->m_aabb3, Rgba8::DARK_GREEN	);
	AddVertsForAABB3D( verts,  m_box4_SE->m_aabb3, Rgba8::DARKER_RED	);
	// Stairs
	AddVertsForAABB3D( verts,  m_stairs1->m_aabb3, Rgba8::DARK_YELLOW ); 
	AddVertsForAABB3D( verts,  m_stairs2->m_aabb3, Rgba8::DARKER_GRAY );
	AddVertsForAABB3D( verts,  m_stairs3->m_aabb3, Rgba8::DARK_YELLOW );
	AddVertsForAABB3D( verts,  m_stairs4->m_aabb3, Rgba8::DARKER_GRAY );
	AddVertsForAABB3D( verts,  m_stairs5->m_aabb3, Rgba8::DARK_YELLOW );
	AddVertsForAABB3D( verts,  m_stairs6->m_aabb3, Rgba8::DARKER_GRAY );
	AddVertsForAABB3D( verts,  m_stairs7->m_aabb3, Rgba8::DARK_YELLOW );
	AddVertsForAABB3D( verts,  m_stairs8->m_aabb3, Rgba8::DARKER_GRAY );
	AddVertsForAABB3D( verts,  m_stairs9->m_aabb3, Rgba8::DARK_YELLOW );
	AddVertsForAABB3D( verts, m_stairs10->m_aabb3, Rgba8::DARKER_GRAY );
	// Slopes							
	AddVertsForAABB3D( verts, m_slope_SE->m_aabb3, Rgba8::PINK			);
	// Elevator
	AddVertsForAABB3D( verts, m_elevator_1->m_aabb3, Rgba8::PINK		);
	AddVertsForAABB3D( verts, m_elevator_2->m_aabb3, Rgba8::PINK		);
	// Cliff
	AddVertsForAABB3D( verts,	   m_cliff->m_aabb3, Rgba8::PINK		);
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode3D::InitializeTrees()
{
	//----------------------------------------------------------------------------------------------------------------------
	// Init tree creature
	//----------------------------------------------------------------------------------------------------------------------
	m_treeCreature = new CreatureBase( Vec3( 250.0f, 100.0f, 70.0f ) );
	m_treeCreature->CreateChildSkeletalSystem( "treeCreature1", m_treeCreature->m_root->m_jointPos_LS, nullptr, m_treeCreature );
	m_treeCreature->CreateLimbsForIKChain( "treeCreature1", 1.0f, 10.0f, JOINT_CONSTRAINT_TYPE_BALL_AND_SOCKET, FloatRange( 0.0f, 180.0f ) );
	m_treeCreature->CreateLimbsForIKChain( "treeCreature1", 2.0f, 10.0f, JOINT_CONSTRAINT_TYPE_BALL_AND_SOCKET, FloatRange( 0.0f,  75.0f ) );
	m_treeCreature->CreateLimbsForIKChain( "treeCreature1", 3.0f, 10.0f, JOINT_CONSTRAINT_TYPE_BALL_AND_SOCKET, FloatRange( 0.0f,  45.0f ) );
	m_treeCreature->CreateLimbsForIKChain( "treeCreature1", 4.0f, 10.0f, JOINT_CONSTRAINT_TYPE_BALL_AND_SOCKET, FloatRange( 0.0f,  25.0f ) );
	m_treeCreature->m_root->m_refVector		= Vec3( 0.0f, 0.0f, 1.0f );
	IK_Chain3D* treeCreature1			= m_treeCreature->GetSkeletonByName( "treeCreature1" );
	treeCreature1->m_firstJoint->m_refVector = m_treeCreature->m_root->m_refVector;

	//----------------------------------------------------------------------------------------------------------------------
	// Trees
	//----------------------------------------------------------------------------------------------------------------------
	m_treeBranch1 = new IK_Chain3D( "Tree1", Vec3(  50.0f, 150.0f,  1.0f ) );
	m_treeBranch2 = new IK_Chain3D( "Tree2", Vec3( 150.0f, 250.0f, 10.0f ) );
	m_treeBranch3 = new IK_Chain3D( "Tree3", Vec3( 150.0f, 150.0f,  1.0f ) );	

	// Create 10 segments for treeBranch1
	for ( int i = 0; i < m_numTreeSegments; i++ )
	{
		m_treeBranch2->CreateNewLimb( i, m_limbLength, Vec3::X_FWD, JOINT_CONSTRAINT_TYPE_HINGE );
		m_treeBranch2->m_jointList[i]->SetConstraints_YPR( FloatRange( 0.0f, 90.0f ), FloatRange( 0.0f, 0.0f ), FloatRange( 0.0f, 0.0f ) );
		m_treeBranch3->CreateNewLimb( i, m_limbLength ); 
	}
	m_treeBranch2->m_firstJoint->SetConstraints_YPR( FloatRange( -180.0f, 180.0f ), FloatRange( -180.0f, 180.0f ), FloatRange( -180.0f, 180.0f ) );
//	m_treeBranch2->m_firstJoint->m_fwdDir = Vec3( 1.0f, 0.0f, 0.0f );

	// Apply constraints to trees
	m_treeBranch1->CreateNewLimb( 0, m_limbLength, Vec3::X_FWD, JOINT_CONSTRAINT_TYPE_HINGE );
	m_treeBranch1->CreateNewLimb( 0, m_limbLength, Vec3::X_FWD, JOINT_CONSTRAINT_TYPE_HINGE );
	m_treeBranch1->m_jointList[1]->SetConstraints_YPR( FloatRange( 0.0f, 90.0f ), FloatRange( 0.0f, 0.0f ), FloatRange( 0.0f, 0.0f ) );
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode3D::RenderTrees( std::vector<Vertex_PCU>& verts ) const
{
	AddVertsForSphere3D( verts, m_treeDebugTargetPos, 3.0f, 8.0f, 8.0f, Rgba8::PURPLE );
	m_treeCreature->Render( verts, Rgba8::DARKER_GREEN, Rgba8::SUNSET_ORANGE );
	AddVertsForCylinderZ3D( verts, Vec2( m_treeCreature->m_root->m_jointPos_LS.x, m_treeCreature->m_root->m_jointPos_LS.y ), FloatRange( 0.0f, 70.0f ), 3.0f, 16.0f );
//	m_treeBranch1->Render( verts, Rgba8::DARK_GREEN, Rgba8::BROWN );
	m_treeBranch2->Render( verts, Rgba8::DARK_GREEN, Rgba8::BROWN );
	m_treeBranch3->Render( verts, Rgba8::DARK_GREEN, Rgba8::BROWN );

	// Render IJK basis for each joint
	for ( int i = 0; i < m_treeBranch2->m_jointList.size(); i++ )
	{
		float endPosLength			= 3.0f;
		IK_Joint3D* currentLimb	= m_treeBranch2->m_jointList[i];
		Vec3 fwdPos					= currentLimb->m_jointPos_LS + ( currentLimb->m_fwdDir * endPosLength );
		Vec3 leftDir				= currentLimb->m_fwdDir.GetRotatedAboutZDegrees( 90.0f );
		leftDir						= leftDir.GetNormalized();
		Vec3 leftPos				= currentLimb->m_jointPos_LS + ( leftDir * endPosLength );
		Vec3 upDir					= CrossProduct3D( currentLimb->m_fwdDir, leftDir );
		upDir						= upDir.GetNormalized();
		Vec3 upPos					= currentLimb->m_jointPos_LS + ( upDir * endPosLength );
		AddVertsForArrow3D( verts, currentLimb->m_jointPos_LS,  fwdPos, 0.1f, Rgba8::RED		);		// Fwd
		AddVertsForArrow3D( verts, currentLimb->m_jointPos_LS,	leftPos, 0.1f, Rgba8::GREEN		);		// Left
		AddVertsForArrow3D( verts, currentLimb->m_jointPos_LS,	  upPos, 0.1f, Rgba8::BLUE		);		// Up
	}
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode3D::UpdateTreesToReachOutToPlayer( float deltaSeconds )
{
	IK_Chain3D* treeCreature1 = m_treeCreature->GetSkeletonByName( "treeCreature1" );
	if ( g_debugToggleLegs_2 )
	{
		treeCreature1->m_target.m_currentPos = m_root->m_jointPos_LS;				// #QuadupedHack
	}
	else
	{
		treeCreature1->m_target.m_currentPos = m_quadruped->m_root->m_jointPos_LS;	 
	}

	m_treeBranch1->m_target.m_goalPos  = m_treeDebugTargetPos;
//	m_treeBranch2->m_target.m_goalPos  = m_treeDebugTargetPos;
//	m_treeBranch3->m_target.m_goalPos  = m_treeDebugTargetPos;

//	m_treeBranch1->m_target.m_goalPos = m_root->m_jointPos_LS;
	m_treeBranch2->m_target.m_goalPos = m_root->m_jointPos_LS;
	m_treeBranch3->m_target.m_goalPos = m_root->m_jointPos_LS;

	m_treeBranch1->m_target.m_currentPos = Interpolate( m_treeBranch1->m_target.m_currentPos, m_treeBranch1->m_target.m_goalPos, deltaSeconds * 2.0f );
//	m_treeBranch2->m_target.m_currentPos = Interpolate( m_treeBranch2->m_target.m_currentPos, m_treeBranch2->m_target.m_goalPos, deltaSeconds * 2.0f );
	m_treeBranch3->m_target.m_currentPos = Interpolate( m_treeBranch3->m_target.m_currentPos, m_treeBranch3->m_target.m_goalPos, deltaSeconds * 2.0f );

	m_treeBranch2->m_target.m_currentPos = m_treeBranch2->m_target.m_goalPos;
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode3D::RenderRaycasts( std::vector<Vertex_PCU>& verts ) const
{
	if ( !g_debugRenderRaycast_F2 )
	{
		return;
	}

	//----------------------------------------------------------------------------------------------------------------------
	// DebugRenderRaycast for Quadruped Creature
	//----------------------------------------------------------------------------------------------------------------------
	m_quadruped->DebugRenderRaycasts( verts );

	//----------------------------------------------------------------------------------------------------------------------
	// Mount raycast
	//----------------------------------------------------------------------------------------------------------------------
	// Check if raycast impacted a line		// If true, render raycast impactNormal, impactDist, and ray after hit 
//	if ( m_didRayImpact_MountHeight )
	if ( m_didRayImpact_FWD )
	{
		// Ray until impact			// RayStart to rayEndPoint
		AddVertsForArrow3D( verts, m_raycast_Mount.m_rayStartPos, m_rayEndPos_FWD, m_arrowThickness, m_rayImpactDistColor );
		// Disc at raycast impactPos 
		AddVertsForSphere3D( verts,m_raycast_Mount.m_updatedImpactPos, ( m_arrowThickness * 1.0f ), 8.0f, 8.0f, m_rayImpactDiscColor );
		// Impact normal			// raycast reflection off line 
		AddVertsForArrow3D( verts, m_raycast_Mount.m_updatedImpactPos, m_raycast_Mount.m_updatedImpactPos + ( m_raycast_Mount.m_updatedImpactNormal * 4.0f ), ( m_arrowThickness * 1.0f ), m_rayImpactNormalColor );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Fwd raycast
	//----------------------------------------------------------------------------------------------------------------------
	// Check if raycast impacted a line		// If true, render raycast impactNormal, impactDist, and ray after hit 
	if ( m_didRayImpact_FWD )
	{
		// Ray until impact			// RayStart to rayEndPoint
		AddVertsForArrow3D( verts, m_rayStartPos_FWD, m_rayEndPos_FWD, m_arrowThickness, m_rayImpactDistColor );
		// Disc at raycast impactPos 
		AddVertsForSphere3D( verts, m_updatedImpactPos_FWD, ( m_arrowThickness * 1.0f ), 8.0f, 8.0f, m_rayImpactDiscColor );
		// Impact normal			// raycast reflection off line 
		AddVertsForArrow3D( verts, m_updatedImpactPos_FWD, m_updatedImpactPos_FWD + ( m_updatedImpactNormal_FWD * 4.0f ), ( m_arrowThickness * 1.0f ), m_rayImpactNormalColor );
	}
	else if ( m_didRayImpactClimbableObject_FWD )
	{
		// Ray until impact			// RayStart to rayEndPoint
		AddVertsForArrow3D( verts, m_rayStartPos_FWD, m_rayEndPos_FWD, m_arrowThickness, m_rayImpactDistColor2 );
		// Disc at raycast impactPos 
		AddVertsForSphere3D( verts, m_updatedImpactPos_FWD, ( m_arrowThickness * 1.0f ), 8.0f, 8.0f, m_rayImpactDiscColor );
		// Impact normal			// raycast reflection off line 
		AddVertsForArrow3D( verts, m_updatedImpactPos_FWD, m_updatedImpactPos_FWD + ( m_updatedImpactNormal_FWD * 4.0f ), ( m_arrowThickness * 1.0f ), m_rayImpactNormalColor );

	}
	else
	{
		// Ray start to end with no impact
		AddVertsForArrow3D( verts, m_rayStartPos_FWD, m_rayEndPos_FWD, m_arrowThickness, m_rayDefaultColor );
	}
	//----------------------------------------------------------------------------------------------------------------------
	// Right Arm
	//----------------------------------------------------------------------------------------------------------------------
	// RAD (Right Arm Down)
	if ( m_raycast_rightArmDown.m_didRayImpact )
	{
		// Check if raycast impacted a line		
		// If true, render raycast impactNormal, impactDist, and ray after hit 
		// Ray until impact			// RayStart to rayEndPoint
		AddVertsForArrow3D( verts, m_raycast_rightArmDown.m_rayStartPos, m_raycast_rightArmDown.m_rayEndPos, m_arrowThickness, m_rayImpactDistColor );
		// Disc at raycast impactPos 
		AddVertsForSphere3D( verts, m_raycast_rightArmDown.m_updatedImpactPos, ( m_arrowThickness * 1.0f ), 8.0f, 8.0f, m_rayImpactDiscColor );
		// Impact normal			// raycast reflection off line 
		AddVertsForArrow3D( verts, m_raycast_rightArmDown.m_updatedImpactPos, m_raycast_rightArmDown.m_updatedImpactPos + ( m_raycast_rightArmDown.m_updatedImpactNormal * 4.0f ), ( m_arrowThickness * 1.0f ), m_rayImpactNormalColor );
	}
	else
	{
		// Ray start to end with no impact
		AddVertsForArrow3D( verts, m_raycast_rightArmDown.m_rayStartPos, m_raycast_rightArmDown.m_rayEndPos, m_arrowThickness, m_rayDefaultColor );
	}
	// RAF (Right Arm Forward)
	if ( m_raycast_rightArmFwd.m_didRayImpact )
	{
		// Check if raycast impacted a line		
		// If true, render raycast impactNormal, impactDist, and ray after hit 
		// Ray until impact			// RayStart to rayEndPoint
		AddVertsForArrow3D( verts, m_raycast_rightArmFwd.m_rayStartPos, m_raycast_rightArmFwd.m_rayEndPos, m_arrowThickness, m_rayImpactDistColor );
		// Disc at raycast impactPos 
		AddVertsForSphere3D( verts, m_raycast_rightArmFwd.m_updatedImpactPos, ( m_arrowThickness * 1.0f ), 8.0f, 8.0f, m_rayImpactDiscColor );
		// Impact normal			// raycast reflection off line 
		AddVertsForArrow3D( verts, m_raycast_rightArmFwd.m_updatedImpactPos, m_raycast_rightArmFwd.m_updatedImpactPos + ( m_raycast_rightArmFwd.m_updatedImpactNormal * 4.0f ), ( m_arrowThickness * 1.0f ), m_rayImpactNormalColor );
	}
	else
	{
		// Ray start to end with no impact
		AddVertsForArrow3D( verts, m_raycast_rightArmFwd.m_rayStartPos, m_raycast_rightArmFwd.m_rayEndPos, m_arrowThickness, m_rayDefaultColor );
	}
	// NRAD (Next Right Arm Down)
	if ( m_raycast_NextRightArmDown.m_didRayImpact )
	{
		// Check if raycast impacted a line		
		// If true, render raycast impactNormal, impactDist, and ray after hit 
		// Ray until impact			// RayStart to rayEndPoint
		AddVertsForArrow3D( verts, m_raycast_NextRightArmDown.m_rayStartPos, m_raycast_NextRightArmDown.m_rayEndPos, m_arrowThickness, m_rayImpactDistColor );
		// Disc at raycast impactPos 
		AddVertsForSphere3D( verts, m_raycast_NextRightArmDown.m_updatedImpactPos, ( m_arrowThickness * 1.0f ), 8.0f, 8.0f, m_rayImpactDiscColor );
		// Impact normal			// raycast reflection off line 
		AddVertsForArrow3D( verts, m_raycast_NextRightArmDown.m_updatedImpactPos, m_raycast_NextRightArmDown.m_updatedImpactPos + ( m_raycast_NextRightArmDown.m_updatedImpactNormal * 4.0f ), ( m_arrowThickness * 1.0f ), m_rayImpactNormalColor );
	}
	else
	{
		// Ray start to end with no impact
		AddVertsForArrow3D( verts, m_raycast_NextRightArmDown.m_rayStartPos, m_raycast_NextRightArmDown.m_rayEndPos, m_arrowThickness, m_rayDefaultColor );
	}
	
	//----------------------------------------------------------------------------------------------------------------------
	// Left Arm
	//----------------------------------------------------------------------------------------------------------------------
	// LAD (Left Arm Down)
	if ( m_raycast_LeftArmDown.m_didRayImpact )
	{
		// Check if raycast impacted a line		// If true, render raycast impactNormal, impactDist, and ray after hit 
		// Ray until impact			// RayStart to rayEndPoint
		AddVertsForArrow3D( verts, m_raycast_LeftArmDown.m_rayStartPos, m_raycast_LeftArmDown.m_rayEndPos, m_arrowThickness, m_rayImpactDistColor );
		// Disc at raycast impactPos 
		AddVertsForSphere3D( verts, m_raycast_LeftArmDown.m_updatedImpactPos, ( m_arrowThickness * 1.0f ), 8.0f, 8.0f, m_rayImpactDiscColor );
		// Impact normal			// raycast reflection off line 
		AddVertsForArrow3D( verts, m_raycast_LeftArmDown.m_updatedImpactPos, m_raycast_LeftArmDown.m_updatedImpactPos + ( m_raycast_LeftArmDown.m_updatedImpactNormal * 4.0f ), ( m_arrowThickness * 1.0f ), m_rayImpactNormalColor );
	}
	else
	{
		// Ray start to end with no impact
		AddVertsForArrow3D( verts, m_raycast_LeftArmDown.m_rayStartPos, m_raycast_LeftArmDown.m_rayEndPos, m_arrowThickness, m_rayDefaultColor );
	}
	// LAF (Left Arm Forward)
	if ( m_raycast_LeftArmFwd.m_didRayImpact )
	{
		// Check if raycast impacted a line		// If true, render raycast impactNormal, impactDist, and ray after hit 
		// Ray until impact			// RayStart to rayEndPoint
		AddVertsForArrow3D( verts, m_raycast_LeftArmFwd.m_rayStartPos, m_raycast_LeftArmFwd.m_rayEndPos, m_arrowThickness, m_rayImpactDistColor );
		// Disc at raycast impactPos 
		AddVertsForSphere3D( verts, m_raycast_LeftArmFwd.m_updatedImpactPos, ( m_arrowThickness * 1.0f ), 8.0f, 8.0f, m_rayImpactDiscColor );
		// Impact normal			// raycast reflection off line 
		AddVertsForArrow3D( verts, m_raycast_LeftArmFwd.m_updatedImpactPos, m_raycast_LeftArmFwd.m_updatedImpactPos + ( m_raycast_LeftArmFwd.m_updatedImpactNormal * 4.0f ), ( m_arrowThickness * 1.0f ), m_rayImpactNormalColor );
	}
	else
	{
		// Ray start to end with no impact
		AddVertsForArrow3D( verts, m_raycast_LeftArmFwd.m_rayStartPos, m_raycast_LeftArmFwd.m_rayEndPos, m_arrowThickness, m_rayDefaultColor );
	}
	// NLAD (Next Left Arm Down)
	if ( m_raycast_NextLeftArmDown.m_didRayImpact )
	{
		// Check if raycast impacted a line		// If true, render raycast impactNormal, impactDist, and ray after hit 
		// Ray until impact			// RayStart to rayEndPoint
		AddVertsForArrow3D( verts, m_raycast_NextLeftArmDown.m_rayStartPos, m_raycast_NextLeftArmDown.m_rayEndPos, m_arrowThickness, m_rayImpactDistColor );
		// Disc at raycast impactPos 
		AddVertsForSphere3D( verts, m_raycast_NextLeftArmDown.m_updatedImpactPos, ( m_arrowThickness * 1.0f ), 8.0f, 8.0f, m_rayImpactDiscColor );
		// Impact normal			// raycast reflection off line 
		AddVertsForArrow3D( verts, m_raycast_NextLeftArmDown.m_updatedImpactPos, m_raycast_NextLeftArmDown.m_updatedImpactPos + ( m_raycast_NextLeftArmDown.m_updatedImpactNormal * 4.0f ), ( m_arrowThickness * 1.0f ), m_rayImpactNormalColor );
	}
	else
	{
		// Ray start to end with no impact
		AddVertsForArrow3D( verts, m_raycast_NextLeftArmDown.m_rayStartPos, m_raycast_NextLeftArmDown.m_rayEndPos, m_arrowThickness, m_rayDefaultColor );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Left Foot
	//----------------------------------------------------------------------------------------------------------------------
	// LFD (Left Foot Down)
	if ( m_raycast_leftFootDown.m_didRayImpact )
	{
		// Check if raycast impacted a line		// If true, render raycast impactNormal, impactDist, and ray after hit 
		// Ray until impact			// RayStart to rayEndPoint
		AddVertsForArrow3D( verts, m_raycast_leftFootDown.m_rayStartPos, m_raycast_leftFootDown.m_rayEndPos, m_arrowThickness, m_rayImpactDistColor );
		// Disc at raycast impactPos 
		AddVertsForSphere3D( verts, m_raycast_leftFootDown.m_updatedImpactPos, ( m_arrowThickness * 1.0f ), 8.0f, 8.0f, m_rayImpactDiscColor );
		// Impact normal			// raycast reflection off line 
		AddVertsForArrow3D( verts, m_raycast_leftFootDown.m_updatedImpactPos, m_raycast_leftFootDown.m_updatedImpactPos+ ( m_raycast_leftFootDown.m_updatedImpactNormal * 4.0f ), ( m_arrowThickness * 1.0f ), m_rayImpactNormalColor );
	}
	else
	{
		// Ray start to end with no impact
		AddVertsForArrow3D( verts, m_raycast_leftFootDown.m_rayStartPos, m_raycast_leftFootDown.m_rayEndPos, m_arrowThickness, m_rayDefaultColor );
	}
	// LAF (Left Foot Forward)
	if ( m_raycast_LeftFootFwd.m_didRayImpact )
	{
		// Check if raycast impacted a line		// If true, render raycast impactNormal, impactDist, and ray after hit 
		// Ray until impact			// RayStart to rayEndPoint
		AddVertsForArrow3D( verts, m_raycast_LeftFootFwd.m_rayStartPos, m_raycast_LeftFootFwd.m_rayEndPos, m_arrowThickness, m_rayImpactDistColor );
		// Disc at raycast impactPos 
		AddVertsForSphere3D( verts, m_raycast_LeftFootFwd.m_updatedImpactPos, ( m_arrowThickness * 1.0f ), 8.0f, 8.0f, m_rayImpactDiscColor );
		// Impact normal			// raycast reflection off line 
		AddVertsForArrow3D( verts, m_raycast_LeftFootFwd.m_updatedImpactPos, m_raycast_LeftFootFwd.m_updatedImpactPos + ( m_raycast_LeftFootFwd.m_updatedImpactNormal * 4.0f ), ( m_arrowThickness * 1.0f ), m_rayImpactNormalColor );
	}
	else
	{
		// Ray start to end with no impact
		AddVertsForArrow3D( verts, m_raycast_LeftFootFwd.m_rayStartPos, m_raycast_LeftFootFwd.m_rayEndPos, m_arrowThickness, m_rayDefaultColor );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Right Foot
	//----------------------------------------------------------------------------------------------------------------------
	// RAD (Right Foot Down)
	if ( m_raycast_rightFootDown.m_didRayImpact )
	{
		// Check if raycast impacted a line		// If true, render raycast impactNormal, impactDist, and ray after hit 
		// Ray until impact			// RayStart to rayEndPoint
		AddVertsForArrow3D( verts, m_raycast_rightFootDown.m_rayStartPos, m_raycast_rightFootDown.m_rayEndPos, m_arrowThickness, m_rayImpactDistColor );
		// Disc at raycast impactPos 
		AddVertsForSphere3D( verts, m_raycast_rightFootDown.m_updatedImpactPos, ( m_arrowThickness * 1.0f ), 8.0f, 8.0f, m_rayImpactDiscColor );
		// Impact normal			// raycast reflection off line 
		AddVertsForArrow3D( verts, m_raycast_rightFootDown.m_updatedImpactPos, m_raycast_rightFootDown.m_updatedImpactPos + ( m_raycast_rightFootDown.m_updatedImpactNormal * 4.0f ), ( m_arrowThickness * 1.0f ), m_rayImpactNormalColor );
	}
	else
	{
		// Ray start to end with no impact
		AddVertsForArrow3D( verts, m_raycast_rightFootDown.m_rayStartPos, m_raycast_rightFootDown.m_rayEndPos, m_arrowThickness, m_rayDefaultColor );
	}
	// RAF (Right Foot Forward)
	if ( m_raycast_rightFootFwd.m_didRayImpact )
	{
		// Check if raycast impacted a line		// If true, render raycast impactNormal, impactDist, and ray after hit 
		// Ray until impact			// RayStart to rayEndPoint
		AddVertsForArrow3D( verts, m_raycast_rightFootFwd.m_rayStartPos, m_raycast_rightFootFwd.m_rayEndPos, m_arrowThickness, m_rayImpactDistColor );
		// Disc at raycast impactPos 
		AddVertsForSphere3D( verts, m_raycast_rightFootFwd.m_updatedImpactPos, ( m_arrowThickness * 1.0f ), 8.0f, 8.0f, m_rayImpactDiscColor );
		// Impact normal			// raycast reflection off line 
		AddVertsForArrow3D( verts, m_raycast_rightFootFwd.m_updatedImpactPos, m_raycast_rightFootFwd.m_updatedImpactPos + ( m_raycast_rightFootFwd.m_updatedImpactNormal * 4.0f ), ( m_arrowThickness * 1.0f ), m_rayImpactNormalColor );
	}
	else
	{
		// Ray start to end with no impact
		AddVertsForArrow3D( verts, m_raycast_rightFootFwd.m_rayStartPos, m_raycast_rightFootFwd.m_rayEndPos, m_arrowThickness, m_rayDefaultColor );
	}
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode3D::UpdateRaycastResult3D()
{
	//----------------------------------------------------------------------------------------------------------------------
	// FWD raycast
	//----------------------------------------------------------------------------------------------------------------------
	m_rayStartPos_FWD						= m_root->m_jointPos_LS;
	Vec3 rootFwdDir							= m_root->m_eulerAngles_LS.GetForwardDir_XFwd_YLeft_ZUp();
	m_rayEndPos_FWD							= m_rayStartPos_FWD + ( rootFwdDir * m_raylength_Long );
	Vec3 rayfwdNormal_FWD					= ( m_rayEndPos_FWD - m_rayStartPos_FWD ).GetNormalized();
	m_didRayImpact_FWD						= DidRaycastHitWalkableBlock( m_raycastResult_FWD, m_rayStartPos_FWD, rayfwdNormal_FWD, m_raylength_Long, m_updatedImpactPos_FWD, m_updatedImpactNormal_FWD );
	m_didRayImpactClimbableObject_FWD		= DidRaycastHitClimbableBlock( m_raycastResult_FWD, m_rayStartPos_FWD, rayfwdNormal_FWD, m_raylength_Long, m_updatedImpactPos_FWD, m_updatedImpactNormal_FWD );

	//----------------------------------------------------------------------------------------------------------------------
	// Mount height raycast
	//----------------------------------------------------------------------------------------------------------------------
	Vec3 heightRayZOffset					= Vec3( 0.0f, 0.0f, 15.0f );
	m_raycast_Mount.m_rayStartPos			= m_updatedImpactPos_FWD + ( rootFwdDir * 5.0f ) + heightRayZOffset;
	m_raycast_Mount.m_rayEndPos				= m_raycast_Mount.m_rayStartPos + ( Vec3( 0.0f, 0.0f, -1.0f ) * m_raylength_Long );
	Vec3 rayfwdNormal_MountHeight			= ( m_raycast_Mount.m_rayEndPos - m_raycast_Mount.m_rayStartPos ).GetNormalized();
	m_raycast_Mount.m_didRayImpact			= DidRaycastHitWalkableBlock( m_raycast_Mount.m_raycastResult, m_raycast_Mount.m_rayStartPos, rayfwdNormal_MountHeight, m_raylength_Long, m_raycast_Mount.m_updatedImpactPos, m_raycast_Mount.m_updatedImpactNormal );

	if ( m_raycast_Mount.m_didRayImpact )
	{
		m_mountGoalPos = m_raycast_Mount.m_updatedImpactPos;
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Right Arm
	//----------------------------------------------------------------------------------------------------------------------
	// RAD (Right Arm Down)
	m_raycast_rightArmDown.m_rayStartPos		= m_rightArm->m_finalJoint->m_endPos + Vec3( 0.0f, 0.0f, 5.0f );
	m_raycast_rightArmDown.m_rayEndPos			= m_raycast_rightArmDown.m_rayStartPos + ( Vec3( 0.0f, 0.0f, -1.0f ) * m_raylength_Long );
	Vec3 rayfwdNormal_RAD						= ( m_raycast_rightArmDown.m_rayEndPos - m_raycast_rightArmDown.m_rayStartPos ).GetNormalized();
	m_raycast_rightArmDown.m_didRayImpact		= DidRaycastHitWalkableBlock( m_raycast_rightArmDown.m_raycastResult, m_raycast_rightArmDown.m_rayStartPos, rayfwdNormal_RAD, m_raylength_Long, m_raycast_rightArmDown.m_updatedImpactPos, m_raycast_rightArmDown.m_updatedImpactNormal );
	
	// RAF (Right Arm Forward)
	m_raycast_rightArmFwd.m_rayStartPos			= m_rightArm->m_finalJoint->m_jointPos_LS;
	m_raycast_rightArmFwd.m_rayEndPos			= m_raycast_rightArmFwd.m_rayStartPos + ( rootFwdDir * m_raylength_Short );
	Vec3 rayfwdNormal_RAF						= ( m_raycast_rightArmFwd.m_rayEndPos - m_raycast_rightArmFwd.m_rayStartPos ).GetNormalized();
	m_raycast_rightArmFwd.m_didRayImpact		= DidRaycastHitWalkableBlock( m_raycast_rightArmFwd.m_raycastResult, m_raycast_rightArmFwd.m_rayStartPos, rayfwdNormal_RAF, m_raylength_Short, m_raycast_rightArmFwd.m_updatedImpactPos, m_raycast_rightArmFwd.m_updatedImpactNormal );
	
	// NRAD (Next Right Arm Down)
	m_raycast_NextRightArmDown.m_rayStartPos	= m_debugGoalPos_RA;
	m_raycast_NextRightArmDown.m_rayEndPos		= m_raycast_NextRightArmDown.m_rayStartPos + ( Vec3( 0.0f, 0.0f, -1.0f ) * m_raylength_Long );
	Vec3 rayfwdNormal_NRAD						= ( m_raycast_NextRightArmDown.m_rayEndPos - m_raycast_NextRightArmDown.m_rayStartPos ).GetNormalized();
	m_raycast_NextRightArmDown.m_didRayImpact	= DidRaycastHitWalkableBlock( m_raycast_NextRightArmDown.m_raycastResult, m_raycast_NextRightArmDown.m_rayStartPos, rayfwdNormal_NRAD, m_raylength_Long, m_raycast_NextRightArmDown.m_updatedImpactPos, m_raycast_NextRightArmDown.m_updatedImpactNormal );
	
	//----------------------------------------------------------------------------------------------------------------------
	// Left Arm
	//----------------------------------------------------------------------------------------------------------------------
	// LAD (Left Arm Down)
	m_raycast_LeftArmDown.m_rayStartPos			= m_leftArm->m_finalJoint->m_endPos + Vec3( 0.0f, 0.0f, 5.0f );
	m_raycast_LeftArmDown.m_rayEndPos			= m_raycast_LeftArmDown.m_rayStartPos + ( Vec3( 0.0f, 0.0f, -1.0f ) * m_raylength_Long );
	Vec3 rayfwdNormal_LA						= ( m_raycast_LeftArmDown.m_rayEndPos - m_raycast_LeftArmDown.m_rayStartPos ).GetNormalized();
	m_raycast_LeftArmDown.m_didRayImpact		= DidRaycastHitWalkableBlock( m_raycast_LeftArmDown.m_raycastResult, m_raycast_LeftArmDown.m_rayStartPos, rayfwdNormal_LA, m_raylength_Long, m_raycast_LeftArmDown.m_updatedImpactPos, m_raycast_LeftArmDown.m_updatedImpactNormal );
	// LAF (Left Arm Forward)
	m_raycast_LeftArmFwd.m_rayStartPos			= m_leftArm->m_finalJoint->m_jointPos_LS;
	m_raycast_LeftArmFwd.m_rayEndPos			= m_raycast_LeftArmFwd.m_rayStartPos + ( rootFwdDir * m_raylength_Short );
	Vec3 rayfwdNormal_LAF						= ( m_raycast_LeftArmFwd.m_rayEndPos - m_raycast_LeftArmDown.m_rayStartPos ).GetNormalized();
	m_raycast_LeftArmFwd.m_didRayImpact			= DidRaycastHitWalkableBlock( m_raycast_LeftArmFwd.m_raycastResult, m_raycast_LeftArmFwd.m_rayStartPos, rayfwdNormal_LAF, m_raylength_Short, m_raycast_LeftArmFwd.m_updatedImpactPos, m_raycast_LeftArmFwd.m_updatedImpactNormal );
	// LRAD (Next Left Arm Down)
	m_raycast_NextLeftArmDown.m_rayStartPos		= m_debugGoalPos_LA;
	m_raycast_NextLeftArmDown.m_rayEndPos		= m_raycast_NextLeftArmDown.m_rayStartPos + ( Vec3( 0.0f, 0.0f, -1.0f ) * m_raylength_Long );
	Vec3 rayfwdNormal_NLAD						= ( m_raycast_NextLeftArmDown.m_rayEndPos - m_raycast_NextLeftArmDown.m_rayStartPos ).GetNormalized();
	m_raycast_NextLeftArmDown.m_didRayImpact	= DidRaycastHitWalkableBlock( m_raycast_NextLeftArmDown.m_raycastResult, m_raycast_NextLeftArmDown.m_rayStartPos, rayfwdNormal_NLAD, m_raylength_Long, m_raycast_NextLeftArmDown.m_updatedImpactPos, m_raycast_NextLeftArmDown.m_updatedImpactNormal );

	//----------------------------------------------------------------------------------------------------------------------
	// Right Foot
	//----------------------------------------------------------------------------------------------------------------------
	// RFD (Right Foot Down)
	m_raycast_rightFootDown.m_rayStartPos		= m_rightFoot->m_finalJoint->m_endPos + Vec3( 0.0f, 0.0f, 5.0f );
	m_raycast_rightFootDown.m_rayEndPos			= m_raycast_rightFootDown.m_rayStartPos + ( Vec3( 0.0f, 0.0f, -1.0f ) * m_raylength_Long );
	Vec3 rayfwdNormal_RFD						= ( m_raycast_rightFootDown.m_rayEndPos - m_raycast_rightFootDown.m_rayStartPos ).GetNormalized();
	m_raycast_rightFootDown.m_didRayImpact		= DidRaycastHitWalkableBlock( m_raycast_rightFootDown.m_raycastResult, m_raycast_rightFootDown.m_rayStartPos, rayfwdNormal_RFD, m_raylength_Long, m_raycast_rightFootDown.m_updatedImpactPos, m_raycast_rightFootDown.m_updatedImpactNormal );

	// RFF (Right Foot Forward)	
	m_raycast_rightFootFwd.m_rayStartPos		= m_rightFoot->m_finalJoint->m_jointPos_LS;
	m_raycast_rightFootFwd.m_rayEndPos			= m_raycast_rightFootFwd.m_rayStartPos + ( rootFwdDir * m_raylength_Short );
	Vec3 rayfwdNormal_RFF						= ( m_raycast_rightFootFwd.m_rayEndPos - m_raycast_rightFootFwd.m_rayStartPos ).GetNormalized();
	m_raycast_rightFootFwd.m_didRayImpact		= DidRaycastHitWalkableBlock( m_raycast_rightFootFwd.m_raycastResult, m_raycast_rightFootFwd.m_rayStartPos, rayfwdNormal_RFF, m_raylength_Short, m_raycast_rightFootFwd.m_updatedImpactPos, m_raycast_rightFootFwd.m_updatedImpactNormal );

	//----------------------------------------------------------------------------------------------------------------------
	// Left Foot
	//----------------------------------------------------------------------------------------------------------------------
	// LFD (Left Foot Down)
	m_raycast_leftFootDown.m_rayStartPos		= m_leftFoot->m_finalJoint->m_endPos + Vec3( 0.0f, 0.0f, 5.0f );
	m_raycast_leftFootDown.m_rayEndPos			= m_raycast_leftFootDown.m_rayStartPos + ( Vec3( 0.0f, 0.0f, -1.0f ) * m_raylength_Long );
	Vec3 rayfwdNormal_LFD						= ( m_raycast_leftFootDown.m_rayEndPos - m_raycast_leftFootDown.m_rayStartPos ).GetNormalized();
	m_raycast_leftFootDown.m_didRayImpact		= DidRaycastHitWalkableBlock( m_raycast_leftFootDown.m_raycastResult, m_raycast_leftFootDown.m_rayStartPos, rayfwdNormal_LFD, m_raylength_Long, m_raycast_leftFootDown.m_updatedImpactPos, m_raycast_leftFootDown.m_updatedImpactNormal );

	// LAF (Left Foot Forward)	
	m_raycast_LeftFootFwd.m_rayStartPos			= m_leftFoot->m_finalJoint->m_jointPos_LS;
	m_raycast_LeftFootFwd.m_rayEndPos			= m_raycast_LeftFootFwd.m_rayStartPos + ( rootFwdDir * m_raylength_Short );
	Vec3 rayfwdNormal_LFF						= ( m_raycast_LeftFootFwd.m_rayEndPos - m_raycast_LeftFootFwd.m_rayStartPos ).GetNormalized();
	m_raycast_LeftFootFwd.m_didRayImpact		= DidRaycastHitWalkableBlock( m_raycast_LeftFootFwd.m_raycastResult, m_raycast_LeftFootFwd.m_rayStartPos, rayfwdNormal_LFF, m_raylength_Short, m_raycast_LeftFootFwd.m_updatedImpactPos, m_raycast_LeftFootFwd.m_updatedImpactNormal );
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode3D::MoveRaycastInput( float deltaSeconds )
{
	float moveSpeed = 10.0f;

	//----------------------------------------------------------------------------------------------------------------------
	// Move entire raycast
	//----------------------------------------------------------------------------------------------------------------------
	// Move North
	if ( g_theInput->IsKeyDown( 'J' ) )
	{
		m_raycast_rightArmDown.m_rayStartPos += Vec3( 0.0f, moveSpeed, 0.0f ) * deltaSeconds; 
		m_raycast_rightArmDown.m_rayEndPos   += Vec3( 0.0f, moveSpeed, 0.0f ) * deltaSeconds;
	}
	// Move South
	if ( g_theInput->IsKeyDown( 'L' ) )
	{
		m_raycast_rightArmDown.m_rayStartPos += Vec3( 0.0f, -moveSpeed, 0.0f ) * deltaSeconds;
		m_raycast_rightArmDown.m_rayEndPos   += Vec3( 0.0f, -moveSpeed, 0.0f ) * deltaSeconds;
	}
	// Move East
	if ( g_theInput->IsKeyDown( 'I' ) )
	{
		m_raycast_rightArmDown.m_rayStartPos += Vec3( moveSpeed, 0.0f, 0.0f ) * deltaSeconds;
		m_raycast_rightArmDown.m_rayEndPos   += Vec3( moveSpeed, 0.0f, 0.0f ) * deltaSeconds;
	}
	// Move West
	if ( g_theInput->IsKeyDown( 'K' ) )
	{
		m_raycast_rightArmDown.m_rayStartPos += Vec3( -moveSpeed, 0.0f, 0.0f ) * deltaSeconds;
		m_raycast_rightArmDown.m_rayEndPos   += Vec3( -moveSpeed, 0.0f, 0.0f ) * deltaSeconds;
	}
	// Move Skyward
	if ( g_theInput->IsKeyDown( 'O' ) )
	{
		m_raycast_rightArmDown.m_rayStartPos += Vec3( 0.0f, 0.0f, moveSpeed ) * deltaSeconds;
		m_raycast_rightArmDown.m_rayEndPos   += Vec3( 0.0f, 0.0f, moveSpeed ) * deltaSeconds;
	}
	// Move Groundward
	if ( g_theInput->IsKeyDown( 'U' ) )
	{
		m_raycast_rightArmDown.m_rayStartPos += Vec3( 0.0f, 0.0f, -moveSpeed ) * deltaSeconds;
		m_raycast_rightArmDown.m_rayEndPos   += Vec3( 0.0f, 0.0f, -moveSpeed ) * deltaSeconds;
	}

	//----------------------------------------------------------------------------------------------------------------------
	// East
	if ( g_theInput->IsKeyDown( KEYCODE_UPARROW ) )
	{
		m_raycast_rightArmDown.m_rayEndPos += Vec3( moveSpeed, 0.0f, 0.0f ) * deltaSeconds; 
	}
	// West
	if ( g_theInput->IsKeyDown( KEYCODE_DOWNARROW ) )
	{
		m_raycast_rightArmDown.m_rayEndPos += Vec3( -moveSpeed, 0.0f, 0.0f ) * deltaSeconds; 
	}
	// North
	if ( g_theInput->IsKeyDown( KEYCODE_LEFTARROW ) )
	{
		m_raycast_rightArmDown.m_rayEndPos += Vec3( 0.0f, moveSpeed, 0.0f ) * deltaSeconds; 
	}
	// South
	if ( g_theInput->IsKeyDown( KEYCODE_RIGHTARROW ) )
	{
		m_raycast_rightArmDown.m_rayEndPos += Vec3( 0.0f, -moveSpeed, 0.0f ) * deltaSeconds; 
	}
	// Skyward
	if ( g_theInput->IsKeyDown( 'M' ) )
	{
		m_raycast_rightArmDown.m_rayEndPos += Vec3( 0.0f, 0.0f, moveSpeed ) * deltaSeconds;
	}
	// Groundward
	if ( g_theInput->IsKeyDown( 'N' ) )
	{
		m_raycast_rightArmDown.m_rayEndPos += Vec3( 0.0f, 0.0f, -moveSpeed ) * deltaSeconds;
	}
}


//----------------------------------------------------------------------------------------------------------------------
bool GameMode3D::DidRaycastHitTriangle( RaycastResult3D& raycastResult, Vec3& rayStartPos, Vec3& rayfwdNormal, float rayLength, Vec3& updatedImpactPos, Vec3& updatedImpactNormal )
{
	bool  didImpact		= false;
	float t, u, v		= 0.0f;
	RaycastResult3D tempRayResult;
	// Increment in 3s to form triangles
	for ( int i = 0; i < m_map->m_indexList.size(); i += 3 )
	{
		// Calculate triangles from indexList
		int const& currentIndex_C = m_map->m_indexList[ i + 2];
		int const& currentIndex_A = m_map->m_indexList[ i + 0];
		int const& currentIndex_B = m_map->m_indexList[ i + 1];
		Vec3 vert0		= m_map->m_planeVerts[ currentIndex_A ].m_position;
		Vec3 vert1		= m_map->m_planeVerts[ currentIndex_B ].m_position;
		Vec3 vert2		= m_map->m_planeVerts[ currentIndex_C ].m_position;
		tempRayResult	= RaycastVsTriangle( rayStartPos, rayfwdNormal, rayLength, vert0, vert1, vert2, t, u, v );
		if ( tempRayResult.m_didImpact )
		{
			// If ray hit AND is closer
			if ( tempRayResult.m_impactDist < raycastResult.m_impactDist )
			{
				raycastResult		= tempRayResult;
				updatedImpactPos	= raycastResult.m_impactPos;
				updatedImpactNormal = raycastResult.m_impactNormal;
				didImpact			= true;
			}
		}
	}
	return didImpact;
}


//----------------------------------------------------------------------------------------------------------------------
bool GameMode3D::DidRaycastHitWalkableBlock( RaycastResult3D& raycastResult, Vec3& rayStartPos, Vec3& rayfwdNormal, float rayLength, Vec3& updatedImpactPos, Vec3& updatedImpactNormal )
{
	float superDist_FWD = 500.0f;
	bool  didImpact		= false;
	RaycastResult3D tempRayResult;
	for ( int i = 0; i < m_blockList.size(); i++ )
	{
		// Check if raycast impacted a block
		tempRayResult = RaycastVsAABB3D( rayStartPos, rayfwdNormal, rayLength, m_blockList[i]->m_aabb3 );
		if ( tempRayResult.m_didImpact )
		{
			// And the block is walkable
			if ( m_blockList[i]->m_isWalkable )
			{
				// And the block is close enough
				Vec3 distFromCurrentLineToRay = tempRayResult.m_impactPos - rayStartPos;
				if ( distFromCurrentLineToRay.GetLength() < superDist_FWD )
				{
					// Save the raycastHitInfo 

					// Check for closest line segment to raycast 
					superDist_FWD			= distFromCurrentLineToRay.GetLength();
					didImpact				= true;
					raycastResult			= tempRayResult;
					// Use the updated values below ( impactPos and impactNormal ) for rendering raycast 
					updatedImpactPos		= raycastResult.m_impactPos;
					updatedImpactNormal		= raycastResult.m_impactNormal;
					
				}
			}
		}
	}
	return didImpact;
}

//----------------------------------------------------------------------------------------------------------------------
bool GameMode3D::DidRaycastHitClimbableBlock( RaycastResult3D& raycastResult, Vec3& rayStartPos, Vec3& rayfwdNormal, float rayLength, Vec3& updatedImpactPos, Vec3& updatedImpactNormal )
{
	float superDist_FWD = 500.0f;
	bool  didImpact		= false;
	for ( int i = 0; i < m_blockList.size(); i++ )
	{
		// Check if raycast impacted a block
		raycastResult = RaycastVsAABB3D( rayStartPos, rayfwdNormal, rayLength, m_blockList[i]->m_aabb3 );
		if ( raycastResult.m_didImpact )
		{
			// And the block is walkable
			if ( m_blockList[i]->m_isClimbable )
			{
				// And the block is close enough
				Vec3 distFromCurrentLineToRay = raycastResult.m_impactPos - rayStartPos;
				if ( distFromCurrentLineToRay.GetLength() < superDist_FWD )
				{
					// Save the raycastHitInfo 

					// Check for closest line segment to raycast 
					superDist_FWD		= distFromCurrentLineToRay.GetLength();
					didImpact			= true;
					// Use the updated values below ( impactPos and impactNormal ) for rendering raycast 
					updatedImpactPos	= raycastResult.m_impactPos;
					updatedImpactNormal = raycastResult.m_impactNormal;
				}
			}
		}
	}
	return didImpact;
}