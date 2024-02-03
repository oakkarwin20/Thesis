#pragma once

#include "Engine/Math/Vec2.hpp"

//----------------------------------------------------------------------------------------------------------------------
struct CubicHermiteCurve2D
{
	Vec2 m_startPos;
	Vec2 m_endPos;
	Vec2 m_startVelocity;
	Vec2 m_endVelocity;
};

//----------------------------------------------------------------------------------------------------------------------
class CubicBezierCurve2D
{
public:
	CubicBezierCurve2D( Vec2 startPos, Vec2 guidePos1, Vec2 guidePos2, Vec2 endPos );
	explicit CubicBezierCurve2D( CubicHermiteCurve2D const& fromHermite );
	Vec2 GetPointAtTime( float parametericZeroToOne ) const;
	float GetApproximateLength( int numSubdivisions = 64 ) const;
	Vec2 EvaluateAtApproximateDistance( float distanceAlongCurve, int numSubdivisions = 64 ) const;

	Vec2 GetStartVelocity() const;
	Vec2 GetEndVelocity() const;
	void SetStartVelocity( Vec2 velocity );
	void SetEndVelocity( Vec2 velocity );

public:
	Vec2 m_startPos  = Vec2::ZERO;		// A
	Vec2 m_guidePos1 = Vec2::ZERO;		// B
	Vec2 m_guidePos2 = Vec2::ZERO;		// C
	Vec2 m_endPos	 = Vec2::ZERO;		// D
	Vec2 m_startVelocity;
	Vec2 m_endVelocity;
};