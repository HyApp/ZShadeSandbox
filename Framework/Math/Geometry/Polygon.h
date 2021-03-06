//==================================================================================================================================
// Polygon.h
//
//==================================================================================================================================
// History
//
// -Created on 1/7/2014 by Dustin Watson
//==================================================================================================================================
#ifndef __POLYGON_H
#define __POLYGON_H
//==================================================================================================================================
//==================================================================================================================================

#include <vector>
#include "D3D.h"
#include "ZMath.h"
#include "Vertex.h"
#include "MeshAttributes.h"

//==================================================================================================================================
//==================================================================================================================================
namespace ZShadeSandboxMath {
class Polygon
{
public:

	Polygon();
	
	virtual void Create(vector<ZShadeSandboxMesh::VertexPos> vertices);
	virtual void Create(vector<ZShadeSandboxMesh::VertexColor> vertices);
	virtual void Create(vector<ZShadeSandboxMesh::VertexTex> vertices);
	virtual void Create(vector<ZShadeSandboxMesh::VertexNormalTex> vertices);
	virtual void Create(vector<ZShadeSandboxMesh::VertexNormalTexTan> vertices);
	virtual void Create(vector<ZShadeSandboxMesh::VertexNormalTexTanBiTan> vertices);

	void CreateIndices(vector<UINT> indices);

	template<class TVertexType>
	void InitVertexList(vector<TVertexType>& vertices)
	{
		if (attributes->mVertexCount <= 0) return;
		if (vertices.size() > 0)
			vertices.clear();
		vertices.resize(attributes->mVertexCount);
	}
	
	// Adds a single vertex to the end of the existing set of vertices in the polygon while maintaining the indices
	// Only need to calculate the UV coordinate for VertexTex, VertexNormalTex, VertexNormalTexTan and VertexNormalTexTanBiTan
	void AddVertex(ZShadeSandboxMesh::VertexPos vertex);
	void AddVertex(ZShadeSandboxMesh::VertexColor vertex);
	void AddVertex(ZShadeSandboxMesh::VertexTex vertex);
	void AddVertex(ZShadeSandboxMesh::VertexNormalTex vertex);
	void AddVertex(ZShadeSandboxMesh::VertexNormalTexTan vertex);
	void AddVertex(ZShadeSandboxMesh::VertexNormalTexTanBiTan vertex);
	
	UINT GetIndex(int i);
	
	// This will assign the polygon normal to each vertex after it has been generated to the applicable vertex type
	void AssignNormalToVertex();
	
	ZShadeSandboxMath::XMMath3 GetVertexPosition(int index, ZShadeSandboxMesh::EVertexType::Type type);
	ZShadeSandboxMath::XMMath2 GetVertexTexture(int index, ZShadeSandboxMesh::EVertexType::Type type);

	ZShadeSandboxMesh::VertexPos GetVertexPos(int index);
	ZShadeSandboxMesh::VertexColor GetVertexColor(int index);
	ZShadeSandboxMesh::VertexTex GetVertexTex(int index);
	ZShadeSandboxMesh::VertexNormalTex GetVertexNormalTex(int index);
	ZShadeSandboxMesh::VertexNormalTexTan GetVertexNormalTexTan(int index);
	ZShadeSandboxMesh::VertexNormalTexTanBiTan GetVertexNormalTexTanBiTan(int index);
	
	ZShadeSandboxMath::XMMath3& Normal()		{ return ZShadeSandboxMath::XMMath3(normal); }
	ZShadeSandboxMath::XMMath3 Normal() const	{ return ZShadeSandboxMath::XMMath3(normal); }

	int GetVertexCount()						{ return attributes->mVertexCount; }
	int GetIndexCount()							{ return attributes->mIndexCount; }
	int	GetTriangleCount()						{ return attributes->mTriangleCount; }

	XMFLOAT3& Position()						{ return attributes->mPosition; }
	XMFLOAT3& Rotation()						{ return attributes->mRotation; }
	XMFLOAT3& Scale()							{ return attributes->mScale; }

	XMFLOAT3 Position() const					{ return attributes->mPosition; }
	XMFLOAT3 Rotation() const					{ return attributes->mRotation; }
	XMFLOAT3 Scale() const						{ return attributes->mScale; }
	
	void InitIndexList();
	
	ZShadeSandboxMesh::MeshAttributes*& Attributes() { return attributes; }
	ZShadeSandboxMesh::MeshAttributes* Attributes() const { return attributes; }
	
	string& Name() { return sName; }
	string Name() const { return sName; }

	string& MaterialName() { return sMaterialName; }
	string MaterialName() const { return sMaterialName; }

protected:

	template<class TVertexType>
	vector<TVertexType> LoadVertices(vector<TVertexType> vertices)
	{
		vector<TVertexType> verts;

		int size = vertices.size();

		verts.resize(size);

		//
		// Add the vertices
		//

		for (int vertexID = 0; vertexID < size; vertexID++)
		{
			verts[vertexID] = vertices[vertexID];
		}

		return verts;
	}

	void LoadIndices();

	template<class TVertexType>
	void LoadPolygonNormal(vector<TVertexType> vertices)
	{
		//
		// Generate the polygon normal
		//

		if (vertices.size() == 1)
		{
			normal = ZShadeSandboxMath::XMMath3(1, 1, 1);
			AssignNormalToVertex();
			return;
		}

		ZShadeSandboxMath::XMMath3 vec0(vertices[0].position);
		ZShadeSandboxMath::XMMath3 vec1(vertices[1].position);
		ZShadeSandboxMath::XMMath3 vec2(vertices[attributes->mVertexCount - 1].position);
		ZShadeSandboxMath::XMMath3 edge1 = vec1 - vec0;
		ZShadeSandboxMath::XMMath3 edge2 = vec2 - vec0;
		ZShadeSandboxMath::XMMath3 edgeNormal = edge1.Cross(edge2);
		edgeNormal.Normalize();

		normal = edgeNormal;
		
		AssignNormalToVertex();
	}

	ZShadeSandboxMesh::MeshAttributes* attributes;
	
	XMFLOAT3 normal;

	string sName;
	string sMaterialName;
};
}
//==================================================================================================================================
//==================================================================================================================================
#endif//__POLYGON_H