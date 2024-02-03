
#include "Engine/Math/OBB3D.hpp"
#include "Engine/Core/EngineCommon.hpp"

//----------------------------------------------------------------------------------------------------------------------
OBB3D::OBB3D( Vec3 center, Vec3 iBasis, Vec3 jBasis, Vec3 kBasis, Vec3 halfDimensions )
{
	m_center			= center;
	m_iBasisNormal		= iBasis;
	m_jBasisNormal		= jBasis;
	m_kBasisNormal		= kBasis;
	m_halfDimensions	= halfDimensions;
}


//----------------------------------------------------------------------------------------------------------------------
OBB3D::~OBB3D()
{
}


//----------------------------------------------------------------------------------------------------------------------
// #ToDo fix this function, not fully understanding what the parameter is used for
void OBB3D::GetCornerPoints( Vec3* out_fourCornerWorldPositions ) const
{
	UNUSED( out_fourCornerWorldPositions );

//	Vec3 jBasisNormal	= m_iBasisNormal.GetRotated90Degrees();
	Vec3 jBasisNormal	= m_iBasisNormal;
	Vec3 right			= m_iBasisNormal * m_halfDimensions.x;
	Vec3 up				= jBasisNormal * m_halfDimensions.y;

	Vec3 topRight		= m_center+ right + up;
	Vec3 topLeft		= m_center - right + up;
	Vec3 bottomRight	= m_center + right - up;
	Vec3 bottomLeft		= m_center - right - up;
}
