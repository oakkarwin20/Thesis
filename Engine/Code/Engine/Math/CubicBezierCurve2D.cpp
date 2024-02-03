#include "Engine/Math/CubicBezierCurve2D.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vec2.hpp"

//----------------------------------------------------------------------------------------------------------------------
CubicBezierCurve2D::CubicBezierCurve2D( Vec2 startPos, Vec2 guidePos1, Vec2 guidePos2, Vec2 endPos )
{
	m_startPos  = startPos;
	m_guidePos1 = guidePos1;
	m_guidePos2 = guidePos2;
	m_endPos	= endPos;
}

//----------------------------------------------------------------------------------------------------------------------
CubicBezierCurve2D::CubicBezierCurve2D( CubicHermiteCurve2D const& fromHermite )
{
	m_startPos		= fromHermite.m_startPos; 
	m_endPos		= fromHermite.m_endPos; 
	m_startVelocity = fromHermite.m_startVelocity;
	m_endVelocity	= fromHermite.m_endVelocity;

	m_guidePos1 = ( m_startVelocity + (3.0f * m_startPos) ) / 3.0f;
	m_guidePos2 = ( -1.0f * m_endVelocity + (3.0f * m_endPos) ) / 3.0f;
}
	
//----------------------------------------------------------------------------------------------------------------------
Vec2 CubicBezierCurve2D::GetPointAtTime( float parametericZeroToOne ) const
{
	// Lerp of Lerp of Lerps
	Vec2 E = Interpolate(  m_startPos, m_guidePos1, parametericZeroToOne );
	Vec2 F = Interpolate( m_guidePos1, m_guidePos2, parametericZeroToOne );
	Vec2 G = Interpolate( m_guidePos2,	  m_endPos, parametericZeroToOne );

	Vec2 H = Interpolate( E, F, parametericZeroToOne );
	Vec2 I = Interpolate( F, G, parametericZeroToOne );

	Vec2 J = Interpolate( H, I, parametericZeroToOne );

	return J;
}

//----------------------------------------------------------------------------------------------------------------------
float CubicBezierCurve2D::GetApproximateLength( int numSubdivisions ) const
{
	UNUSED( numSubdivisions );
	return 0.0f;
}

//----------------------------------------------------------------------------------------------------------------------
Vec2 CubicBezierCurve2D::EvaluateAtApproximateDistance( float distanceAlongCurve, int numSubdivisions ) const
{
	UNUSED( distanceAlongCurve );
	UNUSED( numSubdivisions );
	return Vec2();
}

//----------------------------------------------------------------------------------------------------------------------
Vec2 CubicBezierCurve2D::GetStartVelocity() const 
{
	Vec2 startVelocity = ( m_guidePos1 - m_startPos ) * 3.0f;
	return startVelocity;
}

//----------------------------------------------------------------------------------------------------------------------
Vec2 CubicBezierCurve2D::GetEndVelocity() const
{
	Vec2 endVelocty = ( m_endPos - m_guidePos2 ) * 3.0f;
	return endVelocty;
}

//----------------------------------------------------------------------------------------------------------------------
void CubicBezierCurve2D::SetStartVelocity( Vec2 velocity )
{
	Vec2 dividedVelocity = velocity / 3.0f;
	m_guidePos1			 = m_startPos + dividedVelocity;
}

//----------------------------------------------------------------------------------------------------------------------
void CubicBezierCurve2D::SetEndVelocity( Vec2 velocity )
{
	Vec2 dividedVelocity = velocity / 3.0f;
	m_guidePos2			 = m_endPos + dividedVelocity;
}
