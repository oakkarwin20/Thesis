#pragma once

#include "Engine/Math/Vec3.hpp"

//----------------------------------------------------------------------------------------------------------------------
struct AABB3
{
public:
	Vec3 m_mins;
	Vec3 m_maxs;

public:
	// Construction / Destruction
	AABB3();
	~AABB3();
	AABB3( AABB3 const& copyFrom );
	explicit AABB3( float minX, float minY, float minZ, float maxX, float maxY, float maxZ );
	explicit AABB3( Vec3 const& mins, Vec3 const& maxs );

	// Accessors (const methods)
	Vec3 const  GetCenter() const;
	bool		IsPointInside( Vec3 const& point ) const;
	Vec3 const	GetNearestPoint( Vec3 const& referencePosition ) const;

	// Mutators (non-const methods)
	void		SetCenterXY( Vec3 const& newCenter );
	void		SetCenterXYZ( Vec3 const& newCenter );
	void		SetNewZ( float newHeight );


	static AABB3 ZERO_TO_ONE;
};