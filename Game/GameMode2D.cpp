#include "Game/GameMode2D.hpp"
#include "Game/GameCommon.hpp"
#include "Game/App.hpp"

#include "Engine/SkeletalSystem/IK_Chain2D.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"

//----------------------------------------------------------------------------------------------------------------------
GameMode2D::GameMode2D()
{
	m_debugGoalPosition = Vec2( WORLD_SIZE_X - 20.0f, WORLD_CENTER_Y );
}

//----------------------------------------------------------------------------------------------------------------------
GameMode2D::~GameMode2D()
{
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode2D::Startup()
{
	//----------------------------------------------------------------------------------------------------------------------
	// Create new Creature
	//----------------------------------------------------------------------------------------------------------------------
	m_root = new IK_Joint2D( Vec2( WORLD_CENTER_X, WORLD_CENTER_Y ), 1.0f, 1.0f );

	//----------------------------------------------------------------------------------------------------------------------
	// Create new skeletal systems
	//----------------------------------------------------------------------------------------------------------------------
	m_creatureRightArm  = new IK_Chain2D( m_root->m_jointPosition_LS );
	m_creatureLeftArm   = new IK_Chain2D( m_root->m_jointPosition_LS );
	m_creatureRightFoot = new IK_Chain2D( m_root->m_jointPosition_LS );
	m_creatureLeftFoot  = new IK_Chain2D( m_root->m_jointPosition_LS );

	//----------------------------------------------------------------------------------------------------------------------
	// Create all creature body parts (root, joints, limbs)
	//----------------------------------------------------------------------------------------------------------------------
	// Body
	for ( int i = 0; i < 2; i++ )
	{
		 m_creatureRightArm->CreateNewLimb( m_armLength, 0.0f );
		  m_creatureLeftArm->CreateNewLimb( m_armLength, 0.0f );
		 m_creatureLeftFoot->CreateNewLimb( m_armLength, 0.0f );
		m_creatureRightFoot->CreateNewLimb( m_armLength, 0.0f );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Environmental Grass
	//----------------------------------------------------------------------------------------------------------------------
	// Grass1
	m_grass1 = new IK_Chain2D( Vec2( 50.0f, 0.0f ) );
	for ( int i = 0; i < 10; i++ )
	{
		m_grass1->CreateNewLimb( m_grassLength, 90.0f );
	}
	// Grass2
	m_grass2 = new IK_Chain2D( Vec2( 100.0f, 0.0f ) );
	for ( int i = 0; i < 10; i++ )
	{
		m_grass2->CreateNewLimb( m_grassLength, 90.0f );
	}
	// Grass3
	m_grass3 = new IK_Chain2D( Vec2( 150.0f, 0.0f ) );
	for ( int i = 0; i < 10; i++ )
	{
		m_grass3->CreateNewLimb( m_grassLength, 90.0f );
	}
	// Grass4
	m_grass4 = new IK_Chain2D( Vec2( WORLD_SIZE_X, 25.0f ) );
	for ( int i = 0; i < 10; i++ )
	{
		m_grass4->CreateNewLimb( m_grassLength, 90.0f );
	}
	// Grass3
	m_grass5 = new IK_Chain2D( Vec2( WORLD_SIZE_X, 75.0f ) );
	for ( int i = 0; i < 10; i++ )
	{
		m_grass5->CreateNewLimb( m_grassLength, 90.0f );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Get a pointer to the 1st item in the list
	//----------------------------------------------------------------------------------------------------------------------
	// This pointer will be used for updating only ONE item instead of all existing root/joints/limbs simultaneously
	if ( m_creatureRightArm->m_limbList.size() >= 0 )
	{
		m_currentLimb = m_creatureRightArm->m_limbList[0];
	}
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode2D::Update( float deltaSeconds )
{
	UNUSED( deltaSeconds );

	UpdatePlayerMovementInput();
	UpdateGoalPosInput();
	UpdateDebugToggle();

	// Update only the selected limb instead of all existing limbs
//	UpdateLimbToFollowGoalPos( m_currentLimb );
//	UpdateDragLimbToGoalPos( m_creatureBody->m_finalJoint );

	// Set creatureArm1's startPos to always be placed at the root's position to keep the "arm attached to the body"
	 m_creatureRightArm->m_position = m_root->m_jointPosition_LS;
	  m_creatureLeftArm->m_position = m_root->m_jointPosition_LS;
	 m_creatureLeftFoot->m_position = m_root->m_jointPosition_LS;
	m_creatureRightFoot->m_position = m_root->m_jointPosition_LS;

	//----------------------------------------------------------------------------------------------------------------------
	// UpdateDragLimbToGoalPos
	//----------------------------------------------------------------------------------------------------------------------
	if ( m_creatureRightArm->m_finalJoint != nullptr )
	{
//		m_creatureArm1->DragLimb( m_debugGoalPosition );
//		m_creatureArm1->ReachLimb( m_debugGoalPosition );
		CalculateBestFootPos( m_creatureRightArm, m_bestRightArmPos, -45.0f );
		m_creatureRightArm->ReachLimb( m_bestRightArmPos );
	}
	if ( m_creatureLeftArm->m_finalJoint != nullptr )
	{
		CalculateBestFootPos( m_creatureLeftArm, m_bestLeftArmPos, 45.0f );
		m_creatureLeftArm->ReachLimb( m_bestLeftArmPos );
	}
	if ( m_creatureLeftFoot->m_finalJoint != nullptr )
	{
		CalculateBestFootPos( m_creatureLeftFoot, m_bestLeftFootPos, 135.0f );
		m_creatureLeftFoot->ReachLimb( m_bestLeftFootPos );
	}
	if ( m_creatureRightFoot->m_finalJoint != nullptr )
	{
		CalculateBestFootPos( m_creatureRightFoot, m_bestRightFootPos, -135.0f );
		m_creatureRightFoot->ReachLimb( m_bestRightFootPos );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// UpdateReachLimbToGoalPos
	//----------------------------------------------------------------------------------------------------------------------
	if ( m_grass1->m_finalJoint != nullptr )
	{
		m_grass1->ReachLimb( m_root->m_jointPosition_LS ); 
	}
	if ( m_grass2->m_finalJoint != nullptr )
	{
		m_grass2->ReachLimb( m_root->m_jointPosition_LS );
	}
	if ( m_grass3->m_finalJoint != nullptr )
	{
		m_grass3->ReachLimb( m_root->m_jointPosition_LS );
	}
	if ( m_grass4->m_finalJoint != nullptr )
	{
		m_grass4->ReachLimb( m_root->m_jointPosition_LS );
	}
	if ( m_grass5->m_finalJoint != nullptr )
	{
		m_grass5->ReachLimb( m_root->m_jointPosition_LS );
	}

//	UpdatePlacingWalkGoalPositions( m_currentLimb );

	 m_creatureRightArm->Update();
	  m_creatureLeftArm->Update();
	 m_creatureLeftFoot->Update();
	m_creatureRightFoot->Update();
	m_grass1->Update();
	m_grass2->Update();
	m_grass3->Update();
	m_grass4->Update();
	m_grass5->Update();

	UpdatePauseQuitAndSlowMo();
	UpdateGameMode2DCamera();
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode2D::Render() const
{
	RenderWorldObjects();
	RenderUIObjects();
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode2D::Shutdown()
{
	m_creatureRightArm->Shutdown();
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode2D::UpdateGameMode2DCamera()
{
	m_gameModeWorldCamera.SetOrthoView( Vec2( 0.0f, 0.0f ), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y ) );
	   m_gameModeUICamera.SetOrthoView( Vec2( 0.0f, 0.0f ), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y ) );
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode2D::RenderWorldObjects() const
{
	//----------------------------------------------------------------------------------------------------------------------
	// Begin World Camera
	//----------------------------------------------------------------------------------------------------------------------
	g_theRenderer->BeginCamera( m_gameModeWorldCamera );

	// Initialize common variables
	std::vector<Vertex_PCU> textVerts;
	std::vector<Vertex_PCU> verts;
	
	//----------------------------------------------------------------------------------------------------------------------
	// "GameMode2D" title
	//----------------------------------------------------------------------------------------------------------------------
//	AABB2 textBox = AABB2( Vec2( 0.0f, 0.0f ), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y ) );
//	textBox.SetCenter( Vec2( WORLD_CENTER_X, WORLD_SIZE_Y * 0.75f ) );
//	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textBox, 8.0f, "Skeletal PlayGround 2D!", Rgba8::RED );

	//----------------------------------------------------------------------------------------------------------------------
	// Render Creature Body
	//----------------------------------------------------------------------------------------------------------------------
	AddVertsForDisc2D( verts, m_root->m_jointPosition_LS, 3.0f, Rgba8::DARK_GREEN );

	//----------------------------------------------------------------------------------------------------------------------
	// Creature debug
	//----------------------------------------------------------------------------------------------------------------------
	// Root orientation
	Vec2 rootFwd = m_root->m_jointPosition_LS + Vec2::MakeFromPolarDegrees( m_root->m_orientation, 8.0f );
	AddVertsForArrow2D( verts, m_root->m_jointPosition_LS, rootFwd, 4.0f, 0.5f, Rgba8::DARK_YELLOW );

	//----------------------------------------------------------------------------------------------------------------------
	// Skeleton
	//----------------------------------------------------------------------------------------------------------------------
	RenderSkeleton( verts );

	//----------------------------------------------------------------------------------------------------------------------
	// Render debug goal position
	//----------------------------------------------------------------------------------------------------------------------
	if ( g_debugFollowTarget_F11 )
	{
//		AddVertsForDisc2D( verts, m_debugGoalPosition, 1.0f, Rgba8::MAGENTA );
		AddVertsForDisc2D( verts, m_bestRightArmPos,  1.0f, Rgba8::MAGENTA );
		AddVertsForDisc2D( verts, m_bestLeftArmPos,   1.0f, Rgba8::DARK_BLUE );
//		AddVertsForDisc2D( verts, m_bestLeftFootPos,  1.0f, Rgba8::DARK_YELLOW );
//		AddVertsForDisc2D( verts, m_bestRightFootPos, 1.0f, Rgba8::DARK_GREEN );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Render walk goal position
	//----------------------------------------------------------------------------------------------------------------------
	AddVertsForDisc2D( verts, m_walkGoalPos, 2.0f, Rgba8::GREEN );

	//----------------------------------------------------------------------------------------------------------------------
	// Render debug positions & orientations for root, joints, and limbs
	//----------------------------------------------------------------------------------------------------------------------
	RenderDebugObjects( textVerts );
	// Debug ring for selected limb
	if ( m_currentLimb != nullptr )
	{
//		AddVertsForRing2D( verts, m_currentLimb->m_jointPosition_LS, 5.0f, 2.0f, Rgba8::SKYBLUE );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// End world camera
	g_theRenderer->EndCamera( m_gameModeWorldCamera );

	//----------------------------------------------------------------------------------------------------------------------
	// Draw calls
	//----------------------------------------------------------------------------------------------------------------------
	// Draw for world objects
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->DrawVertexArray( static_cast<int>( verts.size() ), verts.data() );

	// Draw for text objects
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( &g_theApp->m_textFont->GetTexture() );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->DrawVertexArray( static_cast<int>( textVerts.size() ), textVerts.data() );
	g_theRenderer->BindTexture( nullptr );
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode2D::RenderUIObjects() const
{
	//----------------------------------------------------------------------------------------------------------------------
	// Begin UI Camera
	g_theRenderer->BeginCamera( m_gameModeUICamera );

	//----------------------------------------------------------------------------------------------------------------------
	// Get or Create font
	std::vector<Vertex_PCU> textVerts;
	float cellHeight	= 2.0f;
	AABB2 textbox1		= AABB2( Vec2( 0.0f, 0.0f ), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y - 1.0f ) );
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight, "Mode (F6/F7 for prev/next): SkeletalPlayground (2D)", Rgba8::YELLOW, 0.75f, Vec2( 0.0f, 1.0f ), TextDrawMode::SHRINK_TO_FIT );

	// Hotkeys
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight, "Hotkey toggles (F1: pos/angle info | F3: debugTargetPos + IJKL | F5: debugGrass)", Rgba8::YELLOW, 0.75f, Vec2( 0.0f, 0.95f ), TextDrawMode::SHRINK_TO_FIT );

	// Initialize and set UI variables
	float fps				= 1.0f / g_theApp->m_gameClock.GetDeltaSeconds();
	float scale				= g_theApp->m_gameClock.GetTimeScale();
	std::string timeText	= Stringf( "Time: %0.2f. FPS: %0.2f, Scale %0.2f.", g_theApp->m_gameClock.GetTotalSeconds(), fps, scale );
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight, timeText, Rgba8::YELLOW, 0.75f, Vec2( 0.98f, 1.0f ), TextDrawMode::SHRINK_TO_FIT );

	// Render CursorPos
//	Vec2 cursorPos				= g_theInput->GetCursorClientPosition();
//	std::string cursorPosText	= Stringf( "CursorPosX: %0.2f, CursorPosY: %0.2f", cursorPos.x, cursorPos.y );
//	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight, cursorPosText, Rgba8::YELLOW, 0.75f, Vec2( 0.0f, 0.95f ), TextDrawMode::SHRINK_TO_FIT );

	// Render TargetPos
	Vec2 targetPos				= m_debugGoalPosition;
	std::string targetPosText	= Stringf( "TargetPosX: %0.2f, TargetPosY: %0.2f", m_debugGoalPosition.x, m_debugGoalPosition.y );
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight, targetPosText, Rgba8::YELLOW, 0.75f, Vec2( 0.0f, 0.90f ), TextDrawMode::SHRINK_TO_FIT );

	//----------------------------------------------------------------------------------------------------------------------
	// End UI Camera
	g_theRenderer->EndCamera( m_gameModeUICamera );

	//----------------------------------------------------------------------------------------------------------------------
	// Draw for UI camera
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( &g_theApp->m_textFont->GetTexture() );
	g_theRenderer->DrawVertexArray( static_cast<int>( textVerts.size() ), textVerts.data() );
	g_theRenderer->BindTexture( nullptr );
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode2D::RenderSkeleton( std::vector<Vertex_PCU>& verts ) const
{
	 m_creatureRightArm->Render( verts );
	  m_creatureLeftArm->Render( verts );
//	 m_creatureLeftFoot->Render( verts );
//	m_creatureRightFoot->Render( verts );

	if ( g_debugToggleGrass )
	{
		m_grass1->Render( verts );
		m_grass2->Render( verts );
		m_grass3->Render( verts );
		m_grass4->Render( verts );
		m_grass5->Render( verts );
	}
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode2D::UpdateDebugToggle()
{
	if ( g_theInput->WasKeyJustPressed( KEYCODE_F1 ) )
	{
		g_debugPrintJointLimbPositions = !g_debugPrintJointLimbPositions;
	}
	
	if ( g_theInput->WasKeyJustPressed( KEYCODE_F2 ) )
	{
		g_debugToggleConstraints_F8 = !g_debugToggleConstraints_F8;
	}

	if ( g_theInput->WasKeyJustPressed( KEYCODE_F3 ) )
	{
		g_debugFollowTarget_F11 = !g_debugFollowTarget_F11;
	}
	if ( g_theInput->WasKeyJustPressed( KEYCODE_F5 ) )
	{
		g_debugToggleGrass = !g_debugToggleGrass;
	}

//	if ( g_debugHighlightSelectedLimb )
//	{
		if ( g_theInput->WasKeyJustPressed( '1') )
		{
			// Loop through limbList and find the currentLimb
//			std::map< std::string, Limb* >::const_iterator limbIter;
//			for ( limbIter = m_skeletalSystem2D->m_limbList.begin(); limbIter != m_skeletalSystem2D->m_limbList.end(); limbIter++ )
			for ( int limbIndex = 0; limbIndex < m_creatureRightArm->m_limbList.size(); limbIndex++ )
			{
				// if the limbs match, increment the currentLimb and set it to the m_currentLimb
//				Limb* currentLimb = limbIter->second;
				IK_Joint2D* currentLimb = m_creatureRightArm->m_limbList[limbIndex];
				if ( currentLimb == m_currentLimb )
				{
					// Increment limbIter
//					limbIter++;
 					limbIndex++;
					// If limbIter is at the end of the list, restart at the beginning
//					if ( limbIter == m_skeletalSystem2D->m_limbList.end() )
//					{
//						limbIter = m_skeletalSystem2D->m_limbList.begin();
//					}
					if ( limbIndex == m_creatureRightArm->m_limbList.size() )
					{
						currentLimb = m_creatureRightArm->m_limbList[0];
					}
//					m_currentLimb = limbIter->second;
					break;
				}
			}
		}
//	}
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode2D::UpdatePlayerMovementInput()
{
	Vec2 moveIntention = Vec2::ZERO;
	bool shouldTurn	   = false;

	// North
	if ( g_theInput->IsKeyDown( 'W' ) )
	{
//		m_root->m_jointPosition_LS += Vec2( 0.0f, 1.0f );
		moveIntention += Vec2( 0.0f, 1.0f );
		shouldTurn     = true;
	}
	// West
	if ( g_theInput->IsKeyDown( 'A' ) )
	{
		moveIntention += Vec2( -1.0f, 0.0f );
		shouldTurn     = true;
	}
	// South
	if ( g_theInput->IsKeyDown( 'S' ) )
	{
		moveIntention += Vec2( 0.0f, -1.0f );
		shouldTurn     = true;
	}
	// East
	if ( g_theInput->IsKeyDown( 'D' ) )
	{
		moveIntention += Vec2( 1.0f, 0.0f );
		shouldTurn     = true;
	}
	moveIntention.ClampLength( 0.5f );
	m_root->m_jointPosition_LS += moveIntention; 

	// Set orientation
	if ( shouldTurn )
	{
		float moveOrientationIntention	= moveIntention.GetOrientationDegrees();	
		float goalOrientation			= GetTurnedTowardDegrees( m_root->m_orientation, moveOrientationIntention, 5.0f );
		m_root->m_orientation			= goalOrientation;
	}
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode2D::UpdateGoalPosInput()
{
	// North
	if ( g_theInput->IsKeyDown( 'I' ) )
	{
		m_debugGoalPosition += Vec2( 0.0f, 1.0f );
	}
	// West
	if ( g_theInput->IsKeyDown( 'J' ) )
	{
		m_debugGoalPosition += Vec2( -1.0f, 0.0f );
	}
	// South
	if ( g_theInput->IsKeyDown( 'K' ) )
	{
		m_debugGoalPosition += Vec2( 0.0f, -1.0f );
	}
	// East
	if ( g_theInput->IsKeyDown( 'L' ) )
	{
		m_debugGoalPosition += Vec2( 1.0f, 0.0f );
	}
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode2D::UpdateDragLimbToGoalPos( IK_Joint2D* limb )
{
	if ( limb != nullptr )
	{
		limb->DragLimb( m_debugGoalPosition );
	}
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode2D::UpdatePlacingWalkGoalPositions( IK_Joint2D* limb )
{
	// Place a goal position directly below the "creature"
	// If the "creature" moves too far right, place a new goal position "some distance" in front of the it.
	// the updated goal position should make the "creature" move its limbs towards the new goal pos
	
	// Place/update goal position
	if ( m_needsNewGoalPos )
	{
		Vec2 rootPos  = m_creatureRightArm->m_position;
		m_walkGoalPos = Vec2( rootPos.x + 15.0f, rootPos.y - 20.0f );
	}

	// Toggle bool for updatingGoalPos if root too far from goalPos
	if ( limb != nullptr )
	{
 		float distGoalToRoot = GetDistance2D( m_walkGoalPos, limb->m_endPos );
		if ( distGoalToRoot > 15.0f )
		{
			m_needsNewGoalPos = true;
		}
		else
		{
			m_needsNewGoalPos = false;
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode2D::CalculateBestFootPos( IK_Chain2D* skeletalSystem2D, Vec2& bestFootPos, float targetAngle )
{
	float totalLimbLength = skeletalSystem2D->m_limbList.size() * m_armLength;

	float distLimbStartToBestFootPos = GetDistance2D( bestFootPos, m_root->m_jointPosition_LS );
	if ( distLimbStartToBestFootPos > totalLimbLength )
	{
		float targetFootAngle	= m_root->m_orientation + targetAngle;
		bestFootPos				= Vec2::MakeFromPolarDegrees( targetFootAngle, totalLimbLength );
		bestFootPos				+= m_root->m_jointPosition_LS;
	}
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode2D::RenderDebugObjects( std::vector<Vertex_PCU>& textVerts ) const
{
	float cellHeight = 2.0f;
	if ( g_debugPrintJointLimbPositions )
	{
		// Goal position
		std::string const goalPosText = Stringf( "X: %0.2f, Y: %0.2f", m_debugGoalPosition.x, m_debugGoalPosition.y );
		g_theApp->m_textFont->AddVertsForText2D( textVerts, m_debugGoalPosition, cellHeight, goalPosText.c_str() );

		// Render limb start & limb end positions and orientations
		std::map< std::string, IK_Joint2D* >::const_iterator limbIter;
		int limbCount = 0;		// count is used for textPos offsets
//		for ( limbIter = m_skeletalSystem2D->m_limbList.begin(); limbIter != m_skeletalSystem2D->m_limbList.end(); limbIter++ )
		for ( int limbIndex = 0; limbIndex < m_creatureRightArm->m_limbList.size(); limbIndex++ )
		{
//			Limb* currentLimb = limbIter->second;
			IK_Joint2D* currentLimb = m_creatureRightArm->m_limbList[limbIndex];
			// Start pos
			std::string const limbStartPosText = Stringf( "X: %0.2f, Y: %0.2f", currentLimb->m_jointPosition_LS.x, currentLimb->m_jointPosition_LS.y );
			g_theApp->m_textFont->AddVertsForText2D( textVerts, Vec2( currentLimb->m_jointPosition_LS.x, currentLimb->m_jointPosition_LS.y + ( limbCount * 2.0f ) ), cellHeight, limbStartPosText.c_str(), Rgba8::RED );
			
			// Start orientation
			std::string const limbOrientationText = Stringf( "Angle: %0.2f", currentLimb->m_orientation );
			g_theApp->m_textFont->AddVertsForText2D( textVerts, Vec2( currentLimb->m_jointPosition_LS.x, currentLimb->m_jointPosition_LS.y + ( limbCount * 3.0f + 4.0f ) ), cellHeight, limbOrientationText.c_str(), Rgba8::BLUE );

			// End pos
//			std::string const limbEndPosText = Stringf( "X: %0.2f, Y: %0.2f", currentLimb->m_endPos.x, currentLimb->m_endPos.y );
//			g_theApp->m_textFont->AddVertsForText2D( textVerts, Vec2( currentLimb->m_endPos.x, currentLimb->m_endPos.y + ( limbCount * 2.0f ) ), cellHeight, limbEndPosText.c_str(), Rgba8::GREEN );
	
			// increment count
			limbCount++;
		}
	}
}
