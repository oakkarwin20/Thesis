#include "Engine/SkeletalSystem/IK_Joint3D.hpp"
#include "Engine/SkeletalSystem/IK_Chain3D.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"


//----------------------------------------------------------------------------------------------------------------------
IK_Joint3D::IK_Joint3D( int index, Vec3 startPos, float length, IK_Chain3D* IK_Chain, JointConstraintType jointConstraintType, EulerAngles orientation, FloatRange yawConstraints, FloatRange pitchConstraints, FloatRange rollConstraints, IK_Joint3D* parent )
	: m_jointIndex( index )
	, m_jointPos_LS( startPos )
	, m_distToChild( length )
	, m_ikChain( IK_Chain )
	, m_jointConstraintType( jointConstraintType )
	, m_eulerAngles_LS( orientation )
	, m_yawConstraints_LS( yawConstraints )
	, m_pitchConstraints_LS( pitchConstraints )
	, m_rollConstraints_LS( rollConstraints )
	, m_parent( parent )
{
}


//----------------------------------------------------------------------------------------------------------------------
IK_Joint3D::~IK_Joint3D()
{
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Joint3D::Update()
{
//	m_orientation = EulerAngles::GetAsEuler_XFwd_YLeft_ZUp( m_fwdDir, m_fwdDir.GetRotatedAboutZDegrees( 90.0f ) );
}

//----------------------------------------------------------------------------------------------------------------------
void IK_Joint3D::RenderIJK( std::vector<Vertex_PCU>& verts, float length ) const
{
	Vec3 fwdPos		= m_jointPos_LS + ( m_fwdDir			* length );
	Vec3 leftPos	= m_jointPos_LS + ( m_leftDir			* length );
	Vec3 upPos		= m_jointPos_LS + ( m_upDir			* length );
	Vec3 axisPos	= m_jointPos_LS + ( m_axisOfRotation	* length * 0.5f );
	AddVertsForArrow3D( verts, m_jointPos_LS,  fwdPos, 0.5f, Rgba8::RED	  );		// Fwd
	AddVertsForArrow3D( verts, m_jointPos_LS, leftPos, 0.5f, Rgba8::GREEN	  );		// Left
	AddVertsForArrow3D( verts, m_jointPos_LS,	 upPos, 0.5f, Rgba8::BLUE	  );		// Up
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Joint3D::UpdateFwdFromEndStart()
{
	m_fwdDir = ( m_endPos - m_jointPos_LS ).GetNormalized();
}



//----------------------------------------------------------------------------------------------------------------------
Vec3 IK_Joint3D::GetLimbEnd()
{
	//----------------------------------------------------------------------------------------------------------------------
	// Direction approach
	//----------------------------------------------------------------------------------------------------------------------
	Vec3 limbEndPos		= m_jointPos_LS + ( m_fwdDir * m_distToChild );
	return limbEndPos;
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Joint3D::SetStartEndPosRelativeToTarget( Vec3 targetPos )
{
	//----------------------------------------------------------------------------------------------------------------------
	// Direction approach
	//----------------------------------------------------------------------------------------------------------------------
	m_endPos	= targetPos;
	m_fwdDir	= ( m_endPos - m_jointPos_LS ).GetNormalized();
	m_jointPos_LS	= m_endPos - ( m_fwdDir * m_distToChild );
}



//----------------------------------------------------------------------------------------------------------------------
// The "target" refers to the child limb's pos and dir(s) when this function is called recursively
//----------------------------------------------------------------------------------------------------------------------
void IK_Joint3D::DragLimb3D( Target target )
{
	//----------------------------------------------------------------------------------------------------------------------
	// Move segmentEndPos to targetPos (segmentStartPos is NOT stuck to m_root or parent)
	//----------------------------------------------------------------------------------------------------------------------
	if ( m_jointConstraintType == JOINT_CONSTRAINT_TYPE_DISTANCE )
	{
		// Set direction and orientation towards targetPos
		SetStartEndPosRelativeToTarget( target.m_currentPos );					// Set startPosXYZ "m_length" away from its targetPosz
	}
	else if ( m_jointConstraintType == JOINT_CONSTRAINT_TYPE_BALL_AND_SOCKET )
	{		 
		// Logic for final limb
		if ( m_child == nullptr )
		{
			//----------------------------------------------------------------------------------------------------------------------
			// 1. Solve for positions and basis vectors 
			//----------------------------------------------------------------------------------------------------------------------
			SetStartEndPosRelativeToTarget( target.m_currentPos );	
			// 1b. UpdateBasis Vectors
			m_leftDir			= target.m_leftDir;
			m_upDir				= CrossProduct3D( m_fwdDir, m_leftDir );
			m_upDir.Normalize();
			m_axisOfRotation	= target.m_leftDir;

			//----------------------------------------------------------------------------------------------------------------------
			// Clamp roll (left vector)
			//----------------------------------------------------------------------------------------------------------------------
			// project currentLeft onto rootLeft
			// compute delta degrees
			// clamp if greater than max degrees
			float angleLeftToParentLeft	= GetAngleDegreesBetweenVectors3D( m_leftDir, target.m_leftDir );
			float maxAngle				= m_rollConstraints_LS.m_max;
			if ( angleLeftToParentLeft > maxAngle )
			{
				float deltaAngle		= angleLeftToParentLeft - maxAngle;
				Vec3 arbitraryAxis		= CrossProduct3D( m_leftDir, target.m_leftDir ).GetNormalized();
				m_leftDir				= RotateVectorAboutArbitraryAxis( m_leftDir, arbitraryAxis, deltaAngle );
				m_leftDir.Normalize();
			}

			m_fwdDir			= CrossProduct3D( m_leftDir, target.m_upDir );
			m_fwdDir.Normalize();
			m_upDir				= CrossProduct3D( m_fwdDir, m_leftDir );
			m_upDir.Normalize();
			m_axisOfRotation	= target.m_leftDir;
			// Set start and end pos based on new dir vectors
			m_endPos	= target.m_currentPos;
			m_jointPos_LS  = m_endPos - ( m_fwdDir * m_distToChild );
		}
		// Logic for limb that have a child
		else 
		{
			if ( m_child->m_jointConstraintType == JOINT_CONSTRAINT_TYPE_HINGE_KNEE )
			{
				//----------------------------------------------------------------------------------------------------------------------
				// 1. Solve for positions and basis vectors relative to child
				//----------------------------------------------------------------------------------------------------------------------
				// 1a. Solve positions 
				m_endPos			= m_child->m_jointPos_LS;
				m_fwdDir			= ( m_endPos - m_jointPos_LS ).GetNormalized();
				m_jointPos_LS			= m_endPos - ( m_fwdDir * m_distToChild );
				// 1b. UpdateBasis Vectors
				m_leftDir			= m_child->m_leftDir;
				m_upDir				= CrossProduct3D( m_fwdDir, m_leftDir );
				m_upDir.Normalize();
				m_axisOfRotation	= target.m_leftDir;

				//----------------------------------------------------------------------------------------------------------------------
				// 2. Clamp direction relative to child
				//----------------------------------------------------------------------------------------------------------------------
				m_fwdDir				 = ProjectVectorOntoPlaneNormalized( m_fwdDir, m_child->m_leftDir );
				float angleFwdToChildFwd = GetSignedAngleDegreesBetweenVectors( m_child->m_fwdDir, m_fwdDir, m_child->m_leftDir );
				float maxAngle			 = m_pitchConstraints_LS.m_max;
				float minAngle			 = m_pitchConstraints_LS.m_min;
				if ( angleFwdToChildFwd > maxAngle )
				{
					m_fwdDir = RotateVectorAboutArbitraryAxis( m_child->m_fwdDir, m_child->m_leftDir, maxAngle );
				}
				else if ( angleFwdToChildFwd < minAngle )
				{
					m_fwdDir = RotateVectorAboutArbitraryAxis( m_child->m_fwdDir, m_child->m_leftDir, minAngle );
				}
				// Update positions based on new dir vectors
				m_endPos	 = m_child->m_jointPos_LS;
				m_jointPos_LS	 = m_endPos - ( m_fwdDir * m_distToChild );

				// Update basis vectors (left & up)
				m_leftDir		 = m_child->m_leftDir;
				m_upDir			 = CrossProduct3D( m_fwdDir, m_leftDir );
				m_upDir.Normalize();
				m_axisOfRotation = m_child->m_leftDir;
			}
			else if ( m_child->m_jointConstraintType == JOINT_CONSTRAINT_TYPE_BALL_AND_SOCKET )
			{
				//----------------------------------------------------------------------------------------------------------------------
				// 1. Solve for positions and basis vectors relative to child
				//----------------------------------------------------------------------------------------------------------------------
				// 1a. Solve positions 
				m_endPos			= m_child->m_jointPos_LS;
				m_fwdDir			= ( m_endPos - m_jointPos_LS ).GetNormalized();
				m_jointPos_LS			= m_endPos - ( m_fwdDir * m_distToChild );
				// 1b. UpdateBasis Vectors
				m_leftDir			= target.m_leftDir;
				m_upDir				= CrossProduct3D( m_fwdDir, m_leftDir );
				m_upDir.Normalize();
				m_leftDir			= CrossProduct3D( m_upDir, m_fwdDir );
				m_leftDir.Normalize();
				m_axisOfRotation	= target.m_leftDir;

				//----------------------------------------------------------------------------------------------------------------------
				// 2. Clamp direction relative to child
				//----------------------------------------------------------------------------------------------------------------------
				float angleFwdToChildFwd = GetSignedAngleDegreesBetweenVectors( m_child->m_fwdDir, m_fwdDir, m_child->m_leftDir );
				float maxAngle			 = m_pitchConstraints_LS.m_max;
				float minAngle			 = m_pitchConstraints_LS.m_min;
				if ( angleFwdToChildFwd > maxAngle )
				{
					m_fwdDir = RotateVectorAboutArbitraryAxis( m_child->m_fwdDir, m_child->m_leftDir, maxAngle );
				}
				else if ( angleFwdToChildFwd < minAngle )
				{
					m_fwdDir = RotateVectorAboutArbitraryAxis( m_child->m_fwdDir, m_child->m_leftDir, minAngle );
				}
				// Update positions based on new dir vectors
				m_endPos	 = m_child->m_jointPos_LS;
				m_jointPos_LS	 = m_endPos - ( m_fwdDir * m_distToChild );

				// Update basis vectors (left & up)
				m_axisOfRotation = m_child->m_leftDir;
			}
		}
	}
	else if ( m_jointConstraintType == JOINT_CONSTRAINT_TYPE_HINGE )
	{
		//----------------------------------------------------------------------------------------------------------------------
		// 1. If finalLimb, solve normally
		//		1a. Set basis vectors as endEffector 
		// 2. Solve currentLimb normally.
		// 3. Compute angles between currentLimb and childLimb to check if angle is within valid range
		//		3a. True:  
		//					3a1. Angle is valid, do nothing
		//		3b. False:	
		//					3b1. Angle is not within valid range, calculate deltaDegrees
		//					3b2. Determine CCW or CW
		// 5. Rotate by deltaDegrees (CCW or CW) 
		//----------------------------------------------------------------------------------------------------------------------

	
		// Logic for limbSegment at the end of the chain
		if ( m_child == nullptr )
		{
			//----------------------------------------------------------------------------------------------------------------------
			// Default FABRIK drag algorithm
			//----------------------------------------------------------------------------------------------------------------------
			m_endPos			= target.m_currentPos;
			m_fwdDir			= target.m_fwdDir;
			m_leftDir			= target.m_leftDir;
			m_upDir				= target.m_upDir;
			m_jointPos_LS			= target.m_currentPos - ( m_fwdDir * m_distToChild );
			m_axisOfRotation	= target.m_upDir;
		}
		else     // Logic for all other limbSegment (moving up the chain)
		{
			//----------------------------------------------------------------------------------------------------------------------
			// Compute angle between childStartEnd and childStartCurrentStart
			// Check if angle is within valid range
			// Else, compute deltaDegrees for clamping
			//----------------------------------------------------------------------------------------------------------------------
			SetStartEndPosRelativeToTarget( target.m_currentPos );										// Set startPosXYZ "m_length" away from its targetPos
			Vec3  childStartToChildEnd			= m_child->m_endPos - m_child->m_jointPos_LS;
			Vec3  childStartToCurrentStart		= m_jointPos_LS - m_child->m_jointPos_LS;
			float angle							= GetAngleDegreesBetweenVectors3D( childStartToChildEnd, childStartToCurrentStart );
			bool  isCCW							= true;
			float deltaDegrees					= 0.0f;
			float maxDegrees_childToCurrent		= 180.0f;
			float minDegrees_childToCurrent		= 180.0f - m_child->m_yawConstraints_LS.m_max;					// (180 - 135 = 45), min = 45
			// Determine CCW or CW
			Vec3  dirCurrentEndStart			= ( m_jointPos_LS - m_endPos ).GetNormalized();
			float dotResult						= DotProduct3D( dirCurrentEndStart, m_child->m_leftDir );
			if ( dotResult > 0.0f )
			{
				// Same side as left dir
				isCCW = true;
			}
			else
			{
				// Opposite side of left dir (right side)
				isCCW = false;
			}
			
			if ( isCCW )
			{
				if ( angle < minDegrees_childToCurrent )
				{
					// Calculate deltaDegrees
					deltaDegrees = minDegrees_childToCurrent - angle;
				}
			}
			else if ( !isCCW )
			{
				if ( angle < maxDegrees_childToCurrent )
				{
					// Calculate deltaDegrees (max - angle to get negative degrees to rotate)
					deltaDegrees = angle - maxDegrees_childToCurrent;
				}
			}

			// Rotate to clamp currentLimb
			dirCurrentEndStart		= ( m_jointPos_LS - m_endPos ).GetNormalized();
			dirCurrentEndStart		= RotateVectorAboutArbitraryAxis( dirCurrentEndStart, m_child->m_upDir, deltaDegrees );
			dirCurrentEndStart		= dirCurrentEndStart.GetNormalized();
			m_jointPos_LS				= m_endPos + ( dirCurrentEndStart * m_distToChild );
			m_fwdDir				= -dirCurrentEndStart.GetNormalized();
			m_leftDir				= RotateVectorAboutArbitraryAxis( m_fwdDir, m_child->m_upDir, 90.0f );
			m_leftDir				= m_leftDir.GetNormalized();
			m_upDir					= CrossProduct3D( m_fwdDir, m_leftDir );
			m_upDir					= m_upDir.GetNormalized();
			m_axisOfRotation		= m_child->m_upDir.GetNormalized();
		}

		//----------------------------------------------------------------------------------------------------------------------
		// Recursively call this function till no parents exist
		//----------------------------------------------------------------------------------------------------------------------
		if ( m_parent != nullptr )
		{
			Target newEndEffector = Target( m_jointPos_LS, m_jointPos_LS, m_fwdDir, m_leftDir, m_upDir );
			m_parent->DragLimb3D( newEndEffector );
		}
	}
	else if ( m_jointConstraintType == JOINT_CONSTRAINT_TYPE_HINGE_KNEE )
	{
		if ( m_child == nullptr )
		{
			m_endPos			= target.m_currentPos;
			m_leftDir			= target.m_leftDir;
			m_fwdDir			= ( m_endPos - m_jointPos_LS ).GetNormalized();
			m_upDir				= CrossProduct3D( m_fwdDir, m_leftDir );
			m_upDir.Normalize();
			m_leftDir			= CrossProduct3D( m_upDir, m_fwdDir );
			m_leftDir.Normalize();
			m_jointPos_LS			= m_endPos - ( m_fwdDir * m_distToChild );
			m_axisOfRotation	= m_leftDir;
			m_targetPos			= target.m_currentPos;
//			DebuggerPrintf( "------------------------------\n" );
//			DebuggerPrintf( "NO inherit\n" );

			//----------------------------------------------------------------------------------------------------------------------
			// Default FABRIK drag algorithm
			//----------------------------------------------------------------------------------------------------------------------
//			if ( m_skeletalSystem->m_shouldParentBendMore )
//			{
//				m_endPos			= endEffector.m_currentPos;
//				m_fwdDir			= ( m_endPos - m_startPos ).GetNormalized();
//				m_leftDir			= endEffector.m_leftDir;
//				m_upDir				= CrossProduct3D( m_fwdDir, m_leftDir );
//				m_upDir.Normalize();
//				m_startPos			= m_endPos - ( m_fwdDir * m_length );
//				m_axisOfRotation	= endEffector.m_leftDir;
//				m_targetPos			= endEffector.m_currentPos;
//				DebuggerPrintf( "------------------------------\n" );
//				DebuggerPrintf( "NO inherit\n" );
//			}
//			else
//			{
//				m_endPos			= endEffector.m_currentPos;
//				m_fwdDir			= endEffector.m_fwdDir;
//				m_leftDir			= endEffector.m_leftDir;
//				m_upDir				= endEffector.m_upDir;
//				m_startPos			= endEffector.m_currentPos - ( m_fwdDir * m_length );
//				m_axisOfRotation	= endEffector.m_leftDir;
//				m_targetPos			= endEffector.m_currentPos;
//				DebuggerPrintf( "------------------------------\n" );
//				DebuggerPrintf( "inherit\n" );
//			}
		}
		else if ( m_child->m_child == nullptr )
		{
/*
			// Set direction and orientation towards targetPos
			m_poleVector = endEffector.m_currentPos + ( endEffector.m_fwdDir * m_length * 1.5f ) + ( endEffector.m_upDir * m_length );
			m_poleVector = m_parent->m_startPos + ( m_parent->m_fwdDir * m_length ) + ( m_parent->m_upDir * m_length );
			SetStartEndPosRelativeToTarget( m_poleVector );										// Set startPosXYZ "m_length" away from its targetPos
*/

			//----------------------------------------------------------------------------------------------------------------------
			// 1. Solve FABRIK as normal
			//----------------------------------------------------------------------------------------------------------------------
			SetStartEndPosRelativeToTarget( target.m_currentPos );							// Set startPosXYZ "m_length" away from its targetPos
			m_axisOfRotation = target.m_leftDir;

			//----------------------------------------------------------------------------------------------------------------------
			// 2. Project onto plane with rotation axis (current leftDir) as the plane's normal 
			//----------------------------------------------------------------------------------------------------------------------
			// Project onto plane normal
			Vec3 currentFwdProjectedOntoLeftPlane = ProjectVectorOntoPlaneNormalized( m_fwdDir, m_axisOfRotation );
			//----------------------------------------------------------------------------------------------------------------------
			// 3. Get angle between vectors (currentFwd and childFwd)
			//----------------------------------------------------------------------------------------------------------------------
			Vec3 refVector		= -target.m_upDir;
			float signedAngle	= GetSignedAngleDegreesBetweenVectors( refVector, currentFwdProjectedOntoLeftPlane, m_axisOfRotation );
			// Check if angle is within bounds
			if ( signedAngle > m_pitchConstraints_LS.m_max )
			{
				m_fwdDir = RotateVectorAboutArbitraryAxis( refVector, m_axisOfRotation, m_pitchConstraints_LS.m_max );
			}
			else if ( signedAngle < -m_pitchConstraints_LS.m_min )
			{
				m_fwdDir = RotateVectorAboutArbitraryAxis( refVector, m_axisOfRotation, -m_pitchConstraints_LS.m_min );
			}
			//----------------------------------------------------------------------------------------------------------------------
			// 3a. Tell the "knee" to bend more"
			//----------------------------------------------------------------------------------------------------------------------
			if ( m_ikChain->m_shouldBendMore )
			{
				Vec3 dirJointToTarget	= ( target.m_currentPos - m_ikChain->m_position_WS ).GetNormalized();
				Vec3 crossResult		= CrossProduct3D( m_fwdDir, dirJointToTarget );
//				if ( crossResult.z > 0.0f )
				if ( crossResult.y > 0.0f )
				{
					// If positive, the targetPos is on the "left" of the currentJoint's fwd
					// Then, we should rotate "clockwise" (away) around the child's left 
//					m_fwdDir = RotateVectorAboutArbitraryAxis( m_fwdDir, m_child->m_leftDir, +m_IKChain->m_degreesToBendKnee_current );
//					DebuggerPrintf( "Knee +\n" );
				}
				else
				{
					// If negative, the targetPos is on the "right" of the currentJoint's fwd
					// Then, we should rotate "clockwise" (away) around the child's left 
//					m_fwdDir = RotateVectorAboutArbitraryAxis( m_fwdDir, m_child->m_leftDir, +m_IKChain->m_degreesToBendKnee_current );
//					DebuggerPrintf( "Knee -\n" );
				}
				//----------------------------------------------------------------------------------------------------------------------
				// 3b. Clamp angles AGAIN
				//----------------------------------------------------------------------------------------------------------------------
				refVector	= -target.m_upDir;
				signedAngle = GetSignedAngleDegreesBetweenVectors( refVector, m_fwdDir, m_axisOfRotation );
				// Check if angle is within bounds
//				if ( signedAngle > m_pitchConstraints.m_max )
//				{
//					m_fwdDir = RotateVectorAboutArbitraryAxis( refVector, m_axisOfRotation, m_pitchConstraints.m_max );
//				}
//				else if ( signedAngle < -m_pitchConstraints.m_min )
//				{
//					m_fwdDir = RotateVectorAboutArbitraryAxis( refVector, m_axisOfRotation, -m_pitchConstraints.m_min );
//				}
			}
			//----------------------------------------------------------------------------------------------------------------------
			// 4. UpdateBasis Vectors
			//----------------------------------------------------------------------------------------------------------------------
			m_fwdDir.Normalize();
			m_jointPos_LS			= m_endPos - ( m_fwdDir * m_distToChild );
			m_leftDir			= target.m_leftDir;
			m_upDir				= CrossProduct3D( m_fwdDir, m_leftDir );
			m_upDir.Normalize();
		}
		else
		{
			//----------------------------------------------------------------------------------------------------------------------
			// 1. Solve FABRIK as normal
			//----------------------------------------------------------------------------------------------------------------------
			SetStartEndPosRelativeToTarget( target.m_currentPos );							// Set startPosXYZ "m_length" away from its targetPos
			m_axisOfRotation = target.m_leftDir;

			//----------------------------------------------------------------------------------------------------------------------
			// 2. Project onto plane with rotation axis (current leftDir) as the plane's normal 
			//----------------------------------------------------------------------------------------------------------------------
			// Project onto plane normal
			Vec3 currentFwdProjectedOntoLeftPlane = ProjectVectorOntoPlaneNormalized( m_fwdDir, m_axisOfRotation );
			//----------------------------------------------------------------------------------------------------------------------
			// 3. Get angle between vectors (currentFwd and childFwd)
			//----------------------------------------------------------------------------------------------------------------------
			Vec3 refVector		= target.m_fwdDir;
			float signedAngle	= GetSignedAngleDegreesBetweenVectors( refVector, currentFwdProjectedOntoLeftPlane, m_axisOfRotation );
			// Check if angle is within bounds
			if ( signedAngle > m_pitchConstraints_LS.m_max )
			{
				m_fwdDir = RotateVectorAboutArbitraryAxis( refVector, m_axisOfRotation, m_pitchConstraints_LS.m_max );
			}
			else if ( signedAngle < -m_pitchConstraints_LS.m_min )
			{
				m_fwdDir = RotateVectorAboutArbitraryAxis( refVector, m_axisOfRotation, -m_pitchConstraints_LS.m_min );
			}
			//----------------------------------------------------------------------------------------------------------------------
			// 3a. Tell the "hip" to bend more"
			//----------------------------------------------------------------------------------------------------------------------
			if ( m_ikChain->m_shouldBendMore )
			{
				Vec3 dirJointToTarget	= ( target.m_currentPos - m_ikChain->m_position_WS ).GetNormalized();
				Vec3 crossResult		= CrossProduct3D( m_fwdDir, dirJointToTarget );
//				if ( crossResult.z > 0.0f )
				if ( crossResult.y > 0.0f )
				{
					// If positive, the targetPos is on the "left" of the currentJoint's fwd
					// Then, we should rotate "counter-clockwise" (away) around the child's left 
//					m_fwdDir = RotateVectorAboutArbitraryAxis( m_fwdDir, m_child->m_leftDir, -m_IKChain->m_bendMoreDegrees_current );
				}
				else		
				{
					// If negative, the targetPos is on the "right" of the currentJoint's fwd
					// Then, we should rotate "clockwise" (away) around the child's left 
//					m_fwdDir = RotateVectorAboutArbitraryAxis( m_fwdDir, m_child->m_leftDir, -m_IKChain->m_bendMoreDegrees_current );
				}
				if ( m_parent == nullptr )
				{
//					m_skeletalSystem->m_shouldParentBendMore = false;
				}
				//----------------------------------------------------------------------------------------------------------------------
				// 3b. Clamp angles AGAIN
				//----------------------------------------------------------------------------------------------------------------------
				refVector	= target.m_fwdDir;
				signedAngle = GetSignedAngleDegreesBetweenVectors( refVector, m_fwdDir, m_axisOfRotation );
				// Check if angle is within bounds
				if ( signedAngle > m_pitchConstraints_LS.m_max )
				{
					m_fwdDir = RotateVectorAboutArbitraryAxis( refVector, m_axisOfRotation, m_pitchConstraints_LS.m_max );
				}
				else if ( signedAngle < -m_pitchConstraints_LS.m_min )
				{
					m_fwdDir = RotateVectorAboutArbitraryAxis( refVector, m_axisOfRotation, -m_pitchConstraints_LS.m_min );
				}
			}
			//----------------------------------------------------------------------------------------------------------------------
			// 4. UpdateBasis Vectors
			//----------------------------------------------------------------------------------------------------------------------
			m_fwdDir.Normalize();
			m_jointPos_LS			= m_endPos - ( m_fwdDir * m_distToChild );
			m_leftDir			= target.m_leftDir;
			m_upDir				= CrossProduct3D( m_fwdDir, m_leftDir );
			m_upDir.Normalize();

/*
			//----------------------------------------------------------------------------------------------------------------------
			// 2. Project onto plane with rotation axis (current leftDir) as the plane's normal 
			//----------------------------------------------------------------------------------------------------------------------
			// Project onto plane normal
			float lengthOfShadowOnNormal			= DotProduct3D( m_fwdDir, m_axisOfRotation );
			// Project onto plane
			Vec3 amountToSubtractToFlattenVector	= m_axisOfRotation * lengthOfShadowOnNormal;
			Vec3 currentFwdProjectedOntoLeftPlane	= m_fwdDir - amountToSubtractToFlattenVector;
			currentFwdProjectedOntoLeftPlane.Normalize();

			// Get angle between vectors (currentFwd and childFwd)
			Vec3 refVector		= endEffector.m_fwdDir;
			float signedAngle	= GetSignedAngleDegreesBetweenVectors( refVector, currentFwdProjectedOntoLeftPlane, m_axisOfRotation );
			// Check if angle is within bounds
			if ( signedAngle > m_yawConstraints.m_max )
			{
				m_fwdDir = RotateVectorAboutArbitraryAxis( refVector, m_axisOfRotation, m_yawConstraints.m_max );
			}
			else if ( signedAngle < -m_yawConstraints.m_min )
			{
				m_fwdDir = RotateVectorAboutArbitraryAxis( refVector, m_axisOfRotation, -m_yawConstraints.m_min );
			}
*/
		}
		//----------------------------------------------------------------------------------------------------------------------
		// 1. If finalLimb, solve normally
		//		1a. Set basis vectors as endEffector 
		// 2. Solve currentLimb normally.
		// 3. Compute angles between currentLimb and childLimb to check if angle is within valid range
		//		3a. True:  
		//					3a1. Angle is valid, do nothing
		//		3b. False:	
		//					3b1. Angle is not within valid range, calculate deltaDegrees
		//					3b2. Determine CCW or CW
		// 5. Rotate by deltaDegrees (CCW or CW) 
		//----------------------------------------------------------------------------------------------------------------------
	
/*
		// Logic for limbSegment at the end of the chain
		if ( m_child == nullptr )
		{
			//----------------------------------------------------------------------------------------------------------------------
			// Default FABRIK drag algorithm
			//----------------------------------------------------------------------------------------------------------------------
			m_endPos			= endEffector.m_currentPos;
			m_fwdDir			= endEffector.m_fwdDir;
			m_leftDir			= endEffector.m_leftDir;
			m_upDir				= endEffector.m_upDir;
			m_startPos			= endEffector.m_currentPos - ( m_fwdDir * m_length );
			m_axisOfRotation	= -endEffector.m_leftDir;
		}
		else if ( m_child->m_child == nullptr )
		{
			//----------------------------------------------------------------------------------------------------------------------
			// Compute angle between childStartEnd and childStartCurrentStart
			// Check if angle is within valid range
			// Else, compute deltaDegrees for clamping
			//----------------------------------------------------------------------------------------------------------------------
			SetStartEndPosRelativeToTarget( endEffector.m_currentPos );													// Set startPosXYZ "m_length" away from its targetPos		
			float angle							= GetAngleDegreesBetweenVectors3D( m_child->m_fwdDir, m_fwdDir );
			bool  isCCW							= true;
			float deltaDegrees					= 0.0f;
			float maxDegrees_childToCurrent		= m_child->m_yawConstraints.m_max;
			float minDegrees_childToCurrent		= m_child->m_yawConstraints.m_min;					// (180 - 135 = 45), min = 45
			if ( m_skeletalSystem->m_shouldParentBendMore )
			{
				maxDegrees_childToCurrent		+= 2.0f;
				minDegrees_childToCurrent		-= 2.0f;
//				m_skeletalSystem->m_shouldParentBendMore = false;
			}
			// Determine CCW or CW
			float dotResult						= DotProduct3D( m_fwdDir, m_child->m_fwdDir );
			if ( dotResult > 0.0f ) 
			{
				// Same side as left dir 
				isCCW = true;
			}
			else
			{
				// Opposite side of left dir (right side)
				isCCW = false;
			}
			
			if ( isCCW )
			{
				if ( angle > maxDegrees_childToCurrent )
				{
					// Calculate deltaDegrees (max - angle to get negative degrees to rotate)
					deltaDegrees = maxDegrees_childToCurrent - angle;
				}
			}
			else if ( !isCCW )
			{
				if ( angle < minDegrees_childToCurrent )
				{
					// Calculate deltaDegrees
					deltaDegrees = minDegrees_childToCurrent - angle;
				}
			}

			// Rotate to clamp currentLimb
			m_axisOfRotation		= -m_child->m_leftDir.GetNormalized();
			m_fwdDir				= RotateVectorAboutArbitraryAxis( m_fwdDir, m_axisOfRotation, deltaDegrees );
			m_fwdDir				= m_fwdDir.GetNormalized();
			m_startPos				= m_endPos - ( m_fwdDir * m_length );
			m_upDir					= RotateVectorAboutArbitraryAxis( m_fwdDir, m_axisOfRotation, 90.0f );
			m_upDir					= m_upDir.GetNormalized();
			m_leftDir				= CrossProduct3D( m_upDir, m_fwdDir );
			m_leftDir				= m_leftDir.GetNormalized();
		}
		else     // Logic for all other limbSegment (moving up the chain)
		{
			//----------------------------------------------------------------------------------------------------------------------
			// Compute angle between childStartEnd and childStartCurrentStart
			// Check if angle is within valid range
			// Else, compute deltaDegrees for clamping
			//----------------------------------------------------------------------------------------------------------------------
			SetStartEndPosRelativeToTarget( endEffector.m_currentPos );													// Set startPosXYZ "m_length" away from its targetPos		
			float angle							= GetAngleDegreesBetweenVectors3D( m_child->m_fwdDir, m_fwdDir );
			bool  isCCW							= true;
			float deltaDegrees					= 0.0f;
			float maxDegrees_childToCurrent		= m_child->m_yawConstraints.m_max;
			float minDegrees_childToCurrent		= m_child->m_yawConstraints.m_min;					// (180 - 135 = 45), min = 45
			if ( m_skeletalSystem->m_shouldParentBendMore )
			{
				maxDegrees_childToCurrent		*= 1.05f;
				minDegrees_childToCurrent		*= 1.05f;
//				m_skeletalSystem->m_shouldParentBendMore = false;
			}
			// Determine CCW or CW
			float dotResult						= DotProduct3D( m_fwdDir, m_child->m_fwdDir );
			if ( dotResult > 0.0f ) 
			{
				// Same side as left dir 
				isCCW = true;
			}
			else
			{
				// Opposite side of left dir (right side)
				isCCW = false;
			}
			
			if ( isCCW )
			{
				if ( angle > maxDegrees_childToCurrent )
				{
					// Calculate deltaDegrees (max - angle to get negative degrees to rotate)
					deltaDegrees = maxDegrees_childToCurrent - angle;
				}
			}
			else if ( !isCCW )
			{
				if ( angle < minDegrees_childToCurrent )
				{
					// Calculate deltaDegrees
					deltaDegrees = minDegrees_childToCurrent - angle;
				}
			}

			// Rotate to clamp currentLimb
			m_axisOfRotation		= -m_child->m_leftDir.GetNormalized();
			m_fwdDir				= RotateVectorAboutArbitraryAxis( m_fwdDir, m_axisOfRotation, deltaDegrees );
			m_fwdDir				= m_fwdDir.GetNormalized();
			m_startPos				= m_endPos - ( m_fwdDir * m_length );
			m_upDir					= RotateVectorAboutArbitraryAxis( m_fwdDir, m_axisOfRotation, 90.0f );
			m_upDir					= m_upDir.GetNormalized();
			m_leftDir				= CrossProduct3D( m_upDir, m_fwdDir );
			m_leftDir				= m_leftDir.GetNormalized();
		}

//		EulerAngles currentEulerAngles = EulerAngles::GetAsEulerAnglesFromFwdAndLeftBasis_XFwd_YLeft_ZUp( m_fwdDir, m_leftDir );
//		currentEulerAngles.m_yawDegrees 

		//----------------------------------------------------------------------------------------------------------------------
		// Recursively call this function till no parents exist
		//----------------------------------------------------------------------------------------------------------------------
		if ( m_parent != nullptr )
		{
			EndEffector newEndEffector = EndEffector( m_startPos, m_startPos, m_fwdDir, m_leftDir, m_upDir );
			m_parent->DragLimb3D( newEndEffector );
		}
*/
	}
	else if ( m_jointConstraintType == JOINT_CONSTRAINT_TYPE_EULER )
	{
		//----------------------------------------------------------------------------------------------------------------------
		// Context:
		// EE's unconstrained solution is very simple because it just inherit's the target's pos & IJK
		// But everyone else has to compute their JK using crossProduct based on their I and sky
		//----------------------------------------------------------------------------------------------------------------------

		//----------------------------------------------------------------------------------------------------------------------
		// 1. Drag limbs to target
		//----------------------------------------------------------------------------------------------------------------------
		if ( m_ikChain->m_isSingleStep_Debug )
		{
			//  Debug mode
			if ( m_solveSingleStep_Forwards )
			{
				// Update positions and IJK
				if ( m_child != nullptr )
				{
					JointsBeforeEE_Forwards( target );
					ToggleSingleStep_Forwards();
					return;
				}
				// Logic for the endEffector
				else
				{
					finalJoint_Forwards( target );
					ToggleSingleStep_Forwards();
					return;
				}
			}
		}
		else    // Normal mode
		{
			// Update positions and IJK
			if ( m_child != nullptr )
			{
				JointsBeforeEE_Forwards( target );
			}
			// Logic for the endEffector
			else
			{
				finalJoint_Forwards( target );
			}
		}
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Recursively call this function till no parents exist
	//----------------------------------------------------------------------------------------------------------------------
	if ( m_parent != nullptr )
	{
		Target newEndEffector = Target( m_jointPos_LS, m_jointPos_LS, m_fwdDir, m_leftDir, m_upDir );
		m_parent->DragLimb3D( newEndEffector );
	}
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Joint3D::SetConstraints_YPR( FloatRange yawConstraints, FloatRange pitchConstraints, FloatRange rollConstraints )
{
	m_yawConstraints_LS	= yawConstraints;
	m_pitchConstraints_LS	= pitchConstraints;
	m_rollConstraints_LS	= rollConstraints;
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Joint3D::InheritTargetIJK( Target target )
{
	m_fwdDir	= target.m_fwdDir;
	m_leftDir	= target.m_leftDir;
	m_upDir		= target.m_upDir;
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Joint3D::InheritParentIJK()
{
	m_fwdDir	= m_parent->m_fwdDir;
	m_leftDir	= m_parent->m_leftDir;
	m_upDir		= m_parent->m_upDir;
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Joint3D::ComputeJ_Left_K_UpCrossProducts( Target target )
{
	// Compute JK
	Vec3  worldUp			= Vec3( 0.0f, 0.0f, 1.0f );
	Vec3  worldLeft			= Vec3( 0.0f, 1.0f, 0.0f );
	float dotResult_up		= DotProduct3D(   worldUp, m_fwdDir );
	float dotResult_left	= DotProduct3D( worldLeft, m_fwdDir );
	if ( dotResult_up >= 0.99f )
	{
		// Edge case 1. 
		// LimbFwd & worldUp face the same dir
		// Instead, use the worldLeft to compute our JK
		m_upDir			= CrossProduct3D( m_fwdDir, worldLeft );
		m_upDir.Normalize();
		m_leftDir		= CrossProduct3D( m_upDir, m_fwdDir );
		m_leftDir.Normalize();
	}
	else if ( dotResult_left >= 0.99f )
	{
		// Edge case 2.
		// LimbFwd & worldLeft face the same dir 
		// Instead, use the worldUp to compute our JK
		m_leftDir		= CrossProduct3D( worldUp, m_fwdDir );
		m_leftDir.Normalize();
		m_upDir			= CrossProduct3D( m_fwdDir, m_leftDir );
		m_upDir.Normalize();
	}
	else
	{
		// No edge cases
		m_upDir   = CrossProduct3D( m_fwdDir, target.m_leftDir );
		m_upDir.Normalize();
		m_leftDir = CrossProduct3D( m_upDir, m_fwdDir );
		m_leftDir.Normalize();
	}
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Joint3D::ToggleSingleStep_Forwards()
{
	m_solveSingleStep_Forwards = false;
	if ( m_parent != nullptr )
	{
		// Logic for child
		m_parent->m_solveSingleStep_Forwards = true;
	}
	else
	{
		// Logic for parent
		m_solveSingleStep_Backwards = true;
	}
	m_ikChain->m_breakFABRIK = true;
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Joint3D::ToggleSingleStep_Backwards()
{
	if ( m_child != nullptr )
	{
		m_solveSingleStep_Backwards		     = false;
		m_child->m_solveSingleStep_Backwards = true;
	}
	else
	{
		// Logic for end effector
		m_solveSingleStep_Backwards = false;
		m_solveSingleStep_Forwards  = true;
	}
	m_ikChain->m_breakFABRIK = true;
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Joint3D::ClampYPR()
{
	m_eulerAngles_LS.m_yawDegrees	= GetClamped( m_eulerAngles_LS.m_yawDegrees,	m_yawConstraints_LS.m_min,	m_yawConstraints_LS.m_max	);
	m_eulerAngles_LS.m_pitchDegrees	= GetClamped( m_eulerAngles_LS.m_pitchDegrees, m_pitchConstraints_LS.m_min,	m_pitchConstraints_LS.m_max );
	m_eulerAngles_LS.m_rollDegrees	= GetClamped( m_eulerAngles_LS.m_rollDegrees,	m_rollConstraints_LS.m_min,	m_rollConstraints_LS.m_max  );
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Joint3D::JointsBeforeEE_Forwards( Target target )
{
	// Logic for everyone else
	m_endPos	= m_child->m_jointPos_LS;
	m_fwdDir	= ( m_endPos - m_jointPos_LS ).GetNormalized();
	m_jointPos_LS  = m_endPos + ( -m_fwdDir * m_distToChild );
	ComputeJ_Left_K_UpCrossProducts( target );
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Joint3D::finalJoint_Forwards( Target target )
{
	m_endPos	= target.m_currentPos;
	m_fwdDir	= ( m_endPos - m_jointPos_LS ).GetNormalized();
	m_jointPos_LS  = m_endPos + ( -m_fwdDir * m_distToChild );
	ComputeJ_Left_K_UpCrossProducts( target );
}


//----------------------------------------------------------------------------------------------------------------------
Mat44 IK_Joint3D::GetMatrix_LocalToParent()
{
	Mat44 localToParentMatrix = m_eulerAngles_LS.GetAsMatrix_XFwd_YLeft_ZUp();
	localToParentMatrix.SetTranslation3D( m_jointPos_LS );
	return localToParentMatrix;
}


//----------------------------------------------------------------------------------------------------------------------
// Starting from the root's position, apply "offsets" for each child in "relative" IJKTs
// Root to child (currentJoint)
//----------------------------------------------------------------------------------------------------------------------
Mat44 IK_Joint3D::GetMatrix_LocalToModel( Mat44 localToModelMatrix /*= Mat44()*/ )
{
	for ( int i = 0; i <= m_jointIndex; i++ )
	{
		// Start with the parent's matrix and keep appending until
		// we've reaching "this" IK_Segment
		IK_Joint3D* jointIterIndex	= m_ikChain->m_jointList[i];
		Mat44 localToParentMatrix	= jointIterIndex->GetMatrix_LocalToParent();
		localToModelMatrix.Append( localToParentMatrix );
	}
	return localToModelMatrix;
}


//----------------------------------------------------------------------------------------------------------------------
Mat44 IK_Joint3D::GetMatrix_ModelToWorld()
{
	Mat44 modelToWorldMatrix = GetIkChainMatrix_ModelToWorld();
	modelToWorldMatrix		 = GetMatrix_LocalToModel( modelToWorldMatrix );
	return modelToWorldMatrix;
}


//----------------------------------------------------------------------------------------------------------------------
Mat44 IK_Joint3D::GetIkChainMatrix_ModelToWorld()
{
	Mat44 modelToWorldMatrix = m_ikChain->m_eulerAngles_WS.GetAsMatrix_XFwd_YLeft_ZUp();
	modelToWorldMatrix.SetTranslation3D( m_ikChain->m_position_WS );
	return modelToWorldMatrix;
}

//----------------------------------------------------------------------------------------------------------------------
Vec3 IK_Joint3D::GetTransformedPos_WorldToLocal( Vec3 const& pos_WS )
{
	Mat44 modelToWorldMatrix		= GetIkChainMatrix_ModelToWorld();
	Mat44 worldToModelMatrix		= modelToWorldMatrix.GetOrthoNormalInverse();
	Vec3  pos_MS					= worldToModelMatrix.TransformPosition3D( pos_WS );
	// 1b. target_MS to target_LS
	Mat44 localToModelMatrix;
	if ( m_parent )
	{
		localToModelMatrix = m_parent->GetMatrix_LocalToModel();
	}
	else
	{
		// If parent joint does not exist
		return pos_MS;
	}
	Mat44  modelToLocalMatrix		= localToModelMatrix.GetOrthoNormalInverse();
	Vec3   pos_LS					= modelToLocalMatrix.TransformPosition3D( pos_MS );
	return pos_LS;
}