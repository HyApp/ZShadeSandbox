//===============================================================================================================================
// DAEHelper
//
//===============================================================================================================================
// History
//
// -Created on 8/31/2015 by Dustin Watson
//===============================================================================================================================
#ifndef __DAEHELPER_H
#define __DAEHELPER_H
//===============================================================================================================================
//===============================================================================================================================

//
// Includes
//

#include <string>
#include <vector>
#include "D3D.h"
#include "ZMath.h"
#include "DAESkinnedVertex.h"

//===============================================================================================================================
//===============================================================================================================================
namespace ZShadeSandboxMesh {
struct DAEMaterial
{
	std::string mName;
	std::string mEffect;
	std::string mInitFrom;
};

struct DAEEffect
{
	enum EffectType
	{
		eImage,
		eNonImage
	};
	
	EffectType mType;
	
	std::string mName;
	std::string mInitFrom;
	
	XMFLOAT4 mAmbientColor;
	XMFLOAT4 mDiffuseColor;
	XMFLOAT4 mSpecularColor;
	
	float fShininess;
	float fAlhpa;
	bool bSpecularToggle;
};

struct DAEScene
{
	enum SceneType
	{
		eGeometry,
		eLight,
		eCamera
	};
	
	SceneType mType;
	
	ZShadeSandboxMath::XMMatrix mWorld;
	
	std::string mName;
	
	std::vector<std::string> vMaterials;
	
	std::string mGeometryLink;
};

// Contains load data for collada mesh, buffer will not be built here
struct DAELoaderMeshData
{
	std::string mName;
	
	XMFLOAT3 mPosition;
	
	std::vector<std::string> vTextures;
	std::vector<DAEEffect> vMaterials;
	std::vector<DAESkinnedVertex> vVertices;
	std::vector<uint32> vIndices;
	
	std::vector<int> vReuseVertexPositions;
	std::vector<int>* vReverseReuseVertexPositions;
	
	int iTriangleCount;
	int iVertexCount;
	int iIndexCount;
	
	// Matrix used to fix mesh to bone skeleton
	ZShadeSandboxMath::XMMatrix mObjMatrix;
	
	// Helps flip indices from 0,1,2 to 0,2,1
	// to render with cull clockwise face
	int flipIndexOrder(int oldIndex);
	
	std::vector<uint32> optimizeVertexBuffer();
	
	void Optimize();
};
}
//===============================================================================================================================
//===============================================================================================================================
#endif//__DAEHELPER_H