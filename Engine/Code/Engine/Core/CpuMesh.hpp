#pragma once

#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"

#include <vector>

//----------------------------------------------------------------------------------------------------------------------
class CpuMesh
{
public:
/*
	i.	Contains a vector of vertexes and indexes that could be used to render immediately or create vertex and index buffers.
	ii.	Populate this with the results of the obj file loader.
*/

public:
	std::vector<Vertex_PCUTBN>	m_vertexes;
	std::vector<unsigned int>	m_indexes;
};