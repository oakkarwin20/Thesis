#include "Game/FoodManager.hpp"
#include "Game/GameMode3D.hpp"
#include "Game/Quadruped.hpp"

#include "Engine/Core/Time.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/SkeletalSystem/IK_Chain3D.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/VertexUtils.hpp"

//----------------------------------------------------------------------------------------------------------------------
FoodManager::FoodManager( int numFoodOrbs, GameMode3D* game ) 
{
	m_game = game;
	for ( int i = 0; i < numFoodOrbs; i++ )
	{
		Vec3	randPos						= GetRandPos_MaxZ();
		Vec3	randGoalPos					= GetRandPos_MaxZ();
		FoodOrb newFoodOrb					= FoodOrb( randPos );
		newFoodOrb.m_goalPos				= randGoalPos;
		// Create legs for "spider"
		newFoodOrb.m_quadSpider				= new Quadruped( game, randPos );
		std::string legName					= Stringf("leg %0.2f", i); 
		newFoodOrb.m_quadSpider->CreateChildSkeletalSystem(	legName, Vec3::ZERO );
		newFoodOrb.m_quadSpider->CreateLimbsForIKChain( legName, 2.0f, 10.0f, Vec3::X_FWD, JOINT_CONSTRAINT_TYPE_EULER );
		newFoodOrb.m_quadSpider->m_rightArm	= newFoodOrb.m_quadSpider->GetSkeletonByName( legName );
		m_foodList.push_back( newFoodOrb );
	}

}

//----------------------------------------------------------------------------------------------------------------------
FoodManager::~FoodManager()
{
	m_game = nullptr;
}


//----------------------------------------------------------------------------------------------------------------------
void FoodManager::Update( float deltaSeconds )
{
	MoveFoodOrbs( deltaSeconds );
}

//----------------------------------------------------------------------------------------------------------------------
void FoodManager::Render( std::vector<Vertex_PCU>& verts )
{
	for ( int i = 0; i < m_foodList.size(); i++ )
	{
		FoodOrb& currentFoodOrb = m_foodList[ i ];
		if ( !currentFoodOrb.m_isConsumed )
		{
			AddVertsForSphere3D( verts, currentFoodOrb.m_position, 2.0f, 8.0f, 8.0f, Rgba8::YELLOW );
			AddVertsForSphere3D( verts, currentFoodOrb.m_goalPos, 2.0f, 8.0f, 8.0f, Rgba8::DARKER_RED );
			currentFoodOrb.m_quadSpider->Render( verts, Rgba8::DARK_GRAY, Rgba8::WHITE );
			AddVertsForSphere3D( verts, currentFoodOrb.m_quadSpider->m_rightArm->m_firstJoint->m_poleVector, 2.0f, 8.0f, 8.0f, Rgba8::MAGENTA );
			currentFoodOrb.m_quadSpider->m_rightArm->RenderTarget_EE( verts );
			currentFoodOrb.m_quadSpider->m_rightArm->RenderTarget_IJK( verts, 10.0f );
		}
	}
}


//----------------------------------------------------------------------------------------------------------------------
Vec3 FoodManager::GetRandPos_MaxZ()
{
	AABB3 abb3   = m_game->m_floor_SE->m_aabb3;
	Vec3 randPos = Vec3::ZERO;
	randPos.x	 = g_theRNG->RollRandomFloatInRange( abb3.m_mins.x, abb3.m_maxs.x );
	randPos.y	 = g_theRNG->RollRandomFloatInRange( abb3.m_mins.y, abb3.m_maxs.y );
	randPos.z	 = abb3.m_maxs.z + 0.5f;
	return randPos;
}


//----------------------------------------------------------------------------------------------------------------------
void FoodManager::ConsumeFood( FoodOrb& foodToConsume )
{
	foodToConsume.m_isConsumed = true;
}


//----------------------------------------------------------------------------------------------------------------------
void FoodManager::MoveFoodOrbs( float deltaSeconds )
{
	for ( int i = 0; i < m_foodList.size(); i++ )
	{
		FoodOrb& currentFoodOrb = m_foodList[ i ];
		if ( !currentFoodOrb.m_isConsumed && !currentFoodOrb.m_isGrabbed )
		{
			// Check dist from goal pos
			float distCurPosToGoal = GetDistance3D( currentFoodOrb.m_position, currentFoodOrb.m_goalPos );
			// if close enough, generate new goal pos
			if ( distCurPosToGoal < 1.0f )
			{
				currentFoodOrb.m_prevPos = currentFoodOrb.m_goalPos;
				currentFoodOrb.m_goalPos = GetRandPos_MaxZ();
			}
			else
			{
				// if too far, lerp towards goal pos
				float fractionTowardsEnd	= deltaSeconds * 0.5f;
				currentFoodOrb.m_position	= Interpolate( currentFoodOrb.m_position, currentFoodOrb.m_goalPos, fractionTowardsEnd );

				//----------------------------------------------------------------------------------------------------------------------
				// Move IK chains to mimic spider movement animation
				//----------------------------------------------------------------------------------------------------------------------
				IK_Chain3D* rightArm					= currentFoodOrb.m_quadSpider->m_rightArm;
				rightArm->m_position_WS					= currentFoodOrb.m_position;
				rightArm->m_target.m_currentPos			= currentFoodOrb.m_position + Vec3::Y_LEFT	* 10.0f;
				float time								= float( GetCurrentTimeSeconds() );
				float sineSpeedScalar					= 10.0f;

				float distPrevGoalPosToNewGoalPos		= GetDistance3D( currentFoodOrb.m_prevPos, currentFoodOrb.m_goalPos  );
				sineSpeedScalar							= RangeMapClamped( distCurPosToGoal, 
																		   10.0f,
																		   distPrevGoalPosToNewGoalPos, 
																		   325.0f, 335.0f );
				float sine								= SinDegrees( time * sineSpeedScalar );
				sine									= fabsf( sine * 10.0f );
				rightArm->m_target.m_currentPos.z	   += sine;
				rightArm->m_firstJoint->m_poleVector	= rightArm->m_position_WS + ( Vec3::Z_UP * 10.0f );
				currentFoodOrb.m_quadSpider->Update();
// 				fix rangemap clamped?
// 				need to fine tune "walk anims"
// 				fix pole vectors
// 				set root fwd dir based on disp to new goal
				DebuggerPrintf( "\nsineSpeedScalar: %0.2f, sine: %0.2f\n", sineSpeedScalar, sine );
				DebuggerPrintf( "distPrevGoalPosToNewGoalPos: %0.2f, distCurPosToGoal: %0.2f\n\n", distPrevGoalPosToNewGoalPos, distCurPosToGoal );
			}
		}
	}
}