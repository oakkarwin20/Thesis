#include "Engine/Math/CubicBezierCurve2D.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vec2.hpp"
#include "CubicBezierCurve3D.hpp"

//----------------------------------------------------------------------------------------------------------------------
CubicBezierCurve3D::CubicBezierCurve3D( Vec3 startPos, Vec3 guidePos1, Vec3 guidePos2, Vec3 endPos )
{
	m_startPos  = startPos;
	m_guidePos1 = guidePos1;
	m_guidePos2 = guidePos2;
	m_endPos    = endPos;
}

	
//----------------------------------------------------------------------------------------------------------------------
Vec3 CubicBezierCurve3D::GetPointAtTime( float parametericZeroToOne ) const
{
	// Lerp of Lerp of Lerps
	Vec3 E = Interpolate(  m_startPos, m_guidePos1, parametericZeroToOne );
	Vec3 F = Interpolate( m_guidePos1, m_guidePos2, parametericZeroToOne );
	Vec3 G = Interpolate( m_guidePos2,	  m_endPos, parametericZeroToOne );

	Vec3 H = Interpolate( E, F, parametericZeroToOne );
	Vec3 I = Interpolate( F, G, parametericZeroToOne );

	Vec3 J = Interpolate( H, I, parametericZeroToOne );

	return J;
}