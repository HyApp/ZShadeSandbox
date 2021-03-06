//==================================================================================================================================
// DepthOfField.h
//
//==================================================================================================================================
// History
//
// -Created on 4/20/2015 by Dustin Watson
//==================================================================================================================================
#ifndef __DEPTHOFFIELD_H
#define __DEPTHOFFIELD_H
//==================================================================================================================================
//==================================================================================================================================
#include "D3D.h"
#include "PostProcess.h"
//==================================================================================================================================
class DepthOfField : public PostProcess
{
public:
	
	DepthOfField(D3D* d3d);
	~DepthOfField();
	
	void RenderInit(ZShadeSandboxShader::Shader*& shader);
	
private:
	
};
//==================================================================================================================================
//==================================================================================================================================
#endif//__DEPTHOFFIELD_H