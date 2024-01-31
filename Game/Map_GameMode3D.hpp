#pragma once

#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"

#include <vector>

//----------------------------------------------------------------------------------------------------------------------
class GameMode3D;
class VertexBuffer;
class IndexBuffer;

/*
//----------------------------------------------------------------------------------------------------------------------
struct Block
{
	Block( AABB3 aabb3, bool isWalkable, bool isClimbable = false )
		: m_aabb3( aabb3 )
		, m_isWalkable( isWalkable )
		, m_isClimbable( isClimbable )
	{
	}
	~Block()
	{
	}

	AABB3	m_aabb3			= AABB3( Vec3::ZERO, Vec3::ZERO );
	bool	m_isWalkable	= true;
	bool	m_isClimbable	= false;
};
*/


//----------------------------------------------------------------------------------------------------------------------
class Map_GameMode3D
{
public:
	Map_GameMode3D( GameMode3D* game );
	~Map_GameMode3D();

	void Update();
	void Render() const;

public:
	GameMode3D*					m_game			= nullptr;
	VertexBuffer*				m_vbo			= nullptr;
	IndexBuffer*				m_ibo			= nullptr;
	std::vector<Vertex_PCU>		m_planeVerts;
	std::vector<unsigned int>	m_indexList;
	float m_minFloorHeight = -2.0f;
	float m_maxFloorHeight =  2.0f;
};