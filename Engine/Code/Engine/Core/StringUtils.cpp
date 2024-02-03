#include "Engine/Core/StringUtils.hpp"
#include <stdarg.h>

//-----------------------------------------------------------------------------------------------
constexpr int STRINGF_STACK_LOCAL_TEMP_LENGTH = 2048;

//-----------------------------------------------------------------------------------------------
const std::string Stringf( char const* format, ... )
{
	char textLiteral[ STRINGF_STACK_LOCAL_TEMP_LENGTH ];
	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, STRINGF_STACK_LOCAL_TEMP_LENGTH, _TRUNCATE, format, variableArgumentList );	
	va_end( variableArgumentList );
	textLiteral[ STRINGF_STACK_LOCAL_TEMP_LENGTH - 1 ] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	return std::string( textLiteral );
}

//-----------------------------------------------------------------------------------------------
const std::string Stringf( int maxLength, char const* format, ... )
{
	char textLiteralSmall[ STRINGF_STACK_LOCAL_TEMP_LENGTH ];
	char* textLiteral = textLiteralSmall;
	if( maxLength > STRINGF_STACK_LOCAL_TEMP_LENGTH )
		textLiteral = new char[ maxLength ];

	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, maxLength, _TRUNCATE, format, variableArgumentList );	
	va_end( variableArgumentList );
	textLiteral[ maxLength - 1 ] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	std::string returnValue( textLiteral );
	if( maxLength > STRINGF_STACK_LOCAL_TEMP_LENGTH )
		delete[] textLiteral;

	return returnValue;
}


//----------------------------------------------------------------------------------------------------------------------
Strings SplitStringOnDelimiter( std::string const& originalString, std::string delimiterToSplitOn )
{
	size_t pos_start = 0;
	size_t pos_end;
	size_t delim_len = delimiterToSplitOn.length();
	std::string token;
	Strings result;
	while ( ( pos_end = originalString.find( delimiterToSplitOn, pos_start ) ) != std::string::npos )
	{
		token = originalString.substr( pos_start, pos_end - pos_start );
		pos_start = pos_end + delim_len;
		result.push_back( token );
	}
	result.push_back( originalString.substr( pos_start ) );
	return result;
}


//----------------------------------------------------------------------------------------------------------------------
Strings SplitStringOnDelimiter( std::string const& originalString, char delimiterToSplitOn )
{
	Strings result;

	size_t startPos = 0;
	size_t delimiterPos = originalString.find_first_of( delimiterToSplitOn, startPos );

	while ( delimiterPos != std::string::npos )
	{
		std::string substring = std::string( originalString, startPos, delimiterPos - startPos );
		result.push_back( substring );
		startPos = delimiterPos + 1;
		delimiterPos = originalString.find_first_of( delimiterToSplitOn, startPos );
	}

	std::string substring = std::string( originalString, startPos );
	result.push_back( substring );
	return result;
}

//----------------------------------------------------------------------------------------------------------------------
void TrimString( std::string& stringToTrim, char charToTrimOn )
{
	// 1. For loop start to end, if current element is == 'quotes'
	//		1a. Create substring on "quotes" from begin to current index
	//		1b. Create substring on "quotes" from currentIndex + 1 to end
	//		1c. Combine the two substrings
	//		1d. Break the for-loop
	// 2. For loop end to start, if current element is == 'quotes'
	//		2a. Create substring on "quotes" from begin to currentIndex
	//		2b. Create substring on "quotes" from currentIndex + 1 to end
	//		2c. Combine both substrings
	//		2d. Break the for loop

	// Loop start to end
	int stringSize = int( stringToTrim.size() );
	for ( int i = 0; i < stringSize; i++ )
	{
		if ( stringToTrim[i] == charToTrimOn )
		{
			// If index is at the end of the list, no need to recombine the strings. "firstHalfOfString" is the entire string
			if ( i == ( stringToTrim.size()  - 1 ) )
			{
				std::string firstHalfOfString	= std::string( stringToTrim, 0, i ); 
				stringToTrim					= firstHalfOfString;
				break;
			}
			else
			{
				char a = stringToTrim[0];
				char b = stringToTrim[i];
				std::string firstHalfOfString	= std::string( stringToTrim,	   0,	i ); 
				std::string SecondHalfOfString	= std::string( stringToTrim, (i + 1), ( stringSize - 1 ) ); 
				stringToTrim					= firstHalfOfString + SecondHalfOfString;
				break;
			}
		}
	}

	// Loop end to start 
	stringSize = int( stringToTrim.size() );
	for ( int i = ( stringSize - 1 ); i >= 0; i-- )
	{
		if ( stringToTrim[i] == charToTrimOn )
		{
			// If index is at the end of the list, no need to recombine the strings. "firstHalfOfString" is the entire string
			if ( i == 0 )
			{
				std::string firstHalfOfString	= std::string( stringToTrim, 0, i );
				stringToTrim					= firstHalfOfString;
				break;
			}
			else
			{
				std::string firstHalfOfString	= std::string( stringToTrim,	   0, i );
				std::string SecondHalfOfString	= std::string( stringToTrim, (i + 1), ( stringSize - 1 )  );
				stringToTrim					= firstHalfOfString + SecondHalfOfString;
				break;
			}
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
Strings SplitStringWithQuotes( std::string const& originalString, char delimiterToSplitOn )
{
	// Loop through the string
	// Toggle bool on/off when encountering "quotes"
	// Delimit on string
	// Pushback into stringList
	// Move Delimiter to new pos (currentIndex + 1)
	// Keep looping
	// If end of loop has been reached, split string from currentDelimiterPos till stringEnd
	// Combine all strings and return 

	Strings strings;
	int  delimiterCurrentPos	 = 0;
	bool isCurrentlyInsideQuotes = false;
	for ( int i = 0; i < originalString.size(); i++ )
	{
		if ( originalString[i] == '\"' )
		{
			isCurrentlyInsideQuotes = !isCurrentlyInsideQuotes;
		}

		if ( originalString[i] == delimiterToSplitOn )
		{
			if ( isCurrentlyInsideQuotes )
			{
				continue;
			}

			std::string currentSubString = std::string( originalString, delimiterCurrentPos, (i - delimiterCurrentPos) );
			strings.emplace_back( currentSubString );
			delimiterCurrentPos = i + 1;
		}
	}

	// Splits from currentPos till stringEnd
	std::string currentSubString = std::string( originalString, delimiterCurrentPos );
	strings.emplace_back( currentSubString );
	return strings;
}
