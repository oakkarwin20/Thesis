#pragma once

//----------------------------------------------------------------------------------------------------------------------
class FloatRange
{
public:
	FloatRange();
	FloatRange( float min, float max );
	~FloatRange();

	bool IsLowerThanRange( float value );
	bool IsHigherThanRange( float value );
	bool IsOnRange( float value );
	bool IsOverlapping( FloatRange range );
	void SetFromText( char const* text );

public:
	float m_min = 0.f;
	float m_max = 0.f;
};