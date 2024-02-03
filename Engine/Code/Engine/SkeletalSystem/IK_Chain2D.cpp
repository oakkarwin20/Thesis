
#include "Engine/SkeletalSystem/IK_Chain2D.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/MathUtils.hpp"

//----------------------------------------------------------------------------------------------------------------------
IK_Chain2D::IK_Chain2D( Vec2 position )
	: m_position ( position )
{
}

//----------------------------------------------------------------------------------------------------------------------
IK_Chain2D::~IK_Chain2D()
{
}

//----------------------------------------------------------------------------------------------------------------------
void IK_Chain2D::Startup()
{
}

//----------------------------------------------------------------------------------------------------------------------
void IK_Chain2D::Shutdown()
{
}

//----------------------------------------------------------------------------------------------------------------------
void IK_Chain2D::Update()
{
}

//----------------------------------------------------------------------------------------------------------------------
void IK_Chain2D::Render( std::vector<Vertex_PCU>& verts ) const
{
	//----------------------------------------------------------------------------------------------------------------------
	// Render limbs
	//----------------------------------------------------------------------------------------------------------------------
	for ( int limbIndex = 0; limbIndex < m_limbList.size(); limbIndex++ )
	{
		IK_Joint2D* currentLimb	= m_limbList[limbIndex];
		Vec2 limbStart		= currentLimb->m_jointPosition_LS;
		float jointRadius	= currentLimb->m_thickness;
		Vec2 limbEnd		= currentLimb->GetLimbEndXY_IK();
		AddVertsForCapsule2D( verts, limbStart, limbEnd, jointRadius, Rgba8::CYAN );
	}

	for ( int limbIndex = 0; limbIndex < m_limbList.size(); limbIndex++ )
	{
		IK_Joint2D* currentLimb	= m_limbList[limbIndex];
		float jointRadius	= currentLimb->m_thickness + 1.0f;
		Vec2 limbEnd		= currentLimb->GetLimbEndXY_IK();
		AddVertsForDisc2D( verts, limbEnd, jointRadius, Rgba8::DARK_RED );
	}
}

//----------------------------------------------------------------------------------------------------------------------
void IK_Chain2D::CreateNewLimb( float length, float orientation )
{
	IK_Joint2D* newLimb = new IK_Joint2D( Vec2::ZERO, length, 1.0f, orientation );
	if ( m_finalJoint != nullptr )
	{
		// If a "final limb" exists
		newLimb->m_jointPosition_LS = m_finalJoint->GetLimbEndXY_IK();		// Set newLimb's starting position at the end of finalLimb
		newLimb->m_parent   = m_finalJoint;							// Set newLimb's parent pointer to finalLimb
	}
	else
	{
		// if there is NO "final limb"
		newLimb->m_jointPosition_LS = m_position;
	}

	// Update finalLimb to the "newLimb" just created
	m_limbList.push_back( newLimb );
	m_finalJoint = newLimb;
}

//----------------------------------------------------------------------------------------------------------------------
void IK_Chain2D::DragLimb( Vec2 targetPos )
{
	m_finalJoint->DragLimb( targetPos );
}

//----------------------------------------------------------------------------------------------------------------------
void IK_Chain2D::ReachLimb( Vec2 targetPos )
{
	//----------------------------------------------------------------------------------------------------------------------
	// FABRIK (Basically)
	//----------------------------------------------------------------------------------------------------------------------
	// IK pass
	DragLimb( targetPos );					// Sets finalLimb's endPos at targetPos then climbs up hierarchy chain (parents, grand-parents, etc) and sets their endPos at currentLimb's startPos accordingly
	// FK pass
	SetChildrenStartPosAtParentsEndXY();	// 
}

//----------------------------------------------------------------------------------------------------------------------
void IK_Chain2D::SetChildrenStartPosAtParentsEndXY()
{
	for ( int i = 0; i < m_limbList.size(); i++ )
	{
		IK_Joint2D* currentLimb = m_limbList[i];
		if ( currentLimb->m_parent != nullptr )
		{
			currentLimb->m_jointPosition_LS = currentLimb->m_parent->GetLimbEndXY_IK();
		}
		else
		{
			currentLimb->m_jointPosition_LS = m_position;
		}
	}
}

