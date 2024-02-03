#include "Engine/Core/Clock.hpp"
#include "Engine/Core/Time.hpp"

//----------------------------------------------------------------------------------------------------------------------
static Clock g_theSystemClock;
 
//----------------------------------------------------------------------------------------------------------------------
Clock::Clock()
{
	if ( &g_theSystemClock == this )
	{
		m_parent = nullptr;
		return;
	}
	m_parent = &g_theSystemClock;
	g_theSystemClock.AddChild( this );
}

//----------------------------------------------------------------------------------------------------------------------
Clock::Clock( Clock& parent )
{
	m_parent = &parent;
	parent.AddChild( this );
}

//----------------------------------------------------------------------------------------------------------------------
Clock::~Clock()
{
	if ( m_parent == nullptr )
	{
		return;
	}

	for ( int i = 0; i < m_children.size(); i++ )
	{
		m_children[i]->m_parent = nullptr;
	}

	m_parent->RemoveChild( this );
}

//----------------------------------------------------------------------------------------------------------------------
void Clock::Reset()
{
	m_lastUpdateTimeInSeconds	= 0.0f;
	m_totalSeconds				= 0.0f;
	m_deltaSeconds				= 0.0f;
	m_frameCount				= 0;

	// get current time as last updated time
	float currentTime			= static_cast<float>( GetCurrentTimeSeconds() );
	m_lastUpdateTimeInSeconds	= currentTime;
}

//----------------------------------------------------------------------------------------------------------------------
bool Clock::IsPaused() const
{ 
	return m_isPaused;
}

//----------------------------------------------------------------------------------------------------------------------
void Clock::Pause()
{
	m_isPaused = true;
}

//----------------------------------------------------------------------------------------------------------------------
void Clock::Unpause()
{
	m_isPaused = false;
}

//----------------------------------------------------------------------------------------------------------------------
void Clock::TogglePause()
{
	m_isPaused = !m_isPaused;
}

//----------------------------------------------------------------------------------------------------------------------
void Clock::StepSingleFrame()
{
	m_stepSingleFrame = true;
	Unpause();
}

//----------------------------------------------------------------------------------------------------------------------
void Clock::SetTimeScale( float timeScale )
{
	m_timeScale = timeScale;
}

//----------------------------------------------------------------------------------------------------------------------
float Clock::GetTimeScale() const
{
	return m_timeScale;
}

//----------------------------------------------------------------------------------------------------------------------
float Clock::GetDeltaSeconds() const
{
	return m_deltaSeconds;
}

//----------------------------------------------------------------------------------------------------------------------
float Clock::GetTotalSeconds() const
{
	return m_totalSeconds;
}

//----------------------------------------------------------------------------------------------------------------------
size_t Clock::GetFrameCount() const
{
	return m_frameCount;
}

//----------------------------------------------------------------------------------------------------------------------
Clock& Clock::GetSystemClock()
{	
	return g_theSystemClock;
}

//----------------------------------------------------------------------------------------------------------------------
void Clock::TickSystemClock()
{
	g_theSystemClock.Tick(); 
}

//----------------------------------------------------------------------------------------------------------------------
void Clock::Tick()
{
	// Calculate current delta seconds
	float currentTime	= static_cast<float>( GetCurrentTimeSeconds() ); 
	float deltaSeconds	= ( currentTime - m_lastUpdateTimeInSeconds );

	if ( deltaSeconds > m_maxDeltaSeconds )
	{
		deltaSeconds = m_maxDeltaSeconds;
	}

	Advance( deltaSeconds );
	m_lastUpdateTimeInSeconds = currentTime;
}

//----------------------------------------------------------------------------------------------------------------------
void Clock::Advance( float deltaTimeSeconds )
{
	if ( IsPaused() )
	{
		m_deltaSeconds = 0;
		for ( int i = 0; i < m_children.size(); i++ )
		{
			m_children[i]->Advance( m_deltaSeconds );
		}
	}
	else
	{
		m_deltaSeconds				= m_timeScale * deltaTimeSeconds;
		m_lastUpdateTimeInSeconds	= static_cast<float>( GetCurrentTimeSeconds() );
		m_totalSeconds				= m_totalSeconds + m_deltaSeconds;
		m_frameCount++;
	
		for ( int i = 0; i < m_children.size(); i++ )
		{
			m_children[i]->Advance( m_deltaSeconds );
		}
	}

	if ( m_stepSingleFrame )
	{
		Pause();
		m_stepSingleFrame = false;
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Clock::AddChild( Clock* childClock )
{
	m_children.push_back( childClock );
}

//----------------------------------------------------------------------------------------------------------------------
void Clock::RemoveChild( Clock* childClock )
{
	// if child is inside m_children vector, remove childClock, else do nth
	for ( int i = 0; i < m_children.size(); i++ )
	{
		if ( m_children[i] == childClock )
		{
			m_children[i] = m_children[(m_children.size() - 1)];
			m_children.pop_back();
		}
	}

}
