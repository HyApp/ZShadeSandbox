#include "DAESkinnedVertex.h"
#include "ZMath.h"
#include <algorithm>
//==============================================================================================================================
//==============================================================================================================================
bool ZShadeSandboxMesh::DAESkinnedVertex::operator == (const ZShadeSandboxMesh::DAESkinnedVertex& rhs)
{
	return EqualsPosNormTexTan(rhs);
}
//==============================================================================================================================
bool ZShadeSandboxMesh::DAESkinnedVertex::EqualsPos(const ZShadeSandboxMesh::DAESkinnedVertex& rhs)
{
	ZShadeSandboxMath::XMMath3 thisPos(position.x, position.y, position.z);
	ZShadeSandboxMath::XMMath3 rhsPos(rhs.position.x, rhs.position.y, rhs.position.z);
	return (thisPos == rhsPos);
}
//==============================================================================================================================
bool ZShadeSandboxMesh::DAESkinnedVertex::EqualsPosNorm(const ZShadeSandboxMesh::DAESkinnedVertex& rhs)
{
	ZShadeSandboxMath::XMMath3 thisPos(position.x, position.y, position.z);
	ZShadeSandboxMath::XMMath3 rhsPos(rhs.position.x, rhs.position.y, rhs.position.z);
	bool result1 = (thisPos == rhsPos);
	
	ZShadeSandboxMath::XMMath3 tNormal(normal.x, normal.y, normal.z);
	ZShadeSandboxMath::XMMath3 rhsNormal(rhs.normal.x, rhs.normal.y, rhs.normal.z);
	bool result2 = (tNormal - rhsNormal).Length() < 0.1f;
	
	return result1 && result2;
}
//==============================================================================================================================
bool ZShadeSandboxMesh::DAESkinnedVertex::EqualsPosNormTex(const ZShadeSandboxMesh::DAESkinnedVertex& rhs)
{
	ZShadeSandboxMath::XMMath3 thisPos(position.x, position.y, position.z);
	ZShadeSandboxMath::XMMath3 rhsPos(rhs.position.x, rhs.position.y, rhs.position.z);
	bool result1 = (thisPos == rhsPos);
	
	ZShadeSandboxMath::XMMath2 thisUV(texture.x, texture.y);
	ZShadeSandboxMath::XMMath2 rhsUV(rhs.texture.x, rhs.texture.y);
	bool resultUV1 = ZShadeSandboxMath::ZMath::Abs(thisUV.x - rhsUV.x) < 0.001f;
	bool resultUV2 = ZShadeSandboxMath::ZMath::Abs(thisUV.y - rhsUV.y) < 0.001f;
	bool result2 = resultUV1 && resultUV2;
	
	ZShadeSandboxMath::XMMath3 tNormal(normal.x, normal.y, normal.z);
	ZShadeSandboxMath::XMMath3 rhsNormal(rhs.normal.x, rhs.normal.y, rhs.normal.z);
	bool result3 = (tNormal - rhsNormal).Length() < 0.1f;
	
	return result1 && result2 && result3;
}
//==============================================================================================================================
bool ZShadeSandboxMesh::DAESkinnedVertex::EqualsPosNormTexTan(const ZShadeSandboxMesh::DAESkinnedVertex& rhs)
{
	ZShadeSandboxMath::XMMath3 thisPos(position.x, position.y, position.z);
	ZShadeSandboxMath::XMMath3 rhsPos(rhs.position.x, rhs.position.y, rhs.position.z);
	bool result1 = (thisPos == rhsPos);
	
	ZShadeSandboxMath::XMMath2 thisUV(texture.x, texture.y);
	ZShadeSandboxMath::XMMath2 rhsUV(rhs.texture.x, rhs.texture.y);
	bool resultUV1 = ZShadeSandboxMath::ZMath::Abs(thisUV.x - rhsUV.x) < 0.001f;
	bool resultUV2 = ZShadeSandboxMath::ZMath::Abs(thisUV.y - rhsUV.y) < 0.001f;
	bool result2 = resultUV1 && resultUV2;
	
	ZShadeSandboxMath::XMMath3 tNormal(normal.x, normal.y, normal.z);
	ZShadeSandboxMath::XMMath3 rhsNormal(rhs.normal.x, rhs.normal.y, rhs.normal.z);
	bool result3 = (tNormal - rhsNormal).Length() < 0.1f;
	
	ZShadeSandboxMath::XMMath3 tTangent(tangent.x, tangent.y, tangent.z);
	ZShadeSandboxMath::XMMath3 rhsTangent(rhs.tangent.x, rhs.tangent.y, rhs.tangent.z);
	bool result4 = (tTangent - rhsTangent).Length() < 0.1f;
	
	return result1 && result2 && result3 && result4;
}
//==============================================================================================================================