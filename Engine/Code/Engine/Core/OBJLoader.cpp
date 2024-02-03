#include "Engine/Core/OBJLoader.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

//----------------------------------------------------------------------------------------------------------------------
OBJLoader::OBJLoader()
{
}

//----------------------------------------------------------------------------------------------------------------------
OBJLoader::~OBJLoader()
{
}


//----------------------------------------------------------------------------------------------------------------------
// Tenshi version Start
//----------------------------------------------------------------------------------------------------------------------
void OBJLoader::LoadOBJByFileName( const char* filename, Mat44 transformMat, std::vector<Vertex_PCUTBN>& outVertexes, std::vector<unsigned int>& outIndexes )
{
	float startTime = (float)GetCurrentTimeSeconds();
	std::string fileContent;
	FileReadToString( fileContent, filename );
	std::vector<Vec3> v_positions;
	std::vector<Vec2> v_textures;
	std::vector<Vec3> v_normals;
	int faceCounter = 0;
	int triCounter = 0;
	Strings splitByLine;
	float parseTime = (float)GetCurrentTimeSeconds();
	splitByLine = SplitStringOnDelimiter( fileContent, "\r\n" );

	if ( (int)splitByLine.size() <= 1 )
	{
		splitByLine = SplitStringOnDelimiter( fileContent, "\n" );
	}

	for ( int lineIndex = 0; lineIndex < (int)splitByLine.size(); lineIndex++ )
	{
		int lineLength = (int)splitByLine[ lineIndex ].size();
		std::string& nowLine = splitByLine[ lineIndex ];
		if ( lineLength < 2 )
			continue;
		if ( nowLine[ 0 ] == 'v' )
		{
			// texture coordinates
			if ( nowLine[ 1 ] == 't' )
			{
				Strings splitBySpace = SplitStringOnDelimiter( nowLine, " " );
				int nowSet = 0;
				Vec2 vt;
				for ( int index = 1; index < (int)splitBySpace.size() && nowSet < 2; index++ )
				{
					if ( splitBySpace[ index ].size() > 0 )
					{
						if ( nowSet == 0 )
							vt.x = static_cast<float>( atof( splitBySpace[ index ].c_str() ) );
						if ( nowSet == 1 )
							vt.y = static_cast<float>( atof( splitBySpace[ index ].c_str() ) );
						nowSet++;
					}
				}
				if ( nowSet != 0 )
				{
					v_textures.push_back( vt );
				}
			}
			// normals
			else if ( nowLine[ 1 ] == 'n' )
			{
				Strings splitBySpace = SplitStringOnDelimiter( nowLine, " " );
				int nowSet = 0;
				Vec3 vn;
				for ( int index = 1; index < (int)splitBySpace.size() && nowSet < 3; index++ )
				{
					if ( splitBySpace[ index ].size() > 0 )
					{
						if ( nowSet == 0 )
							vn.x = static_cast<float>( atof( splitBySpace[ index ].c_str() ) );
						if ( nowSet == 1 )
							vn.y = static_cast<float>( atof( splitBySpace[ index ].c_str() ) );
						if ( nowSet == 2 )
							vn.z = static_cast<float>( atof( splitBySpace[ index ].c_str() ) );
						nowSet++;
					}
				}
				if ( nowSet != 0 )
				{
					if ( vn.GetLengthSquared() != 1.f )
						vn.Normalize();
					v_normals.push_back( vn );
				}
			}
			// positions
			else
			{
				Strings splitBySpace = SplitStringOnDelimiter( nowLine, " " );
				int nowSet = 0;
				Vec3 v;
				for ( int index = 1; index < (int)splitBySpace.size() && nowSet < 3; index++ )
				{
					if ( splitBySpace[ index ].size() > 0 )
					{
						if ( nowSet == 0 )
							v.x = static_cast<float>( atof( splitBySpace[ index ].c_str() ) );
						if ( nowSet == 1 )
							v.y = static_cast<float>( atof( splitBySpace[ index ].c_str() ) );
						if ( nowSet == 2 )
							v.z = static_cast<float>( atof( splitBySpace[ index ].c_str() ) );
						nowSet++;
					}
				}
				if ( nowSet != 0 )
				{
					v_positions.push_back( v );
				}
			}
		}

		else if ( nowLine[ 0 ] == 'f' )
		{
			faceCounter++;
			Strings splitBySpace = SplitStringOnDelimiter( nowLine, " " );
			Strings facesLine;
			// remove empty string caused by extra spaces
			for ( int i = 1; i < (int)splitBySpace.size(); i++ )
			{
				if ( splitBySpace[ i ].size() > 0 )
				{
					facesLine.push_back( splitBySpace[ i ] );
				}
			}
			// Get all vertexes of the face
			std::vector<Vertex_PCUTBN> faceVertexes;
			std::vector<unsigned int> faceIndexes;
			int inital = (int)outVertexes.size();
			for ( int vertexIndex = 0; vertexIndex < (int)facesLine.size(); vertexIndex++ )
			{
				faceIndexes.push_back( inital++ );
				Strings faceInfo = SplitStringOnDelimiter( facesLine[ vertexIndex ], "/" );
				int vIndex = -1;
				int vtIndex = -1;
				int vnIndex = -1;
				if ( faceInfo[ 0 ].size() > 0 )
					vIndex = static_cast<int>( atoi( faceInfo[ 0 ].c_str() ) ) - 1;
				if ( faceInfo.size() > 1 && faceInfo[ 1 ].size() > 0 )
					vtIndex = static_cast<int>( atoi( faceInfo[ 1 ].c_str() ) ) - 1;
				if ( faceInfo.size() > 2 && faceInfo[ 2 ].size() > 0 )
					vnIndex = static_cast<int>( atoi( faceInfo[ 2 ].c_str() ) ) - 1;

				Vertex_PCUTBN vertex;
				vertex.m_color = Rgba8::WHITE;
				vertex.m_position = v_positions[ vIndex ];
				if ( vtIndex != -1 )
					vertex.m_uvTexCoords = v_textures[ vtIndex ];
				if ( vnIndex != -1 )
					vertex.m_normal = v_normals[ vnIndex ];
				faceVertexes.push_back( vertex );
			}
			// Caculate the normal if it's not given
			if ( faceVertexes[ 0 ].m_normal == Vec3() )
			{
				Vec3 normal = CrossProduct3D( ( faceVertexes[ 1 ].m_position - faceVertexes[ 0 ].m_position ), ( faceVertexes[ 2 ].m_position - faceVertexes[ 1 ].m_position ) );
				normal.Normalize();
				for ( int vertexIndex = 0; vertexIndex < (int)faceVertexes.size(); vertexIndex++ )
				{
					faceVertexes[ vertexIndex ].m_normal = normal;
				}
			}
			for ( int vertexIndex = 0; vertexIndex < (int)faceVertexes.size(); vertexIndex++ )
			{
				outVertexes.push_back( faceVertexes[ vertexIndex ] );

			}
			for ( int vertexIndex = 1; vertexIndex < (int)faceIndexes.size() - 1; vertexIndex++ )
			{

				outIndexes.push_back( faceIndexes[ 0 ] );
				outIndexes.push_back( faceIndexes[ vertexIndex ] );
				outIndexes.push_back( faceIndexes[ vertexIndex + 1 ] );
				triCounter++;
			}
		}
	}

	if ( outVertexes.size() < v_positions.size() )
	{
		for ( int i = 0; i + 2 < v_positions.size(); i += 3 )
		{
			Vertex_PCUTBN v0, v1, v2;
			v0.m_position = v_positions[ i ];
			v1.m_position = v_positions[ i + 1 ];
			v2.m_position = v_positions[ i + 2 ];

			v0.m_color = Rgba8::WHITE;
			v1.m_color = Rgba8::WHITE;
			v2.m_color = Rgba8::WHITE;

			Vec3 normal = CrossProduct3D( ( v_positions[ i + 1 ] - v_positions[ i ] ), ( v_positions[ i + 2 ] - v_positions[ i + 1 ] ) );
			normal.Normalize();

			v0.m_normal = normal;
			v1.m_normal = normal;
			v2.m_normal = normal;

			outVertexes.push_back( v0 );
			outVertexes.push_back( v1 );
			outVertexes.push_back( v2 );
		}
		for ( int i = 0; i < (int)outVertexes.size(); i++ )
		{
			outIndexes.push_back( i );
		}
	}

	TransformVertexArray3D( outVertexes, transformMat );

	float createTime = (float)GetCurrentTimeSeconds();

	DebuggerPrintf( Stringf( "\n------------------------------------------- \nLoaded .obj file %s\n [file data]	vertexes: %d  texture coordinates: %d  normals: %d  faces: %d  triangles: %d\n [loaded mesh]	vertexes: %d	indexes: %d\n [time]		parse: %.6f seconds	create: %.6f seconds \n-------------------------------------------\n\n"
		, filename
		, (int)v_positions.size(), (int)v_textures.size(), (int)v_normals.size(), faceCounter, triCounter
		, (int)outVertexes.size(), (int)outIndexes.size()
		, ( parseTime - startTime ), ( createTime - parseTime ) ).c_str() );

}
//----------------------------------------------------------------------------------------------------------------------
// Tenshi version End
//----------------------------------------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------------------------------------
// Oak Version
//----------------------------------------------------------------------------------------------------------------------
void OBJLoader::LoadObjFile( std::string const& fileName, std::vector<Vertex_PCUTBN>& outVertsList, std::vector<unsigned int>& outIndexList, Mat44 transform )
{
	double timeBeforeLoading = GetCurrentTimeSeconds();

	// Read the file
	std::string outString;
	FileReadToString( outString, fileName.c_str() );
//	FileReadToString( outString, "Data/Models/Cube_v.obj" );
//	FileReadToString( outString, "Data/Models/Cube_vf.obj" );
//	FileReadToString( outString, "Data/Models/Cube_vfn.obj" );
//	FileReadToString( outString, "Data/Models/Cube_vfnt.obj" );
//	FileReadToString( outString, "Data/Models/Cow.obj" );
//	FileReadToString( outString, "Data/Models/Skyscraper.obj" );
//	FileReadToString( outString, "Data/Models/SovietMainBattleTank.obj" );
//	FileReadToString( outString, "Data/Models/Tank1.obj" );
//	FileReadToString( outString, "Data/Models/Tank2.obj" );
//	FileReadToString( outString, "Data/Models/Tank3.obj" );
//	FileReadToString( outString, "Data/Models/Tank4.obj" );
//	FileReadToString( outString, "Data/Models/Teapot.obj" );
//	FileReadToString( outString, "Data/Models/Teddy.obj" );

	// Split obj file data into strings based on '\n'
 	Strings strings = SplitStringOnDelimiter( outString, '\n' );

	// Remove '\r'
	for ( int i = 0; i < strings.size(); i++ )
	{
		if ( strings[i].find( '\r' ) != std::string::npos )
		{
			// == npos means '\r' was NOT found
			strings[i].erase( remove( strings[i].begin(), strings[i].end(), '\r' ) );
		}	
	}

	double timeAfterLoading = GetCurrentTimeSeconds();
	double parsingDuration = timeAfterLoading - timeBeforeLoading;

	std::vector<Vec3> positionList;
	std::vector<Vec2> uvTexCoordList;
	std::vector<Vec3> normalsList;

	int numFaces = 0;

	// Parse OBJ data
	for ( int i = 0; i < strings.size(); i++ )
	{
		std::string firstTwoCharOfThisString = strings[i].substr( 0, 2 );
		if ( firstTwoCharOfThisString == " " )
		{
			SplitStringOnDelimiter( firstTwoCharOfThisString, '/' );
		}
		if ( firstTwoCharOfThisString == "v " )
		{
			Strings vertPosString	= SplitStringOnDelimiter( strings[i], ' ' );
			int stringElement		= int(vertPosString.size());

			float x			= float( atof( vertPosString[stringElement - 3].c_str() ) );
			float y			= float( atof( vertPosString[stringElement - 2].c_str() ) );
			float z			= float( atof( vertPosString[stringElement - 1].c_str() ) );
			Vec3 position	= Vec3( x, y, z );
			positionList.emplace_back( position );			
		}
		if ( firstTwoCharOfThisString == "vn" )
		{
			Strings vertexNormalString = SplitStringOnDelimiter( strings[i], ' ' );
			float x			= float( atof( vertexNormalString[1].c_str() ) );
			float y			= float( atof( vertexNormalString[2].c_str() ) );
			float z			= float( atof( vertexNormalString[3].c_str() ) );
			Vec3 normals	= Vec3( x, y, z );
			normalsList.emplace_back( normals );
		}
		if ( firstTwoCharOfThisString == "vt" )
		{
			Strings uvtextCoordString = SplitStringOnDelimiter( strings[i], ' ' );
			float u			= float( atof( uvtextCoordString[1].c_str() ) );
			float v			= float( atof( uvtextCoordString[2].c_str() ) );
			Vec2 texCoords	= Vec2( u, v );
			uvTexCoordList.emplace_back( texCoords );
		}
		if ( firstTwoCharOfThisString == "f " )
		{
			Strings currentString = SplitStringOnDelimiter( strings[i], ' ' );

			// Start loop at '2' to skip 'f' at the start of the line and also the "space" after the 'f'
			for ( int j = 2; j < currentString.size() - 1; j++ )		
			{		
				int startIndex = 1;
				while ( currentString[startIndex] == " " || currentString[startIndex] == "" )
				{
					startIndex++;
				}
				
				int uvIndexA = -1;
				int uvIndexB = -1;
				int uvIndexC = -1;
				Strings subStringA = SplitStringOnDelimiter( currentString[startIndex], '/' );
				Strings subStringB = SplitStringOnDelimiter( currentString[j + 0	 ], '/' );
				Strings subStringC = SplitStringOnDelimiter( currentString[j + 1	 ], '/' );
				if ( subStringA.size() >= 3 )
				{
					uvIndexA = int( atof( subStringA[1].c_str() ) );
				}
				if ( subStringB.size() >= 3 )
				{
					uvIndexB = int( atof( subStringB[1].c_str() ) );
				}
				if ( subStringC.size() >= 3 )
				{
					uvIndexC = int( atof( subStringC[1].c_str() ) );
				}

				Vec3 triPosVertA		= Vec3::ZERO;		
				Vec3 triPosVertB		= Vec3::ZERO;		
				Vec3 triPosVertC		= Vec3::ZERO;		
				Vec3 currentTriNormal	= Vec3::ZERO;		
				Vec2 currentUVsA		= Vec2::ZERO;		
				Vec2 currentUVsB		= Vec2::ZERO;		
				Vec2 currentUVsC		= Vec2::ZERO;		
				int posIndexA			= 0;
				int posIndexB			= 0;
				int posIndexC			= 0;

				if ( positionList.size() != 0 )
				{
					posIndexA			= int( atof( currentString[startIndex].c_str() ) );
					posIndexB			= int( atof( currentString[j + 0     ].c_str() ) );
					posIndexC			= int( atof( currentString[j + 1     ].c_str() ) );
					triPosVertA			= positionList[posIndexA - 1];
					triPosVertB			= positionList[posIndexB - 1];
					int indexC			= posIndexC - 1;
					if ( indexC > 0 )
					{
						triPosVertC = positionList[indexC];
					}
					else
					{
						indexC = 0;
						triPosVertC = positionList[indexC];
						posIndexC = 1;
					}
				}				
			
				if ( uvTexCoordList.size() != 0 ) 
				{
//					int uvIndex			= int( atof( currentString[uvIndex].c_str() ) );
					if ( uvIndexA != -1 )
					{
						currentUVsA	= uvTexCoordList[uvIndexA - 1]; 
					}
					if ( uvIndexB != -1 )
					{
						currentUVsB	= uvTexCoordList[uvIndexB - 1]; 
					}
					if ( uvIndexC != -1 )
					{
						currentUVsC	= uvTexCoordList[uvIndexC - 1]; 
					}
				}

				// Calculate normals
				Vec3 posA = positionList[posIndexA - 1];
				Vec3 posB = positionList[posIndexB - 1];
				Vec3 posC = positionList[posIndexC - 1];

				Vec3 u = ( posB - posA ).GetNormalized();
				Vec3 v = ( posC - posB ).GetNormalized();

				currentTriNormal	= CrossProduct3D( u, v );
				currentTriNormal	= currentTriNormal.GetNormalized();
				normalsList.emplace_back( currentTriNormal );
				normalsList.emplace_back( currentTriNormal );
				normalsList.emplace_back( currentTriNormal );

				// Create Vertex_PCUTBN for current triangle, add to outVertsList
				Vertex_PCUTBN verts1 = Vertex_PCUTBN( triPosVertA, Rgba8::WHITE, currentUVsA, Vec3::ZERO, Vec3::ZERO, currentTriNormal );
				Vertex_PCUTBN verts2 = Vertex_PCUTBN( triPosVertB, Rgba8::WHITE, currentUVsB, Vec3::ZERO, Vec3::ZERO, currentTriNormal );
				Vertex_PCUTBN verts3 = Vertex_PCUTBN( triPosVertC, Rgba8::WHITE, currentUVsC, Vec3::ZERO, Vec3::ZERO, currentTriNormal );
				outVertsList.emplace_back( verts1 );																
				outVertsList.emplace_back( verts2 );
				outVertsList.emplace_back( verts3 );
				
				numFaces++;
			}
		}
	}

	//----------------------------------------------------------------------------------------------------------------------
	// V
	//----------------------------------------------------------------------------------------------------------------------
	if ( positionList.size() != 0 && normalsList.size() == 0 && uvTexCoordList.size() == 0 )
	{
		CalculateAndPopulateNormalsList( normalsList, positionList );

		//----------------------------------------------------------------------------------------------------------------------
		// If normals were NOT provided
		//----------------------------------------------------------------------------------------------------------------------
		int remainder = positionList.size() % 3;
		if ( remainder == 0 )
		{
			for ( int i = 0; i < positionList.size(); i += 3 )
			{
				Vec3 posA = positionList[i    ];
				Vec3 posB = positionList[i + 1];
				Vec3 posC = positionList[i + 2];

				Vec3 u = ( posB - posA ).GetNormalized();
				Vec3 v = ( posC - posB ).GetNormalized();

				Vec3 normal = CrossProduct3D( u, v );
				normal		= normal.GetNormalized();
				normalsList.emplace_back( normal );
				normalsList.emplace_back( normal );
				normalsList.emplace_back( normal );
			}
		}

		// Populate texCoordsList IF texCoords were NOT provided
		for ( int i = 0; i < positionList.size(); i++ )
		{
			Vec2 texCoords = Vec2::ZERO;
			uvTexCoordList.emplace_back( texCoords );
		}
	}

	//----------------------------------------------------------------------------------------------------------------------
	// VFN
	//----------------------------------------------------------------------------------------------------------------------
	if ( positionList.size() != 0 && normalsList.size() != 0 && uvTexCoordList.size() != 0 )
	{
		CalculateAndPopulateNormalsList( normalsList, positionList );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Combine all lists (pos, uv, normals) to create Vertex_PCUTBN
	//----------------------------------------------------------------------------------------------------------------------
	if ( outVertsList.size() == 0 )
	{
		for ( int i = 0; i < positionList.size(); i++ )
		{
			Vec3 currentPos		= positionList[i];
			Vec3 currentNormal	= normalsList[i];
			Vec2 currentUVs		= uvTexCoordList[i];
			Vertex_PCUTBN verts	= Vertex_PCUTBN( currentPos, Rgba8::WHITE, currentUVs, Vec3::ZERO, Vec3::ZERO, currentNormal );
			outVertsList.emplace_back( verts );
//			outIndexList.emplace_back( i );
		}
	}

	for ( int i = 0; i < outVertsList.size(); i++ )
	{
		outIndexList.emplace_back( i );
	}

	TransformVertexArray3D( outVertsList, transform );

	//----------------------------------------------------------------------------------------------------------------------
	// OutputObjStats
	int verts	= int( outVertsList.size() );
	int indexes	= int( outIndexList.size() );
	int numTri	= int( float( verts ) / 3.0f );

	// Print debug messages
	double timeAfterCreating = GetCurrentTimeSeconds();
	double creationDuration	 = timeAfterCreating - timeAfterLoading;
	DebuggerPrintf( "---------------------------------------------------------------------\n" );
	DebuggerPrintf( "Loaded .obj file Data/Models/%s\n", fileName.c_str() );
	DebuggerPrintf( "[file data]    verts: %d, texCoords: %d, normals: %d, faces: %d, triangles: %d \n", positionList.size(), uvTexCoordList.size(), normalsList.size(), numFaces, numTri );
	DebuggerPrintf( "[loaded mesh]  vertexes: %d ms,  indexes %d \n", verts, indexes );
	DebuggerPrintf( "[time]         parse: %f ms,     create: %f \n", parsingDuration, creationDuration );
	DebuggerPrintf( "---------------------------------------------------------------------\n" );
}

//----------------------------------------------------------------------------------------------------------------------
void OBJLoader::CalculateAndPopulateNormalsList( std::vector<Vec3>& normalsList, std::vector<Vec3> positionList )
{
	//----------------------------------------------------------------------------------------------------------------------
	// If normals were NOT provided
	//----------------------------------------------------------------------------------------------------------------------
	int remainder = positionList.size() % 3;
	if ( remainder == 0 )
	{
		for ( int i = 0; i < positionList.size(); i += 3 )
		{
			Vec3 posA = positionList[i    ];
			Vec3 posB = positionList[i + 1];
			Vec3 posC = positionList[i + 2];

			Vec3 u = ( posB - posA ).GetNormalized();
			Vec3 v = ( posC - posB ).GetNormalized();

			Vec3 normal = CrossProduct3D( u, v );
			normal		= normal.GetNormalized();
			normalsList.emplace_back( normal );
			normalsList.emplace_back( normal );
			normalsList.emplace_back( normal );
		}
	}
//	else
//	{
//
//
//
//	}
}
//*/
