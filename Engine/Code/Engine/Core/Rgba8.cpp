#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"

//----------------------------------------------------------------------------------------------------------------------
Rgba8 const Rgba8::RED					= Rgba8( 255,   0,   0, 255 );
Rgba8 const Rgba8::DARK_RED				= Rgba8( 160,   0,   0, 255 );
Rgba8 const Rgba8::DARKER_RED			= Rgba8( 139,   0,   0, 255 );
Rgba8 const Rgba8::GREEN				= Rgba8(   0, 255,   0, 255 );
Rgba8 const Rgba8::DARK_GREEN			= Rgba8(   0, 160,   0, 255 );
Rgba8 const Rgba8::DARKER_GREEN			= Rgba8(   0, 100,   0, 255 );
Rgba8 const Rgba8::BLUE					= Rgba8(   0,   0, 255, 255 );
Rgba8 const Rgba8::SKYBLUE				= Rgba8( 135, 206, 235, 255 );
Rgba8 const Rgba8::LIGHTBLUE			= Rgba8(  10, 100, 200, 255 );
Rgba8 const Rgba8::DARK_BLUE			= Rgba8(  10,  50, 100, 255 );
Rgba8 const Rgba8::WHITE				= Rgba8( 255, 255, 255, 255 );
Rgba8 const Rgba8::TRANSLUCENT_WHITE	= Rgba8( 255, 255, 255,  30 );
Rgba8 const Rgba8::TRANSLUCENT_BLACK	= Rgba8(   0,   0,   0, 150 );
Rgba8 const Rgba8::BLACK				= Rgba8(   0,   0,   0, 255 );
Rgba8 const Rgba8::LIGHTBLACK			= Rgba8(  20,  20,  20, 255 );
Rgba8 const Rgba8::ORANGE				= Rgba8( 255, 100,   0, 255 );
Rgba8 const Rgba8::SUNSET_ORANGE		= Rgba8( 255,  87,	50, 255 );
Rgba8 const Rgba8::GRAY					= Rgba8( 191, 191, 191, 255 );
Rgba8 const Rgba8::DARK_GRAY			= Rgba8( 128, 128, 128, 255 );
Rgba8 const Rgba8::DARKER_GRAY			= Rgba8(  99, 102, 106, 255 );
Rgba8 const Rgba8::YELLOW				= Rgba8( 255, 255,   0, 255 );
Rgba8 const Rgba8::DARK_YELLOW			= Rgba8( 255, 173,   1, 255 );
Rgba8 const Rgba8::MAGENTA				= Rgba8( 255,	0, 255, 255 );
Rgba8 const Rgba8::CYAN					= Rgba8( 103, 242, 209, 255 );
Rgba8 const Rgba8::DARK_CYAN			= Rgba8(  10, 140, 100, 255 );
Rgba8 const Rgba8::BROWN				= Rgba8( 150,  75,   0, 255 );
Rgba8 const Rgba8::PURPLE				= Rgba8( 128,   0, 128, 255 );
Rgba8 const Rgba8::INDIGO				= Rgba8(  75,   0, 130, 255 );
Rgba8 const Rgba8::PINK					= Rgba8( 207,  41, 116, 255 );

//----------------------------------------------------------------------------------------------------------------------
Rgba8::Rgba8()
{
	r = 255;
	g = 255;
	b = 255;
	a = 255;
}

//----------------------------------------------------------------------------------------------------------------------
Rgba8::Rgba8(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha)
{
	r = red;
	g = green;
	b = blue;
	a = alpha;
}

//----------------------------------------------------------------------------------------------------------------------
void Rgba8::SetFromText( char const* text )
{
	Strings delimitedText = SplitStringOnDelimiter( text, ',' );
	r = unsigned char( atoi( delimitedText[0].data() ) );
	g = unsigned char( atoi( delimitedText[1].data() ) );
	b = unsigned char( atoi( delimitedText[2].data() ) );
	if ( delimitedText.size() > 3 )
	{
		a = unsigned char( atoi( delimitedText[3].data() ) );
	}

	//	Strings string;
	//	string = SplitStringOnDelimiter( text, ',' );
	//	string.
	//	if ( string.size() == 4 )
	//	{
	//		r = *( string[0].c_str() );
	//		g = *( string[1].c_str() );
	//		b = *( string[2].c_str() );
	//		a = *( string[3].c_str() );
	//	}
	//
	//	r = *( string[0].c_str() );
	//	g = *( string[1].c_str() );
	//	b = *( string[2].c_str() );
}

//----------------------------------------------------------------------------------------------------------------------
void Rgba8::GetAsFloat( float* colorAsFloat ) const
{
	colorAsFloat[0] = RangeMapClamped( static_cast<float>( r ), 0.0f, 255.0f, 0.0f, 1.0f );
	colorAsFloat[1] = RangeMapClamped( static_cast<float>( g ), 0.0f, 255.0f, 0.0f, 1.0f );
	colorAsFloat[2] = RangeMapClamped( static_cast<float>( b ), 0.0f, 255.0f, 0.0f, 1.0f );
	colorAsFloat[3] = RangeMapClamped( static_cast<float>( a ), 0.0f, 255.0f, 0.0f, 1.0f );
}

//----------------------------------------------------------------------------------------------------------------------
bool Rgba8::operator!=( const Rgba8& compare ) const
{
	return ( ( r != compare.r ) && ( g != compare.g ) && ( b != compare.b ) && ( a != compare.a ) );

}

//----------------------------------------------------------------------------------------------------------------------
bool Rgba8::operator==( const Rgba8& compare ) const
{
	return ( (r == compare.r) && (g == compare.g) && (b == compare.b) && (a == compare.a) );
}

