#include "Engine/Window/Window.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

#define WIN32_LEAN_AND_MEAN		// always #define this before #including <windows.h>
#include <windows.h>
#include <commdlg.h>


//----------------------------------------------------------------------------------------------------------------------
// Global and class static variables
Window*		Window::s_mainWindow	= nullptr;
HWND		m_windowHandle			= nullptr;
HDC			m_displayContext		= nullptr;

//-----------------------------------------------------------------------------------------------
// Handles Windows (Win32) messages/events; i.e. the OS is trying to tell us something happened.
// This function is called by Windows whenever we ask it for notifications
//
// #SD1ToDo: We will move this function to a more appropriate place later on...
//
LRESULT CALLBACK WindowsMessageHandlingProcedure(HWND windowHandle, UINT wmMessageCode, WPARAM wParam, LPARAM lParam)
{
	Window* window = Window::GetWindow();
	GUARANTEE_OR_DIE( window != nullptr, "Window::GetWindow() returned null" );

	InputSystem* input = window->GetConfig().m_inputSystem;
	GUARANTEE_OR_DIE( input != nullptr, "Window::GetConfig().m_inputSystem was null!" );
	 
	switch (wmMessageCode)
	{
		// App close requested via "X" button, or right-click "Close Window" on task bar, or "Close" from system menu, or Alt-F4
		case WM_CLOSE:
		{
			FireEvent( "quit" );
			return 0; // "Consumes" this message (tells Windows "okay, we handled it")
		}

		// Raw physical keyboard "key-was-just-depressed" event (case-insensitive, not translated)
		case WM_KEYDOWN:
		{			
			EventArgs args;
			args.SetValue( "KeyCode", Stringf( "%d", (unsigned char)wParam ) );
		 	FireEvent( "KeyPressed", args );
			return 0;
		}

		// Raw physical keyboard "key-was-just-released" event (case-insensitive, not translated)
		case WM_KEYUP:
		{
			EventArgs args;
			args.SetValue( "KeyCode", Stringf( "%d", (unsigned char)wParam ) );
			FireEvent( "KeyReleased", args );
			return 0;
		}

		case WM_LBUTTONDOWN:
		{
			unsigned char keyCode = (unsigned char)KEYCODE_LEFT_MOUSE;
			if ( input )
			{
				input->HandleKeyPressed( keyCode );
				return 0;
			}
			break;
		}

		case WM_LBUTTONUP:
		{
			unsigned char keyCode = KEYCODE_LEFT_MOUSE;
			if (input)
			{
				input->HandleKeyReleased(keyCode);
				return 0;
			}
			break;
		}
		case WM_RBUTTONUP:
		{
			unsigned char keyCode = KEYCODE_RIGHT_MOUSE;
			if (input)
			{
				input->HandleKeyReleased(keyCode);
				return 0;
			}
			break;
		}

		case WM_RBUTTONDOWN:
		{
			unsigned char keyCode = (unsigned char)KEYCODE_RIGHT_MOUSE;
			if ( input )
			{
				input->HandleKeyPressed( keyCode );
				return 0;
			}
			break;
		}

		case WM_CHAR:
		{
			EventArgs args;
			args.SetValue( "KeyCode", Stringf( "%d", (unsigned char)wParam ) );
			FireEvent( "CharInput", args );
			return 0;
		}
	}
		// Send back to Windows any unhandled/unconsumed messages we want other apps to see (e.g. play/pause in music apps, etc.)
		return DefWindowProc(windowHandle, wmMessageCode, wParam, lParam);
}
 
//----------------------------------------------------------------------------------------------------------------------
Window::Window( WindowConfig const& config )
	: m_config( config )
{
	s_mainWindow = this;
}

//----------------------------------------------------------------------------------------------------------------------
Window::~Window()
{
}

//----------------------------------------------------------------------------------------------------------------------
void Window::Startup()
{
	//CreateOSWindow(applicationInstanceHandle );	// #SD1ToDo: this will move to Window.cpp
	CreateOSWindow();
}

//----------------------------------------------------------------------------------------------------------------------
void Window::Shutdown()
{
}

//----------------------------------------------------------------------------------------------------------------------
void Window::BeginFrame()
{
	// Process OS messages (keyboard/mouse button clicked, application lost/gained focus, etc.)
	RunMessagePump(); // calls our own WindowsMessageHandlingProcedure() function for us!

}

//----------------------------------------------------------------------------------------------------------------------
void Window::EndFrame()
{
}


//----------------------------------------------------------------------------------------------------------------------
//Vec2 Window::GetCursorNormalizedPos() const
//{
//	return Vec2( 0.5f, 0.5f );
//}


//void Window::CreateOSWindow(void* applicationInstanceHandle, float clientAspect)
void Window::CreateOSWindow()
{
	float clientAspect = m_config.m_clientAspect;
	if ( ( !m_config.m_isFullScreen ) && ( m_config.m_windowSize != IntVec2( -1, -1 ) ) )
	{
		// Update the aspectRation based on the "windowSize" X (width) and Y (height)
		// Width / Height will give us the ACTUAL ratio we want to use, instead of the "clientAspect" variable
		clientAspect = float( m_config.m_windowSize.x ) / float( m_config.m_windowSize.y );
	}

	// Define a window style/class
	WNDCLASSEX windowClassDescription;
	memset(&windowClassDescription, 0, sizeof(windowClassDescription));
	windowClassDescription.cbSize			= sizeof(windowClassDescription);
	windowClassDescription.style			= CS_OWNDC; // Redraw on move, request own Display Context
	windowClassDescription.lpfnWndProc		= static_cast<WNDPROC>(WindowsMessageHandlingProcedure); // Register our Windows message-handling function
	windowClassDescription.hInstance		= GetModuleHandle(NULL);
	windowClassDescription.hIcon			= NULL;
	windowClassDescription.hCursor			= NULL;
	windowClassDescription.lpszClassName	= TEXT("Simple Window Class");
	RegisterClassEx(&windowClassDescription);

	// #SD1ToDo: Add support for fullscreen mode (requires different window style flags than windowed mode)
	DWORD windowStyleFlags	 = WS_CAPTION | WS_BORDER | WS_THICKFRAME | WS_SYSMENU | WS_OVERLAPPED;
	DWORD windowStyleExFlags = WS_EX_APPWINDOW;

	// Get desktop rect, dimensions, aspect
	RECT desktopRect;
	HWND desktopWindowHandle = GetDesktopWindow();
	GetClientRect(desktopWindowHandle, &desktopRect);
	float desktopWidth		 = (float)( desktopRect.right  - desktopRect.left );
	float desktopHeight		 = (float)( desktopRect.bottom - desktopRect.top  );
	float desktopAspect		 = desktopWidth / desktopHeight;

	float clientWidth  = 0.0f;
	float clientHeight = 0.0f;
	if ( !m_config.m_isFullScreen )
	{
		windowStyleFlags = WS_CAPTION | WS_BORDER | WS_THICKFRAME | WS_SYSMENU | WS_OVERLAPPED;

		// Only scale the client to be "90%" of the desktop if a "windowSize" is NOT specified
		if ( m_config.m_windowSize == IntVec2( -1, -1 ) )
		{
			// Calculate maximum client size (as some % of desktop size)
			constexpr float maxClientFractionOfDesktop	= 0.90f;
			clientWidth									= desktopWidth  * maxClientFractionOfDesktop;
			clientHeight								= desktopHeight * maxClientFractionOfDesktop;
		}
		else
		{
			// Set the client width and height according to the "windowSize" specified in the gameConfig
			clientWidth  = float( m_config.m_windowSize.x );
			clientHeight = float( m_config.m_windowSize.y );
		}


		if (clientAspect > desktopAspect)
		{
			// Client window has a wider aspect than desktop; shrink client height to match its width
			clientHeight = clientWidth / clientAspect;
		}
		else
		{
			// Client window has a taller aspect than desktop; shrink client width to match its height
			clientWidth = clientHeight * clientAspect;
		}
	}
	else	// If Fullscreen
	{
		windowStyleFlags	= WS_POPUP;
		clientWidth			= desktopWidth;
		clientHeight		= desktopHeight;
	}


	// Calculate client rect bounds by centering the client area
	float clientMarginX = 0.5f * ( desktopWidth  - clientWidth  );
	float clientMarginY = 0.5f * ( desktopHeight - clientHeight );
	RECT clientRect;
	if ( m_config.m_windowPosition == IntVec2( -1, -1 ) )
	{
		// Set the clientPos to "screen center" if the "windowPosition" was NOT specified
		clientRect.left		= (int)clientMarginX;
		clientRect.right	= clientRect.left + (int)clientWidth;
		clientRect.top		= (int)clientMarginY;
		clientRect.bottom	= clientRect.top + (int)clientHeight;
	}
	else
	{
		// Use the "windowPosition" if it WAS specified
		clientRect.left		= m_config.m_windowPosition.x;
		clientRect.right	= clientRect.left + int( clientWidth );
		clientRect.top		= m_config.m_windowPosition.y;
		clientRect.bottom	= clientRect.top + int( clientHeight );
	}

	// Calculate the outer dimensions of the physical window, including frame et. al.
	RECT windowRect = clientRect;
	AdjustWindowRectEx(&windowRect, windowStyleFlags, FALSE, windowStyleExFlags);
	 
	WCHAR windowTitle[1024];
	MultiByteToWideChar(GetACP(), 0, m_config.m_windowTitle.c_str(), -1, windowTitle, sizeof(windowTitle) / sizeof(windowTitle[0]));
	m_windowHandle = CreateWindowEx(
		windowStyleExFlags,
		windowClassDescription.lpszClassName,
		windowTitle,
		windowStyleFlags,
		windowRect.left,
		windowRect.top,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		NULL,
		NULL,
		GetModuleHandle(NULL),
		NULL);

	ShowWindow(m_windowHandle, SW_SHOW);
	SetForegroundWindow(m_windowHandle);
	SetFocus(m_windowHandle);

	m_displayContext = GetDC( m_windowHandle );

	HCURSOR cursor = LoadCursor(NULL, IDC_ARROW);
	SetCursor(cursor);

	m_clientDimensions = IntVec2( static_cast<int>( clientWidth ), static_cast<int>( clientHeight ) );
	m_hwnd = m_windowHandle;
}

//-----------------------------------------------------------------------------------------------
// Processes all Windows messages (WM_xxx) for this app that have queued up since last frame.
// For each message in the queue, our WindowsMessageHandlingProcedure (or "WinProc") function
//	is called, telling us what happened (key up/down, minimized/restored, gained/lost focus, etc.)
void Window::RunMessagePump()
{
	MSG queuedMessage;
	for (;; )
	{
		const BOOL wasMessagePresent = PeekMessage(&queuedMessage, NULL, 0, 0, PM_REMOVE);
		if (!wasMessagePresent)
		{
			break;
		}

		TranslateMessage(&queuedMessage);
		DispatchMessage(&queuedMessage); // This tells Windows to call our "WindowsMessageHandlingProcedure" (a.k.a. "WinProc") function
	}
}

//----------------------------------------------------------------------------------------------------------------------
void* Window::GetHwnd() const
{
	return m_hwnd;
}

//----------------------------------------------------------------------------------------------------------------------
IntVec2 Window::GetClientDimensions() const
{
	return m_clientDimensions;
}

//----------------------------------------------------------------------------------------------------------------------
Vec2 Window::GetNormalizedCursorPos() const
{
	HWND  windowHandle = HWND( m_hwnd );
	POINT cursorCoords;
	RECT  clientRect;
	::GetCursorPos( & cursorCoords );										// In screen coordinates, (0,0) top-left
	::ScreenToClient( windowHandle, &cursorCoords );						// relative to the window interior
	::GetClientRect( windowHandle, &clientRect );							// size of window interior (0,0 to width, height)
	float cursorX = float( cursorCoords.x ) / float( clientRect.right  );	// normalized x position
	float cursorY = float( cursorCoords.y ) / float( clientRect.bottom );	// normalized y position
	return Vec2( cursorX, 1.0f - cursorY );									// We want (0,0) in the bottom left
}

//----------------------------------------------------------------------------------------------------------------------
bool Window::HasFocus()
{
	return m_hwnd == ::GetActiveWindow();
}

//----------------------------------------------------------------------------------------------------------------------
std::string Window::GetXMLFileName( std::string const& directoryPath )
{
	// SetCurrentDirectoryA(directoryPath.c_str());
	constexpr int maxfileNameLength				= 256;
	char filePath[MAX_PATH] = {};
	OPENFILENAMEA infoAboutUserFileSelection	=	{};
	infoAboutUserFileSelection.lStructSize		=	sizeof(OPENFILENAMEA);
	infoAboutUserFileSelection.lpstrFile		=	filePath;
	infoAboutUserFileSelection.nMaxFile			=	maxfileNameLength;
	infoAboutUserFileSelection.lpstrInitialDir  =	directoryPath.c_str();
	infoAboutUserFileSelection.lpstrDefExt		=	"xml";
	infoAboutUserFileSelection.lpstrFilter		=	"XML Files\0*.xml\0\0";

	char currentDirectory[MAX_PATH] = {};
	DWORD status = GetCurrentDirectoryA(MAX_PATH, currentDirectory);
	if (status == 0)
	{
		return "INVALID STATUS";
	}
	BOOL returnInfo = GetOpenFileNameA(&infoAboutUserFileSelection);
	if (!returnInfo)
	{
		return "INVALID RETURN INFO";
	}
	SetCurrentDirectoryA(currentDirectory);

	Strings			textAfterSplittingOnBackslash	= SplitStringOnDelimiter(filePath, '\\');
	size_t			listLength						= textAfterSplittingOnBackslash.size();
	std::string 	fileName						= textAfterSplittingOnBackslash[listLength - 1];
	return fileName;
}