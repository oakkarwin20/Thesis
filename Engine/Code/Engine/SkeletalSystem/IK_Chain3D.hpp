 #pragma once

#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/SkeletalSystem/IK_Joint3D.hpp"

#include <string>
#include <map>
#include <vector>


//----------------------------------------------------------------------------------------------------------------------
class CreatureBase;
class BitmapFont;

//----------------------------------------------------------------------------------------------------------------------
enum ChainSolveType
{
	CHAIN_SOLVER_FABRIK,
	CHAIN_SOLVER_CCD,
	CHAIN_SOLVER_NUM,
};

//----------------------------------------------------------------------------------------------------------------------
enum AnchorStates
{
	ANCHOR_STATE_LOCKED,		//  Is grounded, not moving,  is locked.			// Limb is placed on a valid position and is currently locked, it cannot be moved.
	ANCHOR_STATE_MOVING,		//  Not grounded,  is moving, not locked.			// Limb is moving to a valid position and is NOT locked. 
	ANCHOR_STATE_FREE,			//  Is grounded, not moving, not locked.			// Limb finished moving, is grounded, and is placed in a valid position, but it is NOT locked.
	ANCHOR_STATE_NUM,
};


//----------------------------------------------------------------------------------------------------------------------
struct Target
{
	Target() {};
	Target( Vec3 currentPos, Vec3 goalPos, Vec3 fwdDir = Vec3( 1.0f, 0.0f, 0.0f ), Vec3 leftDir = Vec3( 0.0f, 1.0f, 0.0f ), Vec3 upDir = Vec3( 0.0f, 0.0f, 1.0f ), Rgba8 color = Rgba8::WHITE )
	{
		m_currentPos	= currentPos;
		m_goalPos		= goalPos;
		m_fwdDir		= fwdDir;
		m_leftDir		= leftDir;
		m_upDir			= upDir;
		m_color			= color;
	};
	~Target() {};

	Vec3	m_currentPos	= Vec3::ZERO;
	Vec3	m_goalPos		= Vec3::ZERO;
	Vec3	m_fwdDir		= Vec3( 1.0f, 0.0f, 0.0f );
	Vec3	m_leftDir		= Vec3( 0.0f, 1.0f, 0.0f );
	Vec3	m_upDir			= Vec3( 0.0f, 0.0f, 1.0f );
	Rgba8	m_color			= Rgba8::WHITE;
};


//----------------------------------------------------------------------------------------------------------------------
class IK_Chain3D
{
public:
	IK_Chain3D( std::string name, Vec3 position, IK_Joint3D* ownerSkeletonFirstJoint = nullptr, CreatureBase* const creatureOwner = nullptr, bool shouldReachInsteadOfDrag = true );
	~IK_Chain3D();

	void	Startup();
	void	Shutdown();
	void	Update();
	void	Render( 
					std::vector<Vertex_PCU>&	verts, 
					Rgba8 const&				limbColor, 
					Rgba8 const&				jointColor, 
					bool						renderDebugJointBasis		= false, 
					bool						renderDebugCurrentPos_EE	= false )	const;

	void CreateNewJoint	 (	Vec3				const&	position_localSpace, 
						 	EulerAngles					orientation_localSpace	= EulerAngles(), 
						 	FloatRange					yawConstraints			= FloatRange( -180.0f, 180.0f ), 
						 	FloatRange					pitchConstraints		= FloatRange( -180.0f, 180.0f ), 
						 	FloatRange					rollConstraints 		= FloatRange( -180.0f, 180.0f ) 
						 );

	void	CreateNewLimb(  int							limbIndex, 
							float						length, 
							Vec3				const&	fwdDir					= Vec3::X_FWD, 
							JointConstraintType			jointConstraintType		= JOINT_CONSTRAINT_TYPE_EULER, 
							IK_Chain3D*					IK_Chain				= nullptr, 
							FloatRange					yawConstraints			= FloatRange( -180.0f, 180.0f ), 
							FloatRange					pitchConstraints		= FloatRange( -180.0f, 180.0f ), 
							FloatRange					rollConstraints			= FloatRange( -180.0f, 180.0f )
						 );

	void	CreateNewLimbs(	float						limbLength, 
							float						numLimbs, 
							Vec3				const&	fwdDir				= Vec3(1.0f, 0.0f, 0.0f), 
							bool				const&	isFinalLimbSubBase	= false, 
							JointConstraintType const&  jointConstraintType = JOINT_CONSTRAINT_TYPE_DISTANCE, 
							FloatRange			const&	yawConstraints		= FloatRange(-180.0f, 180.0f), 
							FloatRange			const&	pitchConstraints	= FloatRange(-180.0f, 180.0f), 
							FloatRange			const&	rollConstraints		= FloatRange(-180.0f, 180.0f)
						   );

	// Render Functions
	void RenderTarget_IJK					( std::vector<Vertex_PCU>& verts, float endPosLength )																		const;
	void RenderTarget_EE					( std::vector<Vertex_PCU>& verts, float radius = 2.0f )																		const;
	void DebugDrawJoints_IJK				( std::vector<Vertex_PCU>& verts, float arrowThickness = 0.1f, float endPosLength = 3.0f )									const;
	void DebugDrawConstraints_YPR			( std::vector<Vertex_PCU>& verts, float length )																			const;
	void DebugDrawParentBasis				( std::vector<Vertex_PCU>& verts, float length = 4.0f )																		const;

	void DebugDrawTarget					( std::vector<Vertex_PCU>& verts, float			radius,		
																			  Rgba8 const&  color			= Rgba8::WHITE, 
																			  bool			renderBasis		= false,
																			  float			basisLength		= 1.0f )													const;

	void DebugTextJointPos_WorldSpace		( std::vector<Vertex_PCU>& verts, float			textHeight, 
																			  Vec3 const&	camera_jBasis, 
																			  Vec3 const&	camera_kBasis, 
																			  BitmapFont*	textFont  )																	const;

	void DebugTextJointPos_LocalSpace		( std::vector<Vertex_PCU>& verts, float			textHeight, 
																			  Vec3 const&	camera_jBasis, 
																			  Vec3 const&	camera_kBasis, 
																			  BitmapFont*	textFont  )																	const;

	void DebugTextConstraints_YPR			( std::vector<Vertex_PCU>& verts, float			textHeight, 
																			  float			lengthToTextPosFromStart, 
																			  float			heightOffset, 
																			  Vec3 const&	jBasis, 
																			  Vec3 const&	kBasis, 
																			  BitmapFont*	textFont )																	const;


	//----------------------------------------------------------------------------------------------------------------------
	// Iterative Solver Functions
	//----------------------------------------------------------------------------------------------------------------------
	// CCD
	void	Solve_CCD		( Target target );
	void	CCD_Forward		( Target target );
	// FABRIK
	void	FABRIK_Forward	( Target target );
	void	Solve_FABRIK	( Target target );
	void	FABRIK_Backward ( Target target );
	void	OnlyChild_Backwards			( IK_Joint3D* const currentLimb, Target target );
	void	FirstChild_Backwards		( IK_Joint3D* const currentLimb, Target target );
	void	FinalChild_Backwards		( IK_Joint3D* const currentLimb, Target target );
	void	HasChildAndParents_Backwards( IK_Joint3D* const currentLimb );
	void	ConstrainYPR_Backwards		( IK_Joint3D* const currentLimb, Target target );

	//----------------------------------------------------------------------------------------------------------------------
	// Analytical Solver Functions
	//----------------------------------------------------------------------------------------------------------------------
	void	SolveTwoBoneIK_TriangulationMethod( Target target );
	void	ComputeBendAngle_Cos_Sine( IK_Joint3D* const limbA, IK_Joint3D* const limbB, Target target, Vec3 const& limbStartPos );

	//----------------------------------------------------------------------------------------------------------------------
	// Queries
	//----------------------------------------------------------------------------------------------------------------------
	bool		CanMove();
	bool		TryUnlockAndToggleAnchor( IK_Chain3D* const refSkeleton, bool didRayImpact = false, Vec3 const& updatedRayImpactPos = Vec3::ZERO );		// GenericRefactoring
	bool		UpdateDistEeToTarget_ALSO_CHECK_IfDistChangedSinceLastFrame( Target target );
	bool		IsAnyJointBentToMaxConstraints();

	//----------------------------------------------------------------------------------------------------------------------
	// Util Functions
	//----------------------------------------------------------------------------------------------------------------------
	float		GetMaxLengthOfSkeleton();
	void		UpdateTargetOrientationToRef( Vec3 const& fwd, Vec3 const& left, Vec3 const& up );
	EulerAngles	GetEulerFromFwdDir( IK_Joint3D* curJoint, Vec3 const& fwdDir );
	EulerAngles	GetEulerFromFwdAndLeft( Vec3 const& fwdDir, Vec3 const& leftDir );
	void		ResetAllJointsEuler();
	float		GetDistEeToTarget( Target target );

	//----------------------------------------------------------------------------------------------------------------------
	// Matrix Util transform functions
	//----------------------------------------------------------------------------------------------------------------------

	//----------------------------------------------------------------------------------------------------------------------
	// Anchor states
	//----------------------------------------------------------------------------------------------------------------------
	// Setters 
	void SetAnchor_Locked();
	void SetAnchor_Moving();
	void SetAnchor_Free();
	// Getters
	bool IsAnchor_Locked();
	bool IsAnchor_Moving();
	bool IsAnchor_Free();

public:
	// Core variables in local space
	std::string					m_name							= std::string( "Un-Named" );
	Vec3						m_position_WS					= Vec3::ZERO;
	EulerAngles					m_eulerAngles_WS				= EulerAngles();
	std::vector<IK_Joint3D*>	m_jointList;
	IK_Joint3D*					m_finalJoint					= nullptr;			// The limb at the end of the limbList
	IK_Joint3D*					m_firstJoint					= nullptr;			// The limb at the start of the limbList. Aka, parent
	bool						m_shouldReachInsteadOfDrag		= true;
	bool						m_shouldBendMore				= false;			// This should be true when 'Deadlock' occurs and the entire chain needs to bendMore "away from its target"
	CreatureBase*				m_creatureOwner					= nullptr;
	IK_Joint3D*					m_ownerSkeletonFirstJoint		= nullptr;			
	Vec3						m_localOffsetToOwner			= Vec3::ZERO;		
	std::vector<IK_Chain3D*>	m_childChainList;
	IK_Chain3D*					m_parentChain					= nullptr;
	bool						m_shouldRender					= true;
	float						m_distEeToTarget				= 99999.9f;

	// End Effector
	Target		m_target;
	float		m_prevDistEE_EndToTarget	= 0.0f;
	Vec3		m_targetPos_LastFrame		= Vec3::ZERO;
	// Anchored
	AnchorStates m_anchorState = ANCHOR_STATE_LOCKED;

	//----------------------------------------------------------------------------------------------------------------------
	// Debug variables for screen printing
	//----------------------------------------------------------------------------------------------------------------------
	float m_degrees			= 0.0f;
	float m_clampedDegrees	= 0.0f;
	Vec3  m_debugVector		= Vec3::ZERO;

	int  m_iterCount			= 0;

	bool m_isSingleStep_Debug	= false;
	bool m_breakFABRIK			= false;

	ChainSolveType m_solverType = CHAIN_SOLVER_FABRIK;

	float m_bestDistSolvedThisFrame = 0.0f;
};