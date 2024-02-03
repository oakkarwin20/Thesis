#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"

#include <math.h>

//----------------------------------------------------------------------------------------------------------------------
Vec3 Vec3::ZERO			= Vec3(	 0.0f,  0.0f,  0.0f );
Vec3 Vec3::NEGATIVE_ONE = Vec3( -1.0f, -1.0f, -1.0f );
Vec3 Vec3::NEGATIVE_Z	= Vec3(  0.0f,  0.0f, -1.0f );
Vec3 Vec3::X_FWD		= Vec3(  1.0f,  0.0f,  0.0f );
Vec3 Vec3::Y_LEFT		= Vec3(  0.0f,  1.0f,  0.0f );
Vec3 Vec3::Z_UP			= Vec3(  0.0f,  0.0f,  1.0f );

//----------------------------------------------------------------------------------------------------------------------
Vec3::Vec3()
{
}

Vec3::Vec3( Vec2 initialXY ) 
	: x( initialXY.x )
	, y ( initialXY.y )
	, z ( 0.0f )
{
}

//----------------------------------------------------------------------------------------------------------------------
Vec3::Vec3( float initialX, float initialY )
	: x ( initialX )
	, y ( initialY )
	, z ( 0.0f )
{
}

//----------------------------------------------------------------------------------------------------------------------
Vec3 const Vec3::MakeFromSphericalRadians( float yawRadians, float pitchRadians, float length )
{
	Vec3 vectorInCartesianCoords;
	vectorInCartesianCoords.x =  cosf( yawRadians ) * cosf( pitchRadians );
	vectorInCartesianCoords.y =  sinf( yawRadians ) * cosf( pitchRadians );
	vectorInCartesianCoords.z = -sinf( pitchRadians );
	vectorInCartesianCoords  *= length;
	return vectorInCartesianCoords;
}

//----------------------------------------------------------------------------------------------------------------------
Vec3 const Vec3::MakeFromSphericalDegrees( float yawDegrees, float pitchDegrees, float length )
{
	float pitchRadians  = ConvertDegreesToRadians( pitchDegrees );
	float yawRadians = ConvertDegreesToRadians( yawDegrees );

	Vec3 vectorInCartesianCoords;
	vectorInCartesianCoords = MakeFromSphericalRadians( yawRadians, pitchRadians, length );
	
	return vectorInCartesianCoords;
}

//----------------------------------------------------------------------------------------------------------------------
void Vec3::SetFromText( char const* text )
{
	Strings string;
	string = SplitStringOnDelimiter( text, ',' );
	x = static_cast<float>( atof( ( string[0].c_str() ) ) );
	y = static_cast<float>( atof( ( string[1].c_str() ) ) );
	z = static_cast<float>( atof( ( string[2].c_str() ) ) );
}

//----------------------------------------------------------------------------------------------------------------------
float Vec3::GetLength() const
{
	return sqrtf((x * x) + (y * y) + (z * z));
}

//----------------------------------------------------------------------------------------------------------------------
float Vec3::GetLengthXY() const
{
	return sqrtf((x * x) + (y * y));
}

//----------------------------------------------------------------------------------------------------------------------
float Vec3::GetLengthSquared() const
{
	return ((x * x) + (y * y) + (z * z));
}

//----------------------------------------------------------------------------------------------------------------------
float Vec3::GetLengthXYSquared() const
{
	return ((x * x) + (y * y));
}

//----------------------------------------------------------------------------------------------------------------------
float Vec3::GetAngleAboutZRadians() const
{
	return atan2f(y, x);
}

//----------------------------------------------------------------------------------------------------------------------
float Vec3::GetAngleAboutZDegrees() const
{
	float angleInRadians = GetAngleAboutZRadians();
	float angleInDegrees = ConvertRadiansToDegrees(angleInRadians);
	return angleInDegrees;
}	

//----------------------------------------------------------------------------------------------------------------------
Vec3 const Vec3::GetRotatedAboutZRadians( float deltaRadians ) const
{
	// Get orientation
	float angle = atan2f( y, x );

	// Add rotate 
	float newAngle = angle + deltaRadians;

	// Get length
	float radius = sqrtf( ( x * x ) + ( y * y ) );
	
	float newX			= radius * cosf( newAngle );
	float newY			= radius * sinf( newAngle );
	Vec3 rotatedVector	= Vec3( newX, newY, z );
//	rotatedVector		= rotatedVector.GetNormalized();
	return rotatedVector;
}

//----------------------------------------------------------------------------------------------------------------------
Vec3 const Vec3::GetRotatedAboutZDegrees(float deltaDegrees) const
{
	float angleinRadians		= ConvertDegreesToRadians( deltaDegrees   );
	Vec3  rotatedAboutZDegrees	= GetRotatedAboutZRadians( angleinRadians );
	return rotatedAboutZDegrees;
}

//----------------------------------------------------------------------------------------------------------------------
Vec3 const Vec3::GetClamped(float maxLength) const
{
	float length = GetLength();

	if (length > maxLength)
	{
		float normalize = ( maxLength / length );
		return Vec3( normalize * x, normalize * y, normalize * z);
	}
	else
	{
		return Vec3( x, y, z);
	}
}


//----------------------------------------------------------------------------------------------------------------------
void Vec3::Normalize()
{
	 Vec3 currentVec3 = this->GetNormalized();
	 x = currentVec3.x;
	 y = currentVec3.y;
	 z = currentVec3.z;
}


//----------------------------------------------------------------------------------------------------------------------
Vec3 const Vec3::GetNormalized() const
{
	float length	= GetLength();
	if ( length == 0.0f )
	{
		return Vec3::ZERO;
	}
	float normalize = 1.0f / length;
	return Vec3( normalize * x, normalize * y, normalize * z );
}

//----------------------------------------------------------------------------------------------------------------------
void Vec3::SetLength(float newLength)
{
	float oldLength = GetLength();
	if ( oldLength == 0.0f )
	{
		oldLength = 1.0f;
	}
	float scale = newLength / oldLength;

	x *= scale;
	y *= scale;
	z *= scale;
}

//----------------------------------------------------------------------------------------------------------------------
Vec3 const Vec3::GetReflected( Vec3 const& bounceSurfaceNormal ) const
{
//	Vec2 projectedOntoNormal = GetProjectedOnto2D( *this, bounceSurfaceNormal );
	//	Vec2 projectedOntoTangent	= *this - projectedOntoNormal;
	//	Vec2 projectedReflected		= ( -projectedOntoNormal ) + ( projectedOntoTangent );
	//	return projectedReflected;

//	Vec2 wallToReflectOff = bounceSurfaceNormal.GetRotated90Degrees();
//
//	// calculate normal a vector along the wall and another vector facing south
//	float distOfProjectedVecOnFowardNormal = DotProduct2D( *this, bounceSurfaceNormal );					// is negative at this point in calculations
//	float distOfProjectedVecOnWallNormal = DotProduct2D( *this, wallToReflectOff );
//
//	Vec2 projectedVecOnFoward = distOfProjectedVecOnFowardNormal * bounceSurfaceNormal;
//	Vec2 projectedVecOnWall = wallToReflectOff * distOfProjectedVecOnWallNormal;
//	Vec2 reflectedVector = projectedVecOnWall - projectedVecOnFoward;
//	return reflectedVector;

	UNUSED( bounceSurfaceNormal );
	Vec3 reflectedVec = Vec3::ZERO;
	return reflectedVec;
}
//----------------------------------------------------------------------------------------------------------------------
bool Vec3::operator==(Vec3 const& compare) const
{
	return (( x == compare.x ) && ( y == compare.y ) && ( z == compare.z ));
}

//----------------------------------------------------------------------------------------------------------------------
bool Vec3::operator!=(Vec3 const& compare) const
{
	return (( x != compare.x ) || ( y != compare.y ) || ( z != compare.z ));
}

//----------------------------------------------------------------------------------------------------------------------
const Vec3 Vec3::operator+(Vec3 const& vecToAdd) const
{
	float newX = x + vecToAdd.x;
	float newY = y + vecToAdd.y;
	float newZ = z + vecToAdd.z;
	
	return Vec3( newX, newY, newZ );
}

//----------------------------------------------------------------------------------------------------------------------
const Vec3 Vec3::operator-(Vec3 const& vecToSubtract) const
{
	float newX = x - vecToSubtract.x;
	float newY = y - vecToSubtract.y;
	float newZ = z - vecToSubtract.z;

	return Vec3( newX, newY, newZ );
}

//----------------------------------------------------------------------------------------------------------------------
Vec3 const Vec3::operator-() const
{
	return Vec3( -x, -y, -z );
}

//----------------------------------------------------------------------------------------------------------------------
const Vec3 Vec3::operator*(float uniformScale) const
{
	float newX = x * uniformScale;
	float newY = y * uniformScale;
	float newZ = z * uniformScale;

	return Vec3( newX, newY, newZ );
}

//----------------------------------------------------------------------------------------------------------------------
// Vec3 const Vec3::operator*( Vec3 const& vecToMultiply ) const
// {
// 	return Vec3( x * vecToMultiply.x, x * vecToMultiply.y, z * vecToMultiply.z );
// }

//----------------------------------------------------------------------------------------------------------------------
const Vec3 Vec3::operator/(float inverseScale) const // double check this
{
	float newX = x / inverseScale;
	float newY = y / inverseScale;
	float newZ = z / inverseScale;

	return Vec3( newX, newY, newZ );
}

//----------------------------------------------------------------------------------------------------------------------
void Vec3::operator+=(Vec3 const& vecToAdd)
{
	x += vecToAdd.x;
	y += vecToAdd.y;
	z += vecToAdd.z;
}

//----------------------------------------------------------------------------------------------------------------------
void Vec3::operator-=(Vec3 const& vecToSubtract)
{
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
	z -= vecToSubtract.z;
}

//----------------------------------------------------------------------------------------------------------------------
void Vec3::operator*=(float uniformScale)
{
	x *= uniformScale;
	y *= uniformScale;
	z *= uniformScale;
}

//----------------------------------------------------------------------------------------------------------------------
void Vec3::operator/=(float uniformDivisor)
{
	// divide yourself by uniform scale
	// - operator which divides by uniform scale
	
	float scale = 1.0f / uniformDivisor;
	x *= scale;
	y *= scale;
	z *= scale;
}

//----------------------------------------------------------------------------------------------------------------------
void Vec3::operator=(Vec3 const& copyFrom)
{
	x = copyFrom.x;
	y = copyFrom.y;
	z = copyFrom.z;
}

//----------------------------------------------------------------------------------------------------------------------
//void Vec3::RotateAboutZDegrees(float DeltaDegrees)
//{
//	float Radians = ConvertDegreesToRadians(DeltaDegrees);
//	
//	Vec3();
//}
//
//void Vec3::RotateAboutZRadians(float DeltaRadians)
//{
//	Vec3();
//
//}

//----------------------------------------------------------------------------------------------------------------------
Vec3::Vec3(float initialX, float initialY, float initialZ)
{
	x = initialX;
	y = initialY;
	z = initialZ;
}

//----------------------------------------------------------------------------------------------------------------------
Vec3 const operator*(float uniformScale, Vec3 const& vecToScale)
{
	return Vec3(uniformScale * vecToScale.x, uniformScale * vecToScale.y, uniformScale * vecToScale.z);
}
