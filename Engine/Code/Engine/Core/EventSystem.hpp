#pragma once

#include "Engine/Core/NamedStrings.hpp"

#include <vector>
#include <string>
#include <map>
#include <mutex>

//----------------------------------------------------------------------------------------------------------------------
typedef NamedStrings EventArgs;
typedef bool (*EventCallbackFuncPtr)( EventArgs& eventArgs );
typedef std::vector<EventCallbackFuncPtr> EventSubscriberList;

// reference for learning purposes
// typedef std::vector<bool(*)(EventArgs&)> EventSubscriberList;
// std::function<EventArgs&e>
//----------------------------------------------------------------------------------------------------------------------
class EventSystem
{
public:
//	EventSystem( EventSystemConfig const& config );
	EventSystem();
	~EventSystem();
	void Startup();
	void Shutdown();
	void BeginFrame();
	void EndFrame();

	void SubscribeToEvent	 ( std::string const& eventName, EventCallbackFuncPtr functionPtr );
	void UnsubscribeFromEvent( std::string const& eventName, EventCallbackFuncPtr functionPtr );
	void UnsubscribeFromAllEvents( EventCallbackFuncPtr callbackFunc );
	void FireEvent( std::string const& eventName, EventArgs& args );
	void FireEvent( std::string const& eventName );

	void GetNameOfRegisteredCommands( std::vector<std::string>& out_eventNames );

	std::mutex m_eventSystemMutex;

private:
//	EventSystemConfig								m_config;
	std::map< std::string, EventSubscriberList >	m_subscribersForEventNames;
};

//----------------------------------------------------------------------------------------------------------------------
// Standalone global namespace help functions; these foward to "the" event system if it exists
void SubscribeToEvent	 ( std::string const& eventName, EventCallbackFuncPtr functionPtr );
void UnsubscribeFromEvent( std::string const& eventName, EventCallbackFuncPtr functionPtr );
void UnsubscribeFromAllEvents( EventCallbackFuncPtr callbackFunc );
void FireEvent( std::string const& eventName, EventArgs& args );
void FireEvent( std::string const& eventName );