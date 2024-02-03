#pragma once
#include <string>
#include "Engine/Math/IntVec2.hpp"

//----------------------------------------------------------------------------------------------------------------------
struct ID3D11Texture2D;
struct ID3D11ShaderResourceView;

//----------------------------------------------------------------------------------------------------------------------
class Texture 
{
	friend class Renderer;					// Only the Renderer can create new Texture objects!

private:
	Texture();								// can't instantiate directly; must ask Renderer to do it for you
	Texture(Texture const& copy) = delete;	// No copying allowed!  This represents GPU memory.
	~Texture();

public:
	IntVec2					GetDimensions()		const { return m_dimensions; }
	std::string const&		GetImageFilePath()	const { return m_name; }
	
protected:
	std::string					m_name;
	IntVec2						m_dimensions;

	ID3D11Texture2D*			m_texture				= nullptr;
	ID3D11ShaderResourceView*	m_shaderResourceView	= nullptr;
};
