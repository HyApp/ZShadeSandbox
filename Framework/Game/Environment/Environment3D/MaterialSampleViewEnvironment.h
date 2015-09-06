//===============================================================================================================================
// MaterialSampleViewEnvironment
//
// The Material Sample View will use a sphere with a directional light rendered with the light shader to view a material
// Also might use other lights.
//===============================================================================================================================
// History
//
//	-Created on 8/25/2015 by Dustin Watson
//===============================================================================================================================
#ifndef __MATERIALSAMPLEVIEWENVIRONMENT_H
#define __MATERIALSAMPLEVIEWENVIRONMENT_H
//===============================================================================================================================
//===============================================================================================================================

//
// Includes
//

#include "InteractiveEnvironment3D.h"
#include "DirectionalLight.h"
#include "SpotLight.h"
#include "PointLight.h"
#include "CapsuleLight.h"
#include "LightManager.h"
#include "MaterialSampleView.h"

//===============================================================================================================================
//===============================================================================================================================

//
// MaterialSampleView Environment
//
class MaterialSampleViewEnvironment : public InteractiveEnvironment3D
{
public:
	
	MaterialSampleViewEnvironment(EngineOptions* eo);
	~MaterialSampleViewEnvironment();
	
	bool Initialize();
	void Shutdown();
	
	void Update();
	void Render();
	void RenderDeferred();
	
private:
	
	MaterialSampleView* mMaterialSampleView;
	
	ZShadeSandboxLighting::DirectionalLight* mDirLight;
	
	// Not used yet
	ZShadeSandboxLighting::SpotLight* mSpotLight;
	ZShadeSandboxLighting::PointLight* mPointLight;
	ZShadeSandboxLighting::CapsuleLight* mCapsuleLight;
};

//===============================================================================================================================
//===============================================================================================================================
#endif//__MATERIALSAMPLEVIEWENVIRONMENT_H