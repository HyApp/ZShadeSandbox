//===============================================================================================================================
// OceanMesh
//
//===============================================================================================================================
// History
//
// -Created on 9/4/2015 by Dustin Watson
//===============================================================================================================================
#ifndef __OCEANMESH_H
#define __OCEANMESH_H
//===============================================================================================================================
//===============================================================================================================================

//
// Includes
//

#include "D3D.h"
#include "ZMath.h"
#include "MeshAttributes.h"
#include "OceanParameters.h"

//================================================================================================================================================================
//================================================================================================================================================================

//
// Globals
//

#define MESH_INDEX_2D(x, y)	(((y) + vert_rect.bottom) * (128 + 1) + (x) + vert_rect.left)

//===============================================================================================================================
//===============================================================================================================================

class OceanMesh
{
public:
	
	struct QuadNode
	{
		XMFLOAT2 bottom_left;
		float length;
		int lod;
		int sub_node[4];
	};
	
	struct QuadRenderParam
	{
		UINT num_inner_verts;
		UINT num_inner_faces;
		UINT inner_start_index;

		UINT num_boundary_verts;
		UINT num_boundary_faces;
		UINT boundary_start_index;
	};
	
public:
	
	OceanMesh(D3D* d3d, OceanParameters params);
	
	void Init();
	
	int Lods() const { return m_lods; }
	
	ZShadeSandboxMesh::MeshAttributes* Attributes();
	
	QuadRenderParam CheckLookupTable(int lod, int left, int right, int bottom, int top);
	
private:
	
	int generateBoundaryMesh(int left_degree, int right_degree, int bottom_degree, int top_degree, RECT vert_rect, UINT* output);
	int generateInnerMesh(RECT vert_rect, UINT* output);
	
	D3D* m_pD3DSystem;
	
	OceanParameters mOceanParameters;
	
	ZShadeSandboxMesh::MeshAttributes* mAttributes;
	
	//Quad-tree LOD, 0 to 9 (1x1 ~ 512x512)
	int m_lods;
	
	//Pattern lookup array
	QuadRenderParam m_mesh_patterns[9][3][3][3][3];
};

//===============================================================================================================================
//===============================================================================================================================
#endif//__OCEANMESH_H