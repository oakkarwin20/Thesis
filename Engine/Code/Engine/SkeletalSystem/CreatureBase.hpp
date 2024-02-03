#pragma once

#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/SkeletalSystem/IK_Joint3D.hpp"

#include <vector>
#include <string>

//----------------------------------------------------------------------------------------------------------------------
class IK_Joint3D;
class IK_Chain3D;


//----------------------------------------------------------------------------------------------------------------------
class CreatureBase
{
public:
	CreatureBase( Vec3 const& rootStartPos = Vec3::ZERO, float length = 1.0f );
	~CreatureBase();

	void Update();
	void Render( std::vector<Vertex_PCU>& verts, Rgba8 const& limbColor, Rgba8 const& jointColor, bool renderDebugJointBasis = false, bool const& renderDebugCurrentPos_EE = false ) const;

	// Initialization Functions
	void CreateChildSkeletalSystem	 ( std::string const& name, Vec3 const& localOffsetToRoot,  IK_Joint3D* ownerSkeletonFirstJoint = nullptr, CreatureBase* const creatureOwner = nullptr, bool shouldReachInsteadOfDrag = true );
	void CreateLimbsForIKChain		 ( std::string			const&	nameOfSkeletalSystem,
									   float						numLimbs, 
									   float						limbLength			= 1.0f, 
									   Vec3							jointFwdDir			= Vec3::X_FWD,
									   JointConstraintType	const&	jointConstraintType = JOINT_CONSTRAINT_TYPE_DISTANCE,
									   FloatRange			const&	yawConstraints		= FloatRange( -180.0f, 180.0f ),
									   FloatRange			const&	pitchConstraints	= FloatRange( -180.0f, 180.0f ),
									   FloatRange			const&	rollConstraints		= FloatRange( -180.0f, 180.0f ) );
	
	// Util Functions
	IK_Chain3D*	GetSkeletonByName			( std::string const& nameOfSkeletalSystem );
	bool		IsLimbTooFarFromRoot		( std::string const& nameOfSkeletalSystem, float maxDist = -1.0f );
	bool		IsLimbTooFarFromRoot		( IK_Chain3D* const currentLimb, float maxDist );
	bool		IsLimbTooFarFromSegmentEnd	( IK_Chain3D* const currentLimb, IK_Joint3D* const refSegment, float maxDist );
	bool		IsLimbTooFarFromPos			( IK_Chain3D* const currentLimb, Vec3 const& refPosition, float maxDist );

public:
	std::vector<IK_Chain3D*>	m_skeletalSystemsList;
	IK_Joint3D*				m_root					 = nullptr;
};