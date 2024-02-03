#pragma once

struct IntVec3
{
public:
	int x = 0;
	int y = 0;
	int z = 0;

public:
	// Construction / Destruction
	IntVec3();
	~IntVec3();
	IntVec3( IntVec3 const& copyFrom );
	explicit IntVec3( int initialX, int initialY, int initialZ );

	// Accessors (const methods)
	float			GetLength() const;
	int				GetTaxicabLength() const;
	int				GetLengthSquared() const;
	float			GetOrientationRadians() const;
	float			GetOrientationDegrees() const;
	IntVec3 const	GetRotated90Degrees() const;
	IntVec3 const	GetRotatedMinus90Degrees() const;

	// Mutators (non-const methods)
	void		Rotate90Degrees();
	void		RotateMinus90Degrees();

	// Operators (self mutating / non-const)
	void			operator=( IntVec3 const& copyFrom );
	void			operator-=( const IntVec3& vecToSubtract );
	const IntVec3	operator-( const IntVec3& vecToSubtract ) const;
	const IntVec3	operator+( const IntVec3& vecToAdd ) const;

	bool operator==( const IntVec3& compare ) const;
	bool operator!=( const IntVec3& compare ) const;
	void operator+=( IntVec3& vecToAdd );				// intVec3 += intVec3

	void SetFromText( char const* text );

	static IntVec3 ZERO;
	static IntVec3 ONE;
	static IntVec3 NEGATIVE_ONE;

private:

};