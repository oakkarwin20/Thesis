#pragma once

#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/OBB2D.hpp"
#include "Engine/Math/OBB3D.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"

#include <vector> 


//----------------------------------------------------------------------------------------------------------------------
void TransformVertexArrayXY3D( int numVertz, Vertex_PCU* verts, float uniformScaleXY, float rotationDegreesAboutZ, Vec2 const& translationXY );
void TransformVertexArray3D( std::vector<Vertex_PCU>& verts, Mat44 const& transform );
void TransformVertexArray3D( int numVerts, Vertex_PCU* verts, Mat44 const& transform );
void TransformVertexArray3D( std::vector<Vertex_PCUTBN>& verts, Mat44 const& transform );


//----------------------------------------------------------------------------------------------------------------------
// 2D shapes
//----------------------------------------------------------------------------------------------------------------------
void	AddVertsForAABB2D( std::vector<Vertex_PCU>& verts, AABB2 const& bounds, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2( Vec2( 0.0f, 0.0f), Vec2( 1.0f, 1.0f ) ) );
void	AddVertsForBordersAABB2D( std::vector<Vertex_PCU>& verts, AABB2 const& bounds, float thickness = 1.0f, Rgba8 const& color = Rgba8::WHITE );
void	AddVertsForLineSegment2D( std::vector<Vertex_PCU>& verts, Vec2 startPos, Vec2 endPos, float thickness, Rgba8 color );
// void AddVertsForLineSegment2D( std::vector<Vertex_PCU>& verts, LineSegment2D const& lineSegment, float thickness, Rgba8 color );		// Have to create LineSegment class
void	AddVertsForLineSegment3D( std::vector<Vertex_PCU>& verts, Vec3 startPos, Vec3 endPos, float thickness, Rgba8 color );
void	AddVertsForDisc2D_UV( std::vector<Vertex_PCU>& verts, Vec2 centerPos, float radius, Rgba8 color, AABB2 const& UV = AABB2::ZERO_TO_ONE );
void	AddVertsForDisc2D( std::vector<Vertex_PCU>& verts, Vec2 centerPos, float radius, Rgba8 color );
void	AddVertsForRing2D( std::vector<Vertex_PCU>& verts, Vec2 centerPos, float radius, float thickness, Rgba8 color );
void	AddVertsForCapsule2D( std::vector<Vertex_PCU>& verts, Vec2 const& boneStart, Vec2 const& boneEnd, float radius, Rgba8 const& color );
void	AddVertsForOBB2D( std::vector<Vertex_PCU>& verts, OBB2D const& box, Rgba8 const& color );
void	AddVertsForOBB3D( std::vector<Vertex_PCU>& verts, OBB3D const& box, Rgba8 const& color );
void	AddVertsForInfiniteLine( std::vector<Vertex_PCU>& verts, Vec2 start, Vec2 end, float thickness, Rgba8 color );
void	AddVertsForArrow2D( std::vector<Vertex_PCU>& verts, Vec2 tailPos, Vec2 tipPos, float arrowSize, float lineThickness, Rgba8 color );
void	AddVertsForHexagon2D( std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexList, Vec2 centerPos, float radius, Rgba8 color );
void	AddVertsForHexagon2D( std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexList, Vec2 centerPos, float radius, Rgba8 color );
void	AddVertsForHexagon2D( std::vector<Vertex_PCU>& verts, Vec2 centerPos, float radius, Rgba8 color );
void	AddVertsForHexagonBorders2D( std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexList, Vec2 centerPos, float thickness, Rgba8 color );
void	AddVertsForHexagonBorders2D( std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexList, Vec2 centerPos, float thickness, Rgba8 color );
void	AddVertsForHexagonBorders2D( std::vector<Vertex_PCU>& verts, Vec2 centerPos, float thickness, Rgba8 color );
void	AddVertsForQuad2D(  std::vector<Vertex_PCU>& verts, Vec2 const& bottomLeft, Vec2 const& bottomRight, Vec2 const& topRight, Vec2 const& topLeft, Rgba8 tint, Vec2 UV_bottomLeft, Vec2 UV_bottomRight, Vec2 UV_TopRight, Vec2 UV_TopLeft ); 
void	AddVertsForQuad2D(  std::vector<Vertex_PCU>& verts, Vec2 const& bottomLeft, Vec2 const& bottomRight, Vec2 const& topRight, Vec2 const& topLeft, Rgba8 tint, AABB2 const& UV = AABB2::ZERO_TO_ONE ); 

//----------------------------------------------------------------------------------------------------------------------
// 3D shapes
//----------------------------------------------------------------------------------------------------------------------
void    AddVertsForLine3D( std::vector<Vertex_PCU>& verts, Vec3 const& start, Vec3 const& end, Rgba8 const& color );
void	AddVertsForQuad3D( std::vector<Vertex_PCU>& verts, Vec3 const& bottomLeft, Vec3 const& bottomRight, Vec3 const& topRight, Vec3 const& topLeft, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2( 0.0f, 0.0f, 1.0f, 1.0f )  ); 
void	AddVertsForQuad3D( std::vector<Vertex_PCU>& verts, Vec3 const& bottomLeft, Vec3 const& bottomRight, Vec3 const& topRight, Vec3 const& topLeft, Rgba8 const& color, Vec2 UV_bottomLeft, Vec2 UV_bottomRight, Vec2 UV_TopLeft, Vec2 UV_TopRight ); 
void	AddVertsForQuad3D( std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, Vec3 const& bottomLeft, Vec3 const& bottomRight, Vec3 const& topRight, Vec3 const& topLeft, Rgba8 const& color, AABB2 const& UVs = AABB2::ZERO_TO_ONE ); 
void	AddVertsForQuad3D( std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, Vec3 const& bottomLeft, Vec3 const& bottomRight, Vec3 const& topRight, Vec3 const& topLeft, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE ); 
void	AddVertsForQuad3D( std::vector<Vertex_PCUTBN>& verts, Vec3 const& bottomLeft, Vec3 const& bottomRight, Vec3 const& topRight, Vec3 const& topLeft, Rgba8 const& color, AABB2 const& UVs );
void	AddVertsForRoundedQuad3D( std::vector<Vertex_PCUTBN>& verts, Vec3 const& bottomLeft, Vec3 const& bottomRight, Vec3 const& topRight, Vec3 const& topLeft, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE );
void	AddvertsforCube3D( std::vector<Vertex_PCU>& verts,  Vec3 const& center, float radius, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE );
void	AddvertsforCube3D( std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, AABB3 const& bounds, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE );
void	AddVertsForAABB3D( std::vector<Vertex_PCU>& verts, AABB3 const& bounds, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE );
void	AddVertsForAABBWireframe3D( std::vector<Vertex_PCU>& verts, AABB3 const& bounds, float lineThickness, Rgba8 const& tint = Rgba8::WHITE );
void	AddVertsForCylinder3D( std::vector<Vertex_PCU>& verts, Vec3 const& start, Vec3 const& end, float radius, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE, int numSlices = 8 );
void	AddVertsForCylinderZ3D( std::vector<Vertex_PCU>& verts, Vec2 const& centerXY, FloatRange const& minMaxZ, float radius, float numSlices, Rgba8 const& tint = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE );
void	AddVertsForCylinderZWireframe3D( std::vector<Vertex_PCU>& verts, Vec2 const& centerXY, FloatRange const& minMaxZ, float radius, float numSlices, float lineThickness, Rgba8 const& tint = Rgba8::WHITE );
void	AddVertsForSphere3D( std::vector<Vertex_PCU>& verts, Vec3 const& center, float radius, float numSlices, float numStacks, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE  );
void	AddVertsForSphere3D( std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, Vec3 const& center, float radius, float numSlices, float numStacks, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE );
void	AddVertsForUVSphereZWireframe3D( std::vector<Vertex_PCU>& verts, Vec3 const& center, float radius, float numSlices, float numStacks, float lineThickness, Rgba8 const& tint = Rgba8::WHITE );
// void AddVertsForConeZ3D( std::vector<Vertex_PCU>& verts, Vec3 const& start, Vec3 const& end, float radius, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE, int numSLices = 8 );
void	AddVertsForConeZ3D	  ( std::vector<Vertex_PCU>& verts, Vec2 const& start, FloatRange minMaxZ, float radius, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE, int numSLices = 8 );
void	AddVertsForCone3D	  ( std::vector<Vertex_PCU>& verts, Vec3 const& start, Vec3 const& end, float radius, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE, int numSLices = 8 );
void	AddVertsForArrow3D	  ( std::vector<Vertex_PCU>& verts, Vec3 const& start, Vec3 const& end, float radius, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE );
AABB2	GetVertexBounds2D	  ( std::vector<Vertex_PCU> const& verts );
void	AddVertsForPlane	  ( std::vector<Vertex_PCU>& vertList, std::vector<unsigned int>& indexList, Vec3 const& planeOriginPos_BL, int tileSize = 1, int numTilesX = 1, int numTilesY = 1, Rgba8 const& color = Rgba8::WHITE );
void	AddVertsForTriangle	  ( std::vector<Vertex_PCU>& vertList, Vec3 const& position_BL, float length = 1.0f, Rgba8 const& color = Rgba8::WHITE );
void	AddVertsForArc3D_Z_Up ( std::vector<Vertex_PCU>& verts, FloatRange const& floatRange, float length, Rgba8 const& color = Rgba8::WHITE, Vec2 const& UV = Vec2::ZERO );
void	AddVertsForArc3D_Yaw  ( std::vector<Vertex_PCU>& verts, Mat44 const& matrix, FloatRange const& floatRange, float length, Rgba8 const& color = Rgba8::WHITE, Vec2 const& UV = Vec2::ZERO );
void	AddVertsForArc3D_Pitch( std::vector<Vertex_PCU>& verts, Mat44 const& matrix, FloatRange const& floatRange, float length, Rgba8 const& color = Rgba8::WHITE, Vec2 const& UV = Vec2::ZERO );
void	AddVertsForArc3D_Roll ( std::vector<Vertex_PCU>& verts, Mat44 const& matrix, FloatRange const& floatRange, float length, Rgba8 const& color = Rgba8::WHITE, Vec2 const& UV = Vec2::ZERO );

//----------------------------------------------------------------------------------------------------------------------
// Utilities
//----------------------------------------------------------------------------------------------------------------------
void	CalculateTangents( std::vector<Vertex_PCUTBN>& vertList, std::vector<unsigned int> const& indexList );

//----------------------------------------------------------------------------------------------------------------------
// Not required to have these functions
// void AddVertsForCapsule2D( std::vector<Vertex_PCU>& verts, Capsule2D const& capsule, Rgba8 const& color );
// void AddVertsForLine2D( std::vector<Vertex_PCU>& verts, LineSegment2D const& lineSegment, float thickness, Rgba8 color );