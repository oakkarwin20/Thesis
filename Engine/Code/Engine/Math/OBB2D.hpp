#pragma once

#include "Engine/Math/Vec2.hpp"

//----------------------------------------------------------------------------------------------------------------------
struct OBB2D
{
	OBB2D( Vec2 center, Vec2 iBasis, Vec2 halfDimensions );
	OBB2D();
	~OBB2D();

	void GetCornerPoints( Vec2& out_BL, Vec2& out_BR, Vec2& out_TR, Vec2& out_TL ) const;
	Vec2 GetlocalPosFromWorldPos( Vec2 worldPos ) const;
	Vec2 GetWorldPosFromLocalPos( Vec2 localPos ) const;

	bool IsPointInside( Vec2 const& point ) const;


	Vec2 m_center			= Vec2( 0.0f, 0.0f );
	Vec2 m_iBasisNormal		= Vec2( 1.0f, 0.0f );
	Vec2 m_halfDimensions	= Vec2( 0.0f, 0.0f );
};