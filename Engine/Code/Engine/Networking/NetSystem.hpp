#pragma once

#include "Engine/Core/EngineCommon.hpp"
#include <string>

#define WIN32_LEAN_AND_MEAN
#pragma comment( lib, "Ws2_32.lib" )
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

//----------------------------------------------------------------------------------------------------------------------
class	NetSystem;
extern	NetSystem* g_theNetSystem;

//----------------------------------------------------------------------------------------------------------------------
struct NetSystemConfig
{
	std::string		m_modeString;
	std::string		m_hostAddressString;
	int				m_sendBufferSize		= 2048;
	int				m_recvBufferSize		= 2048;
};

//----------------------------------------------------------------------------------------------------------------------
class NetSystem
{
public:
	//----------------------------------------------------------------------------------------------------------------------
	enum class Mode
	{
		NONE = 0,
		CLIENT,
		SERVER,
	};

	//----------------------------------------------------------------------------------------------------------------------
	enum class ClientState
	{
		INVALID,
		READY_TO_CONNECT,
		CONNECTING,
		CONNECTED,
	};

	//----------------------------------------------------------------------------------------------------------------------
	enum class ServerState
	{
		INVALID,
		LISTENING,
		CONNECTED,
	};

public:
	NetSystem( const NetSystemConfig& config );
	~NetSystem();

	void Startup();
	void Shutdown();
	void BeginFrame();
	void EndFrame();

	bool IsNetworking();
	bool IsConnected();
	std::string GetModeEnumAsString();

	static bool Event_RemoteCommand( EventArgs& args );
	static bool Event_BurstTest( EventArgs& args );
	static bool SendMessage( std::string const& stringToSend );

public:
	// Core Variables
	NetSystemConfig		m_config;
	Mode				m_mode		= Mode::NONE;

	// State variables
	ClientState m_clientState = ClientState::INVALID;
	ServerState m_serverState = ServerState::INVALID;

	// Socket related variables
	uintptr_t		m_clientSocket	= ~0ull;
	uintptr_t		m_listenSocket	= ~0ull;
	unsigned long	m_hostAddress	= 0;
	unsigned short	m_hostPort		= 0;

	// Send and receive buffers
	char* m_sendBuffer = nullptr;
	char* m_recvBuffer = nullptr;

	std::vector<std::string>	m_sendQueue;
	std::string					m_recvRemaining;
};