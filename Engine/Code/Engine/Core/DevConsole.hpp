#pragma once

#include "Engine/Core/Clock.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec2.hpp"

#include <string>
#include <vector>
#include <mutex>

//----------------------------------------------------------------------------------------------------------------------
class	Renderer;
class	Camera;
class	BitmapFont;
class	Stopwatch;
struct	AABB2;

//----------------------------------------------------------------------------------------------------------------------
class	DevConsole;
extern	DevConsole* g_theDevConsole;

//----------------------------------------------------------------------------------------------------------------------
struct DevConsoleLine
{
	Rgba8			m_color;
	std::string		m_text;
};

//----------------------------------------------------------------------------------------------------------------------
struct DevConsoleConfig
{
	Renderer*		m_renderer							= nullptr;
	Camera*			m_camera							= nullptr;
	std::string		m_fontName							= "SquirrelFixedFont";
	float			m_fontAspect						=  0.7f;
	int				m_maxNumLinesDisplayedOnScreen		=	 10;					// Excluding the box for typing
	float			m_cellHeight						=  3.0f;
	float			m_cellWidth							= m_cellHeight * m_fontAspect;
	int				m_maxCommandHistory					= 128;
};

//----------------------------------------------------------------------------------------------------------------------
class DevConsole
{
public:
	DevConsole( DevConsoleConfig const& config );
	~DevConsole();

	void Startup();
	void Shutdown();
	void BeginFrame();
	void EndFrame();

	void Execute( std::string const& consoleCommandText );
	void AddLine( Rgba8 const& color, std::string const& text );
	void Render ( AABB2 const& bounds );
	void ToggleOpen();

//----------------------------------------------------------------------------------------------------------------------
// 	UNUSED
//	static const Rgba8 ERROR;
//	static const Rgba8 WARNING;
//	static const Rgba8 INFO_MAJOR;
//	static const Rgba8 INFO_MINOR;
//	static const Rgba8 COMMAND_ECHO;
//	static const Rgba8 INPUT_TEXT;
//	static const Rgba8 INPUT_CARET;

	static bool Event_KeyPressed		( EventArgs& args );
	static bool Event_CharInput			( EventArgs& args );
	static bool Command_Clear			( EventArgs& args );
	static bool Command_Help			( EventArgs& args );
	static bool Command_Echo			( EventArgs& args );

protected:
	DevConsoleConfig				m_config;
	std::vector<DevConsoleLine>		m_lines;
	std::string						m_inputText;
	int								m_caretPosition			= 0;
	bool							m_caretIsVisible		= true;
	Stopwatch*						m_caretStopwatch;
	std::vector<std::string>		m_commandHistory;
	int								m_historyIndex			= -1;

public:
	std::atomic<bool>				m_isOpen				= false;
	std::mutex						m_devConsoleMutex;
};