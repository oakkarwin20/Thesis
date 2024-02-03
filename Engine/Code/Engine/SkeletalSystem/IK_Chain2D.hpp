 #pragma once

#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/SkeletalSystem/IK_Joint2D.hpp"

#include <string>
#include <map>
#include <vector>

//----------------------------------------------------------------------------------------------------------------------
class IK_Chain2D
{
public:
	IK_Chain2D( Vec2 position );
	~IK_Chain2D();

	void Startup();
	void Shutdown();
	void Update();
	void Render( std::vector<Vertex_PCU>& verts ) const;

	void CreateNewLimb( float length, float orientation );
	void DragLimb( Vec2 targetPos );
	void ReachLimb( Vec2 targetPos );
	void SetChildrenStartPosAtParentsEndXY();

public:
	Vec2						m_position		= Vec2::ZERO;		
	std::vector<IK_Joint2D*>	m_limbList;
	IK_Joint2D*					m_finalJoint		= nullptr;			// The limb at the end of the limbList
};

//----------------------------------------------------------------------------------------------------------------------
// Root
//		AABB2( Vec2 mins, Vec2 maxs )
//			Vec2 centerPos ?
//			Float radius
// Joints
//		Vec2 Position
// Limb
//		Float Length
//		Float Orientation
//		Float range( physics constraints ) ?
// End effector
//		Vec2 position( goal position ?
