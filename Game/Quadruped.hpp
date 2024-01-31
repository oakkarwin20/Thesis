#pragma once

#include "Engine/SkeletalSystem/CreatureBase.hpp"


//----------------------------------------------------------------------------------------------------------------------
class GameMode3D;
class GameMode_BipedWalkAnim_3D;


//----------------------------------------------------------------------------------------------------------------------
struct Raycast
{
	RaycastResult3D		m_raycastResult			= RaycastResult3D();
	Vec3				m_rayEndPos				= Vec3::ZERO;
};


//----------------------------------------------------------------------------------------------------------------------
class Quadruped : public CreatureBase
{
public:
	Quadruped( GameMode3D* game,						  Vec3 const& rootStartPos, float limbLength = 1.0f );
	Quadruped( GameMode_BipedWalkAnim_3D* gameMode_Biped, Vec3 const& rootStartPos, float limbLength = 1.0f );
	~Quadruped();

	void InitLimbs();
	void UpdateLimbs( float deltaSeconds );
	
	void MoveIfNeeded(	IK_Chain3D*			const  ikChain, 
						IK_Chain3D*			const  anchorToggleSkeleton, 
						IK_Joint3D*		const  refSegment, 
						Vec3				const& refPosition,		
						float maxDistFromRef, float maxLength, float fwdStep, float leftStep, 
						CubicBezierCurve3D& bezierCurve, Stopwatch& bezierTimer );

	void SpecifyFootPlacementPos( Vec3& targetPos, IK_Joint3D* refLimb, float maxLength, float fwdStepAmount, float leftStepAmount );
	Vec3 ComputeIdealStepPos( IK_Joint3D const* refLimb, float fwdStepAmound, float leftStepAmount );
	bool DoesRaycastHitFloor( Vec3& refPosition );
	bool DoesRaycastHitFloor( RaycastResult3D& raycastResult, Vec3 rayStartPos, Vec3& rayfwdNormal, float rayLength );
	void UpdateLimbEndToRayImpactPos( Stopwatch& bezierTimer, Raycast& raycast, IK_Chain3D* const skeleton );
	void InitStepBezier	( CubicBezierCurve3D& bezierCurve, IK_Chain3D* const skeleton, Vec3 const& refUpDir, Stopwatch& bezierTimer );
	void UpdateBezier	( CubicBezierCurve3D& bezierCurve, IK_Chain3D* const skeleton, Stopwatch& bezierTimer );

	// Debug rendering
	void DebugRenderBezier( std::vector<Vertex_PCU>& verts, CubicBezierCurve3D const& bezierCurve, Stopwatch const& timer );
	void DebugRenderRaycasts( std::vector<Vertex_PCU>& verts );

public:
	GameMode3D*					m_game						= nullptr;
	GameMode_BipedWalkAnim_3D*	m_gameMode_Biped			= nullptr;
	float						m_offsetRootToHip_Biped		=  5.0f;
	float						m_defaultHeightZ			= 15.0f;
	float						m_numArms					= 2.0f;
	float						m_numHips					= 6.0f;
	float						m_hipLength					= 5.0f;
	float						m_limbLength				= 10.0f;
	float						m_halfLimbLength			= m_limbLength * 0.5f;

	// Skeleton System pointers
	IK_Chain3D* m_hip		 = nullptr;
	IK_Chain3D* m_leftArm   = nullptr;
	IK_Chain3D* m_rightArm  = nullptr;
	IK_Chain3D* m_leftFoot  = nullptr;
	IK_Chain3D* m_rightFoot = nullptr;
	IK_Chain3D* m_leftPalm  = nullptr;
	IK_Chain3D* m_rightPalm = nullptr;
	IK_Chain3D* m_neck		 = nullptr;
	IK_Chain3D* m_head		 = nullptr;

	// Bezier curve and timer
	float stepTimer = 1.0f;
	// Left Arm
	CubicBezierCurve3D  m_bezier_leftArm		= CubicBezierCurve3D( Vec3( -40.0f, 0.0f, 0.0f ), Vec3( -20.0f, 0.0f, 25.0f ), Vec3( 20.0f, 0.0f, 25.0f ), Vec3( 40.0f, 0.0f, 0.0f ) );
	Stopwatch			m_bezierTimer_leftArm	= Stopwatch( &g_theApp->m_gameClock, stepTimer );
	// Right Arm
	CubicBezierCurve3D  m_bezier_rightArm		= CubicBezierCurve3D( Vec3( -40.0f, 0.0f, 0.0f ), Vec3( -20.0f, 0.0f, 25.0f ), Vec3( 20.0f, 0.0f, 25.0f ), Vec3( 40.0f, 0.0f, 0.0f ) );
	Stopwatch			m_bezierTimer_rightArm	= Stopwatch( &g_theApp->m_gameClock, stepTimer );
	// Left Foot
	CubicBezierCurve3D  m_bezier_leftFoot		= CubicBezierCurve3D( Vec3( -40.0f, 0.0f, 0.0f ), Vec3( -20.0f, 0.0f, 25.0f ), Vec3( 20.0f, 0.0f, 25.0f ), Vec3( 40.0f, 0.0f, 0.0f ) );
	Stopwatch			m_bezierTimer_leftFoot	= Stopwatch( &g_theApp->m_gameClock, stepTimer );
	// Right Foot
	CubicBezierCurve3D  m_bezier_rightFoot		= CubicBezierCurve3D( Vec3( -40.0f, 0.0f, 0.0f ), Vec3( -20.0f, 0.0f, 25.0f ), Vec3( 20.0f, 0.0f, 25.0f ), Vec3( 40.0f, 0.0f, 0.0f ) );
	Stopwatch			m_bezierTimer_rightFoot	= Stopwatch( &g_theApp->m_gameClock, stepTimer );

	//----------------------------------------------------------------------------------------------------------------------
	// Debug Variables
	//----------------------------------------------------------------------------------------------------------------------
	RaycastResult3D m_rayResult_Blocks;
	RaycastResult3D m_rayResult_Tri;
	Vec3			m_debugVector		= Vec3::ZERO;

	//----------------------------------------------------------------------------------------------------------------------
	// Raycasts
	//----------------------------------------------------------------------------------------------------------------------
	// Left arm
	Raycast m_raycast_LeftArmDown;
	Raycast m_raycast_RightArmDown;
	Raycast m_raycast_LeftFootDown;
	Raycast m_raycast_RightFootDown;
};
