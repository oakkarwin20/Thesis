#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/EulerAngles.hpp"

//----------------------------------------------------------------------------------------------------------------------
struct Mat44;

//----------------------------------------------------------------------------------------------------------------------
class Camera 
{
public:
	enum Mode
	{
		MODE_ORTHOGRAPHIC,
		MODE_PERSPECTIVE,

		NUM_MODE,
	};

	void SetOrthoView( Vec2 const& bottomLeft, Vec2 const& topRight, float near = 0.0f, float far = 1.0f );
	void SetPerspectiveView( float aspect, float fov, float near, float far );

	Vec2 GetOrthoBottomLeft() const;
	Vec2 GetOrthoTopRight()	  const;
	void Translate2D( Vec2 const& translation );

	Mat44 GetOrthoMatrix()		 const;
	Mat44 GetPerspectiveMatrix() const;
	Mat44 GetProjectionMatrix()  const;

	void  SetRenderBasis( Vec3 const& iBasis, Vec3 const& jBasis, Vec3 const& kBasis );
	Mat44 GetRenderMatrix() const;

	void  SetTransform( Vec3 const& position, EulerAngles const& orientation );
	Mat44 GetViewMatrix() const;

	Vec3 GetDirCamToMouse_ScreenToWorld( Vec2 const& cursorPos_Screenspace, Vec2 const& screenBounds ) const;
	Vec3 GetDirCamToScreenCenter_ScreenToWorld( Vec2 const& screenBounds ) const;

public:
	Vec3			m_position;
	EulerAngles		m_orientation;
		
	Vec3			m_renderIBasis = Vec3( 1.0f, 0.0f, 0.0f );
	Vec3			m_renderJBasis = Vec3( 0.0f, 1.0f, 0.0f );
	Vec3			m_renderKBasis = Vec3( 0.0f, 0.0f, 1.0f );

private:
	Mode m_mode = MODE_ORTHOGRAPHIC;

	// Ortho variables
	Vec2  m_orthoBottomLeft;
	Vec2  m_orthoTopRight;
	float m_orthoNear;
	float m_orthoFar;

	// Perspective variables
	float m_perspectiveAspect;
	float m_perspectiveFOV;
	float m_perspectiveNear;
	float m_perspectiveFar;
};