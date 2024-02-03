#include "Engine/Input/XboxController.hpp"
#include <Windows.h>
#include <Xinput.h>
#include "Engine/Math/MathUtils.hpp"

#pragma comment( lib, "xinput9_1_0" ) 

XboxController::XboxController()
{
}

XboxController::~XboxController()
{
}

bool XboxController::isConnected() const
{
	return m_isConnected;
}

int XboxController::GetControllerID() const
{
	return m_id;
}

AnalogJoystick const& XboxController::GetLeftJoyStick() const
{
	return m_leftJoyStick;
}

AnalogJoystick const& XboxController::GetRightJoyStick() const
{
	return m_rightJoyStick;
}

float XboxController::GetLeftTrigger() const
{
	return m_leftTrigger;
}

float XboxController::GetRightTrigger() const
{
	return m_rightTrigger;
}

KeyButtonState const& XboxController::GetButton(XboxButtonID buttonID) const
{
	// TODO: insert return statement here
	return m_buttons[buttonID];
}

bool XboxController::IsButtonDown(XboxButtonID buttonID) const
{
	return m_buttons[buttonID].m_isPressed;
}

bool XboxController::WasButtonJustPressed(XboxButtonID buttonID) const
{
	return (m_buttons[buttonID].m_wasPressedLastFrame) == false && (m_buttons[buttonID].m_isPressed == true);
}

bool XboxController::WasButtonJustReleased(XboxButtonID buttonID) const
{
	return m_buttons[buttonID].m_isPressed == false;
}

void XboxController::Update()
{
	// check if controller is connected, if not, reset joysticks, buttons, triggers, etc
	// if controller is connected, update state of 14 buttons, 2 triggers, and 2 joysticks

	XINPUT_STATE xboxControllerState = {};

	DWORD errorStatus = XInputGetState( m_id, &xboxControllerState );
	if (errorStatus != ERROR_SUCCESS)
	{
		m_isConnected = false;
		Reset();
		return;
	}

	m_isConnected = true;

	XINPUT_GAMEPAD state = xboxControllerState.Gamepad;

	// Update Buttons
	UpdateButton( BUTTON_A,					state.wButtons, XINPUT_GAMEPAD_A);
	UpdateButton( BUTTON_B,					state.wButtons, XINPUT_GAMEPAD_B);
	UpdateButton( BUTTON_X,					state.wButtons, XINPUT_GAMEPAD_X);
	UpdateButton( BUTTON_Y,					state.wButtons, XINPUT_GAMEPAD_Y);

	UpdateButton( DPAD_UP,					state.wButtons, XINPUT_GAMEPAD_DPAD_UP);
	UpdateButton( DPAD_DOWN,				state.wButtons, XINPUT_GAMEPAD_DPAD_DOWN);
	UpdateButton( DPAD_LEFT,				state.wButtons, XINPUT_GAMEPAD_DPAD_LEFT);
	UpdateButton( DPAD_RIGHT,				state.wButtons, XINPUT_GAMEPAD_DPAD_RIGHT);

	UpdateButton( BUTTON_START,				state.wButtons, XINPUT_GAMEPAD_START);
	UpdateButton( BUTTON_SELECT,			state.wButtons, XINPUT_GAMEPAD_BACK);

	UpdateButton( LEFT_JOYSTICK_BUTTON,		state.wButtons, XINPUT_GAMEPAD_LEFT_THUMB);
	UpdateButton( RIGHT_JOYSTICK_BUTTON,	state.wButtons, XINPUT_GAMEPAD_RIGHT_THUMB);
				  
	UpdateButton( LEFT_SHOULDER,			state.wButtons, XINPUT_GAMEPAD_LEFT_SHOULDER);
	UpdateButton( RIGHT_SHOULDER,			state.wButtons, XINPUT_GAMEPAD_RIGHT_SHOULDER);

	// Update Joysticks
	UpdateJoystick( m_leftJoyStick, state.sThumbLX, state.sThumbLY );
	UpdateJoystick( m_rightJoyStick, state.sThumbRX, state.sThumbRY );

	// Update Triggers
	UpdateTrigger(m_leftTrigger, state.bLeftTrigger);
	UpdateTrigger(m_rightTrigger, state.bRightTrigger);
}

void XboxController::Reset()
{
	m_leftTrigger		= 0.0f;
	m_rightTrigger		= 0.0f;

	//		#ToDo Actually learn how joysticks are being reset
	//		#ToDo How to call default constructor instead?
	//m_leftJoyStick.Reset();
	//m_rightJoyStick.Reset();

	m_leftJoyStick	= AnalogJoystick();
	m_rightJoyStick = AnalogJoystick();

	//		#ToDo How does this work?
	for (int i = 0; i < XboxButtonID::NUM; i++)
	{
		m_buttons[i].m_isPressed = false;
		m_buttons[i].m_wasPressedLastFrame = false;
	}

	/* Keeping for reference and learning purposes
	* 
	//m_leftJoyStick.m_rawPosition = Vec2( 0.0f, 0.0f );
	//m_rightJoyStick		= 0.0f;

	/*
	GetButton(BUTTON_A ).m_isPressed == false;
	GetButton(BUTTON_B ).m_isPressed == false;
	GetButton(BUTTON_X ).m_isPressed == false;
	GetButton(BUTTON_Y ).m_isPressed == false;

	GetButton(DPAD_UP ).m_isPressed == false;
	GetButton(DPAD_DOWN ).m_isPressed == false;
	GetButton(DPAD_LEFT ).m_isPressed == false;
	GetButton(DPAD_RIGHT ).m_isPressed == false;
	
	GetButton(START ).m_isPressed == false;
	GetButton(SELECT ).m_isPressed == false;

	GetButton(LEFT_JOYSTICK_BUTTON ).m_isPressed == false;
	GetButton(RIGHT_JOYSTICK_BUTTON ).m_isPressed == false;
	
	GetButton(LEFT_SHOULDER ).m_isPressed == false;
	GetButton(RIGHT_SHOULDER ).m_isPressed == false;
	*/
}

void XboxController::UpdateJoystick(AnalogJoystick& out_joystick, short rawX, short rawY)
{
	Vec2 normalizedVec = Vec2( static_cast<float>(rawX) / static_cast<float>(MAXSHORT), static_cast<float>(rawY) / static_cast<float>(MAXSHORT) );

	out_joystick.UpdatePosition( normalizedVec.x, normalizedVec.y );
	
	//UpdateJoystick( m_leftJoyStick, rawX, rawY );
	//UpdateJoystick( m_rightJoyStick, rawX, rawY);
}

void XboxController::UpdateTrigger(float& out_triggerValue, unsigned char rawValue)			// Question, did not know float& out_triggerValue meant xbox triggers
{
	out_triggerValue = RangeMap( rawValue, 0.0f, 255.0f, 0.0f, 1.0f );
	
	//m_leftTrigger = RangeMap( rawValue, 0.0f, 1.0f, 0.0f, 255.0f );
	//m_rightTrigger = RangeMap( rawValue, 0.0f, 1.0f, 0.0f, 255.0f );
}

void XboxController::UpdateButton(XboxButtonID buttonID, unsigned short buttonFlags, unsigned short buttonFlag)		// Question check if this function is written correctly
{
	m_buttons[buttonID].m_wasPressedLastFrame = m_buttons[buttonID].m_isPressed;

	bool buttonPressed = ( buttonFlags & buttonFlag );

	m_buttons[buttonID].m_isPressed = buttonPressed;
}
