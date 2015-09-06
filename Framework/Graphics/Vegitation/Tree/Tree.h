//==================================================================================================================================
// Tree.h
//
// This class can render a forest or a single tree
//==================================================================================================================================
// History
//
// -Created on 8/14/2015 by Dustin Watson
//==================================================================================================================================
#ifndef __TREE_H
#define __TREE_H
//==================================================================================================================================
//==================================================================================================================================

//
// Includes
//

#include "D3D.h"
#include "TreeParameters.h"
#include "TreeShader.h"
#include "OBJMesh.h"
#include "MeshAttributes.h"

//==================================================================================================================================
//==================================================================================================================================
class Tree
{
public:
	
	Tree(D3D* d3d, TreeParameters tp);
	
	void Init();
	
	void Render(ZShadeSandboxMesh::MeshRenderParameters mrp, XMFLOAT4 frustumPlanes[6]);
	
	void SetWireframe(bool wire);
	
	// Need to call this only once in the render loop when the tree is loaded
	// so that the billboard texture can be created for the mesh.
	void SaveBillboardTexture(ZShadeSandboxMesh::MeshRenderParameters mrp);
	
	int TreeCount() const { return mTreeParameters->treeCount; }
	int TreesRendered() const { return iNumTreesToDraw; }
	int BillboardsRendered() const { return iNumBillboardTreesToDraw; }
	int TriangleCount() const;
	int RenderedTriangleCount() const;
	
	string TreeCountString() const;
	string TreesRenderedString() const;
	string BillboardsRenderedString() const;
	string TriangleCountString() const;
	string RenderedTriangleCountString() const;
	
	string TreeStats();
	
private:
	
	void UpdateBillboardInstances(XMFLOAT3 cameraPos, vector<ZShadeSandboxMesh::InstanceMatrix>& instanceMatrix);
	void CalculateAABB();
	void CullAABB(XMFLOAT4 frustumPlanes[6], vector<ZShadeSandboxMesh::InstanceMatrix> instanceMatrix);
	void CreateLeaves();
	void CreateBillboards();
	void CreateMesh();
	
	// Leaf data
	ID3D11ShaderResourceView* mLeafTexture;
	ZShadeSandboxMesh::MeshAttributes* mLeafAttributes;
	int iNumLeafTrianglesToDraw;
	
	// Billboard data
	ID3D11ShaderResourceView* mBillboardTexture;
	ZShadeSandboxMesh::MeshAttributes* mBillboardAttributes;
	int iNumBillboardTreesToDraw;
	int iNumBillboardTrianglesToDraw;
	
	// Actual Tree is an obj mesh
	ZShadeSandboxMesh::OBJMesh* mTree;
	int iNumTreesToDraw;
	int iNumTreeTrianglesToDraw;
	
	// The Tree OBJMesh already has an AABB so I need to start using it
	ZShadeSandboxMath::AABB mTreeAABB;
	
	TreeParameters* mTreeParameters;
	
	D3D* mD3DSystem;
	TreeShader* mShader;
};
//==================================================================================================================================
//==================================================================================================================================
#endif//__TREE_H