//==============================================================================
// Material Light Shader
//
//==============================================================================
// History
//
// -Created on 5/17/2015 by Dustin Watson
//==============================================================================
#include "../Environment/Lighting/Lighting.hlsl"
#include "../Environment/Lighting/ShadowMap.hlsl"
#include "../Basic/Matrix.hlsl"

//======================================================================================================

//
// Constants
//

cbuffer cbShadingBuffer : register(b2)
{
	float3	g_EyePosW;
	float	g_Alpha;
	float4	g_ClipPlane;
	float4	g_MaterialDiffuseColor;
	float4	g_MaterialAmbientColor;
	float4	g_MaterialSpecularColor;
	float4	g_MaterialEmissiveColor;
	float	g_Emissivity;
	float	g_Reflectivity;
	float	g_Transmissivity;
	float	g_RefractionIndex;
	float3	g_AlphaToCoverageValue;
	float	g_DetailBrightness;
	float3	g_TransmissionFilter;
	float	g_MaterialSpecularPower;
	float	g_MaterialSpecularIntensity;
	int		g_IlluminationModel;
	int	   	g_FlipTextureH;
	int	   	g_FlipTextureV;
	int		g_UsingDiffuseArrayTexture;
	int	   	g_UsingDiffuseTexture;
	int	   	g_UsingAmbientTexture;
	int	   	g_UsingSpecularTexture;
	int	   	g_UsingEmissiveTexture;
	int	   	g_UsingNormalMapTexture;
	int	   	g_UsingBlendMapTexture;
	int	   	g_UsingDetailMapTexture;
	int		g_UsingAlphaMapTexture;
	int		g_UsingTransparency;
	int		g_UsingShadowMap;
	int		g_UsingSSAOMap;
	int		g_UsingDisplacementMap;
	float	g_FarPlane;
	int		g_SpecularToggle;
	int		g_EnableLighting;
	float2	matpadding;
	int		g_UsingInstancing;
	float	g_BlendAmount;
};

cbuffer cbLightTypeBuffer : register(b3)
{
	int		g_UsingAmbientLight;
	int		g_UsingDirectionalLight;
	int		g_UsingPointLight;
	int		g_UsingSpotLight;
	float3	lighttypepadding;
	int		g_UsingCapsuleLight;
};

cbuffer cbAmbientLightBuffer : register(b4)
{
	float4	g_AmbientColor;
	float3	ambientpadding;
	int		g_AmbientToggle;
};

cbuffer cbDirectionalLightBuffer : register(b5)
{
	float3	g_DirectionalDirection;
	float	g_DirectionalIntensity;
	float4	g_DirectionalAmbient;
	float4	g_DirectionalDiffuse;
	float3	directionalpadding;
	int		g_DirectionalToggle;
};

cbuffer cbSpotLightBuffer : register(b6)
{
	float4	g_SpotAmbientColor;
	float4	g_SpotDiffuseColor;
	float3	g_SpotLightPosition;
	float	g_SpotIntensity;
	float	g_SpotLightRange;
	float	g_SpotCosOuterCone;
	float	g_SpotInnerConeReciprocal;
	float	g_SpotCosineAngle;
	float3	spotpadding;
	int		g_SpotToggle;
};

cbuffer cbPointLightBuffer : register(b7)
{
	float3	g_PointLightPosition;
	float	g_PointLightRange;
	float3	g_PointAttenuation;
	float	g_PointIntensity;
	float4	g_PointAmbientColor;
	float4	g_PointDiffuseColor;
	float3	pointpadding;
	int		g_PointToggle;
};

cbuffer cbCapsuleLightBuffer : register(b8)
{
	float3 	g_CapsuleLightPosition;
	float 	g_CapsuleLightRange;
	float3 	g_CapsuleLightDirection;
	float 	g_CapsuleLightLength;
	float3 	g_CapsuleDirectionLength;
	float 	g_CapsuleIntensity;
	float4	g_CapsuleAmbientColor;
	float4	g_CapsuleDiffuseColor;
	float3	capsulepadding;
	int		g_CapsuleToggle;
};

cbuffer cbMatrixBuffer : register(b9)
{
    matrix g_World;
    matrix g_View;
    matrix g_Proj;
	matrix g_ShadowMatrix;
	matrix g_LightViewMatrix;
    matrix g_LightProjectionMatrix;
};

//======================================================================================================

//
// Textures and samplers
//

Texture2DArray g_MaterialDiffuseArrayTexture 	: register(t0);
Texture2D g_MaterialDiffuseTexture 				: register(t1);
Texture2D g_MaterialAmbientTexture 				: register(t2);
Texture2D g_MaterialSpecularTexture 			: register(t3);
Texture2D g_MaterialEmissiveTexture 			: register(t4);
Texture2D g_MaterialNormalMapTexture 			: register(t5);
Texture2D g_MaterialBlendMapTexture 			: register(t6);
Texture2D g_MaterialDetailMapTexture 			: register(t7);
Texture2D g_MaterialAlphaMapTexture 			: register(t8);
Texture2D g_ShadowMap							: register(t9);
Texture2D g_SSAOMap								: register(t10);
Texture2D g_DisplacementMap						: register(t11);

SamplerState g_PointSampler		: register(s0);
SamplerState g_LinearSampler	: register(s1);

//======================================================================================================

//
// Structs
//

struct VertexInput
{
	float3 position				: POSITION;
	float3 normal				: NORMAL;
	float2 uv					: TEXCOORD0;
	float3 instancePos			: INSTANCEPOS;
	float3 instanceRot			: INSTANCEROT;
	float3 instanceScl			: INSTANCESCL;
	uint instanceID				: SV_InstanceID;
};

struct PixelInput
{
	float4 position				: SV_POSITION;
	float3 positionW			: POSITION;
	float3 normal				: NORMAL;
	float2 uv					: TEXCOORD0;
	float  clip					: TEXCOORD1;
	float4 depth				: TEXCOORD2;
	float4 shadowPos			: TEXCOORD3;
	float4 lightViewPosition	: TEXCOORD4;
	float4 worldPos 			: TEXCOORD5;
	float instanceIndex			: TEXCOORD6;
};

//======================================================================================================

//
// Vertex Shader
//

PixelInput MaterialLightVS(VertexInput input)
{
	PixelInput output;
	
	float3 position = input.position;
	float4x4 world = g_World;
	
	if (g_UsingInstancing == 1)
	{
		if (g_UsingDisplacementMap == 1)
		{
			input.instancePos.y = g_DisplacementMap.SampleLevel(g_PointSampler, input.uv, 0).r;
		}
		
		output.instanceIndex = (float)input.instanceID;
		
		position.x += input.instancePos.x;
		position.y += input.instancePos.y;
		position.z += input.instancePos.z;
		
		// If the instance has a separate rotation and scale build the new world matrix
		world = CreateWorldMatrix(input.instanceRot, input.instanceScl);
	}
	else
	{
		if (g_UsingDisplacementMap == 1)
		{
			position.y = g_DisplacementMap.SampleLevel(g_PointSampler, input.uv, 0).r;
		}
		
		output.instanceIndex = 0.0f;
	}
	
    output.position = mul(float4(position, 1.0), world);
	output.position = mul(output.position, g_View);
	output.position = mul(output.position, g_Proj);
	
	output.positionW = position;
	
	output.uv = input.uv;
	
	output.normal = mul(input.normal, (float3x3)world);
	output.normal = normalize(output.normal);
	
	output.clip = dot(output.position, g_ClipPlane);
	output.depth = output.position;
	
	// Calculate the position of the vertices from the light source
	output.lightViewPosition = mul(float4(position, 1.0), world);
	output.lightViewPosition = mul(output.lightViewPosition, g_LightViewMatrix);
	output.lightViewPosition = mul(output.lightViewPosition, g_LightProjectionMatrix);
	
	output.worldPos = mul(float4(position, 1.0), world);
	
	output.shadowPos = mul(float4(position, 1.0), g_ShadowMatrix);
	
    return output;
}

//======================================================================================================

//
// Pixel Shader
//

float4 MaterialLightPS(PixelInput input) : SV_Target
{
	float4 finalColor;
    
	// 1 - u: flips the texture horizontally
	if (g_FlipTextureH == 1)
		input.uv.x = 1 - input.uv.x;
	
	// 1 - v: flips the texture vertically
	if (g_FlipTextureV == 1)
		input.uv.y = 1 - input.uv.y;
	
	//============================================ Texturing
	
	float4 finalTextureColor = float4(0, 0, 0, 1);
	float4 diffuseLayers0 = float4(0, 0, 0, 1);
	float4 diffuseLayers1 = float4(0, 0, 0, 1);
	float4 diffuseLayers2 = float4(0, 0, 0, 1);
	float4 diffuseLayers3 = float4(0, 0, 0, 1);
	float4 instanceTexture = float4(0, 0, 0, 1);
	
	if (g_UsingDiffuseArrayTexture == 1)
	{
		if (g_UsingInstancing == 1)
		{
			instanceTexture = g_MaterialDiffuseArrayTexture.Sample( g_LinearSampler, float3(input.uv, input.instanceIndex) );
		}
		else
		{
			// Sample layers in texture array.
			diffuseLayers0 = g_MaterialDiffuseArrayTexture.Sample( g_LinearSampler, float3(input.uv, 0.0f) );
			diffuseLayers1 = g_MaterialDiffuseArrayTexture.Sample( g_LinearSampler, float3(input.uv, 1.0f) );
			diffuseLayers2 = g_MaterialDiffuseArrayTexture.Sample( g_LinearSampler, float3(input.uv, 2.0f) );
			diffuseLayers3 = g_MaterialDiffuseArrayTexture.Sample( g_LinearSampler, float3(input.uv, 3.0f) );
		}
	}
	else
	{
		finalTextureColor = (g_UsingDiffuseTexture == 1) ? g_MaterialDiffuseTexture.Sample( g_LinearSampler, input.uv ) : g_MaterialDiffuseColor;
		//finalTextureColor += (g_UsingAmbientTexture == 1) ? g_MaterialAmbientTexture.Sample( g_LinearSampler, input.uv ) : g_MaterialAmbientColor;
		//finalTextureColor += (g_UsingSpecularTexture == 1) ? g_MaterialSpecularTexture.Sample( g_LinearSampler, input.uv ) : g_MaterialSpecularColor;
		//finalTextureColor += (g_UsingEmissiveTexture == 1) ? g_MaterialEmissiveTexture.Sample( g_LinearSampler, input.uv ) : g_MaterialEmissiveColor;
	}
    
	//============================================ Blend Mapping
	
	if (g_UsingBlendMapTexture == 1)
	{
		if (g_UsingDiffuseArrayTexture == 1)
		{
			// Sample the blend map.
			float4 t = g_MaterialBlendMapTexture.Sample( g_LinearSampler, input.uv );
			
			// Blend the layers on top of each other.
			finalTextureColor = diffuseLayers0;
			finalTextureColor = lerp(finalTextureColor, diffuseLayers1, t.r);
			finalTextureColor = lerp(finalTextureColor, diffuseLayers2, t.g);
			finalTextureColor = lerp(finalTextureColor, diffuseLayers3, t.b);
		}
	}
	else
	{
		if (g_UsingDiffuseArrayTexture == 1)
		{
			finalTextureColor = saturate(diffuseLayers0 + diffuseLayers1 + diffuseLayers2 + diffuseLayers3);
		}
	}
	
	if ((g_UsingInstancing == 1) && (g_UsingDiffuseArrayTexture == 1))
	{
		finalTextureColor += instanceTexture;
	}
	
	// Now add the texture color
	finalColor = finalTextureColor;
	
	//============================================ Normal Mapping
	
	float depth = input.depth.z / input.depth.w;
	
	float3 normal = input.normal;
	
	if (g_UsingNormalMapTexture == 1)
	{
		float4 normalMap = g_MaterialNormalMapTexture.Sample( g_LinearSampler, input.uv );
		normal = (normalMap.rgb * 2.0f) - 1.0f;
		normal = normalize(normal);
	}
	
	//============================================ Detail Mapping
	
	// Check if the depth value is close to the screen
	if (g_UsingDetailMapTexture == 1)
	{
		float4 detailColor = g_MaterialDetailMapTexture.Sample( g_LinearSampler, input.uv );
		finalColor *= detailColor * g_DetailBrightness;
	}
	
	//============================================ Shadow Mapping
	
	float shadowColor = 1.0f;
	
	float4 finalLightColor = float4(1, 1, 1, 1);
	
	if (g_UsingShadowMap == 1)
	{
		shadowColor = ComputeShadowColor2(input.lightViewPosition, normal, g_DirectionalLight[0].g_LightDirection, g_ShadowMap);
	}
	
	/* PCH Shadow Mapping
	if (g_UseShadowMap == 1)
	{
		float4 shadowPos = input.shadowPos;
		shadowPos.xyz /= shadowPos.w;
		float depth = shadowPos.z;
		const float dx = SMAP_DX;
		float percentLit = 0.0f;
		const float2 offsets[9] =
		{
			float2(-dx,  -dx), float2(0.0f,  -dx), float2(dx,  -dx),
			float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
			float2(-dx,  +dx), float2(0.0f,  +dx), float2(dx,  +dx)
		};
		
		[unroll]
		for (int i = 0; i < 9; i++)
		{
			percentLit += g_ShadowMap.SampleCmpLevelZero(ShadowMapSamplerComparison, shadowPos.xy + offsets[i], depth).r;
		}
		
		shadowColor = percentLit / 9.0f;
	}*/
	
	//============================================ SSAO
	
	float ssaoColor = 1.0f;
	
	if (g_UsingSSAOMap == 1)
	{
		
	}
	
	//============================================ Lighting
	float3 lightColor = float3(1, 1, 1);
	
	if (g_EnableLighting == 1)
	{
		if (g_UsingAmbientLight == 1 && g_AmbientToggle == 1)
		{
			lightColor = g_AmbientColor;
		}
		
		if (g_UsingDirectionalLight == 1 && g_DirectionalToggle == 1)
		{
			lightColor = DirectionalLightColor
			(	input.positionW
			,	g_EyePosW
			,	g_DirectionalDirection
			,	g_DirectionalIntensity
			,	g_DirectionalDiffuse
			,	normal
			,	g_MaterialDiffuseColor
			,	g_MaterialSpecularPower
			,	g_MaterialSpecularIntensity
			,   shadowColor
			);
		}
		
		if (g_UsingPointLight == 1 && g_AmbientToggle == 1)
		{
			lightColor = PointLightColor
			(	input.positionW
			,	g_EyePosW
			,	g_PointDiffuseColor
			,	g_PointLightPosition
			,	g_PointIntensity
			,	1.0 / g_PointLightRange
			,	normal
			,	g_MaterialDiffuseColor
			,	g_MaterialSpecularPower
			,	g_MaterialSpecularIntensity
			);
		}
		
		if (g_UsingSpotLight == 1 && g_AmbientToggle == 1)
		{
			lightColor = SpotLightColor
			(	input.positionW
			,	g_EyePosW
			,	g_SpotDiffuseColor
			,	g_SpotLightPosition
			,	g_SpotIntensity
			,	1 / g_SpotLightRange
			,	float3(0,0,0)
			,	g_SpotCosOuterCone
			,	g_SpotInnerConeReciprocal
			,	g_SpotCosineAngle
			,	normal
			,	g_MaterialDiffuseColor
			,	g_MaterialSpecularPower
			,	g_MaterialSpecularIntensity
			);
		}
		
		if (g_UsingCapsuleLight == 1 && g_AmbientToggle == 1)
		{
			lightColor = CapsuleLightColor
			(	input.positionW
			,	g_EyePosW
			,	g_CapsuleLightPosition
			,	g_CapsuleLightRange
			,	1.0 / g_CapsuleLightRange
			,	g_CapsuleLightDirection
			,	g_CapsuleLightLength
			,	g_CapsuleDiffuseColor
			,	g_CapsuleDirectionLength
			,	g_CapsuleIntensity
			,	normal
			,	g_MaterialDiffuseColor
			,	g_MaterialSpecularPower
			,	g_MaterialSpecularIntensity
			);
		}
	}
	
	finalColor *= float4(lightColor, g_Alpha);
	
	//============================================ Alpha Map Blending (Alpha-to-Coverage)
	
	if (g_UsingAlphaMapTexture == 1)
	{
		float4 alphaMapColor = g_MaterialAlphaMapTexture.Sample(g_LinearSampler, input.uv);
		if (alphaMapColor.r == g_AlphaToCoverageValue.x
		&&	alphaMapColor.g == g_AlphaToCoverageValue.y
		&&	alphaMapColor.b == g_AlphaToCoverageValue.z)
		{
			//Set the alpha value of this pixel to the blending amount to
			//create the alpha blending effect.
			if (g_UsingTransparency == 1)
				finalColor.a = g_BlendAmount;
			else
				clip(-1);
		}
	}
	else
	{
		finalColor.a = 1.0f;
	}
	
	//============================================ Clipping for water reflections and refractions
	
	clip( input.clip < 0.0 ? -1 : 1 );
	
	// return the final color
	return finalColor;
}

float4 MaterialLightWireframePS(PixelInput input) : SV_Target
{
	clip( input.clip < 0.0 ? -1 : 1 );
	
	return float4(0.9f, 0.9f, 0.9f, 1);
}