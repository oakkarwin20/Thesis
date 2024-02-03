#pragma once

#include <vector>

//----------------------------------------------------------------------------------------------------------------------
template< typename T_TypeOfThingPointedTo >
void ClearAndDeleteEverything( std::vector< T_TypeOfThingPointedTo* >& myVector )
{
	for ( int index = 0; index < int( myVector.size() ); index++ )
	{
		delete myVector[index];
	}

	myVector.clear();
}