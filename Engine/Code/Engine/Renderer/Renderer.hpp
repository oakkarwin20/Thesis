#pragma once

#include "Game/EngineBuildPreferences.hpp"

#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Renderer/Camera.hpp"
#include <Engine/Renderer/Shader.hpp>
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/Renderer/ConstantBuffer.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"

#include <vector>

//----------------------------------------------------------------------------------------------------------------------
#define DX_SAFE_RELEASE( dxObject )		\
{										\
	if ( ( dxObject ) != nullptr )		\
	{									\
		( dxObject )->Release();		\
		( dxObject ) = nullptr;			\
	}									\
}

//----------------------------------------------------------------------------------------------------------------------
// UnDefining Opaque
#if defined( OPAQUE )
#undef OPAQUE
#endif

//----------------------------------------------------------------------------------------------------------------------
struct ID3D11Device;
struct ID3D11DeviceContext;
struct IDXGISwapChain;
struct ID3D11RenderTargetView;
struct ID3D11RasterizerState;
struct ID3D11BlendState;
struct ID3D11SamplerState;
struct ID3D11DepthStencilState;
struct ID3D11DepthStencilView;
struct ID3D11Texture2D;

//----------------------------------------------------------------------------------------------------------------------
class  Window;
class  Texture;
class  BitmapFont;
class  Image;

//----------------------------------------------------------------------------------------------------------------------
enum class BlendMode
{
	ALPHA,
	ADDITIVE,
	OPAQUE,
	COUNT
};

enum class SamplerMode
{
	POINT_CLAMP,
	BILINEAR_WRAP,
	COUNT
};


enum class RasterizerMode
{
	SOLID_CULL_NONE,
	SOLID_CULL_BACK,
	WIREFRAME_CULL_NONE,
	WIREFRAME_CULL_BACK,
	COUNT
};

enum class DepthMode
{
	DISABLED,
	ENABLED,
	COUNT
};

enum class VertexType
{
	VERTEX_PCU,
	VERTEX_PCUTBN,
	NONE,
};


//--------------------------------------------------------------------------------------------------
enum class PrimitiveTopology
{
	D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED				=	0,
	D3D11_PRIMITIVE_TOPOLOGY_POINTLIST				=	1,
	D3D11_PRIMITIVE_TOPOLOGY_LINELIST				=	2,
	D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP				=	3,
	D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST			=	4,
	D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP			=	5,
	D3D11_PRIMITIVE_TOPOLOGY_LINELIST_ADJ			=	10,
	D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ			=	11,
	D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ		=	12,
	D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ		=	13,
};


//----------------------------------------------------------------------------------------------------------------------
struct RendererConfig
{
	Window*	m_window = nullptr;
};

//--------------------------------------------------------------------------------------------------------
class Renderer
{
public:
	Renderer(RendererConfig const& config)
		: m_config(config)
	{
	}
	~Renderer();

	//----------------------------------------------------------------------------------------------------------------------
	void			Startup();
	void			BeginFrame();
	void			EndFrame();
	void			Shutdown();

	//----------------------------------------------------------------------------------------------------------------------
	void			ClearScreen( const Rgba8& clearColor );
	void			BeginCamera( const Camera& camera );
	void			EndCamera( const Camera& camera );
	void			DrawVertexArray( int numVertexes, const Vertex_PCU* vertexes, PrimitiveTopology primitiveTopology = PrimitiveTopology::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
	void			DrawVertexArray( int numVertexes, const Vertex_PCUTBN* vertexes );

	//----------------------------------------------------------------------------------------------------------------------
	// Textures
	Texture*		GetTextureForFileName( char const* imageFilePath );
	Texture*		CreateOrGetTextureFromFile( char const* imageFilePath );
	Texture*		CreateTextureFromFile( char const* imageFilePath );
	Texture*		CreateTextureFromData( char const* name, IntVec2 const& dimensions, int bytesPerTexal, unsigned char* texelData );
	Texture*		CreateTextureFromImage( const Image& image );

	void			BindTexture( Texture const* texture );		// Pointer change, data NO change
	void			BindTexture( Texture const* texture0, Texture const* texture1, Texture const* texture2 );	


	RendererConfig const& GetConfig() const { return m_config; }

	//----------------------------------------------------------------------------------------------------------------------
	// BitmapFont
	BitmapFont*		GetBitmapForFileName( char const* imageFilePath );
	BitmapFont*		CreateOrGetBitmapFont( char const* bitmapFontFilePathWithNoExtension );
	BitmapFont*		CreateOrGetBitmapFontFromFile( char const* bitmapFontFilePathWithNoExtension );

	//----------------------------------------------------------------------------------------------------------------------
	// Shaders
	void			BindShader( Shader* shader );
	Shader*			CreateShader( char const* shaderName, char const* shaderSource, VertexType const& vertexType = VertexType::VERTEX_PCU );
	Shader*			CreateShader( char const* shaderName, VertexType const& vertexType );
	Shader*			CreateOrGetShaderByName( char const* newShaderName, VertexType vertexType = VertexType::VERTEX_PCU );
	bool			CompileShaderToByteCode( std::vector<unsigned char>& outByteCode, char const* name, char const* source, char const* entryPoint, char const* target );

	//----------------------------------------------------------------------------------------------------------------------
	// Vertex Buffer
	VertexBuffer*	CreateVertexBuffer( const size_t size );
	VertexBuffer*	CreateVertexBuffer( const size_t size, unsigned int stride );
	IndexBuffer*	CreateIndexBuffer( const size_t size );
	void			Copy_CPU_To_GPU( const void* data, size_t size, VertexBuffer*& vbo, unsigned int stride );
	void			Copy_CPU_To_GPU( const void* data, size_t size, IndexBuffer*& ibo );
	void			BindVertexBuffer( VertexBuffer* vbo, PrimitiveTopology primitiveTopology = PrimitiveTopology::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
	void			BindIndexBuffer( IndexBuffer* ibo );
	void			DrawVertexBuffer( VertexBuffer* vbo, int vertexCount, int vertexOffset = 0, PrimitiveTopology primitiveTopology = PrimitiveTopology::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
	void			DrawVertexAndIndexBuffer( VertexBuffer* vbo, IndexBuffer* ibo, int indexCount, int indexOffset = 0, int vertexOffset = 0 );

	//----------------------------------------------------------------------------------------------------------------------
	// ConstantBuffer
	ConstantBuffer*		CreateConstantBuffer( const size_t size );
	void				Copy_CPU_To_GPU( const void* data, size_t size, ConstantBuffer* cbo );
	void				BindConstantBuffer( int slot, ConstantBuffer* cbo );

	//----------------------------------------------------------------------------------------------------------------------
	// Model Constants
	void SetModelConstants( Mat44 const& modelMatrix = Mat44(), Rgba8 const& modelColor = Rgba8::WHITE );

	//----------------------------------------------------------------------------------------------------------------------
	// Lighting Constants
	void SetLightingConstants( Vec3 const& sunDirection, float sunIntensity, float ambientIntensity,
							   Vec3 const& worldEyePosition = Vec3::ZERO, int normalMode = 0, int specularMode = 0, 
							   float specularIntensity = 0.0f, float specularPower = 0.0f );

	//----------------------------------------------------------------------------------------------------------------------
	// Initializing Modes and States
	void InitializingBlendState();
	void InitializingSamplerState();
	void InitializingRasterizerState();
	void InitializingDepthState();

	//----------------------------------------------------------------------------------------------------------------------
	// Setting and Checking states
	void SetStateIfChanged();
	void SetBlendMode( BlendMode blendMode );
	void SetSamplerMode( SamplerMode samplerMode );
	void SetRasterizerMode( RasterizerMode rasterizerMode );
	void SetDepthMode( DepthMode depthMode );

	VertexType GetVertexTypeAsEnum( std::string const& vertexType );

public:
	//----------------------------------------------------------------------------------------------------------------------
	std::vector<Texture*>		m_loadedTextures;
	std::vector<BitmapFont*>	m_loadedFonts;
	 
protected:
	//----------------------------------------------------------------------------------------------------------------------
	RendererConfig				m_config;

	std::vector<Shader*>		m_loadedShaders;
	Shader const*				m_currentShader				= nullptr;
	
	ID3D11Device*				m_device					= nullptr;
	ID3D11DeviceContext*		m_deviceContext				= nullptr;
	IDXGISwapChain*				m_swapChain					= nullptr;
	ID3D11RenderTargetView*		m_renderTargetView			= nullptr;
	void*						m_dxgiDebugModule			= nullptr;
	void*						m_dxgiDebug					= nullptr;

	VertexBuffer*				m_immediateVBO_VertexPCU	= nullptr;
	VertexBuffer*				m_immediateVBO_VertexPNCU	= nullptr;
	Shader*						m_defaultShader				= nullptr;
	ConstantBuffer*				m_cameraCBO					= nullptr;
	ConstantBuffer*				m_modelCBO					= nullptr;
	ConstantBuffer*				m_lightingCBO				= nullptr;

	Texture const*				m_defaultTexture			= nullptr;

	ID3D11DepthStencilView*		m_depthStencilView			= nullptr;
	ID3D11Texture2D*			m_depthStencilTexture		= nullptr;

//----------------------------------------------------------------------------------------------------------------------
	ID3D11BlendState*			m_currentBlendState			= nullptr;
	ID3D11SamplerState*			m_currentSamplerState		= nullptr;
	ID3D11RasterizerState*		m_currentRasterizerState	= nullptr;
	ID3D11DepthStencilState*	m_currentDepthStencilState	= nullptr;
	
	BlendMode					m_desiredBlendMode			= BlendMode::ALPHA;
	SamplerMode					m_desiredSamplerMode		= SamplerMode::POINT_CLAMP;
	RasterizerMode				m_desiredRasterizerMode		= RasterizerMode::SOLID_CULL_BACK;
	DepthMode					m_desiredDepthmode			= DepthMode::ENABLED;
	
	ID3D11BlendState*			m_blendStates		[(int) (BlendMode::COUNT)]		= { };
	ID3D11SamplerState*			m_samplerStates		[(int) (SamplerMode::COUNT)]	= { };	
	ID3D11RasterizerState*		m_rasterizerStates	[(int) (RasterizerMode::COUNT)] = { };	
	ID3D11DepthStencilState*	m_depthStencilStates[(int) (DepthMode::COUNT)]		= { };	
};