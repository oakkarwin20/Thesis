#include "Engine/SkeletalSystem/IK_Joint2D.hpp"
#include "Engine/Math/MathUtils.hpp"

//----------------------------------------------------------------------------------------------------------------------
IK_Joint2D::IK_Joint2D( Vec2 startPos, float length, float thickness, float orientation, IK_Joint2D* parent )
	: m_jointPosition_LS( startPos )
	, m_length( length )
	, m_thickness( thickness )
	, m_orientation( orientation )
	, m_parent( parent )
{
	Vec2 endPos = GetLimbEndXY_IK();
}

//----------------------------------------------------------------------------------------------------------------------
IK_Joint2D::~IK_Joint2D()
{
}

//----------------------------------------------------------------------------------------------------------------------
Vec2 IK_Joint2D::GetLimbEndXY_FK( IK_Joint2D* currentLimb )
{
	//----------------------------------------------------------------------------------------------------------------------
	// Forwards Kinematics version
	//----------------------------------------------------------------------------------------------------------------------
	// Climbs up the parent hierarchy
	float addedAngle = currentLimb->m_orientation;
	IK_Joint2D* parent	 = currentLimb->m_parent;
	while ( parent != nullptr )
	{
		addedAngle += parent->m_orientation;
		parent		= parent->m_parent;
	}
	float limbEndX  = currentLimb->m_jointPosition_LS.x + CosDegrees( addedAngle ) * currentLimb->m_length;
	float limbEndY  = currentLimb->m_jointPosition_LS.y + SinDegrees( addedAngle ) * currentLimb->m_length;
	Vec2 limbEndPos = Vec2( limbEndX, limbEndY );
	return limbEndPos;
}

//----------------------------------------------------------------------------------------------------------------------
Vec2 IK_Joint2D::GetLimbEndXY_IK()
{
	float limbEndX  = m_jointPosition_LS.x + CosDegrees( m_orientation ) * m_length;
	float limbEndY  = m_jointPosition_LS.y + SinDegrees( m_orientation ) * m_length;
	Vec2 limbEndPos = Vec2( limbEndX, limbEndY );
	return limbEndPos;
}

//----------------------------------------------------------------------------------------------------------------------
void IK_Joint2D::SetOrientationToTargetPos( Vec2 targetPos )
{
	Vec2 dispStartToTarget  = targetPos - m_jointPosition_LS;
	float newOrientation	= Atan2Degrees( dispStartToTarget.y, dispStartToTarget.x );
	m_orientation			= newOrientation;
}

//----------------------------------------------------------------------------------------------------------------------
void IK_Joint2D::DragLimb( Vec2 targetPos )
{
	// Set orientation towards target pos
	SetOrientationToTargetPos( targetPos );

	// Set startPosXY "m_length" away from its targetPos
	m_jointPosition_LS.x = targetPos.x - CosDegrees( m_orientation ) * m_length;
	m_jointPosition_LS.y = targetPos.y - SinDegrees( m_orientation ) * m_length;

	// If parent exists, set their position according to "my" startPos
	if ( m_parent != nullptr )
	{
		m_parent->DragLimb( m_jointPosition_LS );
	}
}
