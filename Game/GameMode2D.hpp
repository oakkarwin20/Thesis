#pragma once

#include "Game/GameModeBase.hpp"

#include "Engine/Renderer/Camera.hpp"

//----------------------------------------------------------------------------------------------------------------------
class IK_Chain2D;
class IK_Joint2D;

//----------------------------------------------------------------------------------------------------------------------
class GameMode2D : public GameModeBase
{
public:
	GameMode2D();
	virtual ~GameMode2D();
	virtual void Startup();
	virtual void Update( float deltaSeconds );
	virtual void Render() const;
	virtual void Shutdown();

	// Camera and Render Functions
	void UpdateGameMode2DCamera();
	void RenderWorldObjects()	const;
	void RenderUIObjects()		const;

	// Skeleton Functions
	void RenderSkeleton( std::vector<Vertex_PCU>& verts ) const;

	// Update input
	void UpdateDebugToggle();
	void UpdatePlayerMovementInput();
	void UpdateGoalPosInput();
	void UpdateDragLimbToGoalPos( IK_Joint2D* limb );
	
	// Game logic 
	void UpdatePlacingWalkGoalPositions( IK_Joint2D* limb );
	void CalculateBestFootPos( IK_Chain2D* skeletalSystem2D, Vec2& bestFootPos, float targetAngle );

	// Debug functions
	void RenderDebugObjects( std::vector<Vertex_PCU>& verts ) const;

public:
	//----------------------------------------------------------------------------------------------------------------------
	// Camera Variables
	Camera	m_gameModeWorldCamera;
	Camera	m_gameModeUICamera;

	// Skeletal System Variables
	IK_Chain2D*	m_creatureRightArm	= nullptr;
	IK_Chain2D*	m_creatureLeftArm	= nullptr;
	IK_Chain2D*	m_creatureRightFoot	= nullptr;
	IK_Chain2D*	m_creatureLeftFoot	= nullptr;
	IK_Chain2D*	m_grass1			= nullptr;
	IK_Chain2D*	m_grass2			= nullptr;
	IK_Chain2D*	m_grass3			= nullptr;
	IK_Chain2D*	m_grass4			= nullptr;
	IK_Chain2D*	m_grass5			= nullptr;
	Vec2				m_bestRightArmPos	= Vec2::ZERO;
	Vec2				m_bestLeftArmPos	= Vec2::ZERO;
	Vec2				m_bestRightFootPos	= Vec2::ZERO;
	Vec2				m_bestLeftFootPos	= Vec2::ZERO;
	float				m_grassLength		= 5.0f;
	float				m_armLength			= 8.0f;


	// Debug Variables
	Vec2			m_debugGoalPosition	= Vec2::ZERO;
	IK_Joint2D*	m_currentLimb		= nullptr; 

	// Creature Variables
	IK_Joint2D*	m_root				= nullptr;
	Vec2			m_walkGoalPos		= Vec2::ZERO;
	bool			m_needsNewGoalPos	= true;
};
