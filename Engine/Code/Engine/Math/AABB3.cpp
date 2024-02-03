#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"


//----------------------------------------------------------------------------------------------------------------------
AABB3 AABB3::ZERO_TO_ONE = AABB3( 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f );


//----------------------------------------------------------------------------------------------------------------------
AABB3::AABB3()
{
}

//----------------------------------------------------------------------------------------------------------------------
AABB3::~AABB3()
{
}

//----------------------------------------------------------------------------------------------------------------------
AABB3::AABB3( AABB3 const& copyFrom )
{
	m_mins = copyFrom.m_mins;
	m_maxs = copyFrom.m_maxs;
}

//----------------------------------------------------------------------------------------------------------------------
AABB3::AABB3( float minX, float minY, float minZ, float maxX, float maxY, float maxZ )
{
	m_mins.x =  minX;
	m_mins.y =  minY;
	m_mins.z =  minZ;
	m_maxs.x =  maxX;
	m_maxs.y =  maxY;
	m_maxs.z =  maxZ;
}

//----------------------------------------------------------------------------------------------------------------------
AABB3::AABB3( Vec3 const& mins, Vec3 const& maxs )
{
	m_mins = mins;
	m_maxs = maxs;
}

//----------------------------------------------------------------------------------------------------------------------
Vec3 const AABB3::GetCenter() const
{
	Vec3 distanceTraveled = m_maxs - m_mins;
	Vec3 centerPoint = distanceTraveled / 2.0f;
	centerPoint += m_mins;
	return Vec3( centerPoint );
}

//----------------------------------------------------------------------------------------------------------------------
bool AABB3::IsPointInside( Vec3 const& point ) const
{
	if  (	point.x > m_mins.x 
		 && point.y > m_mins.y 
		 && point.z > m_mins.z 
		 &&	point.x < m_maxs.x 
		 && point.y < m_maxs.y 
		 && point.z < m_maxs.z 
		)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//----------------------------------------------------------------------------------------------------------------------
Vec3 const AABB3::GetNearestPoint( Vec3 const& referencePosition ) const
{
	// if ref position is inside box, return ref

	float nearestPointX = GetClamped( referencePosition.x, m_mins.x, m_maxs.x );
	float nearestPointY = GetClamped( referencePosition.y, m_mins.y, m_maxs.y );
	float nearestPointZ = GetClamped( referencePosition.z, m_mins.z, m_maxs.z );
	Vec3 nearestPoint	= Vec3( nearestPointX, nearestPointY, nearestPointZ );
	return nearestPoint;
}


//----------------------------------------------------------------------------------------------------------------------
void AABB3::SetCenterXY( Vec3 const& newCenter )
{
	// preserve the mins and maxs but change the center's position
	float boxWidth	= m_maxs.x - m_mins.x;
	float boxHeight = m_maxs.y - m_mins.y;

	m_mins.x = newCenter.x - ( boxWidth  * 0.5f );
	m_maxs.x = newCenter.x + ( boxWidth  * 0.5f );

	m_maxs.y = newCenter.y + ( boxHeight * 0.5f );
	m_mins.y = newCenter.y - ( boxHeight * 0.5f );
}

//----------------------------------------------------------------------------------------------------------------------
void AABB3::SetCenterXYZ( Vec3 const& newCenter )
{
	// preserve the mins and maxs but change the center's position
	float boxX = m_maxs.x - m_mins.x;
	float boxY = m_maxs.y - m_mins.y;
	float boxZ = m_maxs.z - m_mins.z;

	m_mins.x = newCenter.x - ( boxX * 0.5f );
	m_maxs.x = newCenter.x + ( boxX * 0.5f );

	m_maxs.y = newCenter.y + ( boxY * 0.5f );
	m_mins.y = newCenter.y - ( boxY * 0.5f );

	m_maxs.z = newCenter.z + ( boxZ * 0.5f );
	m_mins.z = newCenter.z - ( boxZ * 0.5f );
}

//----------------------------------------------------------------------------------------------------------------------
void AABB3::SetNewZ( float newZ )
{
	float aabb3Height	= m_maxs.z - m_mins.z;
	m_mins.z			= newZ;
	m_maxs.z			= m_mins.z + aabb3Height;
}
