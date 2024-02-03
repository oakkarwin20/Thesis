
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Math.h"

//----------------------------------------------------------------------------------------------------------------------
void Camera::SetOrthoView( Vec2 const& bottomLeft, Vec2 const& topRight, float near, float far)
{
	m_mode			  = MODE_ORTHOGRAPHIC;

	m_orthoBottomLeft = bottomLeft;
	m_orthoTopRight	  = topRight;
	m_orthoNear		  = near;
	m_orthoFar		  = far;
}

//----------------------------------------------------------------------------------------------------------------------
void Camera::SetPerspectiveView( float aspect, float fov, float near, float far )
{
	m_mode				= MODE_PERSPECTIVE;

	m_perspectiveAspect = aspect;
	m_perspectiveFOV	= fov;
	m_perspectiveNear	= near;
	m_perspectiveFar	= far;
}

//----------------------------------------------------------------------------------------------------------------------
Vec2 Camera::GetOrthoBottomLeft() const
{
	return m_orthoBottomLeft;
}

//----------------------------------------------------------------------------------------------------------------------
Vec2 Camera::GetOrthoTopRight() const
{
	return m_orthoTopRight;
}

//----------------------------------------------------------------------------------------------------------------------
void Camera::Translate2D( Vec2 const& translation )
{
	m_orthoBottomLeft += translation;
	m_orthoTopRight	  += translation;
}

//----------------------------------------------------------------------------------------------------------------------
Mat44 Camera::GetOrthoMatrix() const
{
	Mat44 orthoMatrix;
	orthoMatrix = Mat44::CreateOrthoProjection( m_orthoBottomLeft.x, m_orthoTopRight.x, m_orthoBottomLeft.y, m_orthoTopRight.y ,m_orthoNear, m_orthoFar);
	return orthoMatrix;
}

//----------------------------------------------------------------------------------------------------------------------
Mat44 Camera::GetPerspectiveMatrix() const
{
	Mat44 perspectiveMatrix;
	perspectiveMatrix = Mat44::CreatePerspectiveProjection( m_perspectiveFOV, m_perspectiveAspect, m_perspectiveNear, m_perspectiveFar );
	return perspectiveMatrix;
}

//----------------------------------------------------------------------------------------------------------------------
Mat44 Camera::GetProjectionMatrix() const
{
	Mat44 projectionMatrix;
	Mat44 renderMatrix; 
	
	//----------------------------------------------------------------------------------------------------------------------
	if ( m_mode == MODE_ORTHOGRAPHIC )
	{
		projectionMatrix = GetOrthoMatrix(); 
	}
	else if ( m_mode == MODE_PERSPECTIVE )
	{
		projectionMatrix = GetPerspectiveMatrix();
	}

	renderMatrix = GetRenderMatrix();
	projectionMatrix.Append( renderMatrix );
	return projectionMatrix;
}

//----------------------------------------------------------------------------------------------------------------------
void Camera::SetRenderBasis( Vec3 const& iBasis, Vec3 const& jBasis, Vec3 const& kBasis )
{
	m_renderIBasis = iBasis;
	m_renderJBasis = jBasis;
	m_renderKBasis = kBasis;
}

//----------------------------------------------------------------------------------------------------------------------
Mat44 Camera::GetRenderMatrix() const
{
	Mat44 renderMatrix;
	renderMatrix.SetIJK3D( m_renderIBasis, m_renderJBasis, m_renderKBasis );
	return renderMatrix;
}

//----------------------------------------------------------------------------------------------------------------------
void Camera::SetTransform( Vec3 const& position, EulerAngles const& orientation )
{
	m_position	  = position;
	m_orientation = orientation;
}

//----------------------------------------------------------------------------------------------------------------------
Mat44 Camera::GetViewMatrix() const
{
	Mat44 viewMatrix;
	viewMatrix = m_orientation.GetAsMatrix_XFwd_YLeft_ZUp();
	viewMatrix.SetTranslation3D( m_position );
	viewMatrix = viewMatrix.GetOrthoNormalInverse();
	return viewMatrix;
}


//----------------------------------------------------------------------------------------------------------------------
Vec3 Camera::GetDirCamToMouse_ScreenToWorld( Vec2 const& cursorPos_Screenspace, Vec2 const& screenBounds ) const
{
	Vec3 camFwd, camLeft, camUp;
	m_orientation.GetAsVectors_XFwd_YLeft_ZUp( camFwd, camLeft, camUp );

	Vec3  fwd					= m_perspectiveNear * camFwd;
	float fovRadians			= ConvertDegreesToRadians( m_perspectiveFOV / 2.0f );
	float angleDegrees			= tanf( fovRadians );
	float cursorRatio_X			= RangeMapClamped( cursorPos_Screenspace.x, 0.0f, screenBounds.x, -1.0f, 1.0f );
	float cursorRatio_Y			= RangeMapClamped( cursorPos_Screenspace.y, 0.0f, screenBounds.y, -1.0f, 1.0f );

	Vec3 left					= -cursorRatio_X * m_perspectiveAspect * m_perspectiveNear * angleDegrees * camLeft;
	Vec3 up						= -cursorRatio_Y * m_perspectiveNear * angleDegrees * camUp;

	Vec3 mousePos_WorldSpace	= m_position + fwd + left + up;
	mousePos_WorldSpace			= mousePos_WorldSpace - m_position;
	mousePos_WorldSpace			= mousePos_WorldSpace.GetNormalized();
	return mousePos_WorldSpace;
}


//----------------------------------------------------------------------------------------------------------------------
Vec3 Camera::GetDirCamToScreenCenter_ScreenToWorld( Vec2 const& screenBounds ) const
{
	UNUSED( screenBounds );

	Vec3 camFwd, camLeft, camUp;
	m_orientation.GetAsVectors_XFwd_YLeft_ZUp( camFwd, camLeft, camUp );
	Vec3  fwd							= m_perspectiveNear * camFwd;
	Vec3 screenCenterPos_WorldSpace		= m_position + fwd;
	screenCenterPos_WorldSpace			= screenCenterPos_WorldSpace - m_position;
	screenCenterPos_WorldSpace			= screenCenterPos_WorldSpace.GetNormalized();
	return screenCenterPos_WorldSpace;
}
