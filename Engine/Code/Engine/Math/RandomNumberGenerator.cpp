#include "Engine/Math/RandomNumberGenerator.hpp"
#include <stdlib.h>

#include "Engine/ThirdParty/Squirrel/Noise/RawNoise.hpp"
#include "Engine/Math/FloatRange.hpp"

//----------------------------------------------------------------------------------------------------------------------
int RandomNumberGenerator::RollRandomIntLessThan(int maxNotInclusive)
{
//	return rand() % maxNotInclusive;

	unsigned int randUInt = Get1dNoiseUint( m_position++, m_seed );
	return randUInt % maxNotInclusive;
}
 
//----------------------------------------------------------------------------------------------------------------------
int RandomNumberGenerator::RollRandomIntInRange(int minInclusive, int maxInclusive)
{
	unsigned int randUInt = Get1dNoiseUint( m_position++, m_seed );
	return randUInt % (maxInclusive - minInclusive + 1) + minInclusive;

//	return rand() % (maxInclusive - minInclusive + 1) + minInclusive;
}

//----------------------------------------------------------------------------------------------------------------------
float RandomNumberGenerator::RollRandomFloatZeroToOne()
{
	// This code is the same as the noise code below
//	return Get1dNoiseZeroToOne( m_position++, m_seed );

	// New code using noise
	unsigned int randUInt						= Get1dNoiseUint( m_position++, m_seed );
	constexpr unsigned int MAX_RANDOM_UINT		= 0xffff'ffff;		// 0xFFFFFFFF = largest unsigned int number value
	constexpr double ONE_OVER_MAX_RANDOM_UINT	= 1.0 / double(MAX_RANDOM_UINT);	
	return static_cast<float>( double(randUInt) * ONE_OVER_MAX_RANDOM_UINT );

	// Old code using rand
//	return (float) rand() / (float)RAND_MAX;
}

//----------------------------------------------------------------------------------------------------------------------
float RandomNumberGenerator::RollRandomFloatInRange(float minInclusive, float maxInclusive)
{
//	unsigned int randUInt = Get1dNoiseUint( m_position++, m_seed );
//	return randUInt * ( maxInclusive - minInclusive ) / (float)RAND_MAX + minInclusive;

	float randomZeroToOne	= RollRandomFloatZeroToOne();					// Range [0,1]
	float range				= maxInclusive - minInclusive;					// 37 Max - 33 Min = 4 Range
	float randomValue		= minInclusive + ( randomZeroToOne * range );	// 33 min + ( [0,1] * 4 ) = 33 + [0,4] = [33,37]
	return randomValue;

//	return rand() * ( maxInclusive - minInclusive ) / (float)RAND_MAX + minInclusive;
}

//----------------------------------------------------------------------------------------------------------------------
bool RandomNumberGenerator::RollRandomChance(float chanceToReturnTrue)
{
	float randomFloat = RollRandomFloatZeroToOne();	//#ToDo fix logic 
	
	return (randomFloat < chanceToReturnTrue);
}
