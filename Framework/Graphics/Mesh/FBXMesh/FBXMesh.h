//===============================================================================================================================
// FBXMesh
//
// Skinned Mesh Sample
// http://www.richardssoftware.net/Home/Post/34
// https://github.com/dobkeratops/openfbx
// Still need to associate each skeleton joint for the bone to its mesh subset
// so that the subset will perform animation with a skinned mesh shader.
//   Create a bone hierarchy system and give each bone a name
//===============================================================================================================================
// History
//
// -Created on 4/2/2015 by Dustin Watson
//===============================================================================================================================
#ifndef __FBXMESH_H
#define __FBXMESH_H
//==============================================================================================================================
//==============================================================================================================================

//
// Includes
//

#include <vector>
#include <fstream>
#include <istream>
#include "ZMath.h"
#include "Vertex.h"
#include "D3D.h"
#include "Camera.h"
#include "GameDirectory.h"
#include "ShaderMaterial.h"
#include "AABB.h"
#include "SpherePrimitive.h"
#include "MeshAttributes.h"
#include "MeshRenderParameters.h"
#include "Shader.h"
#include "FBXLoader.h"
#include "MeshType.h"

//==============================================================================================================================
//==============================================================================================================================
namespace ZShadeSandboxMesh {
class FBXMesh
{
	struct GroupSubset
	{
		int startIndex;
		int indexCount;
		string materialName;
		ZShadeSandboxLighting::ShaderMaterial* material;
	};
	
public:
	
	FBXMesh(D3D* d3d, GameDirectory3D* gd3d);
	
	void Load(string filename, bool flipAxisYZ = false, ZShadeSandboxMesh::MeshType::Type mt = ZShadeSandboxMesh::MeshType::Type::eStatic);
	
	void Render(ZShadeSandboxMesh::MeshRenderParameters mrp);
	
	void Scale(XMFLOAT3 v);
	void Rotate(XMFLOAT3 v);
	void Position(XMFLOAT3 v);
	
	void SetWireframe(bool wire) { bWireframe = wire; }
	void SetFarPlane(float value) { fFarPlane = value; }
	
private:
	
	GameDirectory3D* mGD3D;
	D3D* mD3DSystem;
	bool bWireframe;
	float fFarPlane;
	ZShadeSandboxMesh::MeshType::Type mMeshType;
	
	ZShadeSandboxLighting::ShaderMaterial* mMaterial;

	ZShadeSandboxMesh::MeshAttributes* mAttributes;
	
	vector<GroupSubset> mGroups;
	
	FBXLoader* mLoader;
};
}
//==============================================================================================================================
//==============================================================================================================================
#endif//__FBXMESH_H