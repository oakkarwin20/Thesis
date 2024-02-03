#pragma once

#include "Engine/Math/CubicBezierCurve2D.hpp"

#include <vector>

//----------------------------------------------------------------------------------------------------------------------
class Spline
{
public:
	Spline( std::vector<Vec2> const& positions );
	Spline() {}
	~Spline();



public:
	std::vector<CubicBezierCurve2D> m_bezierCurvesList;
};