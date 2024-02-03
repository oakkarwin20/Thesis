#pragma once

#include <vector>
#include <Engine/Core/CpuMesh.hpp>

//----------------------------------------------------------------------------------------------------------------------
class VertexBuffer;
class IndexBuffer;

//----------------------------------------------------------------------------------------------------------------------
class GpuMesh
{
public:
	GpuMesh();
	~GpuMesh();

	void Render() const;
	void CopyCpuToGpu( CpuMesh* cpuMesh );

/*
	i.	 Contains a vertex buffer and index buffer.
	ii.	 Populate this with the results of the CPUMesh.
	iii. Renders the buffers but does not set any state. 
*/

public:
	VertexBuffer*  m_vertexBuffer	 = nullptr;
	IndexBuffer*   m_indexBuffer	 = nullptr;
	int			   m_indexCount		 = 0;
	int			   m_vertexCount	 = 0;
};