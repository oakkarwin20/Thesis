#pragma once

#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/EventSystem.hpp"

#include <string>

//----------------------------------------------------------------------------------------------------------------------
struct Vec2;
struct Vec3;
struct Mat44;
class  Renderer;
class  Camera;

//----------------------------------------------------------------------------------------------------------------------
enum class DebugRenderMode
{
	ALWAYS,
	USE_DEPTH,
	X_RAY,
};

//----------------------------------------------------------------------------------------------------------------------
struct DebugRenderConfig
{
	Renderer*	m_renderer		= nullptr;
	bool		m_startHidden	= false;
};

//----------------------------------------------------------------------------------------------------------------------
// Setup
void DebugRenderSystemStartup( DebugRenderConfig const& config );
void DebugRenderSystemShutdown();

//----------------------------------------------------------------------------------------------------------------------
// Control 
void DebugRenderSetVisible();
void DebugRenderSetHidden();
void DebugRenderClear();

//----------------------------------------------------------------------------------------------------------------------
// Output
void DebugRenderBeginFrame();
void DebugRenderWorld( Camera const& camera );
void DebugRenderScreen( Camera const& camera );
void DebugRenderEndFrame();

//----------------------------------------------------------------------------------------------------------------------
//Geometry
void DebugAddWorldPoint( Vec3 const& pos, 
	float radius, float duration, 
	Rgba8 const& startColor = Rgba8::WHITE, 
	Rgba8 const& endColor	= Rgba8::WHITE, 
	DebugRenderMode mode	= DebugRenderMode::USE_DEPTH );

void DebugAddWorldLine( Vec3 const& start, Vec3 const& end, 
	float radius, float duration, 
	Rgba8 const& startColor = Rgba8::WHITE, 
	Rgba8 const& endColor	= Rgba8::WHITE,
	DebugRenderMode mode	= DebugRenderMode::USE_DEPTH );

void DebugAddWorldWireCylinder( Vec3 const& base, Vec3 const& top, 
	float radius, float duration, 
	Rgba8 const& startColor = Rgba8::WHITE,
	Rgba8 const& endColor	= Rgba8::WHITE, 
	DebugRenderMode mode	= DebugRenderMode::USE_DEPTH );

void DebugAddWorldWireSphere( Vec3 const& center, 
	float radius, float duration, 
	Rgba8 const& startColor = Rgba8::WHITE,
	Rgba8 const& endColor	= Rgba8::WHITE,
	DebugRenderMode mode	= DebugRenderMode::USE_DEPTH );

void DebugAddWorldArrow( Vec3 const& start, Vec3 const& end, 
	float radius, float duration,
	Rgba8 const& startColor = Rgba8::WHITE,
	Rgba8 const& endColor	= Rgba8::WHITE,
	DebugRenderMode mode	= DebugRenderMode::USE_DEPTH );

void DebugAddWorldText( std::string const& text, 
	Mat44 const& transform, float textHeight, 
	Vec2 const& alignment, float duration, 
	Rgba8 const& startColor = Rgba8::WHITE,
	Rgba8 const& endColor	= Rgba8::WHITE,
	DebugRenderMode mode	= DebugRenderMode::USE_DEPTH );

void DebugAddWorldBillboardText( std::string const& text,
	Vec3 const& origin, float textHeight,
	Vec2 const& alignment, float duration, 
	Rgba8 const& startColor = Rgba8::WHITE,
	Rgba8 const& endColor	= Rgba8::WHITE,
	DebugRenderMode mode	= DebugRenderMode::USE_DEPTH );

void DebugAddScreenText( std::string const& text, 
	Vec2 const& position, float cellHeight, 
	Vec2 const& alignment, float duration,
	Rgba8 const& startColor = Rgba8::WHITE,
	Rgba8 const& endColor	= Rgba8::WHITE );

void DebugAddMessage( std::string const& text, 
	float duration,
	Rgba8 const& startColor = Rgba8::WHITE,
	Rgba8 const& endColor	= Rgba8::WHITE );

//----------------------------------------------------------------------------------------------------------------------
// Console commands
bool Command_DebugRenderClear( EventArgs& args );
bool Command_DebugRenderToggle( EventArgs& args );