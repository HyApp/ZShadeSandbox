//==================================================================================================================================
// Bokeh.h
//
//==================================================================================================================================
// History
//
// -Created on 4/20/2015 by Dustin Watson
//==================================================================================================================================
#ifndef __BOKEH_H
#define __BOKEH_H
//==================================================================================================================================
//==================================================================================================================================
#include "D3D.h"
#include "PostProcess.h"
//==================================================================================================================================
class Bokeh : public PostProcess
{
public:
	
	Bokeh(D3D* d3d);
	~Bokeh();
	
	void RenderInit(ZShadeSandboxShader::Shader*& shader);
	
private:
	
};
//==================================================================================================================================
//==================================================================================================================================
#endif//__BOKEH_H