#include "Engine/Core/GpuMesh.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/Renderer//Renderer.hpp"

extern Renderer* g_theRenderer;

//----------------------------------------------------------------------------------------------------------------------
GpuMesh::GpuMesh()
{
	m_vertexBuffer = g_theRenderer->CreateVertexBuffer( sizeof(Vertex_PCUTBN), sizeof(Vertex_PCUTBN) );
	m_indexBuffer  = g_theRenderer->CreateIndexBuffer( sizeof(Vertex_PCUTBN) );
}

//----------------------------------------------------------------------------------------------------------------------
GpuMesh::~GpuMesh()
{
	delete m_indexBuffer;
	delete m_vertexBuffer;
	m_vertexBuffer	= nullptr;
	m_indexBuffer	= nullptr;
}

//----------------------------------------------------------------------------------------------------------------------
void GpuMesh::Render() const
{
	g_theRenderer->DrawVertexAndIndexBuffer( m_vertexBuffer, m_indexBuffer, m_indexCount );
}

//----------------------------------------------------------------------------------------------------------------------
void GpuMesh::CopyCpuToGpu( CpuMesh* cpuMesh )
{
	g_theRenderer->Copy_CPU_To_GPU( cpuMesh->m_vertexes.data(), (size_t)m_vertexBuffer->GetStride() * cpuMesh->m_vertexes.size(), m_vertexBuffer, m_vertexBuffer->GetStride() );
	g_theRenderer->Copy_CPU_To_GPU( cpuMesh->m_indexes.data(),  (size_t)m_indexBuffer->GetStride()  * cpuMesh->m_indexes.size(), m_indexBuffer );
	m_indexCount  = int(  cpuMesh->m_indexes.size() );
	m_vertexCount = int( cpuMesh->m_vertexes.size() );
}
