#pragma once

//----------------------------------------------------------------------------------------------------------------------
struct Vec4 
{
public:
	explicit Vec4( float initialX, float initialY, float initialZ, float initialW );		// explicit constructor (from x, y, z, w)
	
	float x = 0.f;
	float y = 0.f;
	float z = 0.f;
	float w = 0.f;

	Vec4();

/*
	explicit Vec3( Vec2 initialXY );
	explicit Vec3( float initialX, float initialY );
public:
	float		GetLength()const;
	float		GetLengthXY()const;
	float		GetLengthSquared()const;
	float		GetLengthXYSquared()const;
	float		GetAngleAboutZRadians()const;
	float		GetAngleAboutZDegrees()const;
	Vec3 const  GetRotatedAboutZRadians( float deltaRadians)const;
	Vec3 const  GetRotatedAboutZDegrees( float deltaDegrees)const;
	Vec3 const  GetClamped( float MaxLength )const;
	Vec3 const  GetNormalized()const;
*/

	// Operators (const)
	bool		operator==(Vec4 const& compare) const;		// vec4 == vec4
	bool		operator!=(Vec4 const& compare) const;		// vec4 != vec4
	Vec4 const	operator+(Vec4 const& vecToAdd) const;		// vec4 +  vec4
	Vec4 const  operator-(Vec4 const& vecToSubtract) const;	// vec4 -  vec4
	Vec4 const	operator*(float uniformScale) const;		// vec4 * float
	Vec4 const	operator/(float inverseScale) const;		// vec4 / float

	// Operators (self-mutating / non-const)
	void		operator+=(Vec4 const& vecToAdd);			// vec4 += vec4
	void		operator-=(Vec4 const& vecToSubtract);		// vec4 -= vec4
	void		operator*=(float uniformScale);				// vec4 *= float
	void		operator/=(float uniformDivisor);			// vec4 /= float
	void		operator=(Vec4 const& copyFrom);			// vec4  = vec4

/*
//	void		RotateAboutZDegrees(float DeltaDegrees);
//	void		RotateAboutZRadians(float DeltaRadians);
	
	// Standalone "friend" functions that are conceptually, but not actually, part of Vec3::
	friend Vec3 const operator*(float uniformScale, Vec3 const& vecToScale);	// float * vec3

private:
	
*/

};
