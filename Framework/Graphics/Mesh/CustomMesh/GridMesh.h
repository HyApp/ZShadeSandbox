//===============================================================================================================================
// GridMesh
//
//===============================================================================================================================
// History
//
// -Created on 11/19/2014 by Dustin Watson
//===============================================================================================================================
#ifndef __GRIDMESH_H
#define __GRIDMESH_H
//===============================================================================================================================
//===============================================================================================================================

//
// Includes
//
#include "CustomMesh.h"

//===============================================================================================================================
//===============================================================================================================================
namespace ZShadeSandboxMesh {
class GridMesh : public ZShadeSandboxMesh::CustomMesh
{
	float fWidth;
	float fHeight;
	float fDepth;
	
	float fScreenWidth;
	float fScreenHeight;
	
public:
	
	// Loads a basic grid mesh in 3D space
	GridMesh(float width, float height, float depth, D3D* d3d, ZShadeSandboxMesh::MeshParameters mp);
	~GridMesh();
	
	void Initialize();
};
}
//===============================================================================================================================
//===============================================================================================================================
#endif//__GRIDMESH_H