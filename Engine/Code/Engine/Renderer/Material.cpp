#include "Engine/Renderer/Material.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

extern Renderer* g_theRenderer;

//----------------------------------------------------------------------------------------------------------------------
Material::Material( std::string xmlFileName )
{
	ParseXmlData( xmlFileName );
}


//----------------------------------------------------------------------------------------------------------------------
Material::~Material()
{
	m_diffuseTexture		= nullptr;
	m_normalTexture			= nullptr;
	m_specGlossEmitTexture	= nullptr;
	m_shader				= nullptr;
}


//----------------------------------------------------------------------------------------------------------------------
void Material::ParseXmlData( std::string xmlFileName )
{
	XmlDocument materialDefs;

	XmlResult result = materialDefs.LoadFile( xmlFileName.c_str() );
	GUARANTEE_OR_DIE( result == tinyxml2::XML_SUCCESS, Stringf( "Failed to open required Material defs file %s", xmlFileName.c_str() ) );

	XmlElement* rootElement = materialDefs.RootElement();
	GUARANTEE_OR_DIE( rootElement, "is invalid" );

	//----------------------------------------------------------------------------------------------------------------------
	// Name
	//----------------------------------------------------------------------------------------------------------------------
	m_name = ParseXmlAttribute( *rootElement, "name", "INVALID NAME" );
	GUARANTEE_OR_DIE( m_name != "INVALID NAME", Stringf( "Invalid material name was parsed, please provide a valid material", m_name.c_str() ) );
	//----------------------------------------------------------------------------------------------------------------------
	// Vertex Type
	//----------------------------------------------------------------------------------------------------------------------
	std::string vertexType = ParseXmlAttribute( *rootElement, "vertexType", "INVALID VERTEX TYPE" );
	GUARANTEE_OR_DIE( vertexType != "INVALID VERTEX TYPE", Stringf( "Invalid vertex Type was parsed, please provide a valid material", vertexType.c_str() ) );
	m_vertexType = g_theRenderer->GetVertexTypeAsEnum( vertexType );
	//----------------------------------------------------------------------------------------------------------------------
	// Shader
	//----------------------------------------------------------------------------------------------------------------------
	std::string shaderName = ParseXmlAttribute( *rootElement, "shader", "INVALID_SHADER" );
	GUARANTEE_OR_DIE( shaderName != "INVALID_SHADER", Stringf( "Invalid Shader: %s, Provide a valid shader!!!", shaderName.c_str() ) );
	m_shader = g_theRenderer->CreateOrGetShaderByName( shaderName.c_str(), m_vertexType );
	//----------------------------------------------------------------------------------------------------------------------
	// Diffused texture
	//----------------------------------------------------------------------------------------------------------------------
	std::string diffuseTexture = ParseXmlAttribute( *rootElement, "diffuseTexture", "INVALID DIFFUSE TEXTURE" );
	GUARANTEE_OR_DIE( diffuseTexture != "INVALID DIFFUSE TEXTURE", Stringf( "Invalid diffuse texture was parsed, please provide a valid material", diffuseTexture.c_str() ) );
	m_diffuseTexture = g_theRenderer->CreateOrGetTextureFromFile( diffuseTexture.c_str() );
	//----------------------------------------------------------------------------------------------------------------------
	// Normal Texture
	//----------------------------------------------------------------------------------------------------------------------
	std::string normalTexture = ParseXmlAttribute( *rootElement, "normalTexture", "INVALID NORMAL TEXTURE" );
	GUARANTEE_OR_DIE( normalTexture != "INVALID NORMAL TEXTURE", Stringf( "Invalid normal texture was parsed, please provide a valid material", normalTexture.c_str() ) );
	m_normalTexture = g_theRenderer->CreateOrGetTextureFromFile( normalTexture.c_str() );
	//----------------------------------------------------------------------------------------------------------------------
	// SpecGlossEmit Texture
	//----------------------------------------------------------------------------------------------------------------------
	std::string specGlossEmitTexture = ParseXmlAttribute( *rootElement, "specGlossEmitTexture", "INVALID SPEC GLOSS EMIT TEXTURE" );
	GUARANTEE_OR_DIE( specGlossEmitTexture != "INVALID SPEC GLOSS EMIT TEXTURE", Stringf( "Invalid specGlossEmit texture was parsed, please provide a valid material", specGlossEmitTexture.c_str() ) );
	m_specGlossEmitTexture = g_theRenderer->CreateOrGetTextureFromFile( specGlossEmitTexture.c_str() );
	//----------------------------------------------------------------------------------------------------------------------
	// Color
	//----------------------------------------------------------------------------------------------------------------------
	std::string color = ParseXmlAttribute( *rootElement, "color", "INVALID COLOR" );
	GUARANTEE_OR_DIE( color != "INVALID COLOR", Stringf( "Invalid COLOR was parsed, please provide a valid material", color.c_str() ) );
	m_color.SetFromText( color.c_str() );
}
