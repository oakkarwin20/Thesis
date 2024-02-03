#pragma once
#include "Engine/Math/Vec2.hpp"

//-----------------------------------------------------------------------------------------------------------------------
struct AABB2
{
public:
	Vec2 m_mins;
	Vec2 m_maxs;

public:
	// Construction / Destruction
	AABB2();
	~AABB2();
	AABB2( AABB2 const& copyFrom );
	explicit AABB2( float minX, float minY, float maxX, float maxY );
	explicit AABB2( Vec2 const& mins, Vec2 const& maxs );
	explicit AABB2( Vec2 const& center, float width, float height );

	// Accessors (const methods)
	bool		IsPointInside( Vec2 const& point ) const;
	Vec2 const	GetCenter() const;
	Vec2 const	GetDimensions() const;
	Vec2 const	GetNearestPoint( Vec2 const& referencePosition ) const;
	Vec2 const	GetPointAtUV( Vec2 const& uv ) const;
	Vec2 const	GetUVForPoint( Vec2 const& point ) const;

	// Mutators (non-const methods)
	void		Translate( Vec2 const& translationToApply );
	void		SetCenter( Vec2 const& newCenter );
	void		SetBottomLeft( Vec2 const& newBottomLeft );
	void		SetDimensions( Vec2 const& newDimensions );
	void		StretchToIncludePoint( Vec2 const& point );

	//----------------------------------------------------------------------------------------------------------------------
	// Utility functions
	AABB2 GetAABB2AtUVs( Vec2 UVmins, Vec2 UVmaxs );
	void  AddPadding( float paddingX, float paddingY );

	static AABB2 ZERO_TO_ONE;
};