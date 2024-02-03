#pragma once

#include "Engine/Core/EngineCommon.hpp"

#include <string>
#include <vector>

int FileReadToBuffer( std::vector<uint8_t>& outBuffer, const std::string& fileName );
int FileReadToString(		   std::string& outString, const std::string& fileName );

void WriteBinaryBufferToFile( std::vector<char>& outBuffer, const std::string& fileName );
void ReadFileToBinaryBuffer( std::vector<char>& outBuffer, const std::string& fileName );

bool DoesFileExist( std::string const& filePath );