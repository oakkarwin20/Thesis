#pragma once

#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include <iostream>

//----------------------------------------------------------------------------------------------------------------------
class InputSystem;

//----------------------------------------------------------------------------------------------------------------------
struct WindowConfig 
{
	InputSystem*		m_inputSystem		= nullptr;
	std::string			m_windowTitle		= "Untitled App";
	float				m_clientAspect		= 1.0f;
	bool				m_isFullScreen		= true;
	IntVec2				m_windowSize		= IntVec2( -1, -1 );
	IntVec2				m_windowPosition	= IntVec2( -1, -1 );
};

//----------------------------------------------------------------------------------------------------------------------
class Window 
{
public:
	Window(WindowConfig const& config);
	~Window();

	void Startup();
	void Shutdown();
	void BeginFrame();
	void EndFrame();

	WindowConfig const&	GetConfig() const			{ return m_config; }
	static Window* GetWindow()						{ return s_mainWindow; }
 
	void*		GetHwnd() const;
	IntVec2		GetClientDimensions() const;

	Vec2 GetNormalizedCursorPos() const;

	bool HasFocus();

	std::string GetXMLFileName(std::string const& directoryPath);

protected:
	void		CreateOSWindow();
	void		RunMessagePump();

protected:
	WindowConfig		m_config;
	static Window*		s_mainWindow;
	void*				m_hwnd;
	IntVec2				m_clientDimensions;
};