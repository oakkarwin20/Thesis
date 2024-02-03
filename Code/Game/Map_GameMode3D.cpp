#include "Game/Map_GameMode3D.hpp"
#include "Game/GameMode3D.hpp"
#include "Game/App.hpp"
#include "Game/GameCommon.hpp"
#include "Game/FoodManager.hpp"

#include "Engine/Math/RandomNumberGenerator.hpp"


//----------------------------------------------------------------------------------------------------------------------
Map_GameMode3D::Map_GameMode3D( GameMode3D* game )
{
	m_game = game;

	//----------------------------------------------------------------------------------------------------------------------
	// Initialize randomized Z plane
	//----------------------------------------------------------------------------------------------------------------------
	// Randomize floor height
	float height = 22.0f;
	AddVertsForPlane( m_planeVerts, m_indexList, Vec3( 15.0f, 15.0f, 0.0f ), 10, 20, 20 );
	for ( int i = 0; i < m_planeVerts.size(); i++ )
	{
		Vertex_PCU& currentVert		= m_planeVerts[ i ];
		currentVert.m_position.z	= height;
		float rand					= g_theRNG->RollRandomFloatInRange( m_minFloorHeight, m_maxFloorHeight );
		height						+= rand;
	}
	m_vbo = g_theRenderer->CreateVertexBuffer( m_planeVerts.size(), sizeof(Vertex_PCU) );
	m_ibo = g_theRenderer->CreateIndexBuffer (  m_indexList.size() );
	g_theRenderer->Copy_CPU_To_GPU( m_planeVerts.data(), sizeof( Vertex_PCU )   * m_planeVerts.size(), m_vbo, sizeof( Vertex_PCU ) );
	g_theRenderer->Copy_CPU_To_GPU(  m_indexList.data(), sizeof( unsigned int ) *  m_indexList.size(), m_ibo );

	//----------------------------------------------------------------------------------------------------------------------
	// Initialize food orbs
	//----------------------------------------------------------------------------------------------------------------------
	m_foodManager = new FoodManager( 1, m_game );
}


//----------------------------------------------------------------------------------------------------------------------
Map_GameMode3D::~Map_GameMode3D()
{
	m_game = nullptr;
	delete m_vbo;
	m_vbo = nullptr;
	delete m_ibo;
	m_ibo = nullptr;
	delete m_foodManager;
	m_foodManager = nullptr;
}


//----------------------------------------------------------------------------------------------------------------------
void Map_GameMode3D::Update( float deltaSeconds )
{
	if ( g_theInput->WasKeyJustPressed( '8' ) )
	{
		// Randomize floor height
		float height = 25.0f;
		for ( int i = 0; i < m_planeVerts.size(); i++ )
		{
			Vertex_PCU& currentVert		= m_planeVerts[ i ];
			currentVert.m_position.z	= height;
			float rand					= g_theRNG->RollRandomFloatInRange( m_minFloorHeight, m_maxFloorHeight );
			height						-= rand;
		}
		g_theRenderer->Copy_CPU_To_GPU( m_planeVerts.data(), sizeof( Vertex_PCU )   * m_planeVerts.size(), m_vbo, sizeof( Vertex_PCU ) );
		g_theRenderer->Copy_CPU_To_GPU(  m_indexList.data(), sizeof( unsigned int ) *  m_indexList.size(), m_ibo );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Raycast test logic
	//----------------------------------------------------------------------------------------------------------------------
	float rayMaxLength = 100.0f;
	RaycastResult3D tempRayResult;
	tempRayResult.m_rayMaxLength			= rayMaxLength;
	m_game->m_rayVsTri						= tempRayResult;
	m_game->m_rayVsTri.m_rayStartPosition	= m_game->m_gameMode3DWorldCamera.m_position;
	m_game->m_rayVsTri.m_rayFwdNormal		= m_game->m_gameMode3DWorldCamera.m_orientation.GetForwardDir_XFwd_YLeft_ZUp();

	float t, u, v = 0.0f;
	for ( int i = i = 0; i < m_indexList.size(); i += 3 )
	{
		int const& currentIndex_A = m_indexList[ i + 0];
		int const& currentIndex_B = m_indexList[ i + 1];
		int const& currentIndex_C = m_indexList[ i + 2];

		//  m_vbo->m_buffer[ currentIndex ];
		Vec3 vert0 = m_planeVerts[ currentIndex_A ].m_position;
		Vec3 vert1 = m_planeVerts[ currentIndex_B ].m_position;
		Vec3 vert2 = m_planeVerts[ currentIndex_C ].m_position;

		tempRayResult = RaycastVsTriangle( m_game->m_gameMode3DWorldCamera.m_position, m_game->m_gameMode3DWorldCamera.m_orientation.GetForwardDir_XFwd_YLeft_ZUp(), 
																	m_game->m_rayVsTri.m_rayMaxLength, vert0, vert1, vert2, t, u, v );
		if ( tempRayResult.m_didImpact )
		{
			// If ray hit AND is closer
			if ( tempRayResult.m_impactDist < m_game->m_rayVsTri.m_impactDist )
			{
				m_game->m_rayVsTri = tempRayResult;
			}
		}
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Lerp foodBalls to goal positions
	//----------------------------------------------------------------------------------------------------------------------
	m_foodManager->Update( deltaSeconds );
}


//----------------------------------------------------------------------------------------------------------------------
void Map_GameMode3D::Render() const
{
	std::vector<Vertex_PCU> skyVerts;
	std::vector<Vertex_PCU> verts;
	verts.reserve( 441 );
	if ( g_debugRenderRaycast_F2 )
	{
		for ( int i = 0; i < m_planeVerts.size(); i++ )
		{
			Vertex_PCU const& currentVert = m_planeVerts[ i ];
			AddVertsForSphere3D( verts, currentVert.m_position, 1.0f, 4.0f, 4.0f, Rgba8::MAGENTA );
		}
	}

	AddVertsForSphere3D( skyVerts, Vec3::ZERO, 800.0f, 32.0f, 32.0f );

	//----------------------------------------------------------------------------------------------------------------------
	// Render food orbs
	//----------------------------------------------------------------------------------------------------------------------
	m_foodManager->Render( verts );

	// Render plane
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( g_theApp->m_texture_MoonSurface );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->DrawVertexAndIndexBuffer( m_vbo, m_ibo, int( m_indexList.size() ) );
	g_theRenderer->BindTexture( nullptr );

	// Debug Render vert positions
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( g_theApp->m_texture_Galaxy );
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_NONE );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->DrawVertexArray( int( skyVerts.size() ), skyVerts.data() );
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );

	// Debug Render vert positions
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->DrawVertexArray( int( verts.size() ), verts.data() );
}