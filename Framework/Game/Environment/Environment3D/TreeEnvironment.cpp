#include "TreeEnvironment.h"
#include "ShapeContact.h"
#include "MaterialManager.h"
#include "ShaderMaterial.h"
#include "XMMath3.h"
#include "Vertex.h"
//===============================================================================================================================
//===============================================================================================================================
TreeEnvironment::TreeEnvironment(EngineOptions* eo)
:   Environment3D(eo)
,	bTreeBillboardTextureCreated(false)
{
	Initialize();
}
//===============================================================================================================================
TreeEnvironment::~TreeEnvironment()
{
	Shutdown();
}
//===============================================================================================================================
bool TreeEnvironment::Initialize()
{
	// Position the camera around the polygon
	m_CameraSystem->SetPosition(5.0f, 0.0f, 5.0f);
	
	bEnableReflections = false;
	bEnableRefractions = false;
	
	m_CameraSystem->SetRenderReflectionView(bEnableReflections);
	
	fSeaLevel = -5.0f;
	
	TreeParameters tp;
	
	tp.gameDirectory3D = m_GameDirectory3D;
	tp.treeCount = 50;
	
	// Need to get texture and model before this can render
	tp.leafTextureName = "leaf.dds";
	tp.treeModelName = "tree.obj";
	
	mTrees = new Tree(m_D3DSystem, tp);
	
	mTrees->Init();
	
	return true;
}
//===============================================================================================================================
void TreeEnvironment::Shutdown()
{
	Environment3D::Shutdown();
}
//===============================================================================================================================
void TreeEnvironment::Update()
{
	mTrees->SetWireframe(bWireframeMode);
	UpdateSpawnedMeshItems(fFrameTime);
	ToggleLightMeshWireframe(bWireframeMode);
}
//===============================================================================================================================
void TreeEnvironment::Render()
{
	if( Quickwire() )
	{
		mTrees->SetWireframe(true);
		ToggleSpawnedMeshItemsWireframe(true);
		ToggleLightMeshWireframe(true);
		
		m_D3DSystem->TurnOnWireframe();
	}
	else
	{
		if (!bWireframeMode)
		{
			m_D3DSystem->TurnOffCulling();
		}
	}
	
	ZShadeSandboxMesh::MeshRenderParameters mrp;
	mrp.camera = m_CameraSystem;
	mrp.light = mDirLight1;
	
	if (!bTreeBillboardTextureCreated)
	{
		mTrees->SaveBillboardTexture(mrp);
		bTreeBillboardTextureCreated = true;
	}
	
	mTrees->Render(mrp, mFrustumPlanes);
	
	RenderSpawnedMeshItems(mrp);
	RenderLightMesh(mrp);
}
//===============================================================================================================================
void TreeEnvironment::RenderDeferred()
{
	if( Quickwire() )
	{
		mTrees->SetWireframe(true);
		ToggleSpawnedMeshItemsWireframe(true);
		ToggleLightMeshWireframe(true);
		
		m_D3DSystem->TurnOnWireframe();
	}
	else
	{
		if (!bWireframeMode)
		{
			m_D3DSystem->TurnOffCulling();
		}
	}
	
	ZShadeSandboxMesh::MeshRenderParameters mrp;
	mrp.camera = m_CameraSystem;
	mrp.renderDeferred = true;
	mrp.light = mDirLight1;
	
	if (!bTreeBillboardTextureCreated)
	{
		mTrees->SaveBillboardTexture(mrp);
		bTreeBillboardTextureCreated = true;
	}
	
	mTrees->Render(mrp, mFrustumPlanes);
	
	RenderSpawnedMeshItems(mrp);
	RenderLightMesh(mrp);
}
//===============================================================================================================================