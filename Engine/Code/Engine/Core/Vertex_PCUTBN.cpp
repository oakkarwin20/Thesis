#include "Engine/Core/Vertex_PCUTBN.hpp"

//----------------------------------------------------------------------------------------------------------------------
Vertex_PCUTBN::Vertex_PCUTBN( Vec3 const& position, Rgba8 const& color, Vec2 const& uvTexCoords, 
							  Vec3 const& tangent, Vec3 const& biNormal, Vec3 const& normal )
{
	m_position		= position;
	m_color			= color;
	m_uvTexCoords	= uvTexCoords;
	m_tangent		= tangent;
	m_biNormal		= biNormal;
	m_normal		= normal;
}
