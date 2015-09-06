//===============================================================================================================================
// OceanRenderer
//
//===============================================================================================================================
// History
//
// -Created on 9/4/2015 by Dustin Watson
//===============================================================================================================================
#ifndef __OCEANRENDERER_H
#define __OCEANRENDERER_H
//===============================================================================================================================
//===============================================================================================================================

//
// Includes
//

#include "D3D.h"
#include "Camera.h"
#include "ZMath.h"
#include "XMMath.h"
#include "MeshAttributes.h"
#include "OceanParameters.h"
#include "OceanMesh.h"
#include "OceanSurfaceShader.h"
#include "OceanSimulationShader.h"

//===============================================================================================================================
//===============================================================================================================================

class OceanRenderer
{
public:
	
	// The simulation shader is created in the surface class but the surface shader needs
	// texture information created from it.
	OceanRenderer(D3D* d3d, OceanParameters params);
	
	void Init();
	
	void Render(Camera* camera);
	
	void SetReflectionMap(ID3D11ShaderResourceView* srv);
	void SetRefractionMap(ID3D11ShaderResourceView* srv);
	void SetDisplacementMap(ID3D11ShaderResourceView* srv);
	void SetGradientMap(ID3D11ShaderResourceView* srv);
	void SetPerlinMap(ID3D11ShaderResourceView* srv);
	
	void SetWireframe(bool wire);
	
private:
	
	int BuildNodeList(OceanMesh::QuadNode& quad_node, Camera* camera);
	
	bool CheckNodeVisibility(const OceanMesh::QuadNode& quad_node, Camera* camera);
	
	OceanMesh::QuadRenderParam& SelectMeshPattern(const OceanMesh::QuadNode& quad_node);
	
	float EstimateGridCoverage(const OceanMesh::QuadNode& quad_node, Camera* camera, float screen_area);
	
	bool IsLeaf(const OceanMesh::QuadNode& quad_node);
	
	int SearchLeaf(const vector<OceanMesh::QuadNode>& node_list, const ZShadeSandboxMath::XMMath2& point);
	
private:
	
	D3D* m_pD3DSystem;
	
	OceanParameters mOceanParameters;
	
	OceanSurfaceShader* mSurfaceShader;
	
	OceanMesh* mMesh;
	
	// Rendering list for the quad nodes in the mesh
	vector<OceanMesh::QuadNode> m_render_list;
};

//===============================================================================================================================
//===============================================================================================================================
#endif//__OCEANRENDERER_H