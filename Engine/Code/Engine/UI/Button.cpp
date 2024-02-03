#include "Button.hpp"

#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Input/InputSystem.hpp"


//----------------------------------------------------------------------------------------------------------------------
Button::Button( AABB2 const& bounds, std::string text1, std::string text2  )
{
	m_bounds = bounds;
	m_text1	 = text1;
	m_text2	 = text2;
}


//----------------------------------------------------------------------------------------------------------------------
Button::~Button()
{
}


//----------------------------------------------------------------------------------------------------------------------
void Button::Update()
{
	// Get mouse position
	// Check if mousePos isInsideAABB2
	//		True: wasLeftMouseJustPressed
	//		False: Do nothing
//	Vec2 cursorPos_screenSpace2 = g_theInput->GetCursorClientPosition();
}


//----------------------------------------------------------------------------------------------------------------------
bool Button::IsMouseHovered( Vec2 const& cursorPos_screenSpace )
{
	// If the mouse is highlighting this button, check if the mouse was clicked 
	if ( m_bounds.IsPointInside( cursorPos_screenSpace ) )
	{
		m_isHighlighted = true;
		return true;
	}
	else
	{
		return false;
	}


/*
	// If the mouse is highlighting this button, check if the mouse was clicked 
	if ( m_bounds.IsPointInside( cursorPos_screenSpace ) )
	{
		m_isHighlighted = true;
		if ( g_theInput->WasKeyJustPressed( KEYCODE_LEFT_MOUSE ) )
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
*/
}


//----------------------------------------------------------------------------------------------------------------------
void Button::SetIsHighlighted( bool const& isHighlighted )
{
	m_isHighlighted = isHighlighted;
}


//----------------------------------------------------------------------------------------------------------------------
bool Button::GetIsHighlighted()
{
	return m_isHighlighted;
}


//----------------------------------------------------------------------------------------------------------------------
void Button::ToggleIsHighlighted()
{
	m_isHighlighted = !m_isHighlighted;
}