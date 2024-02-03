#include "Game/GameModeBase.hpp"
#include "Game/GameMode_BipedWalkAnim_3D.hpp"
#include "Game/GameMode3D.hpp"
#include "Game/Quadruped.hpp"

#include "Engine/Core/Time.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/SkeletalSystem/IK_Chain3D.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"


//----------------------------------------------------------------------------------------------------------------------
Quadruped::Quadruped( GameMode3D* game, Vec3 const& rootStartPos, float limbLength ) : CreatureBase( rootStartPos, limbLength )
{
	m_game = game;
}


//----------------------------------------------------------------------------------------------------------------------
Quadruped::Quadruped( GameMode_BipedWalkAnim_3D* gameMode_Biped, Vec3 const& rootStartPos, float limbLength /*= FloatRange( 1.0f, 1.0f ) */ ) : CreatureBase( rootStartPos, limbLength )
{
	m_gameMode_Biped = gameMode_Biped;
}

//----------------------------------------------------------------------------------------------------------------------
Quadruped::~Quadruped()
{
}


//----------------------------------------------------------------------------------------------------------------------
void Quadruped::InitLimbs()
{
	//----------------------------------------------------------------------------------------------------------------------
	// 1. Create limbs
	//----------------------------------------------------------------------------------------------------------------------
	// Left arms and shoulders
	Vec3 leftShoulder	= m_root->m_jointPos_LS + ( m_root->m_leftDir * m_offsetRootToHip_Biped );
	CreateChildSkeletalSystem(  "leftArm", leftShoulder, nullptr, this  );
	CreateLimbsForIKChain	 ( "leftArm", 2, m_limbLength, Vec3::X_FWD, JOINT_CONSTRAINT_TYPE_EULER );									// Thigh
	// Right arms and shoulders
	Vec3 rightShoulder	= m_root->m_jointPos_LS + ( -m_root->m_leftDir * m_offsetRootToHip_Biped );
	CreateChildSkeletalSystem( "rightArm", rightShoulder, nullptr, this );
	CreateLimbsForIKChain	 ( "rightArm", 2, m_limbLength, Vec3::X_FWD, JOINT_CONSTRAINT_TYPE_EULER );									// Thigh
	// Hip (Spine)
	Vec3 spineStart		= m_root->m_jointPos_LS;
	CreateChildSkeletalSystem(  "spine", spineStart, nullptr, this );
	CreateLimbsForIKChain	 (  "spine", m_numSpineSegments, m_spineLength, -Vec3::X_FWD, JOINT_CONSTRAINT_TYPE_EULER, FloatRange( -165.0f, 165.0f ), FloatRange( -165.0f, 165.0f ) );
	m_spine				= GetSkeletonByName( "spine" );
	// Legs
	Vec3 leftHip		= m_spine->m_finalJoint->m_endPos - ( m_spine->m_finalJoint->m_leftDir * m_offsetRootToHip_Biped ) + Vec3( 10.0f, 0.0f, 0.0f );
	Vec3 rightHip		= m_spine->m_finalJoint->m_endPos + ( m_spine->m_finalJoint->m_leftDir * m_offsetRootToHip_Biped ) + Vec3( 10.0f, 0.0f, 0.0f );
	CreateChildSkeletalSystem(  "leftFoot",  leftHip, m_spine->m_finalJoint, this );
	CreateChildSkeletalSystem( "rightFoot", rightHip, m_spine->m_finalJoint, this );
	CreateLimbsForIKChain	 (  "leftFoot", m_numArms, m_limbLength );
	CreateLimbsForIKChain	 ( "rightFoot", m_numArms, m_limbLength );
	// Neck
	CreateChildSkeletalSystem ( "neck", Vec3::ZERO, nullptr, this );
	CreateLimbsForIKChain	  ( "neck", 1, m_limbLength, Vec3::X_FWD, JOINT_CONSTRAINT_TYPE_EULER );
	m_neck = GetSkeletonByName( "neck" );
	// Head
	CreateChildSkeletalSystem ( "head", Vec3::ZERO, nullptr, this );
	CreateLimbsForIKChain	  ( "head", 1, m_halfLimbLength, Vec3::X_FWD, JOINT_CONSTRAINT_TYPE_EULER );
	m_head = GetSkeletonByName( "head" );

	//----------------------------------------------------------------------------------------------------------------------
	// 2. Get pointers to limbs
	//----------------------------------------------------------------------------------------------------------------------
	m_leftArm	= GetSkeletonByName( "leftArm"   );
	m_rightArm	= GetSkeletonByName( "rightArm"  );
	m_leftFoot	= GetSkeletonByName( "leftFoot"  );
	m_rightFoot	= GetSkeletonByName( "rightFoot" );

	//----------------------------------------------------------------------------------------------------------------------
	// Create Palms
	//----------------------------------------------------------------------------------------------------------------------
	// Left  Palm
	CreateChildSkeletalSystem	  ( "leftPalm", Vec3::ZERO, nullptr, this );
	CreateLimbsForIKChain		  ( "leftPalm", 1, m_halfLimbLength, Vec3::X_FWD, JOINT_CONSTRAINT_TYPE_EULER );
	m_leftPalm = GetSkeletonByName( "leftPalm" );
	// Right Palm
	CreateChildSkeletalSystem	   ( "rightPalm", Vec3::ZERO, nullptr, this );
	CreateLimbsForIKChain		   ( "rightPalm", 1, m_halfLimbLength, Vec3::X_FWD, JOINT_CONSTRAINT_TYPE_EULER );
	m_rightPalm = GetSkeletonByName( "rightPalm" );
}


//----------------------------------------------------------------------------------------------------------------------
void Quadruped::UpdateLimbs( float deltaSeconds )
{
	//----------------------------------------------------------------------------------------------------------------------
	// Update Quadruped Hip EndEffector
	//----------------------------------------------------------------------------------------------------------------------
	m_spine->m_position_WS			= m_root->m_jointPos_LS;
	m_spine->m_target.m_currentPos	= m_root->m_jointPos_LS - ( m_root->m_fwdDir * (m_spineLength * (m_numSpineSegments + 1) ) );
//	m_spine->m_target.m_fwdDir		= m_root->m_fwdDir;
//	m_spine->m_target.m_leftDir		= m_root->m_leftDir;
//	m_spine->m_target.m_upDir		= m_root->m_upDir;

	//----------------------------------------------------------------------------------------------------------------------
	// Update pole vectors (left & right arms)
	Vec3 upNess								= -( m_rightArm->m_firstJoint->m_upDir  * 10.0f );
	Vec3 fwdNess							=  ( m_rightArm->m_firstJoint->m_fwdDir *  5.0f );
	m_rightArm->m_firstJoint->m_poleVector	= m_rightArm->m_firstJoint->m_jointPos_LS + fwdNess + upNess;
	m_leftArm->m_firstJoint->m_poleVector	=  m_leftArm->m_firstJoint->m_jointPos_LS + fwdNess + upNess;
	// Update pole vectors (left & right feet)
	upNess									= ( m_rightFoot->m_firstJoint->m_upDir  * 10.0f );
	fwdNess									= ( m_rightFoot->m_firstJoint->m_fwdDir *  5.0f );
	m_rightFoot->m_firstJoint->m_poleVector	= m_rightFoot->m_firstJoint->m_jointPos_LS + fwdNess + upNess;
	m_leftFoot->m_firstJoint->m_poleVector	=  m_leftFoot->m_firstJoint->m_jointPos_LS + fwdNess + upNess;
	//----------------------------------------------------------------------------------------------------------------------

	// Init common step variables
	float skeletonMaxLength			 = m_leftArm->GetMaxLengthOfSkeleton() + 2.0f;
	float maxDistStartPosToNewPos	 = m_limbLength * 0.5f;
	float fwdStep					 = m_limbLength * 0.9f; 
	float leftStep					 = m_offsetRootToHip_Biped; 

	float leftArmDistRoot   = GetDistance3D(   m_leftArm->m_finalJoint->m_endPos, m_root->m_jointPos_LS );
	float rightArmDistRoot  = GetDistance3D(   m_leftArm->m_finalJoint->m_endPos, m_root->m_jointPos_LS );
	float leftFootDistRoot  = GetDistance3D(  m_leftFoot->m_finalJoint->m_endPos, m_spine->m_position_WS );
	float rightFootDistRoot = GetDistance3D( m_rightFoot->m_finalJoint->m_endPos, m_spine->m_position_WS );
	DebuggerPrintf( Stringf( "LA: %0.2f, RA: %0.2f,\nLF: %0.2f, RF: %0.2f, MaxLimbLength: %0.2f\n", leftArmDistRoot, rightArmDistRoot,  leftFootDistRoot, rightFootDistRoot, skeletonMaxLength ).c_str() );
	//----------------------------------------------------------------------------------------------------------------------			
	// Arms + Feet
	//----------------------------------------------------------------------------------------------------------------------
	// Left arm
	MoveIfNeeded( m_leftArm, m_rightArm, m_root, m_root->m_jointPos_LS, skeletonMaxLength, maxDistStartPosToNewPos, fwdStep, leftStep, m_bezier_leftArm, m_bezierTimer_leftArm );
	UpdateLimbEndToRayImpactPos( m_bezierTimer_leftArm, m_raycast_LeftArmDown, m_leftArm );
	//----------------------------------------------------------------------------------------------------------------------
	// Right arm
	MoveIfNeeded( m_rightArm, m_leftArm, m_root, m_root->m_jointPos_LS, skeletonMaxLength, maxDistStartPosToNewPos, fwdStep, -leftStep, m_bezier_rightArm, m_bezierTimer_rightArm );
	UpdateLimbEndToRayImpactPos( m_bezierTimer_rightArm, m_raycast_RightArmDown, m_rightArm );
	// Left Foot
	fwdStep	= m_limbLength * 0.5f;
	MoveIfNeeded( m_leftFoot, m_rightFoot, m_spine->m_finalJoint, m_spine->m_finalJoint->m_endPos, skeletonMaxLength, maxDistStartPosToNewPos, fwdStep, -leftStep,  m_bezier_leftFoot, m_bezierTimer_leftFoot );
	UpdateLimbEndToRayImpactPos( m_bezierTimer_leftFoot, m_raycast_LeftFootDown, m_leftFoot );
//	// Right Foot
	MoveIfNeeded( m_rightFoot, m_leftFoot, m_spine->m_finalJoint, m_spine->m_finalJoint->m_endPos, skeletonMaxLength, maxDistStartPosToNewPos, fwdStep,  leftStep, m_bezier_rightFoot, m_bezierTimer_rightFoot );
	UpdateLimbEndToRayImpactPos( m_bezierTimer_rightFoot, m_raycast_RightFootDown, m_rightFoot );
	
	//----------------------------------------------------------------------------------------------------------------------
	// Update palm positions
	//----------------------------------------------------------------------------------------------------------------------
	Vec3 fwd								= RotateVectorAboutArbitraryAxis( m_raycast_LeftArmDown.m_raycastResult.m_impactNormal, m_leftArm->m_target.m_leftDir, 90.0f );
	fwd.Normalize();
	m_debugVector							= fwd;
	m_leftPalm->m_position_WS					= m_leftArm->m_finalJoint->m_endPos;
	m_rightPalm->m_position_WS					= m_rightArm->m_finalJoint->m_endPos;
	m_leftPalm->m_target.m_currentPos		= m_leftPalm->m_position_WS  + fwd * 2.0f;
	fwd										= RotateVectorAboutArbitraryAxis( m_raycast_RightArmDown.m_raycastResult.m_impactNormal, m_rightArm->m_target.m_leftDir, 90.0f );
	fwd.Normalize();
	m_rightPalm->m_target.m_currentPos		= m_rightPalm->m_position_WS + fwd * 2.0f;
	m_leftPalm->m_ownerSkeletonFirstJoint	= nullptr;
	m_rightPalm->m_ownerSkeletonFirstJoint	= nullptr;
	m_leftPalm->Update();
	m_rightPalm->Update();

	//----------------------------------------------------------------------------------------------------------------------
	// Neck
	//----------------------------------------------------------------------------------------------------------------------
	// Keep neck attached to root
	m_neck->m_position_WS				= m_root->m_jointPos_LS;
	// Have neck reach out to head
	Vec3 rootFwdDir						= m_root->m_eulerAngles_LS.GetForwardDir_XFwd_YLeft_ZUp();
	m_neck->m_target.m_goalPos			= ( m_root->m_jointPos_LS + ( rootFwdDir * m_neck->m_firstJoint->m_distToChild ) ) + Vec3( 0.0f, 0.0f, 10.0f );
	m_neck->m_ownerSkeletonFirstJoint	= nullptr;
	m_neck->Update();
	// Lerp neck to goal
	float fractionTowardsEnd			= 0.1f + ( deltaSeconds * 2.0f );
	m_neck->m_target.m_currentPos		= Interpolate( m_neck->m_target.m_currentPos, m_neck->m_target.m_goalPos, fractionTowardsEnd );

	//----------------------------------------------------------------------------------------------------------------------
	// Head
	//----------------------------------------------------------------------------------------------------------------------
	// Keep head in "place"
	m_head->m_firstJoint->m_fwdDir		= rootFwdDir;
	m_head->m_position_WS					= m_neck->m_finalJoint->m_endPos;
	m_head->m_target.m_goalPos			= m_head->m_position_WS + ( m_head->m_firstJoint->m_fwdDir * m_head->m_firstJoint->m_distToChild );
	m_head->m_ownerSkeletonFirstJoint	= nullptr;
	m_head->Update();
	// Lerp head to goal
	m_head->m_target.m_currentPos		= Interpolate( m_head->m_target.m_currentPos, m_head->m_target.m_goalPos, fractionTowardsEnd * 4.0f ); 

	//----------------------------------------------------------------------------------------------------------------------
	// Update Quadruped Height
	//----------------------------------------------------------------------------------------------------------------------
	// Root height
	float goalHeight					= ( m_rightArm->m_target.m_currentPos.z + m_leftArm->m_target.m_currentPos.z ) * 0.5f;
	goalHeight							+= m_defaultHeightZ;

	// Lerp from currentRootPos to goalPos (Breathing)
	float currentTime					= float( GetCurrentTimeSeconds() );
	float sine							= SinDegrees( currentTime * 60.0f );
	float heightOffset					= 1.0f;
	float rootGoalHeightZ				= goalHeight + ( heightOffset * sine );
	fractionTowardsEnd					= 0.01f;
	fractionTowardsEnd					+= deltaSeconds * 8.0f;
	m_root->m_jointPos_LS.z			    = Interpolate( m_root->m_jointPos_LS.z, rootGoalHeightZ, fractionTowardsEnd );

	// Hip height
	goalHeight							= ( m_rightFoot->m_target.m_currentPos.z + m_leftFoot->m_target.m_currentPos.z ) * 0.5f;
	goalHeight							+= m_defaultHeightZ;
	m_spine->m_firstJoint->m_jointPos_LS.z	= goalHeight;
}


//----------------------------------------------------------------------------------------------------------------------
void Quadruped::MoveIfNeeded( IK_Chain3D*		const  IK_Chain, 
							  IK_Chain3D*		const  anchorToggleSkeleton, 
							  IK_Joint3D*		const  refSegment, 
							  Vec3				const& refPosition,		
							  float maxDistFromRef, float maxDistStartPosToNewPos, float fwdStep, float leftStep, 
							  CubicBezierCurve3D& bezierCurve, Stopwatch& bezierTimer )
{
	if ( IsLimbTooFarFromPos( IK_Chain, refPosition, maxDistFromRef ) )
	{
		if ( IK_Chain->TryUnlockAndToggleAnchor( anchorToggleSkeleton ) )
		{
			// Determine Best next step
			SpecifyFootPlacementPos( IK_Chain->m_target.m_goalPos, refSegment, maxDistStartPosToNewPos, fwdStep, leftStep );
			// Setup and start bezier curve
			InitStepBezier( bezierCurve, IK_Chain, refSegment->m_upDir, bezierTimer );
			IK_Chain->UpdateTargetOrientationToRef( refSegment->m_fwdDir, refSegment->m_leftDir, refSegment->m_upDir );
		}
	}
	UpdateBezier( bezierCurve, IK_Chain, bezierTimer );
	IK_Chain->UpdateTargetOrientationToRef( refSegment->m_fwdDir, refSegment->m_leftDir, refSegment->m_upDir );
}


//----------------------------------------------------------------------------------------------------------------------
// #ToDo: Rename "MaxLength" to something else that makes more sense
// Current understanding of "MaxLength" is "maxDistStartPosToNewPos"
//----------------------------------------------------------------------------------------------------------------------
void Quadruped::SpecifyFootPlacementPos( Vec3& targetPos, IK_Joint3D* refLimb, float maxDistStartPosToNewPos, float fwdStepAmount, float leftStepAmount )
{
	Vec3 prevTargetPos	= targetPos;
	// Determine the ideal next step position
	Vec3 idealNewPos	= ComputeIdealStepPos( refLimb, fwdStepAmount, leftStepAmount );

	bool didRayImpact	= false;
	didRayImpact		= DoesRaycastHitFloor( idealNewPos );
	// Raycast against all tri
	// Choose closest hit (between ABB3 and Triangles)

	float distRefPosToOldAlternativePos = 500.0f;
	// Ensure ideal next step is close enough AND on a walkable block
//	float distRefPosToIdealPos = GetDistance3D( idealNewPos, refLimb->m_jointPos_LS );
//	if ( CompareIfFloatsAreEqual( distRefPosToIdealPos, maxLength, 2.0f ) && didRayImpact )
	if ( didRayImpact )
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
		for ( int i = 0; i < m_game->m_blockList.size(); i++ )
		{
			// Ensure currentBlock is "Walkable"
			Block* currentBlock = m_game->m_blockList[i];
			if ( !currentBlock->m_isWalkable )
			{
				continue;
			}

			// Get nearestPoint
			Vec3 alternativeNewPos	= currentBlock->m_aabb3.GetNearestPoint( idealNewPos );
			alternativeNewPos.z		= currentBlock->m_aabb3.m_maxs.z;

			// Determine the closest position as valid 
			float distRootToNewAlternativePos = GetDistance3D( alternativeNewPos, refLimb->m_jointPos_LS );
			if ( distRootToNewAlternativePos <= maxDistStartPosToNewPos )
			{
				nearestPoint3D					= alternativeNewPos;
				distRefPosToOldAlternativePos	= distRootToNewAlternativePos;
			}
			else
			{
				// Keep track of the closest position to creature
				if ( distRootToNewAlternativePos <= distRefPosToOldAlternativePos )
				{
					nearestPoint3D				  = alternativeNewPos;
					distRefPosToOldAlternativePos = distRootToNewAlternativePos;
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
Vec3 Quadruped::ComputeIdealStepPos( IK_Joint3D const* refLimb, float fwdStepAmount , float leftStepAmount )
{
	// Determine the ideal next step position
	Vec3 refLimbFwdDir	= refLimb->m_eulerAngles_LS.GetAsMatrix_XFwd_YLeft_ZUp().GetIBasis3D();
	Vec3 refLimbLeftDir = refLimb->m_eulerAngles_LS.GetAsMatrix_XFwd_YLeft_ZUp().GetJBasis3D();
	Vec3 idealNewPos	= Vec3( refLimb->m_jointPos_LS.x, refLimb->m_jointPos_LS.y, refLimb->m_jointPos_LS.z - m_defaultHeightZ ) +
							  ( refLimbFwdDir  * fwdStepAmount  ) +
							  ( refLimbLeftDir * leftStepAmount );
	return idealNewPos;
}


//----------------------------------------------------------------------------------------------------------------------
bool Quadruped::DoesRaycastHitFloor( Vec3& refPosition )
{
	// Use raycast to ensure the ideal next step is "placed" on a walkable block
	RaycastResult3D rayResult_Blocks;
	RaycastResult3D rayResult_Tri;
	Vec3 rayStartPos			= refPosition + Vec3( 0.0f, 0.0f, 15.0f );
	Vec3 impactPos_Blocks		= Vec3::ZERO;
	Vec3 impactPos_Tri			= Vec3::ZERO;
	Vec3 impactNormal_Blocks	= Vec3::ZERO;
	Vec3 impactNormal_Tris		= Vec3::ZERO;
	bool didRayImpactBlock		= false;
	bool didRayImpactTri		= false;
	didRayImpactBlock			= m_game->DidRaycastHitWalkableBlock( rayResult_Blocks,	rayStartPos, Vec3::NEGATIVE_Z, m_game->m_raylength_Long, impactPos_Blocks, impactNormal_Blocks	);
	didRayImpactTri				= m_game->DidRaycastHitTriangle		( rayResult_Tri,	rayStartPos, Vec3::NEGATIVE_Z, m_game->m_raylength_Long, impactPos_Tri,	   impactNormal_Tris	);

	m_rayResult_Blocks  = rayResult_Blocks;
	m_rayResult_Tri		= rayResult_Tri;

	// Raycast against all tri and blocks
	if ( didRayImpactBlock && didRayImpactTri )
	{ 
		// Choose closest hit (between ABB3 and Triangles) if BOTH hit
		if ( rayResult_Blocks.m_impactDist < rayResult_Tri.m_impactDist )
		{
			refPosition = impactPos_Blocks;
		}
		else if ( rayResult_Blocks.m_impactDist > rayResult_Tri.m_impactDist )
		{
			refPosition = impactPos_Tri;
		}
		return true;
	}
	else if ( didRayImpactBlock && !didRayImpactTri )
	{
		refPosition = impactPos_Blocks;
		return true;
	}
	else if ( !didRayImpactBlock && didRayImpactTri )
	{
		refPosition = impactPos_Tri;
		return true;
	}
	else
	{
		// No one hit
		return false;
	}
}


//----------------------------------------------------------------------------------------------------------------------
bool Quadruped::DoesRaycastHitFloor( RaycastResult3D& raycastResult, Vec3 rayStartPos, Vec3& rayfwdNormal, float rayLength )
{
	// Use raycast to ensure the ideal next step is "placed" on a walkable block
	RaycastResult3D rayResult_Blocks;
	RaycastResult3D rayResult_Tri;
	rayStartPos				   += Vec3( 0.0f, 0.0f, m_defaultHeightZ );
	Vec3 impactPos_Blocks		= Vec3::ZERO;
	Vec3 impactPos_Tri			= Vec3::ZERO;
	Vec3 impactNormal_Blocks	= Vec3::ZERO;
	Vec3 impactNormal_Tris		= Vec3::ZERO;
	bool didRayImpactBlock		= false;
	bool didRayImpactTri		= false;
	didRayImpactBlock			= m_game->DidRaycastHitWalkableBlock( rayResult_Blocks,	rayStartPos, rayfwdNormal, rayLength, impactPos_Blocks, impactNormal_Blocks	);
	didRayImpactTri				= m_game->DidRaycastHitTriangle		( rayResult_Tri,	rayStartPos, rayfwdNormal, rayLength, impactPos_Tri,	  impactNormal_Tris	);

	m_rayResult_Blocks  = rayResult_Blocks;
	m_rayResult_Tri		= rayResult_Tri;

	// Raycast against all tri and blocks
	if ( didRayImpactBlock && didRayImpactTri )
	{ 
		// Choose closest hit (between ABB3 and Triangles) if BOTH hit
		if ( rayResult_Blocks.m_impactDist < rayResult_Tri.m_impactDist )
		{
			raycastResult = m_rayResult_Blocks;
		}
		else if ( rayResult_Blocks.m_impactDist > rayResult_Tri.m_impactDist )
		{
			raycastResult = m_rayResult_Tri;
		}
		return true;
	}
	else if ( didRayImpactBlock && !didRayImpactTri )
	{
		raycastResult = m_rayResult_Blocks;
		return true;
	}
	else if ( !didRayImpactBlock && didRayImpactTri )
	{
		raycastResult = m_rayResult_Tri;
		return true;
	}
	else
	{
		// No one hit
		return false;
	}
}


//----------------------------------------------------------------------------------------------------------------------
void Quadruped::UpdateLimbEndToRayImpactPos( Stopwatch& bezierTimer, Raycast& raycast, IK_Chain3D* const skeleton )
{
	if ( bezierTimer.IsStopped() )
	{
		bool didRayHit = DoesRaycastHitFloor( raycast.m_raycastResult, skeleton->m_target.m_goalPos, Vec3::NEGATIVE_Z, 50.0f );
		if ( didRayHit )
		{
			// Step foot position to ray impact pos every frame
			skeleton->m_target.m_currentPos = raycast.m_raycastResult.m_impactPos;
		}
	}
}


//----------------------------------------------------------------------------------------------------------------------
void Quadruped::InitStepBezier( CubicBezierCurve3D& bezierCurve, IK_Chain3D* const skeleton, Vec3 const& refUpDir, Stopwatch& bezierTimer )
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
	Vec3		ownerUpDir		= refUpDir;
	Vec3		upNess			= ( maxLength * 0.25f ) * ownerUpDir;						// #ToDo: Replace ( maxLength * 0.5f ) with values from raycast approach
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
void Quadruped::UpdateBezier( CubicBezierCurve3D& bezierCurve, IK_Chain3D* const skeleton, Stopwatch& bezierTimer )
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
		Vec3 bezierLerp						 = bezierCurve.GetPointAtTime( bezierTimer.GetElapsedTime() );
		skeleton->m_target.m_currentPos = bezierLerp;
	}
}


//----------------------------------------------------------------------------------------------------------------------
void Quadruped::DebugRenderBezier( std::vector<Vertex_PCU>& verts, CubicBezierCurve3D const& bezierCurve, Stopwatch const& timer )
{
	// Bezier points
	float elaspedTime	= timer.GetElapsedTime();
	Vec3 bezierPosAtT	= bezierCurve.GetPointAtTime( elaspedTime );
	AddVertsForSphere3D( verts, bezierPosAtT, 2.0f, 8.0f, 16.0f, Rgba8::CYAN );
	AddVertsForSphere3D( verts, bezierCurve.m_startPos , 2.0f, 8.0f, 16.0f, Rgba8::RED	   );
	AddVertsForSphere3D( verts, bezierCurve.m_guidePos1, 2.0f, 8.0f, 16.0f, Rgba8::GREEN   );
	AddVertsForSphere3D( verts, bezierCurve.m_guidePos2, 2.0f, 8.0f, 16.0f, Rgba8::BLUE	   );
	AddVertsForSphere3D( verts, bezierCurve.m_endPos   , 2.0f, 8.0f, 16.0f, Rgba8::MAGENTA );
	// Bezier trail
	float thickness = 0.5f;
	Vec3 previousBezierDotPos = bezierCurve.m_startPos;
	int		m_numSubdivisions = 64;
	for ( int subdivisionIndex = 0; subdivisionIndex <= (m_numSubdivisions - 1); subdivisionIndex++ )
	{
		// Calculate subdivisions
		float t						= (1.0f / static_cast<float>(m_numSubdivisions)) * (subdivisionIndex + 1);
		Vec3 currentBezierDotPos	= bezierCurve.GetPointAtTime( t );
		// Render curved line graph
		AddVertsForLineSegment3D( verts, previousBezierDotPos, currentBezierDotPos, thickness, Rgba8::PURPLE );
		previousBezierDotPos = currentBezierDotPos;
	}
}


//----------------------------------------------------------------------------------------------------------------------
void Quadruped::DebugRenderRaycasts( std::vector<Vertex_PCU>& verts )
{
	if ( m_rayResult_Blocks.m_didImpact )
	{	 
		// Ray
		AddVertsForArrow3D ( verts, m_rayResult_Blocks.m_rayStartPosition, m_rayResult_Blocks.m_impactPos, 0.5f, Rgba8::RED );
		// Ray impact normal
		Vec3 normalEndPos = m_rayResult_Blocks.m_impactPos + ( m_rayResult_Blocks.m_impactNormal * 2.0f );
		AddVertsForArrow3D ( verts, m_rayResult_Blocks.m_impactPos, normalEndPos, 0.5f, Rgba8::LIGHTBLUE );
		// Ray impactPos
		AddVertsForSphere3D( verts, m_rayResult_Blocks.m_impactPos, 2.0f, 4.0f, 4.0f, Rgba8::BLUE );
	}
	else
	{
		Vec3 endPos = m_rayResult_Blocks.m_rayStartPosition + ( m_rayResult_Blocks.m_rayFwdNormal * m_rayResult_Blocks.m_rayMaxLength );
		AddVertsForArrow3D( verts, m_rayResult_Blocks.m_rayStartPosition, endPos, 0.5f, Rgba8::DARKER_RED );
	}
	
	if ( m_rayResult_Tri.m_didImpact )
	{
		// Ray
		AddVertsForArrow3D ( verts, m_rayResult_Tri.m_rayStartPosition, m_rayResult_Tri.m_impactPos, 0.5f, Rgba8::GREEN );
		// Ray impact normal
		Vec3 normalEndPos = m_rayResult_Tri.m_impactPos + ( m_rayResult_Tri.m_impactNormal * 2.0f );
		AddVertsForArrow3D( verts, m_rayResult_Tri.m_impactPos, normalEndPos, 0.5f, Rgba8::LIGHTBLUE );
		// Ray impactPos
		AddVertsForSphere3D( verts, m_rayResult_Tri.m_impactPos, 2.0f, 4.0f, 4.0f, Rgba8::BLUE );
	}
	else
	{
		Vec3 endPos = m_rayResult_Tri.m_rayStartPosition + ( m_rayResult_Tri.m_rayFwdNormal * m_rayResult_Tri.m_rayMaxLength );
		AddVertsForArrow3D( verts, m_rayResult_Tri.m_rayStartPosition, endPos, 0.5f, Rgba8::DARKER_GREEN );
	}
}
