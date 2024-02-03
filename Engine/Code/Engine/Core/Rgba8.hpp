#pragma once

//----------------------------------------------------------------------------------------------------------------------
struct Rgba8
{
public:
	unsigned char r = 255;
	unsigned char g = 255;
	unsigned char b = 255;
	unsigned char a = 255;

	Rgba8(); 

	explicit Rgba8(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha = 255);

	static Rgba8 const RED;
	static Rgba8 const DARK_RED;
	static Rgba8 const DARKER_RED;
	static Rgba8 const ORANGE;
	static Rgba8 const SUNSET_ORANGE;
	static Rgba8 const GREEN;
	static Rgba8 const DARK_GREEN;
	static Rgba8 const DARKER_GREEN;
	static Rgba8 const BLUE;
	static Rgba8 const SKYBLUE;
	static Rgba8 const LIGHTBLUE;
	static Rgba8 const DARK_BLUE;
	static Rgba8 const WHITE;
	static Rgba8 const TRANSLUCENT_WHITE;
	static Rgba8 const TRANSLUCENT_BLACK;
	static Rgba8 const BLACK;
	static Rgba8 const LIGHTBLACK;
	static Rgba8 const GRAY;
	static Rgba8 const DARK_GRAY;
	static Rgba8 const DARKER_GRAY;
	static Rgba8 const BROWN;
	static Rgba8 const YELLOW;
	static Rgba8 const DARK_YELLOW;
	static Rgba8 const MAGENTA;
	static Rgba8 const CYAN;
	static Rgba8 const DARK_CYAN;
	static Rgba8 const PURPLE;
	static Rgba8 const INDIGO;
	static Rgba8 const PINK;

	void SetFromText( char const* text );
	void GetAsFloat( float* colorAsFloat ) const;

	bool operator==( const Rgba8& compare ) const;
	bool operator!=( const Rgba8& compare ) const;

private:
	
};