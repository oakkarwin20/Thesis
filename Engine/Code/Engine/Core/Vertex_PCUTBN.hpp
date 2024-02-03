#pragma once

#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Rgba8.hpp"

//----------------------------------------------------------------------------------------------------------------------
struct Vertex_PCUTBN
{
public:
	Vertex_PCUTBN() {};
	~Vertex_PCUTBN() {};

	Vec3	m_position		= Vec3::ZERO;
	Rgba8	m_color			= Rgba8::WHITE;		
	Vec2	m_uvTexCoords	= Vec2::ZERO;	
	Vec3	m_tangent		= Vec3::ZERO;
	Vec3	m_biNormal		= Vec3::ZERO;
	Vec3	m_normal		= Vec3::ZERO;

	explicit Vertex_PCUTBN( Vec3 const& position, Rgba8 const& color, Vec2 const& uvTexCoords, 
							Vec3 const& tangent = Vec3::ZERO, Vec3 const& biNormal = Vec3::ZERO, Vec3 const& normal = Vec3::ZERO );
};