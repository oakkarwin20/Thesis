#pragma once

//----------------------------------------------------------------------------------------------------------------------
struct Vec3;
struct Mat44;

//----------------------------------------------------------------------------------------------------------------------
struct EulerAngles
{
public:
	EulerAngles() = default;
	EulerAngles( float yawDegrees, float pitchDegrees, float rollDegrees );
	void				GetAsVectors_XFwd_YLeft_ZUp( Vec3& out_fowardIBasis, Vec3& out_leftJBasis, Vec3& out_KBasis ) const;
	Mat44				GetAsMatrix_XFwd_YLeft_ZUp() const;
	Vec3				GetForwardDir_XFwd_YLeft_ZUp() const;
	static EulerAngles	GetAsEuler_XFwd_YLeft_ZUp( Vec3 const& forwardIBasis, Vec3 const& leftJBasis );
	static EulerAngles	GetAsEuler_XFwd_YLeft_ZUp( Vec3 const& forwardIBasis );
	static EulerAngles	GetRelativeEulerAngles( EulerAngles const& childEuler, EulerAngles const& parentEuler );
	
	void		SetFromText( char const* text );

public:
	float		m_yawDegrees	=  0.0f;
	float		m_pitchDegrees	=  0.0f; 
	float		m_rollDegrees	=  0.0f;
};