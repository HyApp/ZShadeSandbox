//==================================================================================================================================
// Room.h
//
// http://codentronix.com/2011/06/18/a-simple-3d-room-made-with-directx-and-c/
// This class will represent a 3D room with 4 walls so that an object or entity can be inside it.
//==================================================================================================================================
// History
//
// -Created on 8/18/2015 by Dustin Watson
//==================================================================================================================================
#ifndef __ROOM_H
#define __ROOM_H
//==================================================================================================================================
//==================================================================================================================================

//
// Includes
//

#include "D3D.h"
#include "ZMath.h"
#include "Camera.h"
#include "MemoryMgr.h"
#include "PolygonMesh.h"
#include "ShaderMaterial.h"

/*
Wall Map Example:

#define WORLD_Y 9
#define WORLD_X 10

static int iWallMap[WORLD_Y][WORLD_X] =
{
	1,1,1,1,1,1,1,1,1,1,
	1,0,0,0,0,0,0,0,0,1,
	1,0,1,0,0,0,0,1,0,1,
	1,0,1,0,0,0,0,1,0,1,
	1,0,0,0,0,0,0,1,0,1,
	1,0,1,0,0,0,0,1,0,1,
	1,0,1,0,1,1,1,1,0,1,
	1,0,0,0,0,0,0,0,0,1,
	1,1,1,1,1,1,1,1,1,1
};
*/

//==================================================================================================================================
//==================================================================================================================================
class Room
{
public:
	
	Room(D3D* d3d);
	
	void Render(ZShadeSandboxMesh::MeshRenderParameters mrp);
	
	void SetWallMapSize(int w, int h);
	void AddWallMapValue(int x, int y, int value);
	
	void AssignWallMaterial(std::string wallTexture);
	void AssignCeilingMaterial(std::string ceilingTexture);
	void AssignFloorMaterial(std::string floorTexture);
	
	void BuildPolygonBlueprints();
	void BuildInstancePositions();
	
	void SetWireframe(bool wire);
	
private:
	
	void ConstructWall0Polygon(vector<ZShadeSandboxMesh::VertexNormalTex> verts);
	void ConstructWall1Polygon(vector<ZShadeSandboxMesh::VertexNormalTex> verts);
	void ConstructWall2Polygon(vector<ZShadeSandboxMesh::VertexNormalTex> verts);
	void ConstructWall3Polygon(vector<ZShadeSandboxMesh::VertexNormalTex> verts);
	void ConstructWallPolygon(vector<ZShadeSandboxMesh::VertexNormalTex> verts);
	void ConstructCeilingPolygon(vector<ZShadeSandboxMesh::VertexNormalTex> verts);
	void ConstructFloorPolygon(vector<ZShadeSandboxMesh::VertexNormalTex> verts);
	
private:
	
	bool bRoomBuilt;
	bool bHasWallMaterial;
	bool bHasCeilingMaterial;
	bool bHasFloorMaterial;
	
	SmartArray2D<int> mWallMap;
	
	ZShadeSandboxLighting::ShaderMaterial* mWallMaterial;
	ZShadeSandboxLighting::ShaderMaterial* mCelingMaterial;
	ZShadeSandboxLighting::ShaderMaterial* mFloorMaterial;
	
	ZShadeSandboxMesh::PolygonMesh* mWall0;
	ZShadeSandboxMesh::PolygonMesh* mWall1;
	ZShadeSandboxMesh::PolygonMesh* mWall2;
	ZShadeSandboxMesh::PolygonMesh* mWall3;
	
	ZShadeSandboxMesh::PolygonMesh* mWall;
	ZShadeSandboxMesh::PolygonMesh* mCeiling;
	ZShadeSandboxMesh::PolygonMesh* mFloor;
	
	D3D* m_D3DSystem;
	EngineOptions* m_EngineOptions;
};
//==================================================================================================================================
//==================================================================================================================================
#endif//__ROOM_H