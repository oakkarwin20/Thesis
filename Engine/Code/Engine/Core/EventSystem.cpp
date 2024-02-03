#include "Engine/Core/EventSystem.hpp"

EventSystem* g_theEventSystem = nullptr;

//----------------------------------------------------------------------------------------------------------------------
EventSystem::EventSystem()
{
}
//----------------------------------------------------------------------------------------------------------------------
EventSystem::~EventSystem()
{
}
//----------------------------------------------------------------------------------------------------------------------
void EventSystem::Startup()
{
}
//----------------------------------------------------------------------------------------------------------------------
void EventSystem::Shutdown()
{
}
//----------------------------------------------------------------------------------------------------------------------
void EventSystem::BeginFrame()
{
}
//----------------------------------------------------------------------------------------------------------------------
void EventSystem::EndFrame()
{
}

//----------------------------------------------------------------------------------------------------------------------
void EventSystem::SubscribeToEvent( std::string const& eventName, EventCallbackFuncPtr functionPtr )
{
	m_eventSystemMutex.lock();
	EventSubscriberList& subscribersForThisEvent = m_subscribersForEventNames[ eventName ];
	subscribersForThisEvent.push_back( functionPtr );
	m_eventSystemMutex.unlock();
}

//----------------------------------------------------------------------------------------------------------------------
void EventSystem::UnsubscribeFromEvent( std::string const& eventName, EventCallbackFuncPtr functionPtr )
{
	m_eventSystemMutex.lock();
	EventSubscriberList& subscribersForThisEvent = m_subscribersForEventNames[eventName];
	for ( int i = 0; i < static_cast<int>( subscribersForThisEvent.size() ); ++i )
	{
		if ( subscribersForThisEvent[i] == functionPtr )
		{
			subscribersForThisEvent[i] = nullptr;
		}
	}
	m_eventSystemMutex.unlock();
}

//----------------------------------------------------------------------------------------------------------------------
void EventSystem::UnsubscribeFromAllEvents( EventCallbackFuncPtr callbackFunc )
{
	m_eventSystemMutex.lock();
	for ( auto eventIter = m_subscribersForEventNames.begin(); eventIter != m_subscribersForEventNames.end(); ++ eventIter)
	{
		std::string const& eventName = eventIter->first;
		m_eventSystemMutex.unlock();
		UnsubscribeFromEvent( eventName, callbackFunc );
		m_eventSystemMutex.lock();
	}
	m_eventSystemMutex.unlock();
}

//----------------------------------------------------------------------------------------------------------------------
void EventSystem::FireEvent( std::string const& eventName, EventArgs& args )
{
	m_eventSystemMutex.lock();
	EventSubscriberList& subscribersForThisEvent = m_subscribersForEventNames[ eventName ];
	for ( int i = 0; i < subscribersForThisEvent.size(); ++i )
	{
		EventCallbackFuncPtr callbackFuncPtr = subscribersForThisEvent[i];
		if ( callbackFuncPtr != nullptr )
		{
			m_eventSystemMutex.unlock();
			bool wasConsumed = callbackFuncPtr( args );		// Call the subscriber's callback function
			m_eventSystemMutex.lock();
			if ( wasConsumed )
			{
				break;		// Event was consumed by this subscriber; Don't tell remaining subscribers about the event firing
			}
		}
	}
	m_eventSystemMutex.unlock();
}

//----------------------------------------------------------------------------------------------------------------------
void EventSystem::FireEvent( std::string const& eventName )
{
	EventArgs emptyArgs;
	FireEvent( eventName, emptyArgs );
}

//----------------------------------------------------------------------------------------------------------------------
void EventSystem::GetNameOfRegisteredCommands( std::vector<std::string>& out_eventNames )
{
	m_eventSystemMutex.lock();
	for ( auto eventIter = m_subscribersForEventNames.begin(); eventIter != m_subscribersForEventNames.end(); ++eventIter )
	{
		std::string const& eventString = eventIter->first;
		out_eventNames.push_back( eventString );
	}
	m_eventSystemMutex.unlock();
}

//----------------------------------------------------------------------------------------------------------------------
void SubscribeToEvent( std::string const& eventName, EventCallbackFuncPtr functionPtr )
{
	g_theEventSystem->SubscribeToEvent( eventName, functionPtr );
}

//----------------------------------------------------------------------------------------------------------------------
void UnsubscribeFromEvent( std::string const& eventName, EventCallbackFuncPtr functionPtr )
{
	g_theEventSystem->UnsubscribeFromEvent( eventName, functionPtr );
}

//----------------------------------------------------------------------------------------------------------------------
void UnsubscribeFromAllEvents( EventCallbackFuncPtr callbackFunc )
{
	g_theEventSystem->UnsubscribeFromAllEvents( callbackFunc );
}

//----------------------------------------------------------------------------------------------------------------------
void FireEvent( std::string const& eventName, EventArgs& args )
{
	g_theEventSystem->FireEvent( eventName, args);
}

//----------------------------------------------------------------------------------------------------------------------
void FireEvent( std::string const& eventName )
{
	g_theEventSystem->FireEvent( eventName );
}
