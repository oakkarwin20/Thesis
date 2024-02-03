 #pragma once

#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include <vector>

//----------------------------------------------------------------------------------------------------------------------
class IK_Chain3D;
struct Target;

//----------------------------------------------------------------------------------------------------------------------
enum JointConstraintType
{
	JOINT_CONSTRAINT_TYPE_DISTANCE,					// Free range of motion (orientation/rotates freely), only bound by length of current limb
	JOINT_CONSTRAINT_TYPE_LINEAR,					// Zero range of motion, shares the same orientation as the parent, can only "extend" and "retract"
	JOINT_CONSTRAINT_TYPE_HINGE,					// 1 degree of motion, constrained in distance
	JOINT_CONSTRAINT_TYPE_HINGE_KNEE,
	JOINT_CONSTRAINT_TYPE_ROTATION,					// Free range of motion
	JOINT_CONSTRAINT_TYPE_BALL_AND_SOCKET,
	JOINT_CONSTRAINT_TYPE_EULER,
	JOINT_CONSTRAINT_TYPE_NUM,
};

/*
* Note: Position and direction data for each joint is stored in local space
* Variable naming conventions:
*	1. 'LS' means 'local space'
*	2. 'WS' means 'world space'
*/

//----------------------------------------------------------------------------------------------------------------------
class IK_Joint3D
{
public:
	IK_Joint3D(		int					jointIndex, 
					Vec3				jointPosition, 
					float				length, 
					IK_Chain3D*			skeletalSystem		= nullptr,
					JointConstraintType jointConstraintType = JOINT_CONSTRAINT_TYPE_DISTANCE, 
					EulerAngles			eulerAngles_LS		= EulerAngles(), 
					FloatRange			yawConstraints		= FloatRange(), 
					FloatRange			pitchConstraints	= FloatRange(), 
					FloatRange			rollConstraints		= FloatRange(), 
					IK_Joint3D*			parent				= nullptr 
				);

	~IK_Joint3D();

	void Update();
	
	// Debug Render Functions
	void RenderIJK( std::vector<Vertex_PCU>& verts, float length ) const;

	// Mutators & Modifiers (Getters & setters)
	void	UpdateFwdFromEndStart();
	void	SetEndPosFromFwdDir();
	Vec3	GetLimbEnd();
	void	SetStartEndPosRelativeToTarget( Vec3 targetPos );
	void	DragLimb3D( Target endEffector );
	void	SetConstraints_YPR(  FloatRange    yawConstraints = FloatRange( -180.0f, 180.0f ), 
								 FloatRange	 pitchConstraints = FloatRange( -180.0f, 180.0f ), 
								 FloatRange	 rollConstraints  = FloatRange( -180.0f, 180.0f ) );
	void	InheritTargetIJK( Target target );
	void	InheritParentIJK();
	void	ComputeJ_Left_K_UpCrossProducts( Target target );
	void	ToggleSingleStep_Forwards();
	void	ToggleSingleStep_Backwards();
	void	ClampYPR();

	// Util FABRIK FORWARDS SOlVERS
	void JointsBeforeEE_Forwards( Target target );
	void finalJoint_Forwards	( Target target );

	//----------------------------------------------------------------------------------------------------------------------
	// Matrix functions to jump between spaces
	//----------------------------------------------------------------------------------------------------------------------
	Mat44 GetMatrix_LocalToParent();
	Mat44 GetMatrix_LocalToModel ( Mat44 localToModelMatrix = Mat44() );
	Mat44 GetMatrix_ModelToWorld ();
	// Matrix Util functions
	Mat44 GetIkChainMatrix_ModelToWorld();
	Vec3  GetTransformedPos_WorldToLocal( Vec3 const& pos );

public:
	//----------------------------------------------------------------------------------------------------------------------
	// Core variables
	//----------------------------------------------------------------------------------------------------------------------
	int					m_jointIndex			= 0;
	IK_Chain3D*			m_ikChain				= nullptr;
	float				m_distToChild			= 0.0f;
	EulerAngles			m_eulerAngles_LS		= EulerAngles();					// Expressed in local space (relative to parent)	
	IK_Joint3D*			m_parent				= nullptr;
	IK_Joint3D*			m_child					= nullptr;
	Vec3				m_poleVector			= Vec3::ZERO;
	
	// Position Variables
	Vec3 m_jointPos_LS	= Vec3::ZERO;
	Vec3 m_endPos		= Vec3::ZERO;		// #ToDo: get rid of this variable?
	
	// Direction Variables
	Vec3 m_fwdDir			= Vec3(  1.0f, 0.0f, 0.0f );	
	Vec3 m_leftDir			= Vec3(  0.0f, 1.0f, 0.0f );
	Vec3 m_upDir			= Vec3(  0.0f, 0.0f, 1.0f );
	Vec3 m_axisOfRotation	= Vec3( -1.0f, 0.0f, 0.0f );

	// Constraints variables
	JointConstraintType	  m_jointConstraintType		= JOINT_CONSTRAINT_TYPE_DISTANCE;
	FloatRange			  m_yawConstraints_LS		= FloatRange( -180.0f, 180.0f );							// Specified in local space, relative to parent
	FloatRange			  m_pitchConstraints_LS		= FloatRange( -180.0f, 180.0f );							// Specified in local space, relative to parent
	FloatRange			  m_rollConstraints_LS		= FloatRange( -180.0f, 180.0f );							// Specified in local space, relative to parent
	Vec3				  m_targetPos				= Vec3::ZERO;		

	//----------------------------------------------------------------------------------------------------------------------
	// Debug single step
	//----------------------------------------------------------------------------------------------------------------------
	bool m_solveSingleStep_Forwards  = false;
	bool m_solveSingleStep_Backwards = false;

	// Basis flipping
	Vec3		m_leftDir_lastFrame		= Vec3::Y_LEFT;
	Vec3		m_upDir_lastFrame		= Vec3::Z_UP;
	EulerAngles m_euler_LastFrame		= EulerAngles();
	EulerAngles m_eulerCloserToTarget	= EulerAngles();

	//----------------------------------------------------------------------------------------------------------------------
	// To delete?
	//----------------------------------------------------------------------------------------------------------------------
	Vec3				m_refVector 			= Vec3( 0.0f, 0.0f, -1.0f );		// Ball and socket 
	bool				m_isSubBase				= false;							// Multi end effectors
};