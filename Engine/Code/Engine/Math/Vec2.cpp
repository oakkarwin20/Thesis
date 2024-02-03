#include "Engine/Math/Vec2.hpp"
#include <math.h>
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
//#include "Engine/Core/EngineCommon.hpp"

Vec2 Vec2::ZERO			= Vec2(  0.0f,  0.0f );
Vec2 Vec2::ONE			= Vec2(  1.0f,  1.0f );
Vec2 Vec2::NEGATIVE_ONE = Vec2( -1.0f, -1.0f );


//-----------------------------------------------------------------------------------------------
Vec2::Vec2( const Vec2& copy )
	: x( copy.x )
	, y( copy.y )
{
}

//-----------------------------------------------------------------------------------------------
Vec2::Vec2( float initialX, float initialY )
	: x( initialX )
	, y( initialY )
{
}

//-----------------------------------------------------------------------------------------------
Vec2::Vec2( IntVec2 const& fromIntVec2 )
	: x( (float)fromIntVec2.x )
	, y( (float)fromIntVec2.y )
{
}  

Vec2 const Vec2::MakeFromPolarRadians(float orientationRadians, float length)
{
	float x = length * cosf(orientationRadians);
	float y = length * sinf(orientationRadians);
	return Vec2(x, y);
}
	
Vec2 const Vec2::MakeFromPolarDegrees(float orientationDegrees, float length)
{
	float orientationRadians	  = ConvertDegreesToRadians( orientationDegrees );
	Vec2 vectorInCarteisianCoords = MakeFromPolarRadians( orientationRadians, length );
	return vectorInCarteisianCoords;
}

float Vec2::GetLength() const
{
	return sqrtf(( x * x ) + ( y * y ));
}

float Vec2::GetLengthSquared() const
{
	return (x * x) + (y * y);
}

float Vec2::GetOrientationRadians() const
{
	return atan2f( y, x);
}

float Vec2::GetOrientationDegrees() const
{
	float orientationInRadians = atan2f( y, x );
	float OrientationInDegrees = ConvertRadiansToDegrees(orientationInRadians);
	return OrientationInDegrees;
}

Vec2 const Vec2::GetRotated90Degrees() const
{
	return Vec2( -y, x );
}

Vec2 const Vec2::GetRotatedMinus90Degrees() const
{
	return Vec2( y, -x );
}

Vec2 const Vec2::GetRotatedRadians(float deltaRadians) const
{
	//Converts from Cartesian coordinates to Polar Coordinates
	float r = sqrtf(( x * x ) + ( y * y ));
	float thetaRadians = atan2f( y, x);

	//Rotates orientation
	thetaRadians += deltaRadians;

	//Converts back to Cartesian coordinates from Polar Coordinates
	float rotatedX = r * cosf(thetaRadians);
	float rotatedY = r * sinf(thetaRadians);
	return Vec2( rotatedX, rotatedY );
}

Vec2 const Vec2::GetRotatedDegrees(float deltaDegrees) const
{
	//Converts from Cartesian coordinates to Polar Coordinates
	float r = sqrtf(( x * x ) + ( y * y ));
	float thetaDegrees = Atan2Degrees( y, x);

	//Rotates orientation
	thetaDegrees += deltaDegrees;

	//Converts back to Cartesian coordinates from Polar Coordinates
	float rotatedX = r * CosDegrees(thetaDegrees);
	float rotatedY = r * SinDegrees(thetaDegrees);
	return Vec2( rotatedX, rotatedY );
}

Vec2 const Vec2::GetClamped(float maxLength) const  // outputs test failed if Clamp function is not written
{
	float length = GetLength();

	if (length > maxLength)
	{
		float normalize = ( maxLength / length );
		return Vec2( normalize * x, normalize * y);
	}
	else
	{
		return Vec2( x, y);
	}
}

Vec2 const Vec2::GetNormalized() const
{
	float length = GetLength();
	if ( length == 0.0f )
	{
		return Vec2::ZERO;
	}
	float normalize = 1/length;
	return Vec2( normalize * x, normalize * y );
}

void Vec2::SetOrientationRadians(float newOrientationRadians)
{
	float radius = GetLength();
	x = radius * cosf( newOrientationRadians );
	y = radius * sinf( newOrientationRadians );
}

void Vec2::SetOrientationDegrees(float newOrientationDegrees)
{
	float radius = GetLength();
	x = radius * CosDegrees( newOrientationDegrees );
	y = radius * SinDegrees( newOrientationDegrees );
}

//----------------------------------------------------------------------------------------------------------------------
void Vec2::SetPolarRadians(float newOrientationRadians, float newLength)
{
	x = newLength * cosf( newOrientationRadians );
	y = newLength * sinf( newOrientationRadians );

}

//----------------------------------------------------------------------------------------------------------------------
void Vec2::SetPolarDegrees(float newOrientationDegrees, float newLength)
{
	x = newLength * CosDegrees( newOrientationDegrees );
	y = newLength * SinDegrees( newOrientationDegrees );
}

//----------------------------------------------------------------------------------------------------------------------
void Vec2::Rotate90Degrees()
{
	float newX = -y;
	float newY = x;

	x = newX;
	y = newY;
}

//----------------------------------------------------------------------------------------------------------------------
void Vec2::RotateMinus90Degrees()
{
	float NewX = y;
	float NewY = -x;

	x = NewX;
	y = NewY;
}

//----------------------------------------------------------------------------------------------------------------------
void Vec2::RotateRadians(float DeltaRadians)
{
	//Uses atan2, gets orientation
	float angleInRadians = GetOrientationRadians();

	//Rotates orientation
	angleInRadians += DeltaRadians;
	
	//Inputs new ( x, y ) coordinates
	SetOrientationRadians(angleInRadians);

	/* previous code I wrote, keeping for ref and learning, please disregard.
	float radius = GetLength();
	float angleInRadians = GetOrientationRadians();

	angleInRadians += DeltaRadians;

	x = radius * cosf(angleInRadians);
	y = radius * sinf(angleInRadians);
	*/
}

//----------------------------------------------------------------------------------------------------------------------
void Vec2::RotateDegrees(float DeltaDegrees)
{
	float Radians =	ConvertDegreesToRadians(DeltaDegrees);
	RotateRadians(Radians);

	/* previous code I wrote, keeping for ref and learning, please disregard.
	float radius = (( x * x ) + ( y * y ));

	float newX = radius * CosDegrees(DeltaDegrees);
	float newY = radius * SinDegrees(DeltaDegrees);

	x = newX;
	y = newY;
	*/
}

//----------------------------------------------------------------------------------------------------------------------
void Vec2::SetLength(float newLength)
{
	/*float radius = sqrtf(( x * x ) + ( y * y ));
	radius = newLength;*/

	float oldLength = GetLength();
	float scale = newLength / oldLength;

	x *= scale;
	y *= scale;
}

//----------------------------------------------------------------------------------------------------------------------
void Vec2::ClampLength(float maxLength)
{
	float length = GetLength();

	if (length > maxLength)
	{
		float normalize = ( maxLength / length );
		x *= normalize;
		y *= normalize;
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Vec2::Normalize()
{
	float length = GetLength();
	if ( length == 0.0f )
	{
		x = 0.0f;
		y = 0.0f;
		return;
	}
	float normalize = ( 1 / length );

	x *= normalize;
	y *= normalize;

	/* Stuck, get help
	float radius = (( x * x ) + ( y * y ));
	//float normalizedX = x / radius;
	//float normalizedY = y / radius;

	x = x / radius;
	y = y / radius;
	*/
}

//----------------------------------------------------------------------------------------------------------------------
float Vec2::NormalizeAndGetPreviousLength()
{
	float length = GetLength();

	x = x / length;
	y = y / length;

	return length;
}

//----------------------------------------------------------------------------------------------------------------------
void Vec2::Reflect( Vec2 const& BounceSurfaceNormal )
{
	Vec2 reflectVec = GetReflected( BounceSurfaceNormal );
	this->x = reflectVec.x;
	this->y = reflectVec.y;
}
 
//----------------------------------------------------------------------------------------------------------------------
void Vec2::SetFromText( char const* text )
{
	Strings string;
	string = SplitStringOnDelimiter( text, ',' );
	x =	static_cast<float>(atof( ( string[0].c_str() ) ) );
	y = static_cast<float>(atof( ( string[1].c_str() ) ) );
}

//----------------------------------------------------------------------------------------------------------------------
Vec2 const Vec2::GetReflected( Vec2 const& bounceSurfaceNormal ) const
{
//	Vec2 projectedOntoNormal	= GetProjectedOnto2D( *this, bounceSurfaceNormal );
//	Vec2 projectedOntoTangent	= *this - projectedOntoNormal;
//	Vec2 projectedReflected		= ( -projectedOntoNormal ) + ( projectedOntoTangent );
//	return projectedReflected;

	Vec2 wallToReflectOff					= bounceSurfaceNormal.GetRotated90Degrees();

	// calculate normal a vector along the wall and another vector facing south
	float distOfProjectedVecOnFowardNormal	= DotProduct2D( *this, bounceSurfaceNormal );					// is negative at this point in calculations
	float distOfProjectedVecOnWallNormal	= DotProduct2D( *this, wallToReflectOff );

	Vec2 projectedVecOnFoward	= distOfProjectedVecOnFowardNormal * bounceSurfaceNormal;
	Vec2 projectedVecOnWall		= wallToReflectOff * distOfProjectedVecOnWallNormal;
	Vec2 reflectedVector		= projectedVecOnWall - projectedVecOnFoward;
	return reflectedVector;
}

//----------------------------------------------------------------------------------------------------------------------
Vec2 Vec2::GetFowardNormal( float orientationDegrees )
{
	Vec2 forwardNormal = Vec2( CosDegrees( orientationDegrees ), SinDegrees( orientationDegrees ) );
	return forwardNormal;
}

//-----------------------------------------------------------------------------------------------
const Vec2 Vec2::operator + ( const Vec2& vecToAdd ) const
{
	return Vec2( x + vecToAdd.x, y + vecToAdd.y);
}

//-----------------------------------------------------------------------------------------------
const Vec2 Vec2::operator-( const Vec2& vecToSubtract ) const
{
	return Vec2( x - vecToSubtract.x, y - vecToSubtract.y);
}

//------------------------------------------------------------------------------------------------
const Vec2 Vec2::operator-() const
{
	return Vec2( -x, -y );
}

//-----------------------------------------------------------------------------------------------
const Vec2 Vec2::operator*( float uniformScale ) const
{
	return Vec2( x * uniformScale, y * uniformScale);
}

//------------------------------------------------------------------------------------------------
const Vec2 Vec2::operator*( const Vec2& vecToMultiply ) const
{
	return Vec2( x * vecToMultiply.x, x * vecToMultiply.y);
}

//-----------------------------------------------------------------------------------------------
const Vec2 Vec2::operator/( float inverseScale ) const
{
	float scale = 1 / inverseScale;
	return Vec2( x * scale, y * scale);
}

//-----------------------------------------------------------------------------------------------
void Vec2::operator+=( const Vec2& vecToAdd )
{
	x += vecToAdd.x;
	y += vecToAdd.y;
}

//-----------------------------------------------------------------------------------------------
void Vec2::operator-=( const Vec2& vecToSubtract )
{
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
}

//-----------------------------------------------------------------------------------------------
void Vec2::operator*=( const float uniformScale )
{
	x *= uniformScale;
	y *= uniformScale;
}

//-----------------------------------------------------------------------------------------------
void Vec2::operator/=( const float uniformDivisor )
{
	float scale = 1 / uniformDivisor;
	x *= scale;
	y *= scale;

}

//-----------------------------------------------------------------------------------------------
void Vec2::operator=( const Vec2& copyFrom )
{
	x = copyFrom.x;
	y = copyFrom.y;
}

//-----------------------------------------------------------------------------------------------
const Vec2 operator*( float uniformScale, const Vec2& vecToScale )
{
	return Vec2(uniformScale * vecToScale.x, uniformScale * vecToScale.y);
}

//-----------------------------------------------------------------------------------------------
bool Vec2::operator==( const Vec2& compare ) const
{
	return ((x == compare.x) && (y == compare.y));
}

//-----------------------------------------------------------------------------------------------
bool Vec2::operator!=( const Vec2& compare ) const
{
	return ((x != compare.x) || (y != compare.y));;
}