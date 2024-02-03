#pragma once

#include <string>
#include <vector>

//-----------------------------------------------------------------------------------------------
const std::string Stringf( char const* format, ... );
const std::string Stringf( int maxLength, char const* format, ... );

typedef std::vector<std::string> Strings;

Strings SplitStringOnDelimiter(std::string const& originalString, std::string delimiterToSplitOn);
Strings SplitStringOnDelimiter( std::string const& originalString, char delimiterToSplitOn );
void	TrimString( std::string& stringToTrim, char charToTrimOn );
Strings SplitStringWithQuotes( std::string const & originalString, char delimiterToSplitOn);