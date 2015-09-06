//==================================================================================================================================
// PolygonMesh
//
//==================================================================================================================================
// History
//
// -Created on 6/26/2015 by Dustin Watson
//==================================================================================================================================
#ifndef __POLYGONMESH_H
#define __POLYGONMESH_H
//==================================================================================================================================
//==================================================================================================================================

#include <vector>
#include "D3D.h"
#include "ZMath.h"
#include "Polygon.h"
#include "Vertex.h"
#include "MeshAttributes.h"
#include "Camera.h"
#include "ShaderMaterial.h"
#include "MeshRenderParameters.h"

//==================================================================================================================================
//==================================================================================================================================
namespace ZShadeSandboxMesh {
class PolygonMesh
{
	// For complex polygons need to have it rendered in groups so that rendering does not get messed up.
	// Currently does not work.
	struct Group
	{
		//ZShadeSandboxLighting::ShaderMaterial Material;
		//string Name;
		int IndexStart;
		int IndexCount;
	};
	
public:
	
	PolygonMesh(ZShadeSandboxMath::Polygon* poly, ZShadeSandboxLighting::ShaderMaterial* material);
	
	bool& Wireframe() { return m_Wireframe; }
	
	XMFLOAT3& Position();
	XMFLOAT3& Scale();
	XMFLOAT3& Rotate();
	
	// subsetStartIndex contains the starting index for each group in the vertex buffer
	// also get the index count for the group by counting by using indices.size() - groups.back().IndexStart
	void CreateSubsets(vector<int> subsetStartIndex);
	
	ZShadeSandboxMath::Polygon*& Poly() { return mPolygon; }
	ZShadeSandboxMath::Polygon* Poly() const { return mPolygon; }
	
	ZShadeSandboxMesh::MeshAttributes*& Attributes() { return mPolygon->Attributes(); }
	ZShadeSandboxMesh::MeshAttributes* Attributes() const { return mPolygon->Attributes(); }

	void BuildIndexBuffer();
	void BuildVertexBuffer(ZShadeSandboxMesh::EVertexType::Type type);
	void ReBuildVertexBuffer(ZShadeSandboxMesh::EVertexType::Type type);
	
	void Render(ZShadeSandboxMesh::MeshRenderParameters mrp);
	
protected:
	
	bool m_Wireframe;
	
	ZShadeSandboxLighting::ShaderMaterial* mMaterial;
	
	ZShadeSandboxMath::Polygon* mPolygon;
	
	vector<Group> mGroups;
};
}
//==================================================================================================================================
//==================================================================================================================================
#endif//__POLYGONMESH_H