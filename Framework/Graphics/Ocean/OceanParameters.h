//===============================================================================================================================
// OceanParameters
//
//===============================================================================================================================
// History
//
// -Created on 5/30/2015 by Dustin Watson
//===============================================================================================================================
#ifndef __OCEANPARAMETERS_H
#define __OCEANPARAMETERS_H
//===============================================================================================================================
//===============================================================================================================================

//
// Includes
//

#include "ZMath.h"

//===============================================================================================================================
//===============================================================================================================================
struct OceanParameters
{
	//
	// Default Ocean Parameters
	//
	
	//float 		g_WaveHeight; // Not needed
	float 		g_SpecularShininess;
	XMFLOAT3 	g_LightDirection;
	XMFLOAT3 	g_CamPos;
	float 		g_Time;
	XMFLOAT4 	g_RefractionTint;
	XMFLOAT4	g_DullColor;
	float		g_SeaLevel;
	
	float		g_Skyblending;

	// Water-reflected sky color
	XMFLOAT3		g_SkyColor;

	// The color of bottomless water body
	XMFLOAT3		g_WaterbodyColor;

	// The strength, direction and color of sun streak
	float			g_Shineness;
	XMFLOAT3		g_SunDir;
	XMFLOAT3		g_SunColor;

	// Subdivision thredshold. Any quad covers more pixels than this value needs to be subdivided.
	float		g_UpperGridCoverage;
	// Draw distance = g_PatchLength * 2^g_FurthestCover
	int			g_FurthestCover;
	
	int			g_MeshDimention;

	// the relative values of these are critical to "scale" of the FFT effect
	int			g_OceanSize;
	int			g_PatchSize;
	
	//
	// Tessellation Parameters
	//
	
	float		g_MinDist;
	float		g_MaxDist;
	float		g_MinTess;
	float		g_MaxTess;
	
	//
	// FFT Parameters
	//
	
	// Adjust this parameter to control the simulation speed
	float		g_TimeScale;
	// A scale to control the amplitude. Not the world space height
	float		g_WaveAmplitude;
	// 2D wind direction. No need to be normalized
	XMFLOAT2	g_WindDir;
	// The bigger the wind speed, the larger scale of wave crest.
	// But the wave scale can be no larger than patch_length
	float		g_WindSpeed;
	// Damp out the components opposite to wind direction.
	// The smaller the value, the higher wind dependency
	float		g_WindDependency;
	// Control the scale of horizontal movement. Higher value creates
	// pointy crests.
	float		g_ChoppyScale;
	//size of the height map
	float		g_MapDimension;
	//<JPK> the bigger the length, the more aggressive the effect
	float		g_PatchLength;
	
	//
	// Perlin Wave Parameters
	//
	
	float		g_PerlinSize;
	float		g_PerlinSpeed;
	XMFLOAT3 	g_PerlinAmplitude;
	XMFLOAT3 	g_PerlinGradient;
	XMFLOAT3 	g_PerlinOctave;

	// The parameter is used for fixing an artifact
	// indirect reflection vector
	XMFLOAT3		g_BendParam;
};

//===============================================================================================================================
//===============================================================================================================================
#endif//__OCEANPARAMETERS_H