#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

#include <fstream>

//----------------------------------------------------------------------------------------------------------------------
// Think of function name as "Copy file contents to Buffer"
int FileReadToBuffer( std::vector<uint8_t>& outBuffer, const std::string& fileName )
{
	size_t		numElementsRead = 0;
	FILE*		ptrToFileStream;
	errno_t		errorMessage;

	// Specifies which file I want to read by fileName, and to read as Binary
	errorMessage = fopen_s( &ptrToFileStream, fileName.c_str(), "rb" );
	if ( errorMessage )
	{
//		ERROR_AND_DIE( "Could not open specified file " + errorMessage );
		return false;
	}
	
	if ( ptrToFileStream != nullptr )
	{
		size_t elementCount;
		
		fseek( ptrToFileStream, 0, SEEK_END );				// Specifies a ptrToStartOfFile, offset, endOfFile.	// Move pointer to end of file.
		elementCount	= ftell( ptrToFileStream );			// Calculate distance traveled.						// figures out how many elements there are.
		fseek( ptrToFileStream, 0, SEEK_SET );				// Reset back to start.
		outBuffer.resize( elementCount );					
		numElementsRead	= fread( outBuffer.data(), sizeof(uint8_t), elementCount, ptrToFileStream );			// Copy data from ptrToFileStream to outBuffer.data()  // Also specifiy stride via size and numCount  
		if ( numElementsRead != elementCount )
		{
			ERROR_AND_DIE("Unable to copy contents from file to buffer " + fileName );
		}
		fclose( ptrToFileStream );
	}

	return static_cast<int>( numElementsRead );
}

//----------------------------------------------------------------------------------------------------------------------
// Think of function name as (Copy file contents to outString)
int FileReadToString( std::string& outString, const std::string& fileName )
{
	std::vector<uint8_t> outBuffer;
	int numElementsReadInFile = FileReadToBuffer( outBuffer, fileName );
	if ( numElementsReadInFile <= 0 )
	{
		return 0;
	}
	outBuffer.push_back('\0');
	outString.resize( outBuffer.size() );

	memcpy( outString.data(), outBuffer.data(), outBuffer.size() );

	if ( outString.size() != numElementsReadInFile + 1 )				// +1 to compensate for '\0'	// Marking end of string 
	{
		ERROR_AND_DIE("Unable to copy contents from file to string " + fileName );
	}

	return static_cast<int>( outString.size() );						// return numElements in string or file
}

//----------------------------------------------------------------------------------------------------------------------
void WriteBinaryBufferToFile( std::vector<char>& outBuffer, const std::string& fileName )
{
	std::ofstream outputFile( fileName, std::ios::binary | std::ios::out );
	outputFile.write( outBuffer.data(), outBuffer.size() );
	outputFile.close();
}

//----------------------------------------------------------------------------------------------------------------------
void ReadFileToBinaryBuffer( std::vector<char>& outBuffer, const std::string& fileName )
{
	std::ifstream inputFile( fileName, std::ios::binary | std::ios::in );

	inputFile.seekg( 0, inputFile.end );			// Highlight from file start to file end	// Move caret from start to end
	int length = int( inputFile.tellg() );			// Count totalNumChar highlighted in file
	inputFile.seekg( 0, inputFile.beg );			// Move caret back to file start

	inputFile.read( outBuffer.data(), length );
	inputFile.close();
}


//--------------------------------------------------------------------------------------------------
bool DoesFileExist( std::string const& filePath )
{
	FILE* fileInfoPtr = nullptr;
	errno_t fOpenErr = fopen_s( &fileInfoPtr, filePath.c_str(), "rb" );
	if ( fOpenErr )
	{
		return false;
	}
	fclose( fileInfoPtr );
	return true;
}