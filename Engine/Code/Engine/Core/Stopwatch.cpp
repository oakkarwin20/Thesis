#include "Engine/Core/Stopwatch.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/Time.hpp"

//----------------------------------------------------------------------------------------------------------------------
Stopwatch::Stopwatch( float duration )
{
	// #CheckIfCorrect
	m_duration	= duration;
	m_clock		= &m_clock->GetSystemClock();
}

//----------------------------------------------------------------------------------------------------------------------
Stopwatch::Stopwatch( Clock const* clock, float duration )
{
	// #CheckIfCorrect
	m_clock		= clock;
	m_duration	= duration;
}

//----------------------------------------------------------------------------------------------------------------------
Stopwatch::Stopwatch()
{ 
}

//----------------------------------------------------------------------------------------------------------------------
void Stopwatch::Start()
{
	m_startTime = m_clock->GetTotalSeconds();
}

//----------------------------------------------------------------------------------------------------------------------
void Stopwatch::Restart()
{
	if ( !IsStopped() )
	{
		Start();
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Stopwatch::Stop()
{
	m_startTime = 0.0f;
}

//----------------------------------------------------------------------------------------------------------------------
float Stopwatch::GetElapsedTime() const
{
	if ( IsStopped() )
	{
		return 0.0f;
	}

	float  currentTimeInSeconds  = static_cast<float>( m_clock->GetTotalSeconds() );
	float  elapsedTime			 = currentTimeInSeconds - m_startTime;
	return elapsedTime;
}

//----------------------------------------------------------------------------------------------------------------------
float Stopwatch::GetElapsedFraction() const
{
	if ( m_duration == 0.0f )
	{
		return 0.0f;
	}

	float	elapsedTime		= GetElapsedTime();
	float	elapsedFraction	= elapsedTime / m_duration;			// calculates how many durations have passed
	return	elapsedFraction;
}

//----------------------------------------------------------------------------------------------------------------------
bool Stopwatch::IsStopped() const
{
	return m_startTime == 0.0f;

	//----------------------------------------------------------------------------------------------------------------------
	// Code above is the same as below;
//	if ( m_startTime == 0.0f )
//	{
//		return true;
//	}
//
//	return false;
}

//----------------------------------------------------------------------------------------------------------------------
bool Stopwatch::HasDurationElapsed() const
{
	float elapsedTime = GetElapsedTime();
	return ( elapsedTime > m_duration ) && ( !IsStopped() );

	//----------------------------------------------------------------------------------------------------------------------
	// Code above is the same as below;
//	float elapsedTime = GetElapsedTime();
//	if ( elapsedTime > m_duration && !IsStopped() )
//	{
//		return true;
//	}
//
//	return false;
}

//----------------------------------------------------------------------------------------------------------------------
// Function has to be called inside a while or for loop in game code
bool Stopwatch::DecrementDurationIfElapsed()
{
	bool durationHasElapsed = HasDurationElapsed();
	if ( durationHasElapsed && !IsStopped() )
	{
		m_startTime += m_duration;
		return true;
	}
	return false;
}
