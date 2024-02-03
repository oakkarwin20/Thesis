#pragma once

#include "Engine/Math/Vec3.hpp"

//----------------------------------------------------------------------------------------------------------------------
struct OBB3D
{
	OBB3D( Vec3 center, Vec3 iBasis, Vec3 jBasis, Vec3 kBasis, Vec3 halfDimensions );
	~OBB3D();

	void GetCornerPoints( Vec3* out_fourCornerWorldPositions ) const;
	Vec3 GetlocalPosForWorldPos( Vec3 worldPos ) const;
	Vec3 GetWorldPosForLocalPos( Vec3 localPos ) const;

	Vec3 m_center			= Vec3( 0.0f, 0.0f, 0.0f );
	Vec3 m_iBasisNormal		= Vec3( 1.0f, 0.0f, 0.0f );
	Vec3 m_jBasisNormal		= Vec3( 0.0f, 1.0f, 0.0f );
	Vec3 m_kBasisNormal		= Vec3( 0.0f, 0.0f, 1.0f );
	Vec3 m_halfDimensions	= Vec3( 0.0f, 0.0f, 0.0f );
};