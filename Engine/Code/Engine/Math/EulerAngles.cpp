#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"


//----------------------------------------------------------------------------------------------------------------------
EulerAngles::EulerAngles( float yawDegrees, float pitchDegrees, float rollDegrees )
{
	m_yawDegrees	= yawDegrees;
	m_pitchDegrees	= pitchDegrees;
	m_rollDegrees	= rollDegrees;
}


//----------------------------------------------------------------------------------------------------------------------
void EulerAngles::GetAsVectors_XFwd_YLeft_ZUp( Vec3& out_fowardIBasis, Vec3& out_leftJBasis, Vec3& out_KBasis ) const
{
//	just return the I, J, K instead of the whole matrix
	Mat44 matrix;
	matrix				= GetAsMatrix_XFwd_YLeft_ZUp();
	out_fowardIBasis	= matrix.GetIBasis3D();
	out_leftJBasis		= matrix.GetJBasis3D();
	out_KBasis			= matrix.GetKBasis3D();
}


//----------------------------------------------------------------------------------------------------------------------
Mat44 EulerAngles::GetAsMatrix_XFwd_YLeft_ZUp() const
{
	// return whole matrix after calculations
	// yaw, pitch, then roll
	
	//----------------------------------------------------------------------------------------------------------------------
	// commenting out code to use optimized version below
	// both chunks of code does the same thing
	Mat44 matrix;
//	matrix.AppendZRotation( m_yawDegrees );
//	matrix.AppendYRotation( m_pitchDegrees );
//	matrix.AppendXRotation( m_rollDegrees  );
//	return matrix;

	//----------------------------------------------------------------------------------------------------------------------
	// Optimized version
	Vec3 iBasis;
	Vec3 jBasis;
	Vec3 kBasis;

	float Cy = CosDegrees( m_yawDegrees   );
	float Cp = CosDegrees( m_pitchDegrees );
	float Cr = CosDegrees( m_rollDegrees  );
	float Sy = SinDegrees( m_yawDegrees	  );
	float Sp = SinDegrees( m_pitchDegrees );
	float Sr = SinDegrees( m_rollDegrees  );
	
	iBasis.x = (  Cy * Cp ); 
	iBasis.y = (  Sy * Cp );
	iBasis.z = ( -Sp );

	jBasis.x = ( -Sy * Cr ) + ( Cy * Sp * Sr );
	jBasis.y = (  Cy * Cr ) + ( Sy * Sp * Sr );
	jBasis.z = (  Cp * Sr );

	kBasis.x = (  Sy * Sr ) + ( Cy * Sp * Cr );
	kBasis.y = ( -Cy * Sr ) + ( Sy * Sp * Cr );
	kBasis.z = (  Cp * Cr );

	matrix.SetIJK3D( iBasis, jBasis, kBasis );

	return matrix;
}


//----------------------------------------------------------------------------------------------------------------------
Vec3 EulerAngles::GetForwardDir_XFwd_YLeft_ZUp() const
{
	float cy = CosDegrees( m_yawDegrees );
	float cp = CosDegrees( m_pitchDegrees );
	float sy = SinDegrees( m_yawDegrees );
	float sp = SinDegrees( m_pitchDegrees );

	return Vec3( cy * cp, sy * cp, -sp );
}


//----------------------------------------------------------------------------------------------------------------------
EulerAngles EulerAngles::GetAsEuler_XFwd_YLeft_ZUp( Vec3 const& forwardIBasis, Vec3 const& leftJBasis )
{
	EulerAngles newEulerAngles( 0.0f, 0.0f, 0.0f );
	float sign = 1.0f;
	if ( forwardIBasis.x < 0.0f )
	{
		sign = -1.0f;
	}
	float fwdLengthAlongAxisXY	= sqrtf( ( forwardIBasis.x * forwardIBasis.x ) + ( forwardIBasis.y * forwardIBasis.y ) );
	fwdLengthAlongAxisXY		*= sign;
	bool  isFacingWorldZ		= CompareIfFloatsAreEqual( fwdLengthAlongAxisXY, 0.0f, 0.0001f );
	if ( isFacingWorldZ )
	{
		// Handle Gimble lock edge case
		// This issue occurs when our "fwdDir" is facing world -Z or Z+
		// Since Y is around the "world" Z axis and not the "kBasis", this issue occurs.
		// When this happens, we need to compute EulerAngles from our dirVectors differently
		float yawDegrees	= 0.0f;
//		float pitchDegrees	= ArcSineDegrees( -forwardIBasis.z );
		float pitchDegrees	= Atan2Degrees( -forwardIBasis.z, fwdLengthAlongAxisXY );
		Vec3 upKbasis		= CrossProduct3D( forwardIBasis, leftJBasis );
		upKbasis.Normalize();
		float rollDegrees	= Atan2Degrees( -upKbasis.y, leftJBasis.y );
		newEulerAngles		= EulerAngles( yawDegrees, pitchDegrees, rollDegrees );
	}
	else
	{
		float yawDegrees	= Atan2Degrees( forwardIBasis.y, forwardIBasis.x );
		float pitchDegrees	= Atan2Degrees( -forwardIBasis.z, fwdLengthAlongAxisXY );
		Vec3 upKbasis		= CrossProduct3D( forwardIBasis, leftJBasis );
		upKbasis.Normalize();
		float rollDegrees	= Atan2Degrees( leftJBasis.z, upKbasis.z );

		if ( sign < 0.0f )
		{
			if ( yawDegrees >= 90.0f )
			{
				yawDegrees -= 180.0f;
			}
			else if ( yawDegrees <= -90.0f )
			{
				yawDegrees += 180.0f;
			}
			else if ( pitchDegrees >= 90.0f )
			{
				pitchDegrees -= 180.0f;
			}
			else if ( pitchDegrees <= -90.0f )
			{
				pitchDegrees += 180.0f;
			}
			if ( rollDegrees >= 90.0f )
			{
				rollDegrees -= 180.0f;
			}
			else if ( rollDegrees <= -90.0f )
			{
				rollDegrees += 180.0f;
			}
		}
		newEulerAngles = EulerAngles( yawDegrees, pitchDegrees, rollDegrees );
	}
	return newEulerAngles;
}



//----------------------------------------------------------------------------------------------------------------------
EulerAngles EulerAngles::GetAsEuler_XFwd_YLeft_ZUp( Vec3 const& forwardIBasis )
{
	EulerAngles newEulerAngles( 0.0f, 0.0f, 0.0f );
	float sign = 1.0f;
	if ( forwardIBasis.x < 0.0f )
	{
		sign = -1.0f;
	}
	float fwdLengthAlongAxisXY	= sqrtf( ( forwardIBasis.x * forwardIBasis.x ) + ( forwardIBasis.y * forwardIBasis.y ) );
	fwdLengthAlongAxisXY		*= sign;
	bool  isFacingWorldZ		= CompareIfFloatsAreEqual( fwdLengthAlongAxisXY, 0.0f, 0.0001f );
	if ( isFacingWorldZ )
	{
		// Handle Gimble lock edge case
		// This issue occurs when our "fwdDir" is facing world -Z or Z+
		// Since Y is around the "world" Z axis and not the "kBasis", this issue occurs.
		// When this happens, we need to compute EulerAngles from our dirVectors differently
		float yawDegrees	= 0.0f;
		float pitchDegrees	= Atan2Degrees( -forwardIBasis.z, fwdLengthAlongAxisXY );
		float rollDegrees	= 0.0f;
		newEulerAngles		= EulerAngles( yawDegrees, pitchDegrees, rollDegrees );
	}
	else
	{
		float yawDegrees	= Atan2Degrees( forwardIBasis.y, forwardIBasis.x );
		float pitchDegrees	= Atan2Degrees( -forwardIBasis.z, fwdLengthAlongAxisXY );
		float rollDegrees	= 0.0f;

		if ( sign < 0.0f )
		{
			if ( yawDegrees >= 90.0f )
			{
				yawDegrees -= 180.0f;
			}
			else if ( yawDegrees <= -90.0f )
			{
				yawDegrees += 180.0f;
			}
			else if ( pitchDegrees >= 90.0f )
			{
				pitchDegrees -= 180.0f;
			}
			else if ( pitchDegrees <= -90.0f )
			{
				pitchDegrees += 180.0f;
			}
// 			if ( rollDegrees >= 90.0f )
// 			{
// 				rollDegrees -= 180.0f;
// 			}
// 			else if ( rollDegrees <= -90.0f )
// 			{
// 				rollDegrees += 180.0f;
// 			}
		}
		newEulerAngles = EulerAngles( yawDegrees, pitchDegrees, rollDegrees );
	}
	return newEulerAngles;
}


//----------------------------------------------------------------------------------------------------------------------
//'referenceEuler' is identity
//'guestEuler' is what we want to "project" onto the 'referenceEuler'
//----------------------------------------------------------------------------------------------------------------------
EulerAngles EulerAngles::GetRelativeEulerAngles( EulerAngles const& guestEuler, EulerAngles const& referenceEuler )
{
	EulerAngles relativeEuler;
	relativeEuler.m_yawDegrees	 = guestEuler.m_yawDegrees	 - referenceEuler.m_yawDegrees;
	relativeEuler.m_pitchDegrees = guestEuler.m_pitchDegrees - referenceEuler.m_pitchDegrees;
	relativeEuler.m_rollDegrees	 = guestEuler.m_rollDegrees	 - referenceEuler.m_rollDegrees;
	return relativeEuler;
}


//----------------------------------------------------------------------------------------------------------------------
void EulerAngles::SetFromText( char const* text )
{
	Strings string;
	string = SplitStringOnDelimiter( text, ',' );
	m_yawDegrees	= static_cast<float>( atof( ( string[0].c_str() ) ) );
	m_pitchDegrees	= static_cast<float>( atof( ( string[1].c_str() ) ) );
	m_rollDegrees	= static_cast<float>( atof( ( string[2].c_str() ) ) );
}
