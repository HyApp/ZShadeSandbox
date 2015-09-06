//==============================================================================================================================
// OceanSurface.h
//
//==============================================================================================================================
// History
//
// -Created on 5/30/2015  by Dustin Watson
//==============================================================================================================================
#ifndef __OCEANSURFACE_H
#define __OCEANSURFACE_H
//==============================================================================================================================
//==============================================================================================================================

//
// Includes
//

#include "OceanParameters.h"
#include "OceanRenderer.h"
#include "OceanSimulationShader.h"

//==============================================================================================================================
//==============================================================================================================================

class OceanSurface
{
public:
	
	OceanSurface(D3D* d3d, OceanParameters ocean_params);
	
	void Initialize();
	
	void SetWireframe(bool wire);
	
	void Update(float seaLevel, float dt);
	void Render(Camera* camera);
	
	void SetReflectionMap(ID3D11ShaderResourceView* srv);
	void SetRefractionMap(ID3D11ShaderResourceView* srv);
	
	// Reads height from the FFT displacement map
	XMFLOAT3 ReadWave(int x, int z);

private:
	
	D3D* mD3DSystem;
	
	float fTime;
	int iFrustumCull;
	bool bRootContainsCamera;
	
	OceanParameters mOceanParams;
	
	OceanRenderer* mOceanRenderer;
	ZShadeSandboxShader::OceanSimulationShader* mOceanShader;
};

//==============================================================================================================================
//==============================================================================================================================
#endif//__OCEANSURFACE_H