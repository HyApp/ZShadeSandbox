#include "ProceduralAdditionHelper.h"
#include "ProceduralGeneratorGlobal.h"
#include "ProceduralGenerator.h"
#include "ZMath.h"
using ZShadeSandboxTerrain::ProceduralAdditionHelper;
//===============================================================================================================================
//===============================================================================================================================
ProceduralAdditionHelper::ProceduralAdditionHelper(ZShadeSandboxTerrain::HeightmapContainer heightmapInput, ZShadeSandboxTerrain::ProceduralParameters pp)
:	mProceduralParameters(pp)
{
	mProceduralHeightmap.HeightmapSize() = mProceduralParameters.terrainSize;
	mProceduralHeightmap.SeaLevel() = mProceduralParameters.seaLevel;
	mProceduralHeightmap.Init();
	
	mHeightMapInput.HeightmapSize() = mProceduralParameters.terrainSize;
	mHeightMapInput.SeaLevel() = mProceduralParameters.seaLevel;
	mHeightMapInput.CopyHeight(heightmapInput);
}
//===============================================================================================================================
ProceduralAdditionHelper::~ProceduralAdditionHelper()
{
}
//===============================================================================================================================
void ProceduralAdditionHelper::AddRandomHeightmap()
{
	XMFLOAT2 point(mProceduralParameters.seaLevel, mProceduralParameters.maxHeight);
	
	ZShadeSandboxMath::ZMath::RandomSeed();
	
	for (int z = 0; z < mProceduralParameters.terrainSize; z++)
	{
		for (int x = 0; x < mProceduralParameters.terrainSize; x++)
		{
			float height = mHeightMapInput.ReadHeight(x, z) + ZShadeSandboxMath::ZMath::RandF<float>(point.x, point.y) * 0.01f;
			
			mProceduralHeightmap.UpdateHeight(x, z, height);
		}
	}
}
//===============================================================================================================================
void ProceduralAdditionHelper::AddPerlinNoiseHeightmap()
{
	ZShadeSandboxMath::ZMath::RandomSeed();
	
	for (int z = 0; z < mProceduralParameters.terrainSize; z++)
	{
		for (int x = 0; x < mProceduralParameters.terrainSize; x++)
		{
			float height = mHeightMapInput.ReadHeight(x, z) + ProceduralGeneratorGlobal::PerlinValue(x, z, 1000) * 5.0f;
			
			mProceduralHeightmap.UpdateHeight(x, z, height);
		}
	}
}
//===============================================================================================================================
void ProceduralAdditionHelper::AddFieldNoiseHeightmap()
{
	for (int z = 0; z < mProceduralParameters.terrainSize; z++)
	{
		for (int x = 0; x < mProceduralParameters.terrainSize; x++)
		{
			float height = mHeightMapInput.ReadHeight(x, z) + (float)sin(x);
			
			mProceduralHeightmap.UpdateHeight(x, z, height);
		}
	}
}
//===============================================================================================================================
void ProceduralAdditionHelper::RadiateHeightFromOrigin(int centerX, int centerZ, int radius, float minHeight, float maxHeight, bool addToInput)
{
	float realMaxHeight = maxHeight - minHeight;
	
	for (int z = 0; z < mProceduralParameters.terrainSize; z++)
	{
		for (int x = 0; x < mProceduralParameters.terrainSize; x++)
		{
			float distance = (float)sqrt(SQR((float)abs(x - centerX)) + SQR((float)abs(z - centerZ)));
			
			if (distance < (float)radius)
			{
				float height = minHeight + realMaxHeight * distance / (float)radius;
				
				if (addToInput)
				{
					height += mHeightMapInput.ReadHeight(x, z);
				}
				
				mProceduralHeightmap.UpdateHeight(x, z, height);
			}
			else
			{
				float height = minHeight + realMaxHeight;
				
				if (addToInput)
				{
					height += mHeightMapInput.ReadHeight(x, z);
				}
				
				mProceduralHeightmap.UpdateHeight(x, z, height);
			}
		}
	}
}
//===============================================================================================================================
void ProceduralAdditionHelper::DistortMap(short waveLength, int amplitude)
{
	float* amplitudes = new float[ProceduralGeneratorGlobal::log2(waveLength) + 1];
	for (int i = 0; i < ProceduralGeneratorGlobal::log2(waveLength) + 1; i++) amplitudes[i] = 0;
	amplitudes[ProceduralGeneratorGlobal::log2(waveLength)] = amplitude;
	
	// Add turbulence maps
	
	ZShadeSandboxTerrain::HeightmapContainer outContainerX;
	outContainerX.HeightmapSize() = mProceduralParameters.terrainSize;
	outContainerX.SeaLevel() = mProceduralParameters.seaLevel;
	outContainerX.Init();
	
	ProceduralGeneratorGlobal::BuildDiamondSquare(
		mProceduralParameters.terrainSize,
		1,
		waveLength,
		mProceduralParameters.diamondSquareScale,
		amplitudes,
		EDiamondSquareType::Type::eBicubic,
		outContainerX
	);
	
	ZShadeSandboxTerrain::HeightmapContainer outContainerZ;
	outContainerZ.HeightmapSize() = mProceduralParameters.terrainSize;
	outContainerZ.SeaLevel() = mProceduralParameters.seaLevel;
	outContainerZ.Init();
	
	ProceduralGeneratorGlobal::BuildDiamondSquare(
		mProceduralParameters.terrainSize,
		1,
		waveLength,
		mProceduralParameters.diamondSquareScale,
		amplitudes,
		EDiamondSquareType::Type::eBicubic,
		outContainerZ
	);
	
	ZShadeSandboxTerrain::HeightmapContainer distortedMap;
	distortedMap.HeightmapSize() = mProceduralParameters.terrainSize;
	distortedMap.SeaLevel() = mProceduralParameters.seaLevel;
	distortedMap.Init();
	
	for (int z = 0; z < mProceduralParameters.terrainSize; z++)
	{
		for (int x = 0; x < mProceduralParameters.terrainSize; x++)
		{
			float turbulantHeightX = outContainerX.ReadHeight(x, z);
			float turbulantHeightZ = outContainerZ.ReadHeight(x, z);
			
			float distortedX = max(min(x + turbulantHeightX, mProceduralParameters.terrainSize - 1), 0);
			float distortedZ = max(min(z + turbulantHeightZ, mProceduralParameters.terrainSize - 1), 0);
			
			float height = mProceduralHeightmap.ReadHeight(distortedX + mProceduralParameters.terrainSize * distortedZ);
			
			distortedMap.UpdateHeight(x, z, height);
		}
	}
	
	// Update the procedural map to have the distorted map values
	for (int z = 0; z < mProceduralParameters.terrainSize; z++)
	{
		for (int x = 0; x < mProceduralParameters.terrainSize; x++)
		{
			float height = distortedMap.ReadHeight(x, z);
			
			mProceduralHeightmap.UpdateHeight(x, z, height);
		}
	}
}
//===============================================================================================================================
void ProceduralAdditionHelper::SlopeMap()
{
	ZShadeSandboxTerrain::HeightmapContainer slopeMap;
	slopeMap.HeightmapSize() = mProceduralParameters.terrainSize;
	slopeMap.SeaLevel() = mProceduralParameters.seaLevel;
	slopeMap.Init();
	
	for (int z = 0; z < mProceduralParameters.terrainSize - 1; z++)
	{
		for (int x = 0; x < mProceduralParameters.terrainSize - 1; x++)
		{
			float height1 = mProceduralHeightmap.ReadHeight(x + z * mProceduralParameters.terrainSize - 1);
			float height2 = mProceduralHeightmap.ReadHeight(x + z * mProceduralParameters.terrainSize + 1);
			float height3 = mProceduralHeightmap.ReadHeight(x + z * mProceduralParameters.terrainSize - mProceduralParameters.terrainSize);
			float height4 = mProceduralHeightmap.ReadHeight(x + z * mProceduralParameters.terrainSize + mProceduralParameters.terrainSize);
			
			float height = max(abs(height1 - height2), (height3 - height4));
			
			slopeMap.UpdateHeight(x, z, height);
		}
	}
	
	// Fix the corners
	float height0 = slopeMap.ReadHeight(mProceduralParameters.terrainSize - 1);
	slopeMap.UpdateHeight(0, height0);
	float height1 = slopeMap.ReadHeight(2 * mProceduralParameters.terrainSize - 2);
	slopeMap.UpdateHeight(mProceduralParameters.terrainSize - 1, height1);
	float height2 = slopeMap.ReadHeight(mProceduralParameters.terrainSize - 2 * mProceduralParameters.terrainSize + 1);
	slopeMap.UpdateHeight(mProceduralParameters.terrainSize - mProceduralParameters.terrainSize, height2);
	float height3 = slopeMap.ReadHeight(mProceduralParameters.terrainSize - mProceduralParameters.terrainSize - 2);
	slopeMap.UpdateHeight(mProceduralParameters.terrainSize - 1, height3);
	
	// Upper border
	for (int x = 1; x < mProceduralParameters.terrainSize - 1; x++)
	{
		float height = slopeMap.ReadHeight(x + mProceduralParameters.terrainSize);
		slopeMap.UpdateHeight(x, height);
	}
	
	// Bottom corner
	for (int x = 1; x < mProceduralParameters.terrainSize - 1; x++)
	{
		float height = slopeMap.ReadHeight(mProceduralParameters.terrainSize - 2 * mProceduralParameters.terrainSize + x);
		slopeMap.UpdateHeight(mProceduralParameters.terrainSize - mProceduralParameters.terrainSize + x, height);
	}
	
	// Left border
	for (int z = 1; z < mProceduralParameters.terrainSize - 1; z++)
	{
		float height = slopeMap.ReadHeight(z * mProceduralParameters.terrainSize + 1);
		slopeMap.UpdateHeight(z * mProceduralParameters.terrainSize, height);
	}
	
	// Right border
	for (int z = 1; z < mProceduralParameters.terrainSize - 1; z++)
	{
		float height = slopeMap.ReadHeight(z * mProceduralParameters.terrainSize + mProceduralParameters.terrainSize - 2);
		slopeMap.UpdateHeight(z * mProceduralParameters.terrainSize + mProceduralParameters.terrainSize - 1, height);
	}
	
	// Update the procedural map to have the slope map values
	for (int z = 0; z < mProceduralParameters.terrainSize; z++)
	{
		for (int x = 0; x < mProceduralParameters.terrainSize; x++)
		{
			float height = slopeMap.ReadHeight(x, z);
			
			mProceduralHeightmap.UpdateHeight(x, z, height);
		}
	}
}
//===============================================================================================================================
void ProceduralAdditionHelper::FloodMap(float water)
{
	/*
	unsigned int target = (unsigned int)(water_amount * (double)this->length);
	
	signed short min = this->Min();
	signed short max = this->Max();
	
	// Use the binary search algorithm to find the correct water level
	signed short middle;
	
	while (max - min > 1)
	{
		middle = min + (max - min) / 2;
		
		unsigned int amount = 0;
		
		for (unsigned int i = 0; i < this->length; i++)
		{
			if (this->data[i] >= middle)
				amount++;
		}
		
		if (amount > target)
			min = middle;
		else
			max = middle;
	}
	
	// Shift the heights so given portion of the array is under zero
	this->Add(-middle);
	*/
	
	//int target = (water * mProceduralParameters.terrainSize);
	
	//int min = 
}
//===============================================================================================================================
