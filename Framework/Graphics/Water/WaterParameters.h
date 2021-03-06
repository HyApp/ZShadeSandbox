//=====================================================================================
//WaterParameters.h
//=====================================================================================
#ifndef __WATERPARAMETERS_H
#define __WATERPARAMETERS_H
//==============================================================================================================================
//==============================================================================================================================
#include "D3D.h"
//==============================================================================================================================
//==============================================================================================================================
struct WaterParameters
{
	float 		g_WaveHeight;
	float 		g_SpecularShininess;
	float 		g_Time;
	XMFLOAT3 	g_CamPos;
	XMFLOAT3 	g_LightDirection;
	XMFLOAT4 	g_RefractionTint;
	XMFLOAT4 	g_DullColor;
	float 		g_waterHeight;
	float 		g_waterRadius;
	float 		g_HalfCycle;
	float		g_FlowMapOffset0;
	float		g_FlowMapOffset1;
	float 		g_TexScale;
};
//==============================================================================================================================
//==============================================================================================================================
#endif//__WATERPARAMETERS_H