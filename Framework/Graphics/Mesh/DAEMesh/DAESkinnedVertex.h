//===============================================================================================================================
// DAESkinnedVertex
//
//===============================================================================================================================
// History
//
// -Created on 8/31/2015 by Dustin Watson
//===============================================================================================================================
#ifndef __DAESKINNEDVERTEX_H
#define __DAESKINNEDVERTEX_H
//==============================================================================================================================
//==============================================================================================================================

//
// Includes
//

#include <vector>
#include "CGlobal.h"
#include "Vertex.h"
using namespace std;

//==============================================================================================================================
//==============================================================================================================================
namespace ZShadeSandboxMesh {
struct DAESkinnedVertex
{
	XMFLOAT3 position;
	XMFLOAT3 normal;
	XMFLOAT2 texture;
	XMFLOAT3 tangent;
	
	// 3 weights of each bone (often just 1, 0, 0 to use only 1 bone)
	XMFLOAT3 blendWeights;
	
	// Indices for used bones. Just floats because shader expect that and
	// we can't use UByte4 for lower than vs_2_0 hardware. Uses just int
	// values really.
	XMFLOAT3 blendIndices;
	
	bool EqualsPos(const DAESkinnedVertex& rhs);
	bool EqualsPosNorm(const DAESkinnedVertex& rhs);
	bool EqualsPosNormTex(const DAESkinnedVertex& rhs);
	bool EqualsPosNormTexTan(const DAESkinnedVertex& rhs);
	bool operator == (const DAESkinnedVertex& rhs);
};
}
//==============================================================================================================================
//==============================================================================================================================
#endif//__DAESKINNEDVERTEX_H