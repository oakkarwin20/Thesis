#include "Engine/SkeletalSystem/CreatureBase.hpp"
#include "Engine/SkeletalSystem/IK_Joint3D.hpp"
#include "Engine/SkeletalSystem/IK_Chain3D.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/MathUtils.hpp"

//----------------------------------------------------------------------------------------------------------------------
CreatureBase::CreatureBase( Vec3 const& rootStartPos, float length )
{
	m_root = new IK_Joint3D( 0, rootStartPos, length );
}


//----------------------------------------------------------------------------------------------------------------------
CreatureBase::~CreatureBase()
{
}


//----------------------------------------------------------------------------------------------------------------------
void CreatureBase::Update()
{
	for ( int i = 0; i < m_skeletalSystemsList.size(); i++ )
	{
		m_skeletalSystemsList[i]->Update();
	}
}


//----------------------------------------------------------------------------------------------------------------------
void CreatureBase::Render( std::vector<Vertex_PCU>& verts, Rgba8 const& limbColor, Rgba8 const& jointColor, bool renderDebugJointBasis, bool const& renderDebugCurrentPos_EE ) const
{
	for ( int i = 0; i < m_skeletalSystemsList.size(); i++ )
	{
		if ( m_skeletalSystemsList[ i ]->m_shouldRender )
		{
			m_skeletalSystemsList[i]->Render( verts, limbColor, jointColor, renderDebugJointBasis, renderDebugCurrentPos_EE  );
		}
	}
}


//----------------------------------------------------------------------------------------------------------------------
// localOffsetToRoot is the position of the new skeletal system being created, relative to the root
//----------------------------------------------------------------------------------------------------------------------
void CreatureBase::CreateChildSkeletalSystem( std::string const& name, Vec3 const& localOffsetToRoot,  IK_Joint3D* ownerSkeletonFirstJoint, CreatureBase* const creatureOwner, bool shouldReachInsteadOfDrag )
{
	if ( ownerSkeletonFirstJoint == nullptr )
	{
		ownerSkeletonFirstJoint = m_root;
	}
	IK_Chain3D* newSystem = new IK_Chain3D( name, localOffsetToRoot, ownerSkeletonFirstJoint, creatureOwner, shouldReachInsteadOfDrag );
	m_skeletalSystemsList.emplace_back( newSystem );
}


//----------------------------------------------------------------------------------------------------------------------
void CreatureBase::CreateLimbsForIKChain( std::string const& nameOfSkeletalSystem, float numLimbs, float limbLength, Vec3 jointFwdDir, JointConstraintType const& jointConstraintType, FloatRange const& yawConstraints, FloatRange const& pitchConstraints, FloatRange const& rollConstraints )
{
	 IK_Chain3D* skeleton	= GetSkeletonByName( nameOfSkeletalSystem );
	 int limbListSize		= int( skeleton->m_jointList.size() );
	 for ( int i = 0; i < numLimbs; i++ )
	 {
		 skeleton->CreateNewLimb( (limbListSize + i), limbLength, jointFwdDir, jointConstraintType, nullptr, yawConstraints, pitchConstraints, rollConstraints );
	 }
	 skeleton->m_finalJoint->m_solveSingleStep_Forwards = true;
}


//----------------------------------------------------------------------------------------------------------------------
IK_Chain3D* CreatureBase::GetSkeletonByName( std::string const& nameOfSkeletalSystem )
{
	// Return a pointer to the skeletal system with a matching name
	for ( int i = 0; i < m_skeletalSystemsList.size(); i++ )
	{
		IK_Chain3D* currentSkeleton = m_skeletalSystemsList[ i ];
		if ( currentSkeleton->m_name == nameOfSkeletalSystem )
		{
			return currentSkeleton;
		}
	}
	// If we got here, there was no valid skeletal system matching the "name"
	ERROR_AND_DIE( "INVALID SKELETON NAME" );
}


//----------------------------------------------------------------------------------------------------------------------
// "maxDist" is default initalized to -1. Unless a valid value is specified
// this function will compare dist with the maxLength of the entire skeletal system 
//----------------------------------------------------------------------------------------------------------------------
bool CreatureBase::IsLimbTooFarFromRoot( std::string const& nameOfSkeletalSystem, float maxDist )
{
	IK_Chain3D* currentLimb = GetSkeletonByName( nameOfSkeletalSystem );	
	if ( maxDist == -1.0f )
	{
		// If "maxDist" was NOT specified, use the default value ( "maxLength" of this Skeletal System )
		maxDist = currentLimb->GetMaxLengthOfSkeleton();
	}
	// Check if limb is placed too far from Root
	float distLimbEndToRootSq = GetDistance3D( currentLimb->m_target.m_currentPos, m_root->m_jointPos_LS );
	float maxDistSq = maxDist;
	if ( distLimbEndToRootSq > maxDistSq )
	{
		return true;
	}
	return false;
}


//----------------------------------------------------------------------------------------------------------------------
// "maxDist" is default initalized to -1. Unless a valid value is specified
// this function will compare dist with the maxLength of the entire skeletal system 
//----------------------------------------------------------------------------------------------------------------------
bool CreatureBase::IsLimbTooFarFromRoot( IK_Chain3D* const currentLimb, float maxDist )
{
	if ( maxDist == -1.0f )
	{
		// If "maxDist" was NOT specified, use the default value ( "maxLength" of this Skeletal System )
		maxDist = currentLimb->GetMaxLengthOfSkeleton();
	}
	// Check if limb is placed too far from Root
	float distLimbEndToRootSq	= GetDistance3D( currentLimb->m_finalJoint->m_endPos, m_root->m_jointPos_LS );
	float maxDistSq				= maxDist;
	if ( distLimbEndToRootSq > maxDistSq )
	{
		return true;
	}
	return false;
}


//----------------------------------------------------------------------------------------------------------------------
bool CreatureBase::IsLimbTooFarFromSegmentEnd( IK_Chain3D* const currentLimb, IK_Joint3D* const refSegment, float maxDist )
{
	if ( maxDist == -1.0f )
	{
		// If "maxDist" was NOT specified, use the default value ( "maxLength" of this Skeletal System )
		maxDist = currentLimb->GetMaxLengthOfSkeleton();
	}
	// Check if limb is placed too far from Root
	float distLimbEndToRootSq = GetDistance3D( currentLimb->m_finalJoint->m_endPos, refSegment->m_endPos );
	float maxDistSq			  = maxDist;
	if ( distLimbEndToRootSq > maxDistSq )
	{
		return true;
	}
	return false;
}


//----------------------------------------------------------------------------------------------------------------------
bool CreatureBase::IsLimbTooFarFromPos( IK_Chain3D* const currentLimb, Vec3 const& refPosition, float maxDist )
{
	if ( maxDist == -1.0f )
	{
		// If "maxDist" was NOT specified, use the default value ( "maxLength" of this Skeletal System )
		maxDist = currentLimb->GetMaxLengthOfSkeleton();
	}
	// Check if limb is placed too far from Root
	float distLimbEndToRoot	= GetDistance3D( currentLimb->m_target.m_currentPos, refPosition );
	if ( distLimbEndToRoot > maxDist )
	{
		return true;
	}
	return false;
}