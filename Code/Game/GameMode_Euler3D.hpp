#pragma once

#include "Game/GameModeBase.hpp"
#include "Game/GameCommon.hpp"
#include "Game/App.hpp"

#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/CubicBezierCurve3D.hpp"
#include "Engine/Core/Stopwatch.hpp"


//----------------------------------------------------------------------------------------------------------------------
class Texture;
class IK_Chain3D;
class IK_Joint3D;
class CreatureBase;
class Quadruped;

//----------------------------------------------------------------------------------------------------------------------
class GameMode_Euler3D : public GameModeBase
{
public:
	GameMode_Euler3D();
	virtual ~GameMode_Euler3D();
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
	void UpdateInputDebugPosEE();
	void UpdateCreature();

	// Tree Functions
	void InitializeTrees();
	void RenderCreature( std::vector<Vertex_PCU>& verts, std::vector<Vertex_PCU>& verts_textured ) const;

	// Creature Functions
	void InitializeCreatures();
	void SetLeftFootConstraints();
	void UpdateCreatureInput( float deltaSeconds );
	void UpdatePoleVector();
	void DetermineBestWalkStepPos();
	bool IsLimbIsTooFarFromRoot( IK_Chain3D* currentLimb, Vec3 const& footTargetPos );
	void SpecifyFootPlacementPos( Vec3& targetPos, float fwdStepAmount, float leftStepAmount );

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
	float	m_defaultSpeed	= 20.0f;
	float	m_currentSpeed	= 20.0f;
	float	m_fasterSpeed	= m_defaultSpeed * 4.0f;
	float	m_elevateSpeed  = 6.0f;
	float	m_turnRate		= 90.0f;


	//----------------------------------------------------------------------------------------------------------------------
	// Tree Variables
	//----------------------------------------------------------------------------------------------------------------------
	IK_Chain3D*	 m_treeBranch1			= nullptr;
	float		 m_numTreeSegments		= 3.0f;
	float		 m_limbLength			= 10.0f;
	float		 m_halfLimbLength		= m_limbLength * 0.5f;

	//----------------------------------------------------------------------------------------------------------------------
	// Debug variables
	//----------------------------------------------------------------------------------------------------------------------
	float m_distCamAwayFromPlayer			= 180.0f;
	Vec3  m_debugTargetPos					= Vec3::ZERO;
	float m_debugTargetAngle_PolarCoords	= 0.0f;
	float m_debugTargetLength_PolarCoords	= 0.0f;
	bool  m_debugStepOneFrame				= false;

	//----------------------------------------------------------------------------------------------------------------------
	// Creature variables
	//----------------------------------------------------------------------------------------------------------------------
	CreatureBase*				m_creature				= nullptr;
	std::vector<IK_Chain3D*>	m_creatureSkeletalSystemsList;
	IK_Chain3D*					m_leftFoot				= nullptr;
	IK_Chain3D*					m_rightFoot				= nullptr;

	float	m_maxLength				= 0.0f;

	float	m_numFeet				=  5.0f;
	float	m_rootDefaultHeightZ	= 20.0f;

	//----------------------------------------------------------------------------------------------------------------------
	// Bezier Curves
	//----------------------------------------------------------------------------------------------------------------------
	CubicBezierCurve3D  m_bezierCurve	= CubicBezierCurve3D( Vec3( -40.0f, 0.0f, 0.0f ), Vec3( -20.0f, 0.0f, 25.0f ), Vec3( 20.0f, 0.0f, 25.0f ), Vec3( 40.0f, 0.0f, 0.0f ) );
	Stopwatch			m_timer			= Stopwatch( &g_theApp->m_gameClock, 1.0f );

	//----------------------------------------------------------------------------------------------------------------------
	Quadruped* m_quadruped_bindPose = nullptr;
};
