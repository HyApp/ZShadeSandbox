//==============================================================================================================================
// DirectionalLight.h
//
//==============================================================================================================================
// History
//
// -Created on 4/22/2015  by Dustin Watson
//==============================================================================================================================
#ifndef __DIRECTIONALLIGHT_H
#define __DIRECTIONALLIGHT_H
//==============================================================================================================================
//==============================================================================================================================

//
// Includes
//

#include "D3D.h"
#include "Light.h"

//==============================================================================================================================
//==============================================================================================================================
namespace ZShadeSandboxLighting {
class DirectionalLight : public ZShadeSandboxLighting::Light
{
public:
	
	DirectionalLight();

	ZShadeSandboxLighting::Light* Clone();
};
}
//==============================================================================================================================
//==============================================================================================================================
#endif//__DIRECTIONALLIGHT_H