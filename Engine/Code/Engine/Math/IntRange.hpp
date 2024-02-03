#pragma once

//----------------------------------------------------------------------------------------------------------------------
class IntRange
{
public:
	IntRange();
	IntRange( int min, int max );
	~IntRange();

private:
	int m_min;
	int m_max;
};
