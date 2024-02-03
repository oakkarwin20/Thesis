#pragma once

#include "Engine/Math/Vec3.hpp"


//----------------------------------------------------------------------------------------------------------------------
class CubicBezierCurve3D
{
public:
	CubicBezierCurve3D( Vec3 startPos, Vec3 guidePos1, Vec3 guidePos2, Vec3 endPos );
	Vec3 GetPointAtTime( float parametericZeroToOne ) const;

public:
	Vec3 m_startPos  = Vec3::ZERO;		// A
	Vec3 m_guidePos1 = Vec3::ZERO;		// B
	Vec3 m_guidePos2 = Vec3::ZERO;		// C
	Vec3 m_endPos	 = Vec3::ZERO;		// D
};