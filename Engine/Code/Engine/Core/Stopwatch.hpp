#pragma once

//----------------------------------------------------------------------------------------------------------------------
class Clock;

//----------------------------------------------------------------------------------------------------------------------
class Stopwatch
{
public:
	explicit Stopwatch( float duration );
	Stopwatch( Clock const* clock, float duration );
	Stopwatch(); 

	void	Start();
	void	Restart();
	void	Stop();
	
	float	GetElapsedTime()				const;
	float	GetElapsedFraction()			const;
	bool	IsStopped()						const;
	bool	HasDurationElapsed()			const;
	bool	DecrementDurationIfElapsed();

public:
	Clock const* 	m_clock			= nullptr;
	float			m_startTime		= 0.0f;
	float			m_duration		= 0.0f;
};