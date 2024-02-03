#pragma once

#include "Engine/Core/StringUtils.hpp"
#include "Engine/ThirdParty/TinyXML2/tinyxml2.h"

#include <string>

//----------------------------------------------------------------------------------------------------------------------
struct Rgba8;
struct Vec2;
struct Vec3;
struct EulerAngles;
struct IntVec2;
class  FloatRange;
	
typedef tinyxml2::XMLDocument			XmlDocument;
typedef tinyxml2::XMLElement			XmlElement;
typedef tinyxml2::XMLAttribute			XmlAttribute;
typedef tinyxml2::XMLError				XmlResult;
typedef tinyxml2::XMLNode				XmlNode;

//----------------------------------------------------------------------------------------------------------------------
int			ParseXmlAttribute( XmlElement const& element, char const* attributeName, int						defaultValue );
char		ParseXmlAttribute( XmlElement const& element, char const* attributeName, char						defaultValue );
bool		ParseXmlAttribute( XmlElement const& element, char const* attributeName, bool						defaultValue );
float		ParseXmlAttribute( XmlElement const& element, char const* attributeName, float						defaultValue );
Rgba8		ParseXmlAttribute( XmlElement const& element, char const* attributeName, Rgba8			const&		defaultValue );
Vec2		ParseXmlAttribute( XmlElement const& element, char const* attributeName, Vec2			const&		defaultValue );
IntVec2		ParseXmlAttribute( XmlElement const& element, char const* attributeName, IntVec2		const&		defaultValue );
std::string ParseXmlAttribute( XmlElement const& element, char const* attributeName, std::string	const&		defaultValue );
Strings		ParseXmlAttribute( XmlElement const& element, char const* attributeName, Strings		const&		defaultValue );
FloatRange	ParseXmlAttribute( XmlElement const& element, char const* attributeName, FloatRange		const&		defaultValue );
Vec3		ParseXmlAttribute( XmlElement const& element, char const* attributeName, Vec3			const&		defaultValue );
EulerAngles	ParseXmlAttribute( XmlElement const& element, char const* attributeName, EulerAngles	const&		defaultValue );
std::string ParseXmlAttribute( XmlElement const& element, char const* attributeName, char			const*		defaultValue );


