//===============================================================================================================================
// OBJMesh.h
//
//===============================================================================================================================
// History
//
// -Created on 5/18/2015 by henriyl
// -Updated 5/21/2015 for the engine by Dustin Watson
//===============================================================================================================================
#ifndef __OBJMESH_H
#define __OBJMESH_H
//===============================================================================================================================
//===============================================================================================================================

//
// Includes
//

#include <vector>
#include <fstream>
#include <istream>
#include "Vertex.h"
#include "D3D.h"
#include "Camera.h"
#include "GameDirectory.h"
#include "OBJMeshShader.h"
#include "OBJMeshTessellationShader.h"
#include "ShaderMaterial.h"
#include "OBJGBufferShader.h"
#include "AABB.h"
#include "SpherePrimitive.h"
#include "MeshAttributes.h"
#include "Shader.h"

//===============================================================================================================================
//===============================================================================================================================

typedef unsigned int IndexType;

// SamplerStates etc in hlsl files do nothing if Effect framework is not used
// MaleLow.obj has quads, retriangulated in VS. Some of the triangles still have wrong winding order...

// Need a separate vertex array that includes the real positions of the vertices

namespace ZShadeSandboxMesh
{
class OBJMesh
{
	struct Group
	{
		ZShadeSandboxLighting::ShaderMaterial Material;
		string Name;
		int IndexStart;
		int IndexCount;
	};

	class Mesh
	{
	public:
		
		Mesh(D3D* d3d);
		~Mesh();
		
		bool Load(string filename, bool bRebuildNormals, bool isRHCoordSys);
		void Render(MeshRenderParameters render_param);
		void RenderCustom(MeshRenderParameters render_param);
		void SetFarPlane(float farPlane);

		void Scale(XMFLOAT3 v);
		void Rotate(XMFLOAT3 v);
		void Position(XMFLOAT3 v);
		
		ZShadeSandboxMath::AABB* GetAABB();
		bool IntersectsAABB(XMFLOAT3 point);
		float DistanceAABBToPoint(XMFLOAT3 point);
		void UpdateAABB(XMFLOAT3 center, XMFLOAT3 scale);
		
		ZShadeSandboxMath::SpherePrimitive* GetSphere();
		bool IntersectsSphere(XMFLOAT3 point);
		float DistanceSphereToPoint(XMFLOAT3 point);
		
		void AddInstanceMatrix(
			vector<XMFLOAT3> instancePositions,
			vector<XMFLOAT3> instanceRotations,
			vector<XMFLOAT3> instanceScales
		);

		void AddInstanceMatrix(vector<ZShadeSandboxMesh::InstanceMatrix> instMat);
	
		ID3D11Buffer* InstanceBuffer() { return mAttributes->mInstanceBuffer; }
		
		vector<ZShadeSandboxMesh::InstanceMatrix>& InstanceMatrices();
		vector<ZShadeSandboxMesh::InstanceMatrix> InstanceMatrices() const;
		
		//void AddCustomShader(Shader* shader);
		//void AddCustomTessellationShader(Shader* shader);
		//void AddCustomOBJGBufferShader(Shader* shader);
		ZShadeSandboxShader::Shader*& CustomShader();
		ZShadeSandboxShader::Shader* CustomShader() const;
		ZShadeSandboxShader::Shader*& CustomTessellationShader();
		ZShadeSandboxShader::Shader* CustomTessellationShader() const;
		ZShadeSandboxShader::Shader*& CustomOBJGBufferShader();
		ZShadeSandboxShader::Shader* CustomOBJGBufferShader() const;
		
		bool& HasCustomShader() { return bHasCustomShader; }
		bool HasCustomShader() const { return bHasCustomShader; }
		
		bool& Wireframe() { return bWireframe; }
		bool Wireframe() const { return bWireframe; }
		
		MeshAttributes*& Attributes() { return mAttributes; }
		MeshAttributes* Attributes() const { return mAttributes; }
		
	private:

		MeshAttributes* mAttributes;
		
		bool bHasCustomShader;
		bool bWireframe;
		
		ZShadeSandboxShader::Shader*						mCustomShader;
		ZShadeSandboxShader::Shader*						mCustomTessellationShader;
		ZShadeSandboxShader::Shader*						mCustomOBJGBufferShader;
		
		OBJMeshShader*      	   	mShader;
		OBJMeshTessellationShader*  mTessellationShader;
		OBJGBufferShader*			mOBJGBufferShader;
		D3D*						mD3DSystem;
		vector<Group>				mGroups;

		ZShadeSandboxMath::AABB* mAABB;
		ZShadeSandboxMath::SpherePrimitive* mSphere;
	};

public:

	OBJMesh(D3D* d3d, GameDirectory3D* g3D);
	~OBJMesh();

	void Load(string filename, bool bRebuildNormals = false, bool isRHCoordSys = false);
	void Render(MeshRenderParameters render_param);
	void SetWireframe(bool wire);
	void SetFarPlane(float farPlane);
	
	void Scale(XMFLOAT3 v);
	void Rotate(XMFLOAT3 v);
	void Position(XMFLOAT3 v);
	
	//void AddCustomShader(Shader* shader);
	//void AddCustomTessellationShader(Shader* shader);
	//void AddCustomOBJGBufferShader(Shader* shader);
	ZShadeSandboxShader::Shader*& CustomShader();
	ZShadeSandboxShader::Shader* CustomShader() const;
	ZShadeSandboxShader::Shader*& CustomTessellationShader();
	ZShadeSandboxShader::Shader* CustomTessellationShader() const;
	ZShadeSandboxShader::Shader*& CustomOBJGBufferShader();
	ZShadeSandboxShader::Shader* CustomOBJGBufferShader() const;
	
	bool& HasCustomShader();
	bool HasCustomShader() const;
	
	ZShadeSandboxMath::AABB* GetAABB();
	bool IntersectsAABB(XMFLOAT3 point);
	float DistanceAABBToPoint(XMFLOAT3 point);
	void UpdateAABB(XMFLOAT3 center, XMFLOAT3 scale);
	
	ZShadeSandboxMath::SpherePrimitive* GetSphere();
	bool IntersectsSphere(XMFLOAT3 point);
	float DistanceSphereToPoint(XMFLOAT3 point);
	
	void AddInstanceMatrix(
		vector<XMFLOAT3> instancePositions,
		vector<XMFLOAT3> instanceRotations,
		vector<XMFLOAT3> instanceScales
	);
	void AddInstanceMatrix(vector<ZShadeSandboxMesh::InstanceMatrix> instMat);

	vector<ZShadeSandboxMesh::InstanceMatrix>& InstanceMatrices();
	vector<ZShadeSandboxMesh::InstanceMatrix> InstanceMatrices() const;
	//void AddInstancePositions(vector<XMFLOAT3> v);
	//vector<ZShadeSandboxMesh::InstancePos>& InstancePositions();
	//vector<ZShadeSandboxMesh::InstancePos> InstancePositions() const;
	
	ID3D11Buffer* InstanceBuffer();
	
	MeshAttributes*& Attributes();
	MeshAttributes* Attributes() const;
	
private:
	
	static GameDirectory3D* mGD3D;
	static D3D*  mD3DSystem;
	
	Mesh* m_pMesh;
	
	static bool LoadMtl(string filename, D3D* d3d, map<string, ZShadeSandboxLighting::ShaderMaterial>& materials);
	
	static bool LoadObj(
		string filename,
		D3D* d3d,
		vector<VertexNormalTex>& vertices,
		vector<IndexType>& indices,
		vector<Group>& groups,
		bool& hadNormals,
		bool isRHCoordSys
	);
};
}
//===============================================================================================================================
//===============================================================================================================================
#endif//__OBJMESH_H