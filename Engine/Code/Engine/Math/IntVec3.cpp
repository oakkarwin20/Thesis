#include "Engine/Math/IntVec3.hpp"

#include <math.h>

IntVec3 IntVec3::ZERO			= IntVec3(  0,  0,  0 );
IntVec3 IntVec3::ONE			= IntVec3(  1,  1,  1 );
IntVec3 IntVec3::NEGATIVE_ONE	= IntVec3( -1, -1, -1 );

//----------------------------------------------------------------------------------------------------------------------
IntVec3::IntVec3()
{
}

//----------------------------------------------------------------------------------------------------------------------
IntVec3::~IntVec3()
{
}

//----------------------------------------------------------------------------------------------------------------------
IntVec3::IntVec3( IntVec3 const& copyFrom )
	: x( copyFrom.x )
	, y( copyFrom.y )
	, z( copyFrom.z )
{
}

//----------------------------------------------------------------------------------------------------------------------
IntVec3::IntVec3( int initialX, int initialY, int initialZ )
	: x( initialX )
	, y( initialY )
	, z( initialZ )
{
}

//----------------------------------------------------------------------------------------------------------------------
void IntVec3::operator=( IntVec3 const& copyFrom )
{
	x = copyFrom.x;
	y = copyFrom.y;
	z = copyFrom.z;
}

//----------------------------------------------------------------------------------------------------------------------
void IntVec3::operator-=( const IntVec3& vecToSubtract )
{
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
	z -= vecToSubtract.z;
}

//----------------------------------------------------------------------------------------------------------------------
const IntVec3 IntVec3::operator-( const IntVec3& vecToSubtract ) const
{
	IntVec3 intVec3;
	intVec3 = IntVec3( x - vecToSubtract.x, y - vecToSubtract.y, z - vecToSubtract.z );
	return intVec3;
}

//----------------------------------------------------------------------------------------------------------------------
const IntVec3 IntVec3::operator+( const IntVec3& vecToAdd ) const
{
	IntVec3 intVec3;
	intVec3 = IntVec3( x + vecToAdd.x, y + vecToAdd.y, z + vecToAdd.z );
	return intVec3;
}

//----------------------------------------------------------------------------------------------------------------------
bool IntVec3::operator==( const IntVec3& compare ) const
{
	return ( ( x == compare.x ) && ( y == compare.y ) && ( z == compare.z ) );
}

//----------------------------------------------------------------------------------------------------------------------
bool IntVec3::operator!=( const IntVec3& compare ) const
{
	return ( ( x != compare.x ) && ( y != compare.y ) && ( z != compare.z ) );
}

//----------------------------------------------------------------------------------------------------------------------
void IntVec3::operator+=( IntVec3& vecToAdd )
{
	x += vecToAdd.x;
	y += vecToAdd.y;
	z += vecToAdd.z;
}