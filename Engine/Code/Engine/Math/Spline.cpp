#include "Engine/Math/Spline.hpp"

//----------------------------------------------------------------------------------------------------------------------
Spline::Spline( std::vector<Vec2> const& positions )
{
	if ( positions.size() < 2 )
	{
		return;
	}

	for ( int i = 0; i < positions.size() - 1; i++ )
	{
		// Create cubic Bezier segments
		Vec2 startPoint = positions[i];
		Vec2 endPoint	= positions[i + 1];

		Vec2 startVelocity;
		if ( i == 0 )
		{
			startVelocity = Vec2::ZERO;
		}
		else
		{
			Vec2 prevPoint = positions[i - 1];
			Vec2 nextPoint = positions[i + 1];
			startVelocity  = (nextPoint - prevPoint) / 2.0f;
		}


		Vec2 endVelocity;
		if ( i == positions.size() - 2 )
		{
			endVelocity = Vec2::ZERO;
		}
		else
		{
			Vec2 prevPoint = positions[i];
//			Vec2 nextPoint = positions[i + 1];
			Vec2 nextPoint = positions[i + 2];
			endVelocity = ( nextPoint - prevPoint ) / 2.0f;
		}

		CubicHermiteCurve2D cubicHermite2D;
		cubicHermite2D.m_startPos		= startPoint;
		cubicHermite2D.m_endPos			= endPoint;
		cubicHermite2D.m_startVelocity	= startVelocity;
		cubicHermite2D.m_endVelocity	= endVelocity;

		CubicBezierCurve2D cubicBezier2D = CubicBezierCurve2D( cubicHermite2D );
		m_bezierCurvesList.push_back( cubicBezier2D );
	}
}

//----------------------------------------------------------------------------------------------------------------------
Spline::~Spline()
{
}