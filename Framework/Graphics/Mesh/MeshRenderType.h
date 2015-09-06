//===============================================================================================================================
// MeshRenderType.h
//
//===============================================================================================================================
// History
//
// -Created on 4/15/2015 by Dustin Watson
//===============================================================================================================================
#ifndef __MESHRENDERTYPE_H
#define __MESHRENDERTYPE_H
//===============================================================================================================================
//===============================================================================================================================
namespace ZShadeSandboxMesh
{
	namespace ERenderType
	{
		enum Type
		{
			eTriangleList,
			eTriangleStrip,
			ePointList,
			e3ControlPointPatchList,
			e4ControlPointPatchList
		};
	}
}
//===============================================================================================================================
//===============================================================================================================================
#endif//__MESHRENDERTYPE_H