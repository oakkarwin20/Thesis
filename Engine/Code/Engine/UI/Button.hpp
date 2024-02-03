#pragma once

#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"

#include <vector>
#include <string>

//----------------------------------------------------------------------------------------------------------------------
class  InputSystem;
extern InputSystem* g_theInput;


//----------------------------------------------------------------------------------------------------------------------
class Button
{
public:
	Button( AABB2 const& bounds, std::string text1 = "", std::string text2 = "" );		// Constructor for a button with text
	~Button();

	void Update();

	bool IsMouseHovered( Vec2 const& cursorPos_screenSpace );
	void SetIsHighlighted( bool const& isHighlighted = false );
	bool GetIsHighlighted();
	void ToggleIsHighlighted();

private:
	bool			m_isHighlighted		= false;
public:
	AABB2			m_bounds;
	std::string		m_text1				= "Un-itializedText 1";
	std::string		m_text2				= "Un-itializedText 2";
	std::string		m_text3				= "Un-itializedText 3";
	std::string		m_text4				= "Un-itializedText 4";
	std::string		m_text5				= "Un-itializedText 5";
	std::string		m_text6				= "Un-itializedText 5";
};