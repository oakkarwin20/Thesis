#pragma once

#include "Engine/Math/OBB2D.hpp"
#include "Engine/Math/OBB3D.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/AABB3.hpp"

//----------------------------------------------------------------------------------------------------------------------
enum class BillboardType
{
	NONE = -1,
	WORLD_UP_CAMERA_FACING,
	WORLD_UP_CAMERA_OPPOSING,
	FULL_CAMERA_FACING,
	FULL_CAMERA_OPPOSING,
	COUNT
};

//----------------------------------------------------------------------------------------------------------------------
struct RaycastResult2D
{
	bool	m_didImpact			= false;
	float	m_impactDist		= 0.0f;
	Vec2	m_impactPos			= Vec2( -1.0f, -1.0f );
	Vec2	m_impactNormal		= Vec2( -1.0f, -1.0f );
	float	m_exitDist			= 0.0f;

	Vec2	m_rayForwardNormal	= Vec2( 0.0f, 0.0f );
	Vec2	m_rayStartPos		= Vec2( 0.0f, 0.0f );
	Vec2	m_rayEndPos			= Vec2( 0.0f, 0.0f );
	float	m_rayMaxLength		= 0.0f;
};

//----------------------------------------------------------------------------------------------------------------------
struct RaycastResult3D
{
	bool	m_didImpact			= false;
	float	m_impactDist		= 9999999999.0f;
	Vec3	m_impactPos			= Vec3( 0.0f, 0.0f );
	Vec3	m_impactNormal		= Vec3( 0.0f, 0.0f );
	Vec3	m_rayStartPosition	= Vec3( 0.0f, 0.0f );
	Vec3	m_rayFwdNormal		= Vec3( 0.0f, 0.0f );
	float	m_rayMaxLength		= 1.0f;
};

//----------------------------------------------------------------------------------------------------------------------
// Raycast2D
RaycastResult2D RaycastVsDisc2D( Vec2 startPos, Vec2 fwdNormal, float maxDist, Vec2 discCenter, float discRadius );
RaycastResult2D RaycastVsLine2D( Vec2 rayStartPos, Vec2 rayFwdNormal, float rayMaxLength, Vec2 lineStartPos, Vec2 lineEndPos );
RaycastResult2D RaycastVsAABB2D( Vec2 rayStartPos, Vec2 rayFwdNormal, float rayMaxLength, AABB2 aabb2 );
RaycastResult2D RaycastVsOBB2D( Vec2 rayStartPos, Vec2 rayFwdNormal, float rayMaxLength, OBB2D obb2 );

//----------------------------------------------------------------------------------------------------------------------
// Raycast3D
RaycastResult3D RaycastVsCylinder3D( Vec3 const& rayStartPos, Vec3 const& fwdNormal, float rayLength, Vec2 const& discCenter, float discMinZ, float discMaxZ, float discRadius );
RaycastResult3D RaycastVsAABB3D( Vec3 rayStartPos, Vec3 rayFwdNormal, float rayMaxLength, AABB3 aabb3 );
RaycastResult3D RaycastVsOBB3D( Vec3 rayStartPos, Vec3 rayFwdNormal, float rayMaxLength, OBB3D aabb3 );
RaycastResult3D RaycastVsTriangle( Vec3 const& rayStart, Vec3 const& rayFwdDir, float rayLength, Vec3 const& vert0, Vec3 const& vert1, Vec3 const& vert2, float& t, float& u, float& v );
bool			DoesRaycastHitTriangle( Vec3 const& rayStart, Vec3 const& rayFwdDir, Vec3 const& vert0, Vec3 const& vert1, Vec3 const& vert2, float& t, float& u, float& v );


//-----------------------------------------------------------------------------------------------------------------------
// Clamp and lerp
//----------------------------------------------------------------------------------------------------------------------
float			GetClamped( float value, float minValue, float maxValue );
float			GetClampedZeroToOne( float value );
float			Interpolate( float start, float end, float fractionTowardEnd );
Vec2			Interpolate( Vec2 start, Vec2 end, float fractionTowardEnd );
Vec3			Interpolate( Vec3 start, Vec3 end, float fractionTowardEnd );
Rgba8			Interpolate( Rgba8 startColor, Rgba8 endColor, float fractionTowardEnd );
float			GetFractionWithinRange( float value, float rangeStart, float rangeEnd );
float			RangeMap( float inValue, float inStart, float inEnd, float outStart, float outEnd );
float			RangeMapClamped( float inValue, float inStart, float inEnd, float outStart, float outEnd );
int				RoundDownToInt( float value );

//----------------------------------------------------------------------------------------------------------------------
// Oak utilities
//----------------------------------------------------------------------------------------------------------------------
bool			CompareIfFloatsAreEqual( float valueA, float valueB, float tolerance );
bool			IsFloatWithinRange_Inclusive( float min, float max, float value );
Mat44			ComputeI_Fwd_J_Left_K_Up( Vec3 const& iBasisStart, Vec3 const& iBasisEnd );

//-----------------------------------------------------------------------------------------------------------------------
// Angle utilities
float			ConvertDegreesToRadians( float Degrees);
float			ConvertRadiansToDegrees( float Radians);
float			CosDegrees( float Degrees );
float			SinDegrees( float degrees );
float			TanDegrees( float degrees );
float			Atan2Degrees( float y, float x );
float			ArcSineDegrees( float x );
float			GetShortestAngularDispDegrees( float startDegrees, float EndDegrees );
float			GetTurnedTowardDegrees( float currentDegrees, float goalDegrees, float maxDeltaDegrees );
float			GetAngleDegreesBetweenVectors2D( Vec2 const& a, Vec2 const& b );
float			GetAngleDegreesBetweenVectors3D( Vec3 const& a, Vec3 const& b );
float			GetSignedAngleDegreesBetweenVectors( Vec3 const& referenceVector, Vec3 const& otherVector, Vec3 const& normalVector );
Vec3			RotateVectorAboutArbitraryAxis( Vec3 vectorToRotate, Vec3 arbitraryAxis, float degreesToTurn );

//-----------------------------------------------------------------------------------------------------------------------
// Dot and Cross
float			DotProduct2D( Vec2 const& a, Vec2 const& b );
float			DotProduct3D( Vec3 const& a, Vec3 const& b );
float			DotProduct4D( Vec4 const& a, Vec4 const& b );

float			CrossProduct2D( Vec2 const& a, Vec2 const& b );
Vec3			CrossProduct3D( Vec3 const& a, Vec3 const& b );

float			NormalizeByte( unsigned char byteValue );
unsigned char	DenormalizeByte( float zeroToOne );

//-----------------------------------------------------------------------------------------------------------------------
// Distance & projection utilities
float			GetDistance2D( Vec2 const& positionA, Vec2 const& positionB);
float			GetDistanceSquared2D( Vec2 const& positionA, Vec2 const& positionB ); 
float			GetDistance3D( Vec3 const& positionE, Vec3 const& positionF );							//What about the Z axis?
float			GetDistanceSquared3D( Vec3 const& positionE, Vec3 const& positionF );					//What about the Z axis?
float			GetDistanceXY3D( Vec3 const& positionE, Vec3 const& positionF );
float			GetDistanceXYSquared3D( Vec3 const& positionE, Vec3 const& positionF );
int				GetTaxicabDistance2D( IntVec2 const& pointA, IntVec2 const& pointB );
float			GetProjectedLength2D( Vec2 const& vectorToProject, Vec2 const& VectorToProjectOnto );	// works if Vecs are NOT normalized
float			GetProjectedLength3D( Vec3 const& vectorToProject, Vec3 const& VectorToProjectOnto );	// works if Vecs are NOT normalized
Vec2 const		GetProjectedOnto2D( Vec2 const& vectorToProject, Vec2 const& VectorToProjectOnto );		// works if Vecs are NOT normalized
Vec3 const		GetProjectedOnto3D( Vec3 const& vectorToProject, Vec3 const& VectorToProjectOnto );		// works if Vecs are NOT normalized
Vec3			ProjectVectorOntoPlaneNormalized( Vec3 const& vectorToProject, Vec3 const& planeNormal );

//----------------------------------------------------------------------------------------------------------------------
// Geometric queries
bool			IsPointInsideDisc2D( Vec2 const& point, Vec2 const& discCenter, float discRadius );
bool			IsPointInsideAABB2D( Vec2 const& point, AABB2 const& box );
bool			IsPointInsideAABB3D( Vec3 const& point, AABB3 const& box );
bool			IsPointInsideCapsule2D( Vec2 const& point, Vec2 const& boneStart, Vec2 boneEnd, float radius );
bool			IsPointInsideCapsule3D( Vec3 const& point, Vec3 const& boneStart, Vec3 const& boneEnd, float radius );
bool			IsPointInsideOBB2D( Vec2 const& point, OBB2D const& orientedBox );
bool			IsPointInsideOrientedSector2D( Vec2 const& point, Vec2 const& sectorTip, float sectorFowardDegrees, float sectorApertureDegrees, float sectorRadius );
bool			IsPointInsideDirectedSector2D( Vec2 const& point, Vec2 const& sectorTip, Vec2 const& sectorFowardNormal, float sectorApertureDegrees, float sectorRadius );
bool			IsPointInsideHexagon2D( Vec2 const& point, Vec2 const& center, float circumRadius );
bool			DoDiscsOverlap( Vec2 const& discACenter, float RadiusA, Vec2 const& discBCenter, float RadiusB );
bool			DoSpheresOverlap( Vec3 const& sphereACenter, float RadiusA, Vec3 const& sphereBCenter, float RadiusB );
bool			DoAABB2DOverlap( AABB2 const& boxA, AABB2 const& boxB );
bool			DoAABB3DOverlap( AABB3 const& boxA, AABB3 const& boxB );

Vec2 const		GetNearestPointOnDisc2D( Vec2 const& referencePos, Vec2 const& discCenter, float discRadius );
Vec2 const		GetNearestPointOnInfiniteLine2D( Vec2 const& referencePos, Vec2 const& startPointOnLine, Vec2 const& EndPointOnLine );
Vec2 const		GetNearestPointOnLineSegment2D( Vec2 const& referencePoint, Vec2 const& startPos, Vec2 const& endPos );
Vec2 const		GetNearestPointOnCapsule2D( Vec2 const& refPoint, Vec2 const& BoneStart, Vec2 const& BoneEnd, float radius );	
Vec2 const		GetNearestPointOnOBB2D( Vec2 const& refPoint, OBB2D const& orientedBox );
bool			PushDiscOutOfFixedDisc2D( Vec2& mobileDiscCenter, float mobileDiscRadius, Vec2 const& fixedDiscCenter, float fixedDiscRadius);
bool			PushDiscsOutOfEachOther2D( Vec2& aCenter, float aRadius, Vec2& bCenter, float bRadius );
bool			PushDiscOutOfFixedPoint2D( Vec2& mobileDiscCenter, float discRadius, Vec2 const& fixedPoint );
bool			PushDiscOutOfFixedAABB2D( Vec2& mobileDiscCenter, float discRadius, AABB2 const& fixedBox );

// Discs
void			BounceDiscOffEachOther2D( Vec2& posA, float radiusA, Vec2& velocityA, Vec2& posB, float radiusB, Vec2& velocityB, float elasticityA, float elasticityB );
void			BounceDiscOffFixedDiscBumper2D( Vec2& billiardPos, float billiardRadius, Vec2& billiardVelocity, Vec2 & bumperPos, float bumperRadius, float billiardElasticity, float bumperElasticity );
void			BounceDiscOutOfFixedPoint2D( Vec2& mobileDiscCenter, float mobileDiscRadius, Vec2& mobileDiscVelocity, Vec2 const& fixedPoint, float elasticity );

//----------------------------------------------------------------------------------------------------------------------
// Transformation utilities
void			TransformPosition2D(Vec2& transformedPos, float uniformScale, float rotationDegrees, Vec2 const& translation);  
void			TransformPosition2D(Vec2& transformedPos, Vec2 const& iBasis, Vec2 const& jBasis, Vec2 const& translation );  
void			TransformPositionXY3D(Vec3& transformedPos, float uniformScale, float rotationDegrees, Vec2 const& translation);
void			TransformPositionXY3D(Vec3& transformedPos, Vec2 const& iBasis, Vec2 const& jBasis, Vec2 const& translation );
void			SwapValueOfTwoVariables( float& variableA, float& variableB );

Mat44			GetBillboardMatrix( BillboardType billboardType, Mat44 const& cameraMatrix, Vec3 const& billboardPosition, Vec2 const& billboardScale = Vec2(1.0f, 1.0f) );

//----------------------------------------------------------------------------------------------------------------------
// Bezier standalone functions
float ComputePositionAtCubicBezier1D( float A, float B, float C, float D, float t );
float ComputePositionAtQuinticBezier1D( float A, float B, float C, float D, float E, float F, float t );

//----------------------------------------------------------------------------------------------------------------------
// Easing Functions					// All functions should take in a value [0, 1]
float SmoothStart2( float t );		// EaseInQuadratic
float SmoothStart3( float t );		// EaseInCubic
float SmoothStart4( float t );		// EaseInQuartic
float SmoothStart5( float t );		// EaseInQuintic
float SmoothStart6( float t );		// EaseIn6thOrder

float SmoothStop2( float t );		// EaseOutQuadratic
float SmoothStop3( float t );		// EaseOutCubic
float SmoothStop4( float t );		// EaseOutQuartic
float SmoothStop5( float t );		// EaseOutQuintic
float SmoothStop6( float t );		// EaseOut6thOrder

float SmoothStep3( float t );
float SmoothStep5( float t );

float Hesitate3( float t );
float Hesitate5( float t );

float CustomFunkyEasingFunction( float t );