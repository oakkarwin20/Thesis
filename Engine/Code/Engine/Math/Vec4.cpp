#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <math.h>

Vec4::Vec4( float initialX, float initialY, float initialZ, float initialW  )
	: x( initialX )
	, y( initialY )
	, z( initialZ )
	, w( initialW )
{
}

Vec4::Vec4()
{
}

/*
Vec3::Vec3( Vec2 initialXY ) 
	: x( initialXY.x )
	, y ( initialXY.y )
	, z ( 0.0f )
{
}

Vec3::Vec3( float initialX, float initialY )
	: x ( initialX )
	, y ( initialY )
	, z ( 0.0f )
{
}


float Vec3::GetLength() const
{
	return sqrtf((x * x) + (y * y) + (z * z));
}

float Vec3::GetLengthXY() const
{
	return sqrtf((x * x) + (y * y));
}

float Vec3::GetLengthSquared() const
{
	return ((x * x) + (y * y) + (z * z));
}

float Vec3::GetLengthXYSquared() const
{
	return ((x * x) + (y * y));
}

float Vec3::GetAngleAboutZRadians() const
{
	return atan2f(y, x);;
}

float Vec3::GetAngleAboutZDegrees() const
{
	float AngleInRadians = GetAngleAboutZRadians();
	float AngleInDegrees = ConvertRadiansToDegrees(AngleInRadians);
	return AngleInDegrees;
}	

Vec3 const Vec3::GetRotatedAboutZRadians( float deltaRadians ) const
{
	//get orientation
	float angle = atan2f( y, x );

	//add rotate 
	float newAngle = angle + deltaRadians;

	//get length
	float radius = sqrtf(( x * x ) + ( y * y ));
	
	float newX = radius * cosf(newAngle);
	float newY = radius * sinf(newAngle);

	return Vec3( newX, newY, z);
}

Vec3 const Vec3::GetRotatedAboutZDegrees(float deltaDegrees) const
{
	float angleinRadians = ConvertDegreesToRadians(deltaDegrees);
	Vec3 RotatedAboutZDegrees = GetRotatedAboutZRadians(angleinRadians);
	return RotatedAboutZDegrees;
}

Vec3 const Vec3::GetClamped(float MaxLength) const
{
	(void)MaxLength;
	return Vec3();
}

Vec3 const Vec3::GetNormalized() const
{
	return Vec3();
}
*/

bool Vec4::operator==(Vec4 const& compare) const
{
	return (( x == compare.x ) && ( y == compare.y ) && ( z == compare.z ) && ( w == compare.w ));
}

bool Vec4::operator!=(Vec4 const& compare) const
{
	return (( x != compare.x ) || ( y != compare.y ) || ( z != compare.z ) || ( w != compare.w ));
}

const Vec4 Vec4::operator+(Vec4 const& vecToAdd) const
{
	float newX = x + vecToAdd.x;
	float newY = y + vecToAdd.y;
	float newZ = z + vecToAdd.z;
	float newW = w + vecToAdd.w;
	
	return Vec4( newX, newY, newZ, newW );
}

const Vec4 Vec4::operator-(Vec4 const& vecToSubtract) const
{
	float newX = x - vecToSubtract.x;
	float newY = y - vecToSubtract.y;
	float newZ = z - vecToSubtract.z;
	float newW = w - vecToSubtract.w;

	return Vec4( newX, newY, newZ, newW );
}

const Vec4 Vec4::operator*(float uniformScale) const
{
	float newX = x * uniformScale;
	float newY = y * uniformScale;
	float newZ = z * uniformScale;
	float newW = w * uniformScale;

	return Vec4( newX, newY, newZ, newW );
}

const Vec4 Vec4::operator/(float inverseScale) const // double check this
{
	float newX = x / inverseScale;
	float newY = y / inverseScale;
	float newZ = z / inverseScale;
	float newW = w / inverseScale;

	return Vec4( newX, newY, newZ, newW );
}

void Vec4::operator+=(Vec4 const& vecToAdd)
{
	x += vecToAdd.x;
	y += vecToAdd.y;
	z += vecToAdd.z;
	w += vecToAdd.w;
}

void Vec4::operator-=(Vec4 const& vecToSubtract)
{
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
	z -= vecToSubtract.z;
	w -= vecToSubtract.w;
}

void Vec4::operator*=(float uniformScale)
{
	x *= uniformScale;
	y *= uniformScale;
	z *= uniformScale;
	w *= uniformScale;
}

void Vec4::operator/=(float uniformDivisor)
{
	// divide yourself by uniform scale
	// - operator which divides by uniform scale
	
	float scale = 1.0f / uniformDivisor;
	x *= scale;
	y *= scale;
	z *= scale;
	w *= scale;
}

void Vec4::operator=(Vec4 const& copyFrom)
{
	x = copyFrom.x;
	y = copyFrom.y;
	z = copyFrom.z;
	w = copyFrom.w;
}

//
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

/*
Vec4::Vec4(float initialX, float initialY, float initialZ, float initialW)
{
	x = initialX;
	y = initialY;
	z = initialZ;
	w = initialW;
}

Vec4 const operator*(float uniformScale, Vec4 const& vecToScale)
{
	return Vec4(uniformScale * vecToScale.x, uniformScale * vecToScale.y, uniformScale * vecToScale.z, uniformScale * vecToScale.w);
}
*/