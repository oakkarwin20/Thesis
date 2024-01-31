#pragma once

#include "Game/GameModeBase.hpp"
#include "Game/GameCommon.hpp"
#include "Game/App.hpp"

#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/CubicBezierCurve3D.hpp"
#include "Engine/Core/Stopwatch.hpp"


//----------------------------------------------------------------------------------------------------------------------
class Texture;
class IK_Chain3D;
class IK_Joint3D;
class CreatureBase;
class Quadruped;
class Map_GameMode3D;


//----------------------------------------------------------------------------------------------------------------------
struct Block
{
	Block( AABB3 aabb3, bool isWalkable, bool isClimbable = false )
		: m_aabb3( aabb3 )
		, m_isWalkable( isWalkable )
		, m_isClimbable( isClimbable )
	{
	}
	~Block()
	{
	}

	AABB3	m_aabb3			= AABB3( Vec3::ZERO, Vec3::ZERO );
	bool	m_isWalkable	= true;
	bool	m_isClimbable	= false;
};


//----------------------------------------------------------------------------------------------------------------------
struct Raycast_old
{
	RaycastResult3D		m_raycastResult			= RaycastResult3D();
	Vec3				m_updatedImpactPos		= Vec3::ZERO;
	Vec3				m_updatedImpactNormal	= Vec3::ZERO;
	Vec3				m_rayStartPos			= Vec3::ZERO;
	Vec3				m_rayEndPos				= Vec3::ZERO;
	bool				m_didRayImpact			= false;
};


//----------------------------------------------------------------------------------------------------------------------
class GameMode3D : public GameModeBase
{
public:
	GameMode3D();
	virtual ~GameMode3D();
	virtual void Startup();
	virtual void Update( float deltaSeconds );
	virtual void Render() const;
	virtual void Shutdown();

	// Camera and Render Functions
	void UpdateDebugKeys();
	void UpdateCameraInput( float deltaSeconds );
	void UpdateGameMode3DCamera();
	void RenderWorldObjects()	const;
	void RenderUIObjects()		const;

	// Debug Functions
	void AddVertsForCompass( std::vector<Vertex_PCU>&compassVerts, Vec3 startPosition, float axisLength, float axisThickness ) const;
	void UpdateDebugLimbGoalPositions();
	void UpdateDebugTreePosInput();
	void UpdateTrees();

	// Creature Functions
	void InitializeCreature();
	void ToggleAnchorStates();
	void RenderCreature( std::vector<Vertex_PCU>& verts_Creature, std::vector<Vertex_PCU>& verts_NoTexture,std::vector<Vertex_PCU>& verts_BackfaceCull, std::vector<Vertex_PCU>& verts_Text ) const;
	void UpdateCreature( float deltaSeconds );
	void UpdateCreatureRootPosInput_Walking( float deltaSeconds );
	void UpdateCreatureRootPosInput_Climbing( float deltaSeconds );
	void UpdateCreatureHeight( float deltaSeconds );
	void DetermineBestSprintStepPos();
	void DetermineBestWalkStepPos();
	void DetermineBestClimbPos();
	bool IsLimbIsTooFarFromRoot( IK_Chain3D* currentLimb, Vec3 footTargetPos );
	bool IsLimbIsTooFarFromHip(  IK_Chain3D* currentLimb, Vec3 footTargetPos );
	bool DoesTargetPosOverlapWalkableObject( Vec3& footTargetPos );
	void SpecifyTargetPosForClimbing( Vec3& targetPos, float fwdStepAmount, float leftStepAmount );
	void SpecifyFootPlacementPos( Vec3& targetPos, float fwdStepAmount, float leftStepAmount );
	void SpecifyFootPlacementPos( Vec3& targetPos, IK_Joint3D* refLimb, float fwdStepAmount, float leftStepAmount );
	//----------------------------------------------------------------------------------------------------------------------
	// #GenericRefactoring 
	//----------------------------------------------------------------------------------------------------------------------
	void SpecifyFootPlacementPos( Vec3& targetPos, IK_Joint3D* refLimb, float maxLength, float fwdStepAmount, float leftStepAmount );			
	void InitStepBezier( CubicBezierCurve3D& bezierCurve, IK_Chain3D* const ik_Chain, Stopwatch& bezierTimer );							
	void UpdateBezier  ( CubicBezierCurve3D& bezierCurve, IK_Chain3D* const		ik_Chain, Stopwatch& bezierTimer );								
	//----------------------------------------------------------------------------------------------------------------------
	void TurnCreatureTowardsCameraDir();

	// Mount Mechanic
	void ExecuteMount();

	// Environment
	void InitializeEnvironment();
	void RenderEnvironment( std::vector<Vertex_PCU>& verts ) const;

	// Tree Functions
	void InitializeTrees();
	void RenderTrees( std::vector<Vertex_PCU>& verts ) const;
	void UpdateTreesToReachOutToPlayer( float deltaSeconds );

	// Raycast Functions
	void RenderRaycasts( std::vector<Vertex_PCU>& verts ) const;
	void UpdateRaycastResult3D();
	void MoveRaycastInput( float deltaSeconds );
	bool DidRaycastHitTriangle( RaycastResult3D& raycastResult, Vec3& rayStartPos, Vec3& rayfwdNormal, float rayLength, Vec3& updatedImpactPos, Vec3& updatedImpactNormal );
	bool DidRaycastHitWalkableBlock(  RaycastResult3D& m_raycastResult, Vec3& rayStartPos, Vec3& rayfwdNormal, float rayLength, Vec3& updatedImpactPos, Vec3& updatedImpactNormal );
	bool DidRaycastHitClimbableBlock( RaycastResult3D& m_raycastResult, Vec3& rayStartPos, Vec3& rayfwdNormal, float rayLength, Vec3& updatedImpactPos, Vec3& updatedImpactNormal );

public:
	// Quadruped
	Quadruped* m_quadruped = nullptr;

	// Debug rayVsTri
	RaycastResult3D m_rayVsTri;

	//----------------------------------------------------------------------------------------------------------------------
	// Core Variables
	//----------------------------------------------------------------------------------------------------------------------
	Map_GameMode3D* m_map					= nullptr;
	float			m_sine					= 0.0f;
	float			m_currentTime			= 0.0f;
	float			m_walkLerpSpeed			= 6.0f;
	float			m_sprintLerpSpeed		= m_walkLerpSpeed * 4.0f;
	float			m_goalWalkLerpSpeed		= m_walkLerpSpeed;
	float			m_currentWalkLerpSpeed	= m_goalWalkLerpSpeed;

	//----------------------------------------------------------------------------------------------------------------------
	// Camera Variables
	//----------------------------------------------------------------------------------------------------------------------
	Camera	m_gameMode3DWorldCamera;
	Camera	m_gameMode3DUICamera;
	// Camera movement Variables
//	float	m_defaultSpeed	= 20.0f;
//	float	m_currentSpeed	= 20.0f;
	float	m_defaultSpeed  = 10.0f;
	float	m_currentSpeed  = 10.0f;
	float	m_fasterSpeed	= m_defaultSpeed * 2.0f;
	float	m_elevateSpeed  = 6.0f;
	float	m_turnRate		= 90.0f;

	//----------------------------------------------------------------------------------------------------------------------
	// IkChain_3D variables
	//----------------------------------------------------------------------------------------------------------------------
	std::vector<IK_Chain3D*>	m_creatureSkeletalSystemsList;
	IK_Joint3D*			m_root					= nullptr;
	IK_Chain3D*				m_hip					= nullptr;
	IK_Chain3D*				m_tail					= nullptr;
	IK_Chain3D*				m_head					= nullptr;
	IK_Chain3D*				m_neck					= nullptr;
	IK_Chain3D*				m_rightArm				= nullptr;
	IK_Chain3D*				m_leftArm				= nullptr;
	IK_Chain3D*				m_leftFoot				= nullptr;
	IK_Chain3D*				m_rightFoot				= nullptr;
	float					m_limbLength			= 10.0f;
//	FloatRange				m_limbLength			= FloatRange( 25.0f, 25.0f );

	// Limb length/count/segment Variables
	float					m_numArms				=  4.0f;
	float					m_numFeet				=  4.0f;
	float					m_numHips				=  3.0f;
	float					m_numTailSegments		=  5.0f;
	float					m_numNeckSegments		=  1.0f;
	float					m_rootDefaultHeightZ	= 20.0f;
	float					m_maxArmLength			= m_limbLength * m_numArms;
	float					m_maxFeetLength			= m_limbLength * m_numFeet;

	//----------------------------------------------------------------------------------------------------------------------
	// Creature Limb placement variables
	//----------------------------------------------------------------------------------------------------------------------
	bool m_headHasLookAtTarget		= false;
	bool m_isSprinting				= false;

	// Debug positions
	Vec3 m_debugGoalPos_RA			= Vec3::ZERO;	// "RA" is "Right arm"
	Vec3 m_debugGoalPos_LA			= Vec3::ZERO;	// "LA" is "Left arm"
	Vec3 m_debugGoalPos_RL			= Vec3::ZERO;	// "RL" is "Right leg"
	Vec3 m_debugGoalPos_LL			= Vec3::ZERO;	// "LL" is "Left leg"

	Vec3 m_hipFwdDir		= Vec3::ZERO;;
	Vec3 m_hipLeftDir		= Vec3::ZERO;;
	Vec3 m_loweredHipPos	= Vec3::ZERO;;

	//----------------------------------------------------------------------------------------------------------------------
	// Anchor variables
	//----------------------------------------------------------------------------------------------------------------------
	bool m_rightArmAnchored	= false;
	bool m_leftArmAnchored	= true;

	//----------------------------------------------------------------------------------------------------------------------
	// Creature Movement mechanic
	//----------------------------------------------------------------------------------------------------------------------
	Vec3 m_moveFwdDir = Vec3::ZERO;
	bool m_isClimbing = false;

	//----------------------------------------------------------------------------------------------------------------------
	// Creature Mount mechanic
	//----------------------------------------------------------------------------------------------------------------------
	bool m_shouldMount	= false;
	Vec3 m_mountGoalPos = Vec3::ZERO;

	//----------------------------------------------------------------------------------------------------------------------
	// Tree Variables
	//----------------------------------------------------------------------------------------------------------------------
	CreatureBase*	  m_treeCreature		= nullptr;
	IK_Chain3D*		  m_treeBranch1			= nullptr;
	IK_Chain3D*		  m_treeBranch2			= nullptr;
	IK_Chain3D*		  m_treeBranch3			= nullptr;
	Vec3			  m_treeDebugTargetPos	= Vec3::ZERO;
	float			  m_numTreeSegments		= 2.0f;

	//----------------------------------------------------------------------------------------------------------------------
	// Block Objects
	//----------------------------------------------------------------------------------------------------------------------
	std::vector<Block*> m_blockList; 
	// Floors
	Block* m_floor_NE	= new Block( AABB3(	  10.0f,   10.0f, 0.0f, 400.0f, 1500.0f,   1.0f ), true );
	Block* m_floor_NW	= new Block( AABB3( -200.0f,   10.0f, 0.0f, -10.0f,  200.0f,  40.0f ), true );
	Block* m_floor_SE	= new Block( AABB3(   10.0f, -200.0f, 0.0f, 200.0f,    0.0f,  10.0f ), true );
	Block* m_floor_SW	= new Block( AABB3( -200.0f, -200.0f, 0.0f, -10.0f,    0.0f,  20.0f ), true );
	// Boxes
	Block* m_box1_E		= new Block( AABB3(    0.0f,    0.0f, 0.0f,  50.0f,   50.0f,  50.0f ), true );
	Block* m_box2_NE	= new Block( AABB3(    0.0f,    0.0f, 0.0f,  50.0f,   50.0f,  50.0f ), true );
	Block* m_box3_SE	= new Block( AABB3(    0.0f,    0.0f, 0.0f,  50.0f,   50.0f,  50.0f ), true );
	Block* m_box4_SE	= new Block( AABB3(    0.0f,    0.0f, 0.0f,  50.0f,   50.0f,  50.0f ), true );
	// Stairs
	Block* m_stairs1	= new Block( AABB3(    0.0f,    0.0f, 0.0f,  10.0f,   50.0f,  50.0f ), true );
	Block* m_stairs2	= new Block( AABB3(    0.0f,    0.0f, 0.0f,  10.0f,   50.0f,  50.0f ), true );
	Block* m_stairs3	= new Block( AABB3(    0.0f,    0.0f, 0.0f,  10.0f,   50.0f,  50.0f ), true );
	Block* m_stairs4	= new Block( AABB3(    0.0f,    0.0f, 0.0f,  10.0f,   50.0f,  50.0f ), true );
	Block* m_stairs5	= new Block( AABB3(    0.0f,    0.0f, 0.0f,  10.0f,   50.0f,  50.0f ), true );
	Block* m_stairs6	= new Block( AABB3(    0.0f,    0.0f, 0.0f,  10.0f,   50.0f,  50.0f ), true );
	Block* m_stairs7	= new Block( AABB3(    0.0f,    0.0f, 0.0f,  10.0f,   50.0f,  50.0f ), true );
	Block* m_stairs8	= new Block( AABB3(    0.0f,    0.0f, 0.0f,  10.0f,   50.0f,  50.0f ), true );
	Block* m_stairs9	= new Block( AABB3(    0.0f,    0.0f, 0.0f,  10.0f,   50.0f,  50.0f ), true );
	Block* m_stairs10	= new Block( AABB3(    0.0f,    0.0f, 0.0f,  10.0f,   50.0f,  50.0f ), true );
	// Slopes
	Block* m_slope_SE	= new Block( AABB3(    0.0f,    0.0f, 0.0f,  20.0f,   50.0f,  50.0f ), true );
	// Elevator
	Block* m_elevator_1 = new Block( AABB3(    0.0f,    0.0f, 0.0f, 100.0f,  100.0f,   1.0f ), true );
	Block* m_elevator_2 = new Block( AABB3(    0.0f,    0.0f, 0.0f, 100.0f,  100.0f,   1.0f ), true );
	// Cliff
	Block* m_cliff		= new Block( AABB3(    0.0f,    0.0f, 0.0f, 100.0f,  100.0f, 100.0f ), false, true );

	//----------------------------------------------------------------------------------------------------------------------
	// Raycast Result
	//----------------------------------------------------------------------------------------------------------------------
	//----------------------------------------------------------------------------------------------------------------------
	// Mount Height Ray
	Raycast_old m_raycast_Mount;
	Raycast_old m_raycast_FWD;
	//----------------------------------------------------------------------------------------------------------------------
	// Movement 
	// Fwd Ray
	RaycastResult3D		m_raycastResult_FWD					= RaycastResult3D();		
	Vec3				m_updatedImpactPos_FWD				= Vec3::ZERO;
	Vec3				m_updatedImpactNormal_FWD			= Vec3::ZERO;
	Vec3				m_rayStartPos_FWD					= Vec3::ZERO;
	Vec3				m_rayEndPos_FWD						= Vec3::ZERO;
	bool				m_didRayImpact_FWD					= false;
	bool				m_didRayImpactClimbableObject_FWD	= false;
	//----------------------------------------------------------------------------------------------------------------------
	// Left arm. 
	// "LAD" means "Left arm down"
	Raycast_old m_raycast_LeftArmDown;
	// "LAF" means "Left arm forward"
	Raycast_old m_raycast_LeftArmFwd;
	// "NLAD" means "Next Left arm down"
	Raycast_old m_raycast_NextLeftArmDown;
	//----------------------------------------------------------------------------------------------------------------------
	// Right Arm. 
	// "RAD" means "right arm down"
	Raycast_old m_raycast_rightArmDown;
	// "RAF" means "right arm forward"
	Raycast_old m_raycast_rightArmFwd;
	// NRAD (Next Right Arm Down)
	Raycast_old m_raycast_NextRightArmDown;	
	//----------------------------------------------------------------------------------------------------------------------
	// Right Feet
	// "RFD" means "right foot down"
	Raycast_old m_raycast_rightFootDown;	
	// "RFF" means "right foot fwd"
	Raycast_old m_raycast_rightFootFwd;		
	//----------------------------------------------------------------------------------------------------------------------
	// Left Foot
	// "LFD" means "left Foot down"
	Raycast_old m_raycast_leftFootDown;	
	// "LFF" means "left foot fwd"
	Raycast_old m_raycast_LeftFootFwd;		
	//----------------------------------------------------------------------------------------------------------------------
	// Common raycast variables
	float				m_raylength_Long			= 50.0f;
	float				m_raylength_Short			= 25.0f;
	float				m_arrowThickness			=  0.5f;
	Rgba8				m_rayDefaultColor			= Rgba8::MAGENTA;
	Rgba8				m_rayImpactDistColor		= Rgba8::RED;
	Rgba8				m_rayImpactDistColor2		= Rgba8::DARKER_GREEN;
	Rgba8				m_rayImpactDiscColor		= Rgba8::WHITE;
	Rgba8				m_rayImpactNormalColor		= Rgba8::DARK_BLUE;

	//----------------------------------------------------------------------------------------------------------------------
	// Debug variables
	//----------------------------------------------------------------------------------------------------------------------
	float m_distCamAwayFromPlayer = 10.0f;

	//----------------------------------------------------------------------------------------------------------------------
	// Bezier Curves
	//----------------------------------------------------------------------------------------------------------------------
	// Right Arm
	CubicBezierCurve3D  m_bezierCurve_RightArm	= CubicBezierCurve3D( Vec3( -40.0f, 0.0f, 0.0f ), Vec3( -20.0f, 0.0f, 25.0f ), Vec3( 20.0f, 0.0f, 25.0f ), Vec3( 40.0f, 0.0f, 0.0f ) );
	Stopwatch			m_timer_RightArm		= Stopwatch( &g_theApp->m_gameClock, 1.0f );
	// Left Arm
	CubicBezierCurve3D  m_bezierCurve_LeftArm	= CubicBezierCurve3D( Vec3( -40.0f, 0.0f, 0.0f ), Vec3( -20.0f, 0.0f, 25.0f ), Vec3( 20.0f, 0.0f, 25.0f ), Vec3( 40.0f, 0.0f, 0.0f ) );
	Stopwatch			m_timer_LeftArm			= Stopwatch( &g_theApp->m_gameClock, 1.0f );
	// Right Foot
	CubicBezierCurve3D  m_bezierCurve_RightFoot = CubicBezierCurve3D( Vec3( -40.0f, 0.0f, 0.0f ), Vec3( -20.0f, 0.0f, 25.0f ), Vec3( 20.0f, 0.0f, 25.0f ), Vec3( 40.0f, 0.0f, 0.0f ) );
	Stopwatch			m_timer_RightFoot		= Stopwatch( &g_theApp->m_gameClock, 1.0f );
	// Left Foot
	CubicBezierCurve3D  m_bezierCurve_LeftFoot  = CubicBezierCurve3D( Vec3( -40.0f, 0.0f, 0.0f ), Vec3( -20.0f, 0.0f, 25.0f ), Vec3( 20.0f, 0.0f, 25.0f ), Vec3( 40.0f, 0.0f, 0.0f ) );
	Stopwatch			m_timer_LeftFoot		= Stopwatch( &g_theApp->m_gameClock, 1.0f );
};