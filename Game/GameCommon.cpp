#include "Game/GameCommon.hpp"

//----------------------------------------------------------------------------------------------------------------------
// Debug Globals
//----------------------------------------------------------------------------------------------------------------------
extern bool  g_debugPrintJointLimbPositions		= false;
extern bool  g_debugToggleGrass					= true;
extern bool  g_debugControlElevator				= true;
extern bool  g_debugFreeFly_F1					= false;
extern bool  g_debugRenderRaycast_F2			= false;
extern bool  g_debugBasis_F3					= false;
extern bool  g_debugText_F4						= false;
extern bool  g_debugAngles_F5					= true;
extern bool  g_debugToggleConstraints_F8		= false;
extern bool  g_debugFollowTarget_F11			= true;
extern bool  g_debugRenderBezier_1				= false;
extern bool  g_debugToggleLegs_2				= false;
extern float g_GRAVITY							= 9.8f;


//----------------------------------------------------------------------------------------------------------------------
void DebugDrawRing()
{
}

//----------------------------------------------------------------------------------------------------------------------
void DebugDrawLine( Vec2 const& start, Vec2 const& end, float thickness, Rgba8 const& color )
{
	int triangles = 2;
	int verts	= triangles * 3; 
	float halfThickness = thickness / 2;

	Vec2 length = end - start;
	Vec2 normalizedVec = length.GetNormalized();
	Vec2 getForwardNormal = normalizedVec * halfThickness;
	Vec2 getLeftNormal = getForwardNormal.GetRotated90Degrees();
	
	Vec2 EL = end + getForwardNormal + getLeftNormal;
	Vec2 ER = end + getForwardNormal - getLeftNormal;
	Vec2 SL = start - getForwardNormal + getLeftNormal;
	Vec2 SR = start - getForwardNormal - getLeftNormal;

	Vertex_PCU drawLine[] = 
	{
		Vertex_PCU(Vec3(SR.x, SR.y, 0.0f), color, Vec2(0.0f, 0.0f)), //	Triangle A point A
		Vertex_PCU(Vec3(ER.x, ER.y, 0.0f), color, Vec2(0.0f, 0.0f)), //	Triangle A point B
		Vertex_PCU(Vec3(EL.x, EL.y, 0.0f), color, Vec2(0.0f, 0.0f)), //	Triangle A point C

		Vertex_PCU(Vec3(SR.x, SR.y, 0.0f), color, Vec2(0.0f, 0.0f)), //	Triangle B point A
		Vertex_PCU(Vec3(EL.x, EL.y, 0.0f), color, Vec2(0.0f, 0.0f)), //	Triangle B point B
		Vertex_PCU(Vec3(SL.x, SL.y, 0.0f), color, Vec2(0.0f, 0.0f))  //	Triangle B point C
	};

	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->DrawVertexArray( verts, drawLine );
}