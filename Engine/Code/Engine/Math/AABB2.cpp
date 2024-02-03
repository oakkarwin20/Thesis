#include "AABB2.hpp"
#include "Math.h"
#include "Engine/Math/MathUtils.hpp"

//----------------------------------------------------------------------------------------------------------------------
AABB2 AABB2::ZERO_TO_ONE = AABB2( 0.0f, 0.0f, 1.0f, 1.0f);

//----------------------------------------------------------------------------------------------------------------------
AABB2::AABB2()
{
}

//----------------------------------------------------------------------------------------------------------------------
AABB2::~AABB2()
{
}

//----------------------------------------------------------------------------------------------------------------------
AABB2::AABB2(AABB2 const& copyFrom)
	:m_mins(copyFrom.m_mins)
	,m_maxs(copyFrom.m_maxs)
{
}

//----------------------------------------------------------------------------------------------------------------------
AABB2::AABB2(float minX, float minY, float maxX, float maxY)
	:m_mins( minX, minY )
	,m_maxs( maxX, maxY )
{
}

//----------------------------------------------------------------------------------------------------------------------
AABB2::AABB2(Vec2 const& mins, Vec2 const& maxs)
	:m_mins( mins )
	,m_maxs( maxs )
{
}

//----------------------------------------------------------------------------------------------------------------------
AABB2::AABB2( Vec2 const& center, float width, float height )
{
	m_mins.x = center.x - ( width  * 0.5f );
	m_mins.y = center.y - ( height * 0.5f );
	m_maxs.x = center.x + ( width  * 0.5f );
	m_maxs.y = center.y + ( height * 0.5f );
}

//----------------------------------------------------------------------------------------------------------------------
bool AABB2::IsPointInside(Vec2 const& point) const
{
	if ( point.x > m_mins.x && point.y > m_mins.y 
	  && point.x < m_maxs.x && point.y < m_maxs.y )
	{
		return true;
	}
	else
	{
		return false;
	}
} 

//----------------------------------------------------------------------------------------------------------------------
Vec2 const AABB2::GetCenter() const
{
	Vec2 distanceTraveled = m_maxs - m_mins;
	Vec2 centerPoint = distanceTraveled / 2;
	centerPoint += m_mins;
	return Vec2( centerPoint );
}

//----------------------------------------------------------------------------------------------------------------------
Vec2 const AABB2::GetDimensions() const
{
	//calculate (maxX - minX) and (maxY - minY)
	float lengthX = m_maxs.x - m_mins.x;
	float lengthY = m_maxs.y - m_mins.y;
	return Vec2( lengthX, lengthY );
}

//----------------------------------------------------------------------------------------------------------------------
Vec2 const AABB2::GetNearestPoint(Vec2 const& referencePosition) const
{
	// if ref position is inside box, return ref

	float nearestPointX = GetClamped(referencePosition.x, m_mins.x, m_maxs.x);
	float nearestPointY = GetClamped(referencePosition.y, m_mins.y, m_maxs.y);

	return Vec2(nearestPointX, nearestPointY);
}

//----------------------------------------------------------------------------------------------------------------------
Vec2 const AABB2::GetPointAtUV(Vec2 const& uv) const
{
	//uv means normalized scale 
	/*
	float difX = uv.x * m_maxs.x;
	float difY = uv.y * m_maxs.y;

	float newX = difX + m_mins.x;
	float newY = difY + m_mins.y;

	return Vec2( newX, newY	);
	*/

	float uvX = Interpolate( m_mins.x, m_maxs.x, uv.x);
	float uvY = Interpolate( m_mins.y, m_maxs.y, uv.y);

	return Vec2( uvX, uvY );
}    

//----------------------------------------------------------------------------------------------------------------------
Vec2 const AABB2::GetUVForPoint(Vec2 const& point) const
{
	/* Reference for learning, original wrong code 
	float newX = point.x / m_maxs.x;
	float newY = point.y / m_maxs.y;

	return Vec2( newX, newY );
	*/

	float pointX = GetFractionWithinRange( point.x, m_mins.x, m_maxs.x );
	float pointY = GetFractionWithinRange( point.y, m_mins.y, m_maxs.y );

	return Vec2( pointX, pointY );
}

//----------------------------------------------------------------------------------------------------------------------
void AABB2::Translate(Vec2 const& translationToApply)
{
	// move all mins and maxs position coordinates but adding "translationToApply"
	Vec2 newMins = m_mins + translationToApply;
	Vec2 newMaxs = m_maxs + translationToApply;

	m_mins.x = newMins.x;
	m_mins.y = newMins.y;

	m_maxs.x = newMaxs.x;
	m_maxs.y = newMaxs.y;
}

//----------------------------------------------------------------------------------------------------------------------
void AABB2::SetCenter(Vec2 const& newCenter)
{
	// preserve the mins and maxs but change the center's position
	float boxWidth  = m_maxs.x - m_mins.x;
	float boxHeight = m_maxs.y - m_mins.y;

	m_mins.x = newCenter.x - ( boxWidth  * 0.5f );
	m_maxs.x = newCenter.x + ( boxWidth  * 0.5f );

	m_maxs.y = newCenter.y + ( boxHeight * 0.5f );
	m_mins.y = newCenter.y - ( boxHeight * 0.5f );

	// previous ways I was trying to solve this problem, keeping temporarily for reference and learning purposes
	//----------------------------------------------------------------------------------------------------------------------------------
	//newCenter.x = currentCenter.x + (newCenter.x - currentCenter.x);
	//newCenter.y = currentCenter.y + (newCenter.y - currentCenter.y);

	//currentCenter.x = newCenter.x;
	//currentCenter.y = newCenter.y;

	//m_maxs.x = newCenter.x + ( m_maxs.x / 2 );
	//m_maxs.y = newCenter.y + ( m_maxs.y / 2 );
	//m_mins.x = newCenter.x - ( m_mins.x / 2 );
	//m_mins.y = newCenter.y - ( m_mins.y / 2 );

	//float difMaxX = m_maxs.x - newCenter.x;		// 13 - 11  =  2
	//float difMinX = m_mins.x - newCenter.x;		//  9 - 11  = -2
	//float difMaxY = m_maxs.y - newCenter.y;		//  5 - 2.5	=  2.5
	//float difMinY = m_mins.y - newCenter.y;		//  0 - 2.5	= -2.5

	//m_maxs.x = newCenter.x + difMaxX;	
	//m_mins.x = newCenter.x + difMinX;	
	//m_maxs.y = newCenter.y + difMaxY;	
	//m_mins.y = newCenter.y + difMinY;
	//----------------------------------------------------------------------------------------------------------------------------------
}


//----------------------------------------------------------------------------------------------------------------------
void AABB2::SetBottomLeft( Vec2 const& newBottomLeft )
{
	// Calculate
	float boxWidth  = m_maxs.x - m_mins.x;
	float boxHeight = m_maxs.y - m_mins.y;

	// Set new mins and maxs
	m_mins	 = newBottomLeft;
	m_maxs.x = m_mins.x + boxWidth;
	m_maxs.y = m_mins.y + boxHeight;
}

//----------------------------------------------------------------------------------------------------------------------
void AABB2::SetDimensions(Vec2 const& newDimensions)
{
	// preserve the center point's position but change the mins and maxs
	Vec2 currentCenter = GetCenter();

	float currentX = currentCenter.x;
	float currentY = currentCenter.y;

	float newX = newDimensions.x;
	float newY = newDimensions.y; 
	
	m_mins.x = currentX - ( newX / 2 );
	m_maxs.x = currentX + ( newX / 2 );
	m_mins.y = currentY - ( newY / 2 );
	m_maxs.y = currentY + ( newY / 2 );
}

//----------------------------------------------------------------------------------------------------------------------
void AABB2::StretchToIncludePoint(Vec2 const& point)
{
	//4 if statements of if point is inside box, if point outside, stretch mins or mins coords
	
	if (point.x < m_mins.x)
	{
		m_mins.x = point.x;
	}
	if (point.y < m_mins.y)
	{
		m_mins.y = point.y;
	}
	if (point.x > m_maxs.x)
	{
		m_maxs.x = point.x;
	}
	if (point.y > m_maxs.y)
	{
		m_maxs.y = point.y;
	}
}

//----------------------------------------------------------------------------------------------------------------------
AABB2 AABB2::GetAABB2AtUVs( Vec2 UVmins, Vec2 UVmaxs )
{
	Vec2 mins = GetPointAtUV( UVmins );
	Vec2 maxs = GetPointAtUV( UVmaxs );
	AABB2 box = AABB2( m_mins, maxs );
	return box;
}

//----------------------------------------------------------------------------------------------------------------------
void AABB2::AddPadding( float paddingX, float paddingY )
{
//	m_mins.x -= paddingX;
//	m_maxs.x += paddingX;
//
//	m_mins.y -= paddingY;
//	m_maxs.y += paddingY;

	m_mins.x += paddingX;
	m_maxs.x -= paddingX;

	m_mins.y += paddingY;
	m_maxs.y -= paddingY;

}
