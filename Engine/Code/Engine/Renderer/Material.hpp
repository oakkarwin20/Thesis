#pragma once

#include "Engine/Renderer/Renderer.hpp"


//----------------------------------------------------------------------------------------------------------------------
class Material
{
public:
	Material( std::string xmlFileName );
	~Material();

	void ParseXmlData( std::string xmlFileName );

public:
	std::string m_name					= "UnNamed Material";
	Shader*		m_shader				= nullptr;
	VertexType	m_vertexType			= VertexType::VERTEX_PCUTBN;
	Texture*	m_diffuseTexture		= nullptr;
	Texture*	m_normalTexture			= nullptr;
	Texture*	m_specGlossEmitTexture	= nullptr;
	Rgba8		m_color					= Rgba8::WHITE;
};
