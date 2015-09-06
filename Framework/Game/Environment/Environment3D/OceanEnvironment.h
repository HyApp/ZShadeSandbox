//===============================================================================================================================
// OceanEnvironment
//
//===============================================================================================================================
// History
//
// -Created on 6/21/2014 by Dustin Watson
//===============================================================================================================================
#ifndef __OCEANENVIRONMENT_H
#define __OCEANENVIRONMENT_H
//===============================================================================================================================
//===============================================================================================================================

//
// Includes
//
#pragma region "Includes"
#include "TextureManager.h"
#include "Environment3D.h"
#include "Water.h"
#include "LakeBed.h"
#include "OceanParameters.h"
#include "OceanSurface.h"
#pragma endregion

//===============================================================================================================================
//===============================================================================================================================

//
// Ocean Environment with Sky and Ocean water
//
class OceanEnvironment : public Environment3D
{
public:
	
	OceanEnvironment(EngineOptions* eo);
	~OceanEnvironment();
	
	bool Init();
	void Shutdown();
	
	void Update();
	void Render();
	void RenderDeferred();
	
	void RenderReflection(XMFLOAT4 clipplane);
	void RenderRefraction(XMFLOAT4 clipplane);
	void RenderShadowMap();
	
private:
	
	Water* mWater;
	LakeBed* mLakeBed;
	
	OceanSurface* mOceanSurface;
	
	ZShadeSandboxMesh::SphereMesh* mSphere;
	OceanParameters ocean_params;
	
	// Object in the water
	ZShadeSandboxMesh::CubeMesh* mCubeAboveWater;
	ZShadeSandboxMesh::CubeMesh* mCubeBelowWater;
};

//===============================================================================================================================
//===============================================================================================================================
#endif//__OCEANENVIRONMENT_H