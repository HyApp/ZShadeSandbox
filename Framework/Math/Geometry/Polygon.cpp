#include "Polygon.h"
using ZShadeSandboxMath::Polygon;
//==================================================================================================================================
//==================================================================================================================================
Polygon::Polygon()
{
	attributes = new ZShadeSandboxMesh::MeshAttributes();
}
//==================================================================================================================================
void Polygon::Create(vector<ZShadeSandboxMesh::VertexPos> vertices)
{
	attributes->mVertexCount = vertices.size();
	InitVertexList(attributes->mVerticesPos);
	attributes->mVerticesPos = LoadVertices<ZShadeSandboxMesh::VertexPos>(vertices);
	LoadIndices();
	LoadPolygonNormal(attributes->mVerticesPos);
	attributes->mVertexByteWidth = sizeof(ZShadeSandboxMesh::VertexPos);
	attributes->mTriangleCount = attributes->mVertexCount / 3;
	if (attributes->mVertexCount < 3) attributes->mTriangleCount = 0;
}
//==================================================================================================================================
void Polygon::Create(vector<ZShadeSandboxMesh::VertexColor> vertices)
{
	attributes->mVertexCount = vertices.size();
	InitVertexList(attributes->mVerticesColor);
	attributes->mVerticesColor = LoadVertices<ZShadeSandboxMesh::VertexColor>(vertices);
	LoadIndices();
	LoadPolygonNormal(attributes->mVerticesColor);
	attributes->mVertexByteWidth = sizeof(ZShadeSandboxMesh::VertexColor);
	attributes->mTriangleCount = attributes->mVertexCount / 3;
	if (attributes->mVertexCount < 3) attributes->mTriangleCount = 0;
}
//==================================================================================================================================
void Polygon::Create(vector<ZShadeSandboxMesh::VertexTex> vertices)
{
	attributes->mVertexCount = vertices.size();
	InitVertexList(attributes->mVerticesTex);
	attributes->mVerticesTex = LoadVertices<ZShadeSandboxMesh::VertexTex>(vertices);
	LoadIndices();
	LoadPolygonNormal(attributes->mVerticesTex);
	attributes->mVertexByteWidth = sizeof(ZShadeSandboxMesh::VertexTex);
	attributes->mTriangleCount = attributes->mVertexCount / 3;
	if (attributes->mVertexCount < 3) attributes->mTriangleCount = 0;
}
//==================================================================================================================================
void Polygon::Create(vector<ZShadeSandboxMesh::VertexNormalTex> vertices)
{
	attributes->mVertexCount = vertices.size();
	InitVertexList(attributes->mVerticesNormalTex);
	attributes->mVerticesNormalTex = LoadVertices<ZShadeSandboxMesh::VertexNormalTex>(vertices);
	LoadIndices();
	LoadPolygonNormal(attributes->mVerticesNormalTex);
	attributes->mVertexByteWidth = sizeof(ZShadeSandboxMesh::VertexNormalTex);
	attributes->mTriangleCount = attributes->mVertexCount / 3;
	if (attributes->mVertexCount < 3) attributes->mTriangleCount = 0;
}
//==================================================================================================================================
void Polygon::Create(vector<ZShadeSandboxMesh::VertexNormalTexTan> vertices)
{
	attributes->mVertexCount = vertices.size();
	InitVertexList(attributes->mVerticesNormalTexTan);
	attributes->mVerticesNormalTexTan = LoadVertices<ZShadeSandboxMesh::VertexNormalTexTan>(vertices);
	LoadIndices();
	LoadPolygonNormal(attributes->mVerticesNormalTexTan);
	attributes->mVertexByteWidth = sizeof(ZShadeSandboxMesh::VertexNormalTexTan);
	attributes->mTriangleCount = attributes->mVertexCount / 3;
	if (attributes->mVertexCount < 3) attributes->mTriangleCount = 0;
}
//==================================================================================================================================
void Polygon::Create(vector<ZShadeSandboxMesh::VertexNormalTexTanBiTan> vertices)
{
	attributes->mVertexCount = vertices.size();
	InitVertexList(attributes->mVerticesNormalTexTanBi);
	attributes->mVerticesNormalTexTanBi = LoadVertices<ZShadeSandboxMesh::VertexNormalTexTanBiTan>(vertices);
	LoadIndices();
	LoadPolygonNormal(attributes->mVerticesNormalTexTanBi);
	attributes->mVertexByteWidth = sizeof(ZShadeSandboxMesh::VertexNormalTexTanBiTan);
	attributes->mTriangleCount = attributes->mVertexCount / 3;
	if (attributes->mVertexCount < 3) attributes->mTriangleCount = 0;
}
//==================================================================================================================================
void Polygon::AssignNormalToVertex()
{
	// These do not need a normal
	if (attributes->mVerticesPos.size() > 0) return;
	if (attributes->mVerticesColor.size() > 0) return;
	if (attributes->mVerticesTex.size() > 0) return;
	
	
	// These do
	
	if (attributes->mVerticesNormalTex.size() > 0)
	{
		for (int i = 0; i < attributes->mVertexCount; i++)
		{
			attributes->mVerticesNormalTex[i].normal = normal;
		}
	}
	
	if (attributes->mVerticesNormalTexTan.size() > 0)
	{
		for (int i = 0; i < attributes->mVertexCount; i++)
		{
			attributes->mVerticesNormalTexTan[i].normal = normal;
		}
	}
	
	if (attributes->mVerticesNormalTexTanBi.size() > 0)
	{
		for (int i = 0; i < attributes->mVertexCount; i++)
		{
			attributes->mVerticesNormalTexTanBi[i].normal = normal;
		}
	}
}
//==================================================================================================================================
UINT Polygon::GetIndex(int i)
{
	return attributes->mIndices[i];
}
//==================================================================================================================================
ZShadeSandboxMath::XMMath3 Polygon::GetVertexPosition(int index, ZShadeSandboxMesh::EVertexType::Type type)
{
	switch (type)
	{
		case ZShadeSandboxMesh::EVertexType::Type::VT_Pos:
		{
			return ZShadeSandboxMath::XMMath3(attributes->mVerticesPos[index].position);
		}
		break;
		case ZShadeSandboxMesh::EVertexType::Type::VT_Color:
		{
			return ZShadeSandboxMath::XMMath3(attributes->mVerticesColor[index].position);
		}
		break;
		case ZShadeSandboxMesh::EVertexType::Type::VT_Tex:
		{
			return ZShadeSandboxMath::XMMath3(attributes->mVerticesTex[index].position);
		}
		break;
		case ZShadeSandboxMesh::EVertexType::Type::VT_NormalTex:
		{
			return ZShadeSandboxMath::XMMath3(attributes->mVerticesNormalTex[index].position);
		}
		break;
		case ZShadeSandboxMesh::EVertexType::Type::VT_NormalTexTan:
		{
			return ZShadeSandboxMath::XMMath3(attributes->mVerticesNormalTexTan[index].position);
		}
		break;
		case ZShadeSandboxMesh::EVertexType::Type::VT_NormalTexTanBi:
		{
			return ZShadeSandboxMath::XMMath3(attributes->mVerticesNormalTexTanBi[index].position);
		}
		break;
	}
}
//==================================================================================================================================
ZShadeSandboxMath::XMMath2 Polygon::GetVertexTexture(int index, ZShadeSandboxMesh::EVertexType::Type type)
{
	switch (type)
	{
		case ZShadeSandboxMesh::EVertexType::Type::VT_Tex:
		{
			return ZShadeSandboxMath::XMMath2(attributes->mVerticesTex[index].texture);
		}
		break;
		case ZShadeSandboxMesh::EVertexType::Type::VT_NormalTex:
		{
			return ZShadeSandboxMath::XMMath2(attributes->mVerticesNormalTex[index].texture);
		}
		break;
		case ZShadeSandboxMesh::EVertexType::Type::VT_NormalTexTan:
		{
			return ZShadeSandboxMath::XMMath2(attributes->mVerticesNormalTexTan[index].texture);
		}
		break;
		case ZShadeSandboxMesh::EVertexType::Type::VT_NormalTexTanBi:
		{
			return ZShadeSandboxMath::XMMath2(attributes->mVerticesNormalTexTanBi[index].texture);
		}
		break;
		case ZShadeSandboxMesh::EVertexType::Type::VT_Pos:
		{
			ZShadeMessageCenter::MsgBoxError(NULL, "Cannot get a UV from a vertex position array");
		}
		break;
		case ZShadeSandboxMesh::EVertexType::Type::VT_Color:
		{
			ZShadeMessageCenter::MsgBoxError(NULL, "Cannot get a UV from a vertex color array");
		}
		break;
	}
}
//==================================================================================================================================
void Polygon::CreateIndices(vector<UINT> indices)
{
	attributes->mIndexCount = indices.size();

	InitIndexList();

	for (int indexID = 0; indexID < attributes->mIndexCount; indexID++)
	{
		attributes->mIndices[indexID] = indices[indexID];
	}
}
//==================================================================================================================================
void Polygon::LoadIndices()
{
	if (attributes->mVertexCount == 1)
	{
		attributes->mIndexCount = 1;

		InitIndexList();

		attributes->mIndices[(0 * 3) + 0] = 0;

		return;
	}

	if (attributes->mVertexCount == 2)
	{
		attributes->mIndexCount = 2;

		InitIndexList();

		attributes->mIndices[(0 * 3) + 0] = 0;
		attributes->mIndices[(0 * 3) + 1] = 1;

		return;
	}
	
	attributes->mIndexCount = (attributes->mVertexCount - 2) * 3;

	InitIndexList();

	//
	// Add the indices
	//

	UINT v0, v1, v2;

	for (int indexID = 0; indexID < attributes->mIndexCount / 3; indexID++)
	{
		if (indexID == 0)
		{
			v0 = 0;
			v1 = 1;
			v2 = 2;
		}
		else
		{
			v1 = v2;
			v2++;
		}

		attributes->mIndices[(indexID * 3) + 0] = v0;
		attributes->mIndices[(indexID * 3) + 1] = v1;
		attributes->mIndices[(indexID * 3) + 2] = v2;
	}
}
//==================================================================================================================================
void Polygon::InitIndexList()
{
	if (attributes->mIndexCount <= 0) return;
	if (attributes->mIndices.size() > 0)
		attributes->mIndices.clear();
	attributes->mIndices.resize(attributes->mIndexCount);
}
//==================================================================================================================================
ZShadeSandboxMesh::VertexPos Polygon::GetVertexPos(int index)
{
	return attributes->mVerticesPos[index];
}
//==================================================================================================================================
ZShadeSandboxMesh::VertexColor Polygon::GetVertexColor(int index)
{
	return attributes->mVerticesColor[index];
}
//==================================================================================================================================
ZShadeSandboxMesh::VertexTex Polygon::GetVertexTex(int index)
{
	return attributes->mVerticesTex[index];
}
//==================================================================================================================================
ZShadeSandboxMesh::VertexNormalTex Polygon::GetVertexNormalTex(int index)
{
	return attributes->mVerticesNormalTex[index];
}
//==================================================================================================================================
ZShadeSandboxMesh::VertexNormalTexTan Polygon::GetVertexNormalTexTan(int index)
{
	return attributes->mVerticesNormalTexTan[index];
}
//==================================================================================================================================
ZShadeSandboxMesh::VertexNormalTexTanBiTan Polygon::GetVertexNormalTexTanBiTan(int index)
{
	return attributes->mVerticesNormalTexTanBi[index];
}
//==================================================================================================================================
void Polygon::AddVertex(ZShadeSandboxMesh::VertexPos vertex)
{
	int vertexCount = attributes->mVertexCount;
	
	// Get the current vertices that the polygon contains
	vector<ZShadeSandboxMesh::VertexPos> vertices(vertexCount + 1);
	
	int i = 0;
	
	if (vertexCount > 0)
	{
		for (; i < vertexCount; i++)
		{
			vertices[i] = GetVertexPos(i);
		}
	}
	
	// Finally add the new vertex
	if (vertexCount > 0)
	{
		vertices[(i + 1) - 1] = vertex;
	}
	else
	{
		vertices[i] = vertex;
	}
	
	// Create the new vertex set in the polygon
	Create(vertices);
}
//==================================================================================================================================
void Polygon::AddVertex(ZShadeSandboxMesh::VertexColor vertex)
{
	int vertexCount = attributes->mVertexCount;
	
	// Get the current vertices that the polygon contains
	vector<ZShadeSandboxMesh::VertexColor> vertices(vertexCount + 1);
	
	int i = 0;
	
	if (vertexCount > 0)
	{
		for (; i < vertexCount; i++)
		{
			vertices[i] = GetVertexColor(i);
		}
	}
	
	// Finally add the new vertex
	if (vertexCount > 0)
	{
		vertices[(i + 1) - 1] = vertex;
	}
	else
	{
		vertices[i] = vertex;
	}
	
	// Create the new vertex set in the polygon
	Create(vertices);
}
//==================================================================================================================================
void Polygon::AddVertex(ZShadeSandboxMesh::VertexTex vertex)
{
	int vertexCount = attributes->mVertexCount;
	
	// Get the current vertices that the polygon contains
	vector<ZShadeSandboxMesh::VertexTex> vertices(vertexCount + 1);
	
	int i = 0;
	
	if (vertexCount > 0)
	{
		for (; i < vertexCount; i++)
		{
			vertices[i] = GetVertexTex(i);
		}
	}
	
	// Calculate the UV (http://forum.devmaster.net/t/uvw-unwrap-algorithms/21409)
	//ZShadeSandboxMath::XMMath3 translatedPos(-1 * vertex.position.x, -1 * vertex.position.y, -1 * vertex.position.z);
	//vertex.texture = XMFLOAT2(vertex.position.x * (1 / (size - 1)), vertex.position.z * (1 / (size - 1)));
	
	// Finally add the new vertex
	if (vertexCount > 0)
	{
		vertices[(i + 1) - 1] = vertex;
	}
	else
	{
		vertices[i] = vertex;
	}
	
	// Create the new vertex set in the polygon
	Create(vertices);
}
//==================================================================================================================================
void Polygon::AddVertex(ZShadeSandboxMesh::VertexNormalTex vertex)
{
	int vertexCount = attributes->mVertexCount;
	
	// Get the current vertices that the polygon contains
	vector<ZShadeSandboxMesh::VertexNormalTex> vertices(vertexCount + 1);
	
	int i = 0;
	
	if (vertexCount > 0)
	{
		for (; i < vertexCount; i++)
		{
			vertices[i] = GetVertexNormalTex(i);
		}
	}
	
	// Calculate the UV
	
	// Finally add the new vertex
	if (vertexCount > 0)
	{
		vertices[(i + 1) - 1] = vertex;
	}
	else
	{
		vertices[i] = vertex;
	}
	
	// Create the new vertex set in the polygon
	Create(vertices);
}
//==================================================================================================================================
void Polygon::AddVertex(ZShadeSandboxMesh::VertexNormalTexTan vertex)
{
	int vertexCount = attributes->mVertexCount;
	
	// Get the current vertices that the polygon contains
	vector<ZShadeSandboxMesh::VertexNormalTexTan> vertices(vertexCount + 1);
	
	int i = 0;
	
	if (vertexCount > 0)
	{
		for (; i < vertexCount; i++)
		{
			vertices[i] = GetVertexNormalTexTan(i);
		}
	}
	
	// Calculate the UV
	
	// Finally add the new vertex
	if (vertexCount > 0)
	{
		vertices[(i + 1) - 1] = vertex;
	}
	else
	{
		vertices[i] = vertex;
	}
	
	// Create the new vertex set in the polygon
	Create(vertices);
}
//==================================================================================================================================
void Polygon::AddVertex(ZShadeSandboxMesh::VertexNormalTexTanBiTan vertex)
{
	int vertexCount = attributes->mVertexCount;
	
	// Get the current vertices that the polygon contains
	vector<ZShadeSandboxMesh::VertexNormalTexTanBiTan> vertices(vertexCount + 1);
	
	int i = 0;
	
	if (vertexCount > 0)
	{
		for (; i < vertexCount; i++)
		{
			vertices[i] = GetVertexNormalTexTanBiTan(i);
		}
	}
	
	// Calculate the UV
	
	// Finally add the new vertex
	if (vertexCount > 0)
	{
		vertices[(i + 1) - 1] = vertex;
	}
	else
	{
		vertices[i] = vertex;
	}
	
	// Create the new vertex set in the polygon
	Create(vertices);
}
//==================================================================================================================================