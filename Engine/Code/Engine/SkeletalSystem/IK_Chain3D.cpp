#include "Engine/SkeletalSystem/IK_Chain3D.hpp"
#include "Engine/SkeletalSystem/CreatureBase.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Renderer/BitmapFont.hpp"


//----------------------------------------------------------------------------------------------------------------------
IK_Chain3D::IK_Chain3D( std::string name, Vec3 localOffsetToRoot, IK_Joint3D* ownerSkeletonFirstJoint, CreatureBase* const creatureOwner, bool shouldReachInsteadOfDrag )
{
	m_name							= name;
	m_ownerSkeletonFirstJoint		= ownerSkeletonFirstJoint;
	m_creatureOwner					= creatureOwner;
	m_position_WS					= localOffsetToRoot;
	if ( m_ownerSkeletonFirstJoint != nullptr )
	{
		m_localOffsetToOwner	= localOffsetToRoot - m_ownerSkeletonFirstJoint->m_jointPos_LS;
	}
	m_shouldReachInsteadOfDrag	= shouldReachInsteadOfDrag;
}


//----------------------------------------------------------------------------------------------------------------------
IK_Chain3D::~IK_Chain3D()
{
}

//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::Startup()
{
}

//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::Shutdown()
{
}

//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::Update()
{
	if ( m_shouldReachInsteadOfDrag )
	{
//		ReachTargetPos_FABRIK( m_currentTargetPos );		// Uncomment this to get creature working again		// Refactor these functions 
		if ( m_solverType == CHAIN_SOLVER_FABRIK )
		{
			Solve_FABRIK( m_target );
		}
		else if ( m_solverType == CHAIN_SOLVER_CCD )
		{
			Solve_CCD( m_target );
		}
	}
	else
	{
		if ( m_solverType == CHAIN_SOLVER_FABRIK )
		{
			FABRIK_Forward( m_target );
		}
	}

	for ( int i = 0; i < m_jointList.size(); i++ )
	{
		m_jointList[i]->Update();
	}
	if ( m_solverType == CHAIN_SOLVER_FABRIK )
	{
		if ( m_ownerSkeletonFirstJoint != nullptr )
		{
			m_position_WS = m_ownerSkeletonFirstJoint->m_jointPos_LS + 
						( m_ownerSkeletonFirstJoint->m_fwdDir  * m_localOffsetToOwner.x ) + 
						( m_ownerSkeletonFirstJoint->m_leftDir * m_localOffsetToOwner.y ) + 
						( m_ownerSkeletonFirstJoint->m_upDir   * m_localOffsetToOwner.z );
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
// 'WS' means "world space"
// 'LS' means "local space"
//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::Render( std::vector<Vertex_PCU>& verts, Rgba8 const& limbColor, Rgba8 const& jointColor, bool renderDebugJointBasis, bool renderDebugCurrentPos_EE ) const
{
	if ( m_solverType == CHAIN_SOLVER_FABRIK )
	{
		//----------------------------------------------------------------------------------------------------------------------
		// Render limbs
		//----------------------------------------------------------------------------------------------------------------------
		for ( int limbIndex = 0; limbIndex < m_jointList.size(); limbIndex++ )
		{
			IK_Joint3D* currentLimb	= m_jointList[limbIndex];
			Vec3 limbStart				= currentLimb->m_jointPos_LS;
			Vec3 limbEnd				= currentLimb->m_endPos;
			AddVertsForCylinder3D( verts, limbStart, limbEnd, 0.1f, limbColor );
		}

		//----------------------------------------------------------------------------------------------------------------------
		// Render Joints
		//----------------------------------------------------------------------------------------------------------------------
		for ( int limbIndex = 0; limbIndex < m_jointList.size(); limbIndex++ )
		{
			IK_Joint3D* currentLimb	= m_jointList[limbIndex];
			Vec3 limbEnd				= currentLimb->m_endPos;
			AddVertsForSphere3D( verts, limbEnd, 1.0f, 16.0f, 16.0f, jointColor );
		}

		//----------------------------------------------------------------------------------------------------------------------
		// Render Fwd, Left, Up basis
		//----------------------------------------------------------------------------------------------------------------------
		if ( renderDebugJointBasis )
		{
			DebugDrawJoints_IJK( verts );
		}

		//----------------------------------------------------------------------------------------------------------------------
		// End Effectors
		//----------------------------------------------------------------------------------------------------------------------
		if ( renderDebugCurrentPos_EE )
		{
			RenderTarget_EE( verts );
	//		RenderTarget_EE_IJK_Vectors( verts, endPosLength )
		}
	}
	else if ( m_solverType == CHAIN_SOLVER_CCD )
	{
		//----------------------------------------------------------------------------------------------------------------------
		// Render limbs
		//----------------------------------------------------------------------------------------------------------------------
		Vec3 parentJointPos_WS = Vec3::ZERO;
		for ( int limbIndex = 0; limbIndex < m_jointList.size(); limbIndex++ )
		{
			IK_Joint3D* currentLimb		= m_jointList[limbIndex];
			Mat44 modelToWorldMatrix	= currentLimb->GetMatrix_ModelToWorld();
			Vec3  curJointPos_WS		= modelToWorldMatrix.GetTranslation3D();
			if ( limbIndex > 0 )
			{
				// Only render segments if currentJoint has a parent 
				AddVertsForCylinder3D( verts, parentJointPos_WS, curJointPos_WS, 0.1f, limbColor );
			}
			// Render joints
			AddVertsForSphere3D( verts, curJointPos_WS, 0.1f, 8.0f, 8.0f, jointColor );
			// Update variable for "next" joint to render as a cylinder
			parentJointPos_WS = curJointPos_WS;
		}
	}
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::CreateNewJoint( Vec3 const& position_localSpace, EulerAngles orientation_localSpace, FloatRange yawConstraints, 
																									  FloatRange pitchConstraints, 
																									  FloatRange rollConstraints )
{
	int	limbIndex  = int( m_jointList.size() );

	IK_Joint3D* newJoint = new IK_Joint3D(  
											limbIndex, 
										    position_localSpace, 
										    0,	
										    this, 
										    JOINT_CONSTRAINT_TYPE_EULER, 
										    orientation_localSpace, 
											yawConstraints, 
											pitchConstraints, 
											rollConstraints 
										  );

	// Update this IK_Chain's pointers for "firstJoint" and/or "finalJoint"
	if ( m_finalJoint != nullptr )
	{
		// Set newJoint's parent pointer to finalJoint
		// Note: At this point, "m_finalJoint" is still technically the final joint in 
		//		 the chain, but the "newJoint" is about get added into the "m_jointList"
		//		 and become the new "m_finalJoint"
		m_finalJoint->m_child	= newJoint;
		newJoint->m_parent		= m_finalJoint;									

		// Update "m_distToChild" for "finalJoint"
		m_finalJoint->m_distToChild = newJoint->m_jointPos_LS.GetLength();
	}
	else
	{
		// Logic for "firstJoint" 
		// If this is the "firstJoint", there is NO "finalJoint"
		m_firstJoint = newJoint;
	}

	// Update finalJoint to the "newJoint" just created
	m_jointList.push_back( newJoint );
	m_finalJoint = newJoint;
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::CreateNewLimb( int limbIndex, float length, Vec3 const& fwdDir, JointConstraintType jointConstraintType, IK_Chain3D* IK_Chain,  
																														  FloatRange  yawConstraints, 
																														  FloatRange  pitchConstraints, 
																														  FloatRange  rollConstraints )
{
	if ( IK_Chain == nullptr )
	{
		IK_Chain = this;
	}
	IK_Joint3D* newLimb = new IK_Joint3D( limbIndex, Vec3::ZERO, length, IK_Chain, jointConstraintType, EulerAngles(), 
																											yawConstraints, 
																											pitchConstraints, 
																											rollConstraints );
	if ( m_finalJoint != nullptr )
	{
		// If a "final limb" exists
		newLimb->m_fwdDir				= fwdDir;
		newLimb->m_jointPos_LS		= m_finalJoint->GetLimbEnd();		// Set newLimb's starting position at the end of finalLimb
		newLimb->m_endPos				= newLimb->m_jointPos_LS + ( newLimb->m_fwdDir * newLimb->m_distToChild );
		newLimb->m_parent				= m_finalJoint;									// Set newLimb's parent pointer to finalLimb
		newLimb->m_parent->m_child		= newLimb;	
	}
	else
	{
		// If this is the "First limb", there is NO "final limb"
		newLimb->m_jointPos_LS = m_position_WS;
		newLimb->m_fwdDir			= fwdDir;
		newLimb->m_endPos			= newLimb->m_jointPos_LS + ( newLimb->m_fwdDir * newLimb->m_distToChild );
		m_firstJoint				= newLimb;
	}

	// Update finalLimb to the "newLimb" just created
	m_jointList.push_back( newLimb );
	m_finalJoint = newLimb;
}



//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::CreateNewLimbs( float limbLength, float numLimbs, Vec3 const& fwdDir, bool const& isFinalLimbSubBase, JointConstraintType const& jointConstraintType, FloatRange const& yawConstraints, FloatRange const& pitchConstraints, FloatRange const& rollConstraints )
{
	int limbListSize = int( m_jointList.size() );
	for ( int i = 0; i < numLimbs; i++ )
	{
		CreateNewLimb( ( limbListSize + i ), limbLength, fwdDir, jointConstraintType, nullptr, yawConstraints, pitchConstraints, rollConstraints );
	}
	if ( isFinalLimbSubBase )
	{
		m_finalJoint->m_isSubBase = true;
	}
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::RenderTarget_IJK( std::vector<Vertex_PCU>& verts, float endPosLength ) const
{
	// Render IJK basis for each joint
	// This version uses the m_Left and m_Up vectors
	Vec3 fwdPos					= m_target.m_currentPos + ( m_target.m_fwdDir  * endPosLength			);
	Vec3 leftPos				= m_target.m_currentPos + ( m_target.m_leftDir * endPosLength			);
	Vec3 upPos					= m_target.m_currentPos + ( m_target.m_upDir   * endPosLength			);
//	Vec3 axisPos				= m_target.m_currentPos + ( m_axisOfRotation		 * endPosLength * 0.5f  );
	AddVertsForArrow3D( verts, m_target.m_currentPos,  fwdPos, 0.1f, Rgba8::RED	 );		// Fwd
	AddVertsForArrow3D( verts, m_target.m_currentPos, leftPos, 0.1f, Rgba8::GREEN	 );		// Left
	AddVertsForArrow3D( verts, m_target.m_currentPos,	 upPos, 0.1f, Rgba8::BLUE	 );		// Up
//	AddVertsForArrow3D( verts, m_target.m_currentPos, axisPos, 0.1f, Rgba8::MAGENTA );		// Up
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::RenderTarget_EE( std::vector<Vertex_PCU>& verts, float radius /*= 2.0f*/) const
{
	AddVertsForSphere3D( verts, m_target.m_currentPos, radius, 8.0f, 8.0f, m_target.m_color );
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::DebugDrawJoints_IJK( std::vector<Vertex_PCU>& verts, float arrowThickness /*= 0.1f*/, float endPosLength /*= 3.0f*/ ) const
{
	if ( !m_shouldRender )
	{
		return;
	}

	// Render IJK basis for each joint
	for ( int i = 0; i < m_jointList.size(); i++ )
	{
		IK_Joint3D* currentJoint	= m_jointList[i];
		if ( m_solverType == CHAIN_SOLVER_FABRIK )
		{
			if ( currentJoint->m_parent != nullptr )
			{
				// This version uses the m_Left and m_Up vectors
				Vec3 fwdPos					= currentJoint->m_jointPos_LS + ( currentJoint->m_fwdDir			* endPosLength		  );
				Vec3 leftPos				= currentJoint->m_jointPos_LS + ( currentJoint->m_leftDir			* endPosLength		  );
				Vec3 upPos					= currentJoint->m_jointPos_LS + ( currentJoint->m_upDir			* endPosLength		  );
				Vec3 axisPos				= currentJoint->m_jointPos_LS + ( currentJoint->m_axisOfRotation	* endPosLength * 0.5f );
				AddVertsForArrow3D( verts, currentJoint->m_jointPos_LS, fwdPos,  0.5f, Rgba8::RED	   );		// Fwd
				AddVertsForArrow3D( verts, currentJoint->m_jointPos_LS, leftPos, 0.5f, Rgba8::GREEN   );		// Left
				AddVertsForArrow3D( verts, currentJoint->m_jointPos_LS, upPos,   0.5f, Rgba8::BLUE	   );		// Up
		//		AddVertsForArrow3D( verts, currentLimb->m_startPos,	axisPos, 0.6f, Rgba8::MAGENTA  );				// Axis of Rotation
			}
		}
		else if ( m_solverType == CHAIN_SOLVER_CCD )
		{
			if ( currentJoint->m_parent != nullptr )
			{
				// Get jointPos_LS and transform to WS
				Mat44 modelToWorldMatrix	= currentJoint->GetMatrix_ModelToWorld();
				Vec3  jointPos_WS			= modelToWorldMatrix.GetTranslation3D();

				// This version uses the m_Left and m_Up vectors
				Vec3 fwdPos					= jointPos_WS + ( modelToWorldMatrix.GetIBasis3D()  * endPosLength		  );
				Vec3 leftPos				= jointPos_WS + ( modelToWorldMatrix.GetJBasis3D()	* endPosLength		  );
				Vec3 upPos					= jointPos_WS + ( modelToWorldMatrix.GetKBasis3D()	* endPosLength		  );
				AddVertsForArrow3D( verts, jointPos_WS, fwdPos,  arrowThickness, Rgba8::RED		);		// Fwd
				AddVertsForArrow3D( verts, jointPos_WS, leftPos, arrowThickness, Rgba8::GREEN   );		// Left
				AddVertsForArrow3D( verts, jointPos_WS, upPos,   arrowThickness, Rgba8::BLUE	);		// Up
//				Vec3 axisPos				= jointPos_WS + ( currentJoint->m_axisOfRotation	* endPosLength * 0.5f );
//				AddVertsForArrow3D( verts, currentLimb->m_startPos,	axisPos, 0.6f, Rgba8::MAGENTA  );	// Axis of Rotation
			}
			else
			{
				// Get jointPos_LS and transform to WS
				Mat44 modelToWorldMatrix	= currentJoint->GetMatrix_ModelToWorld();
				Vec3  jointPos_WS			= modelToWorldMatrix.GetTranslation3D();

				Vec3 fwdPos					= jointPos_WS + ( modelToWorldMatrix.GetIBasis3D()	* endPosLength		  );
				Vec3 leftPos				= jointPos_WS + ( modelToWorldMatrix.GetJBasis3D()	* endPosLength		  );
				Vec3 upPos					= jointPos_WS + ( modelToWorldMatrix.GetKBasis3D()	* endPosLength		  );
				AddVertsForArrow3D( verts, jointPos_WS, fwdPos,  arrowThickness, Rgba8::RED		);		// Fwd
				AddVertsForArrow3D( verts, jointPos_WS, leftPos, arrowThickness, Rgba8::GREEN   );		// Left
				AddVertsForArrow3D( verts, jointPos_WS, upPos,   arrowThickness, Rgba8::BLUE	);		// Up
//				Vec3 axisPos				= jointPos_WS + ( currentJoint->m_axisOfRotation	* endPosLength * 0.5f );
//				AddVertsForArrow3D( verts, currentLimb->m_startPos,	axisPos, 0.6f, Rgba8::MAGENTA  );				// Axis of Rotation
			}
		}
	}
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::DebugDrawConstraints_YPR( std::vector<Vertex_PCU>& verts, float length ) const
{
	for ( int i = 0; i < m_jointList.size(); i++ )
	{
		IK_Joint3D* currentJoint = m_jointList[i];
		if ( currentJoint->m_parent != nullptr )
		{
			// Logic for children
			Mat44 fwdMatrix;
			Mat44 leftMatrix;
			Mat44 upMatrix;

			if ( m_solverType == CHAIN_SOLVER_FABRIK )
			{
				fwdMatrix.SetIJKT3D	( currentJoint->m_parent->m_fwdDir,   currentJoint->m_parent->m_leftDir,  currentJoint->m_parent->m_upDir,   currentJoint->m_parent->m_endPos );
				leftMatrix.SetIJKT3D( currentJoint->m_parent->m_fwdDir,  -currentJoint->m_parent->m_upDir,    currentJoint->m_parent->m_leftDir, currentJoint->m_parent->m_endPos );
				upMatrix.SetIJKT3D	( currentJoint->m_parent->m_leftDir, -currentJoint->m_parent->m_upDir,   -currentJoint->m_parent->m_fwdDir,  currentJoint->m_parent->m_endPos );
			}
			else if ( m_solverType == CHAIN_SOLVER_CCD )
			{
				Mat44 modelToWorldMatrix_curJoint		= currentJoint->GetMatrix_ModelToWorld();
				Vec3  curJointPos_WS					= modelToWorldMatrix_curJoint.GetTranslation3D();
				Mat44 modelToWorldMatrix_parentJoint	= currentJoint->m_parent->GetMatrix_ModelToWorld();
				Vec3  parentFwd							= modelToWorldMatrix_parentJoint.GetIBasis3D();
				Vec3  parenLeft							= modelToWorldMatrix_parentJoint.GetJBasis3D();
				Vec3  parentUp							= modelToWorldMatrix_parentJoint.GetKBasis3D();
				fwdMatrix.SetIJKT3D	( parentFwd,  parenLeft, parentUp,  curJointPos_WS );
				leftMatrix.SetIJKT3D( parentFwd, -parentUp,  parenLeft, curJointPos_WS );
				upMatrix.SetIJKT3D	( parenLeft, -parentUp, -parentFwd, curJointPos_WS );
			}

			AddVertsForArc3D_Yaw  ( verts, fwdMatrix,  FloatRange( currentJoint->m_yawConstraints_LS.m_min,   currentJoint->m_yawConstraints_LS.m_max   ), length, Rgba8( 100,   0,   0 ) );
			AddVertsForArc3D_Pitch( verts, leftMatrix, FloatRange( currentJoint->m_pitchConstraints_LS.m_min, currentJoint->m_pitchConstraints_LS.m_max ), length, Rgba8(   0, 100,   0 ) );
			AddVertsForArc3D_Roll ( verts, upMatrix,   FloatRange( currentJoint->m_rollConstraints_LS.m_min,  currentJoint->m_rollConstraints_LS.m_max  ), length, Rgba8(   0,   0, 100 ) );
		}
		else // Logic for Parent
		{
			Mat44 fwdMatrix;
			Mat44 leftMatrix;
			Mat44 upMatrix;
			if ( m_solverType == CHAIN_SOLVER_FABRIK )
			{
				fwdMatrix.SetIJKT3D	( m_creatureOwner->m_root->m_fwdDir,   m_creatureOwner->m_root->m_leftDir,  m_creatureOwner->m_root->m_upDir,	m_firstJoint->m_jointPos_LS );
				leftMatrix.SetIJKT3D( m_creatureOwner->m_root->m_fwdDir,  -m_creatureOwner->m_root->m_upDir,	m_creatureOwner->m_root->m_leftDir, m_firstJoint->m_jointPos_LS );
				upMatrix.SetIJKT3D	( m_creatureOwner->m_root->m_leftDir, -m_creatureOwner->m_root->m_upDir,   -m_creatureOwner->m_root->m_fwdDir,	m_firstJoint->m_jointPos_LS );
			}
			else if ( m_solverType == CHAIN_SOLVER_CCD )
			{
				Vec3 fwd, left, up; 
				m_eulerAngles_WS.GetAsVectors_XFwd_YLeft_ZUp( fwd, left, up );
				Vec3 rootPos_WS = currentJoint->GetMatrix_ModelToWorld().GetTranslation3D();
				fwdMatrix.SetIJKT3D	( fwd,   left,    up, rootPos_WS );
				leftMatrix.SetIJKT3D( fwd,    -up,	left, rootPos_WS );
				upMatrix.SetIJKT3D	( left,   -up,  -fwd, rootPos_WS );
			}
			AddVertsForArc3D_Yaw  ( verts, fwdMatrix,  FloatRange( currentJoint->m_yawConstraints_LS.m_min,   currentJoint->m_yawConstraints_LS.m_max   ), length, Rgba8( 100,   0,   0 ) );
			AddVertsForArc3D_Pitch( verts, leftMatrix, FloatRange( currentJoint->m_pitchConstraints_LS.m_min, currentJoint->m_pitchConstraints_LS.m_max ), length, Rgba8(   0, 100,   0 ) );
			AddVertsForArc3D_Roll ( verts, upMatrix,   FloatRange( currentJoint->m_rollConstraints_LS.m_min,  currentJoint->m_rollConstraints_LS.m_max  ), length, Rgba8(   0,   0, 100 ) );
		}
	}
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::DebugDrawParentBasis( std::vector<Vertex_PCU>& verts, float length /*= 4.0f */) const
{
	for ( int i = 0; i < m_jointList.size(); i++ )
	{
		IK_Joint3D* currentJoint = m_jointList[i];
		if ( m_solverType == CHAIN_SOLVER_FABRIK )
		{
			if ( currentJoint->m_parent != nullptr )
			{
				// Logic for children
				Vec3 startPos	= currentJoint->m_parent->m_endPos;
				Vec3 fwdEnd		= startPos + ( currentJoint->m_parent->m_fwdDir  * 4.0f );
				Vec3 leftEnd	= startPos + ( currentJoint->m_parent->m_leftDir * 4.0f );
				Vec3 upEnd		= startPos + ( currentJoint->m_parent->m_upDir	* 4.0f );
				AddVertsForArrow3D( verts, startPos, fwdEnd,  0.2f, Rgba8::MAGENTA   );
				AddVertsForArrow3D( verts, startPos, leftEnd, 0.2f, Rgba8::CYAN		 );
				AddVertsForArrow3D( verts, startPos, upEnd,   0.2f, Rgba8::LIGHTBLUE );
			}
			else // Logic for Parent
			{
				Vec3 startPos	= m_position_WS;
				Vec3 fwdEnd		= startPos + ( m_creatureOwner->m_root->m_fwdDir  * length );
				Vec3 leftEnd	= startPos + ( m_creatureOwner->m_root->m_leftDir * length );
				Vec3 upEnd		= startPos + ( m_creatureOwner->m_root->m_upDir	  * length );
				AddVertsForArrow3D( verts, startPos, fwdEnd,  0.2f, Rgba8::MAGENTA   );
				AddVertsForArrow3D( verts, startPos, leftEnd, 0.2f, Rgba8::CYAN		 );
				AddVertsForArrow3D( verts, startPos, upEnd,   0.2f, Rgba8::LIGHTBLUE );
			}
		}
		else if ( m_solverType == CHAIN_SOLVER_CCD )
		{
			if ( currentJoint->m_parent != nullptr )
			{
				// Logic for children

				// Transform jointPos_LS to WS
				Mat44 localToModelMatrix = currentJoint->GetMatrix_LocalToModel();
				Vec3 startPos			 = localToModelMatrix.GetTranslation3D();
				Vec3 fwdEnd				 = startPos + ( currentJoint->m_parent->m_fwdDir  * 4.0f );
				Vec3 leftEnd			 = startPos + ( currentJoint->m_parent->m_leftDir * 4.0f );
				Vec3 upEnd				 = startPos + ( currentJoint->m_parent->m_upDir	 * 4.0f );
				AddVertsForArrow3D( verts, startPos, fwdEnd,  0.2f, Rgba8::MAGENTA   );
				AddVertsForArrow3D( verts, startPos, leftEnd, 0.2f, Rgba8::CYAN		 );
				AddVertsForArrow3D( verts, startPos, upEnd,   0.2f, Rgba8::LIGHTBLUE );
			}
			else // Logic for Parent
			{
				Vec3 startPos	= m_position_WS;
				Vec3 fwdEnd		= startPos + ( m_creatureOwner->m_root->m_fwdDir  * length );
				Vec3 leftEnd	= startPos + ( m_creatureOwner->m_root->m_leftDir * length );
				Vec3 upEnd		= startPos + ( m_creatureOwner->m_root->m_upDir	  * length );
				AddVertsForArrow3D( verts, startPos, fwdEnd,  0.2f, Rgba8::MAGENTA   );
				AddVertsForArrow3D( verts, startPos, leftEnd, 0.2f, Rgba8::CYAN		 );
				AddVertsForArrow3D( verts, startPos, upEnd,   0.2f, Rgba8::LIGHTBLUE );
			}
		}
	}
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::DebugDrawTarget( std::vector<Vertex_PCU>& verts, float radius, Rgba8 const& color /*= Rgba8::WHITE*/, bool renderBasis /*= false */, float basisLength /*= 1.0f*/ ) const
{
	AddVertsForSphere3D( verts, m_target.m_currentPos, radius, 8.0f, 8.0f, color );
	if ( renderBasis )
	{
		RenderTarget_IJK( verts, basisLength );	 
	}
}

//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::DebugTextJointPos_WorldSpace( std::vector<Vertex_PCU>& verts, float textHeight, Vec3 const& camera_jBasis, Vec3 const& camera_kBasis, BitmapFont* textFont ) const
{
	Vec3  fwd			= -camera_jBasis;
	Vec3  left			=  camera_kBasis;
	float heightOffset	= 2.0f;
	for ( int i = 0; i < m_jointList.size(); i++ )
	{
		IK_Joint3D* currentLimb = m_jointList[i];

		if ( m_solverType == CHAIN_SOLVER_FABRIK )
		{
			textFont->AddVertsForText3D( verts, Vec3( currentLimb->m_jointPos_LS.x, currentLimb->m_jointPos_LS.y, currentLimb->m_jointPos_LS.z + heightOffset ), 
				fwd, left, textHeight, Stringf( "X: %0.1f, Y: %0.1f, Z: %0.1f", currentLimb->m_jointPos_LS.x, currentLimb->m_jointPos_LS.y, currentLimb->m_jointPos_LS.z ).c_str() );
		}
		else if ( m_solverType == CHAIN_SOLVER_CCD )
		{
			Mat44 localToModelMatrix	= currentLimb->GetMatrix_LocalToModel();
			Vec3 startPos				= localToModelMatrix.GetTranslation3D();
			textFont->AddVertsForText3D( verts, Vec3( startPos.x, startPos.y, startPos.z + heightOffset ),
				fwd, left, textHeight, Stringf( "X: %0.1f, Y: %0.1f, Z: %0.1f", startPos.x, startPos.y, startPos.z ).c_str() );
		}

	}
	textFont->AddVertsForText3D( verts, Vec3( m_target.m_currentPos.x, m_target.m_currentPos.y, m_target.m_currentPos.z + heightOffset ), fwd, left, textHeight, 
		Stringf( "X: %0.1f, Y: %0.1f, Z: %0.1f", m_target.m_currentPos.x, m_target.m_currentPos.y, m_target.m_currentPos.z ).c_str() );
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::DebugTextJointPos_LocalSpace( std::vector<Vertex_PCU>& verts, float textHeight, Vec3 const& camera_jBasis, Vec3 const& camera_kBasis, BitmapFont* textFont ) const
{
	Vec3  fwd			= -camera_jBasis;
	Vec3  left			=  camera_kBasis;
	float heightOffset	= 2.0f;
	for ( int i = 0; i < m_jointList.size(); i++ )
	{
		IK_Joint3D* currentLimb = m_jointList[i];
		if ( m_solverType == CHAIN_SOLVER_CCD )
		{
			Mat44 modelToWorldMatrix	= currentLimb->GetMatrix_ModelToWorld();
			Vec3 jointPos_WS			= modelToWorldMatrix.GetTranslation3D();
			textFont->AddVertsForText3D( verts, Vec3( jointPos_WS.x, jointPos_WS.y, jointPos_WS.z + heightOffset ),
				fwd, left, textHeight, Stringf( "%0.1f, %0.1f, %0.1f",	currentLimb->m_jointPos_LS.x,
																		currentLimb->m_jointPos_LS.y, 
																		currentLimb->m_jointPos_LS.z ).c_str() );
		}
	}
	textFont->AddVertsForText3D( verts, Vec3( m_target.m_currentPos.x, m_target.m_currentPos.y, m_target.m_currentPos.z + heightOffset ), fwd, left, textHeight, 
		Stringf( "%0.1f, %0.1f, %0.1f", m_target.m_currentPos.x, m_target.m_currentPos.y, m_target.m_currentPos.z ).c_str() );
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::DebugTextConstraints_YPR( std::vector<Vertex_PCU>& verts, float textHeight, float lengthToTextPosFromStart, float heightOffset, Vec3 const& jBasis, Vec3 const& kBasis, BitmapFont* textFont ) const
{
	Vec3			fwd			= -jBasis;
	Vec3			left		=  kBasis;
	unsigned char	colorTint	= 255;
	for ( int i = 0; i < m_jointList.size(); i++ )
	{
		IK_Joint3D* currentLimb = m_jointList[i];
		// Logic for children
		if ( currentLimb->m_parent != nullptr )
		{
			//----------------------------------------------------------------------------------------------------------------------
			// 1. Compute end points for YAW min and max
			//----------------------------------------------------------------------------------------------------------------------
			Vec3 start			= currentLimb->m_parent->m_endPos;
			Vec3 end			= start + ( currentLimb->m_parent->m_fwdDir * lengthToTextPosFromStart );
			Vec3 vecToRotate	= end - start;
			Vec3 minTextPos		= RotateVectorAboutArbitraryAxis( vecToRotate, currentLimb->m_parent->m_upDir, currentLimb->m_yawConstraints_LS.m_min );
			Vec3 maxTextPos		= RotateVectorAboutArbitraryAxis( vecToRotate, currentLimb->m_parent->m_upDir, currentLimb->m_yawConstraints_LS.m_max );
			// Add height offsets
			minTextPos			= start + minTextPos + ( currentLimb->m_parent->m_upDir * heightOffset );
			maxTextPos			= start + maxTextPos + ( currentLimb->m_parent->m_upDir * heightOffset );
			// Yaw Min
			textFont->AddVertsForText3D( verts, minTextPos, fwd, left, textHeight, 
										Stringf( "Min%0.1d: %0.1f", i, currentLimb->m_yawConstraints_LS.m_min ).c_str(), Rgba8( colorTint, 0, 0 ) );
			// Yaw Max
			textFont->AddVertsForText3D( verts, maxTextPos, fwd, left, textHeight,
										 Stringf( "Max%0.1d: %0.1f", i, currentLimb->m_yawConstraints_LS.m_max ).c_str(), Rgba8( colorTint, 0, 0 ) );	
			//----------------------------------------------------------------------------------------------------------------------
			// 2. Compute end points for PITCH min and max
			//----------------------------------------------------------------------------------------------------------------------
			minTextPos			= RotateVectorAboutArbitraryAxis( vecToRotate, currentLimb->m_parent->m_leftDir, currentLimb->m_pitchConstraints_LS.m_min );
			maxTextPos			= RotateVectorAboutArbitraryAxis( vecToRotate, currentLimb->m_parent->m_leftDir, currentLimb->m_pitchConstraints_LS.m_max );
			minTextPos			= start + minTextPos + ( currentLimb->m_parent->m_upDir * heightOffset );
			maxTextPos			= start + maxTextPos + ( currentLimb->m_parent->m_upDir * -heightOffset );
			// Pitch Min
			textFont->AddVertsForText3D( verts, minTextPos, fwd, left, textHeight,
				Stringf( "Min%0.1d: %0.1f", i, currentLimb->m_pitchConstraints_LS.m_min ).c_str(), Rgba8( 0, colorTint, 0 ) );
			// Pitch Max
			textFont->AddVertsForText3D( verts, maxTextPos, fwd, left, textHeight,
				Stringf( "Max%0.1d: %0.1f", i, currentLimb->m_pitchConstraints_LS.m_max ).c_str(), Rgba8( 0, colorTint, 0 ) );
			//----------------------------------------------------------------------------------------------------------------------
			// 3. Compute end points for ROLL min and max
			//----------------------------------------------------------------------------------------------------------------------
			end					= start + ( currentLimb->m_parent->m_leftDir * lengthToTextPosFromStart );
			vecToRotate			= end - start;
			minTextPos			= RotateVectorAboutArbitraryAxis( vecToRotate, currentLimb->m_parent->m_fwdDir, currentLimb->m_rollConstraints_LS.m_min );
			maxTextPos			= RotateVectorAboutArbitraryAxis( vecToRotate, currentLimb->m_parent->m_fwdDir, currentLimb->m_rollConstraints_LS.m_max );
			minTextPos			= start + minTextPos + ( currentLimb->m_parent->m_upDir * -heightOffset );
			maxTextPos			= start + maxTextPos + ( currentLimb->m_parent->m_upDir * heightOffset );
			// Roll Min
			textFont->AddVertsForText3D( verts, minTextPos, fwd, left, textHeight,
										Stringf( "Min%0.1d: %0.1f", i, currentLimb->m_rollConstraints_LS.m_min ).c_str(), Rgba8( 0, 0, colorTint ) );
			// Roll Max
			textFont->AddVertsForText3D( verts, maxTextPos, fwd, left, textHeight,
										Stringf( "Max%0.1d: %0.1f", i, currentLimb->m_rollConstraints_LS.m_max ).c_str(), Rgba8( 0, 0, colorTint ) );
		}
		else // Logic for Parent
		{
			//----------------------------------------------------------------------------------------------------------------------
			// 1. Compute end points for YAW min and max
			//----------------------------------------------------------------------------------------------------------------------
			Vec3 start			= currentLimb->m_jointPos_LS;
			Vec3 end			= start + ( m_creatureOwner->m_root->m_fwdDir * lengthToTextPosFromStart );
			Vec3 vecToRotate	= end - start;
			Vec3 minTextPos		= RotateVectorAboutArbitraryAxis( vecToRotate, m_creatureOwner->m_root->m_upDir, currentLimb->m_yawConstraints_LS.m_min );
			Vec3 maxTextPos		= RotateVectorAboutArbitraryAxis( vecToRotate, m_creatureOwner->m_root->m_upDir, currentLimb->m_yawConstraints_LS.m_max );
			// Yaw Min
			textFont->AddVertsForText3D( verts, start + Vec3( minTextPos.x, minTextPos.y, minTextPos.z + heightOffset ), fwd, left, textHeight, 
											Stringf( "Min%0.1d: %0.1f", i, currentLimb->m_yawConstraints_LS.m_min ).c_str(), Rgba8( colorTint, 0, 0 ) );
			// Yaw Max
			textFont->AddVertsForText3D( verts, start + Vec3( maxTextPos.x, maxTextPos.y, maxTextPos.z + heightOffset ), fwd, left, textHeight,
											Stringf( "Max%0.1d: %0.1f", i, currentLimb->m_yawConstraints_LS.m_max ).c_str(), Rgba8( colorTint, 0, 0 ) );
			//----------------------------------------------------------------------------------------------------------------------
			// 2. Compute end points for PITCH min and max
			//----------------------------------------------------------------------------------------------------------------------
			minTextPos			= RotateVectorAboutArbitraryAxis( vecToRotate, m_creatureOwner->m_root->m_leftDir, currentLimb->m_pitchConstraints_LS.m_min );
			maxTextPos			= RotateVectorAboutArbitraryAxis( vecToRotate, m_creatureOwner->m_root->m_leftDir, currentLimb->m_pitchConstraints_LS.m_max );
			minTextPos			= start + minTextPos + ( m_creatureOwner->m_root->m_upDir *  heightOffset );
			maxTextPos			= start + maxTextPos + ( m_creatureOwner->m_root->m_upDir * -heightOffset );
			// Pitch Min
			textFont->AddVertsForText3D( verts, minTextPos, fwd, left, textHeight,
											Stringf( "Min%0.1d: %0.1f", i, currentLimb->m_pitchConstraints_LS.m_min ).c_str(), Rgba8( 0, colorTint, 0 ) );
			// Pitch Max
			textFont->AddVertsForText3D( verts, maxTextPos, fwd, left, textHeight,
											Stringf( "Max%0.1d: %0.1f", i, currentLimb->m_pitchConstraints_LS.m_max ).c_str(), Rgba8( 0, colorTint, 0 ) );
			//----------------------------------------------------------------------------------------------------------------------
			// 3. Compute end points for ROLL min and max
			//----------------------------------------------------------------------------------------------------------------------
			end					= start + ( m_creatureOwner->m_root->m_leftDir * lengthToTextPosFromStart );
			vecToRotate			= end - start; 
			minTextPos			= RotateVectorAboutArbitraryAxis( vecToRotate, m_creatureOwner->m_root->m_fwdDir, currentLimb->m_rollConstraints_LS.m_min );
			maxTextPos			= RotateVectorAboutArbitraryAxis( vecToRotate, m_creatureOwner->m_root->m_fwdDir, currentLimb->m_rollConstraints_LS.m_max );
			minTextPos			= start + minTextPos + ( m_creatureOwner->m_root->m_upDir * -heightOffset );
			maxTextPos			= start + maxTextPos + ( m_creatureOwner->m_root->m_upDir *  heightOffset );
			// Roll Min
			textFont->AddVertsForText3D( verts, minTextPos, fwd, left, textHeight,
											Stringf( "Min%0.1d: %0.1f", i, currentLimb->m_rollConstraints_LS.m_min ).c_str(), Rgba8( 0, 0, colorTint ) );
			// Roll Max
			textFont->AddVertsForText3D( verts, maxTextPos, fwd, left, textHeight,
											Stringf( "Max%0.1d: %0.1f", i, currentLimb->m_rollConstraints_LS.m_max ).c_str(), Rgba8( 0, 0, colorTint ) );
		}
	}
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::Solve_CCD( Target target )
{
	bool  wereChainsReset		= false;
	float tolerance				= 0.01f;
	int	  numIterations			= 5;
	UpdateDistEeToTarget_ALSO_CHECK_IfDistChangedSinceLastFrame( target );
	m_bestDistSolvedThisFrame	= m_distEeToTarget;
	for ( int i = 0; i < numIterations; i++ )
	{
// 		if ( m_distEeToTarget < tolerance )
// 		{
// 			// Stop solving if EE is close enough to target
// 			break;
// 		}
 		if ( m_targetPos_LastFrame == target.m_currentPos )
 		{
 			// Don't solve AT ALL, if target has not moved
// 			break;
 		}
		CCD_Forward( target );
		bool hasDistChanged = UpdateDistEeToTarget_ALSO_CHECK_IfDistChangedSinceLastFrame( target );
		if ( !hasDistChanged )
		{
			if ( m_distEeToTarget > tolerance )
			{
				if ( m_distEeToTarget > 5.0f )
				{
					if ( IsAnyJointBentToMaxConstraints() )
					{
						if ( i < (numIterations - 1))
						{
							if ( !wereChainsReset )
							{
								// 1. Only reset ANY joints' euler is fully constrained					AND
								// 2. distEeToTarget has NOT changed since the last iteration/frame		AND
								// 3. we are still too far away (than tolerance)						AND
								// 4. this is NOT the last iteration (avoid rendering a straight chain )
								ResetAllJointsEuler();
								wereChainsReset = true;
							}
						}
					}
				}
			}
		}
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Once CCD solve is done, check if new solution brought us closer to target
	// if distance is worse, then disregard new solution and keep the best solution found this frame
	//----------------------------------------------------------------------------------------------------------------------
	if ( m_bestDistSolvedThisFrame < m_distEeToTarget )
	{
		if ( wereChainsReset )
		{
			// Replace all joints' euler with previous solution
			for ( int i = 0; i < m_jointList.size(); i++ )
			{
				// Update both versions of euler to have the "best" solutions
				IK_Joint3D* currentJoint			= m_jointList[ i ];
				currentJoint->m_eulerAngles_LS		= currentJoint->m_eulerCloserToTarget;
			}
		}
	}
	else
	{
		// Update closer solutions
		// Replace all joints' euler with previous solution
		for ( int i = 0; i < m_jointList.size(); i++ )
		{
			// Update both versions of euler to have the "best" solutions
			IK_Joint3D* currentJoint			= m_jointList[ i ];
			currentJoint->m_eulerCloserToTarget = currentJoint->m_eulerAngles_LS;
		}
	}
	float distEndOfFrame = GetDistEeToTarget( target );

	// Update target position to keep data fresh
	m_targetPos_LastFrame = target.m_currentPos;

	m_bestDistSolvedThisFrame = 99999.9f;
}


//----------------------------------------------------------------------------------------------------------------------
// Child to root
//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::CCD_Forward( Target target )
{
	float distBeforeSolve_eeToTarget = GetDistEeToTarget( target );
	int	  numLimbs	= ( int( m_jointList.size() ) - 1 );
	for ( int i = numLimbs; i >= 0; i-- )
	{
		IK_Joint3D* currentJoint = m_jointList[ i ];
		if ( currentJoint == m_finalJoint )
		{
			// Skip the final joint (end effector)
			continue;
		}

		//----------------------------------------------------------------------------------------------------------------------
		// Main solver logic
		//----------------------------------------------------------------------------------------------------------------------
		if ( currentJoint->m_parent == nullptr )
		{
 			//----------------------------------------------------------------------------------------------------------------------
 			// Axis angle rotation approach
 			//----------------------------------------------------------------------------------------------------------------------
 			// 1. Transform target and EE pos to root space
			Mat44 modelToWorldMatrix		= m_eulerAngles_WS.GetAsMatrix_XFwd_YLeft_ZUp();
			modelToWorldMatrix.SetTranslation3D( m_position_WS );
			Mat44 worldToModelMatrix		= modelToWorldMatrix.GetOrthoNormalInverse();
			Vec3  target_MS					= worldToModelMatrix.TransformPosition3D( target.m_currentPos );
			// 1b. EE to model Space
 			Mat44 endEffector_LToM_Matrix	= m_finalJoint->GetMatrix_LocalToModel();
 			Vec3  endEffectorPos_MS			= endEffector_LToM_Matrix.GetTranslation3D();			// 'LToW_Matrix' means 'localToWorldMatrix'
 			// 2. Compute disps
 			Vec3  curJointToEE_MS			= endEffectorPos_MS   - currentJoint->m_jointPos_LS;
 			Vec3  curJointToTarget_MS		= target_MS - currentJoint->m_jointPos_LS;
 			// 3. Compute angle between disps
 			float angleToRotate				= GetAngleDegreesBetweenVectors3D( curJointToEE_MS, curJointToTarget_MS );
 			// 4. Compute rotation axis
 			Vec3 rotationAxis				= CrossProduct3D( curJointToEE_MS, curJointToTarget_MS );
 			rotationAxis.Normalize();
 			// 5. Rotate using Axis-angle 
 			currentJoint->m_fwdDir			= RotateVectorAboutArbitraryAxis( currentJoint->m_fwdDir, rotationAxis, angleToRotate );
 			currentJoint->m_fwdDir.Normalize();
			// 6. Update currentJoint eulerAngles
			currentJoint->m_eulerAngles_LS  = GetEulerFromFwdDir( currentJoint, currentJoint->m_fwdDir );
			// 7. Clamp eulerAngles if exceeding constrains
 			currentJoint->ClampYPR();
 			// 8. Re-update dir data again
 			currentJoint->m_eulerAngles_LS.GetAsVectors_XFwd_YLeft_ZUp( currentJoint->m_fwdDir, currentJoint->m_leftDir, currentJoint->m_upDir );
 			int brainCells = 0;
		}
		else
		{
  			//----------------------------------------------------------------------------------------------------------------------
  			// Axis angle rotation approach
  			//----------------------------------------------------------------------------------------------------------------------			
			// 1.  Transform target and EE pos to local space
			// 1a. Note: Target is transformed in 2 steps (World-To-Model, then Model-To-Local)
			//	   ALSO NOTE: "local space" means relative to parent (current joint's world is defined by parent IJKT)
			Mat44 modelToWorldMatrix		= m_eulerAngles_WS.GetAsMatrix_XFwd_YLeft_ZUp();
			modelToWorldMatrix.SetTranslation3D( m_position_WS );
			Mat44 worldToModelMatrix		= modelToWorldMatrix.GetOrthoNormalInverse();
			Vec3  target_MS					= worldToModelMatrix.TransformPosition3D( target.m_currentPos );
  			Mat44 localToModelMatrix		= currentJoint->m_parent->GetMatrix_LocalToModel();
  			Mat44 modelToLocalMatrix		= localToModelMatrix.GetOrthoNormalInverse();
  			Vec3  target_LS					= modelToLocalMatrix.TransformPosition3D( target_MS );
			// 1b. EE to localSpace
			//	   Note: EE is transformed in 2 steps (EeLocal-To-Model, then Model-To-curJointLocal)
			//	   ALSO NOTE: "local space" means relative to parent (current joint's world is defined by parent IJKT)
  			Mat44 endEffector_LToM_Matrix	= m_finalJoint->GetMatrix_LocalToModel();
  			Vec3  endEffectorPos_WS			= endEffector_LToM_Matrix.GetTranslation3D();												// 'LToW_Matrix' means 'localToWorldMatrix'
  			Vec3  endEffectorPos_MS			= modelToLocalMatrix.TransformPosition3D( endEffectorPos_WS	);
  			// 2. Compute disps
  			Vec3  curJointToEE_LS			= endEffectorPos_MS - currentJoint->m_jointPos_LS;
  			Vec3  curJointToTarget_LS		= target_LS - currentJoint->m_jointPos_LS;
  			// 3. Compute angle between disps
  			float angleToRotate				= GetAngleDegreesBetweenVectors3D( curJointToEE_LS, curJointToTarget_LS );
  			// 4. Compute rotation axis 
  			Vec3 rotationAxis				= CrossProduct3D( curJointToEE_LS, curJointToTarget_LS );
  			rotationAxis.Normalize();
  			// 5. Rotate using Axis-angle 
  			currentJoint->m_fwdDir			= RotateVectorAboutArbitraryAxis( currentJoint->m_fwdDir, rotationAxis, angleToRotate );
  			currentJoint->m_fwdDir.Normalize();
  			// 6. Update currentJoint eulerAngles
  			currentJoint->m_eulerAngles_LS  = GetEulerFromFwdDir( currentJoint, currentJoint->m_fwdDir );
			//----------------------------------------------------------------------------------------------------------------------
			// Roll hack
			// 6.5 Reinforce roll
//			Mat44 target_localToWorldMatrix = Mat44( target.m_fwdDir, target.m_leftDir, target.m_upDir, target.m_currentPos );
//  			tried hacking in roll but got confused thinking about how to transform the target's left vector
// 			to local space
// 			reinforcing the roll constraints for the final limb kind of makes sense but the transformation 
// 			is something I'll have to think about 
			//----------------------------------------------------------------------------------------------------------------------
  			// 7. Clamp eulerAngles if exceeding constrains
  			currentJoint->ClampYPR();
  			// 8. Re-update dir data again
  			currentJoint->m_eulerAngles_LS.GetAsVectors_XFwd_YLeft_ZUp( currentJoint->m_fwdDir, currentJoint->m_leftDir, currentJoint->m_upDir );
  			int brainCells = 0;
		}
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Keep track of best solution achieved this frame
	//----------------------------------------------------------------------------------------------------------------------
	float distAfterSolve_eeToTarget = GetDistEeToTarget( target );
	if ( distAfterSolve_eeToTarget < m_bestDistSolvedThisFrame )
	{
		// Update best solved dist if found
		m_bestDistSolvedThisFrame = distAfterSolve_eeToTarget;
	}
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::FABRIK_Forward( Target Target )
{
	m_finalJoint->DragLimb3D( Target );
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::Solve_FABRIK( Target target )
{
	int   m_numIterations		= 1;
	float toleranceDist			= 0.0001f;
	float distToTarget			= 0.0f;
	m_prevDistEE_EndToTarget	= GetDistance3D( m_finalJoint->m_endPos, target.m_currentPos );
	m_breakFABRIK				= false;
	for ( int i = 0; i < m_numIterations; i++ )
	{
		m_iterCount = i;

		// Forwards pass (child to parent)
		FABRIK_Forward( target );					// Sets finalLimb's endPos at targetPos then climbs up hierarchy chain (parents, grand-parents, etc) and sets their endPos at currentLimb's startPos accordingly

		if ( m_breakFABRIK )
		{
			break;
		}

		// Backwards pass (parent to child)
		FABRIK_Backward( target );
//		SolveTwoBoneIK_TriangulationMethod( target );

//		if ( m_limbList.size() == 3 )
//		{
//			SolveTwoBoneIK_TriangulationMethod( target );
//		}


		if ( m_breakFABRIK )
		{
			break;
		}
	}
}


//----------------------------------------------------------------------------------------------------------------------
// Root to child
//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::FABRIK_Backward( Target target )
{
	//----------------------------------------------------------------------------------------------------------------------
	// Ensure target is reachable ahead of time
	//----------------------------------------------------------------------------------------------------------------------
//	bool  isUnreachable			= false;
//	float distToEE				= ( Target.m_currentPos - m_position ).GetLength();
//	Vec3  dirBaseToEE			= ( Target.m_currentPos - m_position ).GetNormalized();
//	float maxLength				= GetTotalLengthOfThisSkeletalSystem();
//	float lengthParentToTarget	= ( m_position - m_target.m_currentPos ).GetLength();
//	if ( distToEE > maxLength )
//	{
//		isUnreachable = true;
//	}

	for ( int i = 0; i < m_jointList.size(); i++ )
	{
		IK_Joint3D* currentLimb = m_jointList[i];
		IK_Joint3D* parentLimb  = currentLimb->m_parent;

		//----------------------------------------------------------------------------------------------------------------------
		// Check Termination condition
		//----------------------------------------------------------------------------------------------------------------------
//		if ( isUnreachable )
//		{
//			if ( currentLimb->m_parent == nullptr )
//			{
//				currentLimb->m_startPos = m_position;
//				currentLimb->m_fwdDir = dirBaseToEE;
//				currentLimb->m_endPos = currentLimb->GetLimbEndMaxLength_IK();
//				continue;
//			}
//			else
//			{
//				currentLimb->m_startPos = currentLimb->m_parent->GetLimbEndMaxLength_IK();
//				currentLimb->m_fwdDir = dirBaseToEE;
//				currentLimb->m_endPos = currentLimb->GetLimbEndMaxLength_IK();
//				continue;
//			}
//		} 

		if ( currentLimb->m_jointConstraintType == JOINT_CONSTRAINT_TYPE_DISTANCE )
		{
			if ( m_parentChain != nullptr )
			{
				if ( currentLimb->m_parent != nullptr )
				{
					// Logic for children limbs (has a parent)
					currentLimb->m_jointPos_LS = currentLimb->m_parent->GetLimbEnd();
					currentLimb->m_endPos = currentLimb->GetLimbEnd();
				}
				else
				{
					// Logic for parent limb (first in hierarchy, does NOT have a parent)
					currentLimb->m_jointPos_LS = m_position_WS;
					currentLimb->m_endPos = currentLimb->GetLimbEnd();
				}
			}
			else
			{
				if ( currentLimb->m_parent != nullptr )
				{
					// Logic for children limbs (has a parent)
					currentLimb->m_jointPos_LS = currentLimb->m_parent->GetLimbEnd();
					currentLimb->m_endPos = currentLimb->GetLimbEnd();
				}
				else
				{
					// Logic for parent limb (first in hierarchy, does NOT have a parent)
					currentLimb->m_jointPos_LS = m_position_WS;
					currentLimb->m_endPos = currentLimb->GetLimbEnd();
				}
			}
		}
		else if ( currentLimb->m_jointConstraintType == JOINT_CONSTRAINT_TYPE_BALL_AND_SOCKET )
		{
			// Cone constraints V2
			//----------------------------------------------------------------------------------------------------------------------
			// Default FARBIK reach algorithm
			//----------------------------------------------------------------------------------------------------------------------
			if ( ( currentLimb->m_parent == nullptr ) && ( currentLimb->m_child == nullptr ) )
			{
				// Logic for ONLY limb (first in hierarchy, does NOT have a parent NOR a child )
				currentLimb->m_jointPos_LS = m_position_WS;
				currentLimb->m_fwdDir	= ( target.m_currentPos - currentLimb->m_jointPos_LS ).GetNormalized();
//				currentLimb->m_fwdDir	= Target.m_fwdDir;
				currentLimb->m_endPos	= currentLimb->GetLimbEnd();
			}
			else if ( currentLimb->m_parent == nullptr )
			{
				// Logic for parent limb (first in hierarchy, does NOT have a parent)
				currentLimb->m_jointPos_LS = m_position_WS;
				currentLimb->m_fwdDir	= ( currentLimb->m_endPos - currentLimb->m_jointPos_LS ).GetNormalized();
				currentLimb->m_endPos	= currentLimb->GetLimbEnd();
			}
			else
			{
				// Logic for children limbs (has a parent)
				currentLimb->m_jointPos_LS = currentLimb->m_parent->GetLimbEnd();
				currentLimb->m_fwdDir   = ( target.m_currentPos - currentLimb->m_jointPos_LS ).GetNormalized();
				currentLimb->m_endPos	= currentLimb->GetLimbEnd();
			}

			//----------------------------------------------------------------------------------------------------------------------
			// Angle Axis rotations approach for angle clamping
			//----------------------------------------------------------------------------------------------------------------------
			// Convert from world-local-polar space and check if current angle (theta) is valid
			if ( currentLimb->m_parent == nullptr )
			{
				// Hard coding the default direction of the 'ONLY child limb' to face world -Z
				float angle					= GetAngleDegreesBetweenVectors3D( currentLimb->m_fwdDir, currentLimb->m_refVector );
				float maxAngle				= currentLimb->m_yawConstraints_LS.m_max;
				if ( angle > maxAngle )
				{
					float deltaAngle		= angle - maxAngle;
					Vec3 vectorToRotate		= ( currentLimb->m_endPos - currentLimb->m_jointPos_LS ).GetNormalized();
					Vec3 arbitraryAxis		= CrossProduct3D( currentLimb->m_fwdDir, currentLimb->m_refVector ).GetNormalized();
					vectorToRotate			= RotateVectorAboutArbitraryAxis( vectorToRotate, arbitraryAxis, deltaAngle );
					currentLimb->m_fwdDir	= vectorToRotate.GetNormalized();
					currentLimb->m_endPos	= currentLimb->GetLimbEnd();
				}

/*
				//----------------------------------------------------------------------------------------------------------------------
				// Rotate parent more
				//----------------------------------------------------------------------------------------------------------------------
				if ( m_iterCount == 0 && m_shouldBendMore )
				{
//					currentLimb->m_fwdDir			= RotateVectorAboutArbitraryAxis( currentLimb->m_fwdDir, currentLimb->m_leftDir, m_bendMoreDegrees_current );
					currentLimb->m_fwdDir			= RotateVectorAboutArbitraryAxis( currentLimb->m_fwdDir, m_ownerSkeletonFirstJoint->m_leftDir, m_bendMoreDegrees_current );
					// Set direction and orientation towards targetPos
					currentLimb->m_startPos			= m_position;
					currentLimb->m_endPos			= currentLimb->m_startPos + ( currentLimb->m_fwdDir * currentLimb->m_length );
					currentLimb->m_upDir			= CrossProduct3D( currentLimb->m_fwdDir, currentLimb->m_leftDir );
					currentLimb->m_upDir.Normalize();
					currentLimb->m_axisOfRotation	= currentLimb->m_leftDir;
				}
*/


				//----------------------------------------------------------------------------------------------------------------------
				// Clamp roll (left vector)
				//----------------------------------------------------------------------------------------------------------------------
				// project currentLeft onto rootLeft
				// compute delta degrees
				// clamp if greater than max degrees
				float angleLeftToRootLeft	= GetSignedAngleDegreesBetweenVectors( m_creatureOwner->m_root->m_leftDir, currentLimb->m_leftDir, m_creatureOwner->m_root->m_upDir );
				maxAngle					= currentLimb->m_rollConstraints_LS.m_max;
				float minAngle				= currentLimb->m_rollConstraints_LS.m_min;
				if ( angleLeftToRootLeft > maxAngle )
				{
					Vec3 arbitraryAxis		= CrossProduct3D( currentLimb->m_leftDir, m_creatureOwner->m_root->m_leftDir ).GetNormalized();
					currentLimb->m_leftDir	= RotateVectorAboutArbitraryAxis( m_creatureOwner->m_root->m_leftDir, arbitraryAxis, maxAngle );
					currentLimb->m_leftDir.Normalize();
				}
				else if ( angleLeftToRootLeft < minAngle )
				{
					Vec3 arbitraryAxis		= CrossProduct3D( currentLimb->m_leftDir, m_creatureOwner->m_root->m_leftDir ).GetNormalized();
					currentLimb->m_leftDir	= RotateVectorAboutArbitraryAxis( m_creatureOwner->m_root->m_leftDir, arbitraryAxis, minAngle );
					currentLimb->m_leftDir.Normalize();
				}
				//----------------------------------------------------------------------------------------------------------------------
				// Compute Basis vectors based on fwdDir
				//----------------------------------------------------------------------------------------------------------------------
				currentLimb->m_upDir			= CrossProduct3D( currentLimb->m_fwdDir, currentLimb->m_leftDir );
				currentLimb->m_upDir.Normalize();
				currentLimb->m_leftDir			= CrossProduct3D( currentLimb->m_upDir, currentLimb->m_fwdDir );
				currentLimb->m_leftDir.Normalize();

				//----------------------------------------------------------------------------------------------------------------------
				// Clamp roll (left vector) AGAIN
				//----------------------------------------------------------------------------------------------------------------------
				// project currentLeft onto rootLeft
				// compute delta degrees
				// clamp if greater than max degrees
				angleLeftToRootLeft			= GetSignedAngleDegreesBetweenVectors( m_creatureOwner->m_root->m_leftDir, currentLimb->m_leftDir, m_creatureOwner->m_root->m_upDir );
				maxAngle					= currentLimb->m_rollConstraints_LS.m_max;
				minAngle					= currentLimb->m_rollConstraints_LS.m_min;
				if ( angleLeftToRootLeft > maxAngle )
				{
					Vec3 arbitraryAxis		= CrossProduct3D( currentLimb->m_leftDir, m_creatureOwner->m_root->m_leftDir ).GetNormalized();
					currentLimb->m_leftDir	= RotateVectorAboutArbitraryAxis( m_creatureOwner->m_root->m_leftDir, arbitraryAxis, maxAngle );
					currentLimb->m_leftDir.Normalize();
				}
				else if ( angleLeftToRootLeft < minAngle )
				{
					Vec3 arbitraryAxis		= CrossProduct3D( currentLimb->m_leftDir, m_creatureOwner->m_root->m_leftDir ).GetNormalized();
					currentLimb->m_leftDir	= RotateVectorAboutArbitraryAxis( m_creatureOwner->m_root->m_leftDir, arbitraryAxis, minAngle );
					currentLimb->m_leftDir.Normalize();
				}
				currentLimb->m_fwdDir			= CrossProduct3D( currentLimb->m_leftDir, currentLimb->m_upDir );
				currentLimb->m_fwdDir.Normalize();
				// #ToDo: Handle edge case when leftDir is facing worldUp

				// Set start and end pos based on new dir vectors
				currentLimb->m_jointPos_LS = m_position_WS;
				currentLimb->m_endPos	= currentLimb->GetLimbEnd();
				currentLimb->m_axisOfRotation	= currentLimb->m_leftDir;
			}
			else	// Logic for children
			{
				//----------------------------------------------------------------------------------------------------------------------
				// Clamp yaw pitch (fwd vector)
				//----------------------------------------------------------------------------------------------------------------------
				// project currentLeft onto rootLeft
				// compute delta degrees
				// clamp if greater than max degrees
				float angleFwdToParentFwd	= GetAngleDegreesBetweenVectors3D( currentLimb->m_parent->m_fwdDir, currentLimb->m_fwdDir );
				float maxAngle				= currentLimb->m_yawConstraints_LS.m_max;
				if ( angleFwdToParentFwd > maxAngle )
				{
					float deltaAngle		= angleFwdToParentFwd - maxAngle;
					Vec3 arbitraryAxis		= CrossProduct3D( currentLimb->m_fwdDir, currentLimb->m_parent->m_fwdDir ).GetNormalized();
					currentLimb->m_fwdDir	= RotateVectorAboutArbitraryAxis( currentLimb->m_fwdDir, arbitraryAxis, deltaAngle );
					currentLimb->m_fwdDir.Normalize();
				}

				currentLimb->m_leftDir = target.m_leftDir;
				//----------------------------------------------------------------------------------------------------------------------
				// Clamp roll (left vector)
				//----------------------------------------------------------------------------------------------------------------------
				// project currentLeft onto rootLeft
				// compute delta degrees
				// clamp if greater than max degrees
				float angleLeftToRootLeft	= GetAngleDegreesBetweenVectors3D( currentLimb->m_leftDir, currentLimb->m_parent->m_leftDir );
				maxAngle					= currentLimb->m_rollConstraints_LS.m_max;
				if ( angleLeftToRootLeft > maxAngle )
				{
					float deltaAngle		= angleLeftToRootLeft - maxAngle;
					Vec3 arbitraryAxis		= CrossProduct3D( currentLimb->m_leftDir, currentLimb->m_parent->m_leftDir ).GetNormalized();
					currentLimb->m_leftDir	= RotateVectorAboutArbitraryAxis( currentLimb->m_leftDir, arbitraryAxis, deltaAngle );
					currentLimb->m_leftDir.Normalize();
				}

				currentLimb->m_upDir   = CrossProduct3D( currentLimb->m_fwdDir, currentLimb->m_leftDir );
				currentLimb->m_upDir.Normalize();
				currentLimb->m_leftDir = CrossProduct3D( currentLimb->m_upDir, currentLimb->m_fwdDir );
				currentLimb->m_leftDir.Normalize();

				//----------------------------------------------------------------------------------------------------------------------
				// Clamp roll (left vector) AGAIN
				//----------------------------------------------------------------------------------------------------------------------
				// project currentLeft onto rootLeft
				// compute delta degrees
				// clamp if greater than max degrees
				angleLeftToRootLeft			= GetAngleDegreesBetweenVectors3D( currentLimb->m_leftDir, currentLimb->m_parent->m_leftDir );
				maxAngle					= currentLimb->m_rollConstraints_LS.m_max;
				if ( angleLeftToRootLeft > maxAngle )
				{
					float deltaAngle		= angleLeftToRootLeft - maxAngle;
					Vec3 arbitraryAxis		= CrossProduct3D( currentLimb->m_leftDir, currentLimb->m_parent->m_leftDir ).GetNormalized();
					currentLimb->m_leftDir	= RotateVectorAboutArbitraryAxis( currentLimb->m_leftDir, arbitraryAxis, deltaAngle );
					currentLimb->m_leftDir.Normalize();
				}
			}

/*
				Vec3 parentFwd			= currentLimb->m_parent->m_fwdDir.GetNormalized();
				Vec3 BC					= Target.m_currentPos - currentLimb->m_startPos;
				BC						= BC.GetNormalized();
				Vec3 currentFwd			= BC;
				float dotProductResult	= DotProduct3D( BC, parentFwd );

				BC						*= currentLimb->m_length;
				Vec3 BCn				= parentFwd * DotProduct3D( BC, parentFwd );
				float lengthBCn			= BCn.GetLength();
				float angleRadians		= acosf( lengthBCn / currentLimb->m_length );
				float angleDegrees		= ConvertRadiansToDegrees( angleRadians );
				if ( dotProductResult < 0 )
				{
					angleDegrees  = 180.0f - angleDegrees;
				}
				float maxAngle	= currentLimb->m_yawConstraints.m_max;
				if ( angleDegrees > maxAngle )
				{
					// Check angle is invalid (out of bounds), rotate deltaTheta about arbitrary axis
					float deltaAngle		= angleDegrees - maxAngle;
					Vec3 currentEnd			= currentLimb->m_startPos + (currentFwd * currentLimb->m_length );
					Vec3 vectorToRotate		= currentEnd - currentLimb->m_startPos;
					Vec3 arbitraryAxis		= CrossProduct3D( BC, parentFwd ).GetNormalized();

					vectorToRotate			= RotateVectorAboutArbitraryAxis( vectorToRotate, arbitraryAxis, deltaAngle );
					currentLimb->m_fwdDir	= vectorToRotate.GetNormalized();
					currentLimb->m_endPos	= currentLimb->GetLimbEndMaxLength_IK();
				}
*/
/*
			// Cone constraints V1
			//----------------------------------------------------------------------------------------------------------------------
			// Default FARBIK reach algorithm
			//----------------------------------------------------------------------------------------------------------------------
			if ( ( currentLimb->m_parent == nullptr ) && ( currentLimb->m_child == nullptr ) )
			{
				// Logic for ONLY limb (first in hierarchy, does NOT have a parent NOR a child )
				currentLimb->m_startPos = m_position;
				currentLimb->m_fwdDir	= ( Target.m_currentPos - currentLimb->m_startPos ).GetNormalized();
				currentLimb->m_endPos	= currentLimb->GetLimbEndMaxLength_IK();

				//----------------------------------------------------------------------------------------------------------------------
				// Compute basis vectors
				//----------------------------------------------------------------------------------------------------------------------
				Vec3  worldUp	= Vec3( 0.0f, 0.0f, 1.0f );
				Vec3  worldLeft	= Vec3( 0.0f, 0.0f, 1.0f );
				float dotResult = DotProduct3D( currentLimb->m_fwdDir, worldUp );
				if ( dotResult < 1.0f )
				{
					currentLimb->m_leftDir	= CrossProduct3D( worldUp, currentLimb->m_fwdDir );
					currentLimb->m_leftDir.Normalize();
					currentLimb->m_upDir	= CrossProduct3D( currentLimb->m_fwdDir, currentLimb->m_leftDir );
					currentLimb->m_upDir.Normalize();
				}
				else
				{
					currentLimb->m_upDir   = CrossProduct3D( currentLimb->m_fwdDir, worldLeft );
					currentLimb->m_upDir.Normalize();
					currentLimb->m_leftDir = CrossProduct3D( currentLimb->m_upDir, currentLimb->m_fwdDir );
					currentLimb->m_leftDir.Normalize();
				}
			}
			else if ( currentLimb->m_parent == nullptr )
			{
				// Logic for parent limb (first in hierarchy, does NOT have a parent)
				currentLimb->m_startPos = m_position;
				currentLimb->m_endPos	= currentLimb->GetLimbEndMaxLength_IK();
/*
				//----------------------------------------------------------------------------------------------------------------------
				// Compute basis vectors
				//----------------------------------------------------------------------------------------------------------------------
				Vec3  worldUp	= Vec3( 0.0f, 0.0f, 1.0f );
				Vec3  worldLeft	= Vec3( 0.0f, 0.0f, 1.0f );
				float dotResult = fabsf( DotProduct3D( currentLimb->m_fwdDir, worldUp ) );
				if ( dotResult < 0.9999f )
				{
					currentLimb->m_leftDir	= CrossProduct3D( worldUp, currentLimb->m_fwdDir );
					currentLimb->m_leftDir.Normalize();
					currentLimb->m_upDir	= CrossProduct3D( currentLimb->m_fwdDir, currentLimb->m_leftDir );
					currentLimb->m_upDir.Normalize();
				}
				else
				{
					currentLimb->m_upDir = CrossProduct3D( currentLimb->m_fwdDir, worldLeft );
					currentLimb->m_upDir.Normalize();
					currentLimb->m_leftDir = CrossProduct3D( currentLimb->m_upDir, currentLimb->m_fwdDir );
					currentLimb->m_leftDir.Normalize();
				}
* /
			}
			else
			{
				// Logic for children limbs (has a parent)
				currentLimb->m_startPos = currentLimb->m_parent->GetLimbEndMaxLength_IK();
				currentLimb->m_endPos	= currentLimb->GetLimbEndMaxLength_IK();
			}

			//----------------------------------------------------------------------------------------------------------------------
			// Angle Axis rotations approach for angle clamping
			//----------------------------------------------------------------------------------------------------------------------
			// Convert from world-local-polar space and check if current angle (theta) is valid
			if ( currentLimb->m_parent == nullptr )
			{
				// Hard coding the default direction of the 'ONLY child limb' to face world -Z
				Vec3 worldGround_Z			= Vec3( 0.0f, 0.0f, -1.0f );
				float angle = GetAngleDegreesBetweenVectors3D( currentLimb->m_fwdDir, currentLimb->m_refVector );
				if ( currentLimb->m_refVector == Vec3::ZERO )
				{
					angle = GetAngleDegreesBetweenVectors3D( currentLimb->m_fwdDir, worldGround_Z );
				}
				float maxAngle				= currentLimb->m_yawConstraints.m_max;
				if ( angle > maxAngle )
				{
					float deltaAngle		= angle - maxAngle;
					Vec3 currentEnd			= currentLimb->m_startPos + ( currentLimb->m_fwdDir * currentLimb->m_length );
					Vec3 vectorToRotate		= currentEnd - currentLimb->m_startPos;
					Vec3 arbitraryAxis		= CrossProduct3D( currentLimb->m_fwdDir, worldGround_Z ).GetNormalized();
					vectorToRotate			= RotateVectorAboutArbitraryAxis( vectorToRotate, arbitraryAxis, deltaAngle );
					currentLimb->m_fwdDir	= vectorToRotate.GetNormalized();
					currentLimb->m_endPos	= currentLimb->GetLimbEndMaxLength_IK();
				}

				//----------------------------------------------------------------------------------------------------------------------
				// Compute Basis vectors based on fwdDir
				//----------------------------------------------------------------------------------------------------------------------
				currentLimb->m_leftDir			= CrossProduct3D( currentLimb->m_upDir, currentLimb->m_fwdDir );
				currentLimb->m_leftDir.Normalize();
				currentLimb->m_upDir			= CrossProduct3D( currentLimb->m_fwdDir, currentLimb->m_leftDir );
				currentLimb->m_upDir.Normalize();
				currentLimb->m_axisOfRotation	= currentLimb->m_leftDir;
				// #ToDo: Handle edge case when fwdDir is facing worldUp
			}
			else	// Logic for children
			{
				currentLimb->m_endPos	= currentLimb->GetLimbEndMaxLength_IK();
				Vec3 parentFwd			= currentLimb->m_parent->m_fwdDir.GetNormalized();
				Vec3 BC					= Target.m_currentPos - currentLimb->m_startPos;
				BC						= BC.GetNormalized();
				Vec3 currentFwd			= BC;
				float dotProductResult	= DotProduct3D( BC, parentFwd );

				BC						*= currentLimb->m_length;
				Vec3 BCn				= parentFwd * DotProduct3D( BC, parentFwd );
				float lengthBCn			= BCn.GetLength();
				float angleRadians		= acosf( lengthBCn / currentLimb->m_length );
				float angleDegrees		= ConvertRadiansToDegrees( angleRadians );
				if ( dotProductResult < 0 )
				{
					angleDegrees  = 180.0f - angleDegrees;
				}
				float maxAngle	= currentLimb->m_yawConstraints.m_max;
				if ( angleDegrees > maxAngle )
				{
					// Check angle is invalid (out of bounds), rotate deltaTheta about arbitrary axis
					float deltaAngle		= angleDegrees - maxAngle;
					Vec3 currentEnd			= currentLimb->m_startPos + (currentFwd * currentLimb->m_length );
					Vec3 vectorToRotate		= currentEnd - currentLimb->m_startPos;
					Vec3 arbitraryAxis		= CrossProduct3D( BC, parentFwd ).GetNormalized();

					vectorToRotate			= RotateVectorAboutArbitraryAxis( vectorToRotate, arbitraryAxis, deltaAngle );
					currentLimb->m_fwdDir	= vectorToRotate.GetNormalized();
					currentLimb->m_endPos	= currentLimb->GetLimbEndMaxLength_IK();
				}
			}
*/
		}
		else if ( currentLimb->m_jointConstraintType == JOINT_CONSTRAINT_TYPE_HINGE )
		{
			if ( currentLimb->m_parent != nullptr )
			{
				// Logic for children limbs (has a parent)
				currentLimb->m_jointPos_LS = currentLimb->m_parent->GetLimbEnd();
				currentLimb->m_endPos = currentLimb->GetLimbEnd();
			}
			else
			{
				// Logic for parent limb (first in hierarchy, does NOT have a parent)
				currentLimb->m_jointPos_LS = m_position_WS;
				currentLimb->m_endPos = currentLimb->GetLimbEnd();
			}

			/*
						//----------------------------------------------------------------------------------------------------------------------
						// Default FABRIK reach algorithm
						//----------------------------------------------------------------------------------------------------------------------
						if ( currentLimb->m_parent == nullptr )
						{
							// Logic for parent limb (first in hierarchy, does NOT have a parent)
							currentLimb->m_startPos = m_position;
							currentLimb->m_fwdDir	= ( currentLimb->m_child->m_startPos - currentLimb->m_startPos ).GetNormalized();
							currentLimb->m_leftDir	= RotateVectorAboutArbitraryAxis( currentLimb->m_fwdDir, currentLimb->m_axisOfRotation, 90.0f );
							currentLimb->m_leftDir	= currentLimb->m_leftDir.GetNormalized();
							currentLimb->m_upDir	= RotateVectorAboutArbitraryAxis( currentLimb->m_fwdDir, currentLimb->m_leftDir, -90.0f );
							currentLimb->m_upDir	= currentLimb->m_upDir.GetNormalized();
							currentLimb->m_endPos	= currentLimb->GetLimbEndMaxLength_IK();
						}
						else
						{
							// Logic for children limbs (has a parent)
							currentLimb->m_startPos = currentLimb->m_parent->GetLimbEndMaxLength_IK();
							if ( currentLimb->m_child == nullptr )
							{
								currentLimb->m_fwdDir	= Target.m_fwdDir;
								currentLimb->m_leftDir	= Target.m_leftDir;
								currentLimb->m_upDir	= Target.m_upDir;
								currentLimb->m_startPos = currentLimb->m_parent->GetLimbEndMaxLength_IK();
							}
							else
							{
								currentLimb->m_fwdDir	= ( currentLimb->m_child->m_startPos - currentLimb->m_startPos ).GetNormalized();
								currentLimb->m_startPos = currentLimb->m_parent->GetLimbEndMaxLength_IK();
							}
							currentLimb->m_endPos = currentLimb->GetLimbEndMaxLength_IK();
						}

						//----------------------------------------------------------------------------------------------------------------------
						// New approach to angle constraints
						//----------------------------------------------------------------------------------------------------------------------
						// (Startup)
						// 1. Pass in the axisOfRotation as a parameter into this function from game
						//		1a. Set m_axisOfRotation = axisOfRotation after parsing which parent's basis to use
						// (Update)
						// 2. Set m_axisOfRotation to parentUp
						//		2a. m_currentLeft = Cross( currentUp, currentFwd );
						// 3. Rotate copy of m_currentUp 90 degrees along m_currentLeft axis. ( This will make our currentFwd perpendicular to our currentUp )
						//		3a. m_currentFwd = rotatedVector;
						// 4. Project dispCurrentStartToTarget onto parentFwd
						//		4a. Solve for angle between dispCurrentStartToTarget and parentFwd using acos( projectedLength / currentLength )
						// 5. Solve if we need to rotate CCW (counter-clockwise) or CW (clockwise)
						//		5a. DotProduct( dispCurrentStartToTarget, parentLeft )
						//		5b. if ( dot > 0.0f ) { // we need to rotate CCW by deltaDegrees }
						//		5c. else { // we need to rotate CW by deltaDegrees }
						//		5d. Solve deltaDegrees
						//				5d1. float deltaDegrees = currentAngle - maxAngle ( CCW or CW );
						// 6. Clamp dispCurrentStartToTarget
						//		6a. currentFwd = RotateAboutArbitraryAxis( dispCurrentStartToTarget, left (axisOfRotation), deltaDegrees );
						//		6b. currentEnd = currentStart + ( currentFwd * currentLength );
						// 7. Update m_currentLeftDir
						//		7.a m_currentLeftDir = RotateAboutArbitraryAxis( fwdDir_copy, m_currentUpDir (axisOfRotation), 90.0f );

						if ( currentLimb->m_parent == nullptr )
						{

							// Logic for the base limb
							// currentLimb->m_fwdDir			= Vec3(  0.0f, 0.0f, 1.0f );
							// currentLimb->m_leftDir			= Vec3(  0.0f, 1.0f, 0.0f );
							// currentLimb->m_upDir				= Vec3( -1.0f, 0.0f, 0.0f );
							// currentLimb->m_axisOfRotation	= Vec3( -1.0f, 0.0f, 0.0f );
							// currentLimb->m_endPos			= currentLimb->m_startPos + currentLimb->m_fwdDir * currentLimb->m_length;
						}
						else if ( currentLimb->m_child != nullptr )
						{
							//----------------------------------------------------------------------------------------------------------------------
							// Compute currentJoint's basis vectors, relative to parent
							//----------------------------------------------------------------------------------------------------------------------

							// #ToDo: change this hard coded axisOfRotation later by setting it in the constructor.
							// Have the user pass down the "desired" axis of rotation through game code.
							currentLimb->m_axisOfRotation = currentLimb->m_parent->m_upDir;

							// Flatten fwdDir onto the "2D" plane to make it perpendicular to the upDir.
							// At this point, the fwdDir is not facing the correct direction NOR has it been clamped.
							// It's just rotated onto the plane where we can check against the parent for clamping.
							// The "left" is also still incorrect, but that will be computed at end of this function once all calculations are complete.
							currentLimb->m_upDir	= currentLimb->m_parent->m_upDir;
							Vec3 tempFwd			= ( currentLimb->m_child->m_startPos - currentLimb->m_startPos ).GetNormalized();
							Vec3 tempLeft			= Vec3( 0.0f, 1.0f, 0.0f );
							if ( tempFwd == currentLimb->m_upDir )
							{
								tempLeft			= CrossProduct3D( currentLimb->m_upDir, Vec3( 1.0f, 0.0f, 0.0f ) );
							}
							else
							{
								tempLeft			= CrossProduct3D( currentLimb->m_upDir, tempFwd );
							}
							tempLeft				= tempLeft.GetNormalized();
							currentLimb->m_fwdDir	= RotateVectorAboutArbitraryAxis( currentLimb->m_upDir, tempLeft, 90.0f );							// #FixLater: Handle edge case if rotating 90 degrees from "up" results with the fwd Dir facing the "backwards" dir
							currentLimb->m_fwdDir	= currentLimb->m_fwdDir.GetNormalized();

							//----------------------------------------------------------------------------------------------------------------------
							// Compute angle between currentFwd and parentFwd to clamp the fwd according to player specified constraints
							//----------------------------------------------------------------------------------------------------------------------
							float angleDispAndParentFwd = GetAngleDegreesBetweenVectors3D( currentLimb->m_fwdDir, currentLimb->m_parent->m_fwdDir );	// #FixLater: Make sure the angle computed is correct. Figure out when the edge case occurs and we need to subtract 180 from this angle
							// Before we clamp the limb, we need to check which way to rotate (CCW or CW)
							bool  isCCW		= true;
							float dotResult = DotProduct3D( currentLimb->m_fwdDir, currentLimb->m_parent->m_leftDir );
							if ( dotResult > 0.0f )
							{
								isCCW = false;
							}
							else
							{
								isCCW = true;
							}

							// Solve deltaDegrees to determine what direction and how much to rotate our fwd.
							float deltaDegrees = 0.0f;
							if ( isCCW )
							{
								if ( angleDispAndParentFwd > currentLimb->m_yawConstraints.m_min )
								{
									deltaDegrees = angleDispAndParentFwd - currentLimb->m_yawConstraints.m_min;
								}
							}
							else
							{
								if ( angleDispAndParentFwd > currentLimb->m_yawConstraints.m_max )
								{
									deltaDegrees = currentLimb->m_yawConstraints.m_max - angleDispAndParentFwd;
								}
							}

							// Clamp dispStartToTarget
							currentLimb->m_fwdDir	= RotateVectorAboutArbitraryAxis( currentLimb->m_fwdDir, currentLimb->m_upDir, deltaDegrees );
							currentLimb->m_fwdDir	= currentLimb->m_fwdDir.GetNormalized();
							// Update member variables accordingly
							currentLimb->m_leftDir	= RotateVectorAboutArbitraryAxis( currentLimb->m_fwdDir, currentLimb->m_upDir, 90.0f );
							currentLimb->m_endPos	= currentLimb->m_startPos + ( currentLimb->m_fwdDir * currentLimb->m_length );
						}
						else
						{
							//----------------------------------------------------------------------------------------------------------------------
							// Compute currentJoint's basis vectors, relative to parent
							//----------------------------------------------------------------------------------------------------------------------

							// #ToDo: change this hard coded axisOfRotation later by setting it in the constructor.
							// Have the user pass down the "desired" axis of rotation through game code.
							currentLimb->m_axisOfRotation = currentLimb->m_parent->m_upDir;

							// Flatten fwdDir onto the "2D" plane to make it perpendicular to the upDir.
							// At this point, the fwdDir is not facing the correct direction NOR has it been clamped.
							// It's just rotated onto the plane where we can check against the parent for clamping.
							// The "left" is also still incorrect, but that will be computed at end of this function once all calculations are complete.
							currentLimb->m_upDir	= currentLimb->m_parent->m_upDir;
							Vec3 tempFwd			= ( Target.m_currentPos - currentLimb->m_startPos ).GetNormalized();
							Vec3 tempLeft			= Target.m_leftDir;
							if ( tempFwd == currentLimb->m_upDir )
							{
								tempLeft			= CrossProduct3D( currentLimb->m_upDir, Target.m_fwdDir );
							}
							else
							{
								tempLeft			= CrossProduct3D( currentLimb->m_upDir, tempFwd );
							}
							tempLeft				= tempLeft.GetNormalized();
							currentLimb->m_fwdDir	= RotateVectorAboutArbitraryAxis( currentLimb->m_upDir, tempLeft, 90.0f );							// #FixLater: Handle edge case if rotating 90 degrees from "up" results with the fwd Dir facing the "backwards" dir
							currentLimb->m_fwdDir	= currentLimb->m_fwdDir.GetNormalized();

							//----------------------------------------------------------------------------------------------------------------------
							// Compute angle between currentFwd and parentFwd to clamp the fwd according to player specified constraints
							//----------------------------------------------------------------------------------------------------------------------
							float angleDispAndParentFwd = GetAngleDegreesBetweenVectors3D( currentLimb->m_fwdDir, currentLimb->m_parent->m_fwdDir );	// #FixLater: Make sure the angle computed is correct. Figure out when the edge case occurs and we need to subtract 180 from this angle
							// Before we clamp the limb, we need to check which way to rotate (CCW or CW)
							bool  isCCW			= true;
							Vec3  dirStartToEE	= ( Target.m_currentPos - currentLimb->m_startPos ).GetNormalized();
							float dotResult		= DotProduct3D( dirStartToEE, currentLimb->m_parent->m_leftDir );
							if ( dotResult > 0.0f )
							{
								isCCW = false;
							}
							else
							{
								isCCW = true;
							}

							// Solve deltaDegrees to determine what direction and how much to rotate our fwd.
							float deltaDegrees = 0.0f;
							if ( isCCW )
							{
								if ( angleDispAndParentFwd > currentLimb->m_yawConstraints.m_min )
								{
									deltaDegrees	= angleDispAndParentFwd - currentLimb->m_yawConstraints.m_min;
									isUnreachable	= true;
									i				= -1;
								}
							}
							else
							{
								if ( angleDispAndParentFwd > currentLimb->m_yawConstraints.m_max )
								{
									deltaDegrees = currentLimb->m_yawConstraints.m_max - angleDispAndParentFwd;
								}
							}

							// Clamp dispStartToTarget and update member variables accordingly
							currentLimb->m_fwdDir	= RotateVectorAboutArbitraryAxis( currentLimb->m_fwdDir, currentLimb->m_upDir, deltaDegrees );
							currentLimb->m_fwdDir	= currentLimb->m_fwdDir.GetNormalized();
							currentLimb->m_leftDir	= RotateVectorAboutArbitraryAxis( currentLimb->m_fwdDir, currentLimb->m_upDir, 90.0f );
							currentLimb->m_endPos	= currentLimb->m_startPos + ( currentLimb->m_fwdDir * currentLimb->m_length );
						}
			*/


			//----------------------------------------------------------------------------------------------------------------------
			// Working version
			//----------------------------------------------------------------------------------------------------------------------
/*
			//----------------------------------------------------------------------------------------------------------------------
			// Solving each joint's up and left direction vectors based on the axisOfRotation computed from CrossProduct( currentLimb's FwdDir, targetUp )
			//----------------------------------------------------------------------------------------------------------------------
			// 1. Solve cross product to get the axis of rotation
			// 2. Rotate a normalized vector 90 degrees (counter clockwise) from the fwdDir and set that rotatedVector as the upDir
			currentLimb->m_axisOfRotation	= CrossProduct3D( currentLimb->m_fwdDir, m_target.m_leftDir );		// Take the negative TargetLeft vector
			currentLimb->m_axisOfRotation	= currentLimb->m_axisOfRotation.GetNormalized();
			float dotResult					= DotProduct3D( currentLimb->m_fwdDir, m_target.m_leftDir );
			if ( fabsf( dotResult ) >= 1.0f )
			{
				// Make sure currentLimb's fwd is not facing the same dir as the Target's upDir because the crossResult will be ZERO.
				currentLimb->m_axisOfRotation	= CrossProduct3D( currentLimb->m_fwdDir, -m_target.m_fwdDir );	// Take the TargetFwd vector
				currentLimb->m_axisOfRotation	= currentLimb->m_axisOfRotation.GetNormalized();
			}

			//	We compute our "up" by rotating 90 from our fwd
			//	The rotationAxis is determined by using the crossProduct method above
			//	It considers the Target's directions (up or negated fwd) and our currentLimb's fwdDir

			Vec3 newUpLeft			= RotateVectorAboutArbitraryAxis( currentLimb->m_fwdDir, currentLimb->m_axisOfRotation, 90.0f );
			currentLimb->m_leftDir	= newUpLeft.GetNormalized();
			Vec3 newUp				= CrossProduct3D( currentLimb->m_fwdDir, currentLimb->m_leftDir );
			currentLimb->m_upDir	= newUp.GetNormalized();


			//----------------------------------------------------------------------------------------------------------------------
			// New Version for attempting hinge constraints using the pre-computed angleAxis rotations
			//----------------------------------------------------------------------------------------------------------------------
			// Convert from world-local-polar space and check if current angle (theta) is valid
			if ( currentLimb->m_parent != nullptr )
			{
				// Project currentStartToTargetPos onto parentFwd to compute angle offset from parent
				Vec3 dispStartToTarget							= targetPos - currentLimb->m_startPos;
				dispStartToTarget								= dispStartToTarget.GetNormalized();
				Vec3 currentFwd									= dispStartToTarget;
				dispStartToTarget								*= currentLimb->m_length;
				Vec3 parentFwd									= currentLimb->m_parent->m_fwdDir.GetNormalized();
				Vec3 dispStartToTargetProjectedOntoParentFwd	= parentFwd * DotProduct3D( dispStartToTarget, parentFwd );
				float lengthBCn									= dispStartToTargetProjectedOntoParentFwd.GetLength();
				float angleRadians								= acosf( lengthBCn / currentLimb->m_length );
				float angleDegrees								= ConvertRadiansToDegrees( angleRadians );

				// Ensure we calculate the correct angle when projecting onto parent
				float dotResult_parentFwd = DotProduct3D( currentFwd, parentFwd );
				if ( dotResult_parentFwd < 0 )
				{
					// This makes sure we have the angle between parentFwd and currentFwd not the "opposite" angle outside
					angleDegrees = 180.0f - angleDegrees;
				}

				// Use correct clampingValue based on our "half" of the circle
				// We determine if we need to rotate clockwise "CW" or counterClockwise "CCW" using this dotProduct
				float maxAngle				= 0.0f;
				float DotResult_CW_or_CCW	= DotProduct3D( currentFwd, currentLimb->m_parent->m_leftDir );
				if ( DotResult_CW_or_CCW > 0.0f )
				{
					// We are on top, use the yawConstraint.Max value for clamping
					maxAngle = currentLimb->m_yawConstraints.m_max;
				}
				else
				{
					// We are on bottom, use the yawConstraint.Min value for clamping
					maxAngle = currentLimb->m_yawConstraints.m_min;
				}

				// Apply hinge constraints
				if ( angleDegrees > maxAngle )
				{
					// Check angle is invalid (out of bounds), rotate deltaTheta about arbitrary axis
					float deltaAngle		= angleDegrees - maxAngle;
					Vec3 currentEnd			= currentLimb->m_startPos + ( currentFwd * currentLimb->m_length );
					Vec3 vectorToRotate		= currentEnd - currentLimb->m_startPos;
//					arbitraryAxis.x			= fabsf( arbitraryAxis.x );
//					arbitraryAxis.y			= fabsf( arbitraryAxis.y );
//					arbitraryAxis.z			= fabsf( arbitraryAxis.z );

					vectorToRotate			= RotateVectorAboutArbitraryAxis( vectorToRotate, currentLimb->m_axisOfRotation, deltaAngle );
					currentLimb->m_fwdDir	= vectorToRotate.GetNormalized();
					currentLimb->m_endPos	= currentLimb->GetLimbEndMaxLength_IK();
				}
			}
*/


/*
			//----------------------------------------------------------------------------------------------------------------------
			// Old Version attempted hinge constraints using the angleAxis rotations
			//----------------------------------------------------------------------------------------------------------------------
			// Angle Axis rotations approach for angle clamping
			//----------------------------------------------------------------------------------------------------------------------
			// Convert from world-local-polar space and check if current angle (theta) is valid
			if ( currentLimb->m_parent != nullptr )
			{
				Vec3 parentFwd			= currentLimb->m_parent->m_fwdDir.GetNormalized();
				Vec3 BC					= targetPos - currentLimb->m_startPos;
				BC						= BC.GetNormalized();
				Vec3 currentFwd			= BC;

				BC						*= currentLimb->m_length;
				Vec3 BCn				= parentFwd * DotProduct3D( BC, parentFwd );
				Vec3 p					= BCn + currentLimb->m_startPos;
				Vec3 pc					= ( targetPos - p ).GetNormalized();
				float lengthBCn			= BCn.GetLength();
				float angleRadians		= acosf( lengthBCn / currentLimb->m_length );
				float angleDegrees		= ConvertRadiansToDegrees( angleRadians );

				// Ensure we calculate the correct angle when projecting onto parent
				float dotProductResult_parentFwd = DotProduct3D( currentFwd, parentFwd );
				// Agreement with parent
				if ( dotProductResult_parentFwd < 0 )
				{
					angleDegrees  = 180.0f - angleDegrees;
				}

				// Use correct clampingValue based on our "half" of the circle
				float maxAngle						= 0.0f;
				Vec3 arbitraryAxis					= CrossProduct3D( BC, parentFwd ).GetNormalized();
				//				Vec3 parentLeft						= RotateVectorAboutArbitraryAxis( parentFwd, arbitraryAxis, 90.0f );
				Vec3 parentLeft						= RotateVectorAboutArbitraryAxis( parentFwd, Vec3( 0.0f, 0.0f, 1.0f ), 90.0f );
				parentLeft							= parentLeft.GetNormalized();

				float DotProductResult_topBottom	= DotProduct3D( pc, parentLeft );
				if ( DotProductResult_topBottom > 0.0f )
				{
					// We are on top, use the yawConstraint.Max value for clamping
					maxAngle = currentLimb->m_yawConstraints.m_max;
				}
				else
				{
					// We are on bottom, use the yawConstraint.Min value for clamping
					maxAngle = currentLimb->m_yawConstraints.m_min;
				}

				// Apply cone constraints
				if ( angleDegrees > maxAngle )
				{
					// Check angle is invalid (out of bounds), rotate deltaTheta about arbitrary axis
					float deltaAngle		= angleDegrees - maxAngle;
					Vec3 currentEnd			= currentLimb->m_startPos + (currentFwd * currentLimb->m_length );
					Vec3 vectorToRotate		= currentEnd - currentLimb->m_startPos;
					//					arbitraryAxis.x			= fabsf( arbitraryAxis.x );
					//					arbitraryAxis.y			= fabsf( arbitraryAxis.y );
					//					arbitraryAxis.z			= fabsf( arbitraryAxis.z );

					vectorToRotate			= RotateVectorAboutArbitraryAxis( vectorToRotate, arbitraryAxis, deltaAngle );
					currentLimb->m_fwdDir	= vectorToRotate.GetNormalized();
					currentLimb->m_endPos	= currentLimb->GetLimbEndMaxLength_IK();
				}
			}
*/

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------

/*
			//----------------------------------------------------------------------------------------------------------------------
			// Apply angle constraints in parent's local space
			//----------------------------------------------------------------------------------------------------------------------
			if ( currentLimb->m_parent != nullptr )
			{
				Vec3  parentfwd				= currentLimb->m_parent->m_fwdDir;
				currentLimb->m_startPos		= currentLimb->m_parent->m_endPos;

				Vec3  startToTarget			= targetPos - currentLimb->m_startPos;
				float lengthBToQ			= GetProjectedLength3D( startToTarget, parentfwd );
				Vec3  q						= currentLimb->m_startPos + ( parentfwd * lengthBToQ );

				Vec3 qToTarget				= ( startToTarget - q ).GetNormalized();
				float lengthQToTarget		= qToTarget.GetLength();

				float degrees				= Atan2Degrees( lengthQToTarget, lengthBToQ );
				m_degrees					= degrees;

				if ( !currentLimb->m_yawConstraints.IsOnRange( degrees ) )
				{
					degrees	= GetClamped( degrees, currentLimb->m_yawConstraints.m_min, currentLimb->m_yawConstraints.m_max );
					m_clampedDegrees = degrees;
				}

				Vec2 clampedPos				= Vec2::MakeFromPolarDegrees( degrees, currentLimb->m_length );
				Vec3 tBasis					= ( targetPos - q ).GetNormalized();
				// Step in parentFwdDir
				Vec3 newX = ( parentfwd * clampedPos.x );
				// Step in tBasis
				Vec3 newY = ( tBasis * clampedPos.y );

				Vec3 newEndPos				= currentLimb->m_startPos + newX + newY;
				currentLimb->m_fwdDir		= ( newEndPos - currentLimb->m_startPos ).GetNormalized();
				currentLimb->m_endPos		= currentLimb->m_startPos + ( currentLimb->m_fwdDir * currentLimb->m_length );
			}
*/

//			else
//			{
//				// Logic for parent limb (first in hierarchy, does NOT have a parent)
//				currentLimb->m_startPos = m_position;
//				currentLimb->m_endPos = currentLimb->GetLimbEndMaxLength_IK();
//			}

			//----------------------------------------------------------------------------------------------------------------------
			// Get parent's direction
			// Dot product with parent's dir
			// Check if current dir is valid 
				// True: do nothing
				// False: clamp dir

//			if ( currentLimb->m_parent != nullptr )
//			{
//				// Check if current yaw angle is on yawConstraints range
//				// True: set the start, end, dir, and orientation values
//				// False: clamp the rotations, then set the start, end, dir, and orientation values
//
//				// Logic for children limbs (has a parent)
//				currentLimb->m_startPos = currentLimb->m_parent->GetLimbEndMaxLength_IK();
//				currentLimb->m_endPos	= currentLimb->GetLimbEndMaxLength_IK();

//				//----------------------------------------------------------------------------------------------------------------------
//				// Convert fwdDir to YPR
//				// Check fwdDir_YPR is on constraint range
//				EulerAngles eulerAngles;
//				eulerAngles = eulerAngles.GetAsEulerAngles_XFwd_YLeft_ZUp( currentLimb->m_fwdDir, currentLimb->m_fwdDir.GetRotatedAboutZDegrees( 90.0f ) );
//
//				float deltaYaw = eulerAngles.m_yawDegrees - currentLimb->m_parent->m_orientation.m_yawDegrees;
//				//----------------------------------------------------------------------------------------------------------------------
//				if ( currentLimb->m_yawConstraints.IsOnRange( deltaYaw ) )
//				{
//
//				}
//				else
//				{
//					float clampedYaw						= GetClamped( deltaYaw, currentLimb->m_yawConstraints.m_min, currentLimb->m_yawConstraints.m_max );
//					currentLimb->m_orientation.m_yawDegrees = currentLimb->m_parent->m_orientation.m_yawDegrees + clampedYaw;
//					currentLimb->m_fwdDir					= currentLimb->m_orientation.GetForwardDir_XFwd_YLeft_ZUp();
//					currentLimb->m_endPos					= currentLimb->GetLimbEndMaxLength_IK();
//				}
//*/
//			}
//			else
//			{
//				// Logic for parent limb (first in hierarchy, does NOT have a parent)
//				currentLimb->m_startPos = m_position;
//				currentLimb->m_endPos	= currentLimb->GetLimbEndMaxLength_IK();

//				// Convert fwdDir to YPR
//				// Check fwdDir_YPR is on constraint range
//				EulerAngles eulerAngles;
//				eulerAngles = eulerAngles.GetAsEulerAngles_XFwd_YLeft_ZUp( currentLimb->m_fwdDir, currentLimb->m_fwdDir.GetRotatedAboutZDegrees( 90.0f ) );
//				//----------------------------------------------------------------------------------------------------------------------
//				if ( currentLimb->m_yawConstraints.IsOnRange( eulerAngles.m_yawDegrees ) )
//				{
//
//				}
//				else
//				{
//					float clampedYaw						= GetClamped( eulerAngles.m_yawDegrees, currentLimb->m_yawConstraints.m_min, currentLimb->m_yawConstraints.m_max );
//					currentLimb->m_orientation.m_yawDegrees = clampedYaw;
//					currentLimb->m_fwdDir					= currentLimb->m_orientation.GetForwardDir_XFwd_YLeft_ZUp();
//					currentLimb->m_endPos					= currentLimb->GetLimbEndMaxLength_IK();
//				}
//*/
//			}
		}
		else if ( currentLimb->m_jointConstraintType == JOINT_CONSTRAINT_TYPE_HINGE_KNEE )
		{			
			//----------------------------------------------------------------------------------------------------------------------
			// Knee Hinge constraints algorithm
			// 1. Solve normally
			// 2. Project onto plane 
			// 3. Check if angle between currentFwd and parentFwd is out of bounds
			// 4. Clamp if necessary
			// 5. Update basis vectors
			//----------------------------------------------------------------------------------------------------------------------

			//----------------------------------------------------------------------------------------------------------------------
			// 1. FABRIK solution with no constraints
			//----------------------------------------------------------------------------------------------------------------------
			if ( currentLimb->m_parent == nullptr )
			{
				// Logic for parent limb (first in hierarchy, does NOT have a parent)
				currentLimb->m_jointPos_LS				= m_position_WS;
				// Flatten currentFwd onto plane where EE's left is the plane normal
				Vec3 newFwdProjectedOntoLeft_EE		= ProjectVectorOntoPlaneNormalized( currentLimb->m_fwdDir, target.m_leftDir );
				// Update currentLimb's endPos with the newProjectedFwd
				currentLimb->m_fwdDir				= newFwdProjectedOntoLeft_EE;
				currentLimb->m_endPos				= currentLimb->GetLimbEnd();
				//----------------------------------------------------------------------------------------------------------------------
				// Update basis vectors
				//----------------------------------------------------------------------------------------------------------------------
				currentLimb->m_leftDir		  = target.m_leftDir;
				currentLimb->m_upDir		  = CrossProduct3D( currentLimb->m_fwdDir, currentLimb->m_leftDir );
				currentLimb->m_upDir.Normalize();
				currentLimb->m_axisOfRotation = target.m_leftDir;

/*
				//----------------------------------------------------------------------------------------------------------------------
				// Compute basis vectors
				//----------------------------------------------------------------------------------------------------------------------
				Vec3  worldUp	= Vec3( 0.0f, 0.0f, 1.0f );
				Vec3  worldLeft	= Vec3( 0.0f, 1.0f, 0.0f );
				float dotResult = DotProduct3D( currentLimb->m_fwdDir, worldUp );
				if ( dotResult < 1.0f )
				{
					currentLimb->m_leftDir	= CrossProduct3D( worldUp, currentLimb->m_fwdDir );
					currentLimb->m_leftDir.Normalize();
					currentLimb->m_upDir	= CrossProduct3D( currentLimb->m_fwdDir, currentLimb->m_leftDir );
					currentLimb->m_upDir.Normalize();
				}
				else
				{
					currentLimb->m_upDir	= CrossProduct3D( currentLimb->m_fwdDir, worldLeft );
					currentLimb->m_upDir.Normalize();
					currentLimb->m_leftDir	= CrossProduct3D( currentLimb->m_upDir, currentLimb->m_fwdDir );
					currentLimb->m_leftDir.Normalize();
				}
*/
			}
			else
			{
				// Logic for children limbs (has a parent)
				currentLimb->m_axisOfRotation	= currentLimb->m_parent->m_leftDir;
				currentLimb->m_jointPos_LS			= currentLimb->m_parent->m_endPos;	
				//----------------------------------------------------------------------------------------------------------------------
				// 2. Project onto plane with rotation axis (parent's leftDir) as the plane's normal 
				//----------------------------------------------------------------------------------------------------------------------
				Vec3 currentFwdProjectedOntoLeftPlane	= ProjectVectorOntoPlaneNormalized( currentLimb->m_fwdDir, currentLimb->m_parent->m_leftDir );
				currentLimb->m_fwdDir					= currentFwdProjectedOntoLeftPlane;
				//----------------------------------------------------------------------------------------------------------------------
				// 3. & 4. Check if angle between currentFwd and parentFwd is out of bounds AND clamp if necessary
				//----------------------------------------------------------------------------------------------------------------------
				// Compute angle between currentFwd and parentFwd
				if ( currentLimb->m_child == nullptr )
				{
					Vec3  refVector			 = currentLimb->m_parent->m_fwdDir;
					float signedAngleDegrees = GetSignedAngleDegreesBetweenVectors( refVector, currentFwdProjectedOntoLeftPlane, currentLimb->m_axisOfRotation );
					// Check if angle is within bounds
					if ( signedAngleDegrees > currentLimb->m_yawConstraints_LS.m_max )
					{
						currentLimb->m_fwdDir = RotateVectorAboutArbitraryAxis( refVector, currentLimb->m_axisOfRotation, currentLimb->m_yawConstraints_LS.m_max );
					}
					else if ( signedAngleDegrees < currentLimb->m_yawConstraints_LS.m_min )
					{
						currentLimb->m_fwdDir = RotateVectorAboutArbitraryAxis( refVector, currentLimb->m_axisOfRotation, currentLimb->m_yawConstraints_LS.m_min );
					}
				}
				else	// If this joint's child is NOT null (it exists)
				{
					float signedAngleDegrees = GetSignedAngleDegreesBetweenVectors( currentLimb->m_parent->m_fwdDir, currentFwdProjectedOntoLeftPlane, currentLimb->m_axisOfRotation );
					// Check if angle is within bounds
					if ( signedAngleDegrees > currentLimb->m_yawConstraints_LS.m_max )
					{
						currentLimb->m_fwdDir = RotateVectorAboutArbitraryAxis( currentLimb->m_parent->m_fwdDir, currentLimb->m_axisOfRotation, currentLimb->m_yawConstraints_LS.m_max );
					}
					else if ( signedAngleDegrees < currentLimb->m_yawConstraints_LS.m_min )
					{
						currentLimb->m_fwdDir = RotateVectorAboutArbitraryAxis( currentLimb->m_parent->m_fwdDir, currentLimb->m_axisOfRotation, currentLimb->m_yawConstraints_LS.m_min );
					}
				}
				//----------------------------------------------------------------------------------------------------------------------
				// 5. Update basis vectors
				//----------------------------------------------------------------------------------------------------------------------
				currentLimb->m_endPos	= currentLimb->GetLimbEnd();
				currentLimb->m_leftDir  = currentLimb->m_parent->m_leftDir;
				currentLimb->m_upDir	= CrossProduct3D( currentLimb->m_fwdDir, currentLimb->m_leftDir );
				currentLimb->m_upDir.Normalize();
			}			
		}
		else if ( currentLimb->m_jointConstraintType == JOINT_CONSTRAINT_TYPE_EULER )
		{
			// Debug mode
			if ( m_isSingleStep_Debug )
			{
				// Early out checks for single step
				if ( !currentLimb->m_solveSingleStep_Backwards )
				{
					continue;
				}
				if ( m_breakFABRIK )
				{
					break;
				}

				//----------------------------------------------------------------------------------------------------------------------
				// 1. Solve FABRIK_Backward (No Squeeze or Stretch)
				//----------------------------------------------------------------------------------------------------------------------				
				// Logic for only child
				if ( currentLimb->m_parent == nullptr && currentLimb->m_child == nullptr )
				{
					OnlyChild_Backwards( currentLimb, target );
				}
				// Logic for first child (no parent)
				else if ( parentLimb == nullptr )
				{
					FirstChild_Backwards( currentLimb, target );
				}
				// Logic for final child (End Effector)
				else if ( currentLimb->m_child == nullptr )
				{
					FinalChild_Backwards( currentLimb, target );
				}
				else	// Logic for limbs with children and parents
				{
					HasChildAndParents_Backwards( currentLimb );
				}
				currentLimb->ToggleSingleStep_Backwards();
				currentLimb->ComputeJ_Left_K_UpCrossProducts( target );

				//----------------------------------------------------------------------------------------------------------------------
				// 2. Constrain in Euler (YPR)
				//----------------------------------------------------------------------------------------------------------------------
				ConstrainYPR_Backwards( currentLimb, target );
				// Re-update endPos based on new IJK
				currentLimb->m_endPos = currentLimb->GetLimbEnd();
			}
			else    // Normal mode (Updating every frame)
			{
				//----------------------------------------------------------------------------------------------------------------------
				// 1. Solve FABRIK_Backward (No Squeeze or Stretch)
				//----------------------------------------------------------------------------------------------------------------------
				// Logic for only child (no child OR parent)
				if ( parentLimb == nullptr && currentLimb->m_child == nullptr )
				{
					OnlyChild_Backwards( currentLimb, target );
				}
				// Logic for first child (no parent)
				else if ( parentLimb == nullptr )
				{
					FirstChild_Backwards( currentLimb, target );
				}
				// Logic for final child (End Effector)
				else if ( currentLimb->m_child == nullptr )
				{
					SolveTwoBoneIK_TriangulationMethod( target );
					FinalChild_Backwards( currentLimb, target );
				}
				else	// Logic for limbs with children and parents
				{
					HasChildAndParents_Backwards( currentLimb );
				}
				currentLimb->ComputeJ_Left_K_UpCrossProducts( target );

				//----------------------------------------------------------------------------------------------------------------------
				// 2. Constrain in Euler (YPR)
				//----------------------------------------------------------------------------------------------------------------------
				ConstrainYPR_Backwards( currentLimb, target );
				// Re-update endPos based on new IJK
				currentLimb->m_endPos = currentLimb->GetLimbEnd();
			}
		}		
	}
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::OnlyChild_Backwards( IK_Joint3D* const currentLimb, Target target )
{
	currentLimb->m_jointPos_LS = m_position_WS;
	currentLimb->m_leftDir  = target.m_leftDir;
	currentLimb->m_upDir	= target.m_upDir;
	currentLimb->m_fwdDir	= ( target.m_currentPos - currentLimb->m_jointPos_LS ).GetNormalized();
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::FirstChild_Backwards( IK_Joint3D* const currentLimb, Target target )
{
	SolveTwoBoneIK_TriangulationMethod( target );
	currentLimb->m_jointPos_LS = m_position_WS;
	currentLimb->UpdateFwdFromEndStart();
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::FinalChild_Backwards( IK_Joint3D* const currentLimb, Target target )
{
//	currentLimb->InheritTargetIJK( target );
	currentLimb->m_jointPos_LS = currentLimb->m_parent->m_endPos;
	currentLimb->m_fwdDir	= ( target.m_currentPos - currentLimb->m_jointPos_LS ).GetNormalized();
	currentLimb->m_endPos	= currentLimb->GetLimbEnd();
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::HasChildAndParents_Backwards( IK_Joint3D* const currentLimb )
{
	currentLimb->InheritParentIJK();
	currentLimb->m_jointPos_LS = currentLimb->m_parent->m_endPos;
	currentLimb->UpdateFwdFromEndStart();
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::ConstrainYPR_Backwards( IK_Joint3D* const currentLimb, Target target )
{
	if ( currentLimb->m_yawConstraints_LS.m_min		== -180.0f &&
		 currentLimb->m_yawConstraints_LS.m_max		==  180.0f &&
		 currentLimb->m_pitchConstraints_LS.m_min	== -180.0f &&
		 currentLimb->m_pitchConstraints_LS.m_max	==  180.0f &&
		 currentLimb->m_rollConstraints_LS.m_min	== -180.0f &&
		 currentLimb->m_rollConstraints_LS.m_max	==  180.0f 
		)
	{
		return;
	}

	//----------------------------------------------------------------------------------------------------------------------
	// 2. Constrain in Euler (YPR)
	//----------------------------------------------------------------------------------------------------------------------
	IK_Joint3D* parentLimb = currentLimb->m_parent;

	// Get Euler (Yaw, Pitch, Roll ) from IJK
	currentLimb->m_eulerAngles_LS = GetEulerFromFwdDir( currentLimb, currentLimb->m_fwdDir );
	if ( parentLimb == nullptr )
	{
		currentLimb->ClampYPR();
		// Convert Euler back to IJK
		currentLimb->m_eulerAngles_LS.GetAsVectors_XFwd_YLeft_ZUp( currentLimb->m_fwdDir, currentLimb->m_leftDir, currentLimb->m_upDir );
	}
	else   // Logic for children
	{
		//----------------------------------------------------------------------------------------------------------------------
		// Transform targetPos into currentJoint localSpace to solve relative angle 
		Mat44 parentMatrix;
		parentMatrix.SetIJKT3D( parentLimb->m_fwdDir, parentLimb->m_leftDir, parentLimb->m_upDir, parentLimb->m_endPos );			// 'LS' means "local space"
		Mat44 parentInvMatrix			= parentMatrix.GetOrthoNormalInverse();
		Vec3 dirToTarget_LS				= parentInvMatrix.TransformPosition3D( target.m_currentPos );								
		dirToTarget_LS.Normalize();
		currentLimb->m_eulerAngles_LS	= GetEulerFromFwdDir( currentLimb, dirToTarget_LS );
		//----------------------------------------------------------------------------------------------------------------------
		// Clamp if necessary (relative to parent)
		currentLimb->ClampYPR();
		//----------------------------------------------------------------------------------------------------------------------
		// Convert back from parent space to world space
		Mat44 localToWorldMatrix		= parentMatrix;
		Mat44 localMatrix				= currentLimb->m_eulerAngles_LS.GetAsMatrix_XFwd_YLeft_ZUp();
		localToWorldMatrix.Append( localMatrix );
		currentLimb->m_eulerAngles_LS	= GetEulerFromFwdDir( currentLimb, localToWorldMatrix.GetIBasis3D() );
		currentLimb->m_fwdDir			= localToWorldMatrix.GetIBasis3D();
		currentLimb->m_leftDir			= localToWorldMatrix.GetJBasis3D();
		currentLimb->m_upDir			= localToWorldMatrix.GetKBasis3D();
	}
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::SolveTwoBoneIK_TriangulationMethod( Target target )
{
	//----------------------------------------------------------------------------------------------------------------------
	// Bend the entire chain more using the "Triangulation Method"
	//----------------------------------------------------------------------------------------------------------------------
	if ( m_creatureOwner != nullptr )
	{
		if ( m_jointList.size() == 2 )
		{
			ComputeBendAngle_Cos_Sine( m_firstJoint, m_finalJoint, target, m_position_WS );	
			// Go through Backwards pass AGAIN to update all limbs and constrain angles
//			FABRIK_Backward( target );
		}
		else if ( m_jointList.size() == 3 )
		{
			ComputeBendAngle_Cos_Sine( m_finalJoint->m_parent, m_finalJoint, target, m_finalJoint->m_parent->m_parent->m_endPos );
			// Go through Backwards pass AGAIN to update all limbs and constrain angles
			//			FABRIK_Backward( target );
		}
		else if ( m_jointList.size() > 3 )
		{
//			ComputeBendAngle_Cos_Sine( m_finalLimb->m_parent, m_finalLimb, target, m_finalLimb->m_parent->m_parent->m_endPos );	
			// Go through Backwards pass AGAIN to update all limbs and constrain angles
//			FABRIK_Backward( target );
		}
	}
}


//----------------------------------------------------------------------------------------------------------------------
// limbA is usually the "finalLimb's parent"
// limbB is usually the "finalLimb"
// LimbStartPos refers to limbA's "origin" position (E.g., limbA's parent endPos OR IK_Chain's position if limbA is the "firstLimb")
//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::ComputeBendAngle_Cos_Sine( IK_Joint3D* const limbA, IK_Joint3D* const limbB, Target target, Vec3 const& limbStartPos )
{
	//----------------------------------------------------------------------------------------------------------------------
	// Compute angle to "bendMore" using the Law of Cos and Sine
	//----------------------------------------------------------------------------------------------------------------------
	// Law of Cosine
	float a								= limbA->m_child->m_distToChild;
	float b								= limbA->m_distToChild;
	float d3							= ( target.m_currentPos - limbStartPos ).GetLength();
	float tempResult					= ( ( d3 * d3 ) - ( a * a ) - ( b * b ) ) / ( -2 * a * b );
	tempResult							= GetClamped( tempResult, -1.0f, 1.0f );
	float obtuseAngle					= acosf( tempResult );
	obtuseAngle							= ConvertRadiansToDegrees( obtuseAngle );
	// Law of Sine
	obtuseAngle							= SinDegrees( obtuseAngle ); 
	float numerator						= ( a * obtuseAngle );
	float denominator					= d3;
	if ( numerator == 0.0f && denominator == 0.0f )
	{
		// Handle edge case to avoid "nans" when ( 0.0f / 0.0f )
		tempResult						= 0.0f; 
	}
	else
	{
		tempResult						= numerator / denominator; 
	}
	float parentAngleToBendMore			= asinf( tempResult );
	parentAngleToBendMore				= ConvertRadiansToDegrees( parentAngleToBendMore );
	parentAngleToBendMore			   *= -1.0f;
	//----------------------------------------------------------------------------------------------------------------------
	// #ToDo: Make sure invalid rotation axis are not computed by requiring a pole vector be used
	//		  or figure out a way to compute a valid rotation axis that will allow convergence for constrained FABRIK 
	Vec3 dirStartToTarget				= ( limbB->m_endPos - limbStartPos ).GetNormalized();
	Vec3 rotationAxis					= CrossProduct3D( limbA->m_fwdDir, limbB->m_fwdDir );
	rotationAxis.Normalize();
	if ( limbA->m_poleVector != Vec3::ZERO )
	{
		Vec3 dirToPoleVector			= ( limbA->m_poleVector - limbA->m_jointPos_LS ).GetNormalized(); 
		rotationAxis					= CrossProduct3D( dirToPoleVector, dirStartToTarget );
		limbA->m_leftDir			= rotationAxis.GetNormalized();
	}
	if ( rotationAxis == Vec3::ZERO )
	{
		rotationAxis = m_creatureOwner->m_root->m_leftDir;
	}
	m_debugVector = rotationAxis;
	limbA->m_fwdDir				= RotateVectorAboutArbitraryAxis( dirStartToTarget, rotationAxis, parentAngleToBendMore );
	// Set direction and orientation towards targetPos
	limbA->m_jointPos_LS			= limbStartPos;
	limbA->m_endPos				= limbA->m_jointPos_LS + ( limbA->m_fwdDir * limbA->m_distToChild );
	limbA->ComputeJ_Left_K_UpCrossProducts( target );
	limbA->m_axisOfRotation		= limbA->m_leftDir;	
}

//----------------------------------------------------------------------------------------------------------------------
bool IK_Chain3D::CanMove()
{
	bool canMove = false;
	// Cannot move, is locked.					// Stay in the same place
	if ( m_anchorState == ANCHOR_STATE_LOCKED )
	{
		canMove = false;
	}
	// Is moving, cannot lock.					// In the process of moving, don't do anything
	else if ( m_anchorState == ANCHOR_STATE_MOVING )
	{
		canMove = true;
	}
	// Is not moving, can lock, can also move,	// Limb can be locked or moved
	else if ( m_anchorState == ANCHOR_STATE_FREE )
	{
		canMove = true;
	}
	return canMove;
}


//----------------------------------------------------------------------------------------------------------------------
// RefSkeleton is the skeleton to compare against
//----------------------------------------------------------------------------------------------------------------------
bool IK_Chain3D::TryUnlockAndToggleAnchor( IK_Chain3D* const refSkeleton, bool didRayImpact, Vec3 const& updatedRayImpactPos )
{
	// Cannot move, is locked.					// Stay in the same place
	if ( IsAnchor_Locked() )
	{
		// Clamp my body to stay in the same place until the other limb is finished moving and this limb unlocks
		// Then set a new foot placement and move

		// Check if other limb can be anchored and my limb can be unlocked
		if ( refSkeleton->IsAnchor_Free() || refSkeleton->IsAnchor_Locked() )
		{
			SetAnchor_Moving();
			refSkeleton->SetAnchor_Locked();
			return true;
		}
		else
		{
			if ( didRayImpact )
			{
				m_target.m_goalPos = updatedRayImpactPos;
			}
			return false;
		}
	}
	// Is moving, cannot lock.					// In the process of moving, don't do anything
	if ( IsAnchor_Moving() )
	{
		return true;
	}
	// Is not moving, can lock, can also move,	// Limb can be locked or moved
	if ( IsAnchor_Free() )
	{
		if ( didRayImpact )
		{
			m_target.m_goalPos = updatedRayImpactPos;
		}
		SetAnchor_Moving();
		refSkeleton->SetAnchor_Locked();
		return true;
	}
	return false;
}


//----------------------------------------------------------------------------------------------------------------------
bool IK_Chain3D::UpdateDistEeToTarget_ALSO_CHECK_IfDistChangedSinceLastFrame( Target target )
{
	// Update distEeToTarget after each solve
	// 	Vec3 eePos_WS		= m_finalJoint->GetMatrix_ModelToWorld().GetTranslation3D();
	// 	Vec3 dispEeToTarget	= target.m_currentPos - eePos_WS;
	// 	m_distEeToTarget	= dispEeToTarget.GetLength();

	float newDist = GetDistEeToTarget( target );
	// Check if distance has changed between each solve
	bool distHasChanged = true;
	if ( CompareIfFloatsAreEqual( newDist, m_distEeToTarget, 0.000000001f ) )
	{
		distHasChanged = false;
	}
	m_distEeToTarget	= newDist;
	return distHasChanged;
}


//----------------------------------------------------------------------------------------------------------------------
bool IK_Chain3D::IsAnyJointBentToMaxConstraints()
{
	for ( int i = 0; i < m_jointList.size(); i++ )
	{
		IK_Joint3D* currentJoint = m_jointList[i];
		// Yaw
		if  ( currentJoint->m_eulerAngles_LS.m_yawDegrees >= currentJoint->m_yawConstraints_LS.m_max ||
			  currentJoint->m_eulerAngles_LS.m_yawDegrees <= currentJoint->m_yawConstraints_LS.m_min	
			)
		{
			// Don't consider this joint as clamped if it has no degrees of freedom for yaw
			if ( currentJoint->m_yawConstraints_LS.m_min != currentJoint->m_yawConstraints_LS.m_max )
			{
				return true;
			}
		}
		// Pitch
		if  ( currentJoint->m_eulerAngles_LS.m_pitchDegrees >= currentJoint->m_pitchConstraints_LS.m_max ||
			  currentJoint->m_eulerAngles_LS.m_pitchDegrees	<= currentJoint->m_pitchConstraints_LS.m_min	
			)
		{
			// Don't consider this joint as clamped if it has no degrees of freedom for pitch 
			if ( currentJoint->m_pitchConstraints_LS.m_min != currentJoint->m_pitchConstraints_LS.m_max )
			{
				return true;
			}
		}			
	}
	return false;
}


//----------------------------------------------------------------------------------------------------------------------
float IK_Chain3D::GetMaxLengthOfSkeleton()
{
	float totalLength = 0.0f;
	for ( int i = 0; i < m_jointList.size(); i++ )
	{
		IK_Joint3D const* currentSegment = m_jointList[ i ];
		totalLength += currentSegment->m_distToChild;
	}
	return totalLength;
}


//----------------------------------------------------------------------------------------------------------------------
// The ref vectors parameters belong to the reference orientation
// Does NOT belong to "us"
//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::UpdateTargetOrientationToRef( Vec3 const& fwd, Vec3 const& left, Vec3 const& up )
{
	m_target.m_fwdDir	= fwd;
	m_target.m_leftDir	= left;
	m_target.m_upDir	= up;
}


//----------------------------------------------------------------------------------------------------------------------
// To reduce complexity and ambiguity, this function eliminates roll when computing Euler from ONLY FwdDir
//----------------------------------------------------------------------------------------------------------------------
EulerAngles IK_Chain3D::GetEulerFromFwdDir( IK_Joint3D* curJoint, Vec3 const& fwdDir )
{
	EulerAngles newEulerAngles = EulerAngles();
	float sign = 1.0f;
	if ( fwdDir.x < 0.0f )
	{
		// Preserve the negative sign since we lose it 
		// by squaring the length when calculating "projectedFwdOnPlaneXY"
		sign = -1.0f;
	}
	float projectedFwdOnPlaneXY	= sqrtf( ( fwdDir.x * fwdDir.x ) + ( fwdDir.y * fwdDir.y ) );
	bool  isFacingWorldZ		= CompareIfFloatsAreEqual( projectedFwdOnPlaneXY, 0.0f, 0.0001f );
	if ( isFacingWorldZ )
	{
		// Handle Gimble lock edge case
		// This issue occurs when our "fwdDir" is facing world -Z or Z+
		// Since Y is around the "world" Z axis and not the "kBasis", this issue occurs.
		// When this happens, we need to compute EulerAngles from our dirVectors differently
		float yawDegrees	= 0.0f;
		float pitchDegrees	= Atan2Degrees( -fwdDir.z, projectedFwdOnPlaneXY );
		float rollDegrees	= 0.0f;
		newEulerAngles		= EulerAngles( yawDegrees, pitchDegrees, rollDegrees );
	}
	else
	{
		//----------------------------------------------------------------------------------------------------------------------
		// Handle ambiguity edge case
		// One vector can be represented with multiple EulerAngles
		//----------------------------------------------------------------------------------------------------------------------
// 		// Prefer "yaw" over pitch
// 		float yawDegrees_y		= Atan2Degrees(  fwdDir.y, fwdDir.x );
// 		float pitchDegrees_y	= Atan2Degrees( -fwdDir.z, projectedFwdOnPlaneXY );
// 		float rollDegrees_y		= 0.0f;
// 
// 		//----------------------------------------------------------------------------------------------------------------------
// 		// Handle ambiguity edge case
// 		// One vector can be represented with multiple EulerAngles
// 		//----------------------------------------------------------------------------------------------------------------------
// 		if ( sign < 0.0f )
// 		{
// 			// Prefer pitch over yaw
// 			if ( curJoint->m_pitchConstraints_LS.m_max >=  90.0f ||
// 				 curJoint->m_pitchConstraints_LS.m_min <= -90.0f )
// 			{
// 				// Re-compute pitch calculations if pitch is preferred over yaw
// 				projectedFwdOnPlaneXY *= sign;
// 				pitchDegrees_y		   = Atan2Degrees( -fwdDir.z, projectedFwdOnPlaneXY );
// 				if ( yawDegrees_y >= 90.0f )
// 				{
// 					yawDegrees_y -= 180.0f;
// 				}
// 				else if ( yawDegrees_y <= -90.0f )
// 				{
// 					yawDegrees_y += 180.0f;
// 				}
// 			}
// 		}
// 		newEulerAngles = EulerAngles( yawDegrees_y, pitchDegrees_y, rollDegrees_y );

		//----------------------------------------------------------------------------------------------------------------------
		// 1. Compute both cases of YPR (yawPreferred & pitchPreferred)
		// 2. Compute and choose "nearest" euler
		// 3. Compare both cases of euler and choose most similar to prevEuler solution
		//----------------------------------------------------------------------------------------------------------------------
		// 1a. Compute Yaw preferred solution
		float yawDegrees_y		= Atan2Degrees(  fwdDir.y, fwdDir.x );
		float pitchDegrees_y	= Atan2Degrees( -fwdDir.z, projectedFwdOnPlaneXY );
		// 1b. Compute pitch preferred solution
		float yawDegrees_p		= yawDegrees_y;
		float pitchDegrees_p	= pitchDegrees_y;
 		if ( sign < 0.0f )
 		{
			// Re-compute pitch calculations
			projectedFwdOnPlaneXY  *= -1.0f;
			yawDegrees_p			= yawDegrees_y;
			pitchDegrees_p			= Atan2Degrees( -fwdDir.z, projectedFwdOnPlaneXY );
			if ( yawDegrees_y >= 90.0f )
			{
				yawDegrees_p -= 180.0f;
			}
			else if ( yawDegrees_y <= -90.0f )
			{
				yawDegrees_p += 180.0f;
			}
		}
//  		// 2. Compute and choose "nearest" euler (Euler comparison)
// 		//	  "_y" means "yawPreferred" 
// 		//	  "_p" means "pitchPreferred"
// 		EulerAngles yawPreferredEuler	= EulerAngles( yawDegrees_y, pitchDegrees_y, 0.0f );
//  		EulerAngles pitchPreferredEuler = EulerAngles( yawDegrees_p, pitchDegrees_p, 0.0f );
//  		float absYawDif_y	= fabsf( m_prevEuler_LastFrame.m_yawDegrees	  ) - fabsf( yawPreferredEuler.m_yawDegrees		);	 
//  		float absPitchDif_y = fabsf( m_prevEuler_LastFrame.m_pitchDegrees ) - fabsf( yawPreferredEuler.m_pitchDegrees	);	 
//  		float absYawPitch_p	= fabsf( m_prevEuler_LastFrame.m_yawDegrees	  ) - fabsf( pitchPreferredEuler.m_yawDegrees	);   
//  		float absPitchDif_p = fabsf( m_prevEuler_LastFrame.m_pitchDegrees ) - fabsf( pitchPreferredEuler.m_pitchDegrees ); 
//  		float totalDif_y	= fabsf( absYawDif_y )	 + fabsf( absPitchDif_y );
//  		float totalDif_p	= fabsf( absYawPitch_p ) + fabsf( absPitchDif_p );
//  		// 3. Compare both cases of euler and choose most similar to prevEuler solution
//   		if ( totalDif_y <= totalDif_p )
//   		{
//   			// yaw preferred solution is more similar
//   			newEulerAngles			= yawPreferredEuler;
//  			m_prevEuler_LastFrame	= newEulerAngles;
//   		}
//   		else if ( totalDif_y > totalDif_p )
//  		{
//   			// pitch preferred solution is more similar
//   			newEulerAngles			= pitchPreferredEuler;
//  			m_prevEuler_LastFrame	= newEulerAngles;
//   		}
//   		else
//   		{
//  			// This case should never get triggered?
//  			int brainCells = 0;
//   		}


		//----------------------------------------------------------------------------------------------------------------------
 		// 2. Convert YPR to vectors (leftDir AND upDir comparison)
		//----------------------------------------------------------------------------------------------------------------------
 		EulerAngles yawPreferredEuler	= EulerAngles( yawDegrees_y, pitchDegrees_y, 0.0f );
 		EulerAngles pitchPreferredEuler = EulerAngles( yawDegrees_p, pitchDegrees_p, 0.0f );
 		Vec3 fwd_y, left_y, up_y;
 		Vec3 fwd_p, left_p, up_p;
 		  yawPreferredEuler.GetAsVectors_XFwd_YLeft_ZUp( fwd_y, left_y, up_y );
 		pitchPreferredEuler.GetAsVectors_XFwd_YLeft_ZUp( fwd_p, left_p, up_p );
 		// 3. Dot product vectors to prev solution
 		float leftSimilarity_y	= DotProduct3D( curJoint->m_leftDir_lastFrame, left_y );
 		float leftSimilarity_p	= DotProduct3D( curJoint->m_leftDir_lastFrame, left_p );
 		float upSimilarity_y	= DotProduct3D( curJoint->m_upDir_lastFrame,   up_y	);
 		float upSimilarity_p	= DotProduct3D( curJoint->m_upDir_lastFrame,   up_p	);
 		// 4. Return euler most "similar" to prev solution
 		// Yaw is preferred
		if ( leftSimilarity_y >= leftSimilarity_p )
		{
 			if ( upSimilarity_y >= upSimilarity_p )
 			{
 				// yawPreferred solution is more similar to the newLeft and newUp 
 				newEulerAngles					= EulerAngles( yawDegrees_y, pitchDegrees_y, 0.0f );
 				curJoint->m_leftDir_lastFrame	= left_y;
				curJoint->m_upDir_lastFrame		= up_y;
				curJoint->m_euler_LastFrame		= newEulerAngles;
 			}
 			else if ( upSimilarity_y < upSimilarity_p )
 			{
				// pitchPreferred solution is more similar to the newLeft and newUp 
				newEulerAngles					= EulerAngles( yawDegrees_p, pitchDegrees_p, 0.0f );
 				curJoint->m_leftDir_lastFrame	= left_p;
 				curJoint->m_upDir_lastFrame		= up_p;
 				curJoint->m_euler_LastFrame		= newEulerAngles;
 			}
 		}
		// Pitch is preferred
 		else if ( leftSimilarity_y < leftSimilarity_p  )
 		{
			if ( upSimilarity_y >= upSimilarity_p )
			{
				// yawPreferred solution is more similar to the newLeft and newUp 
				newEulerAngles					= EulerAngles( yawDegrees_y, pitchDegrees_y, 0.0f );
				curJoint->m_leftDir_lastFrame	= left_y;
				curJoint->m_upDir_lastFrame		= up_y;
				curJoint->m_euler_LastFrame		= newEulerAngles;
			}
			else if ( upSimilarity_y < upSimilarity_p )
			{
				// pitchPreferred solution is more similar to the newLeft and newUp 
				newEulerAngles					= EulerAngles( yawDegrees_p, pitchDegrees_p, 0.0f );
				curJoint->m_leftDir_lastFrame	= left_p;
				curJoint->m_upDir_lastFrame		= up_p;
				curJoint->m_euler_LastFrame		= newEulerAngles;
			}
 		}
		else
		{
			int brainCells = 0;
		}
	}
	return newEulerAngles;
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::ResetAllJointsEuler()
{
	// Reset all joint euler to (0,0,0)
	for ( int i = 0; i < m_jointList.size(); i++ )
	{
		IK_Joint3D* currentJoint	   = m_jointList[ i ];
		currentJoint->m_eulerAngles_LS = EulerAngles();
	}
}


//----------------------------------------------------------------------------------------------------------------------
float IK_Chain3D::GetDistEeToTarget( Target target )
{
	Vec3 eePos_WS		= m_finalJoint->GetMatrix_ModelToWorld().GetTranslation3D();
	Vec3 dispEeToTarget	= target.m_currentPos - eePos_WS;
	float newDist		= dispEeToTarget.GetLength();
	return newDist;
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::SetAnchor_Locked()
{
	m_anchorState = ANCHOR_STATE_LOCKED;
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::SetAnchor_Moving()
{
	m_anchorState = ANCHOR_STATE_MOVING;
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::SetAnchor_Free()
{
	m_anchorState = ANCHOR_STATE_FREE;
}


//----------------------------------------------------------------------------------------------------------------------
bool IK_Chain3D::IsAnchor_Locked()
{
	if ( m_anchorState == ANCHOR_STATE_LOCKED )
	{
		return true;
	}
	return false;
}


//----------------------------------------------------------------------------------------------------------------------
bool IK_Chain3D::IsAnchor_Moving()
{
	if ( m_anchorState == ANCHOR_STATE_MOVING )
	{
		return true;
	}
	return false;
}


//----------------------------------------------------------------------------------------------------------------------
bool IK_Chain3D::IsAnchor_Free()
{
	if ( m_anchorState == ANCHOR_STATE_FREE )
	{
		return true;
	}
	return false;
}