#include "Volume.h"
//==================================================================================================================================
//==================================================================================================================================
Volume::Volume(D3D* d3d, ZShadeSandboxMesh::MeshParameters mp)
:   ZShadeSandboxMesh::CubeMesh(d3d, mp)
{
}
//==================================================================================================================================
Volume::~Volume()
{
}
//==================================================================================================================================
//bool Volume::Intersects(XMFLOAT3 point)
//{
//	return boundary->ContainsPoint(point);
//}
//==================================================================================================================================
//bool Volume::Enters(ZShadeSandboxMesh::CustomMesh* mesh)
//{
//	return Collides(mesh);
//}
//==================================================================================================================================