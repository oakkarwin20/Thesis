#pragma once

#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include <vector>


//----------------------------------------------------------------------------------------------------------------------
class GameMode3D;
class Quadruped;
class IK_Chain3D;


//----------------------------------------------------------------------------------------------------------------------
struct FoodOrb
{
public: 
	FoodOrb( Vec3 const& position )
	{
		m_position = position;
	}

	Vec3		m_position		= Vec3::ZERO;
	Vec3		m_goalPos		= Vec3::ZERO;
	Vec3		m_prevPos		= Vec3::ZERO;
	bool		m_isConsumed	= false;
	bool		m_isGrabbed		= false;
	Quadruped*  m_quadSpider	= nullptr;
};


//----------------------------------------------------------------------------------------------------------------------
class FoodManager
{
public:
	FoodManager( int numFoodOrbs, GameMode3D* game );
	~FoodManager();

	void Update( float deltaSeconds );
	void Render( std::vector<Vertex_PCU>& verts );
	Vec3 GetRandPos_MaxZ();
	void SpawnFoodAtRandPos();
	void ConsumeFood( FoodOrb& foodToConsume );
	void MoveFoodOrbs( float deltaSeconds );


public:
	std::vector<FoodOrb> m_foodList;
	GameMode3D*			 m_game =	nullptr;
};