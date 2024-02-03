#include "Engine//Math/MathUtils.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Core/Clock.hpp"

#include <math.h>

//----------------------------------------------------------------------------------------------------------------------
constexpr float PI = 3.141592653f;

//----------------------------------------------------------------------------------------------------------------------
float ConvertDegreesToRadians(float Degrees) 
{
	return Degrees * ( PI / 180.0f );	
}

//----------------------------------------------------------------------------------------------------------------------
float ConvertRadiansToDegrees(float Radians)
{
	return Radians * ( 180.0f / PI );
}

//----------------------------------------------------------------------------------------------------------------------
float CosDegrees(float degrees)
{
	float radians = ConvertDegreesToRadians(degrees);
	return cosf(radians);
}

//----------------------------------------------------------------------------------------------------------------------
float SinDegrees(float degrees)
{
	float radians = ConvertDegreesToRadians(degrees);
	return sinf(radians);
}


//----------------------------------------------------------------------------------------------------------------------
float TanDegrees( float degrees )
{
	float radians	= ConvertDegreesToRadians( degrees );
	float result	= tanf( radians );
	return result;
}

//----------------------------------------------------------------------------------------------------------------------
float Atan2Degrees(float y, float x)
{
	float radians = atan2f(y, x);
	float degrees = ConvertRadiansToDegrees(radians);
	return degrees;
}

//----------------------------------------------------------------------------------------------------------------------
float ArcSineDegrees( float x )
{
	float radians = asinf( x );
	float degrees = ConvertRadiansToDegrees( radians );
	return degrees;
}

//----------------------------------------------------------------------------------------------------------------------
float GetDistance2D(Vec2 const& positionA, Vec2 const& positionB)
{
	float dx		= (positionB.x - positionA.x);
	float dy		= (positionB.y - positionA.y);
	float distance	= sqrtf((dx * dx) + (dy * dy));
	return distance;
}

//----------------------------------------------------------------------------------------------------------------------
float GetDistanceSquared2D(Vec2 const& positionA, Vec2 const& positionB)
{
	float dx				= (positionB.x - positionA.x);
	float dy				= (positionB.y - positionA.y);
	float distanceSquared	= ((dx * dx) + (dy * dy));
	return distanceSquared;
}

//----------------------------------------------------------------------------------------------------------------------
float GetDistance3D(Vec3 const& positionA, Vec3 const& positionB)
{
	float dx		= (positionB.x - positionA.x);
	float dy		= (positionB.y - positionA.y);
	float dz		= (positionB.z - positionA.z);
	float distance	= sqrtf((dx * dx) + (dy * dy) + (dz * dz));
	return distance;
}

//----------------------------------------------------------------------------------------------------------------------
float GetDistanceSquared3D(Vec3 const& positionA, Vec3 const& positionB)
{
	float dx				= (positionB.x - positionA.x);
	float dy				= (positionB.y - positionA.y);
	float dz				= (positionB.z - positionA.z);
	float distanceSquared	= ((dx * dx) + (dy * dy) + (dz * dz));
	return distanceSquared;
}

//----------------------------------------------------------------------------------------------------------------------
float GetDistanceXY3D(Vec3 const& positionA, Vec3 const& positionB)
{
	float dx		= (positionB.x - positionA.x);
	float dy		= (positionB.y - positionA.y);
	float distance	= sqrtf((dx * dx) + (dy * dy));
	return distance;
}

//----------------------------------------------------------------------------------------------------------------------
float GetDistanceXYSquared3D(Vec3 const& positionA, Vec3 const& positionB)
{
	float dx				= (positionB.x - positionA.x);
	float dy				= (positionB.y - positionA.y);
	float distanceSquared	= (dx * dx) + (dy * dy);
	return distanceSquared;
}

//----------------------------------------------------------------------------------------------------------------------
int GetTaxicabDistance2D(IntVec2 const& pointA, IntVec2 const& pointB)
{
	int distX = abs( pointB.x - pointA.x );
	int distY = abs( pointB.y - pointA.y );
	return ( distX + distY );
}

//----------------------------------------------------------------------------------------------------------------------
float GetProjectedLength2D(Vec2 const& vectorToProject, Vec2 const& VectorToProjectOnto)
{	
	float projectedLength = DotProduct2D( vectorToProject, VectorToProjectOnto.GetNormalized() );
	return projectedLength;
}

//----------------------------------------------------------------------------------------------------------------------
float GetProjectedLength3D( Vec3 const& vectorToProject, Vec3 const& vectorToProjectOnto )
{
	float projectedLength = DotProduct3D( vectorToProject, vectorToProjectOnto );
	return projectedLength;
}

//----------------------------------------------------------------------------------------------------------------------
Vec2 const GetProjectedOnto2D(Vec2 const& vectorToProject, Vec2 const& VectorToProjectOnto)
{
	float projectedLength	= GetProjectedLength2D( vectorToProject, VectorToProjectOnto );
	Vec2 BProjectedOntoA	= projectedLength * VectorToProjectOnto.GetNormalized();
	return BProjectedOntoA;
}

//----------------------------------------------------------------------------------------------------------------------
Vec3 const GetProjectedOnto3D( Vec3 const& vectorToProject, Vec3 const& VectorToProjectOnto )
{
	float projectedLength  = GetProjectedLength3D( vectorToProject, VectorToProjectOnto );
	Vec3  BProjectedOntoA  = projectedLength * VectorToProjectOnto.GetNormalized();
	return BProjectedOntoA;
}


//----------------------------------------------------------------------------------------------------------------------
Vec3 ProjectVectorOntoPlaneNormalized( Vec3 const& vectorToProject, Vec3 const& planeNormal )
{
	// Project onto plane normal
	float lengthOfShadowOnNormal			= DotProduct3D( vectorToProject, planeNormal );
	// Project onto plane
	Vec3 amountToSubtractToFlattenVector	= planeNormal * lengthOfShadowOnNormal;
	Vec3 currentVecProjectedOntoPlane		= vectorToProject - amountToSubtractToFlattenVector;
	currentVecProjectedOntoPlane.Normalize();
	return currentVecProjectedOntoPlane;
}


//----------------------------------------------------------------------------------------------------------------------
bool IsPointInsideDisc2D(Vec2 const& point, Vec2 const& discCenter, float discRadius)
{
	Vec2 dispDiscCenterToPoint	= point - discCenter;
	float distance				= dispDiscCenterToPoint.GetLength();
	return ( distance < discRadius );
}

//----------------------------------------------------------------------------------------------------------------------
bool IsPointInsideAABB2D( Vec2 const& point, AABB2 const& box )
{
	if ( point.x > box.m_mins.x && point.x < box.m_maxs.x && 
		 point.y > box.m_mins.y && point.y < box.m_maxs.y )
	{
		return true;
	}	
	return false;
}

//----------------------------------------------------------------------------------------------------------------------
bool IsPointInsideAABB3D( Vec3 const& point, AABB3 const& box )
{
	if ( point.x > box.m_mins.x && point.x < box.m_maxs.x &&
		 point.y > box.m_mins.y && point.y < box.m_maxs.y &&
		 point.z > box.m_mins.z && point.z < box.m_maxs.z )
	{
		return true;
	}
	return false;
}


//----------------------------------------------------------------------------------------------------------------------
bool IsPointInsideCapsule2D( Vec2 const& point, Vec2 const& boneStart, Vec2 boneEnd, float radius )
{
	Vec2 const& nearestPointOnBone				= GetNearestPointOnLineSegment2D( point, boneStart, boneEnd );
	Vec2 vecFromNearestPointOnBoneToRefPoint	= point - nearestPointOnBone;

	if ( vecFromNearestPointOnBoneToRefPoint.GetLength() > radius )
	{
		return false;
	}

	return true;
}

//---------------------------------------------------------------------------------------------------------------------- 
bool IsPointInsideCapsule3D( Vec3 const& point, Vec3 const& boneStart, Vec3 const& boneEnd, float radius )
{
 	Vec3 SE = boneEnd - boneStart;				// SE is the fwd disp along the bone (start to end)
 	Vec3 SP = point - boneStart;				// SP is the disp from boneStart to point
	if ( DotProduct3D( SE, SP ) < 0.0f )
	{
		// Voronoi region 1; nearest point on bone to P is S
		if ( GetDistanceSquared3D( point, boneStart ) < ( radius * radius ) )
		{
			return true;
		}
		return false;
	}

	Vec3 ES = boneStart - boneEnd;				// ES 
	Vec3 EP = point - boneEnd;					// EP is disp from boneEnd to point
	if ( DotProduct3D( ES, EP ) < 0.0f )
	{
		// Voronoi region II; nearest point on bone to P is E
		if ( GetDistanceSquared3D( point, boneEnd ) < ( radius * radius ) )
		{
			return true;
		}
		return false;
	}

	// If we reached this far in the function, we MUST be in 
	// Voronoi region III; nearest point to bone is somewhere in the middle
	Vec3 SN			  = GetProjectedOnto3D( SP, SE.GetNormalized() );		// SP projected onto SE
	Vec3 nearestPoint = boneStart + SN;						// Nearest point to P along the capsule bone
	if ( GetDistanceSquared3D( point, nearestPoint ) < ( radius * radius ) )
	{
		return true;
	}
	return false;
}

//----------------------------------------------------------------------------------------------------------------------
bool IsPointInsideOBB2D( Vec2 const& point, OBB2D const& orientedBox )
{
	Vec2 jBasisNormal	= orientedBox.m_iBasisNormal.GetRotated90Degrees();
	Vec2 right			= orientedBox.m_iBasisNormal * orientedBox.m_halfDimensions.x;
	Vec2 up				= jBasisNormal * orientedBox.m_halfDimensions.y;

	Vec2 const& center	= orientedBox.m_center;
	Vec2 topRight		= center + right + up;
	Vec2 topLeft		= center - right + up;
	Vec2 bottomRight	= center + right - up;
	Vec2 bottomLeft		= center - right - up;

	Vec2 centerToPoint	= point - orientedBox.m_center;
	Vec2 CPi		= GetProjectedOnto2D( centerToPoint, orientedBox.m_iBasisNormal );
	Vec2 CPj		= GetProjectedOnto2D( centerToPoint, jBasisNormal );

	if ( CPi.GetLength() >= orientedBox.m_halfDimensions.x || 
		 CPj.GetLength() >= orientedBox.m_halfDimensions.y	)
	{
		return false;
	}

	return true;
}

//----------------------------------------------------------------------------------------------------------------------
bool IsPointInsideOrientedSector2D(Vec2 const& point, Vec2 const& sectorTip, float sectorFowardDegrees, float sectorApertureDegrees, float sectorRadius)
{
	// checks angle between x axis to sectorForwardNormal; 0 orientation degrees to foward normal
	Vec2 fowardNormal	= Vec2::MakeFromPolarDegrees( sectorFowardDegrees, sectorRadius );
	bool isPointInside	= IsPointInsideDirectedSector2D( point, sectorTip, fowardNormal, sectorApertureDegrees, sectorRadius );
	return isPointInside;
}

//----------------------------------------------------------------------------------------------------------------------
bool IsPointInsideDirectedSector2D(Vec2 const& point, Vec2 const& sectorTip, Vec2 const& sectorFowardNormal, float sectorApertureDegrees, float sectorRadius)
{
	// compare if dist from ( P - C ) < radius, then return;
	// calculate halfAperature;
	// if angle between vectorCenterToPointAndForwardNormal < halfAperture;

	// Check 1; point is longer than radius
	Vec2 disp = point - sectorTip;
	if ( disp.GetLength() > sectorRadius )
	{
		return false;
	} 

	// Check2; angle > halfAperature
	float halfApertureDegrees						= sectorApertureDegrees / 2.0f;
	float angleBetweenCenterToPointAndForwardNormal = GetAngleDegreesBetweenVectors2D( sectorFowardNormal, disp );
	if ( angleBetweenCenterToPointAndForwardNormal > halfApertureDegrees )
	{
		return false;
	}

	return true;
}


//----------------------------------------------------------------------------------------------------------------------
bool IsPointInsideHexagon2D( Vec2 const& point, Vec2 const& hexCenter, float circumRadius )
{
	int	  numPizzaSlices  = 6;
	float degreesPerSlice = 60.0f;
	for ( int i = 0; i < numPizzaSlices; i++ )
	{
		float currentOrientation = degreesPerSlice * i;
		float nextOrientation	 = degreesPerSlice + currentOrientation;

		Vec2 disp1		= Vec2::MakeFromPolarDegrees( currentOrientation, circumRadius );
		Vec2 disp2		= Vec2::MakeFromPolarDegrees( nextOrientation,   circumRadius );
		Vec2 currentPos	= hexCenter + disp1;
		Vec2 nextPos	= hexCenter + disp2;

		Vec2 dispPointToCurrentVert = currentPos - point;
		Vec2 dispPointToNextVert	= nextPos - point;
		float crossProductResult	= CrossProduct2D( dispPointToCurrentVert, dispPointToNextVert );

		if ( crossProductResult	< 0 )
		{
			return false;
		}
	}

	return true;
}


//----------------------------------------------------------------------------------------------------------------------
bool DoDiscsOverlap(Vec2 const& discACenter, float RadiusA, Vec2 const& discBCenter, float RadiusB)
{
	float dx		= (discBCenter.x - discACenter.x);
	float dy		= (discBCenter.y - discACenter.y);
	float Distance	= sqrtf((dx * dx) + (dy * dy));
	float Diameter	= (RadiusA + RadiusB);
	
	bool DiscsOverlap = false;
	if (Diameter > Distance)
	{
		DiscsOverlap = true;
	}

	return DiscsOverlap;
}

//----------------------------------------------------------------------------------------------------------------------
bool DoSpheresOverlap(Vec3 const& sphereACenter, float RadiusA, Vec3 const& sphereBCenter, float RadiusB)
{
	float dx			= (sphereBCenter.x - sphereACenter.x);
	float dy			= (sphereBCenter.y - sphereACenter.y);
	float dz			= (sphereBCenter.z - sphereACenter.z);
	float distance		= sqrtf((dx * dx) + (dy * dy) + (dz * dz));
	float sumOfRadii	= (RadiusA + RadiusB);	
	return distance < sumOfRadii;
}

//----------------------------------------------------------------------------------------------------------------------
bool DoAABB2DOverlap( AABB2 const& boxA, AABB2 const& boxB )
{
	UNUSED( boxA );
	UNUSED( boxB );
	return false;
}

//----------------------------------------------------------------------------------------------------------------------
bool DoAABB3DOverlap( AABB3 const& boxA, AABB3 const& boxB )
{
	if ( boxA.m_maxs.x < boxB.m_mins.x )
	{
		return false;
	}
	if ( boxA.m_mins.x > boxB.m_maxs.x )
	{
		return false;
	}
	if ( boxA.m_maxs.y < boxB.m_mins.y )
	{
		return false;
	}
	if ( boxA.m_mins.y > boxB.m_maxs.y )
	{
		return false;
	}
	if ( boxA.m_maxs.z < boxB.m_mins.z )
	{
		return false;
	}
	if ( boxA.m_mins.z > boxB.m_maxs.z )
	{
		return false;
	}

	return true;
}

//----------------------------------------------------------------------------------------------------------------------
Vec2 const GetNearestPointOnDisc2D( Vec2 const& referencePos, Vec2 const& discCenter, float discRadius )
{
	// If refPosition is on Disc, and distance from refPosition to discCenter <= Radius, then return refPosition 
	// else, return point on edge of disc in orientation of refPosition 

	Vec2 dispDiscCenterToRefPos	= referencePos - discCenter;
	float distance				= dispDiscCenterToRefPos.GetLength();

	if (distance < discRadius)
	{
		return referencePos;
	}

	//Vec2 pointOnEdgeOfDisc =
	dispDiscCenterToRefPos.ClampLength( discRadius );
	
	return dispDiscCenterToRefPos + discCenter;
}

//----------------------------------------------------------------------------------------------------------------------
Vec2 const GetNearestPointOnInfiniteLine2D( Vec2 const& referencePos, Vec2 const& startPointOnLine, Vec2 const& endPointOnLine )
{
	Vec2 const& startToEnd = endPointOnLine - startPointOnLine;
	Vec2 const& startToRef = referencePos - startPointOnLine;

	Vec2 const& projectedVector = GetProjectedOnto2D( startToRef, startToEnd );
	Vec2 nearestPointOnLine = projectedVector + startPointOnLine;
	return nearestPointOnLine;
}

//----------------------------------------------------------------------------------------------------------------------
Vec2 const GetNearestPointOnLineSegment2D( Vec2 const& referencePoint, Vec2 const& startPos, Vec2 const& endPos )
{
	Vec2 dispStartToEnd = endPos - startPos;
	Vec2 dispEndToStart = startPos - endPos;
	Vec2 dispStartToRef = referencePoint - startPos;
	Vec2 dispEndToRef	= referencePoint - endPos;

	float dotProductStartToPoint = DotProduct2D( dispStartToEnd, dispStartToRef);
	float dotProductEndToPoint	 = DotProduct2D( dispEndToStart, dispEndToRef);

	// Is in Voronoi Region 1
	if ( dotProductStartToPoint < 0 )
	{
		return startPos;
	}
	// Is in Voronoi Region 2
	if ( dotProductEndToPoint < 0 )
	{
		return endPos;
	}
	// Is in Voronoi Region 3
	dispStartToEnd.GetNormalized();
	Vec2 const& projectedLength		= GetProjectedOnto2D( dispStartToRef, dispStartToEnd );
	Vec2 nearestPointOnLine			= projectedLength + startPos;
	return nearestPointOnLine;
}

//----------------------------------------------------------------------------------------------------------------------
Vec2 const GetNearestPointOnCapsule2D( Vec2 const& refPoint, Vec2 const& boneStart, Vec2 const& boneEnd, float radius )
{
	Vec2 const& nearestPointOnBone				= GetNearestPointOnLineSegment2D( refPoint, boneStart, boneEnd );
	Vec2 vecFromNearestPointOnBoneToRefPoint	= refPoint - nearestPointOnBone;
	vecFromNearestPointOnBoneToRefPoint.ClampLength( radius );
	Vec2 NearestPointOnCapsuleToRefPoint		= vecFromNearestPointOnBoneToRefPoint + nearestPointOnBone;

	return NearestPointOnCapsuleToRefPoint;
}

//----------------------------------------------------------------------------------------------------------------------
Vec2 const GetNearestPointOnOBB2D( Vec2 const& refPoint, OBB2D const& orientedBox )
{
	Vec2 jBasisNormal		= orientedBox.m_iBasisNormal.GetRotated90Degrees();
	Vec2 pointToBoxCenter	= refPoint - orientedBox.m_center;
	float projectedI		= GetProjectedLength2D( pointToBoxCenter, orientedBox.m_iBasisNormal );
	float projectedJ		= GetProjectedLength2D( pointToBoxCenter, jBasisNormal );

	float clampedIDist		= GetClamped( projectedI, ( -orientedBox.m_halfDimensions.x ), ( orientedBox.m_halfDimensions.x ) );
	float clampedJDist		= GetClamped( projectedJ, ( -orientedBox.m_halfDimensions.y ), ( orientedBox.m_halfDimensions.y ) );

	Vec2 clampedNearestPoint = orientedBox.m_center + ( clampedIDist * orientedBox.m_iBasisNormal ) + ( clampedJDist * jBasisNormal );

	return clampedNearestPoint;
}

//----------------------------------------------------------------------------------------------------------------------
bool PushDiscOutOfFixedDisc2D(Vec2& mobileDiscCenter, float mobileDiscRadius, Vec2 const& fixedDiscCenter, float fixedDiscRadius)
{
	// if not overlapping, return false
	// if overlapping, push out of disc, then return true

	if ( DoDiscsOverlap( mobileDiscCenter, mobileDiscRadius, fixedDiscCenter, fixedDiscRadius ) == false )
	{ 
		return false;
	}

		// calculated and normalized overlap vector then added total distance from fixedDiscCenter 
		Vec2 vectorBetweenDiscs			=  mobileDiscCenter - fixedDiscCenter;
		vectorBetweenDiscs.Normalize();
 		float sumOfRadii				= mobileDiscRadius + fixedDiscRadius;
		Vec2 scaledDisplacementVector	= vectorBetweenDiscs * sumOfRadii;
		mobileDiscCenter = fixedDiscCenter + scaledDisplacementVector;

		return true;
}

//----------------------------------------------------------------------------------------------------------------------
bool PushDiscsOutOfEachOther2D( Vec2& aCenter, float aRadius, Vec2& bCenter, float bRadius )
{
	if ( DoDiscsOverlap( aCenter, aRadius, bCenter, bRadius ) == false )
	{
		return false;
	}

	float sumOfRadii = aRadius + bRadius;
	Vec2 displacement = bCenter - aCenter;
	float distanceBetweenDiscs = displacement.GetLength();
	Vec2 overlap = displacement.GetNormalized() * ( sumOfRadii - distanceBetweenDiscs );

	//float distanceBetweenDiscs = displacement.GetLength();
	//float overlapDistance = sumOfRadii - distanceBetweenDiscs;
	//float halfoverlapDistance = overlapDistance / 2.0f;

	aCenter -= overlap * 0.5f;
	bCenter += overlap * 0.5f;
	return true;
}

//----------------------------------------------------------------------------------------------------------------------
bool PushDiscOutOfFixedPoint2D( Vec2& mobileDiscCenter, float discRadius, Vec2 const& fixedPoint )
{
	Vec2 displacementToPoint = mobileDiscCenter - fixedPoint;
	float magnitude = displacementToPoint.GetLength();
	float overlapLength = discRadius - magnitude;
	Vec2 overlapVec = displacementToPoint.GetNormalized() * overlapLength;

	if ( magnitude <= discRadius )
	{
		mobileDiscCenter += overlapVec;
		return true;
	}
	return false;
}

//----------------------------------------------------------------------------------------------------------------------
bool PushDiscOutOfFixedAABB2D( Vec2& mobileDiscCenter, float discRadius, AABB2 const& fixedBox )
{
	// if discCenter is inside AABB2, return false
	// Only push back if discCenter is outside

	if ( fixedBox.IsPointInside( mobileDiscCenter ) == true )
	{
		return false;
	}

	Vec2 nearestPointOnBox = fixedBox.GetNearestPoint( mobileDiscCenter );
	Vec2 scale = mobileDiscCenter - nearestPointOnBox;
	float magnitudeFromBoxToDisc = scale.GetLength();
	float overlapLength = discRadius - magnitudeFromBoxToDisc;

	if ( magnitudeFromBoxToDisc <= discRadius )
	{
		scale.Normalize();
		Vec2 overlapVec = scale * overlapLength;
		mobileDiscCenter += overlapVec;
		return true;
	}

	return false;
}

//----------------------------------------------------------------------------------------------------------------------
void BounceDiscOffEachOther2D( Vec2& posA, float radiusA, Vec2& velocityA, Vec2& posB, float radiusB, Vec2& velocityB, float elasticityA, float elasticityB )
{
	bool isPushed = PushDiscsOutOfEachOther2D( posA, radiusA, posB, radiusB );

	// Handle edge case if discs are spawned on top of each other
	if ( !isPushed )
	{
		return;
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Calculate vector A to B, then project reflection velocity vector with elasticity

	// Vector A to B
	Vec2 dispAToB = ( posB - posA ).GetNormalized();
	float velA_ProjectedDispAToB = GetProjectedLength2D( velocityA, dispAToB );
	float velB_ProjectedDispAToB = GetProjectedLength2D( velocityB, dispAToB );

	if ( velA_ProjectedDispAToB > velB_ProjectedDispAToB )
	{
		Vec2 VAI = velA_ProjectedDispAToB * dispAToB;
		Vec2 VBI = velB_ProjectedDispAToB * dispAToB;

		Vec2 VAJ = velocityA - VAI;
		Vec2 VBJ = velocityB - VBI;

		Vec2 VA_Bounced = Vec2::NEGATIVE_ONE;
		Vec2 VB_Bounced = Vec2::NEGATIVE_ONE;

		float superElasticity = elasticityA * elasticityB;

		// Converging, swap velocities
		VA_Bounced = VAJ + ( VBI * superElasticity );
		VB_Bounced = VBJ + ( VAI * superElasticity );

		// Apply reflections to current velocities
		velocityA = VA_Bounced;
		velocityB = VB_Bounced;
	}
}

//----------------------------------------------------------------------------------------------------------------------
void BounceDiscOffFixedDiscBumper2D( Vec2& billiardPos, float billiardRadius, Vec2& billiardVelocity,
	Vec2& bumperPos, float bumperRadius,
	float billiardElasticity, float bumperElasticity )
{
	// Check if discs overlaps // Distance between two discs <= radiusA + radiusB
	bool isPushed = PushDiscOutOfFixedDisc2D( billiardPos, billiardRadius, bumperPos, bumperRadius );
	if ( !isPushed )
	{
		return;
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Calculate vector A to B, then project reflection velocity vector with elasticity
	Vec2 discAToDiscB		= bumperPos - billiardPos;
	float superElasticity	= billiardElasticity * bumperElasticity;

	// Vector A to B
	Vec2 discAToDiscB_Normalized	= discAToDiscB.GetNormalized();
	Vec2 VAN_withoutElasticity		= GetProjectedOnto2D( billiardVelocity, discAToDiscB_Normalized );
	Vec2 VAT						= billiardVelocity - VAN_withoutElasticity;
	Vec2 VAN						= VAN_withoutElasticity * superElasticity;

	// Exchange velocities
	Vec2 VA_Bounced = VAT - VAN;

	// Apply reflections to current velocities
	billiardVelocity = VA_Bounced;
}

//----------------------------------------------------------------------------------------------------------------------
void BounceDiscOutOfFixedPoint2D( Vec2& mobileDiscCenter, float mobileDiscRadius, Vec2& mobileDiscVelocity, Vec2 const& fixedPoint, float elasticity )
{
	if ( !PushDiscOutOfFixedPoint2D( mobileDiscCenter, mobileDiscRadius, fixedPoint ) )
	{
		return;
	}

	Vec2 surfaceNormal					= ( mobileDiscCenter - fixedPoint ).GetNormalized();
	float projectedLengthAlongNormal	= DotProduct2D( mobileDiscVelocity, surfaceNormal );
	Vec2 projectionAlongNormal			= surfaceNormal * projectedLengthAlongNormal;
	Vec2 projectionAlongTangent			= mobileDiscVelocity - projectionAlongNormal;
	mobileDiscVelocity					= projectionAlongTangent + ( -projectionAlongNormal * elasticity );
}

//----------------------------------------------------------------------------------------------------------------------
void TransformPosition2D( Vec2& transformedPos, float uniformScale, float rotationDegrees, Vec2 const& translation )
{
	transformedPos *= uniformScale;
	transformedPos.RotateDegrees( rotationDegrees );
	transformedPos += translation;
}

//----------------------------------------------------------------------------------------------------------------------
void TransformPosition2D( Vec2& transformedPos, Vec2 const& iBasis, Vec2 const& jBasis, Vec2 const& translation )
{
	transformedPos = ( iBasis * transformedPos.x ) + ( jBasis * transformedPos.y );
	transformedPos += translation;
}

//----------------------------------------------------------------------------------------------------------------------
void TransformPositionXY3D( Vec3& transformedPos, float uniformScale, float rotationDegrees, Vec2 const& translation )
{
	transformedPos.x *= uniformScale;
	transformedPos.y *= uniformScale;

	float Radians				= ConvertDegreesToRadians( rotationDegrees );
	float Distance				= sqrtf( ( transformedPos.x * transformedPos.x ) + ( transformedPos.y * transformedPos.y ) );
	float OrientatioInRadians	= atan2f( transformedPos.y, transformedPos.x ); //Angle I'm facing

	OrientatioInRadians += Radians;

	transformedPos.x = Distance * cosf( OrientatioInRadians );
	transformedPos.y = Distance * sinf( OrientatioInRadians );

	transformedPos.x += translation.x;
	transformedPos.y += translation.y;
}

//----------------------------------------------------------------------------------------------------------------------
void TransformPositionXY3D( Vec3& transformedPos, Vec2 const& iBasis, Vec2 const& jBasis, Vec2 const& translation )
{
	Vec2 posToTransformXY = Vec2( transformedPos.x, transformedPos.y );
	TransformPosition2D( posToTransformXY, iBasis, jBasis, translation );
	transformedPos = Vec3( posToTransformXY.x, posToTransformXY.y, transformedPos.z );
}

//----------------------------------------------------------------------------------------------------------------------
void SwapValueOfTwoVariables( float& variableA, float& variableB )
{
	// TempVariable preserves value of variableA to enable swap between both variables A and B
	float tempVariable;
	tempVariable	= variableA;
	variableA		= variableB;
	variableB		= tempVariable;
}

//----------------------------------------------------------------------------------------------------------------------
Mat44 GetBillboardMatrix( BillboardType billboardType, Mat44 const& cameraMatrix, Vec3 const& billboardPosition, Vec2 const& billboardScale )
{
	UNUSED( billboardScale );

	Mat44 billboardMatrix;

	// Billboarded entity's basis and position
	Vec3 iBasis		= Vec3( 1.0f, 0.0f, 0.0f );
	Vec3 jBasis		= Vec3( 0.0f, 1.0f, 0.0f );
	Vec3 kBasis		= Vec3( 0.0f, 0.0f, 1.0f );
	Vec3 tPosition	= billboardPosition;

	// Camera or target's basis and position
	Vec3 forward	= cameraMatrix.GetIBasis3D();
	Vec3 left		= cameraMatrix.GetJBasis3D();
	Vec3 up			= cameraMatrix.GetKBasis3D();
	Vec3 position	= cameraMatrix.GetTranslation3D();

	Vec3 worldZSkyward	= Vec3( 0.0f, 0.0f, 1.0f );
	Vec3 worldYLeft		= Vec3( 0.0f, 1.0f, 0.0f );

	if ( billboardType == BillboardType::FULL_CAMERA_OPPOSING )
	{
		iBasis = -forward;
		jBasis = -left;
		kBasis = up;
	}
	else if ( billboardType == BillboardType::WORLD_UP_CAMERA_FACING )
	{
		kBasis		= worldZSkyward;
		iBasis		= position - tPosition;
		iBasis.z	= 0.0f;
		//		iBasis		= -forward;
		iBasis		= iBasis.GetNormalized();
		jBasis		= CrossProduct3D( kBasis, iBasis );
		jBasis		= jBasis.GetNormalized();
	}
	else if ( billboardType == BillboardType::WORLD_UP_CAMERA_OPPOSING )
	{
		kBasis = worldZSkyward;
		iBasis = -forward;
		jBasis = -left;
	}
	else if ( billboardType == BillboardType::FULL_CAMERA_FACING )
	{
		iBasis = position - tPosition;
		iBasis = iBasis.GetNormalized();
		//		iBasis	= -forward;
		float dotProductAbs = abs( DotProduct3D( iBasis, worldZSkyward ) );

		if ( dotProductAbs < 1 )	// If parallel
		{
			jBasis = CrossProduct3D( worldZSkyward, iBasis );
			jBasis = jBasis.GetNormalized();
			kBasis = CrossProduct3D( iBasis, jBasis );
			kBasis = kBasis.GetNormalized();
		}
		else						// If not parallel
		{
			kBasis = CrossProduct3D( iBasis, worldYLeft );
			kBasis = kBasis.GetNormalized();
			jBasis = CrossProduct3D( kBasis, iBasis );
			jBasis = jBasis.GetNormalized();
		}
	}

	billboardMatrix.SetIJKT3D( iBasis, jBasis, kBasis, tPosition );
	return billboardMatrix;
}

//----------------------------------------------------------------------------------------------------------------------
float ComputePositionAtCubicBezier1D( float A, float B, float C, float D, float t )
{
	float S = 1 - t;

	float E = ( S * A ) + ( t * B );
	float F = ( S * B ) + ( t * C );
	float G = ( S * C ) + ( t * D );

	float H = ( S * E ) + ( t * F );
	float I = ( S * F ) + ( t * G );

	float point = ( S * H ) + ( t * I );
	return point;
}

//----------------------------------------------------------------------------------------------------------------------
float ComputePositionAtQuinticBezier1D( float A, float B, float C, float D, float E, float F, float t )
{
	float s = 1 - t;

	float G = ( s * A ) + ( t * B );
	float H = ( s * B ) + ( t * C );
	float I = ( s * C ) + ( t * D );
	float J = ( s * D ) + ( t * E );
	float K = ( s * E ) + ( t * F );

	float L = ( s * G ) + ( t * H );
	float M = ( s * H ) + ( t * I );
	float N = ( s * I ) + ( t * J );
	float O = ( s * J ) + ( t * K );

	float P = ( s * L ) + ( t * M );
	float Q = ( s * M ) + ( t * N );
	float R = ( s * N ) + ( t * O );

	float S = ( s * P ) + ( t * Q );
	float T = ( s * Q ) + ( t * R );

	float point = ( s * S ) + ( t * T );

	return point;
}

//----------------------------------------------------------------------------------------------------------------------
float SmoothStart2( float t )
{
	float t2 = t * t;
	return t2;
}

//----------------------------------------------------------------------------------------------------------------------
float SmoothStart3( float t )
{
	float t3 = t * t * t;
	return t3;
}

//----------------------------------------------------------------------------------------------------------------------
float SmoothStart4( float t )
{
	float t4 = t * t * t * t;
	return t4;
}

//----------------------------------------------------------------------------------------------------------------------
float SmoothStart5( float t )
{
	float t5 = t * t * t * t * t;
	return t5;
}

//----------------------------------------------------------------------------------------------------------------------
float SmoothStart6( float t )
{
	float t6 = t * t * t * t * t * t;
	return t6;
}

//----------------------------------------------------------------------------------------------------------------------
float SmoothStop2( float t )
{
	float s = ( 1 - t );
	float t2 = 1 - ( s * s );
	return t2;
}

//----------------------------------------------------------------------------------------------------------------------
float SmoothStop3( float t )
{
	float s = ( 1 - t );
	float t3 = 1 - ( s * s * s );
	return t3;
}

//----------------------------------------------------------------------------------------------------------------------
float SmoothStop4( float t )
{
	float s = ( 1 - t );
	float t4 = 1 - ( s * s * s * s );
	return t4;
}

//----------------------------------------------------------------------------------------------------------------------
float SmoothStop5( float t )
{
	float s = ( 1 - t );
	float t5 = 1 - ( s * s * s * s * s );
	return t5;
}

//----------------------------------------------------------------------------------------------------------------------
float SmoothStop6( float t )
{
	float s = ( 1 - t );
	float t6 = 1 - ( s * s * s * s * s * s );
	return t6;
}

//----------------------------------------------------------------------------------------------------------------------
float SmoothStep3( float t )
{
	float smoothStep3 = ComputePositionAtCubicBezier1D( 0.0f, 0.0f, 1.0f, 1.0f, t );
	return smoothStep3;
}

//----------------------------------------------------------------------------------------------------------------------
float SmoothStep5( float t )
{
	float smoothStep5 = ComputePositionAtQuinticBezier1D( 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, t );
	return smoothStep5;
}

//----------------------------------------------------------------------------------------------------------------------
float Hesitate3( float t )
{
	float hesitate3 = ComputePositionAtCubicBezier1D( 0.0f, 1.0f, 0.0f, 1.0f, t );
	return hesitate3;
}

//----------------------------------------------------------------------------------------------------------------------
float Hesitate5( float t )
{
	float hesitate5 = ComputePositionAtQuinticBezier1D( 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, t );
	return hesitate5;
}

//----------------------------------------------------------------------------------------------------------------------
float CustomFunkyEasingFunction( float t )
{
	// 	float customFunky	= ComputePositionAtQuinticBezier1D( 0.0f, (SinDegrees(t * 25.0f) + 2.0f ) * 0.5f, 1.0f, 0.0f, SinDegrees(t * 100.0f), 1.0f, t );
	//	float customFunky	= ComputePositionAtCubicBezier1D( 0.0f, (SinDegrees(t * 25.0f) + 2.0f ) * 0.5f, quinticBezier, SinDegrees(t * 100.0f), t );
	//	float customFunky	= t * ComputePositionAtQuinticBezier1D( 0.0f, 1.0f, -0.5f, 0.0f, 0.5f, 1.0f, t );
	//	float customFunky	= (CosDegrees(t * -50.0f) * SinDegrees(t * 100.0f) );
	float customFunky = ComputePositionAtQuinticBezier1D( 0.0f, 1.0f, t * t, 1.0f, 0.0f, 1.0f, t );

	return customFunky;
}

//----------------------------------------------------------------------------------------------------------------------
RaycastResult2D RaycastVsDisc2D( Vec2 startPos, Vec2 fwdNormal, float maxDist, Vec2 discCenter, float discRadius )
{
	RaycastResult2D raycastResult;
	raycastResult.m_rayForwardNormal = fwdNormal;
	raycastResult.m_rayStartPos = startPos;
	raycastResult.m_rayMaxLength = maxDist;

	Vec2  SC = discCenter - raycastResult.m_rayStartPos;
	float SCi = GetProjectedLength2D( SC, fwdNormal );
	Vec2  j = raycastResult.m_rayForwardNormal.GetRotated90Degrees();
	float SCj = GetProjectedLength2D( SC, j );

	//----------------------------------------------------------------------------------------------------------------------
	// Checking miss cases
	// disc is too far right or left
	if ( SCj > discRadius || SCj < -discRadius )
	{
		raycastResult.m_didImpact = false;
		return raycastResult;
	}
	// disc is before or after ray
	if ( SCj < -discRadius || SCj > raycastResult.m_rayMaxLength + discRadius )
	{
		raycastResult.m_didImpact = false;
		return raycastResult;
	}

	// Calculating impactDist
	float a						= sqrtf( ( discRadius * discRadius ) - ( SCj * SCj ) );
	raycastResult.m_exitDist	= SCi + a;
	raycastResult.m_impactDist	= SCi - a;

	// rayStartPoint is inside disc
	Vec2 distFromDistToRayStart = discCenter - raycastResult.m_rayStartPos;
	if ( distFromDistToRayStart.GetLength() < discRadius )
	{
		raycastResult.m_impactPos		= raycastResult.m_rayStartPos;
		raycastResult.m_impactDist		= 0;
		raycastResult.m_impactNormal	= -raycastResult.m_rayForwardNormal;
		raycastResult.m_didImpact		= true;
		return raycastResult;
	}

	// raycast is too short or too long
	if ( raycastResult.m_impactDist < 0 || raycastResult.m_impactDist > maxDist )
	{
		raycastResult.m_didImpact = false;
		return raycastResult;
	}

	raycastResult.m_impactPos		= startPos + ( raycastResult.m_rayForwardNormal * raycastResult.m_impactDist );
	raycastResult.m_impactNormal	= ( raycastResult.m_impactPos - discCenter ).GetNormalized();
	raycastResult.m_didImpact		= true;
	return raycastResult;
}

//----------------------------------------------------------------------------------------------------------------------
RaycastResult2D RaycastVsLine2D( Vec2 rayStartPos, Vec2 rayFwdNormal, float rayMaxLength, Vec2 lineStartPos, Vec2 lineEndPos )
{
	// Initialize raycast
	RaycastResult2D raycastHitResult;
	RaycastResult2D raycastMissResult;
	raycastHitResult.m_rayStartPos		= rayStartPos;
	raycastHitResult.m_rayForwardNormal = rayFwdNormal;
	raycastHitResult.m_rayMaxLength		= rayMaxLength;
	Vec2 j = rayFwdNormal.GetRotated90Degrees();

	// Initialize line segment
	Vec2 rayStartToLineEnd			= lineEndPos - rayStartPos;
	Vec2 rayStartToLineStart		= lineStartPos - rayStartPos;
	raycastHitResult.m_rayEndPos	= rayStartPos + ( rayFwdNormal * rayMaxLength );

	// Calculate projected lengths
	float lineEndj		= GetProjectedLength2D( rayStartToLineEnd, j );
	float lineStartj	= GetProjectedLength2D( rayStartToLineStart, j );

	// Straddle Test
	if ( lineStartj <= 0 && lineEndj <= 0 )
	{
		return raycastMissResult;	// Too far left
	}
	if ( lineStartj >= 0 && lineEndj >= 0 )
	{
		return raycastMissResult;	// Too far right
	}

	// Calculate time to get impact point on line
	float time			= -lineStartj / ( lineEndj - lineStartj );		// time for parametric equation // aka "ratio" or "fraction" or "percentage"
	Vec2 lineStartToEnd = lineEndPos - lineStartPos;
	Vec2 point			= lineStartPos + ( time * lineStartToEnd );

	// Fill in raycast information
	Vec2 rayStartToPoint		= point - rayStartPos;
	float rayStartToPointLength = GetProjectedLength2D( rayStartToPoint, rayFwdNormal );
	if ( rayStartToPointLength < 0 )
	{
		return raycastMissResult;
	}

	raycastHitResult.m_impactDist = rayStartToPointLength;
	raycastHitResult.m_impactPos  = rayStartPos + ( raycastHitResult.m_rayForwardNormal * raycastHitResult.m_impactDist );

	// Calculate line i and j (forward and left)
	Vec2 lineFwdNormal  = lineStartToEnd.GetNormalized();
	Vec2 lineLeftNormal = lineFwdNormal.GetRotated90Degrees();

	// Check if impact normal faces correct direction
	if ( ( DotProduct2D( rayFwdNormal, lineLeftNormal ) > 0 ) )
	{
		lineLeftNormal *= -1;
	}

	// Check if too late or early
	if ( raycastHitResult.m_impactDist > rayMaxLength )
	{
		return raycastMissResult;	// Line is too far ahead
	}
	if ( raycastHitResult.m_impactDist < 0 )
	{
		return raycastMissResult;	// Line is too far behind
	}

	raycastHitResult.m_didImpact = true;
	raycastHitResult.m_impactNormal = lineLeftNormal;
	return raycastHitResult;
}

//----------------------------------------------------------------------------------------------------------------------
RaycastResult2D RaycastVsAABB2D( Vec2 rayStartPos, Vec2 rayFwdNormal, float rayMaxLength, AABB2 aabb2 )
{
	// Extra notes
	// This raycast uses 't', aka time, and overlaps on min max ranges to determine when the ray has hit and exited the box.
	// 
	// Enter 1st min range means ray is directly below/above or beside the box.
	// Enter 2nd min range means ray raycast hit the box.							// The instant both ranges overlap, the ray hits.
	// Exit  1st max range means the ray just exited the box.
	// Exit  2nd max range means ray is no longer below/above or beside the box.

	// Initialize raycast variables
	RaycastResult2D raycastHitResult;
	RaycastResult2D raycastMissResult;
	raycastHitResult.m_rayStartPos		= rayStartPos;
	raycastHitResult.m_rayForwardNormal = rayFwdNormal;
	raycastHitResult.m_rayMaxLength		= rayMaxLength;

	// Check if rayStart is already inside a box
	if ( aabb2.IsPointInside( rayStartPos ) )
	{
		Vec2 impactPos					= rayStartPos;
		Vec2 impactNormal				= -1.0f * rayFwdNormal;
		raycastHitResult.m_impactPos	= impactPos;
		raycastHitResult.m_impactNormal = impactNormal;
		raycastHitResult.m_didImpact	= true;
		return raycastHitResult;
	}

	// Calculate 't' for minX, maxX, minY and maxY
	float minX = ( aabb2.m_mins.x - rayStartPos.x );
	float minY = ( aabb2.m_mins.y - rayStartPos.y );
	float maxX = ( aabb2.m_maxs.x - rayStartPos.x );
	float maxY = ( aabb2.m_maxs.y - rayStartPos.y );

	float timeMinX = minX / rayFwdNormal.x;
	float timeMinY = minY / rayFwdNormal.y;
	float timeMaxX = maxX / rayFwdNormal.x;
	float timeMaxY = maxY / rayFwdNormal.y;

	// Set float ranges based on ray dir
	bool isRangeReversedX = false;
	bool isRangeReversedY = false;

	FloatRange floatRangeX;
	FloatRange floatRangeY;
	if ( timeMinX < timeMaxX )
	{
		floatRangeX = FloatRange( timeMinX, timeMaxX );
	}
	else
	{
		floatRangeX = FloatRange( timeMaxX, timeMinX );
		isRangeReversedX = true;
	}
	if ( timeMinY < timeMaxY )
	{
		floatRangeY = FloatRange( timeMinY, timeMaxY );
	}
	else
	{
		floatRangeY = FloatRange( timeMaxY, timeMinY );
		isRangeReversedY = true;
	}
	// Determine hit result since X and Y ranges overlap
	if ( floatRangeX.IsOverlapping( floatRangeY ) )
	{
		raycastHitResult.m_didImpact = true;

		float overlapStartTime = -1.0f;

		// Choose earlier side between X or Y. 
		// The higher min value gets hit first
		// floatRangr.Min is 't'
		// rayHitPos = start + ( fwd * 't' )
		if ( floatRangeX.m_min > floatRangeY.m_min )
		{
			if ( floatRangeX.m_min < 0 )
			{
				return raycastMissResult;
			}

			// Raycast will hit bottom or top of AABB2 first
			overlapStartTime				= floatRangeX.m_min;
			Vec2 impactPoint				= rayStartPos + ( floatRangeX.m_min * rayFwdNormal );
			raycastHitResult.m_impactPos	= impactPoint;
		}
		else
		{
			// Raycast will hit left or right of AABB2 first
			overlapStartTime				= floatRangeY.m_min;
			Vec2 impactPoint				= rayStartPos + ( floatRangeY.m_min * rayFwdNormal );
			raycastHitResult.m_impactPos	= impactPoint;
		}

		if ( overlapStartTime < 0 )
		{
			// Edge case checking if box is "behind" the ray
			return raycastMissResult;
		}

		float impactDist = ( raycastHitResult.m_impactPos - rayStartPos ).GetLength();
		raycastHitResult.m_impactDist = impactDist;

		// Distance check to ensure ray is long enough to "reach" the box
		if ( impactDist > rayMaxLength )
		{
			return raycastMissResult;
		}

		// Impact normal check
		if ( raycastHitResult.m_impactPos.x == aabb2.m_mins.x )				// Left impact normal
		{
			Vec2 impactNormal = Vec2( -1.0f, 0.0f );
			raycastHitResult.m_impactNormal = impactNormal;
		}
		else if ( raycastHitResult.m_impactPos.x == aabb2.m_maxs.x )		// Right impact normal
		{
			Vec2 impactNormal = Vec2( 1.0f, 0.0f );
			raycastHitResult.m_impactNormal = impactNormal;
		}
		else if ( raycastHitResult.m_impactPos.y == aabb2.m_mins.y )		// Bottom impact normal
		{
			Vec2 impactNormal = Vec2( 0.0f, -1.0f );
			raycastHitResult.m_impactNormal = impactNormal;
		}
		else if ( raycastHitResult.m_impactPos.y == aabb2.m_maxs.y )		// Top impact normal
		{
			Vec2 impactNormal = Vec2( 0.0f, 1.0f );
			raycastHitResult.m_impactNormal = impactNormal;
		}

		//		raycastHitResult.m_exitDist 
		//		Vec2 impactNormal				= ( raycastHitResult.m_impactPos - rayStartPos ).GetNormalized();
		//		raycastHitResult.m_impactNormal = impactNormal;
		return raycastHitResult;
	}

	return raycastMissResult;
}


//----------------------------------------------------------------------------------------------------------------------
RaycastResult2D RaycastVsOBB2D( Vec2 rayStartPos, Vec2 rayFwdNormal, float rayMaxLength, OBB2D obb2 )
{
	Vec2 rayEndPos_WorldSpace	= rayStartPos + ( rayFwdNormal * rayMaxLength );
	Vec2 rayStart_LocalPos		= obb2.GetlocalPosFromWorldPos( rayStartPos );
	Vec2 rayEndPos_LocalPos		= obb2.GetlocalPosFromWorldPos( rayEndPos_WorldSpace );
	Vec2 rayFwd_LocalSpace		= ( rayEndPos_LocalPos - rayStart_LocalPos ).GetNormalized();

	// Initialize raycast variables
	RaycastResult2D raycastHitResult_WorldSpace;
	RaycastResult2D raycastHitResult_LocalSpace;
	RaycastResult2D raycastMissResult;

	Vec2 jBasis = obb2.m_iBasisNormal.GetRotated90Degrees();
	Vec2 maxs	= obb2.m_center + ( obb2.m_halfDimensions.x * obb2.m_iBasisNormal ) + ( obb2.m_halfDimensions.y * jBasis );
	Vec2 mins	= obb2.m_center - ( obb2.m_halfDimensions.x * obb2.m_iBasisNormal ) - ( obb2.m_halfDimensions.y * jBasis );
	//	AABB2 aabb2							= AABB2( mins, maxs );
	AABB2 aabb2					= AABB2( obb2.m_center, obb2.m_halfDimensions.x * 2.0f, obb2.m_halfDimensions.y * 2.0f );
	raycastHitResult_LocalSpace = RaycastVsAABB2D( rayStart_LocalPos, rayFwd_LocalSpace, rayMaxLength, aabb2 );
	//	raycastHitResult					= RaycastVsAABB2D( rayStartPos, rayFwdNormal, rayMaxLength, aabb2 );

	Vec2 impactPos_LocalSpace					= raycastHitResult_LocalSpace.m_impactPos;
	Vec2 imactNormalEndPos_LocalSpace			= impactPos_LocalSpace + raycastHitResult_LocalSpace.m_impactNormal;
	Vec2 imactNormalEndPos_WorldSpace			= obb2.GetWorldPosFromLocalPos( imactNormalEndPos_LocalSpace );
	Vec2 impactPos_WorldSpace					= obb2.GetWorldPosFromLocalPos( impactPos_LocalSpace );
	raycastHitResult_WorldSpace.m_impactPos		= impactPos_WorldSpace;
	Vec2 impactNormal_WorldSpace				= ( imactNormalEndPos_WorldSpace - impactPos_WorldSpace ).GetNormalized();
	raycastHitResult_WorldSpace.m_impactPos		= impactPos_WorldSpace;
	raycastHitResult_WorldSpace.m_impactNormal	= impactNormal_WorldSpace;

	return raycastHitResult_WorldSpace;
}


//----------------------------------------------------------------------------------------------------------------------
RaycastResult3D RaycastVsCylinder3D( Vec3 const& rayStartPos, Vec3 const& fwdNormal, float rayLength, Vec2 const& discCenter, float actorMinZ, float actorMaxZ, float discRadius )
{
	RaycastResult3D raycastHitResult;
	RaycastResult3D raycastMissResult;
	RaycastResult2D raycastAgainstDisc2D;

	// Make V2 versions
	Vec2 rayStartPos2D	= Vec2( rayStartPos.x, rayStartPos.y );
	Vec2 fwdNormalV2	= Vec2( fwdNormal.x, fwdNormal.y );

	Vec2  rayXY = fwdNormalV2 * rayLength;

	// Calculate raycast
	raycastAgainstDisc2D = RaycastVsDisc2D( rayStartPos2D, fwdNormalV2.GetNormalized(), rayLength, discCenter, discRadius );
	if ( raycastAgainstDisc2D.m_didImpact )
	{
		// Calculate scale and hitPosZ
		float enterDiscTime = ( raycastAgainstDisc2D.m_impactPos.x - raycastAgainstDisc2D.m_rayStartPos.x ) / fwdNormal.x;
		float hitPosZ		= rayStartPos.z + ( enterDiscTime * fwdNormal.z );

		// Hit Body
		// Check if raycastVector is within Z range
		if ( hitPosZ > actorMinZ && hitPosZ < actorMaxZ )
		{
			raycastHitResult.m_didImpact		= true;
			raycastHitResult.m_impactPos		= Vec3( raycastAgainstDisc2D.m_impactPos.x, raycastAgainstDisc2D.m_impactPos.y, hitPosZ );
			raycastHitResult.m_impactDist		= ( raycastHitResult.m_impactPos - rayStartPos ).GetLength();
			raycastHitResult.m_impactNormal		= Vec3( raycastAgainstDisc2D.m_impactNormal.x, raycastAgainstDisc2D.m_impactNormal.y, 0.0f );
			raycastHitResult.m_rayFwdNormal		= fwdNormal;
			raycastHitResult.m_rayMaxLength		= rayLength;
			raycastHitResult.m_rayStartPosition = rayStartPos;
			return raycastHitResult;
		}
		else
		{
			Vec3 raycastEndPos = rayStartPos + ( fwdNormal * rayLength );

			// Hit Bottom
			// Check if vectorEndPos is lower than actor's min and ray is shooting upwards
			if ( ( rayStartPos.z <= actorMinZ && raycastEndPos.z >= actorMinZ && fwdNormal.z > 0.0f ) && raycastAgainstDisc2D.m_didImpact )
			{
				float time				= ( actorMinZ - rayStartPos.z ) / fwdNormal.z;
				Vec3 enterCylinderPos	= rayStartPos + ( time * fwdNormal );
				Vec2 enterCylinderPosV2 = Vec2( enterCylinderPos.x, enterCylinderPos.y );

				if ( IsPointInsideDisc2D( enterCylinderPosV2, discCenter, discRadius ) )
				{
					raycastHitResult.m_didImpact		= true;
					raycastHitResult.m_impactPos		= enterCylinderPos;
					raycastHitResult.m_impactDist		= ( raycastHitResult.m_impactPos - rayStartPos ).GetLength();
					raycastHitResult.m_impactNormal		= Vec3( 0.0f, 0.0f, -1.0f );
					raycastHitResult.m_rayFwdNormal		= fwdNormal;
					raycastHitResult.m_rayMaxLength		= rayLength;
					raycastHitResult.m_rayStartPosition = rayStartPos;
					return raycastHitResult;
				}
				else
				{
					return raycastMissResult;
				}
			}

			// Hit Top
			// Check if vectorEndPos is higher than actor's max and ray is shooting downwards
			if ( ( rayStartPos.z >= actorMaxZ && raycastEndPos.z < actorMaxZ && fwdNormal.z < 0.0f ) && raycastAgainstDisc2D.m_didImpact )
			{
				float time				= ( actorMaxZ - rayStartPos.z ) / fwdNormal.z;
				Vec3 exitCylinderPos	= rayStartPos + ( time * fwdNormal );
				Vec2 exitCylinderPosV2	= Vec2( exitCylinderPos.x, exitCylinderPos.y );

				if ( IsPointInsideDisc2D( exitCylinderPosV2, discCenter, discRadius ) )
				{
					raycastHitResult.m_didImpact		= true;
					raycastHitResult.m_impactPos		= exitCylinderPos;
					raycastHitResult.m_impactDist		= ( raycastHitResult.m_impactPos - rayStartPos ).GetLength();
					raycastHitResult.m_impactNormal		= Vec3( 0.0f, 0.0f, 1.0f );
					raycastHitResult.m_rayFwdNormal		= fwdNormal;
					raycastHitResult.m_rayMaxLength		= rayLength;
					raycastHitResult.m_rayStartPosition = rayStartPos;
					return raycastHitResult;
				}
				else
				{
					return raycastMissResult;
				}
			}
		}
	}
	else
	{
		return raycastMissResult;
	}

	return raycastHitResult;
}


//----------------------------------------------------------------------------------------------------------------------
RaycastResult3D RaycastVsAABB3D( Vec3 rayStartPos, Vec3 rayFwdNormal, float rayMaxLength, AABB3 aabb3 )
{
	// Extra notes
	// This raycast uses 't', aka time, and overlaps on min max ranges to determine when the ray has hit and exited the box.
	// 
	// Enter 1st min range means ray is directly below/above or beside the box.
	// Enter 2nd min range means ray raycast hit the box.							// The instant both ranges overlap, the ray hits.
	// Exit  1st max range means the ray just exited the box.
	// Exit  2nd max range means ray is no longer below/above or beside the box.

	// Initialize raycast variables
	RaycastResult3D raycastHitResult;
	RaycastResult3D raycastMissResult;
	raycastHitResult.m_rayStartPosition = rayStartPos;
	raycastHitResult.m_rayFwdNormal		= rayFwdNormal;
	raycastHitResult.m_rayMaxLength		= rayMaxLength;

	// Check if rayStart is already inside a box
	if ( aabb3.IsPointInside( rayStartPos ) )
	{
		Vec3 impactPos					= rayStartPos;
		Vec3 impactNormal				= -1.0f * rayFwdNormal;
		raycastHitResult.m_impactPos	= impactPos;
		raycastHitResult.m_impactNormal = impactNormal;
		raycastHitResult.m_didImpact	= true;
		return raycastHitResult;
	}

	// Calculate 't' for minX, maxX, minY and maxY
	float minX = ( aabb3.m_mins.x - rayStartPos.x );
	float minY = ( aabb3.m_mins.y - rayStartPos.y );
	float minZ = ( aabb3.m_mins.z - rayStartPos.z );
	float maxX = ( aabb3.m_maxs.x - rayStartPos.x );
	float maxY = ( aabb3.m_maxs.y - rayStartPos.y );
	float maxZ = ( aabb3.m_maxs.z - rayStartPos.z );

	float timeMinX = minX / rayFwdNormal.x;
	float timeMinY = minY / rayFwdNormal.y;
	float timeMinZ = minZ / rayFwdNormal.z;
	float timeMaxX = maxX / rayFwdNormal.x;
	float timeMaxY = maxY / rayFwdNormal.y;
	float timeMaxZ = maxZ / rayFwdNormal.z;

	// Set float ranges based on ray dir
	bool isRangeReversedX = false;
	bool isRangeReversedY = false;
	bool isRangeReversedZ = false;

	FloatRange floatRangeX;
	FloatRange floatRangeY;
	FloatRange floatRangeZ;
	if ( timeMinX < timeMaxX )
	{
		floatRangeX = FloatRange( timeMinX, timeMaxX );
	}
	else
	{
		floatRangeX = FloatRange( timeMaxX, timeMinX );
		isRangeReversedX = true;
	}
	if ( timeMinY < timeMaxY )
	{
		floatRangeY = FloatRange( timeMinY, timeMaxY );
	}
	else
	{
		floatRangeY = FloatRange( timeMaxY, timeMinY );
		isRangeReversedY = true;
	}
	if ( timeMinZ < timeMaxZ )
	{
		floatRangeZ = FloatRange( timeMinZ, timeMaxZ );
	}
	else
	{
		floatRangeZ = FloatRange( timeMaxZ, timeMinZ );
		isRangeReversedZ = true;
	}
	// Determine hit result since X and Y ranges overlap
	if ( floatRangeX.IsOverlapping( floatRangeY ) &&
		floatRangeX.IsOverlapping( floatRangeZ ) &&
		floatRangeY.IsOverlapping( floatRangeZ )
		)
	{
		raycastHitResult.m_didImpact = true;

		float overlapStartTime = -1.0f;

		// Choose earlier side between X or Y. 
		// The higher min value gets hit first
		// floatRangr.Min is 't'
		// rayHitPos = start + ( fwd * 't' )
		if ( floatRangeX.m_min > floatRangeY.m_min &&
			floatRangeX.m_min > floatRangeZ.m_min )
		{
			if ( floatRangeX.m_min < 0 )
			{
				return raycastMissResult;
			}

			// Raycast will hit bottom or top of AABB2 first
			overlapStartTime = floatRangeX.m_min;
			Vec3 impactPoint = rayStartPos + ( floatRangeX.m_min * rayFwdNormal );
			raycastHitResult.m_impactPos = impactPoint;
		}
		else if ( floatRangeY.m_min > floatRangeX.m_min &&
			floatRangeY.m_min > floatRangeZ.m_min )
		{
			if ( floatRangeY.m_min < 0 )
			{
				return raycastMissResult;
			}

			// Raycast will hit left or right of AABB2 first
			overlapStartTime = floatRangeY.m_min;
			Vec3 impactPoint = rayStartPos + ( floatRangeY.m_min * rayFwdNormal );
			raycastHitResult.m_impactPos = impactPoint;
		}
		else
		{
			if ( floatRangeZ.m_min < 0 )
			{
				return raycastMissResult;
			}

			// Raycast is happening on Z axis
			overlapStartTime = floatRangeZ.m_min;
			Vec3 impactPoint = rayStartPos + ( floatRangeZ.m_min * rayFwdNormal );
			raycastHitResult.m_impactPos = impactPoint;
		}

		if ( overlapStartTime < 0 )
		{
			// Edge case checking if box is "behind" the ray
			return raycastMissResult;
		}

		float impactDist = ( raycastHitResult.m_impactPos - rayStartPos ).GetLength();
		raycastHitResult.m_impactDist = impactDist;

		// Distance check to ensure ray is long enough to "reach" the box
		if ( impactDist > rayMaxLength )
		{
			return raycastMissResult;
		}

		// Impact normal check
		if ( CompareIfFloatsAreEqual( raycastHitResult.m_impactPos.x, aabb3.m_mins.x, 0.1f ) )				// Left impact normal
		{
			Vec3 impactNormal = Vec3( -1.0f, 0.0f, 0.0f );
			raycastHitResult.m_impactNormal = impactNormal;
		}
		else if ( CompareIfFloatsAreEqual( raycastHitResult.m_impactPos.x, aabb3.m_maxs.x, 0.1f ) )		// Right impact normal
		{
			Vec3 impactNormal = Vec3( 1.0f, 0.0f, 0.0f );
			raycastHitResult.m_impactNormal = impactNormal;
		}
		else if ( CompareIfFloatsAreEqual( raycastHitResult.m_impactPos.y, aabb3.m_mins.y, 0.1f ) )		// Bottom impact normal
		{
			Vec3 impactNormal = Vec3( 0.0f, -1.0f, 0.0f );
			raycastHitResult.m_impactNormal = impactNormal;
		}
		else if ( CompareIfFloatsAreEqual( raycastHitResult.m_impactPos.y, aabb3.m_maxs.y, 0.0f ) )		// Top impact normal
		{
			Vec3 impactNormal = Vec3( 0.0f, 1.0f, 0.0f );
			raycastHitResult.m_impactNormal = impactNormal;
		}
		else if ( CompareIfFloatsAreEqual( raycastHitResult.m_impactPos.z, aabb3.m_mins.z, 0.1f ) )		// Bottom impact normal
		{
			Vec3 impactNormal = Vec3( 0.0f, 0.0f, -1.0f );
			raycastHitResult.m_impactNormal = impactNormal;
		}
		else if ( CompareIfFloatsAreEqual( raycastHitResult.m_impactPos.z, aabb3.m_maxs.z, 0.0f ) )		// Top impact normal
		{
			Vec3 impactNormal = Vec3( 0.0f, 0.0f, 1.0f );
			raycastHitResult.m_impactNormal = impactNormal;
		}

		return raycastHitResult;
	}

	return raycastMissResult;
}


//----------------------------------------------------------------------------------------------------------------------
RaycastResult3D RaycastVsOBB3D( Vec3 rayStartPos, Vec3 rayFwdNormal, float rayMaxLength, OBB3D aabb3 )
{
	return RaycastResult3D();
}


//----------------------------------------------------------------------------------------------------------------------
RaycastResult3D RaycastVsTriangle( Vec3 const& rayStart, Vec3 const& rayFwdDir, float rayLength, Vec3 const& vert0, Vec3 const& vert1, Vec3 const& vert2, float& t, float& u, float& v )
{
	// Init raycastResult
	RaycastResult3D rayHitResult;
	rayHitResult.m_rayStartPosition = rayStart;
	rayHitResult.m_didImpact		= false;
	rayHitResult.m_rayFwdNormal		= rayFwdDir;
	rayHitResult.m_rayMaxLength		= rayLength;
	// Fill in hit info if rayHitTri
	bool doesRayHitTri = DoesRaycastHitTriangle( rayStart, rayFwdDir, vert0, vert1, vert2, t, u, v );
	if ( doesRayHitTri )
	{
		rayHitResult.m_didImpact		= true;
		rayHitResult.m_rayStartPosition = rayStart;
		Vec3 v0v1						= vert1 - vert0;
		Vec3 v0v2						= vert2 - vert0;
		rayHitResult.m_impactNormal		= CrossProduct3D( v0v1, v0v2 );
		rayHitResult.m_impactNormal.Normalize();
		rayHitResult.m_rayFwdNormal		= rayFwdDir;
		// Compute hit position from (u,t,v)
//		Vec3 hitPos						= ( u * vert0 ) + ( v * vert1 ) + ( t * vert2 );
		Vec3 uNess = ( vert1 - vert0 ) * u;
		Vec3 vNess = ( vert2 - vert0 ) * v;
		Vec3 hitPos						= vert0 + uNess + vNess;
		rayHitResult.m_impactPos		= hitPos;
		rayHitResult.m_impactDist		= ( hitPos - rayStart ).GetLength();
	}
	return rayHitResult;
}


//----------------------------------------------------------------------------------------------------------------------
bool DoesRaycastHitTriangle( Vec3 const& rayStart, Vec3 const& rayFwdDir, Vec3 const& vert0, Vec3 const& vert1, Vec3 const& vert2, float& t, float& u, float& v )
{
	// Init common variables
	Vec3 v0v1			= vert1 - vert0;
	Vec3 v0v2			= vert2 - vert0;
	Vec3 uNessVector	= CrossProduct3D( rayFwdDir, v0v2 );		// Same as "u" if ray is perpendicular to tri
	float determinant	= DotProduct3D( v0v1, uNessVector );

	// Test early out and Cull triangle back face
	// if determinant is negative, ray hits tri back face
	// if determinant is near 0, ray is parallel to tri
	if ( determinant < 0.0f )
	{
		return false;
	}
	if ( CompareIfFloatsAreEqual( fabsf( determinant ), 0.0f, 0.00001f ) )
	{
		return false;
	}

	// Compute ray projected onto tri expressed in ( u, v, t )
	float invDeterminant = 1.0f / determinant;			// Inverse in compute for performance reasons (multiply instead of divide)

	// Compute U by projecting v0ToRayStart onto uNessVector
	Vec3 v0ToRayStart	= rayStart - vert0;
	u					= DotProduct3D( v0ToRayStart, uNessVector ) * invDeterminant;
	if ( u < 0.0f || u > 1.0f )
	{
		return false;
	}

	// Compute V by projecting rayFwdDir onto vNessVector
	Vec3 vNessVector = CrossProduct3D( v0ToRayStart, v0v1 );
	v				 = DotProduct3D( rayFwdDir, vNessVector ) * invDeterminant;
	if ( (v < 0.0f) || ((u + v) > 1.0f) )
	{
		return false;
	}

	// T
	t  = DotProduct3D( v0v2, vNessVector );
	t *= invDeterminant;

	return true;
}


//----------------------------------------------------------------------------------------------------------------------
float GetClamped(float value, float minValue, float maxValue)
{
	if (value >= maxValue)
	{
		return maxValue;
	}
	if (value <= minValue)
	{
		return minValue;
	}
	return value;
}

//----------------------------------------------------------------------------------------------------------------------
float GetClampedZeroToOne(float value)
{
	if (value >= 1.0f)
	{
		return 1.0f;
	}
	else if (value <= 0.0f)
	{
		return 0.0f;
	}
	return value;
}

//----------------------------------------------------------------------------------------------------------------------
float Interpolate(float start, float end, float fractionTowardEnd)
{
	float distance	 = end - start;
	float percentage = distance * fractionTowardEnd;
	float newValue	 = start + percentage; 
	return newValue;
}

//----------------------------------------------------------------------------------------------------------------------
Vec2 Interpolate( Vec2 start, Vec2 end, float fractionTowardEnd )
{
	Vec2 lerpedResult = start + ( end - start ) * fractionTowardEnd; 
	return lerpedResult;
}

//----------------------------------------------------------------------------------------------------------------------
Vec3 Interpolate( Vec3 start, Vec3 end, float fractionTowardEnd )
{
	Vec3 lerpedResult = start + ( end - start ) * fractionTowardEnd;
	return lerpedResult;
}

//----------------------------------------------------------------------------------------------------------------------
Rgba8 Interpolate( Rgba8 startColor, Rgba8 endColor, float fractionTowardEnd )
{
	Rgba8 blendedColor;
	blendedColor.r = static_cast<char>( Interpolate( startColor.r, endColor.r, fractionTowardEnd ) );
	blendedColor.g = static_cast<char>( Interpolate( startColor.g, endColor.g, fractionTowardEnd ) );
	blendedColor.b = static_cast<char>( Interpolate( startColor.b, endColor.b, fractionTowardEnd ) );
	blendedColor.a = static_cast<char>( Interpolate( startColor.a, endColor.a, fractionTowardEnd ) );
	return blendedColor;
}

//----------------------------------------------------------------------------------------------------------------------
float GetFractionWithinRange(float value, float rangeStart, float rangeEnd) 
{
	float length		= (rangeEnd - rangeStart);
	float pointOnLength = (value - rangeStart); 
	float fraction		= pointOnLength / length;
	return fraction;
}

//----------------------------------------------------------------------------------------------------------------------
float RangeMap(float valueToConvert, float originalRangeStart, float originalRangeEnd, float convertedRangeStart, float convertedRangeEnd) // Renamed variables from in/out start/end
{
	float fractionInValue			= GetFractionWithinRange( valueToConvert, originalRangeStart, originalRangeEnd) ;
	float range						= convertedRangeEnd - convertedRangeStart;
	float pointOnRangeMinusStart	= range * fractionInValue;
	float pointOnRangePlusStart		= pointOnRangeMinusStart + convertedRangeStart;
	return pointOnRangePlusStart;
}

//----------------------------------------------------------------------------------------------------------------------
// Note: InStart and InEnd can only vary from (lower - higher) ranges
// Logic is broke for (higher - lower) ranges
//----------------------------------------------------------------------------------------------------------------------
float RangeMapClamped(float inValue, float inStart, float inEnd, float outStart, float outEnd)
{
	if ( inValue <= inStart )
	{
		return outStart;
	}
	else if ( inValue >= inEnd )
	{
		return outEnd;
	}

	float rangeMap = RangeMap( inValue, inStart, inEnd, outStart, outEnd );

	return rangeMap;
}

//----------------------------------------------------------------------------------------------------------------------
int RoundDownToInt(float value)
{
	return static_cast<int>(floorf( value ));
}

//----------------------------------------------------------------------------------------------------------------------
bool CompareIfFloatsAreEqual( float valueA, float valueB, float tolerance )
{
	if  ( valueA >= ( valueB - tolerance ) &&
		  valueA <= ( valueB + tolerance )
		)
	{
		return true;
	}
	return false;
}


//----------------------------------------------------------------------------------------------------------------------
bool IsFloatWithinRange_Inclusive( float min, float max, float value )
{
	if ( value >= min && value <= max )
	{
		return true;
	}
	return false;
}


//----------------------------------------------------------------------------------------------------------------------
Mat44 ComputeI_Fwd_J_Left_K_Up( Vec3 const& iBasisStart, Vec3 const& iBasisEnd )
{
	Mat44 matrix;
	Vec3 dispSE		= iBasisEnd - iBasisStart;
	Vec3 zSkyward	= Vec3( 0.0f, 0.0f, 1.0f );
	Vec3 iBasis		= dispSE.GetNormalized();
	Vec3 jBasis		= CrossProduct3D( zSkyward, iBasis );
	jBasis.Normalize();
	Vec3 kBasis		= zSkyward;
	// Set jBasis to y-axis if iBasis is parallel to zBasis
	if ( jBasis == Vec3(0.0f, 0.0f, 0.0f) )
	{
		// Use world left instead
		jBasis = Vec3( 0.0f, 1.0f, 0.0f );
		kBasis = CrossProduct3D( iBasis, jBasis );
		kBasis.Normalize();
		jBasis = CrossProduct3D( kBasis, iBasis );
		jBasis.Normalize();
	}
	else
	{
		kBasis = CrossProduct3D( iBasis, jBasis );
		kBasis.Normalize();
	}
	matrix.SetIJK3D( iBasis, jBasis, kBasis );
	matrix.SetTranslation3D( iBasisStart );
	return matrix;
}


//----------------------------------------------------------------------------------------------------------------------
float GetShortestAngularDispDegrees(float startDegrees, float endDegrees )
{
	// if angle to turn is > 180 degrees, then turn the other way
	
	float circle = 360.0f;

	while (startDegrees > circle)
	{
		startDegrees -= circle;
	}
	while ( endDegrees > circle)
	{
		endDegrees -= circle;
	}
	while (startDegrees < 0 )
	{
		startDegrees += circle;
	}
	while ( endDegrees < 0)
	{
		endDegrees += circle;
	}

	float angDisp = endDegrees - startDegrees;
	float absAngDisp = fabsf( angDisp );

	if (absAngDisp <= 180.0f)
	{
		return angDisp;
	}
	else if ( angDisp > 180.0f)
	{
		return angDisp - circle;
	}
	else if ( angDisp < -180.0f)
	{
		return circle + angDisp;
	}

	return angDisp;
}

//----------------------------------------------------------------------------------------------------------------------
float GetTurnedTowardDegrees(float currentDegrees, float goalDegrees, float maxDeltaDegrees)
{
	float distance = GetShortestAngularDispDegrees(currentDegrees, goalDegrees);
	 
	if (distance > 0)
	{
		if (distance <= maxDeltaDegrees) 
		{
			return goalDegrees;
		}
		else if (distance > maxDeltaDegrees)
		{
			currentDegrees += maxDeltaDegrees;
			return currentDegrees;
		} 
	}
	else
	{
		if (distance <= -maxDeltaDegrees)
		{
			currentDegrees -= maxDeltaDegrees;
			return currentDegrees;
		}
		else if ( distance > -maxDeltaDegrees)
		{
			return goalDegrees;
		}
	}
	return maxDeltaDegrees;
}


//----------------------------------------------------------------------------------------------------------------------
float GetAngleDegreesBetweenVectors2D(Vec2 const& a, Vec2 const& b)
{
	Vec2 directionA				= a.GetNormalized();
	Vec2 directionB				= b.GetNormalized();
	float similarity			= DotProduct2D( directionA, directionB );
	float clampedSimiliarity	= GetClamped( similarity, -1.0f, 1.0f );
	float thetaRadians			= acosf( clampedSimiliarity );
	float thetaDegrees			= ConvertRadiansToDegrees( thetaRadians );
	return thetaDegrees;
}


//----------------------------------------------------------------------------------------------------------------------
float GetAngleDegreesBetweenVectors3D( Vec3 const& a, Vec3 const& b ) 
{
	Vec3  directionA			= a.GetNormalized();
	Vec3  directionB			= b.GetNormalized();
	float similarity			= DotProduct3D( directionA, directionB );
	float clampedSimiliarity	= GetClamped( similarity, -1.0f, 1.0f );
	float thetaRadians			= acosf( clampedSimiliarity );
	float thetaDegrees			= ConvertRadiansToDegrees( thetaRadians );
	return thetaDegrees;
}


//----------------------------------------------------------------------------------------------------------------------
// referenceVector	= the "fwd" vector which we consider to be at 0 degrees
// vectorToCompare	= the vector we will use to compute the signed angle, relative to the reference vector
// normalVector		= the vector perpendicular to both the 'referenceVector' and the 'vectorToCompare'
// returns			= The signed angle between the 'referenceVector' and the 'otherVector' (range of -179.9f to 180.0f)
float GetSignedAngleDegreesBetweenVectors( Vec3 const& referenceVector, Vec3 const& vectorToCompare, Vec3 const& normalVector )
{
	float unsignedAngle			= GetAngleDegreesBetweenVectors3D( referenceVector, vectorToCompare );
	Vec3  perpendicularVector	= CrossProduct3D( referenceVector, vectorToCompare );
	perpendicularVector.Normalize();
	float signedAngle			= DotProduct3D( perpendicularVector, normalVector );
	if ( signedAngle >= 0.0f )
	{
		signedAngle = 1.0f;
	}
	else
	{
		signedAngle = -1.0f;
	}
	signedAngle *= unsignedAngle;
	return signedAngle;
}


//----------------------------------------------------------------------------------------------------------------------
Vec3 RotateVectorAboutArbitraryAxis( Vec3 vectorToRotate, Vec3 arbitraryAxis, float degreesToTurn )
{
	Vec3 rotatedVector;
	Vec3 a = ( CosDegrees( degreesToTurn ) * vectorToRotate );
	Vec3 b = ( SinDegrees( degreesToTurn ) * ( CrossProduct3D( arbitraryAxis, vectorToRotate ) ) );
	Vec3 c = ( 1 - CosDegrees( degreesToTurn ) ) * ( DotProduct3D( arbitraryAxis, vectorToRotate ) * arbitraryAxis ); 
	rotatedVector = a + b + c; 
	return rotatedVector;
}


//----------------------------------------------------------------------------------------------------------------------
float DotProduct2D(Vec2 const& a, Vec2 const& b)
{
	float dotProduct = ( a.x * b.x ) + ( a.y * b.y );
	return dotProduct;
}

//----------------------------------------------------------------------------------------------------------------------
float DotProduct3D( Vec3 const& a, Vec3 const& b )
{
	float dotProduct = ( a.x * b.x ) + ( a.y * b.y ) + ( a.z * b.z );
	return dotProduct;
}

//----------------------------------------------------------------------------------------------------------------------
float DotProduct4D( Vec4 const& a, Vec4 const& b )
{
	float dotProduct = ( (a.x * b.x) + (a.y * b.y) + (a.z * b.z) + (a.w * b.w) );
	return dotProduct;
}

//----------------------------------------------------------------------------------------------------------------------
float CrossProduct2D( Vec2 const& a, Vec2 const& b )
{
	float crossProduct2D = a.x*b.y - a.y*b.x ;
	return crossProduct2D;
}

//----------------------------------------------------------------------------------------------------------------------
Vec3 CrossProduct3D( Vec3 const& a, Vec3 const& b )
{
	Vec3 crossProduct3D = Vec3( (a.y*b.z - a.z*b.y), (a.z*b.x - a.x*b.z), (a.x*b.y - a.y*b.x) ); 
	return crossProduct3D;
}

//----------------------------------------------------------------------------------------------------------------------
float NormalizeByte( unsigned char byteValue )
{
	float normalizedByte = RangeMap( static_cast<float>( byteValue ), 0.0f, 255.0f, 0.0f, 1.0f );
	return normalizedByte;
}

//----------------------------------------------------------------------------------------------------------------------
unsigned char DenormalizeByte( float zeroToOne )
{
	float deNormalizeByte = RangeMap( zeroToOne, 0.0f, 1.0f, 0.0f, 256.0f );
//	float deNormalizeByte = RangeMapClamped( zeroToOne, 0.0f, 1.0f, 0.0f, 256.0f );
	
	if ( deNormalizeByte > 255.0f )
	{
		deNormalizeByte = 255.0f;
	}
	
	return static_cast<unsigned char>( deNormalizeByte );
}
