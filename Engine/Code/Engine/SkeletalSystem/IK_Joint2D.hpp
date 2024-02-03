 #pragma once

#include "Engine/Math/Vec2.hpp"

//----------------------------------------------------------------------------------------------------------------------
class IK_Joint2D
{
public:
	IK_Joint2D( Vec2 startPos, float length, float thickness = 1.0f, float orientation = 0.0f, IK_Joint2D* parent = nullptr );
	~IK_Joint2D();

	Vec2 GetLimbEndXY_FK( IK_Joint2D* currentLimb );
	Vec2 GetLimbEndXY_IK();
	void SetOrientationToTargetPos( Vec2 targetPos );
	void DragLimb( Vec2 targetPos );

public:
	// Core variables
	float			m_length			= 0.0f;
	float			m_thickness			= 0.0f;
	float			m_orientation		= 0.0f;
	IK_Joint2D*		m_parent			= nullptr;
	
	// Position variables
	Vec2		m_jointPosition_LS	= Vec2::ZERO;
	Vec2		m_endPos			= Vec2::ZERO;
//	FloatRange	m_physicsConstraints;				// #ToDo: Add later, not needed now for the first iteration
};