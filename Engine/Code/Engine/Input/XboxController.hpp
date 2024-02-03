#pragma once
#include "Engine/Input/KeyButtonState.hpp"
#include "Engine/Input/AnalogJoystick.hpp"

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
enum XboxButtonID
{
	INVALID					= -1,
	BUTTON_A				=  0,
	BUTTON_B				=  1,
	BUTTON_X				=  2,
	BUTTON_Y				=  3,
							  
	DPAD_UP					=  4,
	DPAD_DOWN				=  5,
	DPAD_LEFT				=  6,
	DPAD_RIGHT				=  7,
							  
	BUTTON_START			=  8,
	BUTTON_SELECT			=  9,
					  
	LEFT_JOYSTICK_BUTTON	=  10,
	RIGHT_JOYSTICK_BUTTON	=  11,
	LEFT_SHOULDER			=  12,
	RIGHT_SHOULDER			=  13,

	NUM						=  14
};

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
class XboxController 
{
	friend class InputSystem;		// Question what is friend class?

public:
	XboxController();
	~XboxController();

	bool					isConnected() const;
	int						GetControllerID() const;
	AnalogJoystick const&	GetLeftJoyStick() const;
	AnalogJoystick const&	GetRightJoyStick() const;
	float					GetLeftTrigger() const;
	float					GetRightTrigger() const;
	KeyButtonState const&	GetButton( XboxButtonID buttonID ) const;
	bool					IsButtonDown( XboxButtonID buttonID ) const;
	bool					WasButtonJustPressed( XboxButtonID buttonID ) const;
	bool					WasButtonJustReleased( XboxButtonID buttonID ) const;

	bool m_currentState = false;
	bool m_previousState = false;

private:
	void					Update();
	void					Reset();
	void					UpdateJoystick( AnalogJoystick& out_joystick, short rawX, short rawY );
	void					UpdateTrigger( float& out_triggerValue, unsigned char rawValue);
	void					UpdateButton( XboxButtonID buttonID, unsigned short buttonFlags, unsigned short buttonFlag );

private:
	int						m_id				= -1;
	bool					m_isConnected		= false;
	float					m_leftTrigger		= 0.0f;
	float					m_rightTrigger		= 0.0f;

	KeyButtonState m_buttons[ XboxButtonID::NUM ];

	AnalogJoystick			m_leftJoyStick;
	AnalogJoystick			m_rightJoyStick;
};