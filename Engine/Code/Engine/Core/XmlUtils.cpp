#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Core/Rgba8.hpp"

#include <string>

//----------------------------------------------------------------------------------------------------------------------
int ParseXmlAttribute( XmlElement const& element, char const* attributeName, int defaultValue )
{
	// gets value at index attributeName
	char const* attributeValueAsText = element.Attribute( attributeName );
	int attributeValueAsValue = defaultValue;
	if ( attributeValueAsText )
	{
		attributeValueAsValue = atoi(attributeValueAsText);
	}

	return attributeValueAsValue;
}

//----------------------------------------------------------------------------------------------------------------------
char ParseXmlAttribute( XmlElement const& element, char const* attributeName, char defaultValue )
{
	char const* attributeValueAsText = element.Attribute( attributeName );
	char attributeValueAsValue = defaultValue;
	if ( attributeValueAsText )
	{
		attributeValueAsValue = *attributeValueAsText;
	}

	return attributeValueAsValue;
}

//----------------------------------------------------------------------------------------------------------------------
bool ParseXmlAttribute( XmlElement const& element, char const* attributeName, bool defaultValue )
{
	char const* attributeValueAsText = element.Attribute( attributeName );
	bool attributeValueAsValue = defaultValue;
	if ( attributeValueAsText )
	{
		if ( strcmp(attributeValueAsText, "true") == 0 )
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	return attributeValueAsValue;
}

//----------------------------------------------------------------------------------------------------------------------
float ParseXmlAttribute( XmlElement const& element, char const* attributeName, float defaultValue )
{
	char const* attributeValueAsText = element.Attribute( attributeName );
	float attributeValueAsValue = defaultValue;
	if ( attributeValueAsText )
	{
		attributeValueAsValue = static_cast<float>( atof(attributeValueAsText) );
	}

	return attributeValueAsValue;;
}

//----------------------------------------------------------------------------------------------------------------------
Rgba8 ParseXmlAttribute( XmlElement const& element, char const* attributeName, Rgba8 const& defaultValue )
{
	char const* attributeValueAsText = element.Attribute( attributeName );
	Rgba8 attributeValueAsValue = defaultValue;
	if ( attributeValueAsText )
	{
		attributeValueAsValue.SetFromText( attributeValueAsText );
	}

	return attributeValueAsValue;
}

//----------------------------------------------------------------------------------------------------------------------
Vec2 ParseXmlAttribute( XmlElement const& element, char const* attributeName, Vec2 const& defaultValue )
{
	char const* attributeValueAsText = element.Attribute( attributeName );
	Vec2 attributeValueAsValue = defaultValue;
	if ( attributeValueAsText )
	{
		attributeValueAsValue.SetFromText( attributeValueAsText );
	}

	return attributeValueAsValue;
}

//----------------------------------------------------------------------------------------------------------------------
IntVec2 ParseXmlAttribute( XmlElement const& element, char const* attributeName, IntVec2 const& defaultValue )
{
	char const* attributeValueAsText = element.Attribute( attributeName );
	IntVec2 attributeValueAsValue = defaultValue;
	if ( attributeValueAsText )
	{
		attributeValueAsValue.SetFromText( attributeValueAsText );
	}

	return attributeValueAsValue;
}

//----------------------------------------------------------------------------------------------------------------------
std::string ParseXmlAttribute( XmlElement const& element, char const* attributeName, std::string const& defaultValue )
{
	char const* attributeValueAsText = element.Attribute( attributeName );
	std::string attributeValueAsValue = defaultValue;
	if ( attributeValueAsText )
	{
		attributeValueAsValue = attributeValueAsText;
	}

	return attributeValueAsValue;
}

//----------------------------------------------------------------------------------------------------------------------
Strings ParseXmlAttribute( XmlElement const& element, char const* attributeName, Strings const& defaultValue )
{
	char const* attributeValueAsText = element.Attribute( attributeName );
	Strings attributeValueAsValue = defaultValue;

	if ( attributeValueAsText )
	{
		SplitStringOnDelimiter( attributeValueAsText, ',' );
	}

	return attributeValueAsValue;
}

//----------------------------------------------------------------------------------------------------------------------
FloatRange ParseXmlAttribute( XmlElement const& element, char const* attributeName, FloatRange const& defaultValue )
{
	char const* attributeValueAsText  = element.Attribute( attributeName );
	FloatRange attributesValueAsValue = defaultValue;

	if ( attributeValueAsText )
	{
		attributesValueAsValue.SetFromText( attributeValueAsText );
	}
	return attributesValueAsValue;
}

//----------------------------------------------------------------------------------------------------------------------
Vec3 ParseXmlAttribute( XmlElement const& element, char const* attributeName, Vec3 const& defaultValue )
{
	char const* attributeValueAsText = element.Attribute( attributeName );
	Vec3 attributeValueAsValue		 = defaultValue;
	if ( attributeValueAsText )
	{
		attributeValueAsValue.SetFromText( attributeValueAsText );
	}

	return attributeValueAsValue;
}

//----------------------------------------------------------------------------------------------------------------------
EulerAngles ParseXmlAttribute( XmlElement const& element, char const* attributeName, EulerAngles const& defaultValue )
{
	char const* attributeValueAsText = element.Attribute( attributeName );
	EulerAngles attributeValueAsValue = defaultValue;
	if ( attributeValueAsText )
	{
		attributeValueAsValue.SetFromText( attributeValueAsText );
	}

	return attributeValueAsValue;
}

//----------------------------------------------------------------------------------------------------------------------
std::string ParseXmlAttribute( XmlElement const& element, char const* attributeName, char const* defaultValue )
{
	char const* attributeValueAsText = element.Attribute( attributeName );
	std::string attributeValueAsValue = defaultValue;
	if ( attributeValueAsText )
	{
		attributeValueAsValue = attributeValueAsText;
	}

	return attributeValueAsValue;
}
