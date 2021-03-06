#include "ModelEnvironment.h"
#include "LightManager.h"
#include "CapsuleLight.h"
#include "AmbientLight.h"
#include "LightShadingBuffers.h"
#include "MaterialManager.h"
//===============================================================================================================================
//===============================================================================================================================
ModelEnvironment::ModelEnvironment(EngineOptions* eo)
:   Environment3D(eo)
{
	m_CameraSystem->SetPosition(0.0f, 40.0f, 10.0f);
	
	bEnableShadows = true;

	bEnableDeferredShading = false;
	
	// Not using any reflections
	m_CameraSystem->SetRenderReflectionView( false );
	
	// Create a random plane
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
	
	//mp.pos = XMFLOAT3(0, 1, 0);
	mp.scale = XMFLOAT3(20, 1, 20);
	mp.material = MaterialManager::Instance()->GetMaterial("Stone");
	//mPlane = new ZShadeSandboxMesh::PlaneMesh(200.0f, 0.0f, m_D3DSystem, mp, true);
	mPlane = new ZShadeSandboxMesh::PlaneMesh(m_D3DSystem, mp, "Models\\plane01.txt");
	
	//mp.pos = XMFLOAT3(2, 2, 0);
	mp.material = MaterialManager::Instance()->GetMaterial("White");//Red
	mp.scale = XMFLOAT3(1, 1, 1);
	mp.pos = XMFLOAT3(0, 1, 0);
	mPickingSphere = new ZShadeSandboxMesh::SphereMesh(0.2f, 10, 10, m_D3DSystem, mp);

	float size = 10;
	float r = size / 100;
	//mp.pos = XMFLOAT3(4, 2, 0);
	mp.scale = XMFLOAT3(size, size, size);
	mp.pos = XMFLOAT3(50, 5, -45);
	mp.material = MaterialManager::Instance()->GetMaterial("Floor");//Floor
	mSphere = new ZShadeSandboxMesh::SphereMesh(r, size, size, m_D3DSystem, mp);
	
	//mp.pos = XMFLOAT3(0, 2, 2);
	mp.pos = XMFLOAT3(-45, 5, -30);
	mp.material = MaterialManager::Instance()->GetMaterial("Ice");//Ice
	mGeosphereMesh = new ZShadeSandboxMesh::GeosphereMesh(0.5f, 2, m_D3DSystem, mp);

	//mp.pos = XMFLOAT3(4, 2, 2);
	mp.pos = XMFLOAT3(30, 2, 60);
	mp.scale = XMFLOAT3(1, 1, 1);
	mp.material = MaterialManager::Instance()->GetMaterial("White");
	mCylindricalMesh = new ZShadeSandboxMesh::CylindricalMesh(0.5f, 0.5f, 3.0f, 10, 10, m_D3DSystem, mp);

	//mp.pos = XMFLOAT3(-2, 2, 0);
	mp.pos = XMFLOAT3(-50, 5, 40);
	mp.scale = XMFLOAT3(5, 5, 5);
	mp.material = MaterialManager::Instance()->GetMaterial("Wall");
	ZShadeSandboxMesh::CubeMesh* cubeMesh = new ZShadeSandboxMesh::CubeMesh(m_D3DSystem, mp, "Models\\cube.txt");
	vector<XMFLOAT3> positions;
	positions.push_back(XMFLOAT3(-50, 0, 40));
	positions.push_back(XMFLOAT3(20, 0, 0));
	positions.push_back(XMFLOAT3(80, 0, 10));
	positions.push_back(XMFLOAT3(-50, 0, 100));
	vector<XMFLOAT3> rotations;
	positions.push_back(XMFLOAT3(0, 0, 0));
	positions.push_back(XMFLOAT3(0, 0, 0));
	positions.push_back(XMFLOAT3(0, 0, 0));
	positions.push_back(XMFLOAT3(0, 0, 0));
	vector<XMFLOAT3> scales;
	positions.push_back(XMFLOAT3(10, 1, 10));
	positions.push_back(XMFLOAT3(1, 1, 1));
	positions.push_back(XMFLOAT3(1, 10, 1));
	positions.push_back(XMFLOAT3(20, 1, 20));
	cubeMesh->AddInstanceMatrix(positions, rotations, scales);

	//mp.pos = XMFLOAT3(1, 1, 0);
	//mp.material = MaterialManager::Instance()->GetMaterial("Floor");
	//mCircleMesh = new ZShadeSandboxMesh::CircleMesh(m_D3DSystem, 0.5f, mp);

	//m_SpawnedMeshContainer.push_back(mSphere);
	m_SpawnedMeshContainer.push_back(mGeosphereMesh);
	//m_SpawnedMeshContainer.push_back(mCylindricalMesh);
	//m_SpawnedMeshContainer.push_back(mCircleMesh);
	m_SpawnedMeshContainer.push_back(cubeMesh);
	
	// Enable the capsule lights for this scene
	//ZShadeSandboxLighting::LightManager::Instance()->ToggleCapsuleLights(true);
	
	// spaceCompound
	mSpaceCompound = new ZShadeSandboxMesh::OBJMesh(m_D3DSystem, m_GameDirectory3D);
	mSpaceCompound->Load("Models//spaceCompound.obj", false, false);
	mSpaceCompound->Scale(XMFLOAT3(2.3f, 1.3f, 2.3f));
	mSpaceCompound->Position(XMFLOAT3(100, -1, 100));
	
	// MaleLow.obj
	mHuman = new ZShadeSandboxMesh::OBJMesh(m_D3DSystem, m_GameDirectory3D);
	mHuman->Load("Models//MaleLow.obj", false, true);
	mHuman->Scale(XMFLOAT3(0.3f, 0.3f, 0.3f));
	mHuman->Position(XMFLOAT3(50, 15, 50));
	
	mFBXMesh = new ZShadeSandboxMesh::FBXMesh(m_D3DSystem, m_GameDirectory3D);
	mFBXMesh->Load("Models//tree4.fbx", true);
	mFBXMesh->Scale(XMFLOAT3(2.0f, 2.0f, 2.0f));
	mFBXMesh->Position(XMFLOAT3(0, 20, 0));
}
//===============================================================================================================================
ModelEnvironment::~ModelEnvironment()
{
	Shutdown();
}
//===============================================================================================================================
bool ModelEnvironment::Initialize()
{
	return true;
}
//===============================================================================================================================
void ModelEnvironment::Shutdown()
{
	Environment3D::Shutdown();
}
//===============================================================================================================================
void ModelEnvironment::Update()
{
	// Need to get toggle update time based on the amount of triangles rendered (This needs to be fixed)
	float dt = fFrameTime;
	
	mPlane->SetWireframe(bWireframeMode);
	
	// Uncomment if using the picking sphere
	mPickingSphere->SetWireframe(bWireframeMode);
	
	UpdateSpawnedMeshItems(dt);
	
	mSpaceCompound->SetWireframe(bWireframeMode);
	mHuman->SetWireframe(bWireframeMode);
	//mFBXMesh->SetWireframe(bWireframeMode);
	
	mSpaceCompound->SetFarPlane(mEngineOptions->fFarPlane);
	mHuman->SetFarPlane(mEngineOptions->fFarPlane);
	//mFBXMesh->SetFarPlane(mEngineOptions->fFarPlane);
	
	ToggleLightMeshWireframe(bWireframeMode);
	
	//// OBJ Mesh Collision test with camera
	//bool touchedAABB = false;
	//bool touchedSphere = false;
	//
	//if (mSpaceCompound->IntersectsAABB(eye))
	//{
	//	touchedAABB = true;
	//}
	//
	//if (mHuman->IntersectsAABB(eye))
	//{
	//	touchedAABB = true;
	//}
	//
	///*if (mSpaceCompound->IntersectsSphere(eye))
	//{
	//	touchedSphere = true;
	//}
	//
	//if (mHuman->IntersectsSphere(eye))
	//{
	//	touchedSphere = true;
	//}*/
	//
	//bToggleGBufferDebugging = touchedAABB;// && touchedSphere;

	// If the camera touched an OBJ Mesh then render everything in wireframe mode
	//bWireframeMode = touchedAABB && touchedSphere;
}
//===============================================================================================================================
void ModelEnvironment::Render()
{
	// Mouse Picking
	XMMATRIX world = XMMatrixIdentity();
	mPickingRay = m_CameraSystem->PickingRay(mLastMousePos.x, mLastMousePos.y, world);
	if (mPickingRay != NULL)
	{
		if (bLeftMouseDown)
		{
			bLeftMouseDown = false;
			bool hit = false;
			XMFLOAT3 hitPoint;
			ZShadeSandboxMath::Ray ray = *mPickingRay;
			// Picking Test goes here
			if (hit)
			{
				// Place the sphere at the point of intersection
				mPickingSphere->Position() = hitPoint;
			}
		}
	}
	
	if (!bWireframeMode)
	{
		m_D3DSystem->TurnOnCulling();
	}
	
	//Quick wire mode to view the objects underlying triangles quickly
	if( Quickwire() )
	{
		mPlane->SetWireframe(true);
		
		ToggleSpawnedMeshItemsWireframe(true);
		
		mPickingSphere->SetWireframe(true);
		
		mSpaceCompound->SetWireframe(true);
		mHuman->SetWireframe(true);
		//mFBXMesh->SetWireframe(true);
		
		ToggleLightMeshWireframe(true);
		
		m_D3DSystem->TurnOnWireframe();
	}
	
	ZShadeSandboxMesh::MeshRenderParameters mrp;
	mrp.camera = m_CameraSystem;
	mrp.light = mDirLight1;
	
	RenderSpawnedMeshItems(mrp);

	mrp.useInstancing = false;
	
	//mFBXMesh->Render(mrp);

	// Render the plane in tessellation mode
	//m_D3DSystem->TurnOnWireframe();
	//mPlane->SetWireframe(true);
	//mrp.fTessellationFactor = m_TessFactor;
	//mrp.bTessellate = true;
	//mrp.renderType = ZShadeSandboxMesh::ERenderType::e3ControlPointPatchList;
	//mPlane->EnableShadowMap(bEnableShadows);
	//mPlane->SetShadowMapSRV(mShadowTexture->SRView);
	//mPlane->SetSSAOMapSRV(0);
	mPlane->Render(mrp);
	
	// Uncomment if using the picking sphere
	//mrp.bTessellate = false;
	//mrp.rt = RenderType::eTriangleList;
	//mPickingSphere->EnableShadowMap(bEnableShadows);
	//mPickingSphere->SetShadowMapSRV(mShadowTexture->SRView);
	//mPickingSphere->SetSSAOMapSRV(0);
	mPickingSphere->Render(mrp);
	
	//ZShadeSandboxMesh::OBJMeshRenderParameters omrp;
	//omrp.camera = m_CameraSystem.get();
	//omrp.renderType = ZShadeSandboxMesh::ERenderType::eTriangleList;
	//omrp.renderType = ZShadeSandboxMesh::ERenderType::e3ControlPointPatchList;
	//omrp.maxTess = 64.0f;
	//omrp.minTess = 0.0f;
	//omrp.maxTessDist = 500.0f;
	//omrp.minTessDist = 20.0f;
	//omrp.tessellate = true;
	mSpaceCompound->Render(mrp);
	mHuman->Render(mrp);
	
	RenderLightMesh(mrp);
	
	m_D3DSystem->TurnOffCulling();
}
//===============================================================================================================================
void ModelEnvironment::RenderDeferred()
{
	if (!bWireframeMode)
	{
		mPlane->SetWireframe(true);
		
		ToggleSpawnedMeshItemsWireframe(true);
		
		mPickingSphere->SetWireframe(true);
		
		mSpaceCompound->SetWireframe(true);
		mHuman->SetWireframe(true);
		//mFBXMesh->SetWireframe(true);
		
		ToggleLightMeshWireframe(true);
		
		m_D3DSystem->TurnOnCulling();
	}

	ZShadeSandboxMesh::MeshRenderParameters mrp;
	mrp.camera = m_CameraSystem;
	mrp.renderDeferred = true;
	mrp.light = mDirLight1;
	
	mPlane->Render(mrp);

	RenderSpawnedMeshItems(mrp);

	mPickingSphere->Render(mrp);
	
	mSpaceCompound->Render(mrp);
	mHuman->Render(mrp);
	//mFBXMesh->Render(mrp);

	RenderLightMesh(mrp);
}
//===============================================================================================================================
void ModelEnvironment::RenderShadowMap()
{
	mDirLight1->Update();
	
	ZShadeSandboxMesh::MeshRenderParameters mrp;
	mrp.camera = m_CameraSystem;
	mrp.light = mDirLight1;
	mrp.shadowMap = true;
	
	//mPlane->Render(mrp);
	
	RenderSpawnedMeshItems(mrp);
	
	mPickingSphere->Render(mrp);
	
	RenderLightMesh(mrp);
}
//===============================================================================================================================