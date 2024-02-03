#pragma once
#include <vector>

//----------------------------------------------------------------------------------------------------------------------
struct IntVec2;

//----------------------------------------------------------------------------------------------------------------------
class HeatMaps
{
public:
	HeatMaps( IntVec2 const& dimensions );
	~HeatMaps();

	// #ToDo write this function properly
	void SetAllValues();

	// #ToDo add simple methods to get, set, and add heat values to any tileCoorrds in the heat map;

private:
	std::vector<float> m_values;
};