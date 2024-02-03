
#include "Engine/Math/OBB2D.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/MathUtils.hpp"

//----------------------------------------------------------------------------------------------------------------------
OBB2D::OBB2D( Vec2 center, Vec2 iBasis, Vec2 halfDimensions )
{
	m_center			= center;	
	m_iBasisNormal		= iBasis;
	m_halfDimensions	= halfDimensions;
}


//----------------------------------------------------------------------------------------------------------------------
OBB2D::OBB2D()
{
}


//----------------------------------------------------------------------------------------------------------------------
OBB2D::~OBB2D()
{
}


//----------------------------------------------------------------------------------------------------------------------
// #ToDo fix this function, not fully understanding what the parameter is used for
void OBB2D::GetCornerPoints( Vec2& out_BL, Vec2& out_BR, Vec2& out_TR, Vec2& out_TL ) const
{
	Vec2 jBasisNormal	= m_iBasisNormal.GetRotated90Degrees();
	Vec2 right			= m_iBasisNormal * m_halfDimensions.x;
	Vec2 up				= jBasisNormal * m_halfDimensions.y;
	
	Vec2 topRight		= m_center + right + up;
	Vec2 topLeft		= m_center - right + up;
	Vec2 bottomRight	= m_center + right - up;
	Vec2 bottomLeft		= m_center - right - up;

	out_BL = bottomLeft;
	out_BR = bottomRight;
	out_TL = topLeft;
	out_TR = topRight;
}


//----------------------------------------------------------------------------------------------------------------------
Vec2 OBB2D::GetlocalPosFromWorldPos( Vec2 worldPos ) const
{
	// Define world position relative to obb2's centerPos
	// How many I and J steps away is the worldPos from the obbCenterPos?
	Vec2 jBasis					= m_iBasisNormal.GetRotated90Degrees();
	Vec2 dispCenterToWorld		= worldPos - m_center;
	float sj					= GetProjectedLength2D( dispCenterToWorld, jBasis );
	float si					= GetProjectedLength2D( dispCenterToWorld, m_iBasisNormal );
//	Vec2 localPos				= ( m_iBasisNormal * si ) + ( jBasis * sj );
	Vec2 localPos				= Vec2( si, sj );
	return localPos;

}


//----------------------------------------------------------------------------------------------------------------------
Vec2 OBB2D::GetWorldPosFromLocalPos( Vec2 localPos ) const
{
	// Get offset from center to local pos
	// Add center and offsets to get world position from the local space
	Vec2 jBasis		= m_iBasisNormal.GetRotated90Degrees();
	Vec2 worldPos	= m_center + ( localPos.x * m_iBasisNormal ) * ( localPos.y * jBasis );
	return worldPos;
}


//----------------------------------------------------------------------------------------------------------------------
bool OBB2D::IsPointInside( Vec2 const& point ) const
{
	Vec2 minX; 
	point;
/*
	if ( point.x > m_mins.x && point.y > m_mins.y
		&& point.x < m_maxs.x && point.y < m_maxs.y )
	{
		return true;
	}
	else
	{
		return false;
	}
*/

	return false;
}
