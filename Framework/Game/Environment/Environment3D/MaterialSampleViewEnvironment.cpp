#include "MaterialSampleViewEnvironment.h"
#include "ShapeContact.h"
#include "MaterialManager.h"
#include "ShaderMaterial.h"
#include "XMMath3.h"
#include "Vertex.h"
//===============================================================================================================================
//===============================================================================================================================
MaterialSampleViewEnvironment::MaterialSampleViewEnvironment(EngineOptions* eo)
:   InteractiveEnvironment3D(eo)
{
	Initialize();
}
//===============================================================================================================================
MaterialSampleViewEnvironment::~MaterialSampleViewEnvironment()
{
	Shutdown();
}
//===============================================================================================================================
bool MaterialSampleViewEnvironment::Initialize()
{
	// Position the camera to view the sphere
	m_CameraSystem->SetPosition(10.0f, 2.0f, 10.0f);
	
	// Need to be able to rotate the camera initially (Need to figure out correct value to use)
	m_CameraSystem->UpdateYaw(5);
	
	bEnableReflections = false;
	bEnableRefractions = false;
	
	m_CameraSystem->SetRenderReflectionView(bEnableReflections);
	
	fSeaLevel = -5.0f;
	
	mDirLight = new ZShadeSandboxLighting::DirectionalLight();
	mDirLight->LightType() = ZShadeSandboxLighting::ELightType::eDirectional;
	mDirLight->DiffuseColor() = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mDirLight->AmbientColor() = XMFLOAT4(0.15f, 0.15f, 0.15f, 1.0f);
	mDirLight->SpecularColor() = XMFLOAT4(0.8f, 0.8f, 0.7f, 1.0f);
	mDirLight->Position() = XMFLOAT3(0, 10, 0);
	mDirLight->Direction() = XMFLOAT3(0.936016f, -0.343206f, 0.0780013f);
	mDirLight->Perspective()->SceneWidth() = 512;
	mDirLight->Perspective()->SceneHeight() = 512;
	mDirLight->SetLens(0.1f, 10.0f);
	
	ZShadeSandboxLighting::LightManager::Instance()->AddLight(mDirLight);
	ZShadeSandboxLighting::DeferredShaderManager::Instance()->AddLight(mDirLight);
	
	// Starts out with no material so that one can be added or created
	mMaterialSampleView = new MaterialSampleView(m_D3DSystem);
	mMaterialSampleView->Init();
	
	return true;
}
//===============================================================================================================================
void MaterialSampleViewEnvironment::Shutdown()
{
	Environment3D::Shutdown();
}
//===============================================================================================================================
void MaterialSampleViewEnvironment::Update()
{
	mMaterialSampleView->SetWireframe(bWireframeMode);
	UpdateSpawnedMeshItems(fFrameTime);
	ToggleLightMeshWireframe(bWireframeMode);
}
//===============================================================================================================================
void MaterialSampleViewEnvironment::Render()
{
	if( Quickwire() )
	{
		mMaterialSampleView->SetWireframe(true);
		ToggleSpawnedMeshItemsWireframe(true);
		ToggleLightMeshWireframe(true);
		
		m_D3DSystem->TurnOnWireframe();
	}
	//else
	//{
	//	if (!bWireframeMode)
	//	{
	//		m_D3DSystem->TurnOffCulling();
	//	}
	//}
	
	ZShadeSandboxMesh::MeshRenderParameters mrp;
	mrp.camera = m_CameraSystem;
	mrp.light = mDirLight;
	mrp.renderLight = true;
	
	mMaterialSampleView->Render(mrp);
	
	RenderSpawnedMeshItems(mrp);
	RenderLightMesh(mrp);
}
//===============================================================================================================================
void MaterialSampleViewEnvironment::RenderDeferred()
{
	if( Quickwire() )
	{
		mMaterialSampleView->SetWireframe(true);
		ToggleSpawnedMeshItemsWireframe(true);
		ToggleLightMeshWireframe(true);
		
		m_D3DSystem->TurnOnWireframe();
	}
	//else
	//{
	//	if (!bWireframeMode)
	//	{
	//		m_D3DSystem->TurnOffCulling();
	//	}
	//}
	
	ZShadeSandboxMesh::MeshRenderParameters mrp;
	mrp.camera = m_CameraSystem;
	mrp.renderDeferred = true;
	mrp.light = mDirLight;
	mrp.renderLight = true;
	
	mMaterialSampleView->Render(mrp);
	
	RenderSpawnedMeshItems(mrp);
	RenderLightMesh(mrp);
}
//===============================================================================================================================