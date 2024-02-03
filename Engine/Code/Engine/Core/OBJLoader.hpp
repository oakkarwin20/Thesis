#pragma once

#include "Engine/Math/Mat44.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Core/StringUtils.hpp"

#include <string>
#include <vector>

//----------------------------------------------------------------------------------------------------------------------
class OBJLoader
{
public:
	OBJLoader();
	~OBJLoader();

	//----------------------------------------------------------------------------------------------------------------------
	// Tenshi Version
	//----------------------------------------------------------------------------------------------------------------------
	static void LoadOBJByFileName(const char* filename, Mat44 transformMat, std::vector<Vertex_PCUTBN>& outVertexes, std::vector<unsigned int>& outIndexes);

	//----------------------------------------------------------------------------------------------------------------------
	// Oak Version
	//----------------------------------------------------------------------------------------------------------------------
	static void LoadObjFile( std::string const& fileName, std::vector<Vertex_PCUTBN>& outVerts, std::vector<unsigned int>& outIndex, Mat44 transform );
	static void CalculateAndPopulateNormalsList( std::vector<Vec3>& normalsList, std::vector<Vec3> positionList );
/*
*/

public:

};