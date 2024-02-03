#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/XmlUtils.hpp"

void NamedStrings::PopulateFromXmlElementAttributes( XmlElement const& element )
{
//	element.FirstChildElement();
//	element.FirstAttribute();

	// get first element in xml file
	// get first attribute in element
	// add those keyValuePairs to NamedString 
	// advance to next attribute and repeat line 15				// Question, how to loop through all attributes
	// if nullptr, return 

//	m_keyValuePairs;
	
	XmlAttribute const* firstAttribute = element.FirstAttribute();

	while ( firstAttribute )
	{
		char const* key = firstAttribute->Name();
		char const* value = firstAttribute->Value();
		m_keyValuePairs[key] = value;
		firstAttribute = firstAttribute->Next();
	}
}

void NamedStrings::SetValue( std::string const& keyName, std::string const& newValue )
{
	m_keyValuePairs[keyName] = newValue;	// This will add or replace existing key value pair
}

std::string NamedStrings::GetValue( std::string const& keyName, std::string const& defaultValue ) const
{
	auto found = m_keyValuePairs.find(keyName);
	if ( found == m_keyValuePairs.end() )		//		Not found
	{
		return defaultValue;
	}
	return found->second;
}

bool NamedStrings::GetValue( std::string const& keyName, bool defaultValue ) const
{
	auto found = m_keyValuePairs.find( keyName );
	if ( found == m_keyValuePairs.end() )
	{
		return defaultValue;
	}

	if ( strcmp(found->second.c_str(), "true" ) == 0 || 
		 strcmp(found->second.c_str(), "True" ) == 0 || 
		 strcmp(found->second.c_str(), "TRUE" ) == 0 )
	{
		return true;
	}
	else if ( strcmp( found->second.c_str(), "false" ) == 0 ||
			  strcmp( found->second.c_str(), "False" ) == 0 ||
			  strcmp( found->second.c_str(), "FALSE" ) == 0 )
	{
		return false;
	}

	ERROR_AND_DIE("Bool GetValue is return is invalid");
}

std::string NamedStrings::GetValue( std::string const& keyName, char const* defaultValue ) const
{
	auto found = m_keyValuePairs.find( keyName );
	if ( found == m_keyValuePairs.end() )		//		Not found
	{
		return defaultValue;
	}
	return found->second;
}

Rgba8 NamedStrings::GetValue( std::string const& keyName, Rgba8 const& defaultValue ) const
{
	auto found = m_keyValuePairs.find( keyName );
	if ( found == m_keyValuePairs.end() )		//		Not found
	{
		return defaultValue;
	}

	Rgba8 foundRgba8;
	foundRgba8.SetFromText( found->second.c_str() );
	return foundRgba8;
}

int NamedStrings::GetValue( std::string const& keyName, int defaultValue ) const
{
	auto found = m_keyValuePairs.find( keyName );
	if ( found == m_keyValuePairs.end() )		//		Not found
	{
		return defaultValue;
	}
	return atoi( found->second.c_str() );
}

float NamedStrings::GetValue( std::string const& keyName, float defaultValue ) const
{
	auto found = m_keyValuePairs.find( keyName );
	if ( found == m_keyValuePairs.end() )		//		Not found
	{
		return defaultValue;
	}
	return static_cast<float>(atof( found->second.c_str() ));		// get value as string, then access string's char array, then convert to double then cast to float  
}


//----------------------------------------------------------------------------------------------------------------------
Vec2 NamedStrings::GetValue( std::string const& keyName, Vec2 const& defaultValue ) const
{
	auto found = m_keyValuePairs.find( keyName );
	if ( found == m_keyValuePairs.end() )		//		Not found
	{
		return defaultValue;
	}

	Vec2 foundVec2;
	foundVec2.SetFromText( found->second.c_str() );
	return foundVec2;
}


//----------------------------------------------------------------------------------------------------------------------
Vec3 NamedStrings::GetValue( std::string const& keyName, Vec3 const& defaultValue ) const
{
	auto found = m_keyValuePairs.find( keyName );
	if ( found == m_keyValuePairs.end() )		//		Not found
	{
		return defaultValue;
	}

	Vec3 foundVec3;
	foundVec3.SetFromText( found->second.c_str() );
	return foundVec3;
}


//----------------------------------------------------------------------------------------------------------------------
IntVec2 NamedStrings::GetValue( std::string const& keyName, IntVec2 const& defaultValue ) const
{
	auto found = m_keyValuePairs.find( keyName );
	if ( found == m_keyValuePairs.end() )		//		Not found
	{
		return defaultValue;
	}

	IntVec2 foundIntVec2;
	foundIntVec2.SetFromText( found->second.c_str() );
	return foundIntVec2;
}
