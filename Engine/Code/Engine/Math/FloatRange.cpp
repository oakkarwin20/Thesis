#include "Engine/Math/FloatRange.hpp"
#include "Engine/Core/StringUtils.hpp"

//----------------------------------------------------------------------------------------------------------------------
FloatRange::FloatRange()
{
}

//----------------------------------------------------------------------------------------------------------------------
FloatRange::FloatRange( float min, float max )
{
	m_min = min;
	m_max = max;
}

//----------------------------------------------------------------------------------------------------------------------
FloatRange::~FloatRange()
{
}

//----------------------------------------------------------------------------------------------------------------------
bool FloatRange::IsLowerThanRange( float valueInclusive )
{
	if ( valueInclusive <= m_min )
	{
		return true;
	}
	return false;
}

//----------------------------------------------------------------------------------------------------------------------
bool FloatRange::IsHigherThanRange( float valueInclusive )
{
	if ( valueInclusive >= m_max )
	{
		return true;
	}
	return false;
}

//----------------------------------------------------------------------------------------------------------------------
bool FloatRange::IsOnRange( float value )
{
	if ( value >= m_min && value <= m_max )
	{
		return true;
	}
	return false;
}

//----------------------------------------------------------------------------------------------------------------------
bool FloatRange::IsOverlapping( FloatRange range )
{
//	if ( IsOnRange( range.m_min ) || IsOnRange( range.m_max ) || range.IsOnRange( m_min ) || range.IsOnRange( m_max ) )
	if ( IsOnRange( range.m_min ) || IsOnRange( range.m_max ) || range.IsOnRange( m_min ) || range.IsOnRange( m_max ) )
	{
		return true;
	}
	return false;
}

//----------------------------------------------------------------------------------------------------------------------
void FloatRange::SetFromText( char const* text )
{
	Strings string;
	string = SplitStringOnDelimiter( text, '~' );
	m_min = static_cast<float>( atof( ( string[0].c_str() ) ) );
	m_max = static_cast<float>( atof( ( string[1].c_str() ) ) );
}
