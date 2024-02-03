#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/StringUtils.hpp"

#include <math.h>

IntVec2 IntVec2::ZERO			= IntVec2(  0,  0 );
IntVec2 IntVec2::ONE			= IntVec2(  1,  1 );
IntVec2 IntVec2::NEGATIVE_ONE	= IntVec2( -1, -1 );

//----------------------------------------------------------------------------------------------------------------------
IntVec2::IntVec2()
{
}

//----------------------------------------------------------------------------------------------------------------------
IntVec2::~IntVec2()
{
}

//----------------------------------------------------------------------------------------------------------------------
IntVec2::IntVec2(IntVec2 const& copyFrom)
	: x( copyFrom.x ) 
	, y( copyFrom.y )
{
}

//----------------------------------------------------------------------------------------------------------------------
IntVec2::IntVec2(int initialX, int initialY)
	: x( initialX )
	, y( initialY )
{
}

float IntVec2::GetLength() const
{
	return sqrtf( (float( x * x) + ( y * y )));
}

int IntVec2::GetTaxicabLength() const
{
	return abs(x) + abs(y);
}

int IntVec2::GetLengthSquared() const
{
	return (( x * x ) + ( y * y ));
}
 
float IntVec2::GetOrientationRadians() const
{
	return atan2f( static_cast<float>( y ), static_cast<float>( x ) );
}

float IntVec2::GetOrientationDegrees() const
{
	return Atan2Degrees( static_cast<float>( y ), static_cast<float>( x ) );
}

IntVec2 const IntVec2::GetRotated90Degrees() const
{
	return IntVec2( -y, x );
}

IntVec2 const IntVec2::GetRotatedMinus90Degrees() const
{
	return IntVec2( y, -x );
}

void IntVec2::Rotate90Degrees()
{
	int newX = -y;
	int newY = x;

	x = newX;
	y = newY;
}

void IntVec2::RotateMinus90Degrees()
{
	int newX = y;
	int newY = -x;

	x = newX;
	y = newY;
}

void IntVec2::operator=(IntVec2 const& copyFrom)
{
	x = copyFrom.x;
	y = copyFrom.y;
}

//-----------------------------------------------------------------------------------------------
void IntVec2::operator-=( const IntVec2& vecToSubtract )
{
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
}

//----------------------------------------------------------------------------------------------------------------------
bool IntVec2::operator==( const IntVec2& compare ) const
{
	return ( ( x == compare.x ) && ( y == compare.y ) );
}

//----------------------------------------------------------------------------------------------------------------------
bool IntVec2::operator!=( const IntVec2& compare ) const
{
	return ( ( x != compare.x ) || ( y != compare.y ) );
}

//----------------------------------------------------------------------------------------------------------------------
IntVec2 const IntVec2::operator+( const IntVec2& intVecToAdd ) const
{
	return IntVec2( x + intVecToAdd.x, y + intVecToAdd.y );
}

//-----------------------------------------------------------------------------------------------
const IntVec2 IntVec2::operator-( const IntVec2& vecToSubtract ) const
{
	return IntVec2( x - vecToSubtract.x, y - vecToSubtract.y );
}

//----------------------------------------------------------------------------------------------------------------------
void IntVec2::SetFromText( char const* text )
{
	Strings string;
	string = SplitStringOnDelimiter( text, ',' );
	x = static_cast<int>( atof( ( string[0].c_str() ) ) );
	y = static_cast<int>( atof( ( string[1].c_str() ) ) );
}
