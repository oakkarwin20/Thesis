#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec4.hpp"

#include "Engine/Core/EngineCommon.hpp"

//----------------------------------------------------------------------------------------------------------------------
Mat44::Mat44()
{
	// setting to default identity
	m_values[Ix] = 1;		m_values[Jx] = 0;		m_values[Kx] = 0;		m_values[Tx] = 0; 
	m_values[Iy] = 0;		m_values[Jy] = 1;		m_values[Ky] = 0;		m_values[Ty] = 0;
	m_values[Iz] = 0;		m_values[Jz] = 0;		m_values[Kz] = 1;		m_values[Tz] = 0;
	m_values[Iw] = 0;		m_values[Jw] = 0;		m_values[Kw] = 0;		m_values[Tw] = 1;
}

//----------------------------------------------------------------------------------------------------------------------
Mat44::Mat44( Vec2 const& iBasis2D, Vec2 const& jBasis2D, Vec2 const& translation2D )
{
	m_values[Ix] = iBasis2D.x;
	m_values[Iy] = iBasis2D.y;
	m_values[Iz] = 0;
	m_values[Iw] = 0;

	m_values[Jx] = jBasis2D.x;
	m_values[Jy] = jBasis2D.y;
	m_values[Jz] = 0;
	m_values[Jw] = 0;

	m_values[Kx] = 0;
	m_values[Ky] = 0;
	m_values[Kz] = 1;
	m_values[Kw] = 0;

	m_values[Tx] = translation2D.x;
	m_values[Ty] = translation2D.y;
	m_values[Tz] = 0;
	m_values[Tw] = 1;
}

//----------------------------------------------------------------------------------------------------------------------
Mat44::Mat44( Vec3 const& iBasis3D, Vec3 const& jBasis3D, Vec3 const& kBasis3D, Vec3 const& translation3D )
{
	m_values[Ix] = iBasis3D.x;
	m_values[Iy] = iBasis3D.y;
	m_values[Iz] = iBasis3D.z;
	m_values[Iw] = 0;

	m_values[Jx] = jBasis3D.x;
	m_values[Jy] = jBasis3D.y;
	m_values[Jz] = jBasis3D.z;
	m_values[Jw] = 0;

	m_values[Kx] = kBasis3D.x;
	m_values[Ky] = kBasis3D.y;
	m_values[Kz] = kBasis3D.z;
	m_values[Kw] = 0;

	m_values[Tx] = translation3D.x;
	m_values[Ty] = translation3D.y;
	m_values[Tz] = translation3D.z;
	m_values[Tw] = 1;
}

//----------------------------------------------------------------------------------------------------------------------
Mat44::Mat44( Vec4 const& iBasis4D, Vec4 const& jBasis4D, Vec4 const& kBasis4D, Vec4 const& translation4D )
{
	m_values[Ix] = iBasis4D.x;
	m_values[Iy] = iBasis4D.y;
	m_values[Iz] = iBasis4D.z;
	m_values[Iw] = iBasis4D.w;

	m_values[Jx] = jBasis4D.x;
	m_values[Jy] = jBasis4D.y;
	m_values[Jz] = jBasis4D.z;
	m_values[Jw] = jBasis4D.w;

	m_values[Kx] = kBasis4D.x;
	m_values[Ky] = kBasis4D.y;
	m_values[Kz] = kBasis4D.z;
	m_values[Kw] = kBasis4D.w;

	m_values[Tx] = translation4D.x;
	m_values[Ty] = translation4D.y;
	m_values[Tz] = translation4D.z;
	m_values[Tw] = translation4D.w;
}

//----------------------------------------------------------------------------------------------------------------------
Mat44::Mat44( float const* sixteenValuesBasisMajor )
{
	m_values[Ix] = sixteenValuesBasisMajor[0];
	m_values[Iy] = sixteenValuesBasisMajor[1];
	m_values[Iz] = sixteenValuesBasisMajor[2];
	m_values[Iw] = sixteenValuesBasisMajor[3];
	
	m_values[Jx] = sixteenValuesBasisMajor[4];
	m_values[Jy] = sixteenValuesBasisMajor[5];
	m_values[Jz] = sixteenValuesBasisMajor[6];
	m_values[Jw] = sixteenValuesBasisMajor[7];
	
	m_values[Kx] = sixteenValuesBasisMajor[8];
	m_values[Ky] = sixteenValuesBasisMajor[9];
	m_values[Kz] = sixteenValuesBasisMajor[10];
	m_values[Kw] = sixteenValuesBasisMajor[11];

	m_values[Tx] = sixteenValuesBasisMajor[12];
	m_values[Ty] = sixteenValuesBasisMajor[13];
	m_values[Tz] = sixteenValuesBasisMajor[14];
	m_values[Tw] = sixteenValuesBasisMajor[15];
}

//----------------------------------------------------------------------------------------------------------------------
Mat44 const Mat44::CreateTranslation2D( Vec2 const& translationXY )
{
	Mat44 matrix;
	matrix.m_values[Tx] = translationXY.x;
	matrix.m_values[Ty] = translationXY.y;
	return matrix;
}

//----------------------------------------------------------------------------------------------------------------------
Mat44 const Mat44::CreateTranslation3D( Vec3 const& translationXYZ )
{
	Mat44 matrix;
	matrix.m_values[Tx] = translationXYZ.x;
	matrix.m_values[Ty] = translationXYZ.y;
	matrix.m_values[Tz] = translationXYZ.z;
	return matrix;
}

//----------------------------------------------------------------------------------------------------------------------
Mat44 const Mat44::CreateUniformScale2D( float uniformScaleXY )
{
	Mat44 matrix;
	matrix.m_values[Ix] *= uniformScaleXY;
	matrix.m_values[Iy] *= uniformScaleXY;

	matrix.m_values[Jx] *= uniformScaleXY;
	matrix.m_values[Jy] *= uniformScaleXY;
	return matrix;
}

//----------------------------------------------------------------------------------------------------------------------
Mat44 const Mat44::CreateUniformScale3D( float uniformScaleXYZ )
{
	Mat44 matrix;
	matrix.m_values[Ix] *= uniformScaleXYZ;
	matrix.m_values[Iy] *= uniformScaleXYZ;
	matrix.m_values[Iz] *= uniformScaleXYZ;

	matrix.m_values[Jx] *= uniformScaleXYZ;
	matrix.m_values[Jy] *= uniformScaleXYZ;
	matrix.m_values[Jz] *= uniformScaleXYZ;

	matrix.m_values[Kx] *= uniformScaleXYZ;
	matrix.m_values[Ky] *= uniformScaleXYZ;
	matrix.m_values[Kz] *= uniformScaleXYZ;

	return matrix;
}

//----------------------------------------------------------------------------------------------------------------------
Mat44 const Mat44::CreateNonUniformScale2D( Vec2 const& nonUniformScaleXY )
{
	Mat44 matrix;
	matrix.m_values[Ix] *= nonUniformScaleXY.x;
	matrix.m_values[Iy] *= nonUniformScaleXY.y;

	matrix.m_values[Jx] *= nonUniformScaleXY.x;
	matrix.m_values[Jy] *= nonUniformScaleXY.y;

	return matrix;
}

//----------------------------------------------------------------------------------------------------------------------
Mat44 const Mat44::CreateNonUniformScale3D( Vec3 const& nonUniformScaleXYZ )
{
	Mat44 matrix;
	matrix.m_values[Ix] *= nonUniformScaleXYZ.x;
	matrix.m_values[Iy] *= nonUniformScaleXYZ.y;
	matrix.m_values[Iz] *= nonUniformScaleXYZ.z;
	matrix.m_values[Jx] *= nonUniformScaleXYZ.x;
	matrix.m_values[Jy] *= nonUniformScaleXYZ.y;
	matrix.m_values[Jz] *= nonUniformScaleXYZ.z;
	matrix.m_values[Kx] *= nonUniformScaleXYZ.x;
	matrix.m_values[Ky] *= nonUniformScaleXYZ.y;
	matrix.m_values[Kz] *= nonUniformScaleXYZ.z;
	return matrix;
}

//----------------------------------------------------------------------------------------------------------------------
Mat44 const Mat44::CreateXRotationDegrees( float rotationDegreesAboutX )
{
	Mat44 matrix;
	matrix.m_values[Jy] =   CosDegrees( rotationDegreesAboutX );
	matrix.m_values[Jz] =   SinDegrees( rotationDegreesAboutX );
	matrix.m_values[Ky] =  -SinDegrees( rotationDegreesAboutX );
	matrix.m_values[Kz] =   CosDegrees( rotationDegreesAboutX );
	return matrix;
}

//----------------------------------------------------------------------------------------------------------------------
Mat44 const Mat44::CreateYRotationDegrees( float rotationDegreesAboutY )
{
	Mat44 matrix;

	matrix.m_values[Ix] =  CosDegrees( rotationDegreesAboutY );
	matrix.m_values[Iz] = -SinDegrees( rotationDegreesAboutY );
	matrix.m_values[Kx] =  SinDegrees( rotationDegreesAboutY );
	matrix.m_values[Kz] =  CosDegrees( rotationDegreesAboutY );
	return matrix;
}

//----------------------------------------------------------------------------------------------------------------------
Mat44 const Mat44::CreateZRotationDegrees( float rotationDegreesAboutZ )
{
	Mat44 matrix;
	matrix.m_values[Ix] = CosDegrees( rotationDegreesAboutZ );
	matrix.m_values[Iy] = SinDegrees( rotationDegreesAboutZ );
	matrix.m_values[Jx] = -SinDegrees( rotationDegreesAboutZ );
	matrix.m_values[Jy] = CosDegrees( rotationDegreesAboutZ );
	return matrix;
}

//----------------------------------------------------------------------------------------------------------------------
Mat44 const Mat44::CreateOrthoProjection( float left, float right, float bottom, float top, float zNear, float zFar )
{
	Mat44 orthoMatrix;
	orthoMatrix.m_values[Ix] = (            (2) / (right - left));
	orthoMatrix.m_values[Tx] = ( left + right ) / (left - right);
	orthoMatrix.m_values[Jy] = (		    (2) / (top - bottom));
	orthoMatrix.m_values[Ty] = (  top + bottom) / (bottom - top);
	orthoMatrix.m_values[Kz] = (	   	    (1) / (zFar - zNear));
	orthoMatrix.m_values[Tz] = (	   (-zNear) / (zFar - zNear));
	return orthoMatrix;
}

//----------------------------------------------------------------------------------------------------------------------
// FOV means vertical aperture 
Mat44 const Mat44::CreatePerspectiveProjection( float fovYDegrees, float aspect, float zNear, float zFar )
{
	Mat44 perspectiveMatrix;
	float fovYRadians = ConvertDegreesToRadians( fovYDegrees );
	float scale		  = (1.0f / tanf(fovYRadians / 2.0f) );

	perspectiveMatrix.m_values[Jy] = CosDegrees( fovYDegrees * 0.5f ) / SinDegrees( fovYDegrees * 0.5f );	// Equals 1 if vertical field of view = 90
	perspectiveMatrix.m_values[Ix] = scale / aspect;
	perspectiveMatrix.m_values[Kz] = zFar / (zFar - zNear);	
	perspectiveMatrix.m_values[Kw] = 1.0f;								// This puts Z into the W component( int prep for the w-divide )
	perspectiveMatrix.m_values[Tz] = (zNear * zFar) / (zNear - zFar);	
	perspectiveMatrix.m_values[Tw] = 0.0f;								// Otherwise, we would be putting Z+1 (instead of Z) into the W component

	return perspectiveMatrix;
}

//----------------------------------------------------------------------------------------------------------------------
Vec2 const Mat44::TransformVectorQuantity2D( Vec2 const& vectorQuantityXY ) const
{
	Vec2 vectorQuantity2D;
	vectorQuantity2D.x = ( m_values[Ix] * vectorQuantityXY.x ) + ( m_values[Jx] * vectorQuantityXY.y );
	vectorQuantity2D.y = ( m_values[Iy] * vectorQuantityXY.x ) + ( m_values[Jy] * vectorQuantityXY.y );
	return vectorQuantity2D;
}

//----------------------------------------------------------------------------------------------------------------------
Vec3 const Mat44::TransformVectorQuantity3D( Vec3 const& vectorQuantityXYZ ) const
{
	Vec3 vectorQuantity3D;
	vectorQuantity3D.x = ( m_values[Ix] * vectorQuantityXYZ.x ) + ( m_values[Jx] * vectorQuantityXYZ.y ) + ( m_values[Kx] * vectorQuantityXYZ.z );
	vectorQuantity3D.y = ( m_values[Iy] * vectorQuantityXYZ.x ) + ( m_values[Jy] * vectorQuantityXYZ.y ) + ( m_values[Ky] * vectorQuantityXYZ.z );
	vectorQuantity3D.z = ( m_values[Iz] * vectorQuantityXYZ.x ) + ( m_values[Jz] * vectorQuantityXYZ.y ) + ( m_values[Kz] * vectorQuantityXYZ.z );
	return vectorQuantity3D;
}

//----------------------------------------------------------------------------------------------------------------------
Vec2 const Mat44::TransformPosition2D( Vec2 const& positionXY ) const
{
	Vec2 transformPos2D;
	transformPos2D.x = ( m_values[Ix] * positionXY.x ) + ( m_values[Jx] * positionXY.y ) + ( m_values[Tx] );
	transformPos2D.y = ( m_values[Iy] * positionXY.x ) + ( m_values[Jy] * positionXY.y ) + ( m_values[Ty] );
	return transformPos2D;
} 

//----------------------------------------------------------------------------------------------------------------------
Vec3 const Mat44::TransformPosition3D( Vec3 const& position3D ) const
{
	Vec3 transformPos3D;
	transformPos3D.x = ( m_values[Ix] * position3D.x ) + ( m_values[Jx] * position3D.y ) + ( m_values[Kx] * position3D.z ) + ( m_values[Tx] );
	transformPos3D.y = ( m_values[Iy] * position3D.x ) + ( m_values[Jy] * position3D.y ) + ( m_values[Ky] * position3D.z ) + ( m_values[Ty] );
	transformPos3D.z = ( m_values[Iz] * position3D.x ) + ( m_values[Jz] * position3D.y ) + ( m_values[Kz] * position3D.z ) + ( m_values[Tz] );
	return transformPos3D;
}

// #ToDo fix function, logic incorrect
//----------------------------------------------------------------------------------------------------------------------
Vec4 const Mat44::TransformHomogeneous3D( Vec4 const& position3D ) const
{
	Vec4 transformHomo3D;
	transformHomo3D.x = ( m_values[Ix] * position3D.x ) + ( m_values[Jx] * position3D.y ) + ( m_values[Kx] * position3D.z ) + ( m_values[Tx] * position3D.w );
	transformHomo3D.y = ( m_values[Iy] * position3D.x ) + ( m_values[Jy] * position3D.y ) + ( m_values[Ky] * position3D.z ) + ( m_values[Ty] * position3D.w );
	transformHomo3D.z = ( m_values[Iz] * position3D.x ) + ( m_values[Jz] * position3D.y ) + ( m_values[Kz] * position3D.z ) + ( m_values[Tz] * position3D.w );
	transformHomo3D.w = ( m_values[Iw] * position3D.x ) + ( m_values[Jw] * position3D.y ) + ( m_values[Kw] * position3D.z ) + ( m_values[Tw] * position3D.w );
	return transformHomo3D;
}

//----------------------------------------------------------------------------------------------------------------------
float* Mat44::GetAsFloatArray()
{
	return m_values;
}

//----------------------------------------------------------------------------------------------------------------------
float const* Mat44::GetAsFloatArray() const
{
	return m_values;
}

//----------------------------------------------------------------------------------------------------------------------
Vec2 const Mat44::GetIBasis2D() const
{
	Vec2 iBasis2D = Vec2(m_values[Ix], m_values[Iy] );
	return iBasis2D;
}

//----------------------------------------------------------------------------------------------------------------------
Vec2 const Mat44::GetJBasis2D() const
{
	Vec2 jBasis2D = Vec2( m_values[Jx], m_values[Jy] );
	return jBasis2D;
}

//----------------------------------------------------------------------------------------------------------------------
Vec2 const Mat44::GetTranslation2D() const
{
	Vec2 translation2D;
	translation2D.x = m_values[Tx];
	translation2D.y = m_values[Ty];
	return translation2D;
}

//----------------------------------------------------------------------------------------------------------------------
Vec3 const Mat44::GetIBasis3D() const
{
	Vec3 iBasis3D = Vec3( m_values[Ix], m_values[Iy], m_values[Iz] );
	return iBasis3D;
}

//----------------------------------------------------------------------------------------------------------------------
Vec3 const Mat44::GetJBasis3D() const
{
	Vec3 jBasis3D = Vec3( m_values[Jx], m_values[Jy], m_values[Jz] );
	return jBasis3D;
}

//----------------------------------------------------------------------------------------------------------------------
Vec3 const Mat44::GetKBasis3D() const
{
	Vec3 kBasis3D = Vec3( m_values[Kx], m_values[Ky], m_values[Kz] );
	return kBasis3D;
}

//----------------------------------------------------------------------------------------------------------------------
Vec3 const Mat44::GetTranslation3D() const
{	
	Vec3 translate3D;
	translate3D.x = m_values[Tx];
	translate3D.y = m_values[Ty];
	translate3D.z = m_values[Tz];
	return translate3D;
}

//----------------------------------------------------------------------------------------------------------------------
Vec4 const Mat44::GetIBasis4D() const
{
	Vec4 IBasis4D = Vec4( m_values[Ix], m_values[Iy], m_values[Iz], m_values[Iw] );
	return IBasis4D;
}

//----------------------------------------------------------------------------------------------------------------------
Vec4 const Mat44::GetJBasis4D() const
{
	Vec4 JBasis4D = Vec4( m_values[Jx], m_values[Jy], m_values[Jz], m_values[Jw] );
	return JBasis4D;;
}

//----------------------------------------------------------------------------------------------------------------------
Vec4 const Mat44::GetKBasis4D() const
{
	Vec4 KBasis4D = Vec4( m_values[Kx], m_values[Ky], m_values[Kz], m_values[Kw] );
	return KBasis4D;
}

//----------------------------------------------------------------------------------------------------------------------
Vec4 const Mat44::GetTranslation4D() const
{
	Vec4 translate4D;
	translate4D.x = m_values[Tx];
	translate4D.y = m_values[Ty];
	translate4D.z = m_values[Tz];
	translate4D.w = m_values[Tw];
	return translate4D;
}

//----------------------------------------------------------------------------------------------------------------------
Mat44 const Mat44::GetOrthoNormalInverse() const
{
	// Create a rotationMatrix from this matrix' IJK basis
	Mat44 rotationMatrix;
	rotationMatrix.SetIJK3D( this->GetIBasis3D(), this->GetJBasis3D(), this->GetKBasis3D() );		
	// Transpose rotationMatrix IJK basis 
	rotationMatrix.Transpose();

	// Isolate Translation column
	Mat44 translationMatrix;
	translationMatrix.SetTranslation3D( this->GetTranslation3D() );
	// Inverse translationMatrix
	translationMatrix.m_values[Tx] *= -1;
	translationMatrix.m_values[Ty] *= -1;
	translationMatrix.m_values[Tz] *= -1;

	// Recombine 
	rotationMatrix.Append( translationMatrix );

	return rotationMatrix;
}

//----------------------------------------------------------------------------------------------------------------------
void Mat44::SetTranslation2D( Vec2 const& translationXY )
{
	m_values[Tx] = translationXY.x;
	m_values[Ty] = translationXY.y;
	m_values[Tz] = 0;
	m_values[Tw] = 1;
}

//----------------------------------------------------------------------------------------------------------------------
void Mat44::SetTranslation3D( Vec3 const& translationXYZ )
{
	m_values[Tx] = translationXYZ.x;
	m_values[Ty] = translationXYZ.y;
	m_values[Tz] = translationXYZ.z;
	m_values[Tw] = 1;
}

//----------------------------------------------------------------------------------------------------------------------
void Mat44::SetIJ2D( Vec2 const& iBasis2D, Vec2 const& jBasis2D )
{
	m_values[Ix] = iBasis2D.x;
	m_values[Iy] = iBasis2D.y;
	m_values[Iz] = 0;
	m_values[Iw] = 0;

	m_values[Jx] = jBasis2D.x;
	m_values[Jy] = jBasis2D.y;
	m_values[Jz] = 0;
	m_values[Jw] = 0;
}

//----------------------------------------------------------------------------------------------------------------------
void Mat44::SetIJT2D( Vec2 const& iBasis2D, Vec2 const& jBasis2D, Vec2 const& translationXY )
{
	m_values[Ix] = iBasis2D.x;
	m_values[Iy] = iBasis2D.y;
	m_values[Iz] = 0;
	m_values[Iw] = 0;

	m_values[Jx] = jBasis2D.x;
	m_values[Jy] = jBasis2D.y;
	m_values[Jz] = 0;
	m_values[Jw] = 0;

	m_values[Tx] = translationXY.x;
	m_values[Ty] = translationXY.y;
	m_values[Tz] = 0;
	m_values[Tw] = 1;
}

//----------------------------------------------------------------------------------------------------------------------
void Mat44::SetIJK3D( Vec3 const& iBasis3D, Vec3 const& jBasis3D, Vec3 const& kBasis3D )
{
	m_values[Ix] = iBasis3D.x;
	m_values[Iy] = iBasis3D.y;
	m_values[Iz] = iBasis3D.z;
	m_values[Iw] = 0;

	m_values[Jx] = jBasis3D.x;
	m_values[Jy] = jBasis3D.y;
	m_values[Jz] = jBasis3D.z;
	m_values[Jw] = 0;

	m_values[Kx] = kBasis3D.x;
	m_values[Ky] = kBasis3D.y;
	m_values[Kz] = kBasis3D.z;
	m_values[Kw] = 0;
}

//----------------------------------------------------------------------------------------------------------------------
void Mat44::SetIJKT3D( Vec3 const& iBasis3D, Vec3 const& jBasis3D, Vec3 const& kBasis3D, Vec3 const& translationXYZ )
{
	m_values[Ix] = iBasis3D.x;
	m_values[Iy] = iBasis3D.y;
	m_values[Iz] = iBasis3D.z;
	m_values[Iw] = 0;

	m_values[Jx] = jBasis3D.x;
	m_values[Jy] = jBasis3D.y;
	m_values[Jz] = jBasis3D.z;
	m_values[Jw] = 0;

	m_values[Kx] = kBasis3D.x;
	m_values[Ky] = kBasis3D.y;
	m_values[Kz] = kBasis3D.z;
	m_values[Kw] = 0;

	m_values[Tx] = translationXYZ.x;
	m_values[Ty] = translationXYZ.y;
	m_values[Tz] = translationXYZ.z;
	m_values[Tw] = 1;
}

//----------------------------------------------------------------------------------------------------------------------
void Mat44::SetIJKT4D( Vec4 const& iBasis4D, Vec4 const& jBasis4D, Vec4 const& kBasis4D, Vec4 const& translation4D )
{
	m_values[Ix] = iBasis4D.x;
	m_values[Iy] = iBasis4D.y;
	m_values[Iz] = iBasis4D.z;
	m_values[Iw] = iBasis4D.w;

	m_values[Jx] = jBasis4D.x;
	m_values[Jy] = jBasis4D.y;
	m_values[Jz] = jBasis4D.z;
	m_values[Jw] = jBasis4D.w;

	m_values[Kx] = kBasis4D.x;
	m_values[Ky] = kBasis4D.y;
	m_values[Kz] = kBasis4D.z;
	m_values[Kw] = kBasis4D.w;

	m_values[Tx] = translation4D.x;
	m_values[Ty] = translation4D.y;
	m_values[Tz] = translation4D.z;
	m_values[Tw] = translation4D.w;
}

//----------------------------------------------------------------------------------------------------------------------
// Will swap the basis vectors like the GetRotated90(), note* example only applies in 2D
void Mat44::Transpose()
{
	SwapValueOfTwoVariables( m_values[Iy], m_values[Jx] );
	SwapValueOfTwoVariables( m_values[Iz], m_values[Kx] );
	SwapValueOfTwoVariables( m_values[Jz], m_values[Ky] );
	SwapValueOfTwoVariables( m_values[Iw], m_values[Tx] );
	SwapValueOfTwoVariables( m_values[Jw], m_values[Ty] );
	SwapValueOfTwoVariables( m_values[Kw], m_values[Tz] );
}

//----------------------------------------------------------------------------------------------------------------------
void Mat44::OrthoNormalize_XFwd_YLeft_ZUp()
{
	// Vec3 DotProduct version
	// OrthoNormalize K Basis onto I Basis
	Vec3 iNormalized		= this->GetIBasis3D().GetNormalized();
	Vec3 kBasis			    = this->GetKBasis3D();
	Vec3 kProjectedOnto_I	= GetProjectedOnto3D( kBasis, iNormalized );
	kBasis				   -= kProjectedOnto_I;
	Vec3 kNormalized		= kBasis.GetNormalized();
	
	// OrthoNormalize J Basis onto I Basis
	Vec3 jBasis			    = this->GetJBasis3D();
	Vec3 jProjectedOnto_I	= GetProjectedOnto3D( jBasis, iNormalized );
	jBasis				   -= jProjectedOnto_I;
	
	// OrthoNormalize J Basis onto K Basis
	Vec3 jProjectedOnto_K	= GetProjectedOnto3D( jBasis, kNormalized );
	jBasis				   -= jProjectedOnto_K;
	Vec3 jNormalized		= jBasis.GetNormalized();

	this->SetIJK3D( iNormalized, jNormalized, kNormalized );
}

//----------------------------------------------------------------------------------------------------------------------
void Mat44::Append( Mat44 const& appendThis )
{
	Mat44 matrix = *this;
													   
	m_values[Ix] = ( matrix.m_values[Ix] * appendThis.m_values[Ix] ) + ( matrix.m_values[Jx] * appendThis.m_values[Iy] ) + ( matrix.m_values[Kx] * appendThis.m_values[Iz] ) + ( matrix.m_values[Tx] * appendThis.m_values[Iw] );
	m_values[Iy] = ( matrix.m_values[Iy] * appendThis.m_values[Ix] ) + ( matrix.m_values[Jy] * appendThis.m_values[Iy] ) + ( matrix.m_values[Ky] * appendThis.m_values[Iz] ) + ( matrix.m_values[Ty] * appendThis.m_values[Iw] );
	m_values[Iz] = ( matrix.m_values[Iz] * appendThis.m_values[Ix] ) + ( matrix.m_values[Jz] * appendThis.m_values[Iy] ) + ( matrix.m_values[Kz] * appendThis.m_values[Iz] ) + ( matrix.m_values[Tz] * appendThis.m_values[Iw] );
	m_values[Iw] = ( matrix.m_values[Iw] * appendThis.m_values[Ix] ) + ( matrix.m_values[Jw] * appendThis.m_values[Iy] ) + ( matrix.m_values[Kw] * appendThis.m_values[Iz] ) + ( matrix.m_values[Tw] * appendThis.m_values[Iw] );
	
	m_values[Jx] = ( matrix.m_values[Ix] * appendThis.m_values[Jx] ) + ( matrix.m_values[Jx] * appendThis.m_values[Jy] ) + ( matrix.m_values[Kx] * appendThis.m_values[Jz] ) + ( matrix.m_values[Tx] * appendThis.m_values[Jw] );
	m_values[Jy] = ( matrix.m_values[Iy] * appendThis.m_values[Jx] ) + ( matrix.m_values[Jy] * appendThis.m_values[Jy] ) + ( matrix.m_values[Ky] * appendThis.m_values[Jz] ) + ( matrix.m_values[Ty] * appendThis.m_values[Jw] );
	m_values[Jz] = ( matrix.m_values[Iz] * appendThis.m_values[Jx] ) + ( matrix.m_values[Jz] * appendThis.m_values[Jy] ) + ( matrix.m_values[Kz] * appendThis.m_values[Jz] ) + ( matrix.m_values[Tz] * appendThis.m_values[Jw] );
	m_values[Jw] = ( matrix.m_values[Iw] * appendThis.m_values[Jx] ) + ( matrix.m_values[Jw] * appendThis.m_values[Jy] ) + ( matrix.m_values[Kw] * appendThis.m_values[Jz] ) + ( matrix.m_values[Tw] * appendThis.m_values[Jw] );
	
	m_values[Kx] = ( matrix.m_values[Ix] * appendThis.m_values[Kx] ) + ( matrix.m_values[Jx] * appendThis.m_values[Ky] ) + ( matrix.m_values[Kx] * appendThis.m_values[Kz] ) + ( matrix.m_values[Tx] * appendThis.m_values[Kw] );
	m_values[Ky] = ( matrix.m_values[Iy] * appendThis.m_values[Kx] ) + ( matrix.m_values[Jy] * appendThis.m_values[Ky] ) + ( matrix.m_values[Ky] * appendThis.m_values[Kz] ) + ( matrix.m_values[Ty] * appendThis.m_values[Kw] );
	m_values[Kz] = ( matrix.m_values[Iz] * appendThis.m_values[Kx] ) + ( matrix.m_values[Jz] * appendThis.m_values[Ky] ) + ( matrix.m_values[Kz] * appendThis.m_values[Kz] ) + ( matrix.m_values[Tz] * appendThis.m_values[Kw] );
	m_values[Kw] = ( matrix.m_values[Iw] * appendThis.m_values[Kx] ) + ( matrix.m_values[Jw] * appendThis.m_values[Ky] ) + ( matrix.m_values[Kw] * appendThis.m_values[Kz] ) + ( matrix.m_values[Tw] * appendThis.m_values[Kw] );

	m_values[Tx] = ( matrix.m_values[Ix] * appendThis.m_values[Tx] ) + ( matrix.m_values[Jx] * appendThis.m_values[Ty] ) + ( matrix.m_values[Kx] * appendThis.m_values[Tz] ) + ( matrix.m_values[Tx] * appendThis.m_values[Tw] );
	m_values[Ty] = ( matrix.m_values[Iy] * appendThis.m_values[Tx] ) + ( matrix.m_values[Jy] * appendThis.m_values[Ty] ) + ( matrix.m_values[Ky] * appendThis.m_values[Tz] ) + ( matrix.m_values[Ty] * appendThis.m_values[Tw] );
	m_values[Tz] = ( matrix.m_values[Iz] * appendThis.m_values[Tx] ) + ( matrix.m_values[Jz] * appendThis.m_values[Ty] ) + ( matrix.m_values[Kz] * appendThis.m_values[Tz] ) + ( matrix.m_values[Tz] * appendThis.m_values[Tw] );
	m_values[Tw] = ( matrix.m_values[Iw] * appendThis.m_values[Tx] ) + ( matrix.m_values[Jw] * appendThis.m_values[Ty] ) + ( matrix.m_values[Kw] * appendThis.m_values[Tz] ) + ( matrix.m_values[Tw] * appendThis.m_values[Tw] );
}

//----------------------------------------------------------------------------------------------------------------------
void Mat44::AppendXRotation( float degreesRotationAboutX )
{
	Mat44 matrix;
	matrix = CreateXRotationDegrees( degreesRotationAboutX );
	Append( matrix );
}

//----------------------------------------------------------------------------------------------------------------------
void Mat44::AppendYRotation( float degreesRotationAboutY )
{
	Mat44 matrix;
	matrix = CreateYRotationDegrees( degreesRotationAboutY );
	Append( matrix );
}

//----------------------------------------------------------------------------------------------------------------------
void Mat44::AppendZRotation( float degreesRotationAboutZ )
{
	Mat44 matrix;
	matrix = CreateZRotationDegrees( degreesRotationAboutZ );
	Append( matrix );
}

//----------------------------------------------------------------------------------------------------------------------
void Mat44::AppendTranslation2D( Vec2 const& translationXY )
{
	Mat44 matrix;
	matrix = CreateTranslation2D( translationXY );
	Append( matrix );
}

//----------------------------------------------------------------------------------------------------------------------
void Mat44::AppendTranslation3D( Vec3 const& translationXYZ )
{
	Mat44 matrix;
	matrix = CreateTranslation3D( translationXYZ );
	Append( matrix );
}

//----------------------------------------------------------------------------------------------------------------------
void Mat44::AppendScaleUniform2D( float uniformScaleXY )
{
	Mat44 matrix;
	matrix = CreateUniformScale2D( uniformScaleXY );
	Append( matrix );
}

//----------------------------------------------------------------------------------------------------------------------
void Mat44::AppendScaleUniform3D( float uniformScaleXYZ )
{
	Mat44 matrix;
	matrix = CreateUniformScale3D( uniformScaleXYZ );
	Append( matrix );
}

//----------------------------------------------------------------------------------------------------------------------
void Mat44::AppendScaleNonUniform2D( Vec2 const& nonUniformScaleXY )
{
	Mat44 matrix;
	matrix = CreateNonUniformScale2D( nonUniformScaleXY );
	Append( matrix );
}

//----------------------------------------------------------------------------------------------------------------------
void Mat44::AppendScaleNonUniform3D( Vec3 const& nonUniformScaleXYZ )
{
	Mat44 matrix;
	matrix = CreateNonUniformScale3D( nonUniformScaleXYZ );
	Append( matrix );
} 