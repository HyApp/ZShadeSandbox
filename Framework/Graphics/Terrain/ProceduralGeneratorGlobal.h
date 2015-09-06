//===============================================================================================================================
// Procedural Generator Global
//
// Contains a set of generic global helper functions for procedurally generating a heightmap
//===============================================================================================================================
// History
//
// -Created on 6/16/2015 by Dustin Watson
//===============================================================================================================================
#ifndef __PROCEDURALGENERATORGLOBAL_H
#define __PROCEDURALGENERATORGLOBAL_H
//===============================================================================================================================
//===============================================================================================================================

//
// Includes
//

#include "HeightErosion.h"
#include "WaterErosion.h"
#include "HeightmapContainer.h"
#include "ProceduralParameters.h"

// Value representing invalid tile.
#define PT_INVALID_HEIGHT -32768

// Minimum reachable height.
#define PT_MIN_HEIGHT -32767

// Maximum reachable height.
#define PT_MAX_HEIGHT 32767

//===============================================================================================================================
//===============================================================================================================================
namespace ZShadeSandboxTerrain {
class ProceduralGeneratorGlobal
{
public:
	
	ProceduralGeneratorGlobal();
	~ProceduralGeneratorGlobal();
	
	static void BuildRandomHeightmap
	(	int size
	,	int minHeight
	,	int maxHeight
	,	ZShadeSandboxTerrain::HeightmapContainer& outContainer
	);
	static void BuildPerlinNoiseHeightmap(int size, ZShadeSandboxTerrain::HeightmapContainer& outContainer);
	static void BuildFieldNoiseHeightmap(int size, ZShadeSandboxTerrain::HeightmapContainer& outContainer);
	
	// Uses the diamond-square mid-point displacement algorithm
	static void BuildDiamondSquare
	(	int size
	,	int minHeight
	,	int maxHeight
	,	int waveLength
	,	float scale
	,	EDiamondSquareType::Type diamondSqrType
	,	ZShadeSandboxTerrain::HeightmapContainer& outContainer
	);
	static void BuildDiamondSquare
	(	int size
	,	int minHeight
	,	int maxHeight
	,	float scale
	,	float* amplitudes
	,	EDiamondSquareType::Type diamondSqrType
	,	ZShadeSandboxTerrain::HeightmapContainer& outContainer
	);
	
	
	/*static void Union(int size, ZShadeSandboxTerrain::HeightmapContainer inContainer, ZShadeSandboxTerrain::HeightmapContainer& outContainer);
	static void Union(int size, ZShadeSandboxTerrain::HeightmapContainer& outContainer);
	static void Crop(int size, float minHeight, float maxHeight, ZShadeSandboxTerrain::HeightmapContainer& outContainer);
	static void Clamp(int size, float minHeight, float maxHeight, ZShadeSandboxTerrain::HeightmapContainer& outContainer);
	static void Invert(ZShadeSandboxTerrain::HeightmapContainer& outContainer);
	static float Min(ZShadeSandboxTerrain::HeightmapContainer outContainer, float& outMin);
	static float Max(ZShadeSandboxTerrain::HeightmapContainer outContainer, float& outMax);*/
	
	static signed int log2(signed int x);
	static float PerlinValue(int x, int y, int random);
	static float BicubicInterpolation(float p[4][4]);
	
private:
	
	static float ReadOverflowProceduralHeight(int x, int z, int size, ZShadeSandboxTerrain::HeightmapContainer outContainer);
	
private:
	
	static vector<ZShadeSandboxTerrain::HeightData> mVerticalOverflow;
	static vector<ZShadeSandboxTerrain::HeightData> mHorizontalOverflow;
};
}
//===============================================================================================================================
#endif//__PROCEDURALGENERATORGLOBAL_H