//=====================================================================================
// Vertex.h
//=====================================================================================
#ifndef __VERTEX_H
#define __VERTEX_H
//==============================================================================================================================
//==============================================================================================================================
#include "ZMath.h"
#include "D3DIncludes.h"
//==============================================================================================================================
//==============================================================================================================================
namespace ZShadeSandboxMesh {
namespace EVertexType
{
	enum Type
	{
		VT_None,
		VT_Pos,
		VT_Color,
		VT_Tex,
		VT_NormalTex,
		VT_NormalTexTan,
		VT_NormalTexTanBi
	};
}
//==============================================================================================================================
//==============================================================================================================================
struct InstancePos
{
	XMFLOAT3 position;
};
//==============================================================================================================================
//==============================================================================================================================
struct InstanceMatrix
{
	XMFLOAT3 position;
	XMFLOAT3 rotation;
	XMFLOAT3 scale;
};
//==============================================================================================================================
//==============================================================================================================================
struct VertexPos
{
	XMFLOAT3 position;
};
//==============================================================================================================================
//==============================================================================================================================
struct VertexColor
{
	XMFLOAT3 position;
	XMFLOAT4 color;
};
//==============================================================================================================================
//==============================================================================================================================
struct VertexTex
{
	XMFLOAT3 position;
	XMFLOAT2 texture;
};
//==============================================================================================================================
//==============================================================================================================================
struct VertexNormalTex
{
	XMFLOAT3 position;
	XMFLOAT3 normal;
	XMFLOAT2 texture;
};
struct vertexVNTSort
{
	bool operator () (VertexNormalTex i, VertexNormalTex j)
	{
		return (i.position.x < j.position.x && i.position.y < j.position.y && i.position.z < j.position.z);
	}
};
inline bool myvertexVNTCheck(VertexNormalTex i, VertexNormalTex j)
{
	return (i.position.x == j.position.x && i.position.y == j.position.y && i.position.z == j.position.z);
}
//==============================================================================================================================
//==============================================================================================================================
struct VertexNormalTexTan
{
	XMFLOAT3 position;
	XMFLOAT3 normal;
	XMFLOAT2 texture;
	XMFLOAT3 tangentU;
};
//==============================================================================================================================
//==============================================================================================================================
struct VertexNormalTexTanBiTan
{
	XMFLOAT3 position;
	XMFLOAT3 normal;
	XMFLOAT2 texture;
	XMFLOAT4 tangent;
	XMFLOAT3 bitangent;
};
//==============================================================================================================================
//==============================================================================================================================
struct VertexTexSprite
{
	XMFLOAT2 position;
	XMFLOAT2 texture;
};
//==============================================================================================================================
//==============================================================================================================================
// Google: skin mesh shader
// http://forum.quest3d.com/index.php?topic=62710.0
struct VertexNormalTexTanSkinned
{
	XMFLOAT3 position;
	XMFLOAT3 normal;
	XMFLOAT2 texture;
	XMFLOAT3 tangent;
	XMFLOAT4 blendWeights;
	XMFLOAT4 blendIndices;
};
//==============================================================================================================================
//==============================================================================================================================
struct VertexLayout
{
	static D3D11_INPUT_ELEMENT_DESC mesh_layout_pos[1];
	static D3D11_INPUT_ELEMENT_DESC mesh_layout_pos_instance[2];
	static D3D11_INPUT_ELEMENT_DESC mesh_layout_pos_instance_rot_scl[4];
	
	static D3D11_INPUT_ELEMENT_DESC mesh_layout_pos_color[2];
	static D3D11_INPUT_ELEMENT_DESC mesh_layout_pos_color_instance[3];
	static D3D11_INPUT_ELEMENT_DESC mesh_layout_pos_color_instance_rot_scl[5];
	
	static D3D11_INPUT_ELEMENT_DESC mesh_layout_pos_tex[2];
	static D3D11_INPUT_ELEMENT_DESC mesh_layout_pos_tex_instance[3];
	static D3D11_INPUT_ELEMENT_DESC mesh_layout_pos_tex_instance_rot_scl[5];
	
	static D3D11_INPUT_ELEMENT_DESC mesh_layout_pos_normal_tex[3];
	static D3D11_INPUT_ELEMENT_DESC mesh_layout_pos_normal_tex_instance[4];
	static D3D11_INPUT_ELEMENT_DESC mesh_layout_pos_normal_tex_instance_rot_scl[6];
	
	static D3D11_INPUT_ELEMENT_DESC mesh_layout_pos_normal_tex_tan[4];
	static D3D11_INPUT_ELEMENT_DESC mesh_layout_pos_normal_tex_tan_instance[5];
	static D3D11_INPUT_ELEMENT_DESC mesh_layout_pos_normal_tex_tan_instance_rot_scl[7];
	
	static D3D11_INPUT_ELEMENT_DESC mesh_layout_pos_normal_tex_tan_bitan[5];
	static D3D11_INPUT_ELEMENT_DESC mesh_layout_pos_normal_tex_tan_bitan_instance[6];
	static D3D11_INPUT_ELEMENT_DESC mesh_layout_pos_normal_tex_tan_bitan_instance_rot_scl[8];
	
	static D3D11_INPUT_ELEMENT_DESC mesh_layout_sprite_pos_tex[2];
	static D3D11_INPUT_ELEMENT_DESC mesh_layout_sprite_instanced[8];
	
	static D3D11_INPUT_ELEMENT_DESC mesh_layout_pos_normal_tex_tan_skinned[6];
};
//===============================================================================================================================
//===============================================================================================================================
struct VertexUtil
{
	static ZShadeSandboxMesh::InstancePos LoadInstancePos(float x, float y, float z);
	static ZShadeSandboxMesh::VertexPos LoadVertexPos(float x, float y, float z);
	static ZShadeSandboxMesh::VertexColor LoadVertexColor(float x, float y, float z, float r, float g, float b, float a);
	static ZShadeSandboxMesh::VertexTex LoadVertexTex(float x, float y, float z, float u, float v);
	static ZShadeSandboxMesh::VertexNormalTex LoadVertexNormalTex(float x, float y, float z, float nx, float ny, float nz, float u, float v);
	static ZShadeSandboxMesh::VertexNormalTexTan LoadVertexNormalTexTan(float x, float y, float z, float nx, float ny, float nz, float u, float v, float tanx, float tany, float tanz);
	static ZShadeSandboxMesh::VertexNormalTexTanBiTan LoadVertexNormalTexTan(float x, float y, float z, float nx, float ny, float nz, float u, float v, float tanx, float tany, float tanz, float tanw, float btanx, float btany, float btanz);
	static ZShadeSandboxMesh::VertexNormalTexTanSkinned LoadVertexNormalTexTanSkinned(float x, float y, float z, float nx, float ny, float nz, float u, float v, float tanx, float tany, float tanz, float bw0, float bw1, float bw2, float bw3, float bi0, float bi1, float bi2, float bi3);
	
	static void InitVertex(ZShadeSandboxMesh::InstancePos& v);
	static void InitVertex(ZShadeSandboxMesh::VertexPos& v);
	static void InitVertex(ZShadeSandboxMesh::VertexColor& v);
	static void InitVertex(ZShadeSandboxMesh::VertexTex& v);
	static void InitVertex(ZShadeSandboxMesh::VertexNormalTex& v);
	static void InitVertex(ZShadeSandboxMesh::VertexNormalTexTan& v);
	static void InitVertex(ZShadeSandboxMesh::VertexNormalTexTanBiTan& v);
	static void InitVertex(ZShadeSandboxMesh::VertexNormalTexTanSkinned& v);
};
}
//==============================================================================================================================
//==============================================================================================================================
#endif//__VERTEX_H