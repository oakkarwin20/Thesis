#include "Engine/ThirdParty/stb/stb_image.h"

#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/ConstantBuffer.hpp"
#include "Engine/Renderer/DefaultShader.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"

// #include "Game/SpriteLit.hpp"

//----------------------------------------------------------------------------------------------------------------------
// DirectX 11 includes
#define WIN32_LEAN_AND_MEAN		// Always #define this before #including <windows.h>
#include <windows.h>

#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>

#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "dxgi.lib")
#pragma comment (lib, "d3dcompiler.lib")

#if defined( ENGINE_DEBUG_RENDER )
#include <dxgidebug.h>
#pragma comment( lib, "dxguid.lib" )
#endif

//----------------------------------------------------------------------------------------------------------------------
// UnDefining Opaque
#if defined( OPAQUE )
#undef OPAQUE
#endif

//----------------------------------------------------------------------------------------------------------------------
Vertex_PCU vertices[] =
{
	Vertex_PCU( Vec3( -0.5f, -0.5f, 0.0f ), Rgba8( 255, 255, 255, 255 ), Vec2( 0.0f, 0.0f ) ),
	Vertex_PCU( Vec3(  0.0f,  0.5f, 0.0f ), Rgba8( 255, 255, 255, 255 ), Vec2( 0.0f, 0.0f ) ),
	Vertex_PCU( Vec3(  0.5f, -0.5f, 0.0f ), Rgba8( 255, 255, 255, 255 ), Vec2( 0.0f, 0.0f ) ),
};

//----------------------------------------------------------------------------------------------------------------------
struct LightingConstants
{
	Vec3  SunDirection		= Vec3::ZERO;
	float SunIntensity		= 0.0f;
	float AmbientIntensity	= 0.0f;
	Vec3  WorldEyePosition	= Vec3::ZERO;
	int	  NormalMode		= 0;
	int   SpecularMode		= 0;
	float SpecularIntensity	= 0.0f;
	float SpecularPower		= 0.0f;
};
static const int k_lightingConstantsSlot = 1;

//----------------------------------------------------------------------------------------------------------------------
struct CameraConstants
{
	Mat44 ProjectionMatrix;
	Mat44 ViewMatrix;
};
static const int k_cameraConstantsSlot = 2;

//----------------------------------------------------------------------------------------------------------------------
struct ModelConstants
{
	Mat44 ModelMatrix;
	float ModelColor[4];
};
static const int k_modelConstantsSlot = 3;

//----------------------------------------------------------------------------------------------------------------------
void Render()
{
}

//----------------------------------------------------------------------------------------------------------------------
void Renderer::Startup()
{
	//----------------------------------------------------------------------------------------------------------------------
#if defined( ENGINE_DEBUG_RENDER )
	m_dxgiDebugModule = ( void* ) ::LoadLibraryA( "dxgidebug.dll" );
	if ( m_dxgiDebugModule == nullptr )
	{
		ERROR_AND_DIE( "Could not load dxgidebug.dll" );
	}

	typedef HRESULT( WINAPI* GetDebugModuleCB )( REFIID, void** );
	( ( GetDebugModuleCB ) ::GetProcAddress( (HMODULE)m_dxgiDebugModule, "DXGIGetDebugInterface" ) )( __uuidof( IDXGIDebug ), &m_dxgiDebug );

	if ( m_dxgiDebug == nullptr )
	{
		ERROR_AND_DIE( "Could not load debug module" );
	}
#endif 

	//----------------------------------------------------------------------------------------------------------------------
	// Specifying swapChainDesc values
	DXGI_SWAP_CHAIN_DESC swapChainDesc	= { 0 };
	swapChainDesc.BufferDesc.Width		= m_config.m_window->GetClientDimensions().x;
	swapChainDesc.BufferDesc.Height		= m_config.m_window->GetClientDimensions().y;
	swapChainDesc.BufferDesc.Format		= DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.SampleDesc.Count		= 1;
	swapChainDesc.BufferUsage			= DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount			= 2;
	swapChainDesc.OutputWindow			= static_cast<HWND>( m_config.m_window->GetHwnd() );							
	swapChainDesc.Windowed				= true;
	swapChainDesc.SwapEffect			= DXGI_SWAP_EFFECT_FLIP_DISCARD;

	//----------------------------------------------------------------------------------------------------------------------
	// Creating deviceFlags
	unsigned int deviceFlags = 0;
#if defined( ENGINE_DEBUG_RENDER )
	deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif	

	//----------------------------------------------------------------------------------------------------------------------
	// Creating device, deviceContext, and SwapChain
	HRESULT hr;
	hr = D3D11CreateDeviceAndSwapChain
	(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		deviceFlags,									// #ToDo // Question, what are "deviceFlags"? 
		nullptr,
		0,
		D3D11_SDK_VERSION,
		&swapChainDesc,
		&m_swapChain,
		&m_device,
		nullptr,
		&m_deviceContext
	);

	if ( !SUCCEEDED( hr ) )
	{
		ERROR_AND_DIE( "Could not create D3D 11 device and swap chain." );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Get back buffer texture, create RenderTargetView, then release backBuffer once finished
	ID3D11Texture2D* backBuffer;

	hr = m_swapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), (void**)&backBuffer );
	if ( !SUCCEEDED( hr ) )
	{
		ERROR_AND_DIE( "Could not get swap chain buffer." );
	}

	hr = m_device->CreateRenderTargetView( backBuffer, NULL, &m_renderTargetView );
	if ( !SUCCEEDED( hr ) )
	{
		ERROR_AND_DIE( "Could create render target view for swap chain buffer." );
	}

	backBuffer->Release();

	//----------------------------------------------------------------------------------------------------------------------
	// Create and bind Shader
	Shader* shader		= CreateShader( "Default", defaultShaderSource );
//	Shader* shader		= CreateShader( "SpriteLit", spriteLitShaderSource );
	m_defaultShader		= shader;
	BindShader( shader );
	
	//----------------------------------------------------------------------------------------------------------------------
	// Creating default Image and setting values to m_defaultTexture
	// Then Binding texture
	Image defaultImage				= Image( IntVec2( 2, 2), Rgba8( 255, 255, 255, 255 ) );
	defaultImage.m_imageFilePath	= "DEFAULT";
	m_defaultTexture				= CreateTextureFromImage( defaultImage );
	BindTexture( m_defaultTexture );

	//----------------------------------------------------------------------------------------------------------------------
	// Creating vertexBuffer and constantBuffer
	UINT modelSize = sizeof(ModelConstants);
	UINT lightSize = sizeof(LightingConstants);

	m_cameraCBO					= CreateConstantBuffer( sizeof(CameraConstants)						 );
//	m_modelCBO					= CreateConstantBuffer( sizeof(ModelConstants)						 );
	m_modelCBO					= CreateConstantBuffer( modelSize									 );

	m_lightingCBO				= CreateConstantBuffer( lightSize									 );
	m_immediateVBO_VertexPCU	= CreateVertexBuffer  ( sizeof(Vertex_PCU),	   sizeof(Vertex_PCU)	 );
	m_immediateVBO_VertexPNCU	= CreateVertexBuffer  ( sizeof(Vertex_PCUTBN), sizeof(Vertex_PCUTBN) );

	// Setting default blendMode to Alpha 
//	SetBlendMode( BlendMode::ALPHA );

		//----------------------------------------------------------------------------------------------------------------------
		// Initializing all states in startup
		//----------------------------------------------------------------------------------------------------------------------
		// Blend Mode
		InitializingBlendState();
		// Sampler State
		InitializingSamplerState();
		// Rasterizer State
		InitializingRasterizerState();
		// Depth Stencil State
		InitializingDepthState();
		//----------------------------------------------------------------------------------------------------------------------
		// Initializing all states in startup
		//----------------------------------------------------------------------------------------------------------------------

	//----------------------------------------------------------------------------------------------------------------------
//	// Specifying samplerState description
//	D3D11_SAMPLER_DESC samplerDesc	= { 0 };
//	samplerDesc.Filter				= D3D11_FILTER_MIN_MAG_MIP_POINT;
//	samplerDesc.AddressU			= D3D11_TEXTURE_ADDRESS_CLAMP;
//	samplerDesc.AddressV			= D3D11_TEXTURE_ADDRESS_CLAMP;
//	samplerDesc.AddressW			= D3D11_TEXTURE_ADDRESS_CLAMP;
//	samplerDesc.ComparisonFunc		= D3D11_COMPARISON_NEVER;
//	samplerDesc.MaxLOD				= D3D11_FLOAT32_MAX;
//
//	// Creating samplerState
//	hr = m_device->CreateSamplerState( &samplerDesc, &m_currentSamplerState );
//	if ( !SUCCEEDED( hr ) )
//	{
//		ERROR_AND_DIE( "Could not create sampler state." );
//	}
//
//	// Set samplers
//	m_deviceContext->PSSetSamplers( 0, 1, &m_currentSamplerState );

//	//----------------------------------------------------------------------------------------------------------------------
//	// Specifying rasterizerDesc values
//	D3D11_RASTERIZER_DESC rasterizerDesc	= { 0 };
//	rasterizerDesc.FillMode					= D3D11_FILL_SOLID;
//	rasterizerDesc.CullMode					= D3D11_CULL_BACK;
//	rasterizerDesc.DepthClipEnable			= true;
//	rasterizerDesc.AntialiasedLineEnable	= true;
//	rasterizerDesc.FrontCounterClockwise	= true;
//
//	// Create Rasterizer State
//	hr = m_device->CreateRasterizerState( &rasterizerDesc, &m_currentRasterizerState );
//	if ( !SUCCEEDED( hr ) )
//	{
//		ERROR_AND_DIE( "Could not create rasterizer state." );
//	}
//
//	// Set the Rasterizer State
//	m_deviceContext->RSSetState( m_currentRasterizerState );

	//----------------------------------------------------------------------------------------------------------------------
	// Setting Model Constants
	SetModelConstants();

	//----------------------------------------------------------------------------------------------------------------------
	// Creating and setting depth stencil view and state
	D3D11_TEXTURE2D_DESC texture2DDesc	= { 0 };
	texture2DDesc.Width					= m_config.m_window->GetClientDimensions().x;
	texture2DDesc.Height				= m_config.m_window->GetClientDimensions().y;
	texture2DDesc.MipLevels				= 1;
	texture2DDesc.ArraySize				= 1;
	texture2DDesc.Usage					= D3D11_USAGE_DEFAULT;
	texture2DDesc.Format				= DXGI_FORMAT_D24_UNORM_S8_UINT;
	texture2DDesc.BindFlags				= D3D11_BIND_DEPTH_STENCIL;
	texture2DDesc.SampleDesc.Count		= 1;

	//----------------------------------------------------------------------------------------------------------------------
	//Create Texture 2D
	hr = m_device->CreateTexture2D( &texture2DDesc, nullptr, &m_depthStencilTexture );
	if ( !SUCCEEDED( hr ) )
	{
		ERROR_AND_DIE( "Could not create Texture2D" );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Create Depth Stencil View
	hr = m_device->CreateDepthStencilView( m_depthStencilTexture, nullptr, &m_depthStencilView );
	if ( !SUCCEEDED( hr ) )
	{
		ERROR_AND_DIE( "Could not create Depth Stencil View" );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// 

	SetLightingConstants( Vec3( 2.0f, 1.0f, -1.0f ), 1.0f, 1.0f );
//	//----------------------------------------------------------------------------------------------------------------------
//	// Create and set a Depth Stencil State
//	// Creating and Specifying Depth Stencil description
//	D3D11_DEPTH_STENCIL_DESC depthStencilDesc	= { 0 };
//	depthStencilDesc.DepthEnable				= true;
//	depthStencilDesc.DepthWriteMask				= D3D11_DEPTH_WRITE_MASK_ALL;		// Means pixels behind will be blocked since this is opaque
//	depthStencilDesc.DepthFunc					= D3D11_COMPARISON_LESS_EQUAL;
//
//	// Create Depth Stencil State
//	hr = m_device->CreateDepthStencilState( &depthStencilDesc, &m_currentDepthStencilState );
//	if ( !SUCCEEDED( hr ) )
//	{
//		ERROR_AND_DIE( "Could not create Depth Stencil State" );
//	}
//
//	// Set Depth Stencil State
//	m_deviceContext->OMSetDepthStencilState( m_currentDepthStencilState, 0 );
}

//----------------------------------------------------------------------------------------------------------------------
Renderer::~Renderer()
{
}

//----------------------------------------------------------------------------------------------------------------------
void Renderer::BeginFrame()
{
	m_deviceContext->OMSetRenderTargets( 1, &m_renderTargetView, m_depthStencilView );

}

//----------------------------------------------------------------------------------------------------------------------
void Renderer::EndFrame()
{
	HRESULT hr;
	hr = m_swapChain->Present( 0, 0 );
	if ( hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET )
	{
		ERROR_AND_DIE( "Device has been lost, application will now terminate." );
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Renderer::Shutdown()
{
	for ( int i = 0; i < m_loadedTextures.size(); i++ )
	{
		delete m_loadedTextures[i];
	}
	for ( int i = 0; i < m_loadedFonts.size(); i++ )
	{
		delete m_loadedFonts[i];	
	}
	for ( int i = 0; i < m_loadedShaders.size(); i++ )
	{
		delete m_loadedShaders[i];
	}

	delete m_defaultTexture;
	m_defaultTexture = nullptr;

	m_currentShader = nullptr;

	delete m_immediateVBO_VertexPCU;
	m_immediateVBO_VertexPCU = nullptr;

	delete m_immediateVBO_VertexPNCU;
	m_immediateVBO_VertexPNCU = nullptr;
	
	delete m_cameraCBO;
	m_cameraCBO = nullptr;

	delete m_modelCBO;
	m_modelCBO = nullptr;

	delete m_lightingCBO;
	m_lightingCBO = nullptr;

	for ( int i = 0; i < static_cast<int>( BlendMode::COUNT ); i++ )
	{
		DX_SAFE_RELEASE( m_blendStates[i] );
	}

	for ( int i = 0; i < static_cast<int>( RasterizerMode::COUNT ); i++ )
	{
		DX_SAFE_RELEASE( m_rasterizerStates[i] );
	}

	for ( int i = 0; i < static_cast<int>( SamplerMode::COUNT ); i++ )
	{
		DX_SAFE_RELEASE( m_samplerStates[i] );
	}

	for ( int i = 0; i < static_cast<int>( DepthMode::COUNT ); i++ )
	{
		DX_SAFE_RELEASE( m_depthStencilStates[i] );
	}

	DX_SAFE_RELEASE( m_device );
	DX_SAFE_RELEASE( m_deviceContext );
	DX_SAFE_RELEASE( m_swapChain );
	DX_SAFE_RELEASE( m_renderTargetView );
//	DX_SAFE_RELEASE( m_currentRasterizerState );
//	DX_SAFE_RELEASE( m_currentSamplerState );
//	DX_SAFE_RELEASE( m_currentDepthStencilState	);
	DX_SAFE_RELEASE( m_depthStencilView		);
	DX_SAFE_RELEASE( m_depthStencilTexture );

#if defined( ENGINE_DEBUG_RENDER )
	( (IDXGIDebug*)m_dxgiDebug )->ReportLiveObjects( DXGI_DEBUG_ALL, (DXGI_DEBUG_RLO_FLAGS)( DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_IGNORE_INTERNAL ) );

	( (IDXGIDebug*)m_dxgiDebug )->Release();
	m_dxgiDebug = nullptr;

	::FreeLibrary( (HMODULE)m_dxgiDebugModule );
	m_dxgiDebugModule = nullptr;
#endif
}

//----------------------------------------------------------------------------------------------------------------------
void Renderer::ClearScreen(const Rgba8& clearColor)
{	
	float	arrayRgba8AsFloat[4];
	clearColor.GetAsFloat( arrayRgba8AsFloat );

	// #ToDo // Clarify how this works, Initially wasn't sure how to pass in the parameter for color.GetAsFloat()
	m_deviceContext->ClearRenderTargetView( m_renderTargetView, arrayRgba8AsFloat );

	// Clear Depth Stencil View
	m_deviceContext->ClearDepthStencilView( m_depthStencilView, D3D11_CLEAR_DEPTH, 1, 0 );

}

//----------------------------------------------------------------------------------------------------------------------
void Renderer::BeginCamera(const Camera& camera)
{
	// Set the Viewport
	D3D11_VIEWPORT viewport = { 0 };
	viewport.TopLeftX		= 0;
	viewport.TopLeftY		= 0;
	viewport.Width			= static_cast<float>( m_config.m_window->GetClientDimensions().x );
	viewport.Height			= static_cast<float>( m_config.m_window->GetClientDimensions().y );
	viewport.MinDepth		= 0;
	viewport.MaxDepth		= 1;
	m_deviceContext->RSSetViewports( 1, &viewport );

	//----------------------------------------------------------------------------------------------------------------------
	// Creating local CameraConstants and setting member values
	CameraConstants cameraConstants;
	cameraConstants.ProjectionMatrix = camera.GetProjectionMatrix();
	cameraConstants.ViewMatrix		 = camera.GetViewMatrix();

	Copy_CPU_To_GPU( &cameraConstants, sizeof(cameraConstants), m_cameraCBO );
	BindConstantBuffer( k_cameraConstantsSlot, m_cameraCBO );
}

//----------------------------------------------------------------------------------------------------------------------
void Renderer::EndCamera(const Camera& camera)
{
	UNUSED( camera );
}
 
//----------------------------------------------------------------------------------------------------------------------
void Renderer::DrawVertexArray( int numVertexes, const Vertex_PCU* vertexes, PrimitiveTopology primitiveTopology )
{ 
	// #Question // how to do this?
	Copy_CPU_To_GPU( vertexes, ( sizeof( Vertex_PCU ) * numVertexes ), m_immediateVBO_VertexPCU, sizeof( Vertex_PCU ) );
	DrawVertexBuffer( m_immediateVBO_VertexPCU, numVertexes, 0, primitiveTopology ); 
}

//----------------------------------------------------------------------------------------------------------------------
void Renderer::DrawVertexArray( int numVertexes, const Vertex_PCUTBN* vertexes )
{
	Copy_CPU_To_GPU( vertexes, ( sizeof( Vertex_PCUTBN ) * numVertexes ), m_immediateVBO_VertexPNCU, sizeof( Vertex_PCUTBN ) );
	DrawVertexBuffer( m_immediateVBO_VertexPNCU, numVertexes, 0 );
}

//----------------------------------------------------------------------------------------------------------------------
Texture* Renderer::GetTextureForFileName( char const* imageFilePath )
{
	for ( int textureIndex = 0; textureIndex < m_loadedTextures.size(); textureIndex++ )
	{
		if ( m_loadedTextures[textureIndex]->m_name == imageFilePath )
		{
			return m_loadedTextures[textureIndex];
		}
	}
	return nullptr;
}

//------------------------------------------------------------------------------------------------
Texture* Renderer::CreateOrGetTextureFromFile( char const* imageFilePath )
{
	// See if we already have this texture previously loaded
	Texture* existingTexture = GetTextureForFileName( imageFilePath );
	if ( existingTexture )
	{
		return existingTexture;
	}

	// Never seen this texture before!  Let's load it.
	Texture* newTexture = CreateTextureFromFile( imageFilePath );
	return newTexture;
}

//------------------------------------------------------------------------------------------------
Texture* Renderer::CreateTextureFromFile( char const* imageFilePath )
{
	//IntVec2 dimensions = IntVec2( 0, 0 );		// This will be filled in for us to indicate image width & height
	//int bytesPerTexel = 0;						// This will be filled in for us to indicate how many color components the image had (e.g. 3=RGB=24bit, 4=RGBA=32bit)
	//int numComponentsRequested = 0;				// don't care; we support 3 (24-bit RGB) or 4 (32-bit RGBA)

	//											// Load (and decompress) the image RGB(A) bytes from a file on disk into a memory buffer (array of bytes)
	//stbi_set_flip_vertically_on_load( 1 );		// We prefer uvTexCoords has origin (0,0) at BOTTOM LEFT
	//unsigned char* texelData = stbi_load( imageFilePath, &dimensions.x, &dimensions.y, &bytesPerTexel, numComponentsRequested );

	//// Check if the load was successful
	//GUARANTEE_OR_DIE( texelData, Stringf( "Failed to load image \"%s\"", imageFilePath ) );

	Image*	 newImage		= new Image( imageFilePath );
	Texture* newTexture		= CreateTextureFromImage( *newImage );

	if ( newTexture != nullptr )
	{
		m_loadedTextures.push_back( newTexture );
	}

	//	Texture* newTexture = CreateTextureFromData( imageFilePath, dimensions, bytesPerTexel, texelData );
//	// Free the raw image texel data now that we've sent a copy of it down to the GPU to be stored in video memory
//	stbi_image_free( texelData );

//	m_loadedTextures.push_back( newTexture );
	return newTexture;
}

 
//------------------------------------------------------------------------------------------------
Texture* Renderer::CreateTextureFromData( char const* name, IntVec2 const& dimensions, int bytesPerTexel, unsigned char* texelData )
{
	// Check if the load was successful
	GUARANTEE_OR_DIE( texelData, Stringf( "CreateTextureFromData failed for \"%s\" - texelData was null!", name ) );
	GUARANTEE_OR_DIE( bytesPerTexel >= 3 && bytesPerTexel <= 4, Stringf( "CreateTextureFromData failed for \"%s\" - unsupported BPP=%i (must be 3 or 4)", name, bytesPerTexel ) );
	GUARANTEE_OR_DIE( dimensions.x > 0 && dimensions.y > 0, Stringf( "CreateTextureFromData failed for \"%s\" - illegal texture dimensions (%i x %i)", name, dimensions.x, dimensions.y ) );

	Texture* newTexture = new Texture();
	newTexture->m_name = name; // NOTE: m_name must be a std::string, otherwise it may point to temporary data!
	newTexture->m_dimensions = dimensions;

	m_loadedTextures.push_back( newTexture );
	return newTexture;
}

//----------------------------------------------------------------------------------------------------------------------
Texture* Renderer::CreateTextureFromImage( const Image& image )
{
	//----------------------------------------------------------------------------------------------------------------------
	// Creating texture description and specifying values
	D3D11_TEXTURE2D_DESC textureDesc	= { 0 };
	textureDesc.Width					= image.GetDimensions().x; 
	textureDesc.Height					= image.GetDimensions().y;
	textureDesc.MipLevels				= 1;
	textureDesc.ArraySize				= 1;
	textureDesc.Format					= DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.Usage					= D3D11_USAGE_IMMUTABLE;
	textureDesc.BindFlags				= D3D11_BIND_SHADER_RESOURCE;
	textureDesc.SampleDesc.Count		= 1;

	//----------------------------------------------------------------------------------------------------------------------
	// Creating subResourceData
	D3D11_SUBRESOURCE_DATA subResourceData	= { 0 };
	subResourceData.pSysMem					= image.GetRawData();
	subResourceData.SysMemPitch				= textureDesc.Width * sizeof( Rgba8 );

	//----------------------------------------------------------------------------------------------------------------------
	// Creating Texture2D
	Texture* texture		= new Texture();
	texture->m_dimensions	= image.GetDimensions();
	texture->m_name			= image.GetImageFilePath();

	HRESULT hr;
	hr = m_device->CreateTexture2D( &textureDesc, &subResourceData, &texture->m_texture );
	if ( !SUCCEEDED( hr ) )
	{
		ERROR_AND_DIE("Could not create Texture2D");
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Creating shaderResourceView
	m_device->CreateShaderResourceView( texture->m_texture, NULL, &texture->m_shaderResourceView );
	if ( !SUCCEEDED( hr ) )
	{
		ERROR_AND_DIE( "Could not create ShaderResourceView" );
	}

	return texture;
}


//-----------------------------------------------------------------------------------------------
void Renderer::BindTexture( const Texture* texture )
{
	if ( texture == nullptr )
	{
		texture = m_defaultTexture;
	}	
	m_deviceContext->PSSetShaderResources( 0, 1, &texture->m_shaderResourceView );
}


//----------------------------------------------------------------------------------------------------------------------
void Renderer::BindTexture( Texture const* texture0, Texture const* texture1, Texture const* texture2 )
{
	if ( texture0 == nullptr )
	{
		texture0 = m_defaultTexture;
	}
	m_deviceContext->PSSetShaderResources( 0, 1, &texture0->m_shaderResourceView );
	if ( texture1 == nullptr )
	{
		texture1 = m_defaultTexture;
	}
	m_deviceContext->PSSetShaderResources( 1, 1, &texture1->m_shaderResourceView );
	if ( texture2 == nullptr )
	{
		texture2 = m_defaultTexture;
	}
	m_deviceContext->PSSetShaderResources( 2, 1, &texture2->m_shaderResourceView );
}


//----------------------------------------------------------------------------------------------------------------------
BitmapFont* Renderer::GetBitmapForFileName( char const* bitmapFontFilePathWithNoExtension )
{
	for ( int i = 0; i < m_loadedFonts.size(); i++ )
	{
		if ( m_loadedFonts[i]->m_fontFilePathNameWithNoExtension == bitmapFontFilePathWithNoExtension )
		{
			return m_loadedFonts[i];
		}
	}
	return nullptr;
}

//----------------------------------------------------------------------------------------------------------------------
BitmapFont* Renderer::CreateOrGetBitmapFont( char const* bitmapFontFilePathWithNoExtension )
{
	// See if we already have this texture previously loaded
	BitmapFont* bitmap = GetBitmapForFileName( bitmapFontFilePathWithNoExtension );
	if ( bitmap )
	{
		return bitmap ;
	}

	// Never seen this texture before!  Let's load it.
	BitmapFont* newBitmap = CreateOrGetBitmapFontFromFile( bitmapFontFilePathWithNoExtension );
	return newBitmap;
}
 
//----------------------------------------------------------------------------------------------------------------------
BitmapFont* Renderer::CreateOrGetBitmapFontFromFile( char const* bitmapFontFilePathWithNoExtension )
{
	std::string textureFilePath = Stringf( "%s.png", bitmapFontFilePathWithNoExtension );
	Texture* texture = CreateOrGetTextureFromFile( textureFilePath.c_str() );
	BitmapFont* bitmapfont = new BitmapFont( bitmapFontFilePathWithNoExtension, *texture );
	return bitmapfont;
}

//----------------------------------------------------------------------------------------------------------------------
void Renderer::BindShader( Shader* shader )
{
	if ( shader == nullptr )
	{
		shader = m_defaultShader;
	}		

	m_deviceContext->IASetInputLayout(	shader->m_inputLayout		 );
	m_deviceContext->VSSetShader	 (  shader->m_vertexShader, 0, 0 );
	m_deviceContext->PSSetShader	 (  shader->m_pixelShader, 0, 0  );
	m_currentShader = shader;
}

//----------------------------------------------------------------------------------------------------------------------
Shader* Renderer::CreateShader( char const* shaderName, char const* shaderSource, VertexType const& vertexType )
{
	// Initializing shader
	ShaderConfig	shaderConfig;
	Shader*			shader;
	shaderConfig.m_name = shaderName;
	shader				= new Shader( shaderConfig );

	//----------------------------------------------------------------------------------------------------------------------
	// Compile and create vertexShader
	const char* vertexEntryPoint = shader->m_config.m_vertexEntryPoint.c_str();
	std::vector<byte> vertexBytes;
	bool shaderResult; 

	shaderResult = CompileShaderToByteCode( vertexBytes, shaderName, shaderSource, vertexEntryPoint, "vs_5_0" );
	if ( shaderResult == false )
	{
		ERROR_AND_DIE( "vertexShader failed to compule" );
	}

	HRESULT hr;
	hr = m_device->CreateVertexShader( vertexBytes.data(), vertexBytes.size(), nullptr, &shader->m_vertexShader );
	if ( !SUCCEEDED( hr ) )
	{
		ERROR_AND_DIE( "Could not create vertexShader." );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Compile and create pixelShader
	const char* pixelEntryPoint = shader->m_config.m_pixelEntryPoint.c_str();
	std::vector<byte> pixelBytes;
	shaderResult = CompileShaderToByteCode( pixelBytes, shaderName, shaderSource, pixelEntryPoint, "ps_5_0" );
	if ( shaderResult == false )
	{
		ERROR_AND_DIE( "pixelShader failed to compile" );
	}

	hr = m_device->CreatePixelShader( pixelBytes.data(), pixelBytes.size(), nullptr, &shader->m_pixelShader );
	if ( !SUCCEEDED( hr ) )
	{
		ERROR_AND_DIE( "Could not create pixelShader." );
	}

	// If the name of the shader was found
	if ( vertexType == VertexType::VERTEX_PCUTBN )
	{
		// Create inputLayout // Specifies layout of Vertex_PNCU
		D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
		{
			{  "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,							 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{	  "COLOR", 0,  DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{  "TEXCOORD", 0,    DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{	"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{  "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{	 "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		int numElement = sizeof( inputElementDesc ) / sizeof( inputElementDesc[ 0 ] );
		hr = m_device->CreateInputLayout( inputElementDesc, numElement, vertexBytes.data(), vertexBytes.size(), &shader->m_inputLayout );
		if ( !SUCCEEDED( hr ) )
		{
			ERROR_AND_DIE( "Failed to create Vertex_PCUTBN inputLayout" );
		}

		//----------------------------------------------------------------------------------------------------------------------
		// adding shader we just created to vector of shaders
		m_loadedShaders.push_back( shader );

		return shader;
	}
	else	// Vertex_PCU
	{
		//----------------------------------------------------------------------------------------------------------------------
		// Create inputLayout // Specifies layout of Vertex_PCU
		D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
		{
			{  "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,							 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{	  "COLOR", 0,  DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{  "TEXCOORD", 0,    DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		//----------------------------------------------------------------------------------------------------------------------
		// #Question // what abt pixelBytes? // Should I just make one `std::vector<byte> bytes` instead of 1 each for vertex and pixel?
					 // am I passing in the hard coded shader?
		int numElement = sizeof( inputElementDesc ) / sizeof( inputElementDesc[0] );
		hr = m_device->CreateInputLayout( inputElementDesc, numElement, vertexBytes.data(), vertexBytes.size(), &shader->m_inputLayout );
		if ( !SUCCEEDED( hr ) )
		{
			ERROR_AND_DIE( "Failed to create Vertex_PCU inputLayout" );
		}

		//----------------------------------------------------------------------------------------------------------------------
		// adding shader we just created to vector of shaders
		m_loadedShaders.push_back( shader );
	
		return shader;
	}
}

//----------------------------------------------------------------------------------------------------------------------
Shader* Renderer::CreateShader( char const* shaderName, VertexType const& vertexType )
{
	std::string hlslStringFileName;
	hlslStringFileName = shaderName + std::string(".png");
	std::string outString;

	FileReadToString( outString, hlslStringFileName );

	Shader* shader;
	shader = CreateShader( shaderName, outString.c_str(), vertexType );

	return shader;
}

//----------------------------------------------------------------------------------------------------------------------
Shader* Renderer::CreateOrGetShaderByName( char const* newShaderName, VertexType vertexType )
{
	// check if shader already exists
	for ( int index = 0; index < m_loadedShaders.size(); index++ )
	{
		Shader* loadedShader = m_loadedShaders[index];
		if ( loadedShader != nullptr )
		{
			std::string loadedShaderName = loadedShader->m_config.m_name;
			if ( loadedShaderName == newShaderName )
			{
				return loadedShader;
			}
		}
	}
	// only create new shaders, if it doesn't exist in cache
	std::string shaderFilePath = newShaderName;
	shaderFilePath += ".hlsl";
	std::string shaderSource;
	if ( FileReadToString( shaderSource, shaderFilePath ) )
	{
		Shader* shader = CreateShader( newShaderName, shaderSource.c_str(), vertexType );
		return shader;
	}
	else
	{
		ERROR_AND_DIE( Stringf( "Unable to Load Shader form file: %s", shaderFilePath.c_str() ) );
	}
}

//----------------------------------------------------------------------------------------------------------------------
bool Renderer::CompileShaderToByteCode( std::vector<unsigned char>& outByteCode, char const* name, char const* source, char const* entryPoint, char const* target )
{
	//----------------------------------------------------------------------------------------------------------------------
	DWORD flags = D3DCOMPILE_OPTIMIZATION_LEVEL3;
#if defined( ENGINE_DEBUG_RENDER )
	flags = D3DCOMPILE_DEBUG;
	flags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	ID3DBlob* codeBlob	= nullptr;
	ID3DBlob* errorBlob	= nullptr;

	HRESULT hr;

	// Compile shader
	hr = D3DCompile( source, strlen( source ), name, nullptr, nullptr, entryPoint, target, flags, 0, &codeBlob, &errorBlob );
	if ( !SUCCEEDED( hr ) )
	{
		DebuggerPrintf( static_cast<const char*>( errorBlob->GetBufferPointer() ) );
		ERROR_AND_DIE( "Shader failed to compile" );
	}

	// Resize vector to fit buffer size
	outByteCode.resize( codeBlob->GetBufferSize() );
	// Copy memory from vector as specified by buffer pointer and size
	std::memcpy( outByteCode.data(), codeBlob->GetBufferPointer(),codeBlob->GetBufferSize() );

	DX_SAFE_RELEASE( codeBlob  );
	DX_SAFE_RELEASE( errorBlob );

	return true;
}

//----------------------------------------------------------------------------------------------------------------------
VertexBuffer* Renderer::CreateVertexBuffer( const size_t size ) 
{
	//----------------------------------------------------------------------------------------------------------------------
	// Creating Vertex Buffer
	D3D11_BUFFER_DESC m_bufferDesc	= { 0 };
	m_bufferDesc.Usage				= D3D11_USAGE_DYNAMIC;
	m_bufferDesc.ByteWidth			= static_cast<UINT>( size );
	m_bufferDesc.BindFlags			= D3D11_BIND_VERTEX_BUFFER;
	m_bufferDesc.CPUAccessFlags		= D3D11_CPU_ACCESS_WRITE;
	
	VertexBuffer* vertexBuffer = new VertexBuffer( size );
	
	HRESULT hr;
	hr = m_device->CreateBuffer( &m_bufferDesc, nullptr, &vertexBuffer->m_buffer );
	if ( !SUCCEEDED( hr ) )
	{
		ERROR_AND_DIE( "Could not create verexBuffer." );
	}

	return vertexBuffer;
}

//----------------------------------------------------------------------------------------------------------------------
VertexBuffer* Renderer::CreateVertexBuffer( const size_t size, unsigned int stride )
{
	//----------------------------------------------------------------------------------------------------------------------
	// Creating Vertex Buffer
	D3D11_BUFFER_DESC m_bufferDesc	= { 0 };
	m_bufferDesc.Usage				= D3D11_USAGE_DYNAMIC;
	m_bufferDesc.ByteWidth			= static_cast<UINT>( size );
	m_bufferDesc.BindFlags			= D3D11_BIND_VERTEX_BUFFER;
	m_bufferDesc.CPUAccessFlags		= D3D11_CPU_ACCESS_WRITE;

	VertexBuffer* vertexBuffer		= new VertexBuffer( size );
	vertexBuffer->m_stride			= stride;

	HRESULT hr;
	hr = m_device->CreateBuffer( &m_bufferDesc, nullptr, &vertexBuffer->m_buffer );
	if ( !SUCCEEDED( hr ) )
	{
		ERROR_AND_DIE( "Could not create vertexBuffer." );
	}

	return vertexBuffer;;
}

//----------------------------------------------------------------------------------------------------------------------
IndexBuffer* Renderer::CreateIndexBuffer( const size_t size )
{
	//----------------------------------------------------------------------------------------------------------------------
	// Creating Index Buffer
	D3D11_BUFFER_DESC bufferDesc	= { 0 };
	bufferDesc.Usage				= D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth			= static_cast<UINT>( size );
	bufferDesc.BindFlags			= D3D11_BIND_INDEX_BUFFER;
	bufferDesc.CPUAccessFlags		= D3D11_CPU_ACCESS_WRITE;

	IndexBuffer* indexBuffer = new IndexBuffer( size );

	HRESULT hr;
	hr = m_device->CreateBuffer( &bufferDesc, nullptr, &indexBuffer->m_buffer );
	if ( !SUCCEEDED( hr ) )
	{
		ERROR_AND_DIE( "Could not create IndexBuffer." );
	}

	return indexBuffer;
}

//----------------------------------------------------------------------------------------------------------------------
void Renderer::Copy_CPU_To_GPU( const void* data, size_t size, VertexBuffer*& vbo, unsigned int stride )
{
	// check if immediate vertexBuffer is resizeable on the fly
	// check if vertexBuffer is large enough for data being passed
	// if not, make vertexBuffer bigger to fit. 
	// DO NOT INTRODUCE MEMORY LEAKS!
	
	// checking if buffer is large enough // if too small, resize
	if ( size > vbo->m_size )
	{
		delete vbo;
		vbo = CreateVertexBuffer( size );
		vbo->m_stride = stride;
	}

	D3D11_MAPPED_SUBRESOURCE m_mappedSubResource;
	HRESULT hr;
	hr = m_deviceContext->Map( vbo->m_buffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &m_mappedSubResource );
	if ( !SUCCEEDED( hr ) )
	{
		ERROR_AND_DIE( "Failed to map buffer." );
	}

	memcpy( m_mappedSubResource.pData, data, size );
	m_deviceContext->Unmap( vbo->m_buffer, NULL );
}


//----------------------------------------------------------------------------------------------------------------------
void Renderer::Copy_CPU_To_GPU( const void* data, size_t size, IndexBuffer*& ibo )
{
	// check if immediate vertexBuffer is resizeable on the fly
	// check if vertexBuffer is large enough for data being passed
	// if not, make vertexBuffer bigger to fit. 
	// DO NOT INTRODUCE MEMORY LEAKS!

	// checking if buffer is large enough // if too small, resize
	if ( size > ibo->m_size )
	{
		delete ibo;
		ibo = CreateIndexBuffer( size );
	}

	D3D11_MAPPED_SUBRESOURCE m_mappedSubResource;
	HRESULT hr;
	hr = m_deviceContext->Map( ibo->m_buffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &m_mappedSubResource );
	if ( !SUCCEEDED( hr ) )
	{
		ERROR_AND_DIE( "Failed to map buffer." );
	}

	memcpy( m_mappedSubResource.pData, data, size );
	m_deviceContext->Unmap( ibo->m_buffer, NULL );
}

//----------------------------------------------------------------------------------------------------------------------
void Renderer::BindVertexBuffer( VertexBuffer* vbo, PrimitiveTopology primitiveTopology )
{
	UINT strides = vbo->GetStride();
	UINT offset  = 0;
	m_deviceContext->IASetVertexBuffers( 0, 1, &vbo->m_buffer, &strides, &offset );
	m_deviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY( primitiveTopology ) );
}

//----------------------------------------------------------------------------------------------------------------------
void Renderer::BindIndexBuffer( IndexBuffer* ibo )
{
	m_deviceContext->IASetIndexBuffer( ibo->m_buffer, DXGI_FORMAT_R32_UINT, 0 );
}

//----------------------------------------------------------------------------------------------------------------------
void Renderer::DrawVertexBuffer( VertexBuffer* vbo, int vertexCount, int vertexOffset, PrimitiveTopology primitiveTopology )
{
	SetStateIfChanged();
	BindVertexBuffer( vbo, primitiveTopology );
	m_deviceContext->Draw( vertexCount, vertexOffset );
}

//----------------------------------------------------------------------------------------------------------------------
void Renderer::DrawVertexAndIndexBuffer( VertexBuffer* vbo, IndexBuffer* ibo, int indexCount, int indexOffset, int vertexOffset )
{
	SetStateIfChanged();
	BindVertexBuffer( vbo );
	BindIndexBuffer( ibo );

	m_deviceContext->DrawIndexed( indexCount, indexOffset, vertexOffset );
}

//----------------------------------------------------------------------------------------------------------------------
ConstantBuffer* Renderer::CreateConstantBuffer( const size_t size )
{
	D3D11_BUFFER_DESC constantBufferDesc		= { 0 };
	constantBufferDesc.Usage					= D3D11_USAGE_DYNAMIC;
	constantBufferDesc.ByteWidth				= static_cast<UINT>( size );
	constantBufferDesc.BindFlags				= D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDesc.CPUAccessFlags			= D3D11_CPU_ACCESS_WRITE;

	ConstantBuffer* constantBuffer = new ConstantBuffer( size );

	HRESULT hr;
	hr = m_device->CreateBuffer( &constantBufferDesc, nullptr, &constantBuffer->m_buffer );
	if ( !SUCCEEDED( hr ) )
	{
		ERROR_AND_DIE( "Could not create constant buffer." );
	}

	return constantBuffer;
}

//----------------------------------------------------------------------------------------------------------------------
void Renderer::Copy_CPU_To_GPU( const void* data, size_t size, ConstantBuffer* cbo )
{
	D3D11_MAPPED_SUBRESOURCE m_mappedSubResource;
	HRESULT hr;
	hr = m_deviceContext->Map( cbo->m_buffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &m_mappedSubResource );
	if ( !SUCCEEDED( hr ) )
	{
		ERROR_AND_DIE( "Failed to map constantBuffer." );
	}

	memcpy( m_mappedSubResource.pData, data, size );
	m_deviceContext->Unmap( cbo->m_buffer, NULL );
}

//----------------------------------------------------------------------------------------------------------------------
void Renderer::BindConstantBuffer( int slot, ConstantBuffer* cbo )
{
	m_deviceContext->VSSetConstantBuffers( slot, 1, &cbo->m_buffer );
	m_deviceContext->PSSetConstantBuffers( slot, 1, &cbo->m_buffer );
}

//----------------------------------------------------------------------------------------------------------------------
// Model means Local
void Renderer::SetModelConstants( Mat44 const& modelMatrix, Rgba8 const& modelColor )
{
	ModelConstants modelConstants;
	modelConstants.ModelMatrix = modelMatrix;
	modelColor.GetAsFloat( modelConstants.ModelColor );

	Copy_CPU_To_GPU( &modelConstants, sizeof(ModelConstants), m_modelCBO );
	BindConstantBuffer( k_modelConstantsSlot, m_modelCBO );
}


//----------------------------------------------------------------------------------------------------------------------
void Renderer::SetLightingConstants( Vec3 const& sunDirection,	   float sunIntensity,  float ambientIntensity, 
									 Vec3 const& worldEyePosition, int normalMode,	    int specularMode, 
									 float specularIntensity,	   float specularPower )
{	
	LightingConstants lightingConstants;
	lightingConstants.SunDirection		= sunDirection;
	lightingConstants.SunIntensity		= sunIntensity;
	lightingConstants.AmbientIntensity	= ambientIntensity;
	lightingConstants.WorldEyePosition	= worldEyePosition;
	lightingConstants.NormalMode		= normalMode;
	lightingConstants.SpecularMode		= specularMode;
	lightingConstants.SpecularIntensity	= specularIntensity;
	lightingConstants.SpecularPower		= specularPower;

	Copy_CPU_To_GPU( &lightingConstants, m_lightingCBO->m_size, m_lightingCBO );
	BindConstantBuffer( k_lightingConstantsSlot, m_lightingCBO );
}


//----------------------------------------------------------------------------------------------------------------------
void Renderer::InitializingBlendState()
{
	D3D11_BLEND_DESC blendDesc							= { 0 };
	blendDesc.RenderTarget[0].BlendEnable				= true;
	blendDesc.RenderTarget[0].BlendOp					= D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha				= D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha			= D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOpAlpha				= D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask		= D3D11_COLOR_WRITE_ENABLE_ALL;
	
	//----------------------------------------------------------------------------------------------------------------------
	// Initializing BlendMode Opaque 
	blendDesc.RenderTarget[0].SrcBlend	= D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlend	= D3D11_BLEND_ZERO;

	HRESULT hr;
	hr = m_device->CreateBlendState( &blendDesc, &m_blendStates[(int)BlendMode::OPAQUE] );
	if ( !SUCCEEDED( hr ) )
	{
		ERROR_AND_DIE( "Could Not Create Opaque Blend State." );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Initializing BlendMode Alpha
	blendDesc.RenderTarget[0].SrcBlend	= D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend	= D3D11_BLEND_INV_SRC_ALPHA;

	hr = m_device->CreateBlendState( &blendDesc, &m_blendStates[(int)BlendMode::ALPHA] );
	if ( !SUCCEEDED( hr ) )
	{
		ERROR_AND_DIE( "Could Not Create Alpha Blend State." );
	}
	
	//----------------------------------------------------------------------------------------------------------------------
	// Initializing BlendMode Additive
	blendDesc.RenderTarget[0].SrcBlend	= D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlend	= D3D11_BLEND_ONE;

	hr = m_device->CreateBlendState( &blendDesc, &m_blendStates[(int)BlendMode::ADDITIVE] );
	if ( !SUCCEEDED( hr ) )
	{
		ERROR_AND_DIE( "Could Not Create Additive Blend State." );
	}

//	float floatArray[4] = { 0.0f , 0.0f , 0.0f , 0.0f };
//	m_deviceContext->OMSetBlendState( m_currentBlendState, floatArray, 0xffffffff );

//	//----------------------------------------------------------------------------------------------------------------------
//	//----------------------------------------------------------------------------------------------------------------------
//	D3D11_BLEND_DESC blendDesc = { 0 };
//	blendDesc.RenderTarget[0].BlendEnable = true;
//
//	if ( blendMode == BlendMode::ALPHA )
//	{
//		blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
//		blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
//	}
//	else if ( blendMode == BlendMode::ADDITIVE )
//	{
//		blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
//		blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
//	}
//	else if ( blendMode == BlendMode::OPAQUE )
//	{
//		blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
//		blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
//	}
//
//	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
//	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
//	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
//	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
//	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
//
//	HRESULT hr;
//	hr = m_device->CreateBlendState( &blendDesc, &m_currentBlendState );
//	if ( !SUCCEEDED( hr ) )
//	{
//		ERROR_AND_DIE( "Could Not Create Blend State." );
//	}
//
//	float floatArray[4] = { 0.0f , 0.0f , 0.0f , 0.0f };
//	m_deviceContext->OMSetBlendState( m_currentBlendState, floatArray, 0xffffffff );
//	//----------------------------------------------------------------------------------------------------------------------
//	//----------------------------------------------------------------------------------------------------------------------
}

//----------------------------------------------------------------------------------------------------------------------
void Renderer::InitializingSamplerState()
{
	//----------------------------------------------------------------------------------------------------------------------
	// Sampler State Point Clamped
	D3D11_SAMPLER_DESC samplerDesc	= { 0 };
	samplerDesc.Filter				= D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU			= D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV			= D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW			= D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.ComparisonFunc		= D3D11_COMPARISON_NEVER;
	samplerDesc.MaxLOD				= D3D11_FLOAT32_MAX;

	// Creating Point Clamped samplerState
	HRESULT hr;
	hr = m_device->CreateSamplerState( &samplerDesc, &m_samplerStates[(int)SamplerMode::POINT_CLAMP] );
	if ( !SUCCEEDED( hr ) )
	{
		ERROR_AND_DIE( "Could not create Point Clamped sampler state." );
	}

//	// Set Point Clamped samplers
//	m_deviceContext->PSSetSamplers( 0, 1, &m_samplerStates[(int)SamplerMode::POINT_CLAMP] );

	//----------------------------------------------------------------------------------------------------------------------
	// Sampler State Bi-Linear Wrapped
	samplerDesc.Filter				= D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU			= D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV			= D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW			= D3D11_TEXTURE_ADDRESS_WRAP;

	// Creating Bi-Linear samplerState
	hr = m_device->CreateSamplerState( &samplerDesc, &m_samplerStates[(int)SamplerMode::BILINEAR_WRAP] );
	if ( !SUCCEEDED( hr ) )
	{
		ERROR_AND_DIE( "Could not create Bi-Linear sampler state." );
	}

//	// Set Bi-Linear samplers
//	m_deviceContext->PSSetSamplers( 0, 1, &m_samplerStates[(int)SamplerMode::BILINEAR_WRAP] );
}

//----------------------------------------------------------------------------------------------------------------------
void Renderer::InitializingRasterizerState()
{
	//----------------------------------------------------------------------------------------------------------------------
	// Specifying Solid Cull None rasterizerDesc values
	D3D11_RASTERIZER_DESC rasterizerDesc	= { 0 };
	rasterizerDesc.FillMode					= D3D11_FILL_SOLID;
	rasterizerDesc.CullMode					= D3D11_CULL_NONE;

	// Create Solid Cull None Rasterizer State
	HRESULT hr;
	hr = m_device->CreateRasterizerState( &rasterizerDesc, &m_rasterizerStates[(int)RasterizerMode::SOLID_CULL_NONE] );
	if ( !SUCCEEDED( hr ) )
	{
		ERROR_AND_DIE( "Could not create Solid Cull None rasterizer state." );
	}

//	// Set the Solid Cull None Rasterizer State
//	m_deviceContext->RSSetState( m_currentRasterizerState );

	//----------------------------------------------------------------------------------------------------------------------
	// Specifying Solid Cull Back rasterizerDesc values
	rasterizerDesc.FillMode					= D3D11_FILL_SOLID;
	rasterizerDesc.CullMode					= D3D11_CULL_BACK;
	rasterizerDesc.DepthClipEnable			= true;
	rasterizerDesc.AntialiasedLineEnable	= true;
	rasterizerDesc.FrontCounterClockwise	= true;

	// Create Solid Cull Back Rasterizer State
	hr = m_device->CreateRasterizerState( &rasterizerDesc, &m_rasterizerStates[(int)RasterizerMode::SOLID_CULL_BACK] );
	if ( !SUCCEEDED( hr ) )
	{
		ERROR_AND_DIE( "Could not create Solid Cull Back rasterizer state." );
	}

//	// Set the Solid Cull Back Rasterizer State
//	m_deviceContext->RSSetState( m_currentRasterizerState );

	//----------------------------------------------------------------------------------------------------------------------
	// Specifying Wireframe Cull None rasterizerDesc values
	rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;

	// Create Wireframe Cull None Rasterizer State
	hr = m_device->CreateRasterizerState( &rasterizerDesc, &m_rasterizerStates[(int)RasterizerMode::WIREFRAME_CULL_NONE] );
	if ( !SUCCEEDED( hr ) )
	{
		ERROR_AND_DIE( "Could not create Wireframe Cull None rasterizer state." );
	}

//	// Set the Wireframe Cull None Rasterizer State
//	m_deviceContext->RSSetState( m_currentRasterizerState );

	//----------------------------------------------------------------------------------------------------------------------
	// Specifying Wireframe Cull Back rasterizerDesc values
	rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;

	// Create Wireframe Cull Back Rasterizer State
	hr = m_device->CreateRasterizerState( &rasterizerDesc, &m_rasterizerStates[(int)RasterizerMode::WIREFRAME_CULL_BACK] );
	if ( !SUCCEEDED( hr ) )
	{
		ERROR_AND_DIE( "Could not create Wireframe Cull Back rasterizer state." );
	}

//	// Set the Wireframe Cull Back Rasterizer State
//	m_deviceContext->RSSetState( m_currentRasterizerState );
}

//----------------------------------------------------------------------------------------------------------------------
void Renderer::InitializingDepthState()
{
	//----------------------------------------------------------------------------------------------------------------------
	// Create and set Depth Stencil States
	// Creating and specifying a Depth Enabled Stencil description
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc	= { 0 };
	depthStencilDesc.DepthEnable				= true;
	depthStencilDesc.DepthWriteMask				= D3D11_DEPTH_WRITE_MASK_ALL;		// Means pixels behind will be blocked since this is opaque
	depthStencilDesc.DepthFunc					= D3D11_COMPARISON_LESS_EQUAL;

	// Create Depth Enabled Stencil State
	HRESULT hr;
	hr = m_device->CreateDepthStencilState( &depthStencilDesc, &m_depthStencilStates[(int)DepthMode::ENABLED]  );
	if ( !SUCCEEDED( hr ) )
	{
		ERROR_AND_DIE( "Could not create Depth Enabled Stencil State" );
	}

//	// Set Depth Enabled Stencil State
//	m_deviceContext->OMSetDepthStencilState( m_currentDepthStencilState, 0 );

	//----------------------------------------------------------------------------------------------------------------------
	// Creating and specifying a Depth Disabled Stencil description
	depthStencilDesc.DepthWriteMask				= D3D11_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.DepthFunc					= D3D11_COMPARISON_ALWAYS;

	// Create Depth Disabled Stencil State
	hr = m_device->CreateDepthStencilState( &depthStencilDesc, &m_depthStencilStates[(int)DepthMode::DISABLED] );
	if ( !SUCCEEDED( hr ) )
	{
		ERROR_AND_DIE( "Could not create Depth Disabled Stencil State" );
	}

//	// Set Depth Stencil State
//	m_deviceContext->OMSetDepthStencilState( m_currentDepthStencilState, 0 );
}

//----------------------------------------------------------------------------------------------------------------------
// Change currentState if currentState != m_desiredState; Then set 
void Renderer::SetStateIfChanged()
{
	if ( m_currentBlendState != m_blendStates[static_cast<int>(m_desiredBlendMode)] )
	{
		m_currentBlendState = m_blendStates[static_cast<int>(m_desiredBlendMode)];
		// Set Blend State
		float floatArray[4] = { 0.0f , 0.0f , 0.0f , 0.0f };
		m_deviceContext->OMSetBlendState( m_currentBlendState, floatArray, 0xffffffff );
	}

	if ( m_currentSamplerState != m_samplerStates[static_cast<int>(m_desiredSamplerMode)] )
	{
		m_currentSamplerState = m_samplerStates[static_cast<int>(m_desiredSamplerMode)];

		// Set samplers
		m_deviceContext->PSSetSamplers( 0, 1, &m_currentSamplerState );
	}

	if ( m_currentRasterizerState != m_rasterizerStates[static_cast<int>(m_desiredRasterizerMode)] )
	{
		m_currentRasterizerState = m_rasterizerStates[static_cast<int>(m_desiredRasterizerMode)];

		// Set the Rasterizer State
		m_deviceContext->RSSetState( m_currentRasterizerState );
	}

	if ( m_currentDepthStencilState != m_depthStencilStates[static_cast<int>(m_desiredDepthmode)] )
	{
		m_currentDepthStencilState = m_depthStencilStates[static_cast<int>(m_desiredDepthmode)];

		// Set Depth Stencil State
		m_deviceContext->OMSetDepthStencilState( m_currentDepthStencilState, 0 );
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Renderer::SetBlendMode( BlendMode blendMode )
{
	m_desiredBlendMode = blendMode;
}

//----------------------------------------------------------------------------------------------------------------------
void Renderer::SetSamplerMode( SamplerMode samplerMode )
{
	m_desiredSamplerMode = samplerMode;

	//----------------------------------------------------------------------------------------------------------------------
	// Specifying samplerState description
//	D3D11_SAMPLER_DESC samplerDesc = { 0 };
//
//	if ( samplerMode == SamplerMode::POINT_CLAMP )
//	{
//		samplerDesc.Filter			= D3D11_FILTER_MIN_MAG_MIP_POINT;
//		samplerDesc.AddressU		= D3D11_TEXTURE_ADDRESS_CLAMP;
//		samplerDesc.AddressV		= D3D11_TEXTURE_ADDRESS_CLAMP;
//		samplerDesc.AddressW		= D3D11_TEXTURE_ADDRESS_CLAMP;
//	}
//	if ( samplerMode == SamplerMode::BILINEAR_WRAP )
//	{
//		samplerDesc.Filter			= D3D11_FILTER_MIN_MAG_MIP_LINEAR;
//		samplerDesc.AddressU		= D3D11_TEXTURE_ADDRESS_WRAP;
//		samplerDesc.AddressV		= D3D11_TEXTURE_ADDRESS_WRAP;
//		samplerDesc.AddressW		= D3D11_TEXTURE_ADDRESS_WRAP;
//	}
//	
//	samplerDesc.ComparisonFunc	= D3D11_COMPARISON_NEVER;
//	samplerDesc.MaxLOD			= D3D11_FLOAT32_MAX;
//
//	// Creating samplerState
//	HRESULT hr;
//	hr = m_device->CreateSamplerState( &samplerDesc, &m_currentSamplerState );
//	if ( !SUCCEEDED( hr ) )
//	{
//		ERROR_AND_DIE( "Could not create sampler state." );
//	}
//
//	// Set samplers
//	m_deviceContext->PSSetSamplers( 0, 1, &m_currentSamplerState );
}

//----------------------------------------------------------------------------------------------------------------------
void Renderer::SetRasterizerMode( RasterizerMode rasterizerMode )
{
	m_desiredRasterizerMode = rasterizerMode;

//	//----------------------------------------------------------------------------------------------------------------------
//	// Specifying rasterizerDesc values
//	D3D11_RASTERIZER_DESC rasterizerDesc = { 0 };
//	
//	if ( rasterizerMode == RasterizerMode::SOLID_CULL_NONE )
//	{
//		rasterizerDesc.FillMode = D3D11_FILL_SOLID;
//		rasterizerDesc.CullMode = D3D11_CULL_NONE;
//	}
//	if ( rasterizerMode == RasterizerMode::SOLID_CULL_BACK)
//	{
//		rasterizerDesc.FillMode = D3D11_FILL_SOLID;
//		rasterizerDesc.CullMode = D3D11_CULL_BACK;
//	}
//	if ( rasterizerMode == RasterizerMode::WIREFRAME_CULL_NONE )
//	{
//		rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
//		rasterizerDesc.CullMode = D3D11_CULL_NONE;
//	}
//	if ( rasterizerMode == RasterizerMode::WIREFRAME_CULL_BACK )
//	{
//		rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
//		rasterizerDesc.CullMode = D3D11_CULL_BACK;
//	}
//
//	rasterizerDesc.DepthClipEnable = true;
//	rasterizerDesc.AntialiasedLineEnable = true;
//	rasterizerDesc.FrontCounterClockwise = true;
//
//	// Create Rasterizer State
//	HRESULT hr;
//	hr = m_device->CreateRasterizerState( &rasterizerDesc, &m_currentRasterizerState );
//	if ( !SUCCEEDED( hr ) )
//	{
//		ERROR_AND_DIE( "Could not create rasterizer state." );
//	}
//
//	// Set the Rasterizer State
//	m_deviceContext->RSSetState( m_currentRasterizerState );
}

//----------------------------------------------------------------------------------------------------------------------
void Renderer::SetDepthMode( DepthMode depthMode )
{
	m_desiredDepthmode = depthMode;

//	// Create and set a Depth Stencil State
//	// Creating and Specifying Depth Stencil description
//	D3D11_DEPTH_STENCIL_DESC depthStencilDesc	= { 0 };
//	depthStencilDesc.DepthEnable				= true;
//
//	if ( depthMode == DepthMode::DISABLED )
//	{
//		depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
//		depthStencilDesc.DepthFunc		= D3D11_COMPARISON_ALWAYS;
//	}
//	if ( depthMode == DepthMode::ENABLED )
//	{
//		depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;		// Means pixels behind will be blocked since this is opaque
//		depthStencilDesc.DepthFunc		= D3D11_COMPARISON_LESS_EQUAL;
//	}
//
//	// Create Depth Stencil State
//	HRESULT hr;
//	hr = m_device->CreateDepthStencilState( &depthStencilDesc, &m_currentDepthStencilState );
//	if ( !SUCCEEDED( hr ) )
//	{
//		ERROR_AND_DIE( "Could not create Depth Stencil State" );
//	}
//
//	// Set Depth Stencil State
//	m_deviceContext->OMSetDepthStencilState( m_currentDepthStencilState, 0 );
}


//----------------------------------------------------------------------------------------------------------------------
VertexType Renderer::GetVertexTypeAsEnum( std::string const& vertexType )
{
	if ( vertexType == "Vertex_PCU" )
	{
		return VertexType::VERTEX_PCU;
	}
	else if ( vertexType == "Vertex_PCUTBN" )
	{
		return VertexType::VERTEX_PCUTBN;
	}
	else
	{
		return VertexType::NONE;
	}
}