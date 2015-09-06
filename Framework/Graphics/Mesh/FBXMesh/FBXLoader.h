//===============================================================================================================================
// FBXLoader
//
// http://www.walkerb.net/blog/dx-4/
// http://gamedev.stackexchange.com/questions/59419/c-fbx-animation-importer-using-the-fbx-sdk
// http://www.gamedev.net/page/resources/_/technical/graphics-programming-and-theory/how-to-work-with-fbx-sdk-r3582
// http://www.gamedev.net/page/resources/_/technical/graphics-programming-and-theory/skinned-mesh-animation-using-matrices-r3577
// https://github.com/lang1991/FBXExporter
// https://github.com/lang1991/FBXExporter/blob/master/FBXExporter/FBXExporter.h
//===============================================================================================================================
// History
//
// -Created on 4/2/2015 by Dustin Watson
//===============================================================================================================================
#ifndef __FBXLOADER_H
#define __FBXLOADER_H
//==============================================================================================================================
//==============================================================================================================================

//
// Includes
//

#include <unordered_map>
#include <vector>
#include <fstream>
#include "FBXInclude.h"
#include "CGlobal.h"
#include "ShaderMaterial.h"
#include "FBXVertex.h"
#include "FBXHelper.h"
using namespace std;

//==============================================================================================================================
//==============================================================================================================================
namespace ZShadeSandboxMesh {
class FBXLoader
{
	struct FBXSubsets
	{
		vector<uint32> mJoints;
		uint32 mMaterialIndex;
		string mMaterialName;
		uint32 mPolygonCount;
		//vector<FBXTriangle> mTriangles;
		vector<uint32> mIndices;
		vector<ZShadeSandboxMesh::VertexNormalTexBlend> mVertices;
		unordered_map<uint32, ZShadeSandboxLighting::ShaderMaterial*> mMaterials;
	};
	
public:
	
	FBXLoader(D3D* d3d, GameDirectory3D* gd3d);
	~FBXLoader();
	
	bool Load(const char* model_path);
	
	// Might need to know if each subset has an animation since there could be
	// some mesh subsets that do not have an animation like a torso on a body.
	bool HasAnimation() { return bHasAnimation; }
	string AnimationName() { return mAnimationName; }
	
	// This contains the entire skeleton of the mesh, meaning it has the joints that connect together to form the skeleton
	ZShadeSandboxMesh::FBXSkeleton Skeleton() { return mSkeleton; }
	
	int SubsetCount() { return mSubsets.size(); }
	int SubsetVertexCount(int subsetID);
	int SubsetIndexCount(int subsetID);
	int SubsetPolygonCount(int subsetID);
	int SubsetMaterialIndex(int subsetID);
	string SubsetMaterialName(int subsetID);
	//vector<FBXTriangle> SubsetTriangles(int subsetID);
	vector<uint32> SubsetJoints(int subsetID);
	vector<uint32> SubsetIndices(int subsetID);
	vector<ZShadeSandboxMesh::VertexNormalTexBlend> SubsetVertices(int subsetID);
	ZShadeSandboxLighting::ShaderMaterial* SubsetMaterial(int subsetID, int materialID);
	
private:
	
	// Removes duplicated vertices
	void Optimize();
	void CleanupFbxManager();
	
	bool LoadScene();
	
	// FBX Mesh Data
	
	void LoadMesh(FbxNode* node);
	void LoadGeometry(FbxNode* node);
	bool LoadVertexNormal(FbxMesh* mesh, int inCtrlPointIndex, int inVertexCounter, int normalElement, XMFLOAT3& outNormal);
	bool LoadVertexTexture(FbxMesh* mesh, int inCtrlPointIndex, int inTextureIndex, int inTextureLayer, XMFLOAT2& outTexture);
	bool LoadVertexBinormal(FbxMesh* mesh, int inCtrlPointIndex, int inVertexCounter, int binormalElement, XMFLOAT3& outBinormal);
	bool LoadVertexTangent(FbxMesh* mesh, int inCtrlPointIndex, int inVertexCounter, int tangentElement, XMFLOAT3& outTangent);
	int FindVertex(const ZShadeSandboxMesh::VertexNormalTexBlend& inTargetVertex, const std::vector<ZShadeSandboxMesh::VertexNormalTexBlend>& uniqueVertices);
	
	// FBX Animation
	
	void LoadSkeletonHierarchy(FbxNode* inRootNode);
	void LoadSkeletonHierarchy(FbxNode* inNode, int inDepth, int myIndex, int parentIndex);
	void LoadJointsAndAnimation(FbxNode* inNode);
	void LoadControlPoints(FbxNode* node);
	uint32 FindJointIndexUsingName(string& name);
	
	// Material Data
	
	void AssociateMaterialToMesh(FbxNode* inNode);
	void LoadMaterials(FbxNode* inNode);
	void LoadMaterialAttribute(FbxSurfaceMaterial* inMaterial, uint32 inMaterialIndex, uint32 subsetID);
	void LoadMaterialTexture(FbxSurfaceMaterial* inMaterial, ZShadeSandboxLighting::ShaderMaterial*& ioMaterial);
	
	// Sets the material name for the subset
	void SetMaterialName(ZShadeSandboxLighting::ShaderMaterial*& ioMaterial);
	
private:
	
	static int iFbxMaterialCounter;
	
	ofstream outVertexFile;
	ofstream outIndiceFile;
	ofstream outFile;
	D3D* m_pD3DSystem;
	GameDirectory3D* m_pGD3D;
	FbxManager* m_pFBXManager;
	FbxScene* m_pFbxScene;
	string mInputFilePath;
	bool bHasAnimation;
	int iCurrentSubset;
	vector<FBXSubsets*> mSubsets;
	ZShadeSandboxMesh::FBXSkeleton mSkeleton;
	unordered_map<uint32, ZShadeSandboxMesh::PhysicalPoint*> mControlPoints; // Used to load the mesh vertices only
	string mAnimationName;
	FbxLongLong mAnimationLength;
	LARGE_INTEGER mCPUFreq;
};
}
//==============================================================================================================================
//==============================================================================================================================
#endif//__FBXLOADER_H