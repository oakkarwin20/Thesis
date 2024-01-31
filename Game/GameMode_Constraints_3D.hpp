#pragma once

#include "Game/GameModeBase.hpp"
#include "Game/GameCommon.hpp"

#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Math/MathUtils.hpp"

//----------------------------------------------------------------------------------------------------------------------
class Texture;
class IK_Chain3D;
class IK_Joint3D;
class CreatureBase;


//----------------------------------------------------------------------------------------------------------------------
class GameMode_Constraints_3D : public GameModeBase
{
public:
	GameMode_Constraints_3D();
	virtual ~GameMode_Constraints_3D();
	virtual void Startup();
	virtual void Update( float deltaSeconds );
	virtual void Render() const;
	virtual void Shutdown();

	// Camera and Render Functions
	void UpdateDebugKeys();
	void UpdateCameraInput( float deltaSeconds );
	void UpdateGameMode3DCamera();
	void RenderWorldObjects()	const;
	void RenderUIObjects()		const;

	// Debug Functions
	void AddVertsForCompass( std::vector<Vertex_PCU>&compassVerts, Vec3 startPosition, float axisLength, float axisThickness ) const;
	void UpdateTargetInput( float deltaSeconds);

	//----------------------------------------------------------------------------------------------------------------------
	// CCD 
	//----------------------------------------------------------------------------------------------------------------------
	void InitializeIK_ChainCCD();
	void SetIK_ChainConstraints();

public:
	//----------------------------------------------------------------------------------------------------------------------
	// Core Variables
	//----------------------------------------------------------------------------------------------------------------------
	float m_sine						= 0.0f;
	float m_currentTime					= 0.0f;
	float m_walkLerpSpeed				= 4.0f;
	float m_sprintLerpSpeed				= m_walkLerpSpeed * 4.0f;
	float m_goalWalkLerpSpeed			= m_walkLerpSpeed;
	float m_currentWalkLerpSpeed		= m_goalWalkLerpSpeed;

	//----------------------------------------------------------------------------------------------------------------------
	// Camera Variables
	//----------------------------------------------------------------------------------------------------------------------
	Camera	m_gameMode3DWorldCamera;
	Camera	m_gameMode3DUICamera;
	// Camera movement Variables
	float	m_defaultSpeed	= 2.0f;
	float	m_currentSpeed	= 2.0f;
	float	m_fasterSpeed	= m_defaultSpeed * 4.0f;
	float	m_elevateSpeed  = 6.0f;
	float	m_turnRate		= 90.0f;

	//----------------------------------------------------------------------------------------------------------------------
	// Debug variables
	//----------------------------------------------------------------------------------------------------------------------
	float m_distCamAwayFromPlayer			= 180.0f;
	Vec3  m_debugTargetPos					= Vec3::ZERO;
	float m_debugTargetAngle_PolarCoords	= 0.0f;
	float m_debugTargetLength_PolarCoords	= 0.0f;
	bool  m_debugStepOneFrame				= false;
	float m_debugSine						= 0.0f;
	float m_debugCos						= 0.0f;
	float m_debugPerlinNoise				= 0.0f;

	//----------------------------------------------------------------------------------------------------------------------
	// CCD creature
	//----------------------------------------------------------------------------------------------------------------------
	CreatureBase*	m_creatureCCD		= nullptr;
	IK_Chain3D*		m_ikChain_CCD		= nullptr;
	float			m_limbLength		= 10.0f;
	float			m_halfLimbLength	= m_limbLength * 0.5f;

};
