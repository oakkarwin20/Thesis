#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Core/EngineCommon.hpp"

//----------------------------------------------------------------------------------------------------------------------
void TransformVertexArrayXY3D(int numVertz, Vertex_PCU* verts, float uniformScaleXY, float rotationDegreesAboutZ, Vec2 const& translationXY)
{
	for (int i = 0; i < numVertz; i++)
	{
		TransformPositionXY3D( verts[i].m_position, uniformScaleXY, rotationDegreesAboutZ, translationXY );
	}
}

//----------------------------------------------------------------------------------------------------------------------
void TransformVertexArray3D( std::vector<Vertex_PCU>& verts, Mat44 const& transform )
{
	TransformVertexArray3D( int(verts.size()), verts.data(), transform );
}

//----------------------------------------------------------------------------------------------------------------------
void TransformVertexArray3D( int numVerts, Vertex_PCU* verts, Mat44 const& transform )
{
	for ( int i = 0; i < numVerts; i++ )
	{
		Vec3& position		= verts[i].m_position;
		verts[i].m_position = transform.TransformPosition3D( position );
	}
}

//----------------------------------------------------------------------------------------------------------------------
void TransformVertexArray3D( std::vector<Vertex_PCUTBN>& verts, Mat44 const& transform )
{
	for ( int i = 0; i < verts.size(); i++ )
	{
		Vec3 vertPos			= verts[i].m_position;
		verts[i].m_position		= transform.TransformPosition3D( vertPos );
		verts[i].m_normal		= transform.TransformVectorQuantity3D( verts[i].m_normal );
		verts[i].m_normal		= verts[i].m_normal.GetNormalized();
	}
}

//----------------------------------------------------------------------------------------------------------------------
void AddVertsForAABB2D( std::vector<Vertex_PCU>& verts, AABB2 const& bounds, Rgba8 const& color, AABB2 const& UVs )
{
	Vec3 BL = Vec3(bounds.m_mins.x, bounds.m_mins.y, 0.0f);				// BottomLeft	
	Vec3 BR = Vec3(bounds.m_maxs.x, bounds.m_mins.y, 0.0f);				// BottomRight
	Vec3 TR = Vec3(bounds.m_maxs.x, bounds.m_maxs.y, 0.0f);				// TopRight	
	Vec3 TL = Vec3(bounds.m_mins.x, bounds.m_maxs.y, 0.0f);				// TopLeft	

	verts.push_back( Vertex_PCU( BL, color, UVs.m_mins ) );
	verts.push_back( Vertex_PCU( BR, color, Vec2( UVs.m_maxs.x, UVs.m_mins.y ) ) );
	verts.push_back( Vertex_PCU( TR, color, Vec2( UVs.m_maxs ) ) );

	verts.push_back( Vertex_PCU( BL, color, UVs.m_mins ) );
	verts.push_back( Vertex_PCU( TR, color, Vec2( UVs.m_maxs ) ) );
	verts.push_back( Vertex_PCU( TL, color, Vec2( UVs.m_mins.x, UVs.m_maxs.y ) ) );
}


//----------------------------------------------------------------------------------------------------------------------
void AddVertsForBordersAABB2D( std::vector<Vertex_PCU>& verts, AABB2 const& bounds, float thickness, Rgba8 const& color )
{
	// Calculate border bounds
	float minX = bounds.m_mins.x;
	float minY = bounds.m_mins.y;
	float maxX = bounds.m_maxs.x;
	float maxY = bounds.m_maxs.y;

	// Add verts for border bounds
	Vec2 BL = Vec2( minX, minY );
	Vec2 BR = Vec2( maxX, minY );
	Vec2 TL = Vec2( minX, maxY );
	Vec2 TR = Vec2( maxX, maxY );
	AddVertsForLineSegment2D( verts, BL, BR, thickness, color );
	AddVertsForLineSegment2D( verts, BR, TR, thickness, color );
	AddVertsForLineSegment2D( verts, TR, TL, thickness, color );
	AddVertsForLineSegment2D( verts, TL, BL, thickness, color );
}

//----------------------------------------------------------------------------------------------------------------------
void AddVertsForLineSegment2D( std::vector<Vertex_PCU>& verts, Vec2 startPos, Vec2 endPos, float thickness, Rgba8 color )
{ 
	Vec2 displacement = endPos - startPos;
	float halfthickness = thickness / 2.0f;
	Vec2 iStep = displacement.GetNormalized() * halfthickness;
	Vec2 jStep = iStep.GetRotated90Degrees();

	Vec2 SR = startPos - iStep - jStep;
	Vec2 SL = startPos - iStep + jStep;
	Vec2 ER = endPos + iStep - jStep;
	Vec2 EL = endPos + iStep + jStep;

	verts.push_back( Vertex_PCU( Vec3( SR ), color, Vec2( 0.0f, 0.0f ) ) );
	verts.push_back( Vertex_PCU( Vec3( EL ), color, Vec2( 0.0f, 0.0f ) ) );
	verts.push_back( Vertex_PCU( Vec3( SL ), color, Vec2( 0.0f, 0.0f ) ) );

	verts.push_back( Vertex_PCU( Vec3( SR ), color, Vec2( 0.0f, 0.0f ) ) );
	verts.push_back( Vertex_PCU( Vec3( ER ), color, Vec2( 0.0f, 0.0f ) ) );
	verts.push_back( Vertex_PCU( Vec3( EL ), color, Vec2( 0.0f, 0.0f ) ) );
}

//----------------------------------------------------------------------------------------------------------------------
void AddVertsForLineSegment3D( std::vector<Vertex_PCU>& verts, Vec3 startPos, Vec3 endPos, float thickness, Rgba8 color )
{
	Vec3 displament = endPos - startPos;
	Vec3 mins;
	Vec3 maxs;

	mins.x = startPos.x - thickness;
	mins.y = startPos.y - thickness;
	mins.z = startPos.z - thickness;

	maxs.x = endPos.x + thickness + displament.GetLength();
	maxs.y = endPos.y + thickness;
	maxs.z = endPos.z + thickness;

	//----------------------------------------------------------------------------------------------------------------------
	// I = mins // X = maxs // In (X,Y,Z) order
	mins;
	Vec3 IXI = Vec3( mins.x, mins.y + thickness, mins.z				);
	Vec3 IXX = Vec3( mins.x, mins.y + thickness, mins.z + thickness );
	Vec3 IIX = Vec3( mins.x,			 mins.y, mins.z + thickness );

	Vec3 XII = Vec3( maxs.x, maxs.y - thickness, maxs.z - thickness );
	Vec3 XXI = Vec3( maxs.x,			 maxs.y, maxs.z - thickness );
	maxs;
	Vec3 XIX = Vec3( maxs.x, maxs.y - thickness, maxs.z );

	// West Face
	AddVertsForQuad3D( verts, IXI, mins,  IIX,  IXX, color );
	// East Face						 
	AddVertsForQuad3D( verts, XII,  XXI, maxs,  XIX, color );
	// North Face						    	
	AddVertsForQuad3D( verts, XXI,  IXI,  IXX, maxs, color );
	// South Face						    	
	AddVertsForQuad3D( verts, mins, XII,  XIX,  IIX, color );
	// Sky Face							    	
	AddVertsForQuad3D( verts, IIX,  XIX, maxs,  IXX, color );
	// Ground Face						    	
	AddVertsForQuad3D( verts, XII, mins,  IXI,  XXI, color );

	//----------------------------------------------------------------------------------------------------------------------
//	verts.push_back( Vertex_PCU( mins, color, Vec2( 0.0f, 0.0f ) ) );
//	verts.push_back( Vertex_PCU(  XII, color, Vec2( 0.0f, 0.0f ) ) );
//	verts.push_back( Vertex_PCU(  XIX, color, Vec2( 0.0f, 0.0f ) ) );
//
//	verts.push_back( Vertex_PCU( mins, color, Vec2( 0.0f, 0.0f ) ) );
//	verts.push_back( Vertex_PCU(  XIX, color, Vec2( 0.0f, 0.0f ) ) );
//	verts.push_back( Vertex_PCU(  IIX, color, Vec2( 0.0f, 0.0f ) ) );
//
//	verts.push_back( Vertex_PCU( XII, color, Vec2( 0.0f, 0.0f ) ) );
//	verts.push_back( Vertex_PCU( XXI, color, Vec2( 0.0f, 0.0f ) ) );
//	verts.push_back( Vertex_PCU( maxs, color, Vec2( 0.0f, 0.0f ) ) );
//
//	verts.push_back( Vertex_PCU( XII, color, Vec2( 0.0f, 0.0f ) ) );
//	verts.push_back( Vertex_PCU( maxs, color, Vec2( 0.0f, 0.0f ) ) );
//	verts.push_back( Vertex_PCU( XIX, color, Vec2( 0.0f, 0.0f ) ) );
//
//	verts.push_back( Vertex_PCU( XIX, color, Vec2( 0.0f, 0.0f ) ) );
//	verts.push_back( Vertex_PCU( maxs, color, Vec2( 0.0f, 0.0f ) ) );
//	verts.push_back( Vertex_PCU( IXX, color, Vec2( 0.0f, 0.0f ) ) );
//
//	verts.push_back( Vertex_PCU( XIX, color, Vec2( 0.0f, 0.0f ) ) );
//	verts.push_back( Vertex_PCU( IXX, color, Vec2( 0.0f, 0.0f ) ) );
//	verts.push_back( Vertex_PCU( IIX, color, Vec2( 0.0f, 0.0f ) ) );
//
//	verts.push_back( Vertex_PCU( IXI, color, Vec2( 0.0f, 0.0f ) ) );
//	verts.push_back( Vertex_PCU( mins, color, Vec2( 0.0f, 0.0f ) ) );
//	verts.push_back( Vertex_PCU( IXX, color, Vec2( 0.0f, 0.0f ) ) );
//
//	verts.push_back( Vertex_PCU( mins, color, Vec2( 0.0f, 0.0f ) ) );
//	verts.push_back( Vertex_PCU( IIX, color, Vec2( 0.0f, 0.0f ) ) );
//	verts.push_back( Vertex_PCU( IXX, color, Vec2( 0.0f, 0.0f ) ) );
//
//	verts.push_back( Vertex_PCU( XXI, color, Vec2( 0.0f, 0.0f ) ) );
//	verts.push_back( Vertex_PCU( IXI, color, Vec2( 0.0f, 0.0f ) ) );
//	verts.push_back( Vertex_PCU( IXX, color, Vec2( 0.0f, 0.0f ) ) );
//
//	verts.push_back( Vertex_PCU( XXI, color, Vec2( 0.0f, 0.0f ) ) );
//	verts.push_back( Vertex_PCU( IXX, color, Vec2( 0.0f, 0.0f ) ) );
//	verts.push_back( Vertex_PCU( maxs, color, Vec2( 0.0f, 0.0f ) ) );
//
//	verts.push_back( Vertex_PCU( IXI, color, Vec2( 0.0f, 0.0f ) ) );
//	verts.push_back( Vertex_PCU( XXI, color, Vec2( 0.0f, 0.0f ) ) );
//	verts.push_back( Vertex_PCU( XII, color, Vec2( 0.0f, 0.0f ) ) );
//
//	verts.push_back( Vertex_PCU( IXI, color, Vec2( 0.0f, 0.0f ) ) );
//	verts.push_back( Vertex_PCU( XXI, color, Vec2( 0.0f, 0.0f ) ) );
//	verts.push_back( Vertex_PCU( mins, color, Vec2( 0.0f, 0.0f ) ) );
}

//----------------------------------------------------------------------------------------------------------------------
void AddVertsForDisc2D_UV( std::vector<Vertex_PCU>& verts, Vec2 centerPos, float radius, Rgba8 color, AABB2 const& UV )
{
	int		numPizzaSlices		= 48;
	float	degreesPerSlice		= 360.0f / (float)numPizzaSlices;

	for ( int i = 0; i < numPizzaSlices; i++ )
	{
		float startOrientation = degreesPerSlice * i;
		float endOrientation   = degreesPerSlice + startOrientation;

		Vec2 disp1 = Vec2::MakeFromPolarDegrees( startOrientation, radius );
		Vec2 disp2 = Vec2::MakeFromPolarDegrees( endOrientation, radius );

		Vec2 pos0 = centerPos;
		Vec2 pos1 = centerPos + disp1;
		Vec2 pos2 = centerPos + disp2;

		//----------------------------------------------------------------------------------------------------------------------
		// Calculate UVs
		Vec2 UV_bottomRight;
		Vec2 UV_TopRight;

		UV_bottomRight.x = RangeMapClamped( pos1.x,   0.0f, 180.0f, UV.m_maxs.x, UV.m_mins.x );
		UV_bottomRight.y = RangeMapClamped( pos1.y, 270.0f,  90.0f, UV.m_mins.y, UV.m_maxs.y );
		UV_TopRight.x	 = RangeMapClamped( pos2.x,	  0.0f, 180.0f, UV.m_maxs.x, UV.m_mins.x );
		UV_TopRight.y	 = RangeMapClamped( pos2.y, 270.0f,  90.0f, UV.m_mins.y, UV.m_maxs.y );

//		UV_BottomLeft.x = RangeMapClamped(							currentYawDegrees,   0.0f, 360.0f, UVs.m_mins.x, UVs.m_maxs.x );
//		UV_BottomLeft.y	= RangeMapClamped( currentPitchDegrees + pitchDegreesPerStack, -90.0f,  90.0f, UVs.m_maxs.y, UVs.m_mins.y );
//		UV_TopRight.x	= RangeMapClamped(	   currentYawDegrees + yawDegreesPerSlice,   0.0f, 360.0f, UVs.m_mins.x, UVs.m_maxs.x );
//		UV_TopRight.y	= RangeMapClamped(		  				  currentPitchDegrees, -90.0f,  90.0f, UVs.m_maxs.y, UVs.m_mins.y );

		verts.push_back( Vertex_PCU( Vec3( pos0 ), color, UV_bottomRight			 ) );
		verts.push_back( Vertex_PCU( Vec3( pos1 ), color, UV_bottomRight ) );
		verts.push_back( Vertex_PCU( Vec3( pos2 ), color, UV_TopRight	 ) );

//		verts.push_back( Vertex_PCU( Vec3( pos0 ), color, pos0  ) );
//		verts.push_back( Vertex_PCU( Vec3( pos1 ), color, disp2 ) );
//		verts.push_back( Vertex_PCU( Vec3( pos2 ), color, disp1 ) );
	}
}

//----------------------------------------------------------------------------------------------------------------------
void AddVertsForDisc2D( std::vector<Vertex_PCU>& verts, Vec2 centerPos, float radius, Rgba8 color )
{
	int		numPizzaSlices		= 48;
	float	degreesPerSlice		= 360.0f / (float)numPizzaSlices;

	for ( int i = 0; i < numPizzaSlices; i++ )
	{
		float startOrientation = degreesPerSlice * i;
		float endOrientation   = degreesPerSlice + startOrientation;

		Vec2 disp1 = Vec2::MakeFromPolarDegrees( startOrientation, radius );
		Vec2 disp2 = Vec2::MakeFromPolarDegrees( endOrientation, radius );

		Vec2 pos0 = centerPos;
		Vec2 pos1 = centerPos + disp1;
		Vec2 pos2 = centerPos + disp2;

		verts.push_back( Vertex_PCU( Vec3( pos0 ), color, Vec2( 0.0f, 0.0f ) ) );
		verts.push_back( Vertex_PCU( Vec3( pos1 ), color, Vec2( 0.0f, 0.0f ) ) );
		verts.push_back( Vertex_PCU( Vec3( pos2 ), color, Vec2( 0.0f, 0.0f ) ) );
	}
}


//----------------------------------------------------------------------------------------------------------------------
void AddVertsForRing2D( std::vector<Vertex_PCU>& verts, Vec2 centerPos, float radius, float thickness, Rgba8 color )
{
	int	  numSlices			= 48;
	float degreesPerSlice	= 360.0f / (float)numSlices;

	for ( int i = 0; i < numSlices; i++ )
	{
		float startOrientation	= degreesPerSlice * i;
		float endOrientation	= degreesPerSlice * ( i + 1.0f );
		float halfThickness = thickness / 2.0f; 
	
		Vec2 dispToRadius1 = Vec2::MakeFromPolarDegrees( startOrientation, radius ); 
		Vec2 dispToRadius2 = Vec2::MakeFromPolarDegrees( endOrientation, radius ); 
		Vec2 halfThicknessDispFromStart = Vec2::MakeFromPolarDegrees( startOrientation, halfThickness);
		Vec2 halfThicknessDispFromEnd	= Vec2::MakeFromPolarDegrees( endOrientation, halfThickness);
		
		Vec2 pos1 = centerPos + dispToRadius1;
		Vec2 pos2 = centerPos + dispToRadius2;
	
		Vec2 pos1R = pos1 + halfThicknessDispFromStart;
		Vec2 pos1L = pos1 - halfThicknessDispFromStart;
		Vec2 pos2R = pos2 + halfThicknessDispFromEnd;
		Vec2 pos2L = pos2 - halfThicknessDispFromEnd;
	
		verts.push_back( Vertex_PCU( Vec3( pos1L ), color, Vec2( 0.0f, 0.0f) ) );
		verts.push_back( Vertex_PCU( Vec3( pos1R ), color, Vec2( 0.0f, 0.0f) ) );
		verts.push_back( Vertex_PCU( Vec3( pos2R ), color, Vec2( 0.0f, 0.0f) ) );
		
		verts.push_back( Vertex_PCU( Vec3( pos1L ), color, Vec2( 0.0f, 0.0f) ) );
		verts.push_back( Vertex_PCU( Vec3( pos2R ), color, Vec2( 0.0f, 0.0f) ) );
		verts.push_back( Vertex_PCU( Vec3( pos2L ), color, Vec2( 0.0f, 0.0f) ) );
	}
}


//----------------------------------------------------------------------------------------------------------------------
void AddVertsForCapsule2D( std::vector<Vertex_PCU>& verts, Vec2 const& boneStart, Vec2 const& boneEnd, float radius, Rgba8 const& color )
{
	Vec2 SE								= boneEnd - boneStart;
	Vec2 normalizedStartToEnd			= SE.GetNormalized();
	Vec2 left							= normalizedStartToEnd.GetRotated90Degrees();

	Vec2 SL								= boneStart + (   left * radius );
	Vec2 SR								= boneStart + ( - left * radius );
	                       
	Vec2 EL								= boneEnd + (   left * radius );
	Vec2 ER								= boneEnd + ( - left * radius );

	//----------------------------------------------------------------------------------------------------------------------
	// AddVertsForSector2D
	int		numPizzaSlices		= 48;
	float	degreesPerSlice		= 180.0f / (float)numPizzaSlices;

	//----------------------------------------------------------------------------------------------------------------------
	// Adding sector for boneStart
	Vec2 sectorThetaFromStartLeftToStartRight = SL - boneStart;

	for ( int i = 0; i < numPizzaSlices; i++ )
	{
		float startOrientation	= ( degreesPerSlice * i ) + sectorThetaFromStartLeftToStartRight.GetOrientationDegrees();
		float endOrientation	= degreesPerSlice + startOrientation;

		Vec2 disp1 = Vec2::MakeFromPolarDegrees( startOrientation, radius );
		Vec2 disp2 = Vec2::MakeFromPolarDegrees( endOrientation, radius );

		Vec2 pos0 = boneStart ;
		Vec2 pos1 = boneStart + disp1;
		Vec2 pos2 = boneStart + disp2;

		verts.push_back( Vertex_PCU( Vec3( pos0 ), color, Vec2( 0.0f, 0.0f ) ) );
		verts.push_back( Vertex_PCU( Vec3( pos1 ), color, Vec2( 0.0f, 0.0f ) ) );
		verts.push_back( Vertex_PCU( Vec3( pos2 ), color, Vec2( 0.0f, 0.0f ) ) );
	}
	//----------------------------------------------------------------------------------------------------------------------
	// Adding sector for boneEnd
	Vec2 sectorThetaFromEndLeftToEndRight  = ER - boneEnd;

	for ( int i = 0; i < numPizzaSlices; i++ )
	{
		float startOrientation = ( degreesPerSlice * i ) + sectorThetaFromEndLeftToEndRight .GetOrientationDegrees();
		float endOrientation = degreesPerSlice + startOrientation;

		Vec2 disp1 = Vec2::MakeFromPolarDegrees( startOrientation, radius );
		Vec2 disp2 = Vec2::MakeFromPolarDegrees( endOrientation, radius );

		Vec2 pos0 = boneEnd;
		Vec2 pos1 = boneEnd + disp1;
		Vec2 pos2 = boneEnd + disp2;

		verts.push_back( Vertex_PCU( Vec3( pos0 ), color, Vec2( 0.0f, 0.0f ) ) );
		verts.push_back( Vertex_PCU( Vec3( pos1 ), color, Vec2( 0.0f, 0.0f ) ) );
		verts.push_back( Vertex_PCU( Vec3( pos2 ), color, Vec2( 0.0f, 0.0f ) ) );
	}

	verts.push_back( Vertex_PCU( Vec3(SR), color, Vec2( 0.0f, 0.0f ) ) );
	verts.push_back( Vertex_PCU( Vec3(EL), color, Vec2( 0.0f, 0.0f ) ) );
	verts.push_back( Vertex_PCU( Vec3(SL), color, Vec2( 0.0f, 0.0f ) ) );

	verts.push_back( Vertex_PCU( Vec3(SR), color, Vec2( 0.0f, 0.0f ) ) );
	verts.push_back( Vertex_PCU( Vec3(ER), color, Vec2( 0.0f, 0.0f ) ) );
	verts.push_back( Vertex_PCU( Vec3(EL), color, Vec2( 0.0f, 0.0f ) ) );
}


//----------------------------------------------------------------------------------------------------------------------
void AddVertsForOBB2D( std::vector<Vertex_PCU>& verts, OBB2D const& box, Rgba8 const& color )
{
	Vec2 jBasisNormal	= box.m_iBasisNormal.GetRotated90Degrees();
	Vec2 right			= box.m_iBasisNormal * box.m_halfDimensions.x;
	Vec2 up				= jBasisNormal * box.m_halfDimensions.y;

	Vec2 const& center	= box.m_center;
	Vec2 topRight		= center + right + up;
	Vec2 topLeft		= center - right + up;
	Vec2 bottomRight	= center + right - up;
	Vec2 bottomLeft		= center - right - up;

	verts.push_back( Vertex_PCU( Vec3( topRight ),		color, Vec2( 0.0f, 0.0f ) ) );
	verts.push_back( Vertex_PCU( Vec3( topLeft ),		color, Vec2( 0.0f, 0.0f ) ) );
	verts.push_back( Vertex_PCU( Vec3( bottomRight ),	color, Vec2( 0.0f, 0.0f ) ) );

	verts.push_back( Vertex_PCU( Vec3( topLeft ),		color, Vec2( 0.0f, 0.0f ) ) );
	verts.push_back( Vertex_PCU( Vec3( bottomLeft ),	color, Vec2( 0.0f, 0.0f ) ) );
	verts.push_back( Vertex_PCU( Vec3( bottomRight ),	color, Vec2( 0.0f, 0.0f ) ) );
}


//----------------------------------------------------------------------------------------------------------------------
void AddVertsForOBB3D( std::vector<Vertex_PCU>& verts, OBB3D const& box, Rgba8 const& color )
{
	Vec3 fwd			= box.m_iBasisNormal * box.m_halfDimensions.x;
	Vec3 left			= box.m_jBasisNormal * box.m_halfDimensions.y;
	Vec3 up				= box.m_kBasisNormal * box.m_halfDimensions.z;
	Vec3 const& center	= box.m_center;

	Vec3 EastNorthSky		= center + fwd + left + up;
	Vec3 EastSouthSky		= center + fwd - left + up;
	Vec3 EastNorthGround	= center + fwd + left - up;
	Vec3 EastSouthGround	= center + fwd - left - up; 
	
	Vec3 WestNorthSky		= center - fwd + left + up;
	Vec3 WestSouthSky		= center - fwd - left + up;
	Vec3 WestNorthGround	= center - fwd + left - up;
	Vec3 WestSouthGround	= center - fwd - left - up;

	// East face
	verts.emplace_back( Vertex_PCU( Vec3( EastNorthSky		),	color, Vec2( 0.0f, 1.0f ) ) );
	verts.emplace_back( Vertex_PCU( Vec3( EastSouthSky		),	color, Vec2( 0.0f, 1.0f ) ) );
	verts.emplace_back( Vertex_PCU( Vec3( EastSouthGround   ),	color, Vec2( 0.0f, 1.0f ) ) );
	verts.emplace_back( Vertex_PCU( Vec3( EastNorthSky		),	color, Vec2( 0.0f, 1.0f ) ) );
	verts.emplace_back( Vertex_PCU( Vec3( EastSouthGround   ),	color, Vec2( 0.0f, 1.0f ) ) );
	verts.emplace_back( Vertex_PCU( Vec3( EastNorthGround   ),	color, Vec2( 0.0f, 1.0f ) ) );

	// West face
	verts.emplace_back( Vertex_PCU( Vec3( WestNorthSky		),	color, Vec2( 0.0f, 1.0f ) ) );
	verts.emplace_back( Vertex_PCU( Vec3( WestNorthGround	),	color, Vec2( 0.0f, 1.0f ) ) );
	verts.emplace_back( Vertex_PCU( Vec3( WestSouthGround   ),	color, Vec2( 0.0f, 1.0f ) ) );
	verts.emplace_back( Vertex_PCU( Vec3( WestNorthSky		),	color, Vec2( 0.0f, 1.0f ) ) );
	verts.emplace_back( Vertex_PCU( Vec3( WestSouthGround   ),	color, Vec2( 0.0f, 1.0f ) ) );
	verts.emplace_back( Vertex_PCU( Vec3( WestSouthSky		),	color, Vec2( 0.0f, 1.0f ) ) );

	// North face
	verts.emplace_back( Vertex_PCU( Vec3( EastNorthSky		),	color, Vec2( 0.0f, 1.0f ) ) );
	verts.emplace_back( Vertex_PCU( Vec3( EastNorthGround	),	color, Vec2( 0.0f, 1.0f ) ) );
	verts.emplace_back( Vertex_PCU( Vec3( WestNorthGround   ),	color, Vec2( 0.0f, 1.0f ) ) );
	verts.emplace_back( Vertex_PCU( Vec3( EastNorthSky		),	color, Vec2( 0.0f, 1.0f ) ) );
	verts.emplace_back( Vertex_PCU( Vec3( WestNorthGround   ),	color, Vec2( 0.0f, 1.0f ) ) );
	verts.emplace_back( Vertex_PCU( Vec3( WestNorthSky		),	color, Vec2( 0.0f, 1.0f ) ) );

	// South face
	verts.emplace_back( Vertex_PCU( Vec3( EastSouthSky		),	color, Vec2( 0.0f, 1.0f ) ) );
	verts.emplace_back( Vertex_PCU( Vec3( WestSouthSky		),	color, Vec2( 0.0f, 1.0f ) ) );
	verts.emplace_back( Vertex_PCU( Vec3( WestSouthGround   ),	color, Vec2( 0.0f, 1.0f ) ) );
	verts.emplace_back( Vertex_PCU( Vec3( EastSouthSky		),	color, Vec2( 0.0f, 1.0f ) ) );
	verts.emplace_back( Vertex_PCU( Vec3( WestSouthGround   ),	color, Vec2( 0.0f, 1.0f ) ) );
	verts.emplace_back( Vertex_PCU( Vec3( EastSouthGround	),	color, Vec2( 0.0f, 1.0f ) ) );

	// Sky face
	verts.emplace_back( Vertex_PCU( Vec3( EastNorthSky		),	color, Vec2( 0.0f, 1.0f ) ) );
	verts.emplace_back( Vertex_PCU( Vec3( WestNorthSky		),	color, Vec2( 0.0f, 1.0f ) ) );
	verts.emplace_back( Vertex_PCU( Vec3( WestSouthSky		),	color, Vec2( 0.0f, 1.0f ) ) );
	verts.emplace_back( Vertex_PCU( Vec3( EastNorthSky		),	color, Vec2( 0.0f, 1.0f ) ) );
	verts.emplace_back( Vertex_PCU( Vec3( WestSouthSky		),	color, Vec2( 0.0f, 1.0f ) ) );
	verts.emplace_back( Vertex_PCU( Vec3( EastSouthSky		),	color, Vec2( 0.0f, 1.0f ) ) );

	// Ground face
	verts.emplace_back( Vertex_PCU( Vec3( EastSouthGround	), color, Vec2( 0.0f, 1.0f ) ) );
	verts.emplace_back( Vertex_PCU( Vec3( WestSouthGround	), color, Vec2( 0.0f, 1.0f ) ) );
	verts.emplace_back( Vertex_PCU( Vec3( WestNorthGround	), color, Vec2( 0.0f, 1.0f ) ) );
	verts.emplace_back( Vertex_PCU( Vec3( EastSouthGround	), color, Vec2( 0.0f, 1.0f ) ) );
	verts.emplace_back( Vertex_PCU( Vec3( WestNorthGround	), color, Vec2( 0.0f, 1.0f ) ) );
	verts.emplace_back( Vertex_PCU( Vec3( EastNorthGround	), color, Vec2( 0.0f, 1.0f ) ) );
}


//----------------------------------------------------------------------------------------------------------------------
void AddVertsForInfiniteLine( std::vector<Vertex_PCU>& verts, Vec2 start, Vec2 end, float thickness, Rgba8 color )
{
	Vec2 disp = end - start;
	disp.SetLength( 2000.0f );
	Vec2 lineExtendedFromEnd	= end + disp;
	Vec2 lineExtendedFromStart	= start - disp;
	AddVertsForLineSegment2D( verts, lineExtendedFromStart, lineExtendedFromEnd, thickness, color );
}


//----------------------------------------------------------------------------------------------------------------------
void AddVertsForArrow2D( std::vector<Vertex_PCU>& verts, Vec2 tailPos, Vec2 tipPos, float arrowSize, float lineThickness, Rgba8 color )
{
	AddVertsForLineSegment2D( verts, tailPos, tipPos, lineThickness, color );

	Vec2 tailToTip	= tipPos - tailPos;
	Vec2 iBasis		= tailToTip.GetNormalized(); 
	Vec2 jBasis		= iBasis.GetRotated90Degrees();
	Vec2 kBasis		= -jBasis;

	Vec2 jBasisTiled = jBasis.GetRotatedDegrees(  45.0f );
	Vec2 kBasisTiled = kBasis.GetRotatedDegrees( -45.0f );

	Vec2 leftArrowTip	= ( jBasisTiled * arrowSize ) + tipPos;
	Vec2 rightArrowTip	= ( kBasisTiled * arrowSize ) + tipPos;

	AddVertsForLineSegment2D( verts, tipPos,  leftArrowTip, lineThickness, color );
	AddVertsForLineSegment2D( verts, tipPos, rightArrowTip, lineThickness, color );
}


//----------------------------------------------------------------------------------------------------------------------
void AddVertsForHexagon2D( std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexList, Vec2 centerPos, float radius, Rgba8 color )
{
	int		startIndex		= int( verts.size() );
	int		numPizzaSlices	= 6;
	float	degreesPerSlice = 60.0f;

	// Calculate the vert[0] and vert[1] of the first triangle
	Vec2 pos0				= centerPos;
	Vec2 disp1				= Vec2::MakeFromPolarDegrees( 0.0f, radius );
	Vec2 pos1				= centerPos + disp1;
	float endOrientation	= degreesPerSlice;
	Vec2 disp2				= Vec2::MakeFromPolarDegrees( endOrientation, radius );
	Vec2 pos2				= centerPos + disp2;
	// Add the unique positions ( vert[0] and vert[1] ) to the vertLis
	verts.push_back( Vertex_PCU( Vec3( pos0 ), color, Vec2( 0.0f, 0.0f ) ) );
	verts.push_back( Vertex_PCU( Vec3( pos1 ), color, Vec2( 0.0f, 0.0f ) ) );


	// Loop through the disc and add every "endPos" to vertsList
	for ( int i = 0; i < numPizzaSlices - 1; i++ )
	{
		float startOrientation	= degreesPerSlice * i;
		endOrientation			= degreesPerSlice + startOrientation;
		disp2					= Vec2::MakeFromPolarDegrees( endOrientation, radius );
		pos2					= centerPos + disp2;
		verts.push_back( Vertex_PCU( Vec3( pos2 ), color, Vec2( 0.0f, 0.0f ) ) );
	}


	//----------------------------------------------------------------------------------------------------------------------
	// Loop through numPizzaSlices and create triangles for indexList using vertList
	// Version where index counts are considered
	//----------------------------------------------------------------------------------------------------------------------
	for ( int i = 0; i < numPizzaSlices; i++ )
	{
		if ( i == ( numPizzaSlices - 1 ) )
		{
			// Handle edge case for last slice (index 0, 6, 1)
			indexList.push_back( startIndex );
			indexList.push_back( startIndex + i + 1 );
			indexList.push_back( ( numPizzaSlices - i ) + startIndex );
		}
		else
		{
			// Proceed along vert list as normal
			indexList.push_back( startIndex			);
			indexList.push_back( startIndex + i + 1 );
			indexList.push_back( startIndex + i + 2 );
		}
	}
}


//----------------------------------------------------------------------------------------------------------------------
void AddVertsForHexagon2D( std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexList, Vec2 centerPos, float radius, Rgba8 color )
{
	int		startIndex		= int( verts.size() );
	int		numPizzaSlices	= 6;
	float	degreesPerSlice = 60.0f;

	// Calculate the vert[0] and vert[1] of the first triangle
	Vec2 pos0				= centerPos;
	Vec2 disp1				= Vec2::MakeFromPolarDegrees( 0.0f, radius );
	Vec2 pos1				= centerPos + disp1;
	float endOrientation	= degreesPerSlice;
	Vec2 disp2				= Vec2::MakeFromPolarDegrees( endOrientation, radius );
	Vec2 pos2				= centerPos + disp2;
	Vec3 normal				= CrossProduct3D( Vec3( pos1 - pos0 ), Vec3( pos2 - pos1 ) );
	normal					= normal.GetNormalized();

	// Add the unique positions ( vert[0] and vert[1] ) to the vertLis
	verts.push_back( Vertex_PCUTBN( Vec3( pos0 ), color, Vec2( 0.0f, 1.0f ), Vec3::ZERO, Vec3::ZERO, normal ) );
	verts.push_back( Vertex_PCUTBN( Vec3( pos1 ), color, Vec2( 0.0f, 1.0f ), Vec3::ZERO, Vec3::ZERO, normal ) );


	// Loop through the disc and add every "endPos" to vertsList
	for ( int i = 0; i < numPizzaSlices - 1; i++ )
	{
		float startOrientation	= degreesPerSlice * i;
		endOrientation			= degreesPerSlice + startOrientation;
		disp2					= Vec2::MakeFromPolarDegrees( endOrientation, radius );
		pos2					= centerPos + disp2;

		disp1					= Vec2::MakeFromPolarDegrees( startOrientation, radius );
		pos1					= centerPos + disp1;
		normal					= CrossProduct3D( Vec3( pos1 - pos0 ), Vec3( pos2 - pos1 ) );
		normal					= normal.GetNormalized();
		verts.push_back( Vertex_PCUTBN( Vec3( pos2 ), color, Vec2( 0.0f, 1.0f ), Vec3::ZERO, Vec3::ZERO, normal  ) );
	}


	//----------------------------------------------------------------------------------------------------------------------
	// Loop through numPizzaSlices and create triangles for indexList using vertList
	// Version where index counts are considered
	//----------------------------------------------------------------------------------------------------------------------
	for ( int i = 0; i < numPizzaSlices; i++ )
	{
		if ( i == ( numPizzaSlices - 1 ) )
		{
			// Handle edge case for last slice (index 0, 6, 1)
			indexList.push_back( startIndex );
			indexList.push_back( startIndex + i + 1 );
			indexList.push_back( ( numPizzaSlices - i ) + startIndex );
		}
		else
		{
			// Proceed along vert list as normal
			indexList.push_back( startIndex			);
			indexList.push_back( startIndex + i + 1 );
			indexList.push_back( startIndex + i + 2 );
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
void AddVertsForHexagon2D( std::vector<Vertex_PCU>& verts, Vec2 centerPos, float radius, Rgba8 color )
{
	int		numPizzaSlices	= 6;
	float	degreesPerSlice = 60.0f;
	for ( int i = 0; i < numPizzaSlices; i++ )
	{
		float startOrientation	= degreesPerSlice * i;
		float endOrientation	= degreesPerSlice + startOrientation;

		Vec2 disp1 = Vec2::MakeFromPolarDegrees( startOrientation, radius );
		Vec2 disp2 = Vec2::MakeFromPolarDegrees( endOrientation, radius );

		Vec2 pos0 = centerPos;
		Vec2 pos1 = centerPos + disp1;
		Vec2 pos2 = centerPos + disp2;

		verts.push_back( Vertex_PCU( Vec3( pos0 ), color, Vec2( 0.0f, 0.0f ) ) );
		verts.push_back( Vertex_PCU( Vec3( pos1 ), color, Vec2( 0.0f, 0.0f ) ) );
		verts.push_back( Vertex_PCU( Vec3( pos2 ), color, Vec2( 0.0f, 0.0f ) ) );
	}
}


//----------------------------------------------------------------------------------------------------------------------
void AddVertsForHexagonBorders2D( std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexList, Vec2 centerPos, float thickness, Rgba8 color )
{	
	int	  startIndex		= int( verts.size() );
	int	  numSlices			= 6;
	float degreesPerSlice	= 60.0f;
	float borderThickness	= thickness * 0.1f;

	// Calculate the vert[0] and vert[1] of the first triangle
	Vec2 dispToRadius1					= Vec2::MakeFromPolarDegrees( 0.0f, thickness );
	Vec2 borderThicknessDispFromStart	= Vec2::MakeFromPolarDegrees( 0.0f, borderThickness	);
	Vec2 pos1R							= centerPos + dispToRadius1 + borderThicknessDispFromStart;
	Vec2 pos1L							= centerPos + dispToRadius1 - borderThicknessDispFromStart;
	verts.push_back( Vertex_PCU( Vec3( pos1L ), color, Vec2( 0.0f, 0.0f ) ) );
	verts.push_back( Vertex_PCU( Vec3( pos1R ), color, Vec2( 0.0f, 0.0f ) ) );

	for ( int i = 0; i < numSlices - 1; i++ )
	{
		float endOrientation			= degreesPerSlice * ( i + 1.0f );
		Vec2 dispToRadius2				= Vec2::MakeFromPolarDegrees( endOrientation, thickness			);
		Vec2 halfThicknessDispFromEnd	= Vec2::MakeFromPolarDegrees( endOrientation, borderThickness	);
		Vec2 pos2R						= centerPos + dispToRadius2 + halfThicknessDispFromEnd;
		Vec2 pos2L						= centerPos + dispToRadius2 - halfThicknessDispFromEnd;
		verts.push_back( Vertex_PCU( Vec3( pos2L ), color, Vec2( 0.0f, 1.0f ) ) );
		verts.push_back( Vertex_PCU( Vec3( pos2R ), color, Vec2( 0.0f, 1.0f ) ) );
	}

	// Loop through numPizzaSlices and create triangles for indexList using vertList
	unsigned int a = startIndex + 1;
	unsigned int b = startIndex + 2;
	unsigned int c = startIndex + 0;
	unsigned int d = startIndex + 1;
	unsigned int e = startIndex + 3;
	unsigned int f = startIndex + 2;
	for ( int i = 0; i < numSlices; i++ )
	{
		if ( i == ( numSlices - 1 ) )
		{
			// Handle edge case for last slice (index 0, 6, 1)
			
			// a += 2;
			b  -= 12;
			// c  += 2;
			// d += 2;
			e  -= 12;
			f  -= 12;
			

			/*
				a = 11;
				b =  0;
				c = 10;

				d = 11;
				e =  1;
				f =  0;
			*/

			indexList.push_back( a );
			indexList.push_back( b );
			indexList.push_back( c );
			indexList.push_back( d );
			indexList.push_back( e );
			indexList.push_back( f );
		}
		else
		{
			// Proceed along vert list as normal
			indexList.push_back( a );
			indexList.push_back( b );
			indexList.push_back( c );
			
			indexList.push_back( d );
			indexList.push_back( e );
			indexList.push_back( f );

			a += 2;
			b += 2;
			c += 2;
			d += 2;
			e += 2;
			f += 2;
		}
	}
}


//----------------------------------------------------------------------------------------------------------------------
void AddVertsForHexagonBorders2D( std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexList, Vec2 centerPos, float thickness, Rgba8 color )
{
	int	  startIndex		= int( verts.size() );
	int	  numSlices			= 6;
	float degreesPerSlice	= 60.0f;
	float borderThickness	= thickness * 0.1f;

	// Calculate the vert[0] and vert[1] of the first triangle
	Vec2 dispToRadius1					= Vec2::MakeFromPolarDegrees(			 0.0f, thickness		);
	Vec2 dispToRadius2					= Vec2::MakeFromPolarDegrees( degreesPerSlice, thickness		);
	Vec2 borderThicknessDispFromStart	= Vec2::MakeFromPolarDegrees(			 0.0f, borderThickness	);
	Vec2 pos1R							= centerPos + dispToRadius1 + borderThicknessDispFromStart;
	Vec2 pos1L							= centerPos + dispToRadius1 - borderThicknessDispFromStart;
	Vec2 pos2L							= centerPos + dispToRadius2 - borderThicknessDispFromStart;

	// Calculate normals
	Vec3 normal							= CrossProduct3D( Vec3( pos2L - pos1R ), Vec3( pos1L - pos1R ) );
	normal								= normal.GetNormalized();
	verts.push_back( Vertex_PCUTBN( Vec3( pos1L ), color, Vec2( 0.0f, 1.0f ), Vec3::ZERO, Vec3::ZERO, normal  ) );
	verts.push_back( Vertex_PCUTBN( Vec3( pos1R ), color, Vec2( 0.0f, 1.0f ), Vec3::ZERO, Vec3::ZERO, normal  ) );

	for ( int i = 0; i < numSlices - 1; i++ )
	{
		float startOrientation			= degreesPerSlice * i;
		float endOrientation			= degreesPerSlice * ( i + 1.0f );
		dispToRadius2					= Vec2::MakeFromPolarDegrees( endOrientation, thickness			);
		Vec2 halfThicknessDispFromEnd	= Vec2::MakeFromPolarDegrees( endOrientation, borderThickness	);
		Vec2 pos2R						= centerPos + dispToRadius2 + halfThicknessDispFromEnd;
		pos2L							= centerPos + dispToRadius2 - halfThicknessDispFromEnd;

		// Calculate normals
		dispToRadius1					= Vec2::MakeFromPolarDegrees( startOrientation, thickness );
		borderThicknessDispFromStart	= Vec2::MakeFromPolarDegrees( startOrientation, borderThickness	);
		pos1R							= centerPos + dispToRadius1 + borderThicknessDispFromStart;
		pos1L							= centerPos + dispToRadius1 - borderThicknessDispFromStart;
		normal							= CrossProduct3D( Vec3( pos2L - pos1R ), Vec3( pos1L - pos1R ) );
		normal							= normal.GetNormalized();

		verts.push_back( Vertex_PCUTBN( Vec3( pos2L ), color, Vec2( 0.0f, 1.0f ), Vec3::ZERO, Vec3::ZERO, normal ) );
		verts.push_back( Vertex_PCUTBN( Vec3( pos2R ), color, Vec2( 0.0f, 1.0f ), Vec3::ZERO, Vec3::ZERO, normal ) );
	}

	// Loop through numPizzaSlices and create triangles for indexList using vertList
	unsigned int a = startIndex + 1;
	unsigned int b = startIndex + 2;
	unsigned int c = startIndex + 0;
	unsigned int d = startIndex + 1;
	unsigned int e = startIndex + 3;
	unsigned int f = startIndex + 2;
	for ( int i = 0; i < numSlices; i++ )
	{
		if ( i == ( numSlices - 1 ) )
		{
			// Handle edge case for last slice (index 0, 6, 1)
			// a += 2;
			b  -= 12;
			// c  += 2;
			// d += 2;
			e  -= 12;
			f  -= 12;
			
			indexList.push_back( a );
			indexList.push_back( b );
			indexList.push_back( c );
			indexList.push_back( d );
			indexList.push_back( e );
			indexList.push_back( f );
		}
		else
		{
			// Proceed along vert list as normal
			indexList.push_back( a );
			indexList.push_back( b );
			indexList.push_back( c );
			
			indexList.push_back( d );
			indexList.push_back( e );
			indexList.push_back( f );

			a += 2;
			b += 2;
			c += 2;
			d += 2;
			e += 2;
			f += 2;
		}
	}
}


//----------------------------------------------------------------------------------------------------------------------
void AddVertsForHexagonBorders2D( std::vector<Vertex_PCU>& verts, Vec2 centerPos, float thickness, Rgba8 color )
{
	int	  numSlices			= 6;
	float degreesPerSlice	= 60.0f;
	float borderThickness	= thickness * 0.1f;
	for ( int i = 0; i < numSlices; i++ )
	{
		float startOrientation			= degreesPerSlice * i;
		float endOrientation			= degreesPerSlice * ( i + 1.0f );
//		float halfThickness				= thickness / 2.0f; 
		Vec2 dispToRadius1				= Vec2::MakeFromPolarDegrees( startOrientation, thickness        ); 
		Vec2 dispToRadius2				= Vec2::MakeFromPolarDegrees(   endOrientation, thickness		 ); 
		Vec2 halfThicknessDispFromStart = Vec2::MakeFromPolarDegrees( startOrientation, borderThickness  );
		Vec2 halfThicknessDispFromEnd	= Vec2::MakeFromPolarDegrees(   endOrientation, borderThickness  );

		Vec2 pos1 = centerPos + dispToRadius1;
		Vec2 pos2 = centerPos + dispToRadius2;

		Vec2 pos1R = pos1 + halfThicknessDispFromStart;
		Vec2 pos1L = pos1 - halfThicknessDispFromStart;
		Vec2 pos2R = pos2 + halfThicknessDispFromEnd;
		Vec2 pos2L = pos2 - halfThicknessDispFromEnd;

		verts.push_back( Vertex_PCU( Vec3( pos1L ), color, Vec2( 0.0f, 0.0f) ) );
		verts.push_back( Vertex_PCU( Vec3( pos1R ), color, Vec2( 0.0f, 0.0f) ) );
		verts.push_back( Vertex_PCU( Vec3( pos2R ), color, Vec2( 0.0f, 0.0f) ) );

		verts.push_back( Vertex_PCU( Vec3( pos1L ), color, Vec2( 0.0f, 0.0f) ) );
		verts.push_back( Vertex_PCU( Vec3( pos2R ), color, Vec2( 0.0f, 0.0f) ) );
		verts.push_back( Vertex_PCU( Vec3( pos2L ), color, Vec2( 0.0f, 0.0f) ) );
	}
}


//----------------------------------------------------------------------------------------------------------------------
void AddVertsForQuad2D( std::vector<Vertex_PCU>& verts, Vec2 const& bottomLeft, Vec2 const& bottomRight, Vec2 const& topRight, Vec2 const& topLeft, Rgba8 tint, Vec2 UV_bottomLeft, Vec2 UV_bottomRight, Vec2 UV_TopRight, Vec2 UV_TopLeft )
{
	UNUSED( verts );
	UNUSED( bottomLeft );
	UNUSED( bottomRight );
	UNUSED( topRight );
	UNUSED( topLeft );
	UNUSED( tint );
	UNUSED(  UV_bottomLeft );
	UNUSED( UV_bottomRight );
	UNUSED( UV_TopRight );
	UNUSED( UV_TopLeft );
}


//----------------------------------------------------------------------------------------------------------------------
void AddVertsForQuad2D( std::vector<Vertex_PCU>& verts, Vec2 const& bottomLeft, Vec2 const& bottomRight, Vec2 const& topRight, Vec2 const& topLeft, Rgba8 color, AABB2 const& UVs )
{
	Vec2 bottomLeft_UV	= Vec2( UVs.m_mins.x, UVs.m_mins.y );
	Vec2 bottomRight_UV = Vec2( UVs.m_maxs.x, UVs.m_mins.y );
	Vec2 TopRight_UV	= Vec2( UVs.m_maxs.x, UVs.m_maxs.y );
	Vec2 TopLeft_UV		= Vec2( UVs.m_mins.x, UVs.m_maxs.y );

	verts.push_back( Vertex_PCU( (Vec3) bottomLeft, color, bottomLeft_UV	) );
	verts.push_back( Vertex_PCU( (Vec3) bottomRight, color, bottomRight_UV	) );
	verts.push_back( Vertex_PCU( (Vec3) topRight, color, TopRight_UV		) );

	verts.push_back( Vertex_PCU( (Vec3) bottomLeft, color, bottomLeft_UV	) );
	verts.push_back( Vertex_PCU( (Vec3) topRight, color, TopRight_UV	    ) );
	verts.push_back( Vertex_PCU( (Vec3) topLeft, color, TopLeft_UV			) );
}


//----------------------------------------------------------------------------------------------------------------------
void AddVertsForLine3D( std::vector<Vertex_PCU>& verts, Vec3 const& start, Vec3 const& end, Rgba8 const& color )
{
	verts.push_back( Vertex_PCU( start, color, Vec2::ZERO ) );
	verts.push_back( Vertex_PCU(   end, color, Vec2::ZERO ) );
}


//----------------------------------------------------------------------------------------------------------------------
void AddVertsForQuad3D( std::vector<Vertex_PCU>& verts, Vec3 const& bottomLeft, Vec3 const& bottomRight, Vec3 const& topRight, Vec3 const& topLeft, Rgba8 const& color, AABB2 const& UVs )
{
	Vec2 bottomLeft_UV	= Vec2( UVs.m_mins.x, UVs.m_mins.y );
	Vec2 bottomRight_UV = Vec2( UVs.m_maxs.x, UVs.m_mins.y );
	Vec2 TopRight_UV	= Vec2( UVs.m_maxs.x, UVs.m_maxs.y );
	Vec2 TopLeft_UV		= Vec2( UVs.m_mins.x, UVs.m_maxs.y );

	verts.emplace_back( bottomLeft, color, bottomLeft_UV	);
	verts.emplace_back( bottomRight, color, bottomRight_UV	);
	verts.emplace_back(    topRight, color, TopRight_UV		);
													 
	verts.emplace_back( bottomLeft, color, bottomLeft_UV	);
	verts.emplace_back(   topRight, color, TopRight_UV	    );
	verts.emplace_back(    topLeft, color, TopLeft_UV	    );
}


//----------------------------------------------------------------------------------------------------------------------
void AddVertsForQuad3D( std::vector<Vertex_PCUTBN>& verts, Vec3 const& bottomLeft, Vec3 const& bottomRight, Vec3 const& topRight, Vec3 const& topLeft, Rgba8 const& color, AABB2 const& UVs )
{
	// Calculate forward
	Vec3 tangent	= bottomRight - bottomLeft;
	Vec3 bitangent	= topLeft - bottomLeft;						// bi-tangent
	Vec3 normal		= (CrossProduct3D( tangent, bitangent )).GetNormalized();

	Vec2 bottomLeft_UV	= Vec2( UVs.m_mins.x, UVs.m_mins.y );
	Vec2 bottomRight_UV = Vec2( UVs.m_maxs.x, UVs.m_mins.y );
	Vec2 topRight_UV	= Vec2( UVs.m_maxs.x, UVs.m_maxs.y );
	Vec2 topLeft_UV		= Vec2( UVs.m_mins.x, UVs.m_maxs.y );

	verts.push_back( Vertex_PCUTBN(  bottomLeft, color,  bottomLeft_UV, Vec3::ZERO, Vec3::ZERO, normal ) );
	verts.push_back( Vertex_PCUTBN( bottomRight, color, bottomRight_UV, Vec3::ZERO, Vec3::ZERO, normal ) );
	verts.push_back( Vertex_PCUTBN(    topRight, color,	   topRight_UV, Vec3::ZERO, Vec3::ZERO, normal ) );

	verts.push_back( Vertex_PCUTBN( bottomLeft, color, bottomLeft_UV, Vec3::ZERO, Vec3::ZERO, normal ) );
	verts.push_back( Vertex_PCUTBN(   topRight, color,   topRight_UV, Vec3::ZERO, Vec3::ZERO, normal ) );
	verts.push_back( Vertex_PCUTBN(    topLeft, color,    topLeft_UV, Vec3::ZERO, Vec3::ZERO, normal ) );
}


//----------------------------------------------------------------------------------------------------------------------
void AddVertsForQuad3D( std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, Vec3 const& bottomLeft, Vec3 const& bottomRight, Vec3 const& topRight, Vec3 const& topLeft, Rgba8 const& color, AABB2 const& UVs )
{
	int startIndex = static_cast<int>( verts.size() );

	// Calculate forward
	Vec3 tangent	= bottomRight - bottomLeft;
	Vec3 bitangent	= topLeft - bottomLeft;						// bi-tangent
	Vec3 normal		= (CrossProduct3D( tangent, bitangent )).GetNormalized();

	Vec2 bottomLeft_UV	= Vec2( UVs.m_mins.x, UVs.m_mins.y );
	Vec2 bottomRight_UV = Vec2( UVs.m_maxs.x, UVs.m_mins.y );
	Vec2 topRight_UV	= Vec2( UVs.m_maxs.x, UVs.m_maxs.y );
	Vec2 topLeft_UV		= Vec2( UVs.m_mins.x, UVs.m_maxs.y );

	verts.push_back( Vertex_PCUTBN(  bottomLeft, color,  bottomLeft_UV, Vec3::ZERO, Vec3::ZERO, normal ) );
	verts.push_back( Vertex_PCUTBN( bottomRight, color, bottomRight_UV, Vec3::ZERO, Vec3::ZERO, normal ) );
	verts.push_back( Vertex_PCUTBN(    topRight, color,    topRight_UV, Vec3::ZERO, Vec3::ZERO, normal ) );
	verts.push_back( Vertex_PCUTBN(     topLeft, color,     topLeft_UV, Vec3::ZERO, Vec3::ZERO, normal ) );


	indexes.push_back( startIndex + 0 );
	indexes.push_back( startIndex + 1 );
	indexes.push_back( startIndex + 2 );

	indexes.push_back( startIndex + 0 );
	indexes.push_back( startIndex + 2 );
	indexes.push_back( startIndex + 3 );
}


//----------------------------------------------------------------------------------------------------------------------
void AddVertsForRoundedQuad3D( std::vector<Vertex_PCUTBN>& verts, Vec3 const& bottomLeft, Vec3 const& bottomRight, Vec3 const& topRight, Vec3 const& topLeft, Rgba8 const& color, AABB2 const& UVs )
{
	// Calculate forward
	Vec3 tangent	= bottomRight - bottomLeft;
	Vec3 bitangent	= topLeft - bottomLeft;												// bi-tangent
	Vec3 normal		= (CrossProduct3D( tangent, bitangent )).GetNormalized();

	// TopMiddle
	Vec3 topMiddleLeft	 = ( topRight - topLeft ) / 2.0f;
	Vec3 topMiddle		 = topLeft + topMiddleLeft;
	// BottomMiddle
	Vec3 bottomMiddle	 = bottomLeft + ( tangent / 2 );
	// Left
	Vec3 leftNormalized	 = ( bottomLeft - bottomRight ).GetNormalized();
	// Right
	Vec3 rightNormalized = ( bottomRight - bottomLeft ).GetNormalized();

	// Bottom UVs
	Vec2 bottomLeft_UV	 = Vec2( UVs.m_mins.x, UVs.m_mins.y );
	Vec2 bottomRight_UV  = Vec2( UVs.m_maxs.x, UVs.m_mins.y );
	Vec2 bottomMiddle_UV = UVs.m_mins + ( (bottomRight_UV -  bottomLeft_UV) / 2.0f );
	
	// Top UVs
	Vec2 topRight_UV	 = Vec2( UVs.m_maxs.x, UVs.m_maxs.y );
	Vec2 TopLeft_UV		 = Vec2( UVs.m_mins.x, UVs.m_maxs.y );
	Vec2 topMiddle_UV	 = TopLeft_UV + ( (topRight_UV - TopLeft_UV) / 2.0f );

	// Triangle 1	// 4 triangles from  left to right
	verts.push_back( Vertex_PCUTBN(   bottomLeft, color,   bottomLeft_UV, Vec3::ZERO, Vec3::ZERO, leftNormalized ) );		// BL
	verts.push_back( Vertex_PCUTBN( bottomMiddle, color, bottomMiddle_UV, Vec3::ZERO, Vec3::ZERO, 		  normal ) );		// BM
	verts.push_back( Vertex_PCUTBN(	   topMiddle, color,    topMiddle_UV, Vec3::ZERO, Vec3::ZERO, 		  normal ) );		// TM

	// Triangle 2	
	verts.push_back( Vertex_PCUTBN( bottomLeft, color, bottomLeft_UV, Vec3::ZERO, Vec3::ZERO, leftNormalized ) );			// BL 
	verts.push_back( Vertex_PCUTBN(  topMiddle,	color,  topMiddle_UV, Vec3::ZERO, Vec3::ZERO, 		  normal ) );			// TM
	verts.push_back( Vertex_PCUTBN(	   topLeft, color,    TopLeft_UV, Vec3::ZERO, Vec3::ZERO, 		  normal ) );			// TL

	// Triangle 3
	verts.push_back( Vertex_PCUTBN( bottomMiddle, color, bottomMiddle_UV, Vec3::ZERO, Vec3::ZERO, 		   normal ) );		// BM
	verts.push_back( Vertex_PCUTBN(  bottomRight, color,  bottomRight_UV, Vec3::ZERO, Vec3::ZERO, rightNormalized ) );		// BR
	verts.push_back( Vertex_PCUTBN(		topRight, color,	 topRight_UV, Vec3::ZERO, Vec3::ZERO, rightNormalized ) );		// TR
	
	// Triangle 4
	verts.push_back( Vertex_PCUTBN( bottomMiddle, color, bottomMiddle_UV, Vec3::ZERO, Vec3::ZERO,		   normal ) );		// BM
	verts.push_back( Vertex_PCUTBN(		topRight, color,	 topRight_UV, Vec3::ZERO, Vec3::ZERO, rightNormalized ) );		// TR
	verts.push_back( Vertex_PCUTBN(	   topMiddle, color,	topMiddle_UV, Vec3::ZERO, Vec3::ZERO,		   normal ) );		// TM
}


//----------------------------------------------------------------------------------------------------------------------
void AddVertsForQuad3D( std::vector<Vertex_PCU>& verts, Vec3 const& bottomLeft, Vec3 const& bottomRight, Vec3 const& topLeft, Vec3 const& topRight, Rgba8 const& color, Vec2 UV_bottomLeft, Vec2 UV_bottomRight, Vec2 UV_TopLeft, Vec2 UV_TopRight )
{
	UNUSED( verts			);
	UNUSED( bottomLeft 		);
	UNUSED( bottomRight 	);
	UNUSED( topLeft			);
	UNUSED( topRight		);
	UNUSED( color			);
	UNUSED( UV_bottomLeft	);
	UNUSED( UV_bottomRight	);
	UNUSED( UV_TopLeft		);
	UNUSED( UV_TopRight		);
}


//----------------------------------------------------------------------------------------------------------------------
void AddVertsForQuad3D( std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, Vec3 const& bottomLeft, Vec3 const& bottomRight, Vec3 const& topRight, Vec3 const& topLeft, 
						Rgba8 const& color, AABB2 const& UVs )
{
	Vec2 bottomLeft_UV	= Vec2( UVs.m_mins.x, UVs.m_mins.y );
	Vec2 bottomRight_UV = Vec2( UVs.m_maxs.x, UVs.m_mins.y );
	Vec2 TopRight_UV	= Vec2( UVs.m_maxs.x, UVs.m_maxs.y );
	Vec2 TopLeft_UV		= Vec2( UVs.m_mins.x, UVs.m_maxs.y );
  
	verts.push_back( Vertex_PCU(  bottomLeft, color, bottomLeft_UV  ) );
	verts.push_back( Vertex_PCU( bottomRight, color, bottomRight_UV ) );
	verts.push_back( Vertex_PCU(    topRight, color, TopRight_UV	) );
	verts.push_back( Vertex_PCU(     topLeft, color, TopLeft_UV	    ) );

	int startIndex = static_cast<int>( verts.size() );

	indexes.push_back( startIndex + 0 );
	indexes.push_back( startIndex + 1 );
	indexes.push_back( startIndex + 2 );

	indexes.push_back( startIndex + 0 );
	indexes.push_back( startIndex + 2 );
	indexes.push_back( startIndex + 3 );
}


//----------------------------------------------------------------------------------------------------------------------
void AddvertsforCube3D( std::vector<Vertex_PCU>& verts, Vec3 const& center, float radius, Rgba8 const& color, AABB2 const& UVs )
{
	AABB3 bounds( center - Vec3( radius, radius, radius), center + Vec3( radius, radius, radius) );
	AddVertsForAABB3D( verts, bounds, color, UVs );
}


//----------------------------------------------------------------------------------------------------------------------
void AddvertsforCube3D( std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, AABB3 const& bounds, Rgba8 const& color, AABB2 const& UVs )
{
	Vec3 ESB = Vec3( bounds.m_maxs.x, bounds.m_mins.y, bounds.m_mins.z );		// ESB
	Vec3 ENB = Vec3( bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_mins.z );		// ENB	
	Vec3 ENT = Vec3( bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_maxs.z );		// ENT	
	Vec3 EST = Vec3( bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z );		// EST

	Vec3 WNB = Vec3( bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z );		// WNB	
	Vec3 WSB = Vec3( bounds.m_mins.x, bounds.m_mins.y, bounds.m_mins.z );		// WSB	
	Vec3 WST = Vec3( bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z );		// WST	
	Vec3 WNT = Vec3( bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z );		// WNT	

	AddVertsForQuad3D( verts, indexes, ESB, ENB, ENT, EST, color, UVs );		// East
	AddVertsForQuad3D( verts, indexes, WNB, WSB, WST, WNT, color, UVs );		// West
	AddVertsForQuad3D( verts, indexes, ENB, WNB, WNT, ENT, color, UVs );		// North
	AddVertsForQuad3D( verts, indexes, WSB, ESB, EST, WST, color, UVs );		// South
	AddVertsForQuad3D( verts, indexes, WST, EST, ENT, WNT, color, UVs );		// Sky
	AddVertsForQuad3D( verts, indexes, WNB, ENB, ESB, WSB, color, UVs );		// Ground
}


//----------------------------------------------------------------------------------------------------------------------
void AddVertsForAABB3D( std::vector<Vertex_PCU>& verts, AABB3 const& bounds, Rgba8 const& color, AABB2 const& UVs )
{
	Vec3 ESB = Vec3( bounds.m_maxs.x, bounds.m_mins.y, bounds.m_mins.z );		// ESB
	Vec3 ENB = Vec3( bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_mins.z );		// ENB	
	Vec3 ENT = Vec3( bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_maxs.z );		// ENT	
	Vec3 EST = Vec3( bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z );		// EST

	Vec3 WNB = Vec3( bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z );		// WNB	
	Vec3 WSB = Vec3( bounds.m_mins.x, bounds.m_mins.y, bounds.m_mins.z );		// WSB	
	Vec3 WST = Vec3( bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z );		// WST	
	Vec3 WNT = Vec3( bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z );		// WNT	

	AddVertsForQuad3D( verts, ESB, ENB, ENT, EST, color, UVs );		// East
	AddVertsForQuad3D( verts, WNB, WSB, WST, WNT, color, UVs );		// West
	AddVertsForQuad3D( verts, ENB, WNB, WNT, ENT, color, UVs );		// North
	AddVertsForQuad3D( verts, WSB, ESB, EST, WST, color, UVs );		// South
	AddVertsForQuad3D( verts, WST, EST, ENT, WNT, color, UVs );		// Sky
	AddVertsForQuad3D( verts, WNB, ENB, ESB, WSB, color, UVs );		// Ground
}


//----------------------------------------------------------------------------------------------------------------------
void AddVertsForAABBWireframe3D( std::vector<Vertex_PCU>& verts, AABB3 const& bounds, float lineThickness, Rgba8 const& tint )
{
	UNUSED( verts			);
	UNUSED( bounds			);
	UNUSED( lineThickness 	);
	UNUSED( tint			);
}


//----------------------------------------------------------------------------------------------------------------------
void AddVertsForCylinder3D( std::vector<Vertex_PCU>& verts, Vec3 const& start, Vec3 const& end, float radius, Rgba8 const& color, AABB2 const& UVs, int numSlices )
{
	//----------------------------------------------------------------------------------------------------------------------
	// Create zCylinder
	std::vector<Vertex_PCU> localVerts;
	Vec3 dispSE			= end - start;	
	Vec2 centerXY		= Vec2(0.f, 0.f);
	FloatRange minMaxZ	= FloatRange(0.0f, dispSE.GetLength());
	AddVertsForCylinderZ3D(localVerts, centerXY, minMaxZ, radius, (float)numSlices, color, UVs );

	//----------------------------------------------------------------------------------------------------------------------
	// Calculate I,J,K basis for zCylinder
	Mat44 matrix;
	Vec3 zSkyward	= Vec3( 0.0f, 0.0f, 1.0f );
	Vec3 iBasis		= dispSE.GetNormalized();
	Vec3 jBasis		= CrossProduct3D( zSkyward, iBasis );

	// Set jBasis to y-axis if iBasis is parallel to zBasis
	if ( jBasis == Vec3(0.0f, 0.0f, 0.0f) )
	{
		jBasis = Vec3( 0.0f, 1.0f, 0.0f );
	}
	jBasis		= jBasis.GetNormalized();
	Vec3 kBasis = CrossProduct3D( jBasis, iBasis );

	// Transform zCylinder basis in world
	matrix.SetIJK3D( kBasis, jBasis, iBasis );
	matrix.SetTranslation3D(start);
	TransformVertexArray3D( localVerts, matrix );

	// Storing transformed local verts inside main verts (passed in through the parameter)	
	for ( int i = 0; i < localVerts.size(); i++ )
	{
		verts.push_back( localVerts[i] );
	}
}


//----------------------------------------------------------------------------------------------------------------------
void AddVertsForCylinderZ3D( std::vector<Vertex_PCU>& verts, Vec2 const& centerXY, FloatRange const& minMaxZ, float radius, float numSlices, Rgba8 const& tint, AABB2 const& UVs)
{	
	float yawDegreesPerSlice	= ( 360.0f / numSlices );
	
	for ( int currentSlice = 0; currentSlice < numSlices; currentSlice++ )
	{
		float currentYawDegrees = static_cast<float>( currentSlice ) * yawDegreesPerSlice;
		float nextYawDegrees	= currentYawDegrees + yawDegreesPerSlice;

		Vec2 currentPoint		= centerXY + Vec2::MakeFromPolarDegrees( currentYawDegrees,	radius );
		Vec2 nextPoint			= centerXY + Vec2::MakeFromPolarDegrees(	nextYawDegrees,	radius );

		Vec3 bottomLeftMinZ		= Vec3(	 currentPoint.x,  currentPoint.y, minMaxZ.m_min );		
		Vec3 bottomRightMinZ	= Vec3(		nextPoint.x,	 nextPoint.y, minMaxZ.m_min );	
		Vec3 topLeftMaxZ		= Vec3(	 currentPoint.x,  currentPoint.y, minMaxZ.m_max );	
		Vec3 topRightMaxZ		= Vec3(	    nextPoint.x,	 nextPoint.y, minMaxZ.m_max );		

		// Calculate wall UVs
		Vec2 UV_bottomLeft;
		Vec2 UV_topRight;

		UV_bottomLeft.x = RangeMapClamped( currentYawDegrees, 0.0f, 360.0f, UVs.m_mins.x, UVs.m_maxs.x );
		UV_bottomLeft.y	= UVs.m_mins.y;
		UV_topRight.x	= RangeMapClamped(	  nextYawDegrees, 0.0f, 360.0f, UVs.m_mins.x, UVs.m_maxs.x );
		UV_topRight.y	= UVs.m_maxs.y;

		AddVertsForQuad3D( verts, bottomLeftMinZ, bottomRightMinZ, topRightMaxZ, topLeftMaxZ, tint, AABB2(UV_bottomLeft, UV_topRight) );

		//----------------------------------------------------------------------------------------------------------------------
		// Top and Bottom UV variables
		Vec2 UV_center = Vec2( 0.5f, 0.5f );
		Vec2 UV_currentOrientation;
		Vec2 UV_nextOrientation;
		Vec2 UV_currentOrientationFlipped;
		Vec2 UV_nextOrientationFlipped;

		// Calculate UVs Top face 
		UV_currentOrientation.x			= RangeMapClamped( CosDegrees(currentYawDegrees), -1.0f, 1.0f, UVs.m_mins.x, UVs.m_maxs.x );
		UV_currentOrientation.y			= RangeMapClamped( SinDegrees(currentYawDegrees), -1.0f, 1.0f, UVs.m_mins.y, UVs.m_maxs.y );
		UV_nextOrientation.x			= RangeMapClamped(	  CosDegrees(nextYawDegrees), -1.0f, 1.0f, UVs.m_mins.x, UVs.m_maxs.x );
		UV_nextOrientation.y			= RangeMapClamped(	  SinDegrees(nextYawDegrees), -1.0f, 1.0f, UVs.m_mins.y, UVs.m_maxs.y );

		// Calculate UVs Bottom face 
		UV_currentOrientationFlipped.x  = UV_currentOrientation.x;
		UV_nextOrientationFlipped.x		= UV_nextOrientation.x;

		UV_currentOrientationFlipped.y	= RangeMapClamped( SinDegrees(currentYawDegrees), -1.0f,  1.0f, UVs.m_maxs.y, UVs.m_mins.y );
		UV_nextOrientationFlipped.y		= RangeMapClamped(	  SinDegrees(nextYawDegrees), -1.0f,  1.0f, UVs.m_maxs.y, UVs.m_mins.y );

		// Pushback verts
		// Top face 
		verts.push_back( Vertex_PCU(	 Vec3(centerXY.x, centerXY.y, minMaxZ.m_max), tint, UV_center			  ) );
		verts.push_back( Vertex_PCU(									 topLeftMaxZ, tint, UV_currentOrientation ) );
		verts.push_back( Vertex_PCU(									topRightMaxZ, tint, UV_nextOrientation	  ) );

		// Bottom face 
		verts.push_back( Vertex_PCU(	 Vec3(centerXY.x, centerXY.y, minMaxZ.m_min), tint, UV_center					 ) );
		verts.push_back( Vertex_PCU(								 bottomRightMinZ, tint, UV_nextOrientationFlipped	 ) );
		verts.push_back( Vertex_PCU(								  bottomLeftMinZ, tint, UV_currentOrientationFlipped ) );
	}
}


//----------------------------------------------------------------------------------------------------------------------
void AddVertsForCylinderZWireframe3D( std::vector<Vertex_PCU>& verts, Vec2 const& centerXY, FloatRange const& minMaxZ, float radius, float numSlices, float lineThickness, Rgba8 const& tint )
{
	UNUSED( verts			);
	UNUSED( centerXY		);
	UNUSED( minMaxZ			);
	UNUSED( radius			);
	UNUSED( numSlices		);
	UNUSED( lineThickness	);
	UNUSED( tint			);
}


//----------------------------------------------------------------------------------------------------------------------
void AddVertsForSphere3D( std::vector<Vertex_PCU>& verts, Vec3 const& center, float radius, float numSlices, float numStacks, Rgba8 const& color, AABB2 const& UVs )
{
	float yawDegreesPerSlice	= ( 360.0f / numSlices );
	float pitchDegreesPerStack	= ( 180.0f / numStacks );

	for ( int currentSlice = 0; currentSlice < numSlices; currentSlice++ )
	{
		float currentYawDegrees = static_cast<float>(currentSlice) * yawDegreesPerSlice;

		for ( int currentStack = 0; currentStack < numStacks; currentStack++ )		
		{
			float currentPitchDegrees = -90.0f + (currentStack * pitchDegreesPerStack);	// -90.0f offsets pitch degree to start from "top" of sphere instead of forward

			//----------------------------------------------------------------------------------------------------------------------
			// Verts			
			Vec3 vertsBottomLeft  = Vec3( 0.0f, 0.0f, 0.0f );
			Vec3 vertsBottomRight = Vec3( 0.0f, 0.0f, 0.0f );
			Vec3 vertsTopLeft	  = Vec3( 0.0f, 0.0f, 0.0f );
			Vec3 vertsTopRight	  = Vec3( 0.0f, 0.0f, 0.0f );

			vertsTopLeft	 = center + Vec3::MakeFromSphericalDegrees(						 currentYawDegrees,						   currentPitchDegrees, radius );		// add verts for current as TopLeft										// Current
			vertsTopRight	 = center + Vec3::MakeFromSphericalDegrees( currentYawDegrees + yawDegreesPerSlice,						   currentPitchDegrees, radius );		// then add longitudeOffset to current to add verts for TopRight		// Yaw to the right
			vertsBottomLeft	 = center + Vec3::MakeFromSphericalDegrees(	                     currentYawDegrees, currentPitchDegrees + pitchDegreesPerStack, radius );		// then add latitudeOffset to TopRight to add verts for BottomRight		// Yaw to the right then Pitch down (positive)
			vertsBottomRight = center + Vec3::MakeFromSphericalDegrees( currentYawDegrees + yawDegreesPerSlice, currentPitchDegrees + pitchDegreesPerStack, radius );		// then add latitudeOffset to TopLeft  to add verts for BottomLeft		// Pitch down only from Current

			//----------------------------------------------------------------------------------------------------------------------
			// UVs
			Vec2 UV_BottomLeft;
			Vec2 UV_TopRight;

			UV_BottomLeft.x = RangeMapClamped(							currentYawDegrees,   0.0f, 360.0f, UVs.m_mins.x, UVs.m_maxs.x );
			UV_BottomLeft.y	= RangeMapClamped( currentPitchDegrees + pitchDegreesPerStack, -90.0f,  90.0f, UVs.m_maxs.y, UVs.m_mins.y );
			UV_TopRight.x	= RangeMapClamped(	   currentYawDegrees + yawDegreesPerSlice,   0.0f, 360.0f, UVs.m_mins.x, UVs.m_maxs.x );
			UV_TopRight.y	= RangeMapClamped(		  				  currentPitchDegrees, -90.0f,  90.0f, UVs.m_maxs.y, UVs.m_mins.y );
																									 
			//----------------------------------------------------------------------------------------------------------------------
			// Pushback all verts
			AddVertsForQuad3D( verts, vertsBottomLeft, vertsBottomRight, vertsTopRight, vertsTopLeft, color, AABB2(UV_BottomLeft, UV_TopRight) );
		}
	}
}


//----------------------------------------------------------------------------------------------------------------------
void AddVertsForSphere3D( std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, Vec3 const& center, float radius, float numSlices, float numStacks, Rgba8 const& color, AABB2 const& UVs )
{
	int vertSize = int( verts.size() );

	//----------------------------------------------------------------------------------------------------------------------
	// 1. Add all unique vertex positions
	//----------------------------------------------------------------------------------------------------------------------
	// Add bottom vert (bottom of sphere)
	Vec3 bottomVert	  = center + Vec3::MakeFromSphericalDegrees( 0.0f, 90.0f, radius );
	Vec3 bottomNormal = ( bottomVert - center ).GetNormalized();
	verts.push_back( Vertex_PCUTBN( bottomVert, color, Vec2::ZERO, Vec3::ZERO, Vec3::ZERO, bottomNormal ) );

	// Loop and add all unique verts for stacks and slices EXECEPT top vert (top of sphere)
	float yawDegreesPerSlice	= ( 360.0f / numSlices );
	float pitchDegreesPerStack	= ( 180.0f / numStacks );
	for ( int currentSlice = 0; currentSlice < numSlices; currentSlice++ )
	{
		float currentYawDegrees = static_cast<float>(currentSlice) * yawDegreesPerSlice;
		for ( int currentStack = 1; currentStack < numStacks; currentStack++ )		
		{
			float currentPitchDegrees = 90.0f - ( currentStack * pitchDegreesPerStack );	
			//----------------------------------------------------------------------------------------------------------------------
			// Verts			
			Vec3 vertsBottomLeft  = center + Vec3::MakeFromSphericalDegrees( currentYawDegrees, currentPitchDegrees, radius );		// then add latitudeOffset to TopRight to add verts for BottomRight		// Yaw to the right then Pitch down (positive)

			//----------------------------------------------------------------------------------------------------------------------
			// UVs
			Vec2 UV_BottomLeft;
			UV_BottomLeft.x = RangeMapClamped(	 currentYawDegrees,   0.0f, 360.0f, UVs.m_mins.x, UVs.m_maxs.x );
			UV_BottomLeft.y	= RangeMapClamped( currentPitchDegrees, -90.0f,  90.0f, UVs.m_maxs.y, UVs.m_mins.y );

			// Normal
			Vec3 normal = ( vertsBottomLeft - center ).GetNormalized();

			//----------------------------------------------------------------------------------------------------------------------
			// Pushback verts at currentYaw, with increasing pitch (increasing stack, same slice)
			verts.push_back( Vertex_PCUTBN( vertsBottomLeft, color, UV_BottomLeft, Vec3::ZERO, Vec3::ZERO, normal ) );
		}
	}
	// Add top vert (top of sphere)
	Vec3 topVert	= center + Vec3::MakeFromSphericalDegrees( 0.0f, -90.0f, radius );
	Vec3 topNormal	= ( topVert - center ).GetNormalized();
	verts.push_back( Vertex_PCUTBN( topVert, color, Vec2( 0.0f, 1.0f ), Vec3::ZERO, Vec3::ZERO, topNormal ) );

	//----------------------------------------------------------------------------------------------------------------------
	// 2. Add index List to form a sphere using unique vertex positions
	//----------------------------------------------------------------------------------------------------------------------
	int vertsPerStack = int( numStacks - 1 );		// Excluding bottom and top verts, only quads
	//----------------------------------------------------------------------------------------------------------------------
	// 2a. Tri at bottom of sphere
	//----------------------------------------------------------------------------------------------------------------------
	unsigned int BL = vertSize;
	for ( int currentSlice = 0; currentSlice < numSlices; currentSlice++ )
	{
		if ( currentSlice == ( numSlices - 1 ) )
		{
			// Special case for the final tri to reconnect with the starting vert
			unsigned int TL = vertSize + ( currentSlice * vertsPerStack ) + 1;
			unsigned int TR = BL + 1;
			// Add Tri for bottom of sphere
			indexes.push_back( BL );
			indexes.push_back( TR );
			indexes.push_back( TL );
		}
		else
		{
			unsigned int TL = vertSize + ( currentSlice * vertsPerStack ) + 1;
			unsigned int TR = TL + vertsPerStack;
			// Add Tri for bottom of sphere
			indexes.push_back( BL );
			indexes.push_back( TR );
			indexes.push_back( TL );
		}
	}
	//----------------------------------------------------------------------------------------------------------------------
	// 2b. Tri at top of sphere
	//----------------------------------------------------------------------------------------------------------------------
	unsigned int topVertIndex = int( verts.size() - 1 );
	for ( int currentSlice = 0; currentSlice < numSlices; currentSlice++ )
	{
		if ( currentSlice == ( numSlices - 1 ) )
		{
			// Special case for the final tri to reconnect with the starting vert
				     BL = vertSize + ( currentSlice * vertsPerStack ) + vertsPerStack;
			unsigned BR = vertSize + vertsPerStack;
			// Add Tri for bottom of sphere
			indexes.push_back( topVertIndex );
			indexes.push_back( BL );
			indexes.push_back( BR );
		}
		else
		{
						 BL = vertSize + ( currentSlice * vertsPerStack ) + vertsPerStack;
			unsigned int BR = BL + vertsPerStack;
			// Add Tri for bottom of sphere
			indexes.push_back( topVertIndex );
			indexes.push_back( BL );
			indexes.push_back( BR );
		}
	}
	//----------------------------------------------------------------------------------------------------------------------
	// 2c. Quads for faces of sphere
	//----------------------------------------------------------------------------------------------------------------------
	for ( int currentSlice = 0; currentSlice < numSlices; currentSlice++ )
	{
		for ( int currentStack = 0; currentStack < ( numStacks - 2 ); currentStack++ )
		{
			if ( currentSlice == ( numSlices - 1 ) )
			{
				// Special case for the final tri to reconnect with the starting vert
							 BL = vertSize + ( currentSlice * vertsPerStack ) + 1 + currentStack;
				unsigned int BR = vertSize + 1 + currentStack;
				unsigned int TL = BL + 1;
				unsigned int TR = BR + 1;

				// Add quads as I "move" up the stack
				indexes.push_back( BL );	// BL
				indexes.push_back( BR );	// BR
				indexes.push_back( TR );	// TR

				indexes.push_back( BL );	// BL
				indexes.push_back( TR );	// TR
				indexes.push_back( TL );	// TL
			}
			else
			{
							 BL = vertSize + ( currentSlice * vertsPerStack ) + 1 + currentStack;
				unsigned int BR = BL + vertsPerStack;
				unsigned int TL = BL + 1;
				unsigned int TR = BR + 1;

				// Add quads as I "move" up the stack
				indexes.push_back( BL );	// BL
				indexes.push_back( BR );	// BR
				indexes.push_back( TR );	// TR

				indexes.push_back( BL );	// BL
				indexes.push_back( TR );	// TR
				indexes.push_back( TL );	// TL
			}
		}
	}
}


//----------------------------------------------------------------------------------------------------------------------
void AddVertsForUVSphereZWireframe3D( std::vector<Vertex_PCU>& verts, Vec3 const& center, float radius, float numSlices, float numStacks, float lineThickness, Rgba8 const& tint )
{
	UNUSED( verts			);
	UNUSED(	center			);
	UNUSED(	radius			);
	UNUSED(	numSlices		);
	UNUSED(	numStacks		);
	UNUSED(	lineThickness	);
	UNUSED(	tint			);
}


//----------------------------------------------------------------------------------------------------------------------
void AddVertsForConeZ3D( std::vector<Vertex_PCU>& verts, Vec2 const& start, FloatRange minMaxZ, float radius, Rgba8 const& color, AABB2 const& UVs, int numSlices )
{
	std::vector<Vertex_PCU> localVerts;
	float yawDegreesPerSlice = ( 360.0f / numSlices );
	Vec3 coneTip		= Vec3( start.x, start.y, minMaxZ.m_max );
	Vec3 coneBaseCenter = Vec3( start.x, start.y, minMaxZ.m_min );

	for ( int currentSlice = 0; currentSlice< numSlices; currentSlice++ )
	{
		float currentYawDegrees = static_cast<float>( currentSlice ) * yawDegreesPerSlice;
		float nextYawDegrees	= currentYawDegrees + yawDegreesPerSlice;
		
		// V2 is Vec2
		Vec2 disp1V2				= Vec2::MakeFromPolarDegrees( currentYawDegrees, radius );		
		Vec2 disp2V2				= Vec2::MakeFromPolarDegrees(	 nextYawDegrees, radius );		

		// V3 is Vec3
		Vec3 disp1_V3 = Vec3( disp1V2.x, disp1V2.y, minMaxZ.m_min );
		Vec3 disp2_V3 = Vec3( disp2V2.x, disp2V2.y, minMaxZ.m_min );

		// Disc
		localVerts.push_back( Vertex_PCU( coneBaseCenter + disp2_V3, color, UVs.m_mins ) );
		localVerts.push_back( Vertex_PCU( coneBaseCenter + disp1_V3, color, UVs.m_mins ) );
		localVerts.push_back( Vertex_PCU(		 	 coneBaseCenter, color, UVs.m_mins ) );

		// Funnel 
		localVerts.push_back( Vertex_PCU(				    coneTip, color, UVs.m_mins ) );
		localVerts.push_back( Vertex_PCU( coneBaseCenter + disp1_V3, color, UVs.m_mins ) );
		localVerts.push_back( Vertex_PCU( coneBaseCenter + disp2_V3, color, UVs.m_mins ) ); 
	}

	// Storing transformed local verts inside main verts (passed in through the parameter)
	for ( int i = 0; i < localVerts.size(); i++ )
	{
		verts.push_back( localVerts[i] );
	}
}


//----------------------------------------------------------------------------------------------------------------------
void AddVertsForCone3D( std::vector<Vertex_PCU>& verts, Vec3 const& start, Vec3 const& end, float radius, Rgba8 const& color, AABB2 const& UVs, int numSlices )
{
	//----------------------------------------------------------------------------------------------------------------------
	// Create zCone
	std::vector<Vertex_PCU> localVerts;
	Vec3  coneForward  = ( end - start ).GetNormalized();
	float coneLength   = ( end - start ).GetLength();
	FloatRange minMaxZ = FloatRange( 0.0f, coneLength );
	AddVertsForConeZ3D( localVerts, Vec2::ZERO, minMaxZ, radius, color, UVs, numSlices );
	
	//----------------------------------------------------------------------------------------------------------------------
	// Calculate I,J,K basis for zCone							// Might have to check if values = nan later if divided by inf // Use dotProduct
	Mat44 coneMatrix;
	Vec3 zSkyward = Vec3( 0.0f, 0.0f, 1.0f );

	Vec3 coneLeft = CrossProduct3D( zSkyward, coneForward );
	if ( coneLeft == Vec3( 0.0f, 0.0f, 0.0f ) )					// Set jBasis to y-axis(world left) if coneForward is parallel to z-axis(world up/sky)
	{
		coneLeft = Vec3( 0.0f, 1.0f, 0.0f );
	}
	coneLeft.Normalize();
	Vec3 coneUp = CrossProduct3D( coneForward, coneLeft );		// Calculate coneUp from coneForward and coneLeft
	coneUp.Normalize();

	//----------------------------------------------------------------------------------------------------------------------
	// Transform zCone basis into world
	coneMatrix.SetIJK3D( -coneUp, coneLeft, coneForward );
	coneMatrix.SetTranslation3D( start );
	TransformVertexArray3D( localVerts, coneMatrix );
	
	// Storing transformed local verts inside main verts (passed in through the parameter)
	for ( int i = 0; i < localVerts.size(); i++ )
	{
		verts.push_back( localVerts[i] );
	}
}


//----------------------------------------------------------------------------------------------------------------------
void AddVertsForArrow3D( std::vector<Vertex_PCU>& verts, Vec3 const& start, Vec3 const& end, float radius, Rgba8 const& color, AABB2 const& UVs  )
 {
	std::vector<Vertex_PCU> cylinderLocalVerts;
	std::vector<Vertex_PCU> coneLocalVerts;

	// Resize Arrow length
	Vec3 arrowLength = end - start;
	float arrowScale = 0.8f;

	// Set Cylinder variables
	Vec3 cylinderEnd = start + (arrowLength * arrowScale); 

	// Set Cone variables
	Vec3 coneStart	 = cylinderEnd;
	Vec3 coneEnd	 = end;
	float coneScale	 = 2.5f;
	float coneRadius = radius * coneScale;

	// Adding verts for object then storing transformed local verts inside main verts (passed through parameter)
 	AddVertsForCylinder3D( cylinderLocalVerts, start, cylinderEnd, radius, color, UVs );
	for ( int i = 0; i < cylinderLocalVerts.size(); i++ )
	{
		verts.push_back( cylinderLocalVerts[i] );
	}
 	
	AddVertsForCone3D( coneLocalVerts, coneStart, coneEnd, coneRadius, color, UVs );
	for ( int i = 0; i < coneLocalVerts.size(); i++ )
	{
		verts.push_back( coneLocalVerts[i] );
	}
}


//----------------------------------------------------------------------------------------------------------------------
AABB2 GetVertexBounds2D( std::vector<Vertex_PCU> const& verts )
{
	UNUSED( verts );
	return AABB2();
}


//----------------------------------------------------------------------------------------------------------------------
// Each tile is a uniform square
// tileSize is the width AND height of the tile
//----------------------------------------------------------------------------------------------------------------------
void AddVertsForPlane( std::vector<Vertex_PCU>& vertList, std::vector<unsigned int>& indexList, Vec3 const& planeOriginPos_BL, int tileSize, int numTilesX, int numTilesY, Rgba8 const& color )
{
	int startVert = int( vertList.size() );

	// Add all verts into vertList
	for ( int tileY = 0; tileY <= numTilesY; tileY++ )
	{
		for ( int tileX = 0; tileX < numTilesX; tileX++ )
		{
			// Add only BL vert for every tile
			float yOffset		= float( tileY * tileSize );
			float xOffset		= float( tileX * tileSize );
			Vec3 currentTile_BL = planeOriginPos_BL + Vec3( xOffset, yOffset, 0.0f );

			// Compute UVs
			float u = float( tileX ) / float( numTilesX );
			float v = float( tileY ) / float( numTilesY );

			vertList.push_back( Vertex_PCU( currentTile_BL, color, Vec2( u, v ) ) );
			
			// If this is the "last" tile, add the BR vert too
			if ( tileX == (numTilesX - 1) )
			{
				Vec3 currentTile_BR = currentTile_BL + Vec3( float( tileSize ), 0.0f, 0.0f );
				vertList.push_back( Vertex_PCU( currentTile_BR, color, Vec2( 1.0f, v ) ) );
			}
		}
	}

	// Loop through vertList and create triangles using indexList
	int rowX = 1 + numTilesX;
	for ( int tileY = 0; tileY < numTilesY; tileY++ )
	{
		for ( int tileX = 0; tileX < numTilesX; tileX++ )
		{		
			int startIndex = startVert + tileX + ( tileY * rowX );

			int BL = startIndex;
			int BR = startIndex + 1;
			int TL = startIndex + rowX;
			int TR = startIndex + 1 + rowX;

			// Bottom half of tile
			indexList.push_back( BL );		
			indexList.push_back( BR );		
			indexList.push_back( TL );		

			// Top half of tile
			indexList.push_back( BR );
			indexList.push_back( TR );
			indexList.push_back( TL );
		}
	}

	/*
		// Add verts for origin tile
		float yOffset		= planeOriginPos_BL.y + tileSize;
		float xOffset		= planeOriginPos_BL.x + tileSize;
		Vec3 currentTile_BL = planeOriginPos_BL;
		Vec3 currentTile_BR = currentTile_BL + Vec3( xOffset,	 0.0f, 0.0f );
		Vec3 currentTile_TR = currentTile_BR + Vec3( xOffset, yOffset, 0.0f );
		Vec3 currentTile_TL = currentTile_BL + Vec3(	0.0f, yOffset, 0.0f );
		vertList.push_back( Vertex_PCU( currentTile_BL, color, Vec2( 0.0f, 1.0f ) ) );
		vertList.push_back( Vertex_PCU( currentTile_BR, color, Vec2( 0.0f, 1.0f ) ) );
		vertList.push_back( Vertex_PCU( currentTile_TL, color, Vec2( 0.0f, 1.0f ) ) );
		vertList.push_back( Vertex_PCU( currentTile_TR, color, Vec2( 0.0f, 1.0f ) ) );

		// Add all verts into vertList
		for ( int tileY = 0; tileY < numTilesY; tileY++ )
		{
			for ( int tileX = 0; tileX < numTilesX; tileX++ )
			{
				yOffset				= planeOriginPos_BL.y + ( tileY * tileSize );
				xOffset				= planeOriginPos_BL.x + ( tileX * tileSize );
				Vec3 currentTile_BR = currentTile_BL	  + Vec3( xOffset, yOffset, 0.0f );	
				// currentTile_BR
				vertList.push_back( Vertex_PCU( currentTile_BR, color, Vec2( 0.0f, 1.0f ) ) );
			}
		}
	*/
}


//----------------------------------------------------------------------------------------------------------------------
void AddVertsForTriangle( std::vector<Vertex_PCU>& vertList, Vec3 const& position_BL, float length, Rgba8 const& color )
{
	Vec3 BR = Vec3( position_BL.x + length,	position_BL.y,			position_BL.z );
	Vec3 TL = Vec3( position_BL.x,			position_BL.y + length, position_BL.z );
	vertList.push_back( Vertex_PCU( position_BL, color, Vec2( 0.0f, 0.0f ) ) );
	vertList.push_back( Vertex_PCU(			 BR, color, Vec2( 1.0f, 0.0f ) ) );
	vertList.push_back( Vertex_PCU(			 TL, color, Vec2( 0.0f, 1.0f ) ) );
}


//----------------------------------------------------------------------------------------------------------------------
// This arc lies on the XY plane, with Z up
//----------------------------------------------------------------------------------------------------------------------
void AddVertsForArc3D_Z_Up( std::vector<Vertex_PCU>& verts, FloatRange const& floatRange, float length, Rgba8 const& color, Vec2 const& UV )
{
	// Compute end points for min and max
	Vec3 start			= Vec3::ZERO;
	Vec3 end			= start + ( Vec3::X_FWD * length );
	Vec3 vecToRotate	= end - start;
	Vec3 min			= RotateVectorAboutArbitraryAxis( vecToRotate, Vec3::Z_UP, floatRange.m_min );
	Vec3 max			= RotateVectorAboutArbitraryAxis( vecToRotate, Vec3::Z_UP, floatRange.m_max );
	
	float maxAngle		= floatRange.m_max;
	if ( maxAngle < 0.0f )
	{
		maxAngle = 360.0f - fabsf( maxAngle );
	}
	else
	{
		maxAngle = 360.0f;
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Draw arcs min to start or Zero
	//----------------------------------------------------------------------------------------------------------------------
	float currentAngle		= floatRange.m_min;
	float degreesPerSliced  = currentAngle / 10.0f;
	// Handle edge case to avoid inf loop by adding negative numbers
	if ( degreesPerSliced < 0.0f )
	{
		degreesPerSliced = -degreesPerSliced;
		currentAngle	 = -currentAngle;
	}
	float minEnd			= 360.0f - currentAngle;
	// Don't bother looping if the angle is ZERO
	if ( minEnd != 360.0f )
	{
		for ( float i = minEnd; i < maxAngle; i += degreesPerSliced )
		{
			currentAngle	= i;
			float nextAngle	= currentAngle + degreesPerSliced;

			Vec2 currentEnd_V2	= Vec2::MakeFromPolarDegrees( currentAngle, length );
			Vec2 nextEnd_V2		= Vec2::MakeFromPolarDegrees(	 nextAngle, length );

			Vec3 currentEnd_V3	= Vec3( currentEnd_V2.x, currentEnd_V2.y, 0.0f );
			Vec3 nextEnd_V3		= Vec3(	   nextEnd_V2.x,	nextEnd_V2.y, 0.0f );

			// Draw arcs minStart to Zero
			verts.push_back( Vertex_PCU(		 start, color, UV ) );
			verts.push_back( Vertex_PCU( currentEnd_V3, color, UV ) );
			verts.push_back( Vertex_PCU(	nextEnd_V3, color, UV ) );
		}
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Draw arcs start to Max
	//----------------------------------------------------------------------------------------------------------------------
	currentAngle			= 0;
	degreesPerSliced		= floatRange.m_max / 10.0f;
	// Handle edge case to avoid inf loop by adding negative numbers
	if ( degreesPerSliced < 0.0f )
	{
		degreesPerSliced = -degreesPerSliced;
	}
	for ( float i = 0; i < floatRange.m_max; i += degreesPerSliced )
	{
		currentAngle	= i;
		float nextAngle	= currentAngle + degreesPerSliced;

		Vec2 currentEnd_V2	= Vec2::MakeFromPolarDegrees( currentAngle, length );
		Vec2 nextEnd_V2		= Vec2::MakeFromPolarDegrees(	 nextAngle, length );

		Vec3 currentEnd_V3	= Vec3( currentEnd_V2.x, currentEnd_V2.y, 0.0f );
		Vec3 nextEnd_V3		= Vec3(	   nextEnd_V2.x,	nextEnd_V2.y, 0.0f );

		// Draw arcs start or Zero to max
		verts.push_back( Vertex_PCU(		 start, color, UV ) );
		verts.push_back( Vertex_PCU( currentEnd_V3, color, UV ) );
		verts.push_back( Vertex_PCU(	nextEnd_V3, color, UV ) );
	}
}


//----------------------------------------------------------------------------------------------------------------------
void AddVertsForArc3D_Yaw( std::vector<Vertex_PCU>& verts, Mat44 const& matrix, FloatRange const& floatRange, float length, Rgba8 const& color, Vec2 const& UV )
{
	// Add verts for arc in local space
	std::vector<Vertex_PCU> localVerts;
	AddVertsForArc3D_Z_Up( localVerts, floatRange, length, color, UV );

	// Calculate I,J,K basis for arcZUp
	Mat44 localMatrix = matrix;
	// Transform arc_iBasis in world
	TransformVertexArray3D( localVerts, localMatrix );

	// Storing transformed local verts inside main verts (passed in through the parameter)	
	for ( int i = 0; i < localVerts.size(); i++ )
	{
		verts.push_back( localVerts[i] );
	}
}


//----------------------------------------------------------------------------------------------------------------------
void AddVertsForArc3D_Pitch( std::vector<Vertex_PCU>& verts, Mat44 const& matrix, FloatRange const& floatRange, float length, Rgba8 const& color, Vec2 const& UV )
{
	// Add verts for arc in local space
	std::vector<Vertex_PCU> localVerts;
	AddVertsForArc3D_Z_Up( localVerts, floatRange, length, color, UV );

	// Transform arc_iBasis in world
	Mat44 worldMatrix = matrix;
	TransformVertexArray3D( localVerts, worldMatrix );

	// Storing transformed local verts inside main verts (passed in through the parameter)	
	for ( int i = 0; i < localVerts.size(); i++ )
	{
		verts.push_back( localVerts[i] );
	}
}


//----------------------------------------------------------------------------------------------------------------------
void AddVertsForArc3D_Roll( std::vector<Vertex_PCU>& verts, Mat44 const& matrix, FloatRange const& floatRange, float length, Rgba8 const& color, Vec2 const& UV )
{
	// Add verts for arc in local space
	std::vector<Vertex_PCU> localVerts;
	AddVertsForArc3D_Z_Up( localVerts, floatRange, length, color, UV );

	// Transform arc_iBasis in world
	Mat44 worldMatrix = matrix;
	TransformVertexArray3D( localVerts, worldMatrix );

	// Storing transformed local verts inside main verts (passed in through the parameter)	
	for ( int i = 0; i < localVerts.size(); i++ )
	{
		verts.push_back( localVerts[i] );
	}
}


//----------------------------------------------------------------------------------------------------------------------
void CalculateTangents( std::vector<Vertex_PCUTBN>& vertList, std::vector<unsigned int> const& indexList )
{
	for ( int i = 0; i < indexList.size(); i += 3 )
	{
		//----------------------------------------------------------------------------------------------------------------------
		// Indexes
		int index0 = indexList[ i + 0 ];
		int index1 = indexList[ i + 1 ];
		int index2 = indexList[ i + 2 ];

		//----------------------------------------------------------------------------------------------------------------------
		// Positions
		Vec3& pos_0 = vertList[ index0 ].m_position;
		Vec3& pos_1 = vertList[ index1 ].m_position;
		Vec3& pos_2 = vertList[ index2 ].m_position;
		// Tangent
		Vec3& tan_0 = vertList[ index0 ].m_tangent;
		Vec3& tan_1 = vertList[ index1 ].m_tangent;
		Vec3& tan_2 = vertList[ index2 ].m_tangent;
		// BiNormals
		Vec3& bi_0 = vertList[ index0 ].m_biNormal;
		Vec3& bi_1 = vertList[ index1 ].m_biNormal;
		Vec3& bi_2 = vertList[ index2 ].m_biNormal;
		// UVs
		Vec2& UV_0 = vertList[ index0 ].m_uvTexCoords;
		Vec2& UV_1 = vertList[ index1 ].m_uvTexCoords;
		Vec2& UV_2 = vertList[ index2 ].m_uvTexCoords;

		//----------------------------------------------------------------------------------------------------------------------
		// Edge vectors
		Vec3 edge1 = pos_1 - pos_0;
		Vec3 edge2 = pos_2 - pos_0;
		// UV edge vectors
		float UV_edge1_X = UV_1.x - UV_0.x;
		float UV_edge2_X = UV_2.x - UV_0.x;
		float UV_edge1_Y = UV_1.y - UV_0.y;
		float UV_edge2_Y = UV_2.y - UV_0.y;

		//----------------------------------------------------------------------------------------------------------------------
		// WHAT THE FUCK!
		float r			= 1.0f / ( UV_edge1_X * UV_edge2_Y - UV_edge2_X * UV_edge1_Y );
		Vec3 tangent	= ( edge1 * UV_edge2_Y - edge2 * UV_edge1_Y ) * r;
		Vec3 biNormal	= ( edge2 * UV_edge1_X - edge1 * UV_edge2_X ) * r;

		// Set all tangents and biNormals to ZERO
		tan_0 += Vec3::ZERO;
		tan_1 += Vec3::ZERO;
		tan_2 += Vec3::ZERO;
		bi_0  += Vec3::ZERO;
		bi_1  += Vec3::ZERO;
		bi_2  += Vec3::ZERO;

		// Set tangent and biNormals to new computed versions
		tan_0 += tangent;
		tan_1 += tangent;
		tan_2 += tangent;
		bi_0  += biNormal;
		bi_1  += biNormal;
		bi_2  += biNormal;
	}

	//----------------------------------------------------------------------------------------------------------------------
	// OrthoNormalize each tangent and calculate handedness
	for ( int i = 0; i < vertList.size(); i++ )
	{
		Vec3& tangent	 = vertList[ i ].m_tangent;
		Vec3& biNormal	 = vertList[ i ].m_biNormal;
		Vec3& normal	 = vertList[ i ].m_normal;
		Mat44 TBN_Matrix = Mat44( tangent, biNormal, normal, Vec3::ZERO );
		TBN_Matrix.OrthoNormalize_XFwd_YLeft_ZUp();
		tangent			 = TBN_Matrix.GetIBasis3D();
		biNormal		 = TBN_Matrix.GetJBasis3D();
		normal			 = TBN_Matrix.GetKBasis3D();
	}
}