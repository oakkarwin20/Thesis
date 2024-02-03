#pragma once

#include "Engine/Input/XboxController.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Math/IntVec2.hpp"

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
extern unsigned char const KEYCODE_F1;
extern unsigned char const KEYCODE_F2;
extern unsigned char const KEYCODE_F3;
extern unsigned char const KEYCODE_F4;
extern unsigned char const KEYCODE_F5;
extern unsigned char const KEYCODE_F6;
extern unsigned char const KEYCODE_F7;
extern unsigned char const KEYCODE_F8;
extern unsigned char const KEYCODE_F9;
extern unsigned char const KEYCODE_F10;
extern unsigned char const KEYCODE_F11;
extern unsigned char const KEYCODE_ESC;
extern unsigned char const KEYCODE_SPACE_BAR;
extern unsigned char const KEYCODE_ENTER;
extern unsigned char const KEYCODE_UPARROW;
extern unsigned char const KEYCODE_DOWNARROW;
extern unsigned char const KEYCODE_LEFTARROW;
extern unsigned char const KEYCODE_RIGHTARROW;
extern unsigned char const KEYCODE_LEFT_MOUSE;
extern unsigned char const KEYCODE_RIGHT_MOUSE;
extern unsigned char const KEYCODE_TILDE;
extern unsigned char const KEYCODE_LEFTBRACKET;
extern unsigned char const KEYCODE_RIGHTBRACKET;
extern unsigned char const KEYCODE_BACKSPACE;
extern unsigned char const KEYCODE_INSERT;
extern unsigned char const KEYCODE_DELETE;
extern unsigned char const KEYCODE_HOME;
extern unsigned char const KEYCODE_END;
extern unsigned char const KEYCODE_SHIFT;
extern unsigned char const KEYCODE_CONTROL;
extern unsigned char const KEYCODE_COMMA;
extern unsigned char const KEYCODE_PERIOD;
extern unsigned char const KEYCODE_SEMICOLON;
extern unsigned char const KEYCODE_SINGLEQUOTE;


//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
constexpr int NUM_KEYCODES			= 256;
constexpr int NUM_XBOX_CONTROLLER	= 4;


//----------------------------------------------------------------------------------------------------------------------
struct MouseState 
{
	IntVec2 m_cursorClientPosition;
	IntVec2 m_cursorClientDelta;

	bool m_currentHidden = false;
	bool m_desiredHidden = false;

	bool m_currentRelative = false;
	bool m_desiredRelative = false;
};

//----------------------------------------------------------------------------------------------------------------------
struct InputSystemConfig
{
};

//----------------------------------------------------------------------------------------------------------------------
class InputSystem 
{
public:
	InputSystem(InputSystemConfig const& config);
	~InputSystem();

	void StartUp();
	void BeginFrame();
	void EndFrame();
	void ShutDown();

	bool WasKeyJustPressed				( unsigned char keyCode );
	bool WasKeyJustReleased				( unsigned char keyCode );
	bool IsKeyDown						( unsigned char keyCode );
	void HandleKeyPressed				( unsigned char keyCode );
	void HandleKeyReleased				( unsigned char keyCode );
	XboxController const& GetController	( int controllerID );

	static bool Event_KeyPressed ( EventArgs& args );
	static bool Event_KeyReleased( EventArgs& args );

	//----------------------------------------------------------------------------------------------------------------------
	// Mouse / Cursor functions
	//----------------------------------------------------------------------------------------------------------------------
	void SetCursorMode( bool hidden, bool relative );

	// Returns the current frame cursor delta in pixels, relative to the client region
	Vec2 GetCursorClientDelta()	const;
	
	// Returns the cursor position in pixels, relative to the client region
	Vec2 GetCursorClientPosition() const;

	// Returns the cursor position, normalized to the range [0,1], relative to the client region,
	// with the y-axis inverted to map from Windows conventions to game screen camera conventions
	Vec2 GetCursorNormalizedPosition() const;

	void GetCurrentLocalPos();

protected:
	KeyButtonState m_keyStates	 [ 256 ];
	XboxController m_controllers [ NUM_XBOX_CONTROLLER ];

	InputSystemConfig		m_config;
	static InputSystem*		s_theInputSystem;

public:
	MouseState	m_mouseState;
};