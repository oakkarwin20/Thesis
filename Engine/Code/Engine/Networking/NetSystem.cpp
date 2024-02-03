#include "Engine/Networking/NetSystem.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/DevConsole.hpp"

//----------------------------------------------------------------------------------------------------------------------
NetSystem* g_theNetSystem = nullptr;

//----------------------------------------------------------------------------------------------------------------------
NetSystem::NetSystem( const NetSystemConfig& config )
{
	m_config = config;

	// Initialize buffers based on sizes specified in the config 
	m_sendBuffer = new char[ m_config.m_sendBufferSize ];
	m_recvBuffer = new char[ m_config.m_recvBufferSize ];

	// Initialize the mode based on the config modeString
	std::string const& modeString = m_config.m_modeString;
	if ( modeString == "Server" )
	{
		m_mode = Mode::SERVER;
	}
	else if ( modeString == "Client" )
	{
		m_mode = Mode::CLIENT;
	}
	else
	{
		m_mode = Mode::NONE;
	}
}

//----------------------------------------------------------------------------------------------------------------------
NetSystem::~NetSystem()
{
	delete[] m_sendBuffer;
	delete[] m_recvBuffer;
}

//----------------------------------------------------------------------------------------------------------------------
void NetSystem::Startup()
{
	g_theEventSystem->SubscribeToEvent( "RemoteCommand", NetSystem::Event_RemoteCommand );
	g_theEventSystem->SubscribeToEvent(		"BurstTest", NetSystem::Event_BurstTest		);

	if ( m_mode == Mode::CLIENT )
	{
		// Startup Windows sockets
		WSADATA data;
		int startupResult = WSAStartup( MAKEWORD( 2, 2 ), &data );
		if ( startupResult == SOCKET_ERROR )
		{
			// Ensure socket is started up correctly
			ERROR_AND_DIE( "Socket did not startup correctly!" );
		}

		// Create client socket
		m_clientSocket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

		// Set blocking mode
		unsigned long	blockingMode	= 1;		// 1 means none blocking, 0 means blocking
		int				blockingResult	= ioctlsocket( m_clientSocket, FIONBIO, &blockingMode );
		if ( blockingResult == SOCKET_ERROR )
		{
			// Ensure blocking mode binds the clientSocket correctly
			ERROR_AND_DIE( "Blocking mode did not bind the clientSocket correctly!" );
		}
		m_clientState = ClientState::READY_TO_CONNECT;

		// Get host address from string
		IN_ADDR address;
		int hostAddressResult = inet_pton( AF_INET, m_config.m_hostAddressString.c_str(), &address );
		if ( hostAddressResult == SOCKET_ERROR )
		{
			ERROR_AND_DIE( "The IP address was not converted from v4 to v6 correctly!" );
		}
		m_hostAddress = ntohl( address.S_un.S_addr );

		// Get host port from string
		Strings stringListDelimitedOnColon		= SplitStringOnDelimiter( m_config.m_hostAddressString, ':' );
		size_t  stringListDelimitedOnColonSize	= stringListDelimitedOnColon.size();
		m_hostPort								= unsigned short( ( atoi( stringListDelimitedOnColon[stringListDelimitedOnColonSize - 1].c_str() ) ) );					
	}
	else if ( m_mode == Mode::SERVER )
	{
		// Startup windows sockets
		WSADATA data;
		int startupResult = WSAStartup( MAKEWORD( 2, 2 ), &data );
		if ( startupResult == SOCKET_ERROR )
		{
			// Ensure socket is started up correctly
			ERROR_AND_DIE( "Socket did not startup correctly!" );
		}

		// Create listen socket
		m_listenSocket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

		// Set blocking mode
		unsigned long	blockingMode	= 1;
		int				blockingResult	= ioctlsocket( m_listenSocket, FIONBIO, &blockingMode );
		if ( blockingResult == SOCKET_ERROR )
		{
			// Ensure blocking mode binds the listenSocket correctly
			ERROR_AND_DIE( "Blocking mode did not bind the listenSocket correctly!" );
		}

		// Get host port from string
		m_hostAddress = INADDR_ANY;
		Strings stringListDelimitedList		= SplitStringOnDelimiter( m_config.m_hostAddressString, ':' );
		size_t  stringListDelimitedListSize = stringListDelimitedList.size();
		m_hostPort							= unsigned short( ( atoi( stringListDelimitedList[ stringListDelimitedListSize - 1 ].c_str() ) ) );

		// Bind the listen socket to a port
		sockaddr_in address;
		address.sin_family				= AF_INET;
		address.sin_addr.S_un.S_addr	= htonl( m_hostAddress );
		address.sin_port				= htons( m_hostPort );
		int listenSocketResult			= bind( m_listenSocket, (sockaddr*)&address, int( sizeof( address) ) );
		if ( listenSocketResult == SOCKET_ERROR )
		{
			// Ensure blocking mode binds the listenSocket binds to a port correctly
			m_serverState = ServerState::INVALID;
			ERROR_AND_DIE( "ListenSocket did NOT bind to a port correctly!" );
		}
		m_serverState = ServerState::LISTENING;

		
		// Listen for connections to accept
		int acceptedConnectionResult = listen( m_listenSocket, SOMAXCONN );
		if ( acceptedConnectionResult == SOCKET_ERROR )
		{
			ERROR_AND_DIE( "Something went wrong while listening for connections to accept" );
		}
	}
	else if ( m_mode == Mode::NONE )
	{
	}
}

//----------------------------------------------------------------------------------------------------------------------
void NetSystem::Shutdown()
{
	if ( m_mode == Mode::CLIENT )
	{
		// Close all open sockets
		closesocket( m_clientSocket );

		// Shutdown Windows sockets
		WSACleanup();
	}
	else if ( m_mode == Mode::SERVER )
	{
		// Close all open sockets
		closesocket( m_clientSocket );
		closesocket( m_listenSocket );

		// Shutdown Windows sockets
		WSACleanup();
	}
	else if ( m_mode == Mode::NONE )
	{
	}
}

//----------------------------------------------------------------------------------------------------------------------
void NetSystem::BeginFrame()
{
	//----------------------------------------------------------------------------------------------------------------------
	if ( m_mode == Mode::CLIENT )
	{
		//----------------------------------------------------------------------------------------------------------------------
		//  Client Begin Frame
		//----------------------------------------------------------------------------------------------------------------------
		if ( m_clientState ==  ClientState::READY_TO_CONNECT )
		{
			// Attempt to connect if we haven't already
			sockaddr_in address;
			address.sin_family				= AF_INET;
			address.sin_addr.S_un.S_addr	= htonl( m_hostAddress );
			address.sin_port				= htons( m_hostPort );
			int connectionResult			= connect( m_clientSocket, ( sockaddr* ) ( &address), int( sizeof( address) ) );
			if ( connectionResult == SOCKET_ERROR )
			{
				int errorCode = WSAGetLastError();
				if ( errorCode == WSAEWOULDBLOCK )
				{
					m_clientState = ClientState::CONNECTING;
				}
				else
				{
					m_clientState = ClientState::INVALID;
					ERROR_AND_DIE( "Client failed to connect to the Server!" );
				}
			}
			else
			{
				m_clientState = ClientState::CONNECTING;
			}
		}
		else if ( m_clientState == ClientState::CONNECTING )
		{
			//----------------------------------------------------------------------------------------------------------------------
			// If we failed to connect
			//----------------------------------------------------------------------------------------------------------------------
			// Check if our connection attempt failed
			fd_set failedSockets;
			FD_ZERO( &failedSockets );
			FD_SET( m_clientSocket, &failedSockets );
			timeval failedWaitTime			= {};
			int		failedConnectionResult	= select( 0, NULL, NULL, &failedSockets, &failedWaitTime );
			if ( failedConnectionResult == SOCKET_ERROR )
			{
				m_clientState = ClientState::INVALID;
				ERROR_AND_DIE( "Client connection attempt failed" );
			}

			// if the following is true, the connection failed and we need to connect again
			if ( ( failedConnectionResult > 0 ) && FD_ISSET( m_clientSocket, &failedSockets ) )
			{
				// Our connection attempt failed, retry to connect AGAIN
				m_clientState = ClientState::READY_TO_CONNECT;
				return;
			}

			//----------------------------------------------------------------------------------------------------------------------
			// If our connection attempt succeeds
			//----------------------------------------------------------------------------------------------------------------------
			// Check if our connection attempt completed
			fd_set sockets;
			FD_ZERO( &sockets );
			FD_SET( m_clientSocket, &sockets );
			timeval waitTime					= {};
			int		completedConnectionResult	= select( 0, NULL, &sockets, NULL, &waitTime );
			if ( completedConnectionResult == SOCKET_ERROR )
			{
				ERROR_AND_DIE( "Something went wrong with select" );
			}

			// If the following is true, we are connected
			if ( ( completedConnectionResult > 0 ) && FD_ISSET( m_clientSocket, &sockets ) )
			{
				m_clientState		= ClientState::CONNECTED;
			}
		}
		else if ( m_clientState == ClientState::CONNECTED )
		{
			//----------------------------------------------------------------------------------------------------------------------
			// Receive Result
			//----------------------------------------------------------------------------------------------------------------------
			int receiveResult	= recv( m_clientSocket, m_recvBuffer, m_config.m_recvBufferSize, 0 );
			if ( receiveResult == SOCKET_ERROR )
			{
				// Check if connection broke while receiving data
				int errorCode = WSAGetLastError();
				if ( errorCode == WSAECONNRESET )
				{
					m_clientState = ClientState::READY_TO_CONNECT;
					Shutdown();
					Startup();
					return;
				}
				else if ( errorCode != WSAEWOULDBLOCK )
				{
					DebuggerPrintf("\n--------------------------------------------------------------------------------------------------");
					DebuggerPrintf("\nSomething went wrong while recv the message!!!");
					DebuggerPrintf("\n--------------------------------------------------------------------------------------------------\n");
				}
				// Else, we are in the middle of receiving a message
			}
			else if ( receiveResult == 0 )
			{
				// 0 means connection was broken while nothing is being received, (cleanly broken) 
				m_clientState = ClientState::READY_TO_CONNECT;
				Shutdown();
				Startup();
				return;
			}
			else if ( receiveResult > 0 )
			{
				for ( int i = 0; i < receiveResult; i++ )
				{
					if ( m_recvBuffer[i] != '\0' )
					{
						m_recvRemaining += m_recvBuffer[i];
					}
					else
					{
//						m_recvRemaining = "Echo Message=" + m_recvRemaining;
						g_theDevConsole->Execute( m_recvRemaining );	
						m_recvRemaining.clear();
					}
				}
			}

			//----------------------------------------------------------------------------------------------------------------------
			// Send Result
			//----------------------------------------------------------------------------------------------------------------------
			for ( int i = 0; i < m_sendQueue.size(); i++ )
			{
//				if ( sendBufferSize > m_config.m_sendBufferSize )
//				{
//					ERROR_AND_DIE( "Size of data to send is too big" );
//				}

				strncpy_s( m_sendBuffer, m_config.m_sendBufferSize, m_sendQueue[i].c_str(), strlen( m_sendQueue[i].c_str() ) + 1 );
				int sendBufferSize = (int)strlen( m_sendBuffer ) + 1;
				int sendResult = send( m_clientSocket, m_sendBuffer, sendBufferSize, 0 );
				if ( sendResult == SOCKET_ERROR )
				{
					int errorCode = WSAGetLastError();
					if ( errorCode != WSAEWOULDBLOCK )
					{
						DebuggerPrintf( "\n--------------------------------------------------------------------------------------------------" );
						DebuggerPrintf( "\nSomething Went Wrong While Trying To Send A Message!!!" );
						DebuggerPrintf( "\n--------------------------------------------------------------------------------------------------\n" );
					}
				}
			}
			m_sendQueue.clear();
		}
		else if ( m_clientState == ClientState::INVALID )
		{
		}
	}
	else if ( m_mode == Mode::SERVER )
	{
		switch ( m_serverState )
		{
			case NetSystem::ServerState::INVALID:
			{
				DebuggerPrintf("\n--------------------------------------------------------------------------------------------------");
				DebuggerPrintf("\nInvalid Server State!!!");
				DebuggerPrintf("\n--------------------------------------------------------------------------------------------------\n");
				break;
			}
			case NetSystem::ServerState::LISTENING:
			{
				m_clientSocket = accept( m_listenSocket, NULL, NULL );
				if ( m_clientSocket == INVALID_SOCKET )
				{
					// If an attempt to accept a connection was blocked, then just retry again next frame
					int errorCode = WSAGetLastError();
					if (errorCode == WSAEWOULDBLOCK)
					{
						break;
					}
					else
					{
						DebuggerPrintf("\n--------------------------------------------------------------------------------------------------");
						DebuggerPrintf("\nSomething went wrong while accepting the incoming connection attempt!!!");
						DebuggerPrintf("\n--------------------------------------------------------------------------------------------------\n");
						m_serverState = ServerState::INVALID;
						break;
					}
				}

				DebuggerPrintf("\n--------------------------------------------------------------------------------------------------");
				DebuggerPrintf("\nConnection Accepted!!!");
				DebuggerPrintf("\n--------------------------------------------------------------------------------------------------\n");
				m_serverState				= ServerState::CONNECTED;
				unsigned long blockingMode	= 1;
				int result					= ioctlsocket(m_clientSocket, FIONBIO, &blockingMode);
				if (result == SOCKET_ERROR)
				{
					DebuggerPrintf("\n--------------------------------------------------------------------------------------------------");
					DebuggerPrintf("\nSomething went wrong while setting the blocking mode!!!");
					DebuggerPrintf("\n--------------------------------------------------------------------------------------------------\n");
				}
				break;
			}
			case NetSystem::ServerState::CONNECTED:
			{
				DebuggerPrintf("\n--------------------------------------------------------------------------------------------------");
				DebuggerPrintf("\nConnection Already Exists!!!");
				DebuggerPrintf("\n--------------------------------------------------------------------------------------------------\n");

				//----------------------------------------------------------------------------------------------------------------------
				// Server Receive Data
				//----------------------------------------------------------------------------------------------------------------------
				int result = recv( m_clientSocket, m_recvBuffer, m_config.m_recvBufferSize, 0 );
				if ( result == SOCKET_ERROR )
				{
					int errorCode = WSAGetLastError();
					if ( errorCode == WSAECONNRESET )
					{
						m_serverState = ServerState::LISTENING;
						Shutdown();
						Startup();
						return;
					}
					else if ( errorCode != WSAEWOULDBLOCK )
					{
						DebuggerPrintf("\n--------------------------------------------------------------------------------------------------");
						DebuggerPrintf("\nSomething went wrong while recv the message!!!");
						DebuggerPrintf("\n--------------------------------------------------------------------------------------------------\n");
					}
				}
				else if ( result == 0 )
				{
					m_serverState = ServerState::LISTENING;
					Shutdown();
					Startup();
					return;
				}
				else if ( result > 0 )
				{
					for ( int i = 0; i < result; i++ )
					{
						if ( m_recvBuffer[i] != '\0' )
						{
							m_recvRemaining += m_recvBuffer[i];
						}
						else
						{
							g_theDevConsole->Execute( m_recvRemaining );
//							std::string acknowledgementMessage = "Echo Message=\"Server executed remote command : " + m_recvRemaining + "\"\0";
//							m_sendQueue.push_back( acknowledgementMessage );
							m_recvRemaining.clear();
						}
					}
				}

/*
				//----------------------------------------------------------------------------------------------------------------------
				// Server Send Data
				//----------------------------------------------------------------------------------------------------------------------
				for ( int i = 0; i < m_sendQueue.size(); i++ )
				{
					// Check if sendBufferSize is big enough.
					//		True:  sendBuffer fits in the sendBufferSize, send data normally.
					//		False: sendBuffer is too big, break the data apart and send in a loop until theres nothing left to send.
					// m_config.m_sendBufferSize is maxSendBufferSize
					// sendBufferSize is current sendBufferSize

					int currentSendBufferSize	= (int)strlen( m_sendBuffer ) + 1;
					if ( currentSendBufferSize > m_config.m_sendBufferSize )
					{
						// sendBuffer is too big, break the data apart and send in a loop until theres nothing left to send.
						for ( int i = 0; i < currentSendBufferSize; i++ )
						{
							// Take pieces of the sendBuffer and add it into a temp array
							// When the arraySize equals to the sendBufferSize, send the data
							// Then repeat the loop and start rebuilding again until we run out of data
							char* tempSendBuffer = nullptr;
							int count = 0;
							while ( true )
							{

							}

						}
							// Send Data
							int sendResult		= send( m_clientSocket, m_sendBuffer, currentSendBufferSize, 0 );
							if ( sendResult == SOCKET_ERROR )
							{
								int errorCode = WSAGetLastError();
								if ( errorCode != WSAEWOULDBLOCK )
								{
									DebuggerPrintf( "\n--------------------------------------------------------------------------------------------------" );
									DebuggerPrintf( "\nSomething Went Wrong While Trying To Send A Message!!!" );
									DebuggerPrintf( "\n--------------------------------------------------------------------------------------------------\n" );
								}
							}
					}
					else
					{
						// sendBuffer fits in the sendBufferSize, send data normally.
						strncpy_s( m_sendBuffer, m_config.m_sendBufferSize, m_sendQueue[i].c_str(), strlen( m_sendQueue[i].c_str() ) + 1 );
						int sendResult		= send( m_clientSocket, m_sendBuffer, currentSendBufferSize, 0 );
						if ( sendResult == SOCKET_ERROR )
						{
							int errorCode = WSAGetLastError();
							if ( errorCode != WSAEWOULDBLOCK )
							{
								DebuggerPrintf( "\n--------------------------------------------------------------------------------------------------" );
								DebuggerPrintf( "\nSomething Went Wrong While Trying To Send A Message!!!" );
								DebuggerPrintf( "\n--------------------------------------------------------------------------------------------------\n" );
							}
						}
					}
				}
				m_sendQueue.clear();
				break;
*/

/*
*/
				//----------------------------------------------------------------------------------------------------------------------
				// Server Send Data
				//----------------------------------------------------------------------------------------------------------------------
				for ( int i = 0; i < m_sendQueue.size(); i++ )
				{
//					if ( sendBufferSize > m_config.m_sendBufferSize )
//					{
//						ERROR_AND_DIE( "Size of data to send is too big" );
//					}

					strncpy_s( m_sendBuffer, m_config.m_sendBufferSize, m_sendQueue[i].c_str(), strlen( m_sendQueue[i].c_str() ) + 1 );
					int sendBufferSize	= (int)strlen( m_sendBuffer ) + 1;
					int sendResult		= send( m_clientSocket, m_sendBuffer, sendBufferSize, 0 );
					if ( sendResult == SOCKET_ERROR )
					{
						int errorCode = WSAGetLastError();
						if ( errorCode != WSAEWOULDBLOCK )
						{
							DebuggerPrintf( "\n--------------------------------------------------------------------------------------------------" );
							DebuggerPrintf( "\nSomething Went Wrong While Trying To Send A Message!!!" );
							DebuggerPrintf( "\n--------------------------------------------------------------------------------------------------\n" );
						}
					}
				}
				m_sendQueue.clear();
				break;
			}
			default:
			{
				DebuggerPrintf("\n--------------------------------------------------------------------------------------------------");
				DebuggerPrintf("\nSomething Went Terribly Wrong in Server Begin Frame!!!");
				DebuggerPrintf("\n--------------------------------------------------------------------------------------------------\n");
				break;
			}
		}
	}
	else if ( m_mode == Mode::NONE )
	{
	}
}


//----------------------------------------------------------------------------------------------------------------------
void NetSystem::EndFrame()
{
}


//----------------------------------------------------------------------------------------------------------------------
bool NetSystem::IsNetworking()
{
	if ( ( m_mode == Mode::CLIENT ) || ( m_mode == Mode::SERVER ) )
	{
		return true;
	}
	return false;
}


//----------------------------------------------------------------------------------------------------------------------
bool NetSystem::IsConnected()
{
	if ( m_mode == Mode::CLIENT )
	{
		if ( m_clientState == ClientState::CONNECTED )
		{
			return true;
		}
	}
	else if ( m_mode == Mode::SERVER )
	{
		if ( m_serverState == ServerState::CONNECTED )
		{
			return true;
		}
	}
	return false;
}


//----------------------------------------------------------------------------------------------------------------------
std::string NetSystem::GetModeEnumAsString()
{
	std::string currentMode = "INVALID";
	if ( m_mode == Mode::CLIENT )
	{
		currentMode = "Client";
	}
	else if ( m_mode == Mode::SERVER )
	{
		currentMode = "Server";
	}
	else
	{
		currentMode = "None";
	}
	return currentMode;
}


//----------------------------------------------------------------------------------------------------------------------
bool NetSystem::Event_RemoteCommand( EventArgs& args )
{
	// Parses an argument called command and sends the quoted contents to the server
	std::string commandAndMessage = args.GetValue( "RemoteCommand", "Remote Command Incorrect" );
	if ( commandAndMessage == "" )
	{
		g_theDevConsole->AddLine( Rgba8::RED, "Invalid RemoteCommandMessage" );
		return false;
	}

	Strings commandMessage = SplitStringWithQuotes( commandAndMessage, '=' );
	if ( commandMessage.size() == 2 )
	{
		if ( commandMessage[0] == "Command" )
		{
			TrimString( commandMessage[1], '\"' );
			commandAndMessage = commandMessage[1];
		}
	}	
	g_theNetSystem->m_sendQueue.push_back( commandAndMessage );
	return true;
}

//----------------------------------------------------------------------------------------------------------------------
bool NetSystem::Event_BurstTest( EventArgs& args )
{
	// Can be executed on the client or server
	// Sends 20 messages in one frame, instructing the remote machine to echo the numbers 1 to 20 in order
	// Dev console must scaled and sized so that it can show all the output from this command, on both machines
	for ( int i = 1; i <= 20; i++ )
	{
		g_theNetSystem->m_sendQueue.emplace_back( Stringf( "Echo Message=%d", i ) );
	}
	return true;
}


//----------------------------------------------------------------------------------------------------------------------
bool NetSystem::SendMessage( std::string const& stringToSend )
{
	g_theNetSystem->m_sendQueue.push_back( stringToSend );
	return true;
}
