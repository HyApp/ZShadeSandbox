//===============================================================================================================================
// Bone.h
//
//===============================================================================================================================
// History
//
// -Created on 8/27/2015 by Dustin Watson
//===============================================================================================================================
#ifndef __BONE_H
#define __BONE_H
//===============================================================================================================================
//===============================================================================================================================

//
// Includes
//

#include <vector>
#include <string>
#include "XMMath.h"
#include "XMMatrix.h"
using std::vector;
using std::string;

//===============================================================================================================================
//===============================================================================================================================
namespace ZShadeSandboxMesh {
struct Bone
{
	Bone()
	:   parent(NULL)
	,	position(XMFLOAT3(0, 0, 0))
	,	number(0)
	,	id("")
	{
	}
	
	// Parent bone, very important to get all parent matrices when
    // building the finalMatrix for rendering.
	Bone* parent;
	
	vector<Bone*> children;
	
	// Position, very useful to position bones to show bones in 3D
	XMFLOAT3 position;
	
	// Initial matrix we get from loading the collada model
	ZShadeSandboxMath::XMMatrix initialMatrix;
	
	// Bone number for the skinning process
	int number;
	
	// Id and name of this bone
	string id;
	
	// Animation matrices for the precalculated bone animations
	// These matrices must be set each frame (use time) in order
    // for the animation to work.
	vector<ZShadeSandboxMath::XMMatrix> animationMatrices;
	
	// invBoneMatrix is a special helper matrix loaded directly from
	// the collada file. It is used to transform the final matrix
	// back to a relative format after transforming and rotating each
	// bone with the current animation frame. This is very important
	// because else we would always move and rotate vertices around the
	// center, but thanks to this inverted skin matrix the correct
	// rotation points are used.
	ZShadeSandboxMath::XMMatrix invBoneSkinMatrix;
	
	// Final absolute matrix, which is calculated in UpdateAnimation each
	// frame after all the loading is done. It can directly be used to
	// find out the current bone positions, but for rendering we have
	// to apply the invBoneSkinMatrix first to transform all vertices into
	// the local space.
	ZShadeSandboxMath::XMMatrix finalMatrix;
	
	ZShadeSandboxMath::XMMatrix GetMatrixRecursively()
	{
		ZShadeSandboxMath::XMMatrix ret = initialMatrix;
		
		// multiply the matrix with the parent mesh
		if (parent)
		{
			ret *= parent->GetMatrixRecursively();
		}
		
		return ret;
	}
	
	string toString()
	{
		string boneNumber = ZShadeSandboxGlobal::Convert::ConvertToString<int>(number);
		string bonePos = ZShadeSandboxGlobal::Convert::ConvertFloat3ToString(position);
		return "Bone: id=" + id + ", Number=" + boneNumber + ", Position=" + bonePos;
	}
};
}
//===============================================================================================================================
//===============================================================================================================================
#endif//__BONE_H