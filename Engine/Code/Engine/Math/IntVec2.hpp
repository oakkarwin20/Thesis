#pragma once

struct IntVec2
{
public:
	int x = 0;
	int y = 0;

public:
	// Construction / Destruction
	IntVec2();
	~IntVec2();
	IntVec2 ( IntVec2 const& copyFrom );
	explicit IntVec2 ( int initialX, int initialY );

	// Accessors (const methods)
	float			GetLength() const;		
	int				GetTaxicabLength() const;
	int				GetLengthSquared() const;
	float			GetOrientationRadians() const;
	float			GetOrientationDegrees() const;
	IntVec2 const	GetRotated90Degrees() const;
	IntVec2 const	GetRotatedMinus90Degrees() const;

	// Mutators (non-const methods)
	void		Rotate90Degrees();
	void		RotateMinus90Degrees();

	// Operators (self mutating / non-const)
	void			operator=( IntVec2 const& copyFrom );
	void			operator-=( const IntVec2& vecToSubtract );		

	// Operators (const)
	bool operator==( const IntVec2& compare ) const;
	bool operator!=( const IntVec2& compare ) const;
	IntVec2 const operator+( const IntVec2& vecToAdd )		const;	// intVec2 + intVec2
	IntVec2 const operator-( const IntVec2& vecToSubtract ) const;	// intVec2 - intVec2


	void SetFromText( char const* text );

	static IntVec2 ZERO;
	static IntVec2 ONE;
	static IntVec2 NEGATIVE_ONE;


private:

};