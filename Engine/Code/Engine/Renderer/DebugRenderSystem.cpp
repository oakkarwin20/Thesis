#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/Stopwatch.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Core/EngineCommon.hpp"

#include <vector>

//----------------------------------------------------------------------------------------------------------------------
class ObjectsToRender
{
public:
	ObjectsToRender( float duration );
	~ObjectsToRender();

	void Update();
	void Render( Camera const& camera, Renderer* renderer );

public:
	std::vector<Vertex_PCU> m_vertexes;
	Vec3					m_position					= Vec3( 0.0f, 0.0f, 0.0f);
	Rgba8					m_startColor;
	Rgba8					m_endColor;
	Rgba8					m_currentColor;
	BitmapFont*				m_textFont					= nullptr;
	Texture*				m_texture					= nullptr;
	bool					m_isBillboardFullCamOppo	= false;
	bool					m_isWireframe				= false;
	Stopwatch				m_stopwatch;
	DebugRenderMode			m_mode;
};

//----------------------------------------------------------------------------------------------------------------------
class DebugRenderSystem
{
public:
	DebugRenderSystem( DebugRenderConfig const& config );
	~DebugRenderSystem();

	void Update();
	void RenderWorld( Camera const& worldCamera );
	void RenderScreen( Camera const& screenCamera );

	std::vector<ObjectsToRender> m_worldObjectList;
	std::vector<ObjectsToRender> m_screenObjectList;
	DebugRenderConfig			 m_debugRenderConfig;
};

//----------------------------------------------------------------------------------------------------------------------
// DebugRenderConfig s_debugRenderConfig;
static DebugRenderSystem* s_theDebugRenderSystem = nullptr;
bool isRenderVisible = true;

//----------------------------------------------------------------------------------------------------------------------
// DebugRenderSystem class methods
//----------------------------------------------------------------------------------------------------------------------
DebugRenderSystem::DebugRenderSystem( DebugRenderConfig const& config )
{
	m_debugRenderConfig.m_renderer = config.m_renderer;
	m_debugRenderConfig.m_startHidden = config.m_startHidden;
}

//----------------------------------------------------------------------------------------------------------------------
DebugRenderSystem::~DebugRenderSystem()
{
}

//----------------------------------------------------------------------------------------------------------------------
void DebugRenderSystem::Update()
{
	for ( int i = 0; i < m_worldObjectList.size(); i++ )
	{
		// If duration hasn't elapsed, call object's update
		m_worldObjectList[i].Update();
	}

	// if stopwatch has elapsed, delete object
	m_worldObjectList.erase( 
		remove_if(
			m_worldObjectList.begin(),
			m_worldObjectList.end(),
			[=]( auto const& element )
			{
				return element.m_stopwatch.HasDurationElapsed();
			}
		),
		m_worldObjectList.end()
				);

	for ( int i = 0; i < m_screenObjectList.size(); i++ )
	{
		// If duration hasn't elapsed, call object's update
		m_screenObjectList[i].Update();
	}

	// if stopwatch has elapsed, delete object
	m_screenObjectList.erase(
		remove_if(
			m_screenObjectList.begin(),
			m_screenObjectList.end(),
			[=]( auto const& element )
			{
				return element.m_stopwatch.HasDurationElapsed();
			}
		),
		m_screenObjectList.end()
				);
}

//----------------------------------------------------------------------------------------------------------------------
void DebugRenderSystem::RenderWorld( Camera const& worldCamera )
{
	for ( int i = 0; i < m_worldObjectList.size(); i++ )
	{
		m_worldObjectList[i].Render( worldCamera, m_debugRenderConfig.m_renderer );
	}
}

//----------------------------------------------------------------------------------------------------------------------
void DebugRenderSystem::RenderScreen( Camera const& screenCamera )
{
	for ( int i = 0; i < m_screenObjectList.size(); i++ )
	{
		m_screenObjectList[i].Render( screenCamera, m_debugRenderConfig.m_renderer );
	}
}

//----------------------------------------------------------------------------------------------------------------------
// ObjectsToRender class methods
//----------------------------------------------------------------------------------------------------------------------
ObjectsToRender::ObjectsToRender( float duration )
	: m_stopwatch( duration )
{
	m_stopwatch.Start();
	m_textFont = s_theDebugRenderSystem->m_debugRenderConfig.m_renderer->CreateOrGetBitmapFontFromFile( "Data/Fonts/SquirrelFixedFont" );
}

//----------------------------------------------------------------------------------------------------------------------
ObjectsToRender::~ObjectsToRender()
{
}

//----------------------------------------------------------------------------------------------------------------------
void ObjectsToRender::Update()
{
//	DebuggerPrintf( "%f\n", m_stopwatch.GetElapsedFraction() );
	Rgba8 lerpedColor = Interpolate( m_startColor, m_endColor, m_stopwatch.GetElapsedFraction() );
	m_currentColor = lerpedColor;
}

//----------------------------------------------------------------------------------------------------------------------
void ObjectsToRender::Render( Camera const& camera, Renderer* renderer )
{
//	camera.
	if ( isRenderVisible )
	{
		renderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_NONE );
		if ( m_isWireframe == true )
		{
			renderer->SetRasterizerMode( RasterizerMode::WIREFRAME_CULL_NONE );
		}

		if ( m_isBillboardFullCamOppo )
		{
			Mat44 cameraMatrix		= camera.m_orientation.GetAsMatrix_XFwd_YLeft_ZUp();
//			Mat44 billboardMatrix	= GetBillboardMatrix( BillboardType::FULL_CAMERA_OPPOSING,		cameraMatrix, m_position );
//			Mat44 billboardMatrix	= GetBillboardMatrix( BillboardType::WORLD_UP_CAMERA_FACING,	cameraMatrix, m_position );
//			Mat44 billboardMatrix	= GetBillboardMatrix( BillboardType::FULL_CAMERA_FACING,		cameraMatrix, m_position );
			Mat44 billboardMatrix	= GetBillboardMatrix( BillboardType::WORLD_UP_CAMERA_OPPOSING,	cameraMatrix, m_position );
			TransformVertexArray3D( m_vertexes, billboardMatrix );
		}

		if ( m_mode == DebugRenderMode::USE_DEPTH )
		{
			// Draw call
			renderer->SetBlendMode( BlendMode::ALPHA );
			renderer->SetDepthMode( DepthMode::ENABLED );
			renderer->SetModelConstants( Mat44(), m_currentColor );
			renderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
			renderer->BindTexture( m_texture );
			renderer->BindShader( nullptr );
			renderer->DrawVertexArray( (int)m_vertexes.size(), m_vertexes.data() );
		}
		else if ( m_mode == DebugRenderMode::ALWAYS )
		{
			// Draw call
			renderer->SetBlendMode( BlendMode::ALPHA );
			renderer->SetDepthMode( DepthMode::DISABLED );
			renderer->SetModelConstants( Mat44(), m_currentColor );
			renderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
			renderer->BindTexture( m_texture );
			renderer->BindShader( nullptr );
			renderer->DrawVertexArray( (int)m_vertexes.size(), m_vertexes.data() );
		}
		else if ( m_mode == DebugRenderMode::X_RAY )
		{
			// First Draw call
			renderer->SetBlendMode( BlendMode::ALPHA );
			renderer->SetDepthMode( DepthMode::DISABLED );
			Rgba8 lighterColor = Rgba8( (unsigned char)m_currentColor.r, (unsigned char)m_currentColor.g, (unsigned char)m_currentColor.b, (unsigned char)m_currentColor.a / 2 );
			renderer->SetModelConstants( Mat44(), lighterColor );
			renderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
			renderer->BindTexture( m_texture );
			renderer->BindShader( nullptr );
			renderer->DrawVertexArray( (int)m_vertexes.size(), m_vertexes.data() );
			
			// Second Draw call
			renderer->SetBlendMode( BlendMode::OPAQUE );
			renderer->SetDepthMode( DepthMode::ENABLED );
			renderer->SetModelConstants( Mat44(), m_currentColor );
			renderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
			renderer->BindShader( nullptr );
			renderer->DrawVertexArray( (int)m_vertexes.size(), m_vertexes.data() );
		}
			// Reseting states
		renderer->SetBlendMode( BlendMode::ALPHA );
		renderer->SetDepthMode( DepthMode::ENABLED );
		renderer->SetModelConstants( Mat44(), Rgba8::WHITE );
		renderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );	
		renderer->BindShader( nullptr );
		renderer->BindTexture( nullptr );
	}
}

//----------------------------------------------------------------------------------------------------------------------
// DebugRenderSystem static functions
//----------------------------------------------------------------------------------------------------------------------
void DebugRenderSystemStartup( DebugRenderConfig const& config )
{
	s_theDebugRenderSystem = new DebugRenderSystem( config );

	if ( s_theDebugRenderSystem != nullptr )
	{
		s_theDebugRenderSystem->m_debugRenderConfig.m_renderer		= config.m_renderer;
		s_theDebugRenderSystem->m_debugRenderConfig.m_startHidden	= config.m_startHidden;
	}
}

//----------------------------------------------------------------------------------------------------------------------
void DebugRenderSystemShutdown()
{
}

//----------------------------------------------------------------------------------------------------------------------
void DebugRenderSetVisible()
{
//	DebugRenderConfig debugRenderConfig;
//	debugRenderConfig.m_startHidden = false;
}

//----------------------------------------------------------------------------------------------------------------------
void DebugRenderSetHidden()
{
//	DebugRenderConfig debugRenderConfig;
//	debugRenderConfig.m_startHidden = true;
}

//----------------------------------------------------------------------------------------------------------------------
void DebugRenderClear()
{
	// Delete all objects
	s_theDebugRenderSystem->m_worldObjectList.clear();
	s_theDebugRenderSystem->m_screenObjectList.clear();
}

//----------------------------------------------------------------------------------------------------------------------
void DebugRenderBeginFrame()
{
}

//----------------------------------------------------------------------------------------------------------------------
void DebugRenderWorld( Camera const& camera )
{
	s_theDebugRenderSystem->RenderWorld( camera );
}

//----------------------------------------------------------------------------------------------------------------------
void DebugRenderScreen( Camera const& camera )
{
	s_theDebugRenderSystem->RenderScreen( camera );
}

//----------------------------------------------------------------------------------------------------------------------
void DebugRenderEndFrame()
{
	s_theDebugRenderSystem->Update();
}

//----------------------------------------------------------------------------------------------------------------------
void DebugAddWorldPoint( Vec3 const& pos, float radius, float duration, Rgba8 const& startColor, Rgba8 const& endColor, DebugRenderMode mode )
{
	ObjectsToRender point( duration );
	float numSlices		  = 32.0f;
	float numStacks		  = 16.0f;
	Vec3 newPos			  = Vec3( pos.x, pos.y, pos.z );
	point.m_currentColor  = startColor;
	point.m_startColor    = startColor;
	point.m_endColor	  = endColor;
	point.m_mode		  = mode;
	
	AddVertsForSphere3D( point.m_vertexes, newPos, radius, numSlices, numStacks, point.m_currentColor );
	s_theDebugRenderSystem->m_worldObjectList.push_back( point );
}

//----------------------------------------------------------------------------------------------------------------------
void DebugAddWorldLine( Vec3 const& start, Vec3 const& end, float radius, float duration, Rgba8 const& startColor, Rgba8 const& endColor, DebugRenderMode mode )
{
	ObjectsToRender cylinder( duration );
	cylinder.m_currentColor = startColor;
	cylinder.m_startColor   = startColor;
	cylinder.m_endColor	    = endColor;
	cylinder.m_mode		    = mode;

	AddVertsForCylinder3D( cylinder.m_vertexes, start, end, radius, cylinder.m_currentColor );
	s_theDebugRenderSystem->m_worldObjectList.push_back( cylinder );
}

//----------------------------------------------------------------------------------------------------------------------
void DebugAddWorldWireCylinder( Vec3 const& base, Vec3 const& top, float radius, float duration, Rgba8 const& startColor, Rgba8 const& endColor, DebugRenderMode mode )
{
	ObjectsToRender cylinder( duration );
	cylinder.m_isWireframe	= true;
	cylinder.m_currentColor = startColor;
	cylinder.m_startColor	= startColor;
	cylinder.m_endColor		= endColor;
	cylinder.m_mode			= mode;

	AddVertsForCylinder3D( cylinder.m_vertexes, base, top, radius, cylinder.m_currentColor );
	s_theDebugRenderSystem->m_worldObjectList.push_back( cylinder );
}

//----------------------------------------------------------------------------------------------------------------------
void DebugAddWorldWireSphere( Vec3 const& center, float radius, float duration, Rgba8 const& startColor, Rgba8 const& endColor, DebugRenderMode mode )
{
	float numSlices		= 32.0f;
	float numStacks		= 16.0f;

	ObjectsToRender sphere( duration );
	sphere.m_isWireframe	= true;
	sphere.m_currentColor	= startColor;
	sphere.m_startColor		= startColor;
	sphere.m_endColor		= endColor;
	sphere.m_mode			= mode;

	AddVertsForSphere3D( sphere.m_vertexes, center, radius, numSlices, numStacks, sphere.m_currentColor );
//	s_theDebugRenderSystem->m_debugRenderConfig.m_renderer->SetRasterizerMode( RasterizerMode::WIREFRAME_CULL_NONE );
	s_theDebugRenderSystem->m_worldObjectList.push_back( sphere );
}

//----------------------------------------------------------------------------------------------------------------------
void DebugAddWorldArrow( Vec3 const& start, Vec3 const& end, float radius, float duration, Rgba8 const& startColor, Rgba8 const& endColor, DebugRenderMode mode )
{
	// Initialize object
	ObjectsToRender arrow( duration );
	arrow.m_currentColor = startColor;
	arrow.m_startColor   = startColor;
	arrow.m_endColor     = endColor;
	arrow.m_mode	     = mode;

	// Add verts
//	AddVertsForCone3D( arrow.m_vertexes, start, end, radius, startColor, AABB2::ZERO_TO_ONE, 32 );
	AddVertsForArrow3D( arrow.m_vertexes, start, end, radius, arrow.m_currentColor );
	s_theDebugRenderSystem->m_worldObjectList.push_back( arrow );
}

//----------------------------------------------------------------------------------------------------------------------
void DebugAddWorldText( std::string const& text, Mat44 const& transform, float textHeight, Vec2 const& alignment, float duration, Rgba8 const& startColor, Rgba8 const& endColor, DebugRenderMode mode )
{
	// Initialize object
	ObjectsToRender worldText( duration );
	worldText.m_currentColor = startColor;
	worldText.m_startColor	 = startColor;
	worldText.m_endColor	 = endColor;
	worldText.m_mode		 = mode;	

	// AddVertsForTextInBox3D, transform, set texture, then add to objectList 
	float cellAspect = 1.0f;
	worldText.m_textFont->AddVertsForText3D( worldText.m_vertexes, textHeight, text, worldText.m_currentColor, cellAspect, alignment );

	TransformVertexArray3D( worldText.m_vertexes, transform );

	worldText.m_texture = &worldText.m_textFont->GetTexture();
	s_theDebugRenderSystem->m_worldObjectList.push_back( worldText );
}

//----------------------------------------------------------------------------------------------------------------------
void DebugAddWorldBillboardText( std::string const& text, Vec3 const& origin, float textHeight, Vec2 const& alignment, float duration, Rgba8 const& startColor, Rgba8 const& endColor, DebugRenderMode mode )
{
	// Initialize object
	ObjectsToRender billboardText( duration );
	billboardText.m_currentColor			= startColor;
	billboardText.m_startColor				= startColor;
	billboardText.m_endColor				= endColor;
	billboardText.m_isBillboardFullCamOppo	= true;
	billboardText.m_position				= origin;
	billboardText.m_mode					= mode;
	float cellAspect						= 1.0f;

	// AddVertsForTextInBox3D, assign m_texture, add to objectList
	billboardText.m_textFont->AddVertsForText3D( billboardText.m_vertexes, textHeight, text, billboardText.m_currentColor, cellAspect, alignment );
	
	billboardText.m_texture = &billboardText.m_textFont->GetTexture();
	s_theDebugRenderSystem->m_worldObjectList.push_back( billboardText );
}

//----------------------------------------------------------------------------------------------------------------------
void DebugAddScreenText( std::string const& text, Vec2 const& position, float cellHeight, Vec2 const& alignment, float duration, Rgba8 const& startColor, Rgba8 const& endColor )
{
	// Initialize object
	ObjectsToRender screenText( duration );
	screenText.m_currentColor	= startColor;
	screenText.m_startColor		= startColor;
	screenText.m_endColor		= endColor;

	// Set bounds of textbox
	AABB2 box		= AABB2( Vec2(0.0f, 0.0f), Vec2(0.0f, 0.0f) );
//	AABB2 box		= AABB2( Vec2(0.0f, 0.0f), Vec2(position.x, position.y) );
//	box.SetCenter( Vec2(position.x + halfwidth, position.y) );
	box.SetCenter( Vec2(position.x, position.y) );
	float aspect = 1.0f;

	// Get font and addVertsForTextInBox2D
	screenText.m_textFont->AddVertsForTextInBox2D( screenText.m_vertexes, box, cellHeight, text, screenText.m_currentColor, aspect, alignment );
	screenText.m_texture = &screenText.m_textFont->GetTexture();
	s_theDebugRenderSystem->m_screenObjectList.push_back( screenText );
}

//----------------------------------------------------------------------------------------------------------------------
void DebugAddMessage( std::string const& text, float duration, Rgba8 const& startColor, Rgba8 const& endColor )
{
	UNUSED( text	);
	UNUSED(	duration 	);
	UNUSED(	startColor );
	UNUSED(	endColor	);
}

//----------------------------------------------------------------------------------------------------------------------
bool Command_DebugRenderClear( EventArgs& args )
{
	UNUSED( args );
	DebugRenderClear();
	return true;
}

//----------------------------------------------------------------------------------------------------------------------
bool Command_DebugRenderToggle( EventArgs& args )
{
	UNUSED( args );
	if ( isRenderVisible )
	{
		DebugRenderSetHidden();
	}
	else
	{
		DebugRenderSetVisible();
	}
	return true;
}
