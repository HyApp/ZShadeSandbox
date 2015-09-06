#include "RoomEnvironment.h"
#include "ShapeContact.h"
#include "MaterialManager.h"
#include "ShaderMaterial.h"
#include "XMMath3.h"
#include "Vertex.h"
//===============================================================================================================================
//===============================================================================================================================
RoomEnvironment::RoomEnvironment(EngineOptions* eo)
:   Environment3D(eo)
{
	Initialize();
}
//===============================================================================================================================
RoomEnvironment::~RoomEnvironment()
{
	Shutdown();
}
//===============================================================================================================================
bool RoomEnvironment::Initialize()
{
	#define WORLD_Y 10
	#define WORLD_X 10

	static int iWallMap[WORLD_X][WORLD_Y] =
	{
		1,1,1,1,1,1,1,1,1,1,
		1,0,0,0,0,0,0,0,0,1,
		1,0,1,0,0,1,1,1,0,1,
		1,0,1,0,0,0,0,1,0,1,
		1,0,0,0,0,0,0,1,0,1,
		1,0,0,0,0,0,0,0,0,1,
		1,0,1,0,0,0,0,0,0,1,
		1,0,1,0,0,0,0,0,0,1,
		1,0,0,0,0,0,0,0,0,1,
		1,1,1,1,1,0,1,1,1,1
	};

	// Position the camera around the polygon
	m_CameraSystem->SetPosition(5.0f, 0.0f, 5.0f);
	
	bEnableReflections = false;
	bEnableRefractions = false;
	
	m_CameraSystem->SetRenderReflectionView(bEnableReflections);
	
	fSeaLevel = -5.0f;
	
	mRoom = new Room(m_D3DSystem);
	
	mRoom->SetWallMapSize(WORLD_X, WORLD_Y);
	
	for (int y = 0; y < WORLD_Y; y++)
	{
		for (int x = 0; x < WORLD_X; x++)
		{
			mRoom->AddWallMapValue(x, y, iWallMap[x][y]);
		}
	}
	
	mRoom->AssignWallMaterial("Wall");
	mRoom->AssignCeilingMaterial("Stone");
	mRoom->AssignFloorMaterial("Floor");
	
	mRoom->BuildPolygonBlueprints();
	mRoom->BuildInstancePositions();
	
	return true;
}
//===============================================================================================================================
void RoomEnvironment::Shutdown()
{
	Environment3D::Shutdown();
}
//===============================================================================================================================
void RoomEnvironment::Update()
{
	mRoom->SetWireframe(bWireframeMode);
	UpdateSpawnedMeshItems(fFrameTime);
	ToggleLightMeshWireframe(bWireframeMode);
}
//===============================================================================================================================
void RoomEnvironment::Render()
{
	if( Quickwire() )
	{
		mRoom->SetWireframe(true);
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
	
	mRoom->Render(mrp);
	
	RenderSpawnedMeshItems(mrp);
	RenderLightMesh(mrp);
}
//===============================================================================================================================
void RoomEnvironment::RenderDeferred()
{
	if( Quickwire() )
	{
		mRoom->SetWireframe(true);
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
	
	mRoom->Render(mrp);
	
	RenderSpawnedMeshItems(mrp);
	RenderLightMesh(mrp);
}
//===============================================================================================================================