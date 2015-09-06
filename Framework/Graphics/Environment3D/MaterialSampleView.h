//==================================================================================================================================
// MaterialSampleView.h
//
// This class contains the actual models used for the Material Sample View but not have the full environment for it.
//==================================================================================================================================
// History
//
// -Created on 8/25/2015 by Dustin Watson
//==================================================================================================================================
#ifndef __MATERIALSAMPLEVIEW_H
#define __MATERIALSAMPLEVIEW_H
//==================================================================================================================================
//==================================================================================================================================

//
// Includes
//

#include "D3D.h"
#include "ZMath.h"
#include "Camera.h"
#include "SphereMesh.h"
#include "PlaneMesh.h"
#include "ShaderMaterial.h"

//==================================================================================================================================
//==================================================================================================================================
class MaterialSampleView
{
public:
	
	MaterialSampleView(D3D* d3d);
	
	void Init();
	
	void Render(ZShadeSandboxMesh::MeshRenderParameters mrp);
	
	void AssignMaterial();
	void AssignMaterial(std::string materialName);
	
	void SetWireframe(bool wire);
	
	ZShadeSandboxLighting::ShaderMaterial*& Material() { return mMaterial; }
	ZShadeSandboxLighting::ShaderMaterial* Material() const { return mMaterial; }
	
	bool& HasMaterial() { return bHasMaterial; }
	bool HasMaterial() const { return bHasMaterial; }
	
private:
	
	bool bHasMaterial;
	
	ZShadeSandboxLighting::ShaderMaterial* mMaterial;
	
	ZShadeSandboxMesh::SphereMesh* mMesh;
	ZShadeSandboxMesh::PlaneMesh* mPlane;
	
	D3D* m_D3DSystem;
	EngineOptions* m_EngineOptions;
};
//==================================================================================================================================
//==================================================================================================================================
#endif//__MATERIALSAMPLEVIEW_H