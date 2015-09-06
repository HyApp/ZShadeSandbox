#include "Room.h"
#include "MaterialManager.h"
#include "Vertex.h"
//==================================================================================================================================
//==================================================================================================================================
Room::Room(D3D* d3d)
:	m_D3DSystem(d3d)
,	bRoomBuilt(false)
,	bHasWallMaterial(false)
,	bHasCeilingMaterial(false)
,	bHasFloorMaterial(false)
,	mWallMaterial(0)
,	mCelingMaterial(0)
,	mFloorMaterial(0)
,	mWall0(0)
,	mWall1(0)
,	mWall2(0)
,	mWall3(0)
,	mWall(0)
,	mCeiling(0)
,	mFloor(0)
{
	m_EngineOptions = d3d->GetEngineOptions();
}
//==================================================================================================================================
void Room::Render(ZShadeSandboxMesh::MeshRenderParameters mrp)
{
	if (!bRoomBuilt) return;
	
	// We need to enable instancing so that the entire room created from the wall map will be seen
	mrp.useInstancing = true;
	
	mrp.specifyWorld = true;
	mrp.world = XMMatrixTranspose(XMMatrixIdentity());

	//
	// Render Floor
	//
	
	// Render the actual mesh of the floor with instancing enabled
	mFloor->Render(mrp);
	
	//
	// Render Ceiling
	//
	
	// Render the actual mesh of the ceiling with instancing enabled
	mCeiling->Render(mrp);
	
	//
	// Render Walls
	//
	
	// Render the actual mesh of the walls with instancing enabled
	mWall0->Render(mrp);
	mWall1->Render(mrp);
	mWall2->Render(mrp);
	mWall3->Render(mrp);
	//mWall->Render(mrp);
}
//==================================================================================================================================
void Room::SetWireframe(bool wire)
{
	if (mWall0) mWall0->Wireframe() = wire;
	if (mWall1) mWall1->Wireframe() = wire;
	if (mWall2) mWall2->Wireframe() = wire;
	if (mWall3) mWall3->Wireframe() = wire;
	if (mWall) mWall->Wireframe() = wire;
	if (mCeiling) mCeiling->Wireframe() = wire;
	if (mFloor) mFloor->Wireframe() = wire;
}
//==================================================================================================================================
void Room::SetWallMapSize(int w, int h)
{
	mWallMap.resize(w, h);
}
//==================================================================================================================================
void Room::AddWallMapValue(int x, int y, int value)
{
	mWallMap.insert(x, y, value);
}
//==================================================================================================================================
void Room::AssignWallMaterial(std::string wallTexture)
{
	mWallMaterial = MaterialManager::Instance()->GetMaterial(wallTexture);
	bHasWallMaterial = true;
}
//==================================================================================================================================
void Room::AssignCeilingMaterial(std::string ceilingTexture)
{
	mCelingMaterial = MaterialManager::Instance()->GetMaterial(ceilingTexture);
	bHasCeilingMaterial = true;
}
//==================================================================================================================================
void Room::AssignFloorMaterial(std::string floorTexture)
{
	mFloorMaterial = MaterialManager::Instance()->GetMaterial(floorTexture);
	bHasFloorMaterial = true;
}
//==================================================================================================================================
void Room::BuildInstancePositions()
{
	float xPos, zPos;
	int i, j;

	float scaleWidth = 4;
	float scaleHeight = 2;
	
	int width = mWallMap.width();
	int height = mWallMap.height();
	
	// Need to use instancing to be able to render multiple of each at a different position
	vector<XMFLOAT3> ceilingPositions;
	vector<XMFLOAT3> floorPositions;
	vector<XMFLOAT3> wallPositions;
	
	vector<XMFLOAT3> ceilingRotations;
	vector<XMFLOAT3> floorRotations;
	vector<XMFLOAT3> wallRotations;

	vector<XMFLOAT3> ceilingScales;
	vector<XMFLOAT3> floorScales;
	vector<XMFLOAT3> wallScales;

	for( i = 0, zPos = 5; i < height; i++ )
	{
		xPos = -5;
		
		for( j = 0; j < width; j++ )
		{
			// For each floor and ceiling piece get the position
			//if (mWallMap(j, i) == 0)
			//{
			//	floorPositions.push_back(XMFLOAT3(xPos, -1, zPos));
			//	ceilingPositions.push_back(XMFLOAT3(xPos, 0, zPos));
			//}
			
			// Every spot on the wall map will have a floor and a ceiling
			
			floorPositions.push_back(XMFLOAT3(xPos, -1, zPos));
			floorRotations.push_back(XMFLOAT3(0, 0, 0));
			floorScales.push_back(XMFLOAT3(scaleWidth, scaleHeight, scaleWidth));
			
			ceilingPositions.push_back(XMFLOAT3(xPos, 0, zPos));
			ceilingRotations.push_back(XMFLOAT3(0, 0, 0));
			ceilingScales.push_back(XMFLOAT3(scaleWidth, scaleHeight, scaleWidth));
			
			// For each wall piece get the position
			if (mWallMap(j, i) != 0)
			{
				wallPositions.push_back(XMFLOAT3(xPos, 0, zPos));
				wallRotations.push_back(XMFLOAT3(0, 0, 0));
				wallScales.push_back(XMFLOAT3(scaleWidth, scaleHeight, scaleWidth));
			}
			
			xPos += 2;
		}
		
		zPos -= 2;
	}
	
	// Build the floor and ceiling instance positions
	mFloor->Attributes()->AddInstanceMatrix(floorPositions, floorRotations, floorScales);
	mCeiling->Attributes()->AddInstanceMatrix(ceilingPositions, ceilingRotations, ceilingScales);
	mWall->Attributes()->AddInstanceMatrix(wallPositions, wallRotations, wallScales);
	mWall0->Attributes()->AddInstanceMatrix(wallPositions, wallRotations, wallScales);
	mWall1->Attributes()->AddInstanceMatrix(wallPositions, wallRotations, wallScales);
	mWall2->Attributes()->AddInstanceMatrix(wallPositions, wallRotations, wallScales);
	mWall3->Attributes()->AddInstanceMatrix(wallPositions, wallRotations, wallScales);
	
	bRoomBuilt = true;
}
//==================================================================================================================================
void Room::BuildPolygonBlueprints()
{
	//
	// Build walls
	//
	
	{
		vector<ZShadeSandboxMesh::VertexNormalTex> vertices(24);
		
		//
		// Front facing wall
		//

		// First Triangle
		vertices[0] = ZShadeSandboxMesh::VertexUtil::LoadVertexNormalTex(-1,  1, -1, 1, 1, 1, 0, 0);
		vertices[1] = ZShadeSandboxMesh::VertexUtil::LoadVertexNormalTex( 1,  1, -1, 1, 1, 1, 1, 0);
		vertices[2] = ZShadeSandboxMesh::VertexUtil::LoadVertexNormalTex(-1, -1, -1, 1, 1, 1, 0, 1);

		// Second Triangle
		vertices[3] = ZShadeSandboxMesh::VertexUtil::LoadVertexNormalTex( 1,  1, -1, 1, 1, 1, 1, 0);
		vertices[4] = ZShadeSandboxMesh::VertexUtil::LoadVertexNormalTex( 1, -1, -1, 1, 1, 1, 1, 1);
		vertices[5] = ZShadeSandboxMesh::VertexUtil::LoadVertexNormalTex(-1, -1, -1, 1, 1, 1, 0, 1);
		
		//
		// Left facing wall
		//
		
		// First Triangle
		vertices[6] = ZShadeSandboxMesh::VertexUtil::LoadVertexNormalTex(1,  1, -1, 1, 1, 1, 0, 0);
		vertices[7] = ZShadeSandboxMesh::VertexUtil::LoadVertexNormalTex(1,  1,  1, 1, 1, 1, 1, 0);
		vertices[8] = ZShadeSandboxMesh::VertexUtil::LoadVertexNormalTex(1, -1, -1, 1, 1, 1, 0, 1);

		// Second Triangle
		vertices[9 ] = ZShadeSandboxMesh::VertexUtil::LoadVertexNormalTex(1,  1,  1, 1, 1, 1, 1, 0);
		vertices[10] = ZShadeSandboxMesh::VertexUtil::LoadVertexNormalTex(1, -1,  1, 1, 1, 1, 1, 1);
		vertices[11] = ZShadeSandboxMesh::VertexUtil::LoadVertexNormalTex(1, -1, -1, 1, 1, 1, 0, 1);
		
		//
		// Right facing wall
		//
		
		// First Triangle
		vertices[12] = ZShadeSandboxMesh::VertexUtil::LoadVertexNormalTex(-1,  1, -1, 1, 1, 1, 1, 0);
		vertices[13] = ZShadeSandboxMesh::VertexUtil::LoadVertexNormalTex(-1, -1,  1, 1, 1, 1, 0, 1);
		vertices[14] = ZShadeSandboxMesh::VertexUtil::LoadVertexNormalTex(-1,  1,  1, 1, 1, 1, 0, 0);
		
		// Second Triangle
		vertices[15] = ZShadeSandboxMesh::VertexUtil::LoadVertexNormalTex(-1,  1, -1, 1, 1, 1, 1, 0);
		vertices[16] = ZShadeSandboxMesh::VertexUtil::LoadVertexNormalTex(-1, -1, -1, 1, 1, 1, 1, 1);
		vertices[17] = ZShadeSandboxMesh::VertexUtil::LoadVertexNormalTex(-1, -1,  1, 1, 1, 1, 0, 1);
		
		//
		// Back facing wall
		//
		
		// First Triangle
		vertices[18] = ZShadeSandboxMesh::VertexUtil::LoadVertexNormalTex( 1,  1, 1, 1, 1, 1, 0, 0);
		vertices[19] = ZShadeSandboxMesh::VertexUtil::LoadVertexNormalTex(-1,  1, 1, 1, 1, 1, 1, 0);
		vertices[20] = ZShadeSandboxMesh::VertexUtil::LoadVertexNormalTex( 1, -1, 1, 1, 1, 1, 0, 1);
		
		// Second Triangle
		vertices[21] = ZShadeSandboxMesh::VertexUtil::LoadVertexNormalTex( 1, -1, 1, 1, 1, 1, 0, 1);
		vertices[22] = ZShadeSandboxMesh::VertexUtil::LoadVertexNormalTex(-1, -1, 1, 1, 1, 1, 1, 1);
		vertices[23] = ZShadeSandboxMesh::VertexUtil::LoadVertexNormalTex(-1,  1, 1, 1, 1, 1, 1, 0);
		
		ConstructWallPolygon(vertices);
		
		// Create 4 subset groups for the polygon which refer to the starting index to draw each side of the wall
		std::vector<int> subsetStartIndex(1);
		subsetStartIndex[0] = 0;
		//subsetStartIndex[1] = 4;
		//subsetStartIndex[2] = 8;
		//subsetStartIndex[3] = 12;
		//subsetStartIndex[4] = 16;
		//subsetStartIndex[5] = 20;
		mWall->CreateSubsets(subsetStartIndex);
	}
	
	{
		vector<ZShadeSandboxMesh::VertexNormalTex> vertices(6);

		//
		// Front facing wall
		//

		// First Triangle
		vertices[0] = ZShadeSandboxMesh::VertexUtil::LoadVertexNormalTex(-1,  1, -1, 1, 1, 1, 0, 0);
		vertices[1] = ZShadeSandboxMesh::VertexUtil::LoadVertexNormalTex( 1,  1, -1, 1, 1, 1, 1, 0);
		vertices[2] = ZShadeSandboxMesh::VertexUtil::LoadVertexNormalTex(-1, -1, -1, 1, 1, 1, 0, 1);

		// Second Triangle
		vertices[3] = ZShadeSandboxMesh::VertexUtil::LoadVertexNormalTex( 1,  1, -1, 1, 1, 1, 1, 0);
		vertices[4] = ZShadeSandboxMesh::VertexUtil::LoadVertexNormalTex( 1, -1, -1, 1, 1, 1, 1, 1);
		vertices[5] = ZShadeSandboxMesh::VertexUtil::LoadVertexNormalTex(-1, -1, -1, 1, 1, 1, 0, 1);

		ConstructWall0Polygon(vertices);
	}
	
	{
		vector<ZShadeSandboxMesh::VertexNormalTex> vertices(6);
	
		//
		// Left facing wall
		//
		
		// First Triangle
		vertices[0] = ZShadeSandboxMesh::VertexUtil::LoadVertexNormalTex(1,  1, -1, 1, 1, 1, 0, 0);
		vertices[1] = ZShadeSandboxMesh::VertexUtil::LoadVertexNormalTex(1,  1,  1, 1, 1, 1, 1, 0);
		vertices[2] = ZShadeSandboxMesh::VertexUtil::LoadVertexNormalTex(1, -1, -1, 1, 1, 1, 0, 1);

		// Second Triangle
		vertices[3] = ZShadeSandboxMesh::VertexUtil::LoadVertexNormalTex(1,  1,  1, 1, 1, 1, 1, 0);
		vertices[4] = ZShadeSandboxMesh::VertexUtil::LoadVertexNormalTex(1, -1,  1, 1, 1, 1, 1, 1);
		vertices[5] = ZShadeSandboxMesh::VertexUtil::LoadVertexNormalTex(1, -1, -1, 1, 1, 1, 0, 1);

		ConstructWall1Polygon(vertices);
	}

	{
		vector<ZShadeSandboxMesh::VertexNormalTex> vertices(6);
		
		//
		// Right facing wall
		//
		
		// First Triangle
		vertices[0] = ZShadeSandboxMesh::VertexUtil::LoadVertexNormalTex(-1,  1, -1, 1, 1, 1, 1, 0);
		vertices[1] = ZShadeSandboxMesh::VertexUtil::LoadVertexNormalTex(-1, -1,  1, 1, 1, 1, 0, 1);
		vertices[2] = ZShadeSandboxMesh::VertexUtil::LoadVertexNormalTex(-1,  1,  1, 1, 1, 1, 0, 0);
		
		// Second Triangle
		vertices[3] = ZShadeSandboxMesh::VertexUtil::LoadVertexNormalTex(-1,  1, -1, 1, 1, 1, 1, 0);
		vertices[4] = ZShadeSandboxMesh::VertexUtil::LoadVertexNormalTex(-1, -1, -1, 1, 1, 1, 1, 1);
		vertices[5] = ZShadeSandboxMesh::VertexUtil::LoadVertexNormalTex(-1, -1,  1, 1, 1, 1, 0, 1);

		ConstructWall2Polygon(vertices);
	}
	
	{
		vector<ZShadeSandboxMesh::VertexNormalTex> vertices(6);
		
		//
		// Back facing wall
		//
		
		// First Triangle
		vertices[0] = ZShadeSandboxMesh::VertexUtil::LoadVertexNormalTex( 1,  1, 1, 1, 1, 1, 0, 0);
		vertices[1] = ZShadeSandboxMesh::VertexUtil::LoadVertexNormalTex(-1,  1, 1, 1, 1, 1, 1, 0);
		vertices[2] = ZShadeSandboxMesh::VertexUtil::LoadVertexNormalTex( 1, -1, 1, 1, 1, 1, 0, 1);
		
		// Second Triangle
		vertices[3] = ZShadeSandboxMesh::VertexUtil::LoadVertexNormalTex( 1, -1, 1, 1, 1, 1, 0, 1);
		vertices[4] = ZShadeSandboxMesh::VertexUtil::LoadVertexNormalTex(-1, -1, 1, 1, 1, 1, 1, 1);
		vertices[5] = ZShadeSandboxMesh::VertexUtil::LoadVertexNormalTex(-1,  1, 1, 1, 1, 1, 1, 0);
		
		ConstructWall3Polygon(vertices);
	}
	
	//
	// Ceiling
	//
	{
		vector<ZShadeSandboxMesh::VertexNormalTex> vertices(6);
		
		// First Triangle
		vertices[0] = ZShadeSandboxMesh::VertexUtil::LoadVertexNormalTex( 1, 1,  1, 1, 1, 1, 1, 1);
		vertices[1] = ZShadeSandboxMesh::VertexUtil::LoadVertexNormalTex(-1, 1,  1, 1, 1, 1, 0, 1);
		vertices[2] = ZShadeSandboxMesh::VertexUtil::LoadVertexNormalTex(-1, 1, -1, 1, 1, 1, 0, 0);
		
		// Second Triangle
		vertices[3] = ZShadeSandboxMesh::VertexUtil::LoadVertexNormalTex(-1, 1, -1, 1, 1, 1, 0, 0);
		vertices[4] = ZShadeSandboxMesh::VertexUtil::LoadVertexNormalTex( 1, 1, -1, 1, 1, 1, 1, 0);
		vertices[5] = ZShadeSandboxMesh::VertexUtil::LoadVertexNormalTex( 1, 1,  1, 1, 1, 1, 1, 1);
		
		ConstructCeilingPolygon(vertices);
	}
	
	//
	// Floor
	//
	
	{
		vector<ZShadeSandboxMesh::VertexNormalTex> vertices(6);
		
		// First Triangle
		vertices[0] = ZShadeSandboxMesh::VertexUtil::LoadVertexNormalTex(-1, 0,  1, 1, 1, 1, 0, 0);
		vertices[1] = ZShadeSandboxMesh::VertexUtil::LoadVertexNormalTex( 1, 0,  1, 1, 1, 1, 1, 0);
		vertices[2] = ZShadeSandboxMesh::VertexUtil::LoadVertexNormalTex(-1, 0, -1, 1, 1, 1, 0, 1);
		
		// Second Triangle
		vertices[3] = ZShadeSandboxMesh::VertexUtil::LoadVertexNormalTex( 1, 0,  1, 1, 1, 1, 1, 0);
		vertices[4] = ZShadeSandboxMesh::VertexUtil::LoadVertexNormalTex( 1, 0, -1, 1, 1, 1, 1, 1);
		vertices[5] = ZShadeSandboxMesh::VertexUtil::LoadVertexNormalTex(-1, 0, -1, 1, 1, 1, 0, 1);
		
		ConstructFloorPolygon(vertices);
	}
}
//==================================================================================================================================
void Room::ConstructWall0Polygon(vector<ZShadeSandboxMesh::VertexNormalTex> verts)
{
	if (!bHasWallMaterial) return;
	
	ZShadeSandboxMath::Polygon* polygon = new ZShadeSandboxMath::Polygon();
	
	polygon->Create(verts);
	
	polygon->Attributes()->mD3D = m_D3DSystem;
	
	mWall0 = new ZShadeSandboxMesh::PolygonMesh(polygon, mWallMaterial);
	mWall0->Position() = XMFLOAT3(0, 0, 0);
	mWall0->Scale() = XMFLOAT3(1, 1, 1);
	
	mWall0->BuildVertexBuffer(ZShadeSandboxMesh::EVertexType::Type::VT_NormalTex);
	mWall0->BuildIndexBuffer();
}
//==================================================================================================================================
void Room::ConstructWall1Polygon(vector<ZShadeSandboxMesh::VertexNormalTex> verts)
{
	if (!bHasWallMaterial) return;

	ZShadeSandboxMath::Polygon* polygon = new ZShadeSandboxMath::Polygon();

	polygon->Create(verts);

	polygon->Attributes()->mD3D = m_D3DSystem;

	mWall1 = new ZShadeSandboxMesh::PolygonMesh(polygon, mWallMaterial);
	mWall1->Position() = XMFLOAT3(0, 0, 0);
	mWall1->Scale() = XMFLOAT3(1, 1, 1);

	mWall1->BuildVertexBuffer(ZShadeSandboxMesh::EVertexType::Type::VT_NormalTex);
	mWall1->BuildIndexBuffer();
}
//==================================================================================================================================
void Room::ConstructWall2Polygon(vector<ZShadeSandboxMesh::VertexNormalTex> verts)
{
	if (!bHasWallMaterial) return;

	ZShadeSandboxMath::Polygon* polygon = new ZShadeSandboxMath::Polygon();

	polygon->Create(verts);

	polygon->Attributes()->mD3D = m_D3DSystem;

	mWall2 = new ZShadeSandboxMesh::PolygonMesh(polygon, mWallMaterial);
	mWall2->Position() = XMFLOAT3(0, 0, 0);
	mWall2->Scale() = XMFLOAT3(1, 1, 1);

	mWall2->BuildVertexBuffer(ZShadeSandboxMesh::EVertexType::Type::VT_NormalTex);
	mWall2->BuildIndexBuffer();
}
//==================================================================================================================================
void Room::ConstructWall3Polygon(vector<ZShadeSandboxMesh::VertexNormalTex> verts)
{
	if (!bHasWallMaterial) return;

	ZShadeSandboxMath::Polygon* polygon = new ZShadeSandboxMath::Polygon();

	polygon->Create(verts);

	polygon->Attributes()->mD3D = m_D3DSystem;

	mWall3 = new ZShadeSandboxMesh::PolygonMesh(polygon, mWallMaterial);
	mWall3->Position() = XMFLOAT3(0, 0, 0);
	mWall3->Scale() = XMFLOAT3(1, 1, 1);

	mWall3->BuildVertexBuffer(ZShadeSandboxMesh::EVertexType::Type::VT_NormalTex);
	mWall3->BuildIndexBuffer();
}
//==================================================================================================================================
void Room::ConstructWallPolygon(vector<ZShadeSandboxMesh::VertexNormalTex> verts)
{
	if (!bHasWallMaterial) return;
	
	ZShadeSandboxMath::Polygon* polygon = new ZShadeSandboxMath::Polygon();
	
	polygon->Create(verts);
	
	polygon->Attributes()->mD3D = m_D3DSystem;
	
	mWall = new ZShadeSandboxMesh::PolygonMesh(polygon, mWallMaterial);
	mWall->Position() = XMFLOAT3(0, 0, 0);
	mWall->Scale() = XMFLOAT3(1, 1, 1);
	
	mWall->BuildVertexBuffer(ZShadeSandboxMesh::EVertexType::Type::VT_NormalTex);
	mWall->BuildIndexBuffer();
}
//==================================================================================================================================
void Room::ConstructCeilingPolygon(vector<ZShadeSandboxMesh::VertexNormalTex> verts)
{
	if (!bHasCeilingMaterial) return;
	
	ZShadeSandboxMath::Polygon* polygon = new ZShadeSandboxMath::Polygon();
	
	polygon->Create(verts);
	
	polygon->Attributes()->mD3D = m_D3DSystem;
	
	mCeiling = new ZShadeSandboxMesh::PolygonMesh(polygon, mCelingMaterial);
	mCeiling->Position() = XMFLOAT3(0, 0, 0);
	mCeiling->Scale() = XMFLOAT3(1, 1, 1);
	
	mCeiling->BuildVertexBuffer(ZShadeSandboxMesh::EVertexType::Type::VT_NormalTex);
	mCeiling->BuildIndexBuffer();
}
//==================================================================================================================================
void Room::ConstructFloorPolygon(vector<ZShadeSandboxMesh::VertexNormalTex> verts)
{
	if (!bHasFloorMaterial) return;
	
	ZShadeSandboxMath::Polygon* polygon = new ZShadeSandboxMath::Polygon();
	
	polygon->Create(verts);
	
	polygon->Attributes()->mD3D = m_D3DSystem;
	
	mFloor = new ZShadeSandboxMesh::PolygonMesh(polygon, mFloorMaterial);
	mFloor->Position() = XMFLOAT3(0, 0, 0);
	mFloor->Scale() = XMFLOAT3(1, 1, 1);
	
	mFloor->BuildVertexBuffer(ZShadeSandboxMesh::EVertexType::Type::VT_NormalTex);
	mFloor->BuildIndexBuffer();
}
//==================================================================================================================================
