#include "Engine/Input/InputSystem.hpp"
#include "Engine/Input/XboxController.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

#include <Windows.h>

//----------------------------------------------------------------------------------------------------------------------
InputSystem* InputSystem::s_theInputSystem = nullptr;

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
unsigned char const KEYCODE_F1			 = VK_F1; 
unsigned char const KEYCODE_F2			 = VK_F2; 
unsigned char const KEYCODE_F3			 = VK_F3; 
unsigned char const KEYCODE_F4			 = VK_F4; 
unsigned char const KEYCODE_F5			 = VK_F5; 
unsigned char const KEYCODE_F6			 = VK_F6; 
unsigned char const KEYCODE_F7			 = VK_F7; 
unsigned char const KEYCODE_F8			 = VK_F8; 
unsigned char const KEYCODE_F9			 = VK_F9; 
unsigned char const KEYCODE_F10			 = VK_F10; 
unsigned char const KEYCODE_F11			 = VK_F11; 
unsigned char const KEYCODE_ESC			 = VK_ESCAPE; 
unsigned char const KEYCODE_SPACE_BAR	 = VK_SPACE; 
unsigned char const KEYCODE_ENTER		 = VK_RETURN; 
unsigned char const KEYCODE_UPARROW		 = VK_UP; 
unsigned char const KEYCODE_DOWNARROW	 = VK_DOWN; 
unsigned char const KEYCODE_LEFTARROW	 = VK_LEFT; 
unsigned char const KEYCODE_RIGHTARROW	 = VK_RIGHT;
unsigned char const KEYCODE_LEFT_MOUSE	 = VK_LBUTTON; 
unsigned char const KEYCODE_RIGHT_MOUSE	 = VK_RBUTTON;
unsigned char const KEYCODE_TILDE		 = 0xC0;
unsigned char const KEYCODE_LEFTBRACKET	 = 0xDB;
unsigned char const KEYCODE_RIGHTBRACKET = 0xDD;
unsigned char const KEYCODE_BACKSPACE	 = VK_BACK;
unsigned char const KEYCODE_INSERT		 = VK_INSERT;			
unsigned char const KEYCODE_DELETE		 = VK_DELETE;
unsigned char const KEYCODE_HOME		 = VK_HOME;
unsigned char const KEYCODE_END			 = VK_END;
unsigned char const KEYCODE_SHIFT		 = VK_SHIFT;
unsigned char const KEYCODE_CONTROL		 = VK_CONTROL;
unsigned char const KEYCODE_COMMA		 = VK_OEM_COMMA;
unsigned char const KEYCODE_PERIOD		 = VK_OEM_PERIOD;
unsigned char const KEYCODE_SEMICOLON	 = VK_OEM_1;
unsigned char const KEYCODE_SINGLEQUOTE	 = VK_OEM_7;



//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
InputSystem::InputSystem( InputSystemConfig const& config )
	: m_config(config)
{
	s_theInputSystem = this;
}

//----------------------------------------------------------------------------------------------------------------------
InputSystem::~InputSystem()
{
}

//----------------------------------------------------------------------------------------------------------------------
void InputSystem::StartUp()
{
	for (int i = 0; i < NUM_XBOX_CONTROLLER; i++)
	{
		m_controllers[i].m_id = i;
	}

	// #CheckIfCorrect
	g_theEventSystem->SubscribeToEvent(  "KeyPressed", InputSystem::Event_KeyPressed  );
	g_theEventSystem->SubscribeToEvent( "KeyReleased", InputSystem::Event_KeyReleased );
}

//----------------------------------------------------------------------------------------------------------------------
void InputSystem::BeginFrame()
{
	// call XboxController::Update();
	for (int i = 0; i < NUM_XBOX_CONTROLLER; i++)
	{
		m_controllers[i].Update();
	}

	//----------------------------------------------------------------------------------------------------------------------
	// If currentHidden mode has changed, update currentHidden
	if ( m_mouseState.m_currentHidden != m_mouseState.m_desiredHidden )
	{
		m_mouseState.m_currentHidden = m_mouseState.m_desiredHidden;

		// Set cursor hidden mode appropriately
		if ( m_mouseState.m_currentHidden )
		{
			while ( ::ShowCursor( false ) >= 0 )
			{
			}
		}
		else
		{
			while ( ::ShowCursor( true ) < 0 )
			{
			}
		}
	}
	
	//----------------------------------------------------------------------------------------------------------------------
	// Convert cursorPos from Screen space to Client space
	Window* windowContext	= Window::GetWindow();
	HWND windowHandle		= (HWND)windowContext->GetHwnd();
	POINT cursorPos;
	::GetCursorPos( &cursorPos );						// Get cursorPos in Screen space
	::ScreenToClient( windowHandle, &cursorPos );		// Get cursorPos in Client space

	//----------------------------------------------------------------------------------------------------------------------
	// Rangemap Client bounds to [0, 1]		// Normalize
	RECT clientRect;
	::GetClientRect( windowHandle, &clientRect );

	//----------------------------------------------------------------------------------------------------------------------
	// Calculating center of Client space
	POINT clientCenter;
	clientCenter.x = ( clientRect.left + clientRect.right ) / 2;
	clientCenter.y = ( clientRect.bottom + clientRect.top ) / 2;
		
	//----------------------------------------------------------------------------------------------------------------------
	// If currentRelative mode has just changed
	if ( m_mouseState.m_currentRelative != m_mouseState.m_desiredRelative )
	{
		m_mouseState.m_currentRelative = m_mouseState.m_desiredRelative;
		
		cursorPos = clientCenter;
		m_mouseState.m_cursorClientPosition = IntVec2( clientCenter.x, clientCenter.y );		// Update m_pos to cursorPos after updated to center of Client space
		m_mouseState.m_cursorClientDelta	= IntVec2( 0, 0 );									// Reset Delta
		
		::ClientToScreen( windowHandle, &clientCenter );
		::SetCursorPos( clientCenter.x, clientCenter.y );										// Set Cursor to center of Client space
	}
	// If currentRelative mode is on 
	else if ( m_mouseState.m_currentRelative )
	{
		//----------------------------------------------------------------------------------------------------------------------
		// If Cursor moved, subtract displacement
		float cursorDelta_X = ( static_cast<float>( cursorPos.x - m_mouseState.m_cursorClientPosition.x ) );
		float cursorDelta_Y = ( static_cast<float>( cursorPos.y - m_mouseState.m_cursorClientPosition.y ) );

		m_mouseState.m_cursorClientDelta.x = (int)cursorDelta_X;				// Update Cursor m_delta to calculated delta values
		m_mouseState.m_cursorClientDelta.y = (int)cursorDelta_Y ;			// Update Cursor m_delta to calculated delta values

		::ClientToScreen( windowHandle, &clientCenter );
		::SetCursorPos( clientCenter.x, clientCenter.y );										// Set Cursor to center of Client space
		::GetCursorPos( &cursorPos );
		::ScreenToClient( windowHandle, &cursorPos );

		m_mouseState.m_cursorClientPosition = IntVec2( cursorPos.x, cursorPos.y );				// Set Cursor m_pos to whatever it's currentPos is
	}
	// If currentRelative mode is off
	else
	{
		m_mouseState.m_cursorClientPosition = IntVec2(cursorPos.x, cursorPos.y);				// Set Cursor m_pos to whatever it's currentPos is
	}
}

//----------------------------------------------------------------------------------------------------------------------
void InputSystem::EndFrame()
{
	for (int i = 0; i < NUM_KEYCODES ; i++)
	{
		m_keyStates[i].m_wasPressedLastFrame = m_keyStates[i].m_isPressed;
	}
}

//----------------------------------------------------------------------------------------------------------------------
void InputSystem::ShutDown()
{
}

//----------------------------------------------------------------------------------------------------------------------
bool InputSystem::WasKeyJustPressed(unsigned char keyCode)
{ 
	return (m_keyStates[keyCode].m_isPressed) && (!m_keyStates[keyCode].m_wasPressedLastFrame);
}

//----------------------------------------------------------------------------------------------------------------------
bool InputSystem::WasKeyJustReleased(unsigned char keyCode)
{
	return (!m_keyStates[keyCode].m_isPressed) && (m_keyStates[keyCode].m_wasPressedLastFrame);
}

//----------------------------------------------------------------------------------------------------------------------
bool InputSystem::IsKeyDown(unsigned char keyCode)
{
	return m_keyStates[keyCode].m_isPressed;
}

//----------------------------------------------------------------------------------------------------------------------
void InputSystem::HandleKeyPressed(unsigned char keyCode)
{
	m_keyStates[keyCode].m_isPressed = true;
}

//----------------------------------------------------------------------------------------------------------------------
void InputSystem::HandleKeyReleased(unsigned char keyCode)
{
	m_keyStates[keyCode].m_isPressed = false;
}

//----------------------------------------------------------------------------------------------------------------------
XboxController const& InputSystem::GetController(int controllerID)		// Question, what do I return here?
{
	return m_controllers[controllerID];
}

//----------------------------------------------------------------------------------------------------------------------
bool InputSystem::Event_KeyPressed( EventArgs& args )
{
	if ( !s_theInputSystem )
	{
		return false;
	}
	unsigned char keyCode = static_cast<unsigned char>( args.GetValue("KeyCode", -1) );
	s_theInputSystem->HandleKeyPressed( keyCode );
	return true;
}

//----------------------------------------------------------------------------------------------------------------------
bool InputSystem::Event_KeyReleased( EventArgs& args )
{
	if ( !s_theInputSystem )
	{
		return false;
	}
	unsigned char keyCode = static_cast<unsigned char>( args.GetValue( "KeyCode", -1 ) );
	s_theInputSystem->HandleKeyReleased( keyCode );
	return true;
}

//----------------------------------------------------------------------------------------------------------------------
void InputSystem::SetCursorMode( bool hidden, bool relative )
{
	m_mouseState.m_desiredHidden	= hidden;
	m_mouseState.m_desiredRelative	= relative;
}

//----------------------------------------------------------------------------------------------------------------------
Vec2 InputSystem::GetCursorClientDelta() const
{
	return Vec2((float)m_mouseState.m_cursorClientDelta.x, (float)m_mouseState.m_cursorClientDelta.y);
}

//----------------------------------------------------------------------------------------------------------------------
Vec2 InputSystem::GetCursorClientPosition() const
{
	// return negative Y to flip // Coords will no longer be upside down
	return Vec2( (float)m_mouseState.m_cursorClientPosition.x, (float)m_mouseState.m_cursorClientPosition.y );
}

//----------------------------------------------------------------------------------------------------------------------
Vec2 InputSystem::GetCursorNormalizedPosition() const
{
	Vec2 normalizedCursorClientPosition = GetCursorClientPosition();
	return normalizedCursorClientPosition;
}
