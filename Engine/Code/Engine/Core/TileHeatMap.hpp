#pragma once

#include "Engine/Math/IntVec2.hpp"

#include <vector>
	
//----------------------------------------------------------------------------------------------------------------------
class TileHeatMap
{
public:
	TileHeatMap( IntVec2 const& dimensions, float initialValue = 0.0f );
	~TileHeatMap();

	float GetHeatValueAtTileCoord( IntVec2 const& tileCoords );
	void SetAllValues( float defaultValue );
	void SetHeatValueAtTileCoord( IntVec2 const& tileCoords, float heatValueToSet );
	void AddHeatValueAtTileCoord( IntVec2 const& tileCoords, float heatValueToAdd );

public:
	std::vector<float> m_heatValues;
	IntVec2 const m_dimensions;
};