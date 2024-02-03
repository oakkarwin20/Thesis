#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/Stopwatch.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/Renderer.hpp"

DevConsole* g_theDevConsole = nullptr;

//----------------------------------------------------------------------------------------------------------------------
DevConsole::DevConsole( DevConsoleConfig const& config )
{
	m_config		 = config;
	m_caretStopwatch = new Stopwatch( 0.5f );
}

//----------------------------------------------------------------------------------------------------------------------
DevConsole::~DevConsole()
{
}

//----------------------------------------------------------------------------------------------------------------------
void DevConsole::Startup()
{
	g_theEventSystem->SubscribeToEvent(    "KeyPressed", DevConsole::Event_KeyPressed		);
	g_theEventSystem->SubscribeToEvent(	    "CharInput", DevConsole::Event_CharInput		);
	g_theEventSystem->SubscribeToEvent(	  	    "clear", DevConsole::Command_Clear			);
	g_theEventSystem->SubscribeToEvent(	  	     "help", DevConsole::Command_Help			);	
	g_theEventSystem->SubscribeToEvent(			 "Echo", DevConsole::Command_Echo			);	
//	g_theEventSystem->SubscribeToEvent(	"RemoteCommand", DevConsole::Command_RemoteCommand	);	
//	g_theEventSystem->SubscribeToEvent(		"BurstTest", DevConsole::Command_BurstTest		);	

	//----------------------------------------------------------------------------------------------------------------------
	// Test code for rendering texts
	AddLine( Rgba8::DARK_GRAY, "Type help for a list of commands" );

	// Start stopwatch
	m_caretStopwatch->Start();
}
 
//----------------------------------------------------------------------------------------------------------------------
void DevConsole::Shutdown()
{
}

//----------------------------------------------------------------------------------------------------------------------
void DevConsole::BeginFrame()
{
	if ( !m_isOpen )
	{
		return;
	}
	
	// start caret startTime since Tick in App is called in Begin frame instead of App::startup
	if ( m_caretStopwatch->IsStopped() )
	{
		m_caretStopwatch->Start();
	}

	// If caret stopwatch has been visible for > 0.5f sec, toggle visibility
	if ( m_caretStopwatch->DecrementDurationIfElapsed() )
	{
		m_caretIsVisible = !m_caretIsVisible;
	}
}

//----------------------------------------------------------------------------------------------------------------------
void DevConsole::EndFrame()
{
}

//----------------------------------------------------------------------------------------------------------------------
void DevConsole::Execute( std::string const& consoleCommandText )
{
	EventArgs args;

	//----------------------------------------------------------------------------------------------------------------------
	// Delimit string on 'space'
	Strings delimitedStringOnSpace		= SplitStringOnDelimiter( consoleCommandText, ' ' );
	Strings delimitedStringWithQuotes	= SplitStringWithQuotes(  consoleCommandText, ' ' );
	if ( delimitedStringWithQuotes[0] == "RemoteCommand" )
	{
		delimitedStringOnSpace = delimitedStringWithQuotes;
	}
	if ( delimitedStringWithQuotes[0] == "Server" )
	{
		// Print the command
		AddLine( Rgba8::MAGENTA, consoleCommandText );
		return;
	}
	if ( delimitedStringWithQuotes[0] == "Echo" )
	{
		delimitedStringOnSpace = delimitedStringWithQuotes;
	}


	//----------------------------------------------------------------------------------------------------------------------
	// If string != registered commands, output error message
	std::vector<std::string> vectorOfStrings;
	g_theEventSystem->GetNameOfRegisteredCommands( vectorOfStrings );		// Get all the names of events aka registered commands, then add them into "vectorOfStrings"

	// Go through list of names and check if delimitedString aka "current input text" == any name of registered events
	if ( std::find( vectorOfStrings.begin(), vectorOfStrings.end(), delimitedStringOnSpace[0] ) == vectorOfStrings.end() )
	{
		// If search was not found in the list, input text != any event name. Then, print appropriate message.
		std::string tempString = "Unknown Command: " + consoleCommandText;
		AddLine( Rgba8::RED, tempString );
		return;
	}
	// If key value pair exists after the eventName aka the "delimitedString", split the strings again, but check for '=' this time
	if ( delimitedStringOnSpace.size() == 2 )
	{
		if ( delimitedStringWithQuotes[0] == "RemoteCommand" )
		{
			args.SetValue( delimitedStringWithQuotes[0], delimitedStringWithQuotes[1] );
		} 
		else
		{
			for ( int j = 1; j < delimitedStringOnSpace.size(); j++ )
			{
				Strings delimitedSpaceOnEqualSign	= SplitStringOnDelimiter( delimitedStringOnSpace[j], '=' );
				if ( delimitedSpaceOnEqualSign.size() == 2 )
				{
					args.SetValue( delimitedSpaceOnEqualSign[0], delimitedSpaceOnEqualSign[1] );
				}
				if ( delimitedSpaceOnEqualSign.size() == 3 )
				{
					std::string message = delimitedSpaceOnEqualSign[1] + delimitedSpaceOnEqualSign[2];
					args.SetValue( delimitedSpaceOnEqualSign[0], message );
				}
			}
		}
	}
	FireEvent( delimitedStringOnSpace[0].c_str(), args );
	if ( delimitedStringOnSpace[0] == "Echo" )
	{
		// Do nothing, don't print the command
	}
	else if ( delimitedStringOnSpace[0] == "RemoteCommand" )
	{
		// Do nothing, don't print the command
		AddLine( Rgba8::MAGENTA, consoleCommandText );
	}
	else
	{
		// Print the command
		AddLine( Rgba8::MAGENTA, consoleCommandText );
	}
	
/*
	//----------------------------------------------------------------------------------------------------------------------
	// Delimit string on 'space'
	Strings delimitedStringOnSpace;
	delimitedStringOnSpace = SplitStringOnDelimiter( consoleCommandText, ' ' );
	EventArgs args;

	//----------------------------------------------------------------------------------------------------------------------
	// If string != registered commands, output error message
	std::vector<std::string> vectorOfStrings;
	g_theEventSystem->GetNameOfRegisteredCommands( vectorOfStrings );		// Get all the names of events aka registered commands, then add them into "vectorOfStrings"

	// Go through list of names and check if delimitedString aka "current input text" == any name of registered events
	if ( std::find(vectorOfStrings.begin(), vectorOfStrings.end(), delimitedStringOnSpace[0]) == vectorOfStrings.end() )
	{
		// If search was not found in the list, input text != any event name. Then, print appropriate message.
		std::string tempString = "Unknown Command: " + consoleCommandText;
		AddLine( Rgba8::RED, tempString );
		return;
	}

	// If key value pair exists after the eventName aka the "delimitedString", split the strings again, but check for '=' this time
	if ( delimitedStringOnSpace.size() == 2 )
	{
		for ( int j = 1; j < delimitedStringOnSpace.size(); j++ )		
		{
			Strings delimitedSpaceOnEqualSign;
			delimitedSpaceOnEqualSign = SplitStringOnDelimiter( delimitedStringOnSpace[j], '=' );
			if ( delimitedSpaceOnEqualSign.size() == 2 )
			{
				args.SetValue( delimitedSpaceOnEqualSign[0], delimitedSpaceOnEqualSign[1] );
			}
		}
	}
	FireEvent( delimitedStringOnSpace[0].c_str(), args );
	if ( delimitedStringOnSpace[0] == "Echo" )
	{
		// Do nothing, don't print the command
	}
	else
	{
		// Print the command
		AddLine( Rgba8::MAGENTA, consoleCommandText );
	}
*/
}

//----------------------------------------------------------------------------------------------------------------------
void DevConsole::AddLine( Rgba8 const& color, std::string const& text )
{
	m_devConsoleMutex.lock();
	DevConsoleLine devConsoleLine;
	devConsoleLine.m_color	= color;
	devConsoleLine.m_text	= text;

	m_lines.push_back( devConsoleLine );
	m_devConsoleMutex.unlock();
}

//----------------------------------------------------------------------------------------------------------------------
void DevConsole::Render( AABB2 const& bounds )
{
	if ( !m_isOpen )
	{
		return;
	}

	m_devConsoleMutex.lock();

	//----------------------------------------------------------------------------------------------------------------------
	// Begin DevConsole Camera
	m_config.m_renderer->BeginCamera( *m_config.m_camera );

	//----------------------------------------------------------------------------------------------------------------------
	// Draw background in DevConsole Camera Space
	std::vector<Vertex_PCU> verts;
	AddVertsForAABB2D( verts, bounds, Rgba8::TRANSLUCENT_BLACK );

	m_config.m_renderer->SetBlendMode(BlendMode::ALPHA);
	m_config.m_renderer->SetModelConstants();
	m_config.m_renderer->BindTexture( nullptr );
	m_config.m_renderer->BindShader( nullptr );
	m_config.m_renderer->DrawVertexArray( static_cast<int>( verts.size() ), verts.data() );

	//----------------------------------------------------------------------------------------------------------------------
	// Get or Create font
	BitmapFont* bitmapFont = nullptr;
	bitmapFont = m_config.m_renderer->CreateOrGetBitmapFontFromFile( std::string("Data/Fonts/" + m_config.m_fontName).c_str()  );
	
	AABB2 textBounds;
	textBounds.m_mins.x	= bounds.m_mins.x;
	textBounds.m_maxs.x	= bounds.m_maxs.x;

	//----------------------------------------------------------------------------------------------------------------------
	// Draw previous text
	std::vector<Vertex_PCU> stringVerts;
	for ( int i = static_cast<int>( (m_lines.size() - 1) ); i >= 0; i-- )
	{
		// Calculate bounds for previous text box
		textBounds.m_mins.y += m_config.m_cellHeight;
		textBounds.m_maxs.y = textBounds.m_mins.y + m_config.m_cellHeight;

		Rgba8		 tempColor	= m_lines[i].m_color;
		std::string& tempText	= m_lines[i].m_text;
		bitmapFont->AddVertsForTextInBox2D( stringVerts, textBounds, m_config.m_cellHeight, tempText, tempColor, m_config.m_fontAspect, Vec2( 0.0f, 0.0f ) );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Calculate bounds for current text being typed
	AABB2 inputBounds = AABB2( bounds.m_mins, Vec2( bounds.m_maxs.x, (bounds.m_mins.y + m_config.m_cellHeight) ) );
	bitmapFont->AddVertsForTextInBox2D( stringVerts, inputBounds, m_config.m_cellHeight, m_inputText, Rgba8::WHITE, m_config.m_fontAspect, Vec2::ZERO );

	//----------------------------------------------------------------------------------------------------------------------
	// Calculate pos & bounds Caret 
	std::vector<Vertex_PCU> caretVerts;
	float tempCaretPos		= m_caretPosition * m_config.m_cellWidth;
	AABB2 caretBounds		= AABB2( bounds.m_mins.x, bounds.m_mins.y, m_config.m_cellWidth * 0.1f, bounds.m_mins.y + m_config.m_cellHeight );
	caretBounds.SetCenter( Vec2( tempCaretPos, m_config.m_cellHeight * 0.5f ) );
	
	// Clamp caret to bounds.max.x
	if ( caretBounds.m_mins.x > bounds.m_maxs.x )
	{
		caretBounds.m_mins.x = bounds.m_maxs.x - (m_config.m_cellWidth * 0.1f);
	}

	// If Caret isVisible, then draw Caret 
	if ( m_caretIsVisible )
	{
		// Draw Caret
		AddVertsForAABB2D( caretVerts, caretBounds, Rgba8::WHITE );

		m_config.m_renderer->SetBlendMode( BlendMode::ALPHA );
		m_config.m_renderer->SetModelConstants();
		m_config.m_renderer->BindTexture( nullptr );
		m_config.m_renderer->BindShader( nullptr );
		m_config.m_renderer->DrawVertexArray( static_cast<int>( caretVerts.size() ), caretVerts.data() );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Bind textures and draw text
	m_config.m_renderer->SetBlendMode( BlendMode::ALPHA );
	m_config.m_renderer->SetModelConstants();
	m_config.m_renderer->BindShader( nullptr );
	m_config.m_renderer->BindTexture( &bitmapFont->GetTexture() );
	m_config.m_renderer->DrawVertexArray( static_cast<int>( stringVerts.size() ), stringVerts.data() );
	
	// Unbinding texture
	m_config.m_renderer->BindTexture( nullptr );

	//----------------------------------------------------------------------------------------------------------------------
	// Begin DevConsole Camera
	m_config.m_renderer->EndCamera( *m_config.m_camera );

	m_devConsoleMutex.unlock();
}

//----------------------------------------------------------------------------------------------------------------------
void DevConsole::ToggleOpen()
{
	m_isOpen = !m_isOpen;
}

//----------------------------------------------------------------------------------------------------------------------
bool DevConsole::Event_KeyPressed( EventArgs& args )
{
	// Get ascii value of 'KeyPressed'
	unsigned char keyCode = static_cast<unsigned char>( args.GetValue( ("KeyCode"), -1 ) );

	// Toggle devConsole if keyPressed == 'Tilde'
	if ( keyCode == KEYCODE_TILDE )
	{
		g_theDevConsole->ToggleOpen();
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Check if DevConsole is closed
	if ( !g_theDevConsole->m_isOpen )
	{
		return false;
	}

	// Reset caret blinking every time, inputText is typed
	g_theDevConsole->m_caretIsVisible = true;
	g_theDevConsole->m_caretStopwatch->Restart();

	// Call Execute() if keyPressed == 'Enter' 
	if ( keyCode == KEYCODE_ENTER )
	{
		// If m_inputText.size() == 0, close devConsole
		if ( g_theDevConsole->m_inputText.size() == 0 )
		{
			g_theDevConsole->m_isOpen = false;
			return true;
		}
		
		g_theDevConsole->Execute( g_theDevConsole->m_inputText );
		g_theDevConsole->m_inputText.clear();
		g_theDevConsole->m_caretPosition = 0;
	}

	// Quit DevConsole or clear line if keyPressed == 'Escape'
	if ( keyCode == KEYCODE_ESC )
	{
		// if current line is empty, 
		if ( g_theDevConsole->m_inputText.size() == 0 )
		{
			g_theDevConsole->m_isOpen = false;
		}
		// Clear input line
		g_theDevConsole->m_inputText.clear();
		// Reset Caret pos to start
		g_theDevConsole->m_caretPosition = 0;
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Backspace and Delete 
	if ( keyCode == KEYCODE_BACKSPACE )
	{
		// delete char left of caret
		if ( g_theDevConsole->m_caretPosition == 0 )
		{
			return true;
		}
	
		g_theDevConsole->m_inputText.erase( g_theDevConsole->m_caretPosition - 1, 1 );
		g_theDevConsole->m_caretPosition--;
		return true;
	}
	if ( keyCode == KEYCODE_DELETE )
	{
		// delete char right of caret
		g_theDevConsole->m_inputText.erase( g_theDevConsole->m_caretPosition, 1 );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Move caret movement left
	if ( keyCode == KEYCODE_LEFTARROW )
	{
		g_theDevConsole->m_caretPosition--;
		if ( g_theDevConsole->m_caretPosition < 0 )
		{
			g_theDevConsole->m_caretPosition = 0;
		}
	}
	// Move caret movement right
	if ( keyCode == KEYCODE_RIGHTARROW )
	{
		g_theDevConsole->m_caretPosition++;
		if ( g_theDevConsole->m_caretPosition > g_theDevConsole->m_inputText.size() )
		{
			g_theDevConsole->m_caretPosition = static_cast<int>( g_theDevConsole->m_inputText.size() );
		}
	}
	// Reset caret pos
	if ( keyCode == KEYCODE_HOME )
	{
		g_theDevConsole->m_caretPosition = 0;
	}
	if ( keyCode == KEYCODE_END )
	{
		g_theDevConsole->m_caretPosition = static_cast<int>( g_theDevConsole->m_inputText.size() );
	}

	return true;
}

//----------------------------------------------------------------------------------------------------------------------
bool DevConsole::Event_CharInput( EventArgs& args )
{
	// Check if DevConsole is closed
	if ( !g_theDevConsole->m_isOpen )
	{
		return false;
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Get Ascii values of keys being typed
	unsigned char keyCode = static_cast<unsigned char>( args.GetValue( "KeyCode", -1 ) );

	//----------------------------------------------------------------------------------------------------------------------
	// Keycode validity test
	if ( keyCode >= 32 && keyCode <= 126 && (keyCode != KEYCODE_TILDE && keyCode != '`') )
	{
		// Add keyCode(s) in into current string at current pos 
		g_theDevConsole->m_inputText.insert( g_theDevConsole->m_caretPosition, 1, keyCode );   

		// Move caret pos to the right every time we type
		g_theDevConsole->m_caretPosition++;

		return true;
	}

	return false;
}

//----------------------------------------------------------------------------------------------------------------------
bool DevConsole::Command_Clear( EventArgs& args )
{
	UNUSED( args ); 
	g_theDevConsole->m_lines.clear();
	return true;
}

//----------------------------------------------------------------------------------------------------------------------
bool DevConsole::Command_Help( EventArgs& args )
{
	UNUSED(args);

	std::vector<std::string> eventNames;
	g_theEventSystem->GetNameOfRegisteredCommands( eventNames );

	for ( int i = 0; i < eventNames.size(); i++ )
	{
		g_theDevConsole->AddLine( Rgba8::CYAN, eventNames[i] );
	}
	return true;
}

//----------------------------------------------------------------------------------------------------------------------
bool DevConsole::Command_Echo( EventArgs& args )
{
	//----------------------------------------------------------------------------------------------------------------------
	// Delimit string on 'space'
//	Strings delimitedStringOnSpace;
//	delimitedStringOnSpace = SplitStringOnDelimiter( consoleCommandText, ' ' );

	// Parse the entire string
	// Delimit on '='
	// Add line with just the messageRemo
	 
	std::string message = args.GetValue( "Message", "Echo Command Incorrect" );
	TrimString( message, '\"' );
	g_theDevConsole->AddLine( Rgba8::WHITE, message );
	return false;
}