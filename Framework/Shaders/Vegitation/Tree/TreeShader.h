//===============================================================================================================================
// TreeShader
//
//===============================================================================================================================
// History
//
// -Created on 8/14/2015 by Dustin Watson
//===============================================================================================================================
#ifndef __TREESHADER_H
#define __TREESHADER_H
//==============================================================================================================================
//==============================================================================================================================

//
// Includes
//

#include "D3D.h"
#include "Shader.h"
#include "Camera.h"
#include "Vertex.h"
#include "OBJMesh.h"
#include "TreeParameters.h"

//==============================================================================================================================
//==============================================================================================================================
class TreeShader : public ZShadeSandboxShader::Shader
{
	struct cbTreeBuffer
	{
		XMFLOAT3	g_EyePos;
		float 		g_TreeBillWidth;
		float		g_TreeBillHeight;
		float		padding1;
		float		padding2;
		XMFLOAT4	g_DiffuseColor;
		int			g_HasTexture;
		int			g_HasNormalMap;
		int			g_IsInstance;
		int			g_IsLeaf;
	};
	
	struct cbLeafBuffer
	{
		// This only needs to be loaded once into the constant buffer
		XMFLOAT4X4  g_LeafOnTree[NUM_LEAVES_PER_TREE];
	};
	
	struct cbMatrixBuffer
	{
		XMFLOAT4X4	g_matWorld;
		XMFLOAT4X4	g_matView;
		XMFLOAT4X4	g_matProj;
	};
	
public:
	
	TreeShader(D3D* d3d);
	TreeShader(const TreeShader& other);
	~TreeShader();
	
	bool Initialize();
	void Shutdown();
	
	// This should only be called once after the leaf matrices have been calculated
	void InitializeLeafBuffer(InternalTreeRenderParameters trp);
	
	void SetFarPlane(float farPlane) { fFarPlane = farPlane; }
	void BillTreeWidth(float billTreeWidth) { fBillTreeWidth = billTreeWidth; }
	void BillTreeHeight(float billTreeHeight) { fBillTreeHeight = billTreeHeight; }
	
	void RenderAndSaveBillboard
	(	int leafIndexCount
	,	ZShadeSandboxMesh::MeshRenderParameters mrp
	,	ID3D11ShaderResourceView* leafTexture
	,	ZShadeSandboxMath::AABB treeAABB
	,	ZShadeSandboxMesh::OBJMesh* treeMesh
	,	std::string billboardTextureName
	);
	
	void RenderLeaves
	(	int indexCount
	,	ZShadeSandboxMesh::MeshRenderParameters mrp
	,	ID3D11ShaderResourceView* texture
	);
	
	// The tree OBJ mesh will use this to render the actual tree
	void Render
	(	int indexStart
	,	int indexCount
	,	ZShadeSandboxMesh::MeshRenderParameters mrp
	,	ZShadeSandboxLighting::ShaderMaterial* material
	);
	
	void RenderBillboards
	(	ZShadeSandboxMesh::MeshRenderParameters mrp
	,	ID3D11ShaderResourceView* billboardTexture
	);
	
private:
	
	//
	// Variables
	//
	
	float fFarPlane;
	float fBillTreeWidth;
	float fBillTreeHeight;
	
	ID3D11Buffer* m_pTreeBufferCB;
	ID3D11Buffer* m_pLeafBufferCB;
	ID3D11Buffer* m_pMatrixBufferCB;
};
//==============================================================================================================================
//==============================================================================================================================
#endif//__WATERSHADER_H