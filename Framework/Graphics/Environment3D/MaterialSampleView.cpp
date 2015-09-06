#include "MaterialSampleView.h"
#include "MaterialManager.h"
//==================================================================================================================================
//==================================================================================================================================
MaterialSampleView::MaterialSampleView(D3D* d3d)
:	m_D3DSystem(d3d)
,	bHasMaterial(false)
,	mMaterial(0)
,	mMesh(0)
{
	m_EngineOptions = d3d->GetEngineOptions();
}
//==================================================================================================================================
void MaterialSampleView::Init()
{
	ZShadeSandboxMesh::MeshParameters mp;
	mp.useCustomShader = false;
	mp.vertexType = ZShadeSandboxMesh::EVertexType::VT_NormalTex;
	mp.rotationAxisX = false;
	mp.rotationAxisY = false;
	mp.rotationAxisZ = false;
	mp.pos = XMFLOAT3(0, 0, 0);
	mp.rot = XMFLOAT3(0, 0, 0);
	mp.scale = XMFLOAT3(1, 1, 1);
	mp.shader = 0;
	
	mp.scale = XMFLOAT3(1, 1, 1);
	mp.material = MaterialManager::Instance()->GetMaterial("Stone");
	mPlane = new ZShadeSandboxMesh::PlaneMesh(m_D3DSystem, mp, "Models\\plane01.txt");
	
	// Start out with no material
	mp.pos = XMFLOAT3(0, 2, 0);
	mp.scale = XMFLOAT3(1, 1, 1);
	mp.material = 0;
	mMesh = new ZShadeSandboxMesh::SphereMesh(m_D3DSystem, mp, "Models\\sphere.txt");
}
//==================================================================================================================================
void MaterialSampleView::Render(ZShadeSandboxMesh::MeshRenderParameters mrp)
{
	mPlane->Render(mrp);
	mMesh->Render(mrp);
}
//==================================================================================================================================
void MaterialSampleView::SetWireframe(bool wire)
{
	mPlane->SetWireframe(wire);
	mMesh->SetWireframe(wire);
}
//==================================================================================================================================
void MaterialSampleView::AssignMaterial(std::string materialName)
{
	mMaterial = MaterialManager::Instance()->GetMaterial(materialName);
	mMesh->SetMaterial(mMaterial);
	bHasMaterial = true;
}
//==================================================================================================================================
void MaterialSampleView::AssignMaterial()
{
	mMesh->SetMaterial(mMaterial);
	bHasMaterial = true;
}
//==================================================================================================================================
