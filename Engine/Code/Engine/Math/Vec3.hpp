#pragma once

//----------------------------------------------------------------------------------------------------------------------
struct Vec2;

//----------------------------------------------------------------------------------------------------------------------
struct Vec3 
{
public:
	float x = 0.f;
	float y = 0.f;
	float z = 0.f;

	Vec3();
	explicit Vec3(float initialX, float initialY, float initialZ);		// explicit constructor (from x, y, z)
	explicit Vec3( Vec2 initialXY );
	explicit Vec3( float initialX, float initialY );

	// Static Methods
	static Vec3 const MakeFromSphericalRadians( float yawRadians, float pitchRadians, float length = 1.0f );
	static Vec3 const MakeFromSphericalDegrees( float yawDegrees, float pitchDegrees, float length = 1.0f );

	void		SetFromText( char const* text );

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
	void		Normalize();
	Vec3 const  GetNormalized()const;
	void		SetLength( float newLength );
	Vec3 const  GetReflected( Vec3 const& bounceSurfaceNormal ) const;


	// Operators (const)
	bool		operator==(Vec3 const& compare) const;		// vec3 == vec3
	bool		operator!=(Vec3 const& compare) const;		// vec3 != vec3
	Vec3 const	operator+(Vec3 const& vecToAdd) const;		// vec3 + vec3
	Vec3 const  operator-(Vec3 const& vecToSubtract) const;	// vec3 - vec3
	Vec3 const  operator-() const;								// -vec2, i.e. "unary negation"
	Vec3 const	operator*(float uniformScale) const;		// vec3 * float
//	Vec3 const  operator*( Vec3 const& vecToMultiply ) const;	// vec3 * vec3
	Vec3 const	operator/(float inverseScale) const;		// vec3 / float

	// Operators (self-mutating / non-const)
	void		operator+=(Vec3 const& vecToAdd);			// vec3 += vec3
	void		operator-=(Vec3 const& vecToSubtract);		// vec3 -= vec3
	void		operator*=(float uniformScale);				// vec3 *= float
	void		operator/=(float uniformDivisor);			// vec3 /= float
	void		operator=(Vec3 const& copyFrom);			// vec3 = vec3

	/*
	void		RotateAboutZDegrees(float DeltaDegrees);
	void		RotateAboutZRadians(float DeltaRadians);
	*/

	// Standalone "friend" functions that are conceptually, but not actually, part of Vec3::
	friend Vec3 const operator*(float uniformScale, Vec3 const& vecToScale);	// float * vec3

	static Vec3 ZERO;
	static Vec3 NEGATIVE_ONE;
	static Vec3 NEGATIVE_Z;
	static Vec3 X_FWD;
	static Vec3 Y_LEFT;
	static Vec3 Z_UP;
};
