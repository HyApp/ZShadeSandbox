#include "OceanSurface.h"
#include "OceanParameters.h"
#include "TextureManager.h"
#include "CGlobal.h"
#include "Shader.h"
#include "ShaderCompiler.h"
//==============================================================================================================================
//==============================================================================================================================
OceanSurface::OceanSurface(D3D* d3d, OceanParameters ocean_params)
:   mD3DSystem(d3d)
,   fTime(0)
{
	mOceanParams = ocean_params;
	
	Initialize();
}
//==============================================================================================================================
void OceanSurface::Initialize()
{
	mOceanRenderer = new OceanRenderer(mD3DSystem, mOceanParams);

	mOceanShader = new ZShadeSandboxShader::OceanSimulationShader(mD3DSystem, mOceanParams);

	//update for the first time
	Update(mOceanParams.g_SeaLevel, 0);

	mOceanRenderer->SetDisplacementMap(mOceanShader->getD3D11DisplacementMap());
	mOceanRenderer->SetGradientMap(mOceanShader->getD3D11GradientMap());
	mOceanRenderer->SetPerlinMap(mOceanShader->getD3D11PerlinMap());
}
//==============================================================================================================================
XMFLOAT3 OceanSurface::ReadWave(int x, int z)
{
	return mOceanShader->ReadWave(x, z);
}
//==============================================================================================================================
void OceanSurface::SetReflectionMap(ID3D11ShaderResourceView* srv)
{
	mOceanRenderer->SetReflectionMap(srv);
}
//==============================================================================================================================
void OceanSurface::SetRefractionMap(ID3D11ShaderResourceView* srv)
{
	mOceanRenderer->SetRefractionMap(srv);
}
//==============================================================================================================================
void OceanSurface::SetWireframe(bool wire)
{
	mOceanRenderer->SetWireframe(wire);
}
//==============================================================================================================================
void OceanSurface::Update(float seaLevel, float dt)
{
	fTime += 0.005f * dt;
	mOceanParams.g_Time = fTime;
	mOceanParams.g_SeaLevel = seaLevel;
	mOceanShader->updateDisplacementMap(dt);
}
//==============================================================================================================================
void OceanSurface::Render(Camera* camera)
{
	mOceanRenderer->Render(camera);
}
//==============================================================================================================================