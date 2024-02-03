#include "Engine/Core/TileHeatMap.hpp"
#include "Engine/Math/IntVec2.hpp"

//----------------------------------------------------------------------------------------------------------------------
TileHeatMap::TileHeatMap( IntVec2 const& dimensions, float initialValue ) :
	m_dimensions( dimensions )
{
	int numOfTiles = m_dimensions.x * m_dimensions.y;
	m_heatValues.resize( numOfTiles );
	SetAllValues( initialValue );
}

//----------------------------------------------------------------------------------------------------------------------
TileHeatMap::~TileHeatMap()
{
}

//----------------------------------------------------------------------------------------------------------------------
void TileHeatMap::SetAllValues( float defaultValue )
{
	for ( int i = 0; i < m_heatValues.size(); i++ )
	{
		m_heatValues[i] = defaultValue;
	}
}

//----------------------------------------------------------------------------------------------------------------------
float TileHeatMap::GetHeatValueAtTileCoord( IntVec2 const& tileCoords )
{
	int tileIndex = tileCoords.x + ( tileCoords.y * m_dimensions.x );
	return m_heatValues[tileIndex];
}

//----------------------------------------------------------------------------------------------------------------------
void TileHeatMap::SetHeatValueAtTileCoord( IntVec2 const& tileCoords, float heatValueToSet )
{
	int tileIndex = tileCoords.x + ( tileCoords.y * m_dimensions.x );
	m_heatValues[tileIndex] = heatValueToSet;
}

//----------------------------------------------------------------------------------------------------------------------
void TileHeatMap::AddHeatValueAtTileCoord( IntVec2 const& tileCoords, float heatValueToAdd )
{
	int tileIndex = tileCoords.x + ( tileCoords.y * m_dimensions.x );
	m_heatValues[tileIndex] += heatValueToAdd;
}